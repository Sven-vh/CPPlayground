#include "precomp.h"

// -----------------------------------------------------------
// Initialize the renderer
// -----------------------------------------------------------
void SineWave::Init() {

}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void SineWave::Tick(float deltaTime) {
	deltaTime /= 1000.0f;

	Timer t;
	screen->Clear(0);

	DrawWave();

	PerformanceReport(t);
	time += deltaTime;
}

// -----------------------------------------------------------
// Update user interface (imgui)
// -----------------------------------------------------------
void SineWave::UI() {
	//Imgui for all the settings with dragInt and dragFloat
	ImGui::Begin("Settings");
	ImGui::DragInt("Wave Size", &waveSize);
	ImGui::DragFloat("Rect Size", &rectSize);
	ImGui::DragFloat("Margin", &margin);
	ImGui::DragFloat("Amplitude", &amplitude);
	ImGui::DragFloat("Frequency", &frequency, 0.001f);
	ImGui::DragFloat("Period", &period, 0.001f);
	ImGui::End();
}

// -----------------------------------------------------------
// User wants to close down
// -----------------------------------------------------------
void SineWave::Shutdown() {

}

void SineWave::DrawWave() {
	const int2 center = int2(SCRWIDTH / 2, SCRHEIGHT / 2 - waveSize * rectSize);
	int i = 0;
	//#pragma omp parallel for schedule(dynamic)
	for (int y = 0; y < waveSize; y++) {

		if (y <= waveSize / 2) {
			for (int x = -y; x <= y; x++) {
				const float height = GetHeight(int2(x, y));
				const int2 pos = int2(x * rectSize * margin, y * 2 * rectSize + height);
				DrawRectangle(pos + center);
			}

		} else {
			for (int x = -y + i * 2; x <= y - i * 2; x++) {
				const float height = GetHeight(int2(x, y));
				const int2 pos = int2(x * rectSize * margin, y * 2 * rectSize + height);
				DrawRectangle(pos + center);
			}
			i++;
		}

	}
}

void Tmpl8::SineWave::DrawRectangle(const int2& center) {

	// Define the top-left corner
	const int2 posA = int2(center.x - rectSize / 2.0f, center.y - rectSize / 2.0f);
	const int2 posB = int2(center.x + rectSize / 2.0f, center.y + rectSize / 2.0f);

	static const float rotation = PI / 4.0f;

	screen->Rect(posA.x, posA.y, posB.x, posB.y, rotation, 0xff0000);
}

float Tmpl8::SineWave::GetHeight(const int2& pos) const {
	float x, y;
	if (mouseDown) {
		y = waveSize / 2.0f - pos.y + ((mousePos.y - SCRHEIGHT / 2.0f) / SCRHEIGHT * waveSize);
		x = pos.x - ((mousePos.x - SCRWIDTH / 2.0f) / SCRWIDTH * waveSize);
	} else {
		y = waveSize / 2.0f - pos.y;
		x = pos.x;
	}

	x /= period;
	y /= period;

	float height = amplitude * sin(frequency * (sqrt(x * x + y * y) + time));

	return height;
}

void SineWave::PerformanceReport(Timer& t) {
	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if (alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000.0f / avg, rps = (SCRWIDTH * SCRHEIGHT) / avg;
	printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps / 1000);
}