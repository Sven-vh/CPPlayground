#include "precomp.h"

// -----------------------------------------------------------
// Initialize the renderer
// -----------------------------------------------------------
void CircleFill::Init() {
	screen->Clear(RGBF32_to_RGB8(&backgroundColor));
}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void CircleFill::Tick(float deltaTime) {
	Timer t;

	for (auto& circle : circles) {
		circle.radius += growSpeed * deltaTime;
	}

	//DrawCircles();

	const uint backgroundColoruint = RGBF32_to_RGB8(&this->backgroundColor);

#pragma omp parallel for
	for (int x = 0; x < SCRWIDTH; x++) {
		for (int y = 0; y < SCRHEIGHT; y++) {
			const uint currentColor = screen->GetPixel(x, y);
			if (currentColor != backgroundColoruint) continue;

			const int2 pos = int2(x, y);

			for (auto& circle : circles) {
				const int radius = (int)circle.radius;
				const int radiusSquared = radius * radius;

				const int i = pos.x - circle.pos.x;
				const int j = pos.y - circle.pos.y;

				if (i * i + j * j < radiusSquared) {
					screen->Plot(x, y, circle.color);
					break;
				}
			}

		}
	}

	PerformanceReport(t);
}

// -----------------------------------------------------------
// Update user interface (imgui)
// -----------------------------------------------------------
void CircleFill::UI() {
	ImGui::Begin("Settings");
	ImGui::DragFloat("Start Radius", &startRadius, 0.1f, 0.1f, 100.0f);
	ImGui::DragFloat("Grow Speed", &growSpeed, 0.1f, 0.1f, 100.0f);

	ImGui::ColorEdit4("Background Color", &backgroundColor.x);

	ImGui::Checkbox("Random Color", &randomColor);
	ImGui::ColorEdit4("Circle Color", &circleColor.x);

	if (ImGui::Button("Clear Circles")) {
		circles.clear();
	}

	if (ImGui::Button("Clear Screen")) {
		screen->Clear(RGBF32_to_RGB8(&backgroundColor));
	}

	ImGui::End();

	ImGui::Begin("Ref testing");

	int* myValue = nullptr;
	if (ImGui::Button("Create value")) {
		int value = 5;
		myValue = &value;
	}

	ImGui::DragInt("Value", myValue);

	ImGui::End();
}

// -----------------------------------------------------------
// User wants to close down
// -----------------------------------------------------------
void CircleFill::Shutdown() {

}

void Tmpl8::CircleFill::MouseDown(int button) {
	if (button == 0 && !ImGui::GetIO().WantCaptureMouse) {
		CreateCircle(mousePos, startRadius);
	}
}

void Tmpl8::CircleFill::DrawCircles() {
	const uint backgroundColor = RGBF32_to_RGB8(&this->backgroundColor);
	for (auto circle : circles) {
		if (!circle.enabled) continue;
		const int radius = (int)circle.radius;
		const int x = circle.pos.x;
		const int y = circle.pos.y;

		const int radiusSquared = radius * radius;

		bool isDrawn = false;

#pragma omp parallel for
		for (int i = -radius; i < radius; i++) {
			if (i + x < 0 || i + x >= SCRWIDTH) continue;

			for (int j = -radius; j < radius; j++) {
				if (j + y < 0 || j + y >= SCRHEIGHT) continue;

				if (i * i + j * j < radiusSquared) {

					const uint currentColor = screen->GetPixel(x + i, y + j);

					if (currentColor != backgroundColor) continue;

					screen->Plot(x + i, y + j, circle.color);
					isDrawn = true;
				}
			}
		}

		if (isDrawn) {
			circle.enabled = false;
		}
	}
}

void Tmpl8::CircleFill::CreateCircle(const int2& pos, float radius) {
	DrawCircle  circle;
	circle.pos = pos;
	circle.radius = radius;
	if (randomColor) {
		circle.color = RGBF32_to_RGB8(&float4(RandomFloat(), RandomFloat(), RandomFloat(), 1.0f));
	} else {
		circle.color = RGBF32_to_RGB8(&circleColor);
	}
	circles.push_back(circle);
}

void CircleFill::PerformanceReport(Timer& t) {
	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if (alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000.0f / avg, rps = (SCRWIDTH * SCRHEIGHT) / avg;
	printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps / 1000);
}