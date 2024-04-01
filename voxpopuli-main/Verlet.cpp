#include "precomp.h"

// -----------------------------------------------------------
// Initialize the renderer
// -----------------------------------------------------------
void Verlet::Init() {
	SpawnParticles();
}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void Verlet::Tick(float deltaTime) {
	printf("deltaTime: %f\n", deltaTime);
	Timer t;
	screen->Clear(0xffffff);

	if (mouseDown) {
		uint randomColor = rand() % 0xffffff;
		SpawnParticle(float2(mousePos.x, mousePos.y), float2(0, 0), spawnSize, randomColor);
	}

	const float stepDeltaTime = deltaTime / static_cast<float>(subSteps);
	for (int i = 0; i < subSteps; i++) {
		Update(stepDeltaTime);
	}

	DrawParticles();

	//PerformanceReport(t);
}

// -----------------------------------------------------------
// Update user interface (imgui)
// -----------------------------------------------------------
void Verlet::UI() {
	ImGui::Begin("Settings");
	ImGui::DragInt("Spawn Count", &spawnCount, 0, 1000);
	ImGui::SliderFloat("Spawn Size", &spawnSize, 0.0f, 100.0f);
	if (ImGui::Button("Spawn")) {
		SpawnParticles();
	}
	//clear
	if (ImGui::Button("Clear")) {
		ClearParticles();
	}
	//contraints
	ImGui::SliderFloat("Constraint Radius", &constraintRadius, 0.0f, 1000.0f);
	ImGui::DragFloat2("Constraint Pos", &constraintPosition.x, 0.0f, SCRWIDTH);

	//tickrate
	ImGui::SliderFloat("Tick Rate", &tickRate, 0.0f, 100.0f);

	//update button
	if (ImGui::Button("Update")) {
		Update(1.0f / 60.0f);
	}

	//substeps
	ImGui::SliderInt("Substeps", &subSteps, 1, 10);

	//gravity
	ImGui::SliderFloat("Gravity", &gravity.y, 0.0f, 0.01f);
}

// -----------------------------------------------------------
// User wants to close down
// -----------------------------------------------------------
void Verlet::Shutdown() {

}

void Tmpl8::Verlet::MouseUp(int button) {
	if (button == 0) {
		mouseDown = false;
	}
}

void Tmpl8::Verlet::MouseDown(int button) {
	if (button == 0 && !ImGui::GetIO().WantCaptureMouse) {
		mouseDown = true;

	}
}

void Tmpl8::Verlet::ApplyGravity() {
	for (int i = 0; i < particleCount; i++) {
		accelerations[i] += gravity;
	}
}

void Tmpl8::Verlet::ApplyConstraints() {
	for (int i = 0; i < particleCount; i++) {
		const float2 delta = currentPositions[i] - constraintPosition;
		const float distance = sqrt(delta.x * delta.x + delta.y * delta.y);
		const float size = sizes[i];
		if (distance > constraintRadius - size) {
			const float2 normal = delta / distance;
			currentPositions[i] = constraintPosition + normal * (constraintRadius - size);
		}
	}

}

void Tmpl8::Verlet::UpdatePositions(const float deltaTime) {
	for (int i = 0; i < particleCount; i++) {
		const float2 velocity = currentPositions[i] - previousPositions[i];
		previousPositions[i] = currentPositions[i];
		currentPositions[i] += velocity + accelerations[i] * deltaTime * deltaTime;
		accelerations[i] = float2(0, 0);
	}
}

void Tmpl8::Verlet::DrawParticles() {

	screen->Circle(constraintPosition.x, constraintPosition.y, constraintRadius, 0);

	for (int i = 0; i < particleCount; i++) {
		const uint color = colors[i];
		screen->Circle(currentPositions[i].x, currentPositions[i].y, sizes[i], color);
	}
}

void Tmpl8::Verlet::ClearParticles() {
	currentPositions.clear();
	previousPositions.clear();
	accelerations.clear();
	sizes.clear();
	colors.clear();
	particleCount = 0;
}

void Tmpl8::Verlet::SpawnParticles() {
	for (int i = 0; i < spawnCount; i++) {
		float2 random = RandomInsideUnitCircle() * (constraintRadius - float2(spawnSize / 2.0f));
		random += constraintPosition;
		currentPositions.push_back(random);
		previousPositions.push_back(random);
		accelerations.push_back(float2(0, 0));
		sizes.push_back(spawnSize);
		colors.push_back(rand() % 0xffffff);
		particleCount++;
	}
}

void Tmpl8::Verlet::ResolveCollisions() {
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < particleCount; i++) {
		for (int j = i + 1; j < particleCount; j++) {

			const float2 axis = currentPositions[i] - currentPositions[j];
			const float distance = length(axis);
			const float size = sizes[i] + sizes[j];
			if (distance < size) {
				const float2 normal = axis / distance;
				const float overlap = size - distance;
				currentPositions[i] += normal * overlap * 0.5f;
				currentPositions[j] -= normal * overlap * 0.5f;
			}
		}
	}
}

void Tmpl8::Verlet::Update(const float deltaTime) {
	ApplyGravity();
	ApplyConstraints();
	ResolveCollisions();
	UpdatePositions(deltaTime);
}

void Tmpl8::Verlet::SpawnParticle(const float2& position, const float2& velocity, const float size, const uint color) {
	currentPositions.push_back(position);
	previousPositions.push_back(position - velocity);
	accelerations.push_back(float2(0, 0));
	sizes.push_back(size);
	colors.push_back(color);
	particleCount++;
}


void Verlet::PerformanceReport(Timer& t) {
	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if (alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000.0f / avg, rps = (SCRWIDTH * SCRHEIGHT) / avg;
	printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps / 1000);
}