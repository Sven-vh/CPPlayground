#pragma once
#include "Ant.h"
#define SIMD 1

#define LUT_SIZE 360

class Dots : public TheApp {
public:
	// game flow methods
	void Init();
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
	bool mouseDown = false;
private:
	int spawnCount = 300000;

#if SIMD
	int numAnts = 0;
	std::vector<float> xPositions;
	std::vector<float> yPositions;
	std::vector<float> xVelocities;
	std::vector<float> yVelocities;
	std::vector<float> xDesiredDirections;
	std::vector<float> yDesiredDirections;
	std::vector<UINT8> rColors;
	std::vector<UINT8> gColors;
	std::vector<UINT8> bColors;
	std::vector<__m256> randoms;
	//std::vector<float> angles;
#else
	std::vector<Ant> ants;
#endif
	uint colorLUT[LUT_SIZE];

	float4 antColor = float4(1.0f);
	//0xFF977D5E as a float4
	float4 backgroundColor = float4(0.0f);
	float evaporateSpeed = 0.1f;
	float antSize = 10.0f;

	float MaxSpeed = 7.0f;
	float SteerStrength = 2.0f;
	float WanderStrength = 0.1f;
	bool wander = true;

	bool clearScreen = true;

	float updateAntsTime = 0.0f;
	float clearScreenTime = 0.0f;
	float renderAntsTime = 0.0f;

	void SpawnAnts();
	void ClearAnts();
	void RenderAnts();
	void SubtractScreen(const float deltaTime);
	void UpdateAnts256(const float deltaTime);
	void InitColorLUT();
	//void UpdateAnts512(const float deltaTime);
};