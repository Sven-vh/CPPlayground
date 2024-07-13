#include "precomp.h"

// -----------------------------------------------------------
// Initialize the renderer
// -----------------------------------------------------------
void Hexcells::Init() {
	width = 10;
	height = 10;
	size = 10;
	InitTiles();
}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void Hexcells::Tick(float deltaTime) {
	Timer t;
	screen->Clear(backgroundColor);
	DrawTiles();


	//PerformanceReport(t);
}

// -----------------------------------------------------------
// Update user interface (imgui)
// -----------------------------------------------------------
void Hexcells::UI() {
	//imgui window to change the width and height of the grid and other settings
	ImGui::Begin("Settings");
	bool changed = false;
	changed |= ImGui::SliderInt("Width", &width, 1, 100);
	changed |= ImGui::SliderInt("Height", &height, 1, 100);
	ImGui::SliderInt("Sprite Radius", &size, 1, 100);
	ImGuiUintColorEdit("Background Color", backgroundColor);
	ImGui::SliderFloat2("Tile Offset", &tileOffset.x, 1, 100);
	ImGui::End();

	if (changed) {
		DestroyTiles();
		InitTiles();
	}
}

// -----------------------------------------------------------
// User wants to close down
// -----------------------------------------------------------
void Hexcells::Shutdown() {
	DestroyTiles();
}

void Hexcells::MouseUp(int button) {
	if (button == RIGHT_CLICK) {
		moveCam = false;
	}
}

void Hexcells::MouseDown(int button) {
	if (button == RIGHT_CLICK) {
		moveCam = true;
	}

	if (button == LEFT_CLICK) {
		for (size_t i = 0; i < tiles.size(); i++) {
			Tile* tile = tiles[i];
			if (IsInBoundsOfTile(tile, mousePos)) {
				//print position of clicked tile
				printf("Tile clicked: %d, %d\n", tiles[i]->Position.x, tiles[i]->Position.y);

				//change the color of the clicked tile and its neighbors
				tile->Color = 0xff0000;
				for (size_t j = 0; j < NEIGHBOUR_COUNT; j++) {
					if (tile->neighbors[j] != nullptr) {
						tile->neighbors[j]->Color = 0xff0000;
					}
				}
			}
		}
	}
}

void Hexcells::MouseMove(int x, int y) {
	int2 newPos(x, y);
	if (moveCam) {
		float2 delta = mousePos - newPos;
		cam.Position -= delta / cam.Zoom;
	}
	mousePos = newPos;
}

void Hexcells::MouseWheel(float y) {
	float2 mouseWorldBeforeZoom = (mousePos / cam.Zoom) - cam.Position;

	cam.Zoom *= (1.0f + y * 0.1f);
	if (cam.Zoom < 0.1f) cam.Zoom = 0.1f;
	cam.Position = (mousePos / cam.Zoom) - mouseWorldBeforeZoom;
}

void Hexcells::InitTiles() {
	const size_t newSize = width * height;
	tiles.resize(newSize);

	for (size_t y = 0; y < height; y++) {
		for (size_t x = 0; x < width; x++) {
			Tile* newTile = new Tile();
			newTile->Position = int2(x, y);
			tiles[y * width + x] = newTile;
		}
	}

	for (size_t y = 0; y < height; y++) {
		for (size_t x = 0; x < width; x++) {
			Tile* currentTile = tiles[y * width + x];

			// Calculate position adjustments based on row parity
			int xOffset = y % 2;

			// Top Left
			if (y > 0) {
				currentTile->TopLeft = tiles[(y - 1) * width + x - 1 + xOffset];
			}

			// Top Right
			if (y > 0 && x < width - 1 + xOffset) {
				currentTile->TopRight = tiles[(y - 1) * width + x + xOffset];
			}

			// Bottom Left
			if (y < height - 1 && x > xOffset) {
				currentTile->BottomLeft = tiles[(y + 1) * width + x - 1 + xOffset];
			}

			// Bottom Right
			if (y < height - 1 && x < width - 1 + xOffset) {
				currentTile->BottomRight = tiles[(y + 1) * width + x + xOffset];
			}

			// top
			if (y > 1) {
				currentTile->Top = tiles[(y - 2) * width + x];
			}

			// bottom
			if (y < height - 2) {
				currentTile->Bottom = tiles[(y + 2) * width + x];
			}
		}
	}
}

void Hexcells::DestroyTiles() {
	for (size_t i = 0; i < tiles.size(); i++) {
		delete tiles[i];
	}
	tiles.clear();
}

//draws the tiles with a flat-top orientation
void Hexcells::DrawTiles() {
	const float zoom = cam.Zoom;
	const float newSpriteSize = size * zoom;
	const float2 camPos = cam.Position;
	const float horizontalSpacing = 1.5 * size; // Horizontal distance between tile centers before zoom
	const float verticalSpacing = sqrt(3) * size; // Vertical distance between tile centers before zoom

	for (size_t i = 0; i < tiles.size(); i++) {
		Tile* tile = tiles[i];
		float offsetX = (tile->Position.y % 2) * 0.75f * size * 2.0f;
		const float2 pos = float2(
			(tile->Position.x * (horizontalSpacing * 2.0f) + offsetX + camPos.x) * zoom,
			(tile->Position.y * verticalSpacing / 2.0f + camPos.y) * zoom
		);

		if (IsInBounds(pos, newSpriteSize)) {
			screen->HexagonFilled(pos.x, pos.y, newSpriteSize, tile->Color);
			screen->Hexagon(pos.x, pos.y, newSpriteSize, 0xff0000);
		}
	}
}

inline bool Hexcells::IsInBoundsOfTile(const Tile* tile, const int2& pos) {
	const float zoom = cam.Zoom;
	const float newSpriteSize = size * zoom;
	const float horizontalSpacing = 1.5 * size; // Horizontal distance between tile centers before zoom
	const float verticalSpacing = sqrt(3) * size; // Vertical distance between tile centers before zoom

	float offsetX = (tile->Position.y % 2) * 0.75f * size * 2.0f;
	const float2 tilePos = float2(
		(tile->Position.x * (horizontalSpacing * 2.0f) + offsetX + cam.Position.x) * zoom,
		(tile->Position.y * verticalSpacing / 2.0f + cam.Position.y) * zoom
	);


	const float2 diff = pos - tilePos;
	const float distance = sqrt(diff.x * diff.x + diff.y * diff.y);
	return distance < newSpriteSize;
}

void Hexcells::PerformanceReport(Timer& t) {
	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if (alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000.0f / avg, rps = (SCRWIDTH * SCRHEIGHT) / avg;
	printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps / 1000);
}