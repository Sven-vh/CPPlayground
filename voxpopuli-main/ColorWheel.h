#pragma once

struct Pixel {
	uint color;
	float distance;
	float offset;
};

namespace Tmpl8 {
	class ColorWheel : public TheApp {
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

	private:
		int pixelCount = 100;
		float turnSpeed = 0.0005f;
		float distanceMultiplier = 100.0f;
		std::vector<Pixel> pixels;

		void InitPixels();
	};
} // namespace Tmpl8