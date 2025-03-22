#pragma once
#include <stack>
#define WALL 0x000000
#define EMPTY 0xffffff

namespace Tmpl8 {
	class Maze : public TheApp {
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
		int stepsPerFrame = 1000;
		bool instant = false;
		bool keepLongestPath = true;
		int2 position;

		std::stack<int2> path;
		std::stack<int2> longestPath;

		void Reset(const int2 startPos);
		void Step();

		static inline bool IsInBounds(const int2 pos) {
			return pos.x >= 0 && pos.x < SCRWIDTH && pos.y >= 0 && pos.y < SCRHEIGHT;
		}

		inline bool IsWall(const int2 pos) {
#if HDR
			float3 pixel = screen->pixels[pos.y * SCRWIDTH + pos.x];
#else
			uint pixel = screen->pixels[pos.y * SCRWIDTH + pos.x];
#endif
			return pixel == WALL;
		}
	};
} // namespace Tmpl8