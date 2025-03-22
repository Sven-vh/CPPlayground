#include "precomp.h"

#if HDR
#define ON float4(0.4f, 0.6f, 0.8f, 1.0f)
#define OFF float4(0.75f, 0.75f, 0.75f, 1.0f)
#else
#define ON 0xFFFFFFFF
#define OFF 0
#endif
// -----------------------------------------------------------
// Initialize the renderer
// -----------------------------------------------------------
void GameOfLife::Init() {
#if HDR
	screen = new FLoatSurface(SCRWIDTH, SCRHEIGHT);
#else
	oldScreen = new Surface(SCRWIDTH, SCRHEIGHT);
#endif
	//randomize the screen with on and off cells
	RandomizeScreen();
}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void GameOfLife::Tick(float deltaTime) {
	Timer t;
	//memcpy(oldScreen, screen, sizeof(Surface));
	screen->CopyTo(oldScreen, 0, 0);

	if (!paused) {
		const float msPerFrame = 1000.0f / fps;
		if (time > msPerFrame) {
			RenderScreen();
			time = 0;
		}
	}

	HandleInput();

	time += deltaTime;
	PerformanceReport(t);
}

void GameOfLife::HandleInput() {
	if (mouseDown) {
		//check if Imgui wants to capture the mouse
		ImGuiIO& io = ImGui::GetIO();
		if (!io.WantCaptureMouse) {
			//draw a circle at the mouse position
			screen->Circle(mousePos.x, mousePos.y, drawSize, ON);
		}
	}
}

void GameOfLife::RenderScreen() {
#pragma omp parallel for schedule(dynamic)
	for (int y = 0; y < SCRHEIGHT; y++) {
		for (int x = 0; x < SCRWIDTH; x++) {
			int count = 0;
			for (int i = -1; i <= 1; i++) {
				for (int j = -1; j <= 1; j++) {
					if (i == 0 && j == 0) continue;
					if (oldScreen->GetPixel(x + i, y + j) == ON) {
						count++;
					}
				}
			}
			if (oldScreen->GetPixel(x, y) == ON) {
				if (count < 2 || count > 3) {
					screen->Plot(x, y, OFF);
				}
			} else {
				if (count == 3) {
					screen->Plot(x, y, ON);
				}
			}
		}
	}
}

// -----------------------------------------------------------
// Update user interface (imgui)
// -----------------------------------------------------------
void GameOfLife::UI() {
	//imgui window for the settings
	ImGui::Begin("Settings");
	//button to clear the screen
	if (ImGui::Button("Clear")) {
		screen->Clear(OFF);
	}

	//button to randomize the screen
	if (ImGui::Button("Randomize")) {
		RandomizeScreen();
	}

	//Drag slider for the fps
	ImGui::DragFloat("FPS", &fps, 1, 1, 1000);

	//toggle to pause the game
	ImGui::Checkbox("Paused", &paused);

	//Drag slider for the draw size
	ImGui::DragInt("Draw Size", &drawSize, 1, 1, 100);
	ImGui::End();
}

// -----------------------------------------------------------
// User wants to close down
// -----------------------------------------------------------
void GameOfLife::Shutdown() {

}

void GameOfLife::RandomizeScreen() {
	for (int i = 0; i < SCRWIDTH; i++) {
		for (int j = 0; j < SCRHEIGHT; j++) {
			float r = RandomFloat();
			screen->Plot(i, j, RandomFloat() <= 0.2f ? ON : OFF);
		}
	}
}

void GameOfLife::PerformanceReport(Timer& t) {
	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if (alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000.0f / avg, rps = (SCRWIDTH * SCRHEIGHT) / avg;
	printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps / 1000);
}