#include "precomp.h"

// -----------------------------------------------------------
// Initialize the renderer
// -----------------------------------------------------------
void Lines::Init() {

}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void Lines::Tick(float deltaTime) {
	Timer t;

	screen->Clear(0);


	//fill the points vector with points on the edges of the screen
	for (int i = 0; i < SCRWIDTH / factor; i++) {
		int2 a = int2(i * factor, 0);
		int2 b = int2(i * factor, SCRHEIGHT);
		screen->Line(a.x, a.y, mousePos.x, mousePos.y, 0xffffff);
		screen->Line(b.x, b.y, mousePos.x, mousePos.y, 0xffffff);
	}
	for (int i = 0; i < SCRHEIGHT / factor; i++) {
		int2 a = int2(0, i * factor);
		int2 b = int2(SCRWIDTH, i * factor);
		screen->Line(a.x, a.y, mousePos.x, mousePos.y, 0xffffff);
		screen->Line(b.x, b.y, mousePos.x, mousePos.y, 0xffffff);
	}

	PerformanceReport(t);
}

// -----------------------------------------------------------
// Update user interface (imgui)
// -----------------------------------------------------------
void Lines::UI() {
	//Imgui for all the settings with dragInt and dragFloat
	ImGui::Begin("Settings");
	ImGui::DragFloat("Factor", &factor, 0.1f);
}

// -----------------------------------------------------------
// User wants to close down
// -----------------------------------------------------------
void Lines::Shutdown() {

}

void Lines::PerformanceReport(Timer& t) {
	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if (alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000.0f / avg, rps = (SCRWIDTH * SCRHEIGHT) / avg;
	printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps / 1000);
}