#pragma once

struct Circle {
	float2 pos; // 8 bytes
	float2 velocity; // 8 bytes
};

namespace Tmpl8 {

	class Renderer : public TheApp {
	public:
		// game flow methods
		void Init();
		void Tick(float deltaTime);
		void PerformanceReport(Timer& t);
		void UI();
		void Shutdown();
		// input handling
		void MouseUp(int button) { /* implement if you want to detect mouse button presses */ }
		void MouseDown(int button) { /* implement if you want to detect mouse button presses */ }
		void MouseMove(int x, int y) { mousePos.x = x, mousePos.y = y; }
		void MouseWheel(float y) { /* implement if you want to handle the mouse wheel */ }
		void KeyUp(int key) { /* implement if you want to handle keys */ }
		void KeyDown(int key) { /* implement if you want to handle keys */ }
		// data members
		int2 mousePos;

		// game specific
	private:
		void UpdateParticle(const float deltaTime, Circle& particle) const;
		void ResolveCollision(Circle& particle);

		vector<Circle> particles;

		float gravity = 0.001f;
		float particleSize = 10.0f;
		float collisionDamping = 0.7f;
		int particleCount = 100;

		int collisionIterations = 1;
	};

} // namespace Tmpl8