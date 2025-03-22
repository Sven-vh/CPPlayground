#pragma once

struct DrawingCircle {
	double2 pos;
	float radius;
	uint color;

	bool operator==(const DrawingCircle& other) const {
		return pos == other.pos && radius == other.radius && color == other.color;
	}
};

struct DrawingCamera2D {
public:
	double2 Position;
	long double Zoom = 1.0f;
};

class Drawing : public TheApp {
public:
	// game flow methods
	void Init();
	void Tick(float deltaTime);
	void PerformanceReport(Timer& t);
	void UI();
	void Shutdown();
	// input handling
	void MouseUp(int button);
	void MouseDown(int button);
	void MouseMove(int x, int y);
	void MouseWheel(float y);
	void KeyUp(int key);
	void KeyDown(int key);
	// data members
	int2 mousePos;
	bool mouseDown = false;
	bool moveCam = false;
private:
	std::vector<DrawingCircle > circles;

	Surface* image;

	DrawingCamera2D cam;
	DrawingCamera2D savedCam;

	bool zoomIn = false;
	float zoomSpeed = 10;

	void UpdateCircles(bool distanceCheck = true);
	void DrawCircles();

	DrawingCircle CreateCircle(const double2& pos, long double radius);

	static inline bool IsInBounds(const double2 pos, const float size) {
		return pos.x >= -size && pos.x < SCRWIDTH + size && pos.y >= -size && pos.y < SCRHEIGHT + size;
	}
};

