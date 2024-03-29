#include "precomp.h"

// -----------------------------------------------------------
// Initialize the renderer
// -----------------------------------------------------------
void Renderer::Init() {
	// initialize particles
	for (int i = 0; i < particleCount; i++) {
		Circle particle;
		particle.pos = float2(Rand(SCRWIDTH), Rand(SCRHEIGHT));
		particle.velocity = float2(0.0f);
		particles.push_back(particle);
	}
}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void Renderer::Tick(float deltaTime) {
	//deltaTime /= 1000.0f;
	Timer t;
	screen->Clear(0);

	const int particleCount = particles.size();

#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < particleCount; i++) {
		Circle& particle = particles[i];
		UpdateParticle(deltaTime, particle);
	}

#pragma omp parallel for schedule(dynamic)
	for (int iteration = 0; iteration < collisionIterations; iteration++) {
		for (int i = 0; i < particleCount; i++) {
			Circle& particle = particles[i];

			ResolveCollision(particle);
		}
	}

#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < particleCount; i++) {
		Circle& particle = particles[i];
		screen->Circle(particle.pos.x, particle.pos.y, particleSize, 0xffffff);
	}
	PerformanceReport(t);
}

void Tmpl8::Renderer::PerformanceReport(Timer& t) {
	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if (alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000.0f / avg, rps = (SCRWIDTH * SCRHEIGHT) / avg;
	printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps / 1000);
}

// -----------------------------------------------------------
// Update user interface (imgui)
// -----------------------------------------------------------
void Renderer::UI() {
	//particleCount = 100;
	ImGui::SliderInt("Particle Count", &particleCount, 1, 1000);
	ImGui::DragFloat("Gravity", &gravity, 0.0001f, 0.0f, 0.1f);
	ImGui::DragFloat("Particle Size", &particleSize, 0.1f, 1.0f, 100.0f);
	ImGui::DragFloat("Collision Damping", &collisionDamping, 0.01f, 0.0f, 1.0f);
	ImGui::SliderInt("Collision Iterations", &collisionIterations, 1, 10);

	//clear button to reset the particles
	if (ImGui::Button("Clear")) {
		particles.clear();
		for (int i = 0; i < particleCount; i++) {
			Circle particle;
			particle.pos = float2(Rand(SCRWIDTH), Rand(SCRHEIGHT));
			particle.velocity = float2(0.0f);
			particles.push_back(particle);
		}
	}
}

// -----------------------------------------------------------
// User wants to close down
// -----------------------------------------------------------
void Renderer::Shutdown() {
	particles.clear();
}

void Tmpl8::Renderer::UpdateParticle(const float deltaTime, Circle& particle) const {
	particle.velocity += float2(0.0f, 1.0f) * gravity * deltaTime;
	//clamp velocity
	particle.velocity = clamp(particle.velocity, float2(-2.0f), float2(2.0f));
	particle.pos += particle.velocity * deltaTime;
}

void Tmpl8::Renderer::ResolveCollision(Circle& particle) {
	const float inverseDamper = -0.8f * collisionDamping; // More realistic damping
	const float restitutionCoefficient = 0.5f; // Adjust for bounciness


	bool sideCollision = false;

	if (particle.pos.y > SCRHEIGHT - particleSize) {
		particle.pos.y = SCRHEIGHT - particleSize;
		particle.velocity.y *= inverseDamper;
		sideCollision = true;
	}

	if (particle.pos.y < particleSize) {
		particle.pos.y = particleSize;
		sideCollision = true;
		particle.velocity.y *= inverseDamper;
	}

	if (particle.pos.x > SCRWIDTH - particleSize) {
		particle.pos.x = SCRWIDTH - particleSize;
		sideCollision = true;
		particle.velocity.x *= inverseDamper;
	}

	if (particle.pos.x < particleSize) {
		particle.pos.x = particleSize;
		sideCollision = true;
		particle.velocity.x *= inverseDamper;
	}

	//check for collision with other particles by just pushing both particles away from each other ever so slightly
	for (int i = 0; i < particles.size(); i++) {
		if (i != &particle - &particles[0]) {
			Circle& other = particles[i];
			float2 diff = particle.pos - other.pos;
			float distance = length(diff);
			if (distance < particleSize * 2) {
				float2 correction = diff * (particleSize * 2 - distance) / distance;
					other.pos -= correction * 0.5f;
					particle.pos += correction * 0.5f;
				//if (sideCollision) {
				//	other.pos -= correction;
				//} else {
				//}
			}
		}
	}
}

