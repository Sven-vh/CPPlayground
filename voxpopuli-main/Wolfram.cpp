#include "precomp.h"

#define BOTTOM (SCRHEIGHT - 1)
#define ON 0xFFFFFFFF
#define OFF 0

// -----------------------------------------------------------
// Initialize the renderer
// -----------------------------------------------------------
void Wolfram::Init() {
	screen->Clear(OFF);
	screen->Plot(SCRWIDTH / 2, BOTTOM, ON);
	ruleNumber = 30;
}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void Wolfram::Tick(float deltaTime) {
	deltaTime /= 1000.0f; // Convert to seconds
	totalTime += deltaTime;
	Timer t;


	if (totalTime > 1.0f / tps) {
		Step();
		totalTime -= 1.0f / tps;
		if (interpolate) {
			ruleNumber += 1;
		}
	}


	PerformanceReport(t);
}

void Tmpl8::Wolfram::Step() {
	// move all pixels up
	for (int y = 0; y < SCRHEIGHT - 1; y++) {
		for (int x = 0; x < SCRWIDTH; x++) {
			const int index = x + y * SCRWIDTH;
			const int indexBelow = x + (y + 1) * SCRWIDTH;

			uint colorBelow = screen->pixels[indexBelow];
			if (colorBelow == ON) {
				screen->pixels[index] = ON;
			} else {
				screen->pixels[index] = OFF;
			}

		}
	}

	// apply rules to the bottom row
	for (int x = 0; x < SCRWIDTH; x++) {
		// Use modular arithmetic to wrap around the edges
		const bool left = screen->GetPixel((x - 1 + SCRWIDTH) % SCRWIDTH, BOTTOM - 1) == ON;
		const bool center = screen->GetPixel(x, BOTTOM - 1) == ON;
		const bool right = screen->GetPixel((x + 1) % SCRWIDTH, BOTTOM - 1) == ON;

		const int state = (left << 2) | (center << 1) | right;
		const uint color = (ruleNumber & (1 << state)) ? ON : OFF;

		// Plot the new state
		screen->Plot(x, BOTTOM, color);
	}
}

// -----------------------------------------------------------
// Update user interface (imgui)
// -----------------------------------------------------------
void Wolfram::UI() {
	ImGui::DragInt("Rule number", (int*)&ruleNumber, 1, 0, 255);
	ImGui::Text("Rule: %d", ruleNumber);
	//show checkboxes for the rule
	for (int i = 7; i >= 0; i--) {
		bool checked = (ruleNumber & (1 << i)) != 0; // Simplified condition
		std::string emptyLabel = "##" + std::to_string(i);
		if (ImGui::Checkbox(emptyLabel.c_str(), &checked)) {
			if (checked)
				ruleNumber |= (1 << i); // Set bit
			else
				ruleNumber &= ~(1 << i); // Clear bit
		}
		ImGui::SameLine();
	}

	if (ImGui::Button("Reset")) {
		screen->Clear(OFF);
		screen->Plot(SCRWIDTH / 2, BOTTOM, ON);
	}

	//step
	if (ImGui::Button("Step")) {
		Step();
	}

	ImGui::DragFloat("Ticks per second", &tps, 0.1f, 0.1f, 100.0f);
	ImGui::Checkbox("Interpolate", &interpolate);
}

// -----------------------------------------------------------
// User wants to close down
// -----------------------------------------------------------
void Wolfram::Shutdown() {

}

void Wolfram::PerformanceReport(Timer& t) {
	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if (alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000.0f / avg, rps = (SCRWIDTH * SCRHEIGHT) / avg;
	printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps / 1000);
}