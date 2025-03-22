#pragma once

namespace Tmpl8 {
	struct VerletParticle {
		float2 position;
		float2 previousPosition;
		float2 acceleration;
		uint color;
		uint dummy;

		bool operator==(const VerletParticle& other) const {
			return this == &other;
		}

		bool operator!=(const VerletParticle& other) const {
			return this != &other;
		}
	};
}

struct Cell {
	std::vector<int> particles;
};

namespace Tmpl8 {
	class Verlet : public TheApp {
	public:
		// game flow methods
		void Init();
		void ResizeGrid();
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
		int mouseDown = -1;
	private:

		bool circleConstraint = false;
		bool paused = false;
		bool drawGrid = false;

		float tickRate = 60.0f;
		float totalTime = 0.0f;

		float drawParticle = 0.0f;
		float drawGridTime = 0.0f;

		float updateGrid = 0.0f;
		float applyGravity = 0.0f;
		float applyConstraints = 0.0f;
		float resolveCollisions = 0.0f;
		float updatePositions = 0.0f;

		int subSteps = 8;

		int spawnCount = 2000;
		float particleSize = 5.0f;
		float drawingSizeMultiplier = 1.1f;

		std::vector<VerletParticle> particles;

		float2 gravity = float2(0, 0.001f);
		float mouseForce = 1.0f;
		float maxVelocity = 2.0f;
		float maxColorVelocity = 1.0f;

		float2 constraintPosition = float2(SCRWIDTH / 2, SCRHEIGHT / 2);
		float constraintRadius = 300.0f;

		int2 gridDimensions;
		std::vector<std::vector<Cell>> grid;

		void Update(const float deltaTime);
		void UpdatePositions(const float deltaTime);
		void ApplyConstraints();
		void ResolveCollisions();
		void UpdateGrid();

		void CheckCollisions(Cell& cellA, Cell& cellB);
		void SolveCollision(VerletParticle& a, VerletParticle& b) const;
		void DrawParticles();
		void DrawGrids();

		void ClearParticles();
		void SpawnParticles();
		void SpawnParticle(const float2& position, const float2& velocity, const uint color);
	};
} // namespace Tmpl8