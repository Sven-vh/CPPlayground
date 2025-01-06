#pragma once

typedef unsigned short material_id;

constexpr material_id Empty_ID = 0;
constexpr material_id Sand_ID = 1;
constexpr material_id Bedrock_ID = USHRT_MAX;

struct Particle {
	material_id ID;
	bool HasUpdated;
	uint Color;
	float2 Velocity;

	Particle() : ID(Empty_ID), HasUpdated(false), Color(0), Velocity(0, 0) {}
	Particle(material_id id) : ID(id), HasUpdated(false), Color(0), Velocity(0, 0) {}
	Particle(material_id id, uint color) : ID(id), HasUpdated(false), Color(color), Velocity(0, 0) {}
};

class ParticleWorld {
public:
	ParticleWorld();
	ParticleWorld(int width, int height);
	~ParticleWorld();

	void Resize(const int width, const int height);
	void Clear();
	void Copy(const ParticleWorld& other);

	inline void Set(const int x, const int y, const Particle p);
	inline Particle GetCopy(const int x, const int y) const;
	inline Particle& GetRef(const int x, const int y);
	inline bool InBounds(const int x, const int y) const;

	inline int ToIndex(const int x, const int y) const;
	inline int2 ToCoord(const int index) const;

	int Width;
	int Height;
	Particle* Buffer;
	Particle Bedrock;
};

class SandSimulation : public TheApp {
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
	void MouseMove(int x, int y) { mousePos.x = x, mousePos.y = y; }
	void MouseWheel(float y) { /* implement if you want to handle the mouse wheel */ }
	void KeyUp(int key) { /* implement if you want to handle keys */ }
	void KeyDown(int key) {}
	// data members
	int2 mousePos;
	bool mouseDown;

private:
	ParticleWorld world;
	ParticleWorld newWorld;

	float accumulator;
	int tps = 20;

	int drawSize = 10;
	float probability = 0.5f;

	void UpdateWorld();
	void ResetUpdates();
	void DrawWorld();

	void Square(const int x, const int y, const int size, const float probability, const Particle p);

	// material functions
	void UpdateSand(const int x, const int y);
};

