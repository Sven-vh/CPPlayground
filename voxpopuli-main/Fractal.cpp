#include "precomp.h"

// -----------------------------------------------------------
// Initialize the renderer
// -----------------------------------------------------------
void Circles::Init() {

}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void Circles::Tick(float deltaTime) {
	deltaTime /= 1000.0f;
	Timer t;

	time += deltaTime;

	screen->Clear(0);

	constexpr int2 center = int2(SCRWIDTH / 2, SCRHEIGHT / 2);

	float currentFrequency = pow(abs(sin(time * animationSpeed)), sinePower) * waveFrequency;

#pragma omp parallel for
	for (int y = 0; y < SCRHEIGHT; y++) {
		for (int x = 0; x < SCRWIDTH; x++) {
			const int2 pos = int2(x, y);
			const float distance = length(center - pos);
			const float angle = atan2(float(pos.y - center.y), float(pos.x - center.x));

			const float waveOffset = waveAmplitude * sin(currentFrequency * angle + time * waveSpeed);

			for (float radius = minRadius; radius < maxRadius; radius += stepSize) {

				// Add wave effect to the radius
				const float wavyRadius = radius + waveOffset;

				if (abs(distance - wavyRadius) < lineWidth) {
					float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);
					//gradient based from the distance to the center
					const float t = clamp(wavyRadius / maxDistance, 0.0f, 1.0f);
					//linear
					//color = lerp(color, float4(0.0f), t);
					//easeOutCubic
					//color = lerp(color, float4(0.0f), 1.0f - pow(1.0f - t, 3.0f));
					//easeOutQuint
					color = lerp(color, float4(0.0f), 1.0f - pow(1.0f - t, 5.0f));
					screen->Plot(x, y, RGBF32_to_RGB8(&color));
				}
			}
		}
	}

	PerformanceReport(t);
}

// -----------------------------------------------------------
// Update user interface (imgui)
// -----------------------------------------------------------
void Circles::UI() {
	ImGui::Begin("Settings");
	ImGui::SliderInt("Min Radius", &minRadius, 1, 500);
	ImGui::SliderInt("Max Radius", &maxRadius, 1, 500);
	ImGui::SliderFloat("Step Size", &stepSize, 1, 100);
	ImGui::SliderFloat("Wave Amplitude", &waveAmplitude, 0, 20);
	ImGui::SliderFloat("Wave Frequency", &waveFrequency, 0, 20);
	ImGui::SliderFloat("Wave Speed", &waveSpeed, 0, 20);
	ImGui::SliderFloat("Line Width", &lineWidth, 0, 20);
	ImGui::SliderFloat("Max Distance", &maxDistance, 0, 1000);
	ImGui::SliderFloat("Animation Speed", &animationSpeed, 0, 2.0f);
	ImGui::SliderFloat("Sine Power", &sinePower, 0, 10.0f);
	ImGui::End();
}

// -----------------------------------------------------------
// User wants to close down
// -----------------------------------------------------------
void Circles::Shutdown() {

}

void Circles::PerformanceReport(Timer& t) {
	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if (alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000.0f / avg, rps = (SCRWIDTH * SCRHEIGHT) / avg;
	printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps / 1000);
}