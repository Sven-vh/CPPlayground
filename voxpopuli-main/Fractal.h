#pragma once

class Circles : public TheApp {
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
	int minRadius = 100;
	int maxRadius = 500;
	float stepSize = 10.0f;

	float lineWidth = 1.0f;

	float waveFrequency = 12;
	float waveAmplitude = 12;
	float waveSpeed = 4;

	float sinePower = 3.0f;
	float animationSpeed = 0.5f;

	float maxDistance = 500.0f;

	float time = 0;
};

