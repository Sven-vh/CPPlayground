#include "precomp.h"

// -----------------------------------------------------------
// Initialize the renderer
// -----------------------------------------------------------
void Maze::Init() {
	Reset(int2(0));
}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void Maze::Tick(float deltaTime) {
	Timer t;

	if (instant) {
		while (!path.empty()) {
			Step();
		}
	} else {
		for (int i = 0; i < stepsPerFrame; i++) {
			if (!path.empty()) {
				Step();
			}
		}
	}

	if (path.empty() && keepLongestPath && !longestPath.empty()) {
		//draw longest path

		int2 prev = longestPath.top();
		while (!longestPath.empty()) {
			screen->Plot(prev.x, prev.y, 0x00ff00);
			longestPath.pop();

			if (!longestPath.empty()) {
				int2 next = longestPath.top();
				int2 between = make_int2(make_float2(prev + next) / 2.0f);
				screen->Plot(between.x, between.y, 0x00ff00);
				prev = next;
			}
		}

		//place the finish
		screen->Plot(prev.x, prev.y, 0xff0000);
	}



	PerformanceReport(t);
}

// -----------------------------------------------------------
// Update user interface (imgui)
// -----------------------------------------------------------
void Maze::UI() {

	//reset
	if (ImGui::Button("Reset")) {
		Reset(int2(0));
	}

	ImGui::DragInt("Steps per frame", &stepsPerFrame, 1);
	ImGui::Checkbox("Keep longest path", &keepLongestPath);
	ImGui::Checkbox("Instant", &instant);
}

// -----------------------------------------------------------
// User wants to close down
// -----------------------------------------------------------
void Maze::Shutdown() {

}

void Tmpl8::Maze::MouseDown(int button) {
	if (button == 0 && !ImGui::GetIO().WantCaptureMouse) {
		Reset(mousePos);
	}
}

void Tmpl8::Maze::Reset(const int2 startPos) {
	screen->Clear(WALL);
	position = startPos;
	//clear path
	while (!path.empty()) {
		path.pop();
	}
	path.push(position);
	screen->Plot(position.x, position.y, EMPTY);
}

void Tmpl8::Maze::Step() {
	const int2 directions[4] = { int2(0, -1), int2(1, 0), int2(0, 1), int2(-1, 0) };

	bool found = false;
	int randomIndex = RandomFloat() * 4;

	for (int i = 0; i < 4; i++) {
		randomIndex = (randomIndex + 1) % 4;
		const int2 randomDirection = directions[randomIndex];
		const int2 newPosition = position + randomDirection * 2;

		if (IsInBounds(newPosition)) {
			if (IsWall(newPosition)) {
				screen->Plot(newPosition.x, newPosition.y, EMPTY);
				screen->Plot(position.x + randomDirection.x, position.y + randomDirection.y, EMPTY);
				position = newPosition;
				path.push(position);

				if (keepLongestPath) {
					if (path.size() > longestPath.size()) {
						memcpy(&longestPath, &path, sizeof(path));
					}
				}

				found = true;
				break;
			}
		}
	}

	if (!found) {
		path.pop();
		if (!path.empty()) {
			position = path.top();
			Step();
		}
	}
}

void Maze::PerformanceReport(Timer& t) {
	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if (alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000.0f / avg, rps = (SCRWIDTH * SCRHEIGHT) / avg;
	printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps / 1000);
}