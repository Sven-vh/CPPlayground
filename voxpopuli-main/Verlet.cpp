#include "precomp.h"

// -----------------------------------------------------------
// Initialize the renderer
// -----------------------------------------------------------
void Verlet::Init() {
	ResizeGrid();
	SpawnParticles();
}

void Tmpl8::Verlet::ResizeGrid() {
	gridDimensions = int2(SCRWIDTH / (particleSize * 2.0f), SCRHEIGHT / (particleSize * 2.0f));
	grid.resize(gridDimensions.x);
	for (int i = 0; i < gridDimensions.x; i++) {
		grid[i].resize(gridDimensions.y);
		for (int j = 0; j < gridDimensions.y; j++) {
			grid[i][j].particles.reserve(10);
		}
	}
}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void Verlet::Tick(float deltaTime) {
	//printf("deltaTime: %f\n", deltaTime);
	Timer tTotal;
	if (mouseDown) {
		//uint randomColor = rand() % 0xffffff;
		//SpawnParticle(float2(mousePos.x, mousePos.y), float2(0, 0), randomColor);
		//Particle& p = particles[0];
		//p.position = float2(mousePos.x, mousePos.y);
		//p.previousPosition = p.position;
		//p.acceleration = float2(0, 0);
	}

	//clear the console



	const float stepDeltaTime = deltaTime / static_cast<float>(subSteps);
	for (int i = 0; i < subSteps; i++) {
		if (paused) break;
		Update(stepDeltaTime);
	}

	Timer t;
	t.reset();
	DrawParticles();
	drawParticle = t.elapsed();
	t.reset();
	DrawGrids();
	drawGridTime = t.elapsed();

	PerformanceReport(tTotal);
}

void Tmpl8::Verlet::Update(const float deltaTime) {
	Timer t;
	UpdateGrid();
	updateGrid += t.elapsed();
	t.reset();
	ResolveCollisions();
	resolveCollisions = t.elapsed();
	t.reset();
	UpdatePositions(deltaTime);
	updatePositions = t.elapsed();
	t.reset();
	ApplyConstraints();
	applyConstraints = t.elapsed();
}

// -----------------------------------------------------------
// Update user interface (imgui)
// -----------------------------------------------------------
void Verlet::UI() {
	ImGui::Begin("Settings");

	//imgui text with the amount of particles
	std::string particleNumString = std::to_string(particles.size());
	std::string formattedNumAnts = "";
	int count = 0;
	for (int i = particleNumString.size() - 1; i >= 0; i--) {
		formattedNumAnts = particleNumString[i] + formattedNumAnts;
		count++;
		if (count % 3 == 0 && i != 0) {
			formattedNumAnts = "," + formattedNumAnts;
		}
	}
	ImGui::Text("Particles: %s", formattedNumAnts.c_str());

	//times
	ImGui::Text("Update Grid: %f", updateGrid);
	ImGui::Text("Apply Gravity: %f", applyGravity);
	ImGui::Text("Apply Constraints: %f", applyConstraints);
	ImGui::Text("Resolve Collisions: %f", resolveCollisions);
	ImGui::Text("Update Positions: %f", updatePositions);
	ImGui::Text("Draw Particles: %f", drawParticle);
	ImGui::Text("Draw Grid time: %f", drawGridTime);

	//reset update times
	updateGrid = 0.0f;
	applyGravity = 0.0f;
	applyConstraints = 0.0f;
	resolveCollisions = 0.0f;
	updatePositions = 0.0f;
	drawParticle = 0.0f;
	drawGridTime = 0.0f;

	//grid size
	ImGui::Text("Grid Size: %d x %d", gridDimensions.x, gridDimensions.y);

	ImGui::DragInt("Spawn Count", &spawnCount, 0, 1000);
	if (ImGui::SliderFloat("Spawn Size", &particleSize, 0.0f, 100.0f)) {
		ResizeGrid();
	}
	ImGui::SliderFloat("Drawing Size Multiplier", &drawingSizeMultiplier, 0.0f, 10.0f);
	//max velocity
	ImGui::SliderFloat("Max Velocity", &maxVelocity, 0.0f, 100.0f);
	//max color velocity
	ImGui::SliderFloat("Max Color Velocity", &maxColorVelocity, 0.0f, 5.0f);
	if (ImGui::Button("Spawn")) {
		SpawnParticles();
	}
	//clear
	if (ImGui::Button("Clear")) {
		ClearParticles();
	}
	//contraints
	ImGui::Checkbox("Circle Constraint", &circleConstraint);
	ImGui::Checkbox("Draw Grid bool", &drawGrid);
	ImGui::SliderFloat("Constraint Radius", &constraintRadius, 0.0f, 1000.0f);
	ImGui::DragFloat2("Constraint Pos", &constraintPosition.x, 0.0f, SCRWIDTH);

	//tickrate
	ImGui::Checkbox("Paused", &paused);
	ImGui::SliderFloat("Tick Rate", &tickRate, 0.0f, 100.0f);

	//update button
	if (ImGui::Button("Update")) {
		Update(1.0f / 60.0f);
	}

	//substeps
	ImGui::SliderInt("Substeps", &subSteps, 1, 10);

	//gravity
	ImGui::SliderFloat("Gravity", &gravity.y, 0.0f, 0.01f);
	ImGui::SliderFloat("Mouse Force", &mouseForce, -0.0001f, 0.0001f, "%.8f");
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

void Tmpl8::Verlet::ApplyConstraints() {
	if (circleConstraint) {
		for (auto& p : particles) {
			const float2 delta = p.position - constraintPosition;
			const float distance = sqrt(delta.x * delta.x + delta.y * delta.y);
			if (distance > constraintRadius - particleSize) {
				const float2 normal = delta / distance;
				p.position = constraintPosition + normal * (constraintRadius - particleSize);
			}
		}
	} else {
		//only loop through the particles that are in the cells on the edge of the screen
		for (int i = 0; i < gridDimensions.x; i++) {
			for (int j = 0; j < gridDimensions.y; j++) {
				if (i == 0 || i == gridDimensions.x - 1 || j == 0 || j == gridDimensions.y - 1) {
					Cell& cell = grid[i][j];
					for (const int index : cell.particles) {
						Particle& p = particles[index];
						const float2& pos = p.position;
						if (pos.x < particleSize) {
							p.position.x = particleSize;
							p.previousPosition.x = particleSize;
						} else if (pos.x >= SCRWIDTH - particleSize) {
							p.position.x = SCRWIDTH - particleSize;
							p.previousPosition.x = SCRWIDTH - particleSize;
						}
						if (pos.y < particleSize) {
							p.position.y = particleSize;
							p.previousPosition.y = particleSize;
						} else if (pos.y > SCRHEIGHT - particleSize) {
							p.position.y = SCRHEIGHT - particleSize;
							p.previousPosition.y = SCRHEIGHT - particleSize;
						}
					}
				}
			}
		}
	}

}

void Tmpl8::Verlet::UpdatePositions(const float deltaTime) {

	for (auto& p : particles) {
		if (mouseDown) {
			//p.acceleration += (float2(mousePos.x, mousePos.y) - p.position) * mouseForce;
			//p.acceleration += (float2(mousePos.x, mousePos.y) - p.position) * gravity.y * mouseForce;
			//make the the particles follow the mouse by using gravity, based on the distance to the mouse
			float distance = length(float2(mousePos.x, mousePos.y) - p.position);
			p.acceleration += (float2(mousePos.x, mousePos.y) - p.position) * gravity.y * mouseForce / distance;
		}
		p.acceleration += gravity;
		float2 velocity = p.position - p.previousPosition;
		float speed = length(velocity);
		if (speed > maxVelocity) {
			velocity = normalize(velocity) * maxVelocity;
		}
		p.previousPosition = p.position;
		p.position += velocity + p.acceleration * (deltaTime * deltaTime);
		p.acceleration = float2(0, 0);
	}
}

void Tmpl8::Verlet::SolveCollision(Particle& a, Particle& b) const {
	const float2 axis = a.position - b.position;
	const float distance = length(axis);
	const float size = particleSize + particleSize;

	if (distance < size && distance > FLT_EPSILON) {  // Added epsilon check
		const float2 normal = axis / distance;
		const float overlap = size - distance;
		float2 result = normal * overlap * 0.5f;
		a.position += result;
		b.position -= result;
	}
}

void Tmpl8::Verlet::DrawParticles() {
	if (circleConstraint) {
		screen->Clear(0xffffff);
		screen->Circle(constraintPosition.x, constraintPosition.y, constraintRadius, 0);
	} else {
		screen->Clear(0);
	}
	//for (const auto& p : particles) {
	//	float4 color;
	//	float speed = length(p.position - p.previousPosition);
	//	float ratio = speed / maxColorVelocity;
	//	color = float4(ratio, 1.0f - ratio, 0.0f, 1.0f);

	//	screen->Circle(p.position.x, p.position.y, particleSize * drawingSizeMultiplier, RGBF32_to_RGB8(&color));
	//}

	//draw from top left to bottom right
	float4 fastColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float4 slowColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
	for (int i = 0; i < gridDimensions.x; i++) {
		for (int j = 0; j < gridDimensions.y; j++) {
			Cell& cell = grid[i][j];
			for (const int index : cell.particles) {
				Particle& p = particles[index];
				float4 color;
				float speed = length(p.position - p.previousPosition);
				float ratio = speed / maxColorVelocity;
				color = fastColor * ratio + slowColor * (1.0f - ratio);
				screen->Circle(p.position.x, p.position.y, particleSize * drawingSizeMultiplier, RGBF32_to_RGB8(&color));
			}
		}
	}
}


void Tmpl8::Verlet::DrawGrids() {
	if (!drawGrid) return;
	const float boxSize = particleSize * 2.0f;
	for (int i = 0; i < gridDimensions.x; i++) {
		for (int j = 0; j < gridDimensions.y; j++) {
			//if it's empty, dont draw
			if (grid[i][j].particles.empty()) {
				continue;
			}
			screen->Box(i * boxSize, j * boxSize, i * boxSize + boxSize, j * boxSize + boxSize, 0xffffff);
		}
	}
}

void Tmpl8::Verlet::ClearParticles() {
	particles.clear();
}

void Tmpl8::Verlet::SpawnParticles() {
	float2 random = float2(RandomFloat(), RandomFloat());
	for (int i = 0; i < spawnCount; i++) {
		float2 startPos;
		if (circleConstraint) {
			startPos = RandomInsideUnitCircle() * (constraintRadius - float2(particleSize / 2.0f));
			startPos += constraintPosition;
		} else {
			//spawn them in a square next to eachother that is sqrt(spawncount) by sqrt(spawncount), not random
			int width = static_cast<int>(sqrt(spawnCount));
			int height = static_cast<int>(sqrt(spawnCount));
			startPos = float2((i % width) * particleSize * 2.0f, (i / height) * particleSize * 2.0f);
			startPos += float2(particleSize) + float2(particleSize) * random;
		}
		Particle p;
		p.position = startPos;
		p.previousPosition = startPos;
		p.acceleration = float2(0, 0);
		p.color = rand() % 0xffffff;
		particles.push_back(p);
	}
}

void Tmpl8::Verlet::ResolveCollisions() {
#if 0
	const int particleCount = particles.size();
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < particleCount; i++) {
		float2& posA = particles[i].position;
		for (int j = i + 1; j < particleCount; j++) {
			float2& posB = particles[j].position;
			const float2 axis = posA - posB;
			const float distance = length(axis);
			const float size = particleSize + particleSize;
			if (distance < size) {
				const float2 normal = axis / distance;
				const float overlap = size - distance;
				float2 result = normal * overlap * 0.5f;
				posA += result;
				posB -= result;
			}
		}
	}
#else
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < gridDimensions.x; i++) {
		for (int j = 0; j < gridDimensions.y; j++) {
			Cell& cell = grid[i][j];

			if (cell.particles.empty()) {
				continue;
			}

			//check the 8 surrounding cells
			for (int x = -1; x <= 1; x++) {
				for (int y = -1; y <= 1; y++) {
					if (i + x < 0 || i + x >= gridDimensions.x || j + y < 0 || j + y >= gridDimensions.y) {
						continue;
					}
					Cell& otherCell = grid[i + x][j + y];
					CheckCollisions(cell, otherCell);
				}
			}
		}
	}
#endif
}

void Tmpl8::Verlet::UpdateGrid() {
	for (int i = 0; i < gridDimensions.x; i++) {
		for (int j = 0; j < gridDimensions.y; j++) {
			grid[i][j].particles.clear();
		}
	}

	for (int i = 0; i < particles.size(); i++) {
		const Particle& p = particles[i];
		int x = static_cast<int>(p.position.x / particleSize / 2.0f);
		int y = static_cast<int>(p.position.y / particleSize / 2.0f);
		x = std::max(0, std::min(x, gridDimensions.x - 1));
		y = std::max(0, std::min(y, gridDimensions.y - 1));

		std::vector<int>& cell = grid[x][y].particles;
		if (cell.size() < 10) {
			cell.push_back(i);
		} else {
			//delete the particle
			particles.erase(particles.begin() + i);
		}
	}
}

void Tmpl8::Verlet::CheckCollisions(Cell& cellA, Cell& cellB) {
	for (const int indexA : cellA.particles) {
		Particle& pA = particles[indexA];
		for (const int indexB : cellB.particles) {
			Particle& pB = particles[indexB];
			if (pA != pB) {
				SolveCollision(pA, pB);
			}
		}
	}
}

void Tmpl8::Verlet::SpawnParticle(const float2& position, const float2& velocity, const uint color) {
	Particle p;
	p.position = position;
	p.previousPosition = position;
	p.acceleration = float2(0, 0);
	p.color = color;
	particles.push_back(p);
}


void Verlet::PerformanceReport(Timer& t) {
	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if (alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000.0f / avg, rps = (SCRWIDTH * SCRHEIGHT) / avg;
	printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps / 1000);
}