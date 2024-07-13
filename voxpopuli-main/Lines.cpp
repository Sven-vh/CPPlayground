#include "precomp.h"

// -----------------------------------------------------------
// Initialize the renderer
// -----------------------------------------------------------
void Lines::Init() {
	GenerateLines();
}



// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void Lines::Tick(float deltaTime) {
	Timer t;

	screen->Clear(0);

	for (line l : points) {
		screen->Line(l.from.x, l.from.y, l.to.x, l.to.y, 0xFFFFFFFF);
	}

	PerformanceReport(t);
}

// -----------------------------------------------------------
// Update user interface (imgui)
// -----------------------------------------------------------
void Lines::UI() {
	//Imgui for all the settings with dragInt and dragFloat
	ImGui::Begin("Settings");
	ImGui::DragInt("Line count", &lineCount, 1, 1, 100);
	if (ImGui::Button("Generate")) {
		GenerateLines();
	}
}

// -----------------------------------------------------------
// User wants to close down
// -----------------------------------------------------------
void Lines::Shutdown() {

}

void Lines::GenerateLines() {
	// generate random lines
	points.clear();
	line startLine;
	startLine.from = RandomFloat() > 0.5f ? float2(0, RandomFloat() * SCRHEIGHT) : float2(SCRWIDTH, RandomFloat() * SCRHEIGHT);
	startLine.to = RandomFloat() > 0.5f ? float2(0, RandomFloat() * SCRHEIGHT) : float2(SCRWIDTH, RandomFloat() * SCRHEIGHT);
	points.push_back(startLine);

	for (int i = 0; i < lineCount-1; i++) {
		line newLine;
		newLine.from = points.back().GetRandomPointOnLine();
		//somewhere on the side of the screen
		newLine.to = RandomFloat() > 0.5f ? float2(0, RandomFloat() * SCRHEIGHT) : float2(SCRWIDTH, RandomFloat() * SCRHEIGHT);
		points.push_back(newLine);
	}
}

void Lines::PerformanceReport(Timer& t) {
	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if (alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000.0f / avg, rps = (SCRWIDTH * SCRHEIGHT) / avg;
	printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps / 1000);
}