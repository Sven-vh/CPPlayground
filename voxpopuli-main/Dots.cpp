#include "precomp.h"
#include <cstdint>
// -----------------------------------------------------------
// Initialize the renderer
// -----------------------------------------------------------
void Dots::Init() {
	const DWORD mask = 0b1 << (std::thread::hardware_concurrency() - 1);
	SetThreadAffinityMask(GetCurrentThread(), mask);

	SpawnAnts();
	InitColorLUT();
	uint color = RGBF32_to_RGB8(&backgroundColor);
	screen->Clear(color);
}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void Dots::Tick(float deltaTime) {
	deltaTime /= 100.0f;

	Timer t;
	UpdateAnts256(deltaTime);
	updateAntsTime = t.elapsed();

	t.reset();
	const uint color = RGBF32_to_RGB8(&backgroundColor);
	screen->Clear(color, evaporateSpeed);
	clearScreenTime = t.elapsed();

	t.reset();
	RenderAnts();
	renderAntsTime = t.elapsed();

	//PerformanceReport(t);
}

void Dots::UpdateAnts256(const float deltaTime) {
#if SIMD
	const __m256 width = _mm256_set1_ps((SCRWIDTH - 1));
	const __m256 height = _mm256_set1_ps((SCRHEIGHT - 1));
	const __m256 zero = _mm256_set1_ps(0.0f);
	const __m256 half = _mm256_set1_ps(0.5f);
	const __m256 maxSpeed = _mm256_set1_ps(MaxSpeed);
	const __m256 steerStrength = _mm256_set1_ps(SteerStrength);
	const __m256 wanderStrength = _mm256_set1_ps(WanderStrength);
	const __m256 deltaTimeSIMD = _mm256_set1_ps(deltaTime);
	const __m256 mousePosX = _mm256_set1_ps(mousePos.x);
	const __m256 mousePosY = _mm256_set1_ps(mousePos.y);
	const __m256 epsilon = _mm256_set1_ps(FLT_EPSILON);
	const int iterations = numAnts >> 3;

	#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < numAnts; i += 8) {

		//__m256 xDir, yDir, xVel, yVel, xAcc, yAcc, xPos, yPos;

		__m256 xDirResult, yDirResult;
		__m256 xPos = _mm256_load_ps(&xPositions[i]);
		__m256 yPos = _mm256_load_ps(&yPositions[i]);

		if (wander) {// 6-7ms
			//float2 random = RandomInsideUnitCircle();
			//float x = xDesiredDirections[i] + random.x * WanderStrength;
			//float y = yDesiredDirections[i] + random.y * WanderStrength;
			//float length = sqrt(x * x + y * y);
			//xDesiredDirections[i] = x / length;
			//yDesiredDirections[i] = y / length;

			//changing RandomFloatSIMD256() to _mm256_set1_ps(1.0f) will make it 3ms per frame...

			__m256 xRandom = RandomFloatSIMD256();
			__m256 yRandom = RandomFloatSIMD256();
			__m256 xDir = _mm256_add_ps(_mm256_load_ps(&xDesiredDirections[i]), _mm256_mul_ps(_mm256_sub_ps(xRandom, half), wanderStrength));
			__m256 yDir = _mm256_add_ps(_mm256_load_ps(&yDesiredDirections[i]), _mm256_mul_ps(_mm256_sub_ps(yRandom, half), wanderStrength));
			__m256 length = _mm256_sqrt_ps(_mm256_add_ps(_mm256_mul_ps(xDir, xDir), _mm256_mul_ps(yDir, yDir)));
			xDirResult = _mm256_div_ps(xDir, length);
			yDirResult = _mm256_div_ps(yDir, length);

		} else {// 3ms
			//float dx = mousePos.x - xPositions[i];
			//float dy = mousePos.y - yPositions[i];
			//float length = sqrt(dx * dx + dy * dy);
			//xDesiredDirections[i] = dx / length;
			//yDesiredDirections[i] = dy / length;

			__m256 dx = _mm256_sub_ps(mousePosX, xPos);
			__m256 dy = _mm256_sub_ps(mousePosY, yPos);
			//dx = _mm256_sub_ps(zero, dx);
			//dy = _mm256_sub_ps(zero, dy);
			__m256 length = _mm256_sqrt_ps(_mm256_add_ps(_mm256_mul_ps(dx, dx), _mm256_mul_ps(dy, dy)));
			length = _mm256_add_ps(length, epsilon);
			xDirResult = _mm256_div_ps(dx, length);
			yDirResult = _mm256_div_ps(dy, length);
		}

		_mm256_store_ps(&xDesiredDirections[i], xDirResult);
		_mm256_store_ps(&yDesiredDirections[i], yDirResult);

		//float xDesVel = xDesiredDirections[i] * MaxSpeed;
		//float yDesVel = yDesiredDirections[i] * MaxSpeed;
		__m256 xDesVel = _mm256_mul_ps(xDirResult, maxSpeed);
		__m256 yDesVel = _mm256_mul_ps(yDirResult, maxSpeed);

		__m256 xVelOld = _mm256_load_ps(&xVelocities[i]);
		__m256 yVelOld = _mm256_load_ps(&yVelocities[i]);

		//float xDesSteer = (xDesVel - xVelocities[i]) * SteerStrength;
		//float yDesSteer = (yDesVel - yVelocities[i]) * SteerStrength;
		__m256 xDesSteer = _mm256_mul_ps(_mm256_sub_ps(xDesVel, xVelOld), steerStrength);
		__m256 yDesSteer = _mm256_mul_ps(_mm256_sub_ps(yDesVel, yVelOld), steerStrength);

		//float length = sqrt(xDesSteer * xDesSteer + yDesSteer * yDesSteer);
		//float clampedLength = min(length, SteerStrength);
		//float scale = (length > 0.0f) ? (clampedLength / length) : 0.0f;
		//float xAcc = xDesSteer * scale;
		//float yAcc = yDesSteer * scale;
		__m256 length = _mm256_sqrt_ps(_mm256_add_ps(_mm256_mul_ps(xDesSteer, xDesSteer), _mm256_mul_ps(yDesSteer, yDesSteer)));
		__m256 clampedLength = _mm256_min_ps(length, steerStrength);
		__m256 scale = _mm256_div_ps(clampedLength, _mm256_add_ps(length, epsilon));
		__m256 xAcc = _mm256_mul_ps(xDesSteer, scale);
		__m256 yAcc = _mm256_mul_ps(yDesSteer, scale);


		//float xVel = xVelocities[i] + xAcc * deltaTime;
		//float yVel = yVelocities[i] + yAcc * deltaTime;
		//float velLength = sqrt(xVel * xVel + yVel * yVel);
		//float clampedVelLength = min(velLength, MaxSpeed);
		//float velScale = (velLength > 0.0f) ? (clampedVelLength / velLength) : 0.0f;
		//xVelocities[i] = xVel * velScale;
		//yVelocities[i] = yVel * velScale;

		__m256 xVel = _mm256_add_ps(xVelOld, _mm256_mul_ps(xAcc, deltaTimeSIMD));
		__m256 yVel = _mm256_add_ps(yVelOld, _mm256_mul_ps(yAcc, deltaTimeSIMD));
		//__m256 velLength = _mm256_sqrt_ps(_mm256_add_ps(_mm256_mul_ps(xVel, xVel), _mm256_mul_ps(yVel, yVel)));
		//__m256 clampedVelLength = _mm256_min_ps(velLength, maxSpeed);
		//__m256 velScale = _mm256_div_ps(clampedVelLength, velLength);
		//__m256 newXVel = _mm256_mul_ps(xVel, velScale);
		//__m256 newYVel = _mm256_mul_ps(yVel, velScale);
		_mm256_store_ps(&xVelocities[i], xVel);
		_mm256_store_ps(&yVelocities[i], yVel);


		//xPositions[i] += xVelocities[i] * deltaTime;
		//yPositions[i] += yVelocities[i] * deltaTime;
		__m256 xresult = _mm256_add_ps(xPos, _mm256_mul_ps(xVel, deltaTimeSIMD));
		__m256 yresult = _mm256_add_ps(yPos, _mm256_mul_ps(yVel, deltaTimeSIMD));

		// Adjust x positions with wrapping
		__m256 mask_greater_width = _mm256_cmp_ps(xresult, width, _CMP_GE_OS); // x >= SCRWIDTH
		__m256 mask_less_zero = _mm256_cmp_ps(xresult, zero, _CMP_LT_OS); // x < 0
		__m256 wrapped_greater_width = _mm256_sub_ps(xresult, width); // Wrap around for x >= SCRWIDTH
		__m256 wrapped_less_zero = _mm256_add_ps(xresult, width); // Wrap around for x < 0
		__m256 resultX = _mm256_blendv_ps(xresult, wrapped_greater_width, mask_greater_width); // Apply wrap for x >= SCRWIDTH
		resultX = _mm256_blendv_ps(resultX, wrapped_less_zero, mask_less_zero); // Apply wrap for x < 0
		_mm256_store_ps(&xPositions[i], resultX); // Store the final x positions

		// Adjust y positions with wrapping
		__m256 mask_greater_height = _mm256_cmp_ps(yresult, height, _CMP_GE_OS); // y >= SCRHEIGHT
		__m256 mask_less_zero_y = _mm256_cmp_ps(yresult, zero, _CMP_LT_OS); // y < 0
		__m256 wrapped_greater_height = _mm256_sub_ps(yresult, height); // Wrap around for y >= SCRHEIGHT
		__m256 wrapped_less_zero_y = _mm256_add_ps(yresult, height); // Wrap around for y < 0
		__m256 resultY = _mm256_blendv_ps(yresult, wrapped_greater_height, mask_greater_height); // Apply wrap for y >= SCRHEIGHT
		resultY = _mm256_blendv_ps(resultY, wrapped_less_zero_y, mask_less_zero_y); // Apply wrap for y < 0
		_mm256_store_ps(&yPositions[i], resultY); // Store the final y positions


		//angles[i] = atan2(yVel, xVel);
		//__m256 angle = _mm256_atan2_ps(newYVel, newXVel);
		//_mm256_store_ps(&angles[i], angle);
	}
#else
	for (int i = 0; i < ants.size(); i++) {
		Ant& ant = ants[i];

		if (wander) {
			ant.desiredDirection = normalize(ant.desiredDirection + RandomInsideUnitCircle() * WanderStrength);
		} else {
			ant.desiredDirection = normalize(mousePos - ant.position);
		}

		float2 desiredVelocity = ant.desiredDirection * MaxSpeed;
		float2 desiredSteeringForce = (desiredVelocity - ant.velocity) * SteerStrength;
		float2 acceleration = clampMagnitude(desiredSteeringForce, SteerStrength) / 1.0f;

		ant.velocity = clampMagnitude(ant.velocity + acceleration * deltaTime, MaxSpeed);
		ant.position += ant.velocity * deltaTime;

		if (ant.position.x > SCRWIDTH) ant.position.x = ant.position.x - SCRWIDTH;
		if (ant.position.x < 0) ant.position.x = SCRWIDTH - ant.position.x;
		if (ant.position.y > SCRHEIGHT) ant.position.y = ant.position.y - SCRHEIGHT;
		if (ant.position.y < 0) ant.position.y = SCRHEIGHT - ant.position.y;

		//ant.angle = atan2(ant.velocity.y, ant.velocity.x); //3.7-3.8ms
//ant.angle = atan2_approximation2(ant.velocity.y, ant.velocity.x); // 3.2ms
		ant.angle = atan2_approximation1(ant.velocity.y, ant.velocity.x); // 3.1-3.2ms
	}
#endif
}

void Dots::InitColorLUT() {
	for (int i = 0; i < LUT_SIZE; ++i) {
		float hue = i / static_cast<float>(LUT_SIZE); // Hue between 0 and 1
		float r, g, b;
		float h = hue * 6.0f;
		int j = int(h);
		float f = h - j;
		float q = 1.0f - f;

		switch (j % 6) {
		case 0: r = 1.0f, g = f, b = 0.0f; break;
		case 1: r = q, g = 1.0f, b = 0.0f; break;
		case 2: r = 0.0f, g = 1.0f, b = f; break;
		case 3: r = 0.0f, g = q, b = 1.0f; break;
		case 4: r = f, g = 0.0f, b = 1.0f; break;
		case 5: r = 1.0f, g = 0.0f, b = q; break;
		}

		// Convert to 0-255 range
		uint8_t red = static_cast<uint8_t>(r * 255);
		uint8_t green = static_cast<uint8_t>(g * 255);
		uint8_t blue = static_cast<uint8_t>(b * 255);

		// Pack RGB values into a single uint32_t (ARGB format)
		colorLUT[i] = (0xFF << 24) | (red << 16) | (green << 8) | blue;
	}
}

//void Dots::UpdateAnts512(const float deltaTime) {
//	const __m512 width = _mm512_set1_ps((SCRWIDTH - 1));
//	const __m512 height = _mm512_set1_ps((SCRHEIGHT - 1));
//	const __m512 zero = _mm512_set1_ps(0.0f);
//	const __m512 half = _mm512_set1_ps(0.5f);
//	const __m512 maxSpeed = _mm512_set1_ps(MaxSpeed);
//	const __m512 steerStrength = _mm512_set1_ps(SteerStrength);
//	const __m512 wanderStrength = _mm512_set1_ps(WanderStrength);
//	const __m512 deltaTimeSIMD = _mm512_set1_ps(deltaTime);
//	const __m512 mousePosX = _mm512_set1_ps(mousePos.x);
//	const __m512 mousePosY = _mm512_set1_ps(mousePos.y);
//	const __m512 epsilon = _mm512_set1_ps(FLT_EPSILON);
//	const int iterations = numAnts >> 4;
//
//	//#pragma omp parallel for schedule(dynamic)
//	for (int i = 0; i < numAnts; i += 16) {
//
//		//__m512 xDir, yDir, xVel, yVel, xAcc, yAcc, xPos, yPos;
//
//		__m512 xDirResult, yDirResult;
//		__m512 xPos = _mm512_load_ps(&xPositions[i]);
//		__m512 yPos = _mm512_load_ps(&yPositions[i]);
//
//		if (wander) {
//			//float2 random = RandomInsideUnitCircle();
//			//float x = xDesiredDirections[i] + random.x * WanderStrength;
//			//float y = yDesiredDirections[i] + random.y * WanderStrength;
//			//float length = sqrt(x * x + y * y);
//			//xDesiredDirections[i] = x / length;
//			//yDesiredDirections[i] = y / length;
//			//const int index = Rand(iterations) - 1;
//
//			__m512 xDir = _mm512_add_ps(_mm512_set1_ps(xDesiredDirections[i]), _mm512_mul_ps(_mm512_sub_ps(RandomFloatSIMD512(), half), wanderStrength));
//			__m512 yDir = _mm512_add_ps(_mm512_set1_ps(yDesiredDirections[i]), _mm512_mul_ps(_mm512_sub_ps(RandomFloatSIMD512(), half), wanderStrength));
//			__m512 length = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(xDir, xDir), _mm512_mul_ps(yDir, yDir)));
//			xDirResult = _mm512_div_ps(xDir, length);
//			yDirResult = _mm512_div_ps(yDir, length);
//
//		} else {
//			//float dx = mousePos.x - xPositions[i];
//			//float dy = mousePos.y - yPositions[i];
//			//float length = sqrt(dx * dx + dy * dy);
//			//xDesiredDirections[i] = dx / length;
//			//yDesiredDirections[i] = dy / length;
//
//			__m512 dx = _mm512_sub_ps(mousePosX, xPos);
//			__m512 dy = _mm512_sub_ps(mousePosY, yPos);
//			//dx = _mm512_sub_ps(zero, dx);
//			//dy = _mm512_sub_ps(zero, dy);
//			__m512 length = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(dx, dx), _mm512_mul_ps(dy, dy)));
//			length = _mm512_add_ps(length, epsilon);
//			xDirResult = _mm512_div_ps(dx, length);
//			yDirResult = _mm512_div_ps(dy, length);
//		}
//
//		_mm512_store_ps(&xDesiredDirections[i], xDirResult);
//		_mm512_store_ps(&yDesiredDirections[i], yDirResult);
//
//		//float xDesVel = xDesiredDirections[i] * MaxSpeed;
//		//float yDesVel = yDesiredDirections[i] * MaxSpeed;
//		__m512 xDesVel = _mm512_mul_ps(xDirResult, maxSpeed);
//		__m512 yDesVel = _mm512_mul_ps(yDirResult, maxSpeed);
//
//		//float xDesSteer = (xDesVel - xVelocities[i]) * SteerStrength;
//		//float yDesSteer = (yDesVel - yVelocities[i]) * SteerStrength;
//		__m512 xDesSteer = _mm512_mul_ps(_mm512_sub_ps(xDesVel, xDirResult), steerStrength);
//		__m512 yDesSteer = _mm512_mul_ps(_mm512_sub_ps(yDesVel, yDirResult), steerStrength);
//
//		//float length = sqrt(xDesSteer * xDesSteer + yDesSteer * yDesSteer);
//		//float clampedLength = min(length, SteerStrength);
//		//float scale = (length > 0.0f) ? (clampedLength / length) : 0.0f;
//		//float xAcc = xDesSteer * scale;
//		//float yAcc = yDesSteer * scale;
//		__m512 length = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(xDesSteer, xDesSteer), _mm512_mul_ps(yDesSteer, yDesSteer)));
//		__m512 clampedLength = _mm512_min_ps(length, steerStrength);
//		__m512 scale = _mm512_div_ps(clampedLength, _mm512_add_ps(length, epsilon));
//		__m512 xAcc = _mm512_mul_ps(xDesSteer, scale);
//		__m512 yAcc = _mm512_mul_ps(yDesSteer, scale);
//
//
//		//float xVel = xVelocities[i] + xAcc * deltaTime;
//		//float yVel = yVelocities[i] + yAcc * deltaTime;
//		//float velLength = sqrt(xVel * xVel + yVel * yVel);
//		//float clampedVelLength = min(velLength, MaxSpeed);
//		//float velScale = (velLength > 0.0f) ? (clampedVelLength / velLength) : 0.0f;
//		//xVelocities[i] = xVel * velScale;
//		//yVelocities[i] = yVel * velScale;
//		__m512 xVelOld = _mm512_set1_ps(xVelocities[i]);
//		__m512 yVelOld = _mm512_set1_ps(yVelocities[i]);
//		__m512 xVel = _mm512_add_ps(xVelOld, _mm512_mul_ps(xAcc, deltaTimeSIMD));
//		__m512 yVel = _mm512_add_ps(yVelOld, _mm512_mul_ps(yAcc, deltaTimeSIMD));
//		__m512 velLength = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(xVel, xVel), _mm512_mul_ps(yVel, yVel)));
//		__m512 clampedVelLength = _mm512_min_ps(velLength, maxSpeed);
//		__m512 velScale = _mm512_div_ps(clampedVelLength, velLength);
//		__m512 newXVel = _mm512_mul_ps(xVel, velScale);
//		__m512 newYVel = _mm512_mul_ps(yVel, velScale);
//		_mm512_store_ps(&xVelocities[i], newXVel);
//		_mm512_store_ps(&yVelocities[i], newYVel);
//
//
//		//xPositions[i] += xVelocities[i] * deltaTime;
//		//yPositions[i] += yVelocities[i] * deltaTime;
//		__m512 xresult = _mm512_add_ps(xPos, _mm512_mul_ps(xVelOld, deltaTimeSIMD));
//		__m512 yresult = _mm512_add_ps(yPos, _mm512_mul_ps(yVelOld, deltaTimeSIMD));
//
//		// Adjust x positions with wrapping
//		__mmask16  mask_greater_width = _mm512_cmp_ps_mask(xresult, width, _CMP_GE_OS); // x >= SCRWIDTH
//		__mmask16  mask_less_zero = _mm512_cmp_ps_mask(xresult, zero, _CMP_LT_OS); // x < 0
//		__m512 wrapped_greater_width = _mm512_sub_ps(xresult, width); // Wrap around for x >= SCRWIDTH
//		__m512 wrapped_less_zero = _mm512_add_ps(xresult, width); // Wrap around for x < 0
//		__m512 resultX = _mm512_mask_blend_ps(mask_greater_width, xresult, wrapped_greater_width); // Apply wrap for x >= SCRWIDTH
//		resultX = _mm512_mask_blend_ps(mask_less_zero, resultX, wrapped_less_zero); // Apply wrap for x < 0
//		_mm512_store_ps(&xPositions[i], resultX); // Store the final x positions
//
//		// Adjust y positions with wrapping
//		__mmask16 mask_greater_height = _mm512_cmp_ps_mask(yresult, height, _CMP_GE_OS); // y >= SCRHEIGHT
//		__mmask16 mask_less_zero_y = _mm512_cmp_ps_mask(yresult, zero, _CMP_LT_OS); // y < 0
//		__m512 wrapped_greater_height = _mm512_sub_ps(yresult, height); // Wrap around for y >= SCRHEIGHT
//		__m512 wrapped_less_zero_y = _mm512_add_ps(yresult, height); // Wrap around for y < 0
//		__m512 resultY = _mm512_mask_blend_ps(mask_greater_height, yresult, wrapped_greater_height); // Apply wrap for y >= SCRHEIGHT
//		resultY = _mm512_mask_blend_ps(mask_less_zero_y, resultY, wrapped_less_zero_y); // Apply wrap for y < 0
//		_mm512_store_ps(&yPositions[i], resultY); // Store the final y positions
//
//
//		//angles[i] = atan2(yVel, xVel);
//		__m512 angle = _mm512_atan2_ps(newYVel, newXVel);
//		_mm512_store_ps(&angles[i], angle);
//	}
//}

#if HDR
inline void safe_add(float4& x, const float4& y) {
	x += y;
}
#else
inline void safe_add(uint& x, const uint& y) {
	if (0xffffff - x < y) {
		x = 0xffffff; // Handle overflow, this could be an error or a max value
	} else {
		x += y;
	}
}
#endif

void Dots::RenderAnts() {

#if 1
	//old code
	const float2 size = float2(antSize);
#if HDR
	const float4 color = antColor;
#else
	const uint color = RGBF32_to_RGB8(&antColor);
#endif
	for (uint i = 0; i < numAnts; i++) {
		const int x = xPositions[i];
		const int y = yPositions[i];

		//const float2 vel = float2(xVelocities[i], yVelocities[i]);
		//const float2 dir = normalize(vel);
		//const float4 floatColor = float4(dir.x, dir.y, 0.0f, 1.0f);
		//const uint color = RGBF32_to_RGB8(&floatColor);
		const int index = x + y * SCRWIDTH;
		safe_add(screen->pixels[index], color);
	}

#elif 0
	std::vector<UINT8> rColors;
	std::vector<UINT8> gColors;
	std::vector<UINT8> bColors;
	//new code with SIMD
	for (int i = 0; i < numAnts; i += 8) {
		//const float2 vel = float2(xVelocities[i], yVelocities[i]);
		//const float2 dir = normalize(vel);
		//const float4 floatColor = float4(dir.x, dir.y, 0.0f, 1.0f);
		//const uint color = RGBF32_to_RGB8(&floatColor);

		const __m256 xVel = _mm256_load_ps(&xVelocities[i]);
		const __m256 yVel = _mm256_load_ps(&yVelocities[i]);
		const __m256 xDir = _mm256_div_ps(xVel, _mm256_sqrt_ps(_mm256_add_ps(_mm256_mul_ps(xVel, xVel), _mm256_mul_ps(yVel, yVel))));
		const __m256 yDir = _mm256_div_ps(yVel, _mm256_sqrt_ps(_mm256_add_ps(_mm256_mul_ps(xVel, xVel), _mm256_mul_ps(yVel, yVel))));
		const __m256 r = _mm256_mul_ps(xDir, _mm256_set1_ps(0.5f));
		const __m256 g = _mm256_mul_ps(yDir, _mm256_set1_ps(0.5f));
		const __m256 b = _mm256_setzero_ps();
		__m256i rInt = _mm256_cvtps_epi32(_mm256_mul_ps(r, _mm256_set1_ps(255.0f)));
		__m256i gInt = _mm256_cvtps_epi32(_mm256_mul_ps(g, _mm256_set1_ps(255.0f)));
		__m256i bInt = _mm256_cvtps_epi32(_mm256_mul_ps(b, _mm256_set1_ps(255.0f)));


	}

	for (uint i = 0; i < numAnts; i++) {
		const int x = xPositions[i];
		const int y = yPositions[i];
		const uint color = (rColors[i] << 16) | (gColors[i] << 8) | bColors[i];
		screen->pixels[x + y * SCRWIDTH] = color;
	}

#else
	//old code
	const float2 size = float2(antSize);
	const uint color = RGBF32_to_RGB8(&antColor);
	for (uint i = 0; i < numAnts; i++) {
		const int x = xPositions[i];
		const int y = yPositions[i];

		const float2 vel = float2(xVelocities[i], yVelocities[i]);
		const float2 dir = normalize(vel);
		const float angle = atan2(dir.x, dir.y);
		int index = static_cast<int>((angle / (2.0 * PI)) * LUT_SIZE) % LUT_SIZE;
		if (index < 0) index += LUT_SIZE; // Ensure the index is positive
		const uint color = colorLUT[index];
		screen->pixels[x + y * SCRWIDTH] = color;
}
#endif
}

void Dots::SubtractScreen(const float deltaTime) {
	if (!clearScreen) return;
#pragma omp parallel for schedule(dynamic)
	for (int y = 0; y < SCRHEIGHT; y++) {
		for (int x = 0; x < SCRWIDTH; x++) {
			const float4 color = screen->GetPixel(x, y);
			if (color == backgroundColor) continue;
			const float4 evaporatedValue = max(float4(0.0f), color - evaporateSpeed * deltaTime);
#if HDR
			screen->Plot(x, y, evaporatedValue);
#else
			uint evaporatedColor = RGBF32_to_RGB8(&evaporatedValue);
			screen->Plot(x, y, evaporatedColor);
#endif
		}
	}
}

void Dots::SpawnAnts() {
	//initialize the ants
	for (int i = 0; i < spawnCount; i++) {
#if SIMD
		xPositions.push_back(SCRWIDTH * 0.5f);
		yPositions.push_back(SCRHEIGHT * 0.5f);
		xVelocities.push_back(0.0f);
		yVelocities.push_back(0.0f);
		xDesiredDirections.push_back((RandomFloat() - 0.5f) * 2.0f);
		yDesiredDirections.push_back((RandomFloat() - 0.5f) * 2.0f);
		rColors.push_back(0);
		gColors.push_back(0);
		bColors.push_back(0);
		//angles.push_back(0.0f);
		numAnts++;
#else
		Ant ant;
		ant.position = float2(SCRWIDTH * 0.5f, SCRHEIGHT * 0.5f);
		ants.push_back(ant);
#endif
	}

	//randoms.clear();
	////fill the random seed array
	//for (int i = 0; i < numAnts; i += 8) {
	//	randoms.push_back(RandomFloatSIMD256());
	//}
}

void Dots::ClearAnts() {
#if SIMD
	xPositions.clear();
	yPositions.clear();
	xVelocities.clear();
	yVelocities.clear();
	xDesiredDirections.clear();
	yDesiredDirections.clear();
	rColors.clear();
	gColors.clear();
	bColors.clear();

	//angles.clear();
	numAnts = 0;
#else
	ants.clear();
#endif
}

// -----------------------------------------------------------
// Update user interface (imgui)
// -----------------------------------------------------------
void Dots::UI() {
	ImGui::Begin("Ant Settings");

	ImGui::Text("Update Ants Time: %f ms", updateAntsTime * 1000.0f);
	ImGui::Text("Clear Screen Time: %f ms", clearScreenTime * 1000.0f);
	ImGui::Text("Render Ants Time: %f ms", renderAntsTime * 1000.0f);

	//Imgui text with thousands separator
#if SIMD
	std::string numAntsStr = std::to_string(numAnts);
#else
	std::string numAntsStr = std::to_string(ants.size());
#endif
	std::string formattedNumAnts = "";
	int count = 0;
	for (int i = numAntsStr.size() - 1; i >= 0; i--) {
		formattedNumAnts = numAntsStr[i] + formattedNumAnts;
		count++;
		if (count % 3 == 0 && i != 0) {
			formattedNumAnts = "," + formattedNumAnts;
		}
	}
	ImGui::Text("Ant Count: %s", formattedNumAnts.c_str());

	ImGui::DragInt("Ant Count", &spawnCount, 1, 0, 1000000);
	if (ImGui::Button("Spawn Ants")) {
		SpawnAnts();
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear Ants")) {
		ClearAnts();
	}
	if (ImGui::Button("Clear Screen")) {
		uint color = RGBF32_to_RGB8(&backgroundColor);
		screen->Clear(color);
	}

	ImGui::DragFloat("Ant Size", &antSize, 0.1f, 0.0f, 100.0f);
	ImGui::ColorEdit4("Ant Color", &antColor.x);
	ImGui::ColorEdit4("Background Color", &backgroundColor.x);
	ImGui::DragFloat("Evaporate Speed", &evaporateSpeed, 0.0001f, 0.0f, 1.0f, "%.10f");
	ImGui::DragFloat("MaxSpeed", &MaxSpeed, 0.1f, 0.0f, 10.0f);
	ImGui::DragFloat("SteerStrength", &SteerStrength, 0.1f, 0.0f, 10.0f);
	ImGui::DragFloat("WanderStrength", &WanderStrength, 0.1f, 0.0f, 10.0f);
	ImGui::Checkbox("Wander", &wander);
	ImGui::Checkbox("Clear Screen toggle", &clearScreen);
	ImGui::End();
}

// -----------------------------------------------------------
// User wants to close down
// -----------------------------------------------------------
void Dots::Shutdown() {

}

void Dots::MouseUp(int button) {
	if (!mouseDown) return;
	mouseDown = false;
	wander = true;
}

void Dots::MouseDown(int button) {
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse) return;

	mouseDown = true;
	wander = false;
}

void Dots::PerformanceReport(Timer& t) {
	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if (alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000.0f / avg, rps = (SCRWIDTH * SCRHEIGHT) / avg;
	printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps / 1000);
}