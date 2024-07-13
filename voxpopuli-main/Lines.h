#pragma once

struct line {
	float2 from, to;

	float2 GetRandomPointOnLine() {
		return float2(from.x + (to.x - from.x) * RandomFloat(), from.y + (to.y - from.y) * RandomFloat());
	}
};

class Lines : public TheApp {
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
	std::vector<line> points;
	int lineCount = 10;

	void GenerateLines();
};

