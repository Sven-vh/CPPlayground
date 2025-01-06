#include "precomp.h"

// -----------------------------------------------------------
// Initialize the renderer
// -----------------------------------------------------------
void Drawing::Init() {
	image = new Surface("assets/red.png");

	//DrawingCircle startCircle;
	//startCircle.pos = int2(SCRWIDTH / 2, SCRHEIGHT / 2);
	//startCircle.radius = min(SCRWIDTH, SCRHEIGHT) / 2;
	//startCircle.color = 0xFFFFFF;
	//circles.push_back(startCircle);

	circles.push_back(CreateCircle(int2(SCRWIDTH / 2, SCRHEIGHT / 2), min(SCRWIDTH, SCRHEIGHT) / 2));

}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void Drawing::Tick(float deltaTime) {
	deltaTime /= 1000.0f;
	Timer t;

	if (zoomIn) {
		MouseWheel(deltaTime * zoomSpeed);
	}

	UpdateCircles();
	DrawCircles();


	PerformanceReport(t);
}

// -----------------------------------------------------------
// Update user interface (imgui)
// -----------------------------------------------------------
void Drawing::UI() {
	//window with reset buttons
	ImGui::Begin("Settings");

	//circle count
	ImGui::Text("Circle Count: %d", circles.size());

	//camera pos and zoom
	ImGui::Text("Camera Pos: %.2f, %.2f", cam.Position.x, cam.Position.y);
	ImGui::Text("Camera Zoom: %.2f", cam.Zoom);

	if (ImGui::Button("Reset")) {
		circles.clear();
		circles.push_back(CreateCircle(int2(SCRWIDTH / 2, SCRHEIGHT / 2), min(SCRWIDTH, SCRHEIGHT) / 2));
	}
	//divide button
	if (ImGui::Button("Divide")) {
		UpdateCircles(false);
	}

	if (ImGui::Button("Divide 0 index")) {
		//divide the last circle
		DrawingCircle circle = circles[circles.size() - 1];
		const long double newPos = circle.radius / 2.0f;
		const long double newRadius = circle.radius / 2.0f;

		//remove the original circle
		circles.pop_back();

		circles.push_back(CreateCircle(circle.pos + double2(newPos), newRadius));
		circles.push_back(CreateCircle(circle.pos + double2(-newPos), newRadius));
		circles.push_back(CreateCircle(circle.pos + double2(-newPos, newPos), newRadius));
		circles.push_back(CreateCircle(circle.pos + double2(newPos, -newPos), newRadius));
	}

	//zoomSpeed slider
	ImGui::SliderFloat("Zoom Speed", &zoomSpeed, 0.1f, 10.0f);
	ImGui::End();
}

// -----------------------------------------------------------
// User wants to close down
// -----------------------------------------------------------
void Drawing::Shutdown() {
	delete image;
}

void Drawing::MouseUp(int button) {
	if (button == 0) {
		mouseDown = false;
	}

	if (button == 1) {
		moveCam = false;
	}
}

void Drawing::MouseDown(int button) {
	if (button == 0 && !ImGui::GetIO().WantCaptureMouse) {
		mouseDown = true;
	}

	if (button == 1) {
		moveCam = true;
	}
}

void Drawing::MouseMove(int x, int y) {
	int2 newPos(x, y);
	if (moveCam) {
		double2 delta = mousePos - newPos;
		cam.Position -= delta / cam.Zoom;
	}
	mousePos = newPos;
}

void Drawing::MouseWheel(float y) {
	double2 mouseWorldBeforeZoom = ((double2)mousePos / cam.Zoom) - cam.Position;

	cam.Zoom *= (1.0f + y * 0.1f);
	if (cam.Zoom < 0.1f) cam.Zoom = 0.1f;
	cam.Position = ((double2)mousePos / cam.Zoom) - mouseWorldBeforeZoom;
}

void Drawing::KeyUp(int key) {
	if (key == 90) {
		zoomIn = false;
	}
}

void Drawing::KeyDown(int key) {
	if (key == 90) {
		zoomIn = true;
	}
}

void Drawing::UpdateCircles(bool distanceCheck) {

	// if distanceCheck is false keep going else check if the mouse is down
	if (distanceCheck == false) {
		mouseDown = true;
	}
	if (!mouseDown) return;
	if (distanceCheck == false) {
		mouseDown = false;
	}


	std::vector<int> toRemove;
	std::vector<DrawingCircle> circlesToAdd;

	const double2 worldMousePos = (double2(mousePos) / cam.Zoom) - cam.Position;

	for (int i = 0; i < circles.size(); i++) {
		DrawingCircle& circle = circles[i];
		//if (circle.radius <= 1.0f) continue;
		float distanceToMouse = length(worldMousePos - circle.pos);
		if (distanceToMouse < circle.radius || !distanceCheck) {
			//split the circle into 4 new circles
			const long double newPos = circle.radius / 2.0f;
			const long double newRadius = circle.radius / 2.0f;
			circlesToAdd.push_back(CreateCircle(circle.pos + double2(newPos), newRadius));
			circlesToAdd.push_back(CreateCircle(circle.pos + double2(-newPos), newRadius));
			circlesToAdd.push_back(CreateCircle(circle.pos + double2(-newPos, newPos), newRadius));
			circlesToAdd.push_back(CreateCircle(circle.pos + double2(newPos, -newPos), newRadius));
			//remove the original circle
			toRemove.push_back(&circle - &circles[0]);
		}
	}

	//remove the circles
	for (int i = toRemove.size() - 1; i >= 0; i--) {
		circles.erase(circles.begin() + toRemove[i]);
	}

	//add the new circles
	for (auto& circle : circlesToAdd) {
		circles.push_back(circle);
	}
}

void Drawing::DrawCircles() {
	screen->Clear(0);
	//for (auto& circle : circles) {
	//	screen->Circle(circle.pos.x, circle.pos.y, round(circle.radius), circle.color);
	//}

	const long double zoom = cam.Zoom;
	const double2 camPos = cam.Position;

#pragma omp parallel
	for (size_t i = 0; i < circles.size(); i++) {
		DrawingCircle tile = circles[i];
		const float newSpriteSize = tile.radius * zoom;
		if (newSpriteSize < 0.1f) continue;
		const double2 pos = (double2(tile.pos.x, tile.pos.y) + camPos) * zoom;

		if (IsInBounds(pos, newSpriteSize)) {
			screen->FastBigCircle((int)pos.x, (int)pos.y, newSpriteSize, tile.color);
		}

	}
}

DrawingCircle Drawing::CreateCircle(const double2& pos, long double radius) {
	DrawingCircle circle;
	circle.pos = pos;
	circle.radius = radius;
	//get the color of the pixel at the position of the circle
	double2 uv = pos / double2(SCRWIDTH, SCRHEIGHT);
	circle.color = image->GetPixel((float)uv.x, (float)uv.y);


	return circle;
}

void Drawing::PerformanceReport(Timer& t) {
	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if (alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000.0f / avg, rps = (SCRWIDTH * SCRHEIGHT) / avg;
	printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps / 1000);
}