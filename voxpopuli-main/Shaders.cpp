#include "precomp.h"
#include <complex>

// -----------------------------------------------------------
// Initialize the renderer
// -----------------------------------------------------------
void Shaders::Init() {

}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void Shaders::Tick(float deltaTime) {
	Timer t;

#pragma omp parallel for
	for (int y = 0; y < SCRHEIGHT; y++) {
		for (int x = 0; x < SCRWIDTH; x++) {
			// Map the pixel position to a complex number (c)
			double x0 = (x - SCRWIDTH / 2.0) * 4.0 / SCRWIDTH;
			double y0 = (y - SCRHEIGHT / 2.0) * 4.0 / SCRHEIGHT;
			std::complex<double> c(x0, y0);
			std::complex<double> z = 0;

			int iteration = 0;
			while (abs(z) < 2.0 && iteration < max_iter) {
				z = z * z + c;
				iteration++;
			}

			if (iteration == max_iter) {
				screen->Plot(x, y, 0xFFFFFF);
			} else {
				screen->Plot(x, y, 0);
			}
		}
	}

	PerformanceReport(t);
}

// -----------------------------------------------------------
// Update user interface (imgui)
// -----------------------------------------------------------
void Shaders::UI() {
	ImGui::Begin("Settings");
	ImGui::SliderInt("Max Iterations", &max_iter, 1, 1000);

	ImGui::End();
}

// -----------------------------------------------------------
// User wants to close down
// -----------------------------------------------------------
void Shaders::Shutdown() {

}

void Shaders::PerformanceReport(Timer& t) {
	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if (alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000.0f / avg, rps = (SCRWIDTH * SCRHEIGHT) / avg;
	printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps / 1000);
}