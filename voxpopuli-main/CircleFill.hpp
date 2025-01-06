#pragma once

struct DrawCircle {
	int2 pos;
	float radius;
	uint color;
	bool enabled = true;
};

namespace Tmpl8 {
	class CircleFill : public TheApp {
	public:
		// game flow methods
		void Init();
		void Tick(float deltaTime);
		void PerformanceReport(Timer& t);
		void UI();
		void Shutdown();
		// input handling
		void MouseUp(int button) { /* implement if you want to detect mouse button presses */ }
		void MouseDown(int button);
		void MouseMove(int x, int y) { mousePos.x = x, mousePos.y = y; }
		void MouseWheel(float y) { /* implement if you want to handle the mouse wheel */ }
		void KeyUp(int key) { /* implement if you want to handle keys */ }
		void KeyDown(int key) { /* implement if you want to handle keys */ }
		// data members
		int2 mousePos;

	private:
		std::vector<DrawCircle > circles;
		void DrawCircles();
		void CreateCircle(const int2& pos, float radius);

		float startRadius = 1.0f;
		float growSpeed = 0.1f;

		float4 backgroundColor = float4(0.0f);
		float4 circleColor = float4(1.0f);
		bool randomColor = false;
	};
} // namespace Tmpl8