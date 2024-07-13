#pragma once
#define NEIGHBOUR_COUNT 6

struct Tile {
public:
	bool HasBomb;
	uint Color = 0xffffff;
	int2 Position;
	union {
		struct {
			Tile* Top;
			Tile* TopRight;
			Tile* BottomRight;
			Tile* Bottom;
			Tile* BottomLeft;
			Tile* TopLeft;
		};
		Tile* neighbors[NEIGHBOUR_COUNT];
	};
	/*int test[NEIGHBOUR_COUNT];*/
};

struct Camera2D {
public:
	float2 Position;
	float Zoom = 1.0f;
};

class Hexcells : public TheApp {
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
	void KeyUp(int key) { /* implement if you want to handle keys */ }
	void KeyDown(int key) { /* implement if you want to handle keys */ }
	// data members
private:
	int width = 10;
	int height = 10;
	int size = 10;
	std::vector<Tile*> tiles;

	uint backgroundColor = 0;
	uint spriteColor = 0xffffff;
	float2 tileOffset = float2(21);

	Camera2D cam;

	float moveSpeed = 1;
	float zoomSpeed = 1;

	int2 mousePos;
	bool moveCam;

	void InitTiles();
	void DestroyTiles();
	void DrawTiles();

	inline bool IsInBoundsOfTile(const Tile* tile, const int2& pos);

	static inline bool IsInBounds(const float2 pos, const float size) {
		return pos.x >= -size && pos.x < SCRWIDTH + size && pos.y >= -size && pos.y < SCRHEIGHT + size;
	}

	inline int GetTileIndex(const int2& pos) {
		return pos.y * width + pos.x;
	}
};

