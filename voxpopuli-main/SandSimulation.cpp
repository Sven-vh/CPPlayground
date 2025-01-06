#include "precomp.h"

// -----------------------------------------------------------
// Initialize the renderer
// -----------------------------------------------------------
void SandSimulation::Init() {
	world.Resize(SCRWIDTH, SCRHEIGHT);
	newWorld.Resize(SCRWIDTH, SCRHEIGHT);
	accumulator = 0;
	mouseDown = false;
}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void SandSimulation::Tick(float deltaTime) {
	deltaTime /= 1000.0f;
	Timer t;

	if (mouseDown) {

	}

	accumulator += deltaTime;
	float timeBetweenUpdates = 1.0f / tps;
	if (accumulator >= timeBetweenUpdates) {
		UpdateWorld();
		ResetUpdates();
		accumulator -= timeBetweenUpdates;
	}

	DrawWorld();

	PerformanceReport(t);
}

// -----------------------------------------------------------
// Update user interface (imgui)
// -----------------------------------------------------------
void SandSimulation::UI() {
	ImGui::Begin("Settings");
	//slider for the world size, but make it so it remains square so it needs to keep the ratio of the screen
	static float worldSize = 1.0f; //between 0 and 1
	if (ImGui::SliderFloat("World Size", &worldSize, 0.01f, 1.0f)) {
		int newWidth = SCRWIDTH * worldSize;
		int newHeight = SCRHEIGHT * worldSize;
		world.Resize(newWidth, newHeight);
		newWorld.Resize(newWidth, newHeight);
		//draw a particle in the middle of the screen
		world.Set(newWidth / 2, newHeight / 2, Particle(Sand_ID, 0xFFFFFF));
	}

	//clear button
	if (ImGui::Button("Clear")) {
		world.Resize(world.Width, world.Height);
	}

	//tps
	ImGui::SliderInt("TPS", &tps, 1, 60);

	//slider for the draw size
	ImGui::SliderInt("Draw Size", &drawSize, 1, 100);
	//slider for the probability
	ImGui::SliderFloat("Probability", &probability, 0.0f, 1.0f);
	ImGui::End();
}

// -----------------------------------------------------------
// User wants to close down
// -----------------------------------------------------------
void SandSimulation::Shutdown() {

}

void SandSimulation::MouseUp(int button) {
	if (button == 0) {
		mouseDown = false;
	}
}

void SandSimulation::MouseDown(int button) {
	if (button == 0 && !ImGui::GetIO().WantCaptureMouse) {
		mouseDown = true;

		int x = mousePos.x / (SCRWIDTH / world.Width);
		int y = mousePos.y / (SCRHEIGHT / world.Height);
		x -= drawSize / 2;
		y -= drawSize / 2;
		x = clamp(x, 0, world.Width - drawSize);
		y = clamp(y, 0, world.Height - drawSize);

		Square(x, y, drawSize, probability, Particle(Sand_ID, 0xFFFFFF));
	}
}


void SandSimulation::UpdateWorld() {
	const int worldWidth = world.Width;
	const int worldHeight = world.Height;

	newWorld.Clear();
	for (int y = worldHeight - 1; y >= 0; y--) {
		for (int x = 0; x < worldWidth; x++) {
			Particle p = world.GetCopy(x, y);
			material_id id = p.ID;

			switch (id) {
			case Empty_ID:
				break;
			case Sand_ID:
				UpdateSand(x, y);
				break;
			default:
				break;
			}
		}
	}
	world.Copy(newWorld);
}

void SandSimulation::DrawWorld() {
	const int worldWidth = world.Width;
	const int worldHeight = world.Height;

	const int boxWidth = SCRWIDTH / worldWidth;
	const int boxHeight = SCRHEIGHT / worldHeight;

	screen->Clear(0);

	for (int y = 0; y < worldHeight; y++) {
		for (int x = 0; x < worldWidth; x++) {
			Particle p = world.GetCopy(x, y);
			material_id id = p.ID;

			switch (id) {
			case Empty_ID:
				break;
			case Sand_ID:
				screen->BoxFilled(x * boxWidth, y * boxHeight, (x + 1) * boxWidth, (y + 1) * boxHeight, p.Color);
				break;
			default:
				break;
			}
		}
	}
}

void SandSimulation::ResetUpdates() {
	for (int y = 0; y < world.Height; y++) {
		for (int x = 0; x < world.Width; x++) {
			world.GetRef(x, y).HasUpdated = false;
		}
	}
}

void SandSimulation::Square(const int x, const int y, const int size, const float probability, const Particle p) {
	for (int i = x; i < x + size; i++) {
		for (int j = y; j < y + size; j++) {
			if (RandomFloat() < probability) {
				world.Set(i, j, p);
			}
		}
	}
}

void SandSimulation::UpdateSand(const int x, const int y) {
	Particle& p = world.GetCopy(x, y);
	if (p.HasUpdated) return;

	Particle& below = world.GetRef(x, y + 1);
	Particle& belowLeft = world.GetRef(x - 1, y + 1);
	Particle& belowRight = world.GetRef(x + 1, y + 1);

	if (below.ID == Empty_ID) {
		newWorld.Set(x, y + 1, p);
		world.Set(x, y, Particle(Empty_ID));
	} else if (belowLeft.ID == Empty_ID && belowRight.ID == Empty_ID) {
		if (RandomFloat() < 0.5f) {
			newWorld.Set(x - 1, y + 1, p);
			world.Set(x, y, Particle(Empty_ID));
		} else {
			newWorld.Set(x + 1, y + 1, p);
			world.Set(x, y, Particle(Empty_ID));
		}
	} else if (belowLeft.ID == Empty_ID) {
		newWorld.Set(x - 1, y + 1, p);
		world.Set(x, y, Particle(Empty_ID));
	} else if (belowRight.ID == Empty_ID) {
		newWorld.Set(x + 1, y + 1, p);
		world.Set(x, y, Particle(Empty_ID));
	} else {
		newWorld.Set(x, y, p);
	}
}

void SandSimulation::PerformanceReport(Timer& t) {
	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if (alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000.0f / avg, rps = (SCRWIDTH * SCRHEIGHT) / avg;
	printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps / 1000);
}

















ParticleWorld::ParticleWorld() {
	//initialize bedrock
	Bedrock = Particle(Bedrock_ID);
}

ParticleWorld::ParticleWorld(int width, int height) {
	size_t particleSize = sizeof(Particle);
	Buffer = (Particle*)MALLOC64(width * height * particleSize);
	Width = width;
	Height = height;
	Clear();
}

ParticleWorld::~ParticleWorld() {
	FREE64(Buffer);
}

void ParticleWorld::Resize(const int width, const int height) {
	FREE64(Buffer);
	size_t particleSize = sizeof(Particle);
	Buffer = (Particle*)MALLOC64(width * height * particleSize);
	Width = width;
	Height = height;
	Clear();
}

void ParticleWorld::Clear() {
	size_t particleSize = sizeof(Particle);
	memset(Buffer, 0, Width * Height * particleSize);
}

void ParticleWorld::Copy(const ParticleWorld& other) {
	if (Width != other.Width || Height != other.Height) {
		Resize(other.Width, other.Height);
	}
	memcpy(Buffer, other.Buffer, Width * Height * sizeof(Particle));
}

inline void ParticleWorld::Set(const int x, const int y, const Particle p) {
	if (!InBounds(x, y)) return;
	Buffer[ToIndex(x, y)] = p;
}

inline Particle ParticleWorld::GetCopy(const int x, const int y) const {
	if (!InBounds(x, y)) return Bedrock;
	return Buffer[ToIndex(x, y)];
}

inline Particle& ParticleWorld::GetRef(const int x, const int y) {
	if (!InBounds(x, y)) return Bedrock;
	return Buffer[ToIndex(x, y)];
}

inline bool ParticleWorld::InBounds(const int x, const int y) const {
	if (x < 0 || x >= Width || y < 0 || y >= Height) {
		return false;
	}
	return true;
}

inline int ParticleWorld::ToIndex(const int x, const int y) const {
	return y * Width + x;
}

inline int2 ParticleWorld::ToCoord(const int index) const {
	return int2(index % Width, index / Width);
}
