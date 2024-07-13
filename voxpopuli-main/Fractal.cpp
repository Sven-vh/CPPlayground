#include "precomp.h"

// -----------------------------------------------------------
// Initialize the renderer
// -----------------------------------------------------------
void Fractal::Init() {

}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void Fractal::Tick(float deltaTime) {
    Timer t;

    PerformanceReport(t);
}

// -----------------------------------------------------------
// Update user interface (imgui)
// -----------------------------------------------------------
void Fractal::UI() {

}

// -----------------------------------------------------------
// User wants to close down
// -----------------------------------------------------------
void Fractal::Shutdown() {

}

void Fractal::PerformanceReport(Timer& t) {
	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if (alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000.0f / avg, rps = (SCRWIDTH * SCRHEIGHT) / avg;
	printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps / 1000);
}