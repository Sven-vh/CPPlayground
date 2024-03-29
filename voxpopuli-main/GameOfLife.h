#pragma once

class GameOfLife : public TheApp {
public:
	// game flow methods
	void Init();
	void Tick(float deltaTime);
	void HandleInput();
	void RenderScreen();
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
#if HDR
	FLoatSurface* oldScreen;
#else
	Surface* oldScreen;
#endif
	int drawSize = 10;
	bool paused = false;

	float fps = 20;
	float time = 0;

	void RandomizeScreen();
};

