#include "precomp.h"

// -----------------------------------------------------------
// Initialize the renderer
// -----------------------------------------------------------
void Dots::Init() {
	SpawnAnts();
	uint color = RGBF32_to_RGB8(&backgroundColor);
	screen->Clear(color);
}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void Dots::Tick(float deltaTime) {
	deltaTime /= 100.0f;

	Timer t;
	UpdateAnts(deltaTime);
	updateAntsTime = t.elapsed();

	t.reset();
	const uint color = RGBF32_to_RGB8(&backgroundColor);
	screen->Clear(color, evaporateSpeed);
	clearScreenTime = t.elapsed();

	t.reset();
	RenderAnts();
	renderAntsTime = t.elapsed();

	//PerformanceReport(t);
}

void Dots::UpdateAnts(const float deltaTime) {
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < ants.size(); i++) {
		Ant& ant = ants[i];

		if (wander) {
			ant.desiredDirection = normalize(ant.desiredDirection + RandomInsideUnitCircle() * WanderStrength);
		} else {
			ant.desiredDirection = normalize(mousePos - ant.position);
		}

		float2 desiredVelocity = ant.desiredDirection * MaxSpeed;
		float2 desiredSteeringForce = (desiredVelocity - ant.velocity) * SteerStrength;
		float2 acceleration = clampMagnitude(desiredSteeringForce, SteerStrength) / 1.0f;

		ant.velocity = clampMagnitude(ant.velocity + acceleration * deltaTime, MaxSpeed);
		ant.position += ant.velocity * deltaTime;

		if (ant.position.x > SCRWIDTH) ant.position.x = ant.position.x - SCRWIDTH;
		if (ant.position.x < 0) ant.position.x = SCRWIDTH - ant.position.x;
		if (ant.position.y > SCRHEIGHT) ant.position.y = ant.position.y - SCRHEIGHT;
		if (ant.position.y < 0) ant.position.y = SCRHEIGHT - ant.position.y;

		ant.angle = atan2(ant.velocity.y, ant.velocity.x);
	}
}

void Dots::RenderAnts() {
	const float2 size = float2(antSize);
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < ants.size(); i++) {
		const Ant& ant = ants[i];
		const float2 pos = ant.position;
#if 0
		const float2 min = pos - size * 0.5f;
		const float2 max = pos + size * 0.5f;
		screen->Rect(min.x, min.y, max.x, max.y, ant.angle, color);
#else
#if HDR
		screen->Plot(pos.x, pos.y, antColor);
#else
		uint color = RGBF32_to_RGB8(&antColor);
		screen->Plot(pos.x, pos.y, color);
#endif
#endif
	}
}

void Dots::SubtractScreen(const float deltaTime) {
	if (!clearScreen) return;
#pragma omp parallel for schedule(dynamic)
	for (int y = 0; y < SCRHEIGHT; y++) {
		for (int x = 0; x < SCRWIDTH; x++) {
			const float4 color = screen->GetPixel(x, y);
			if (color == backgroundColor) continue;
			const float4 evaporatedValue = max(float4(0.0f), color - evaporateSpeed * deltaTime);
#if HDR
			screen->Plot(x, y, evaporatedValue);
#else
			uint evaporatedColor = RGBF32_to_RGB8(&evaporatedValue);
			screen->Plot(x, y, evaporatedColor);
#endif
		}
	}
}

void Dots::SpawnAnts() {
	//initialize the ants
	for (int i = 0; i < antCount; i++) {
		Ant ant;
		ant.position = float2(SCRWIDTH * 0.5f, SCRHEIGHT * 0.5f);
		ants.push_back(ant);
	}
}

void Dots::ClearAnts() {
	ants.clear();
}

// -----------------------------------------------------------
// Update user interface (imgui)
// -----------------------------------------------------------
void Dots::UI() {
	ImGui::Begin("Ant Settings");

	ImGui::Text("Update Ants Time: %f ms", updateAntsTime * 1000.0f);
	ImGui::Text("Clear Screen Time: %f ms", clearScreenTime * 1000.0f);
	ImGui::Text("Render Ants Time: %f ms", renderAntsTime * 1000.0f);

	ImGui::DragInt("Ant Count", &antCount, 1, 0, 1000000);
	if (ImGui::Button("Spawn Ants")) {
		SpawnAnts();
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear Ants")) {
		ClearAnts();
	}
	if (ImGui::Button("Clear Screen")) {
		uint color = RGBF32_to_RGB8(&backgroundColor);
		screen->Clear(color);
	}

	ImGui::DragFloat("Ant Size", &antSize, 0.1f, 0.0f, 100.0f);
	ImGui::ColorEdit4("Ant Color", &antColor.x);
	ImGui::ColorEdit4("Background Color", &backgroundColor.x);
	ImGui::DragFloat("Evaporate Speed", &evaporateSpeed, 0.0001f, 0.0f, 1.0f, "%.10f");
	ImGui::DragFloat("MaxSpeed", &MaxSpeed, 0.1f, 0.0f, 10.0f);
	ImGui::DragFloat("SteerStrength", &SteerStrength, 0.1f, 0.0f, 10.0f);
	ImGui::DragFloat("WanderStrength", &WanderStrength, 0.1f, 0.0f, 10.0f);
	ImGui::Checkbox("Wander", &wander);
	ImGui::Checkbox("Clear Screen toggle", &clearScreen);
	ImGui::End();
}

// -----------------------------------------------------------
// User wants to close down
// -----------------------------------------------------------
void Dots::Shutdown() {

}

void Dots::MouseUp(int button) {
	if (!mouseDown) return;
	mouseDown = false;
	for (auto& ant : ants) {
		wander = true;
	}
	wander = true;
}

void Dots::MouseDown(int button) {
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse) return;

	mouseDown = true;
	for (auto& ant : ants) {
		wander = false;
	}
	wander = false;
}

void Dots::PerformanceReport(Timer& t) {
	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if (alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000.0f / avg, rps = (SCRWIDTH * SCRHEIGHT) / avg;
	printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps / 1000);
}