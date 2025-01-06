#include "precomp.h"

// -----------------------------------------------------------
// Initialize the renderer
// -----------------------------------------------------------
void ColorCircle::Init() {
	screen->Clear(0);
}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void ColorCircle::Tick(float deltaTime) {
	Timer t;

	const float newIndex = index + speed * deltaTime;
	constexpr int size = SCRWIDTH * SCRHEIGHT;

	for (float i = index; i < newIndex; ) {
		int currentIndex = static_cast<int>(i) % size; // Wrap index using modulo
		screen->pixels[currentIndex] = color;
		color++;
		i++;
	}

	index = fmod(newIndex, static_cast<float>(size)); // Wrap index after loop

	PerformanceReport(t);
}

// -----------------------------------------------------------
// Update user interface (imgui)
// -----------------------------------------------------------
void ColorCircle::UI() {
	ImGui::Begin("Settings");
	ImGui::DragFloat("Index", &index, 0.1f, 0.1f, 100.0f);
	ImGui::DragFloat("Speed", &speed, 0.1f, 0.1f, 100.0f);

	if (ImGui::Button("Clear")) {
		screen->Clear(0);
	}

	ImGui::End();
}

// -----------------------------------------------------------
// User wants to close down
// -----------------------------------------------------------
void ColorCircle::Shutdown() {

}

void ColorCircle::PerformanceReport(Timer& t) {
	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if (alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000.0f / avg, rps = (SCRWIDTH * SCRHEIGHT) / avg;
	printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps / 1000);
}