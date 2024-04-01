#pragma once

namespace Tmpl8 {
	class Verlet : public TheApp {
	public:
		// game flow methods
		void Init();
		void Tick(float deltaTime);
		void PerformanceReport(Timer& t);
		void UI();
		void Shutdown();
		// input handling
		void MouseUp(int button);
		void MouseDown(int button);
		void MouseMove(int x, int y) { mousePos.x = x, mousePos.y = y; }
		void MouseWheel(float y) { /* implement if you want to handle the mouse wheel */ }
		void KeyUp(int key) { /* implement if you want to handle keys */ }
		void KeyDown(int key) { /* implement if you want to handle keys */ }
		// data members
		int2 mousePos;
		bool mouseDown = false;
	private:

		float tickRate = 60.0f;
		float totalTime = 0.0f;

		int subSteps = 8;

		int spawnCount = 100;
		float spawnSize = 5.0f;

		int particleCount;
		std::vector<float2> currentPositions;
		std::vector<float2> previousPositions;
		std::vector<float2> accelerations;
		std::vector<float> sizes;
		std::vector<uint> colors;

		float2 gravity = float2(0, 0.001f);

		float2 constraintPosition = float2(SCRWIDTH / 2, SCRHEIGHT / 2);
		float constraintRadius = 300.0f;

		void Update(const float deltaTime);
		void UpdatePositions(const float deltaTime);
		void ApplyGravity();
		void ApplyConstraints();
		void ResolveCollisions();

		void DrawParticles();

		void ClearParticles();
		void SpawnParticles();
		void SpawnParticle(const float2& position, const float2& velocity, const float size, const uint color);
	};
} // namespace Tmpl8