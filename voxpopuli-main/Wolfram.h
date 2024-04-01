#pragma once

namespace Tmpl8 {
	class Wolfram : public TheApp {
	public:
		// game flow methods
		void Init();
		void Tick(float deltaTime);
		void Step();
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
		float totalTime = 0;
		float tps = 100;
		bool interpolate = false;
		uint8_t ruleNumber;
	};
} // namespace Tmpl8