#include "precomp.h"

// -----------------------------------------------------------
// Initialize the renderer
// -----------------------------------------------------------
void ColorWheel::Init() {
	InitPixels();
}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void ColorWheel::Tick(float deltaTime) {
	Timer t;

	screen->Clear(0);

	const int count = pixels.size();
	for (int i = 0; i < count; i++) {
		pixels[i].offset += deltaTime * turnSpeed;
	}

	for (int i = 0; i < count; i++) {
		const Pixel& pixel = pixels[i];
		const float xPos = cos(pixel.offset) * pixel.distance * distanceMultiplier;
		const float yPos = sin(pixel.offset) * pixel.distance * distanceMultiplier;
		const int xIndex = (int)(xPos + SCRWIDTH / 2);
		const int yIndex = (int)(yPos + SCRHEIGHT / 2);
		const int index = yIndex * SCRWIDTH + xIndex;
		if (index >= 0 && index < SCRWIDTH * SCRHEIGHT) {
#if HDR
			screen->pixels[index] += pixel.color;
#else
			screen->pixels[index] |= pixel.color;
#endif
		}
	}

	PerformanceReport(t);
}

// -----------------------------------------------------------
// Update user interface (imgui)
// -----------------------------------------------------------
void ColorWheel::UI() {
	//Imgui for all the settings with dragInt and dragFloat
	ImGui::Begin("Settings");
	ImGui::DragFloat("Turn Speed", &turnSpeed, 0.00001f);
	ImGui::DragFloat("Distance Multiplier", &distanceMultiplier);
	ImGui::DragInt("Pixel Count", &pixelCount);
	if (ImGui::Button("Init Pixels")) {
		InitPixels();
	}
	ImGui::End();
}

// -----------------------------------------------------------
// User wants to close down
// -----------------------------------------------------------
void ColorWheel::Shutdown() {

}

void ColorWheel::InitPixels() {
	pixels.resize(pixelCount);
	static uint colors[] = { 0xff0000, 0x00ff00, 0x0000ff };
	for (int i = 0; i < pixelCount; i++) {
		const int randomIndex = rand() % 3;
		pixels[i].color = colors[randomIndex];
		const float2 center = float2(SCRWIDTH / 2, SCRHEIGHT / 2);
		const float2 randomCircle = RandomInsideUnitCircle();
		pixels[i].distance = randomCircle.x * 0.5f + 0.5f;
		pixels[i].offset = randomCircle.y * 2.0f * PI;
		//pixels[i].offset = RandomFloat() * 2.0f * PI;
	}
}

void ColorWheel::PerformanceReport(Timer& t) {
	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if (alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000.0f / avg, rps = (SCRWIDTH * SCRHEIGHT) / avg;
	printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps / 1000);
}