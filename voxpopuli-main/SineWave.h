#pragma once

namespace Tmpl8 {
	class SineWave : public TheApp {
	public:
		// game flow methods
		void Init();
		void Tick(float deltaTime);
		void PerformanceReport(Timer& t);
		void UI();
		void Shutdown();
		// input handling
		void MouseUp(int button) { mouseDown = false; }
		void MouseDown(int button) { mouseDown = true; }
		void MouseMove(int x, int y) { mousePos.x = x, mousePos.y = y; }
		void MouseWheel(float y) { /* implement if you want to handle the mouse wheel */ }
		void KeyUp(int key) { /* implement if you want to handle keys */ }
		void KeyDown(int key) { /* implement if you want to handle keys */ }
		// data members
		int2 mousePos;
		bool mouseDown = false;
	private:

		void DrawWave();
		void DrawRectangle(const int2& center);

		float GetHeight(const int2& pos) const;

		float time;

		int waveSize = 20;
		float rectSize = 10.0f;
		float margin = 1.5f;

		float amplitude = 15.0f;
		float frequency = 10.0f;
		float period = 2.0f;
	};
} // namespace Tmpl8