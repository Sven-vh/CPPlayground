#pragma once

// default screen resolution

//#define SCRWIDTH	100
//#define SCRHEIGHT	100
// #define FULLSCREEN
// #define DOUBLESIZE

namespace Tmpl8 {

class Camera
{
public:
	Camera()
	{
		// setup a basic view frustum
		camPos = float3( 0, 0, -2 );
		camTarget = float3( 0, 0, -1 );
		topLeft = float3( -aspect, 1, 0 );
		topRight = float3( aspect, 1, 0 );
		bottomLeft = float3( -aspect, -1, 0 );
	}
	Ray GetPrimaryRay( const float x, const float y )
	{
		// calculate pixel position on virtual screen plane
		const float u = x * (1.0f / SCRWIDTH);
		const float v = y * (1.0f / SCRHEIGHT);
		const float3 P = topLeft + u * (topRight - topLeft) + v * (bottomLeft - topLeft);
		// return Ray( camPos, normalize( P - camPos ) );
		return Ray( camPos, P - camPos ); 
		// Note: no need to normalize primary rays in a pure voxel world
		// TODO: 
		// - if we have other primitives as well, we *do* need to normalize!
		// - there are far cooler camera models, e.g. try 'Panini projection'.
	}
	bool HandleInput( const float t )
	{
		if (!WindowHasFocus()) return false;
		float speed = 0.0025f * t;
		float3 ahead = normalize( camTarget - camPos );
		float3 tmpUp( 0, 1, 0 );
		float3 right = normalize( cross( tmpUp, ahead ) );
		float3 up = normalize( cross( ahead, right ) );
		bool changed = false;
		if (IsKeyDown( GLFW_KEY_UP )) camTarget -= speed * up, changed = true;
		if (IsKeyDown( GLFW_KEY_DOWN )) camTarget += speed * up, changed = true;
		if (IsKeyDown( GLFW_KEY_LEFT )) camTarget -= speed * right, changed = true;
		if (IsKeyDown( GLFW_KEY_RIGHT )) camTarget += speed * right, changed = true;
		ahead = normalize( camTarget - camPos );
		right = normalize( cross( tmpUp, ahead ) );
		up = normalize( cross( ahead, right ) );
		if (IsKeyDown( GLFW_KEY_A )) camPos -= speed * right, changed = true;
		if (IsKeyDown( GLFW_KEY_D )) camPos += speed * right, changed = true;
		if (GetAsyncKeyState( 'W' )) camPos += speed * ahead, changed = true;
		if (IsKeyDown( GLFW_KEY_S )) camPos -= speed * ahead, changed = true;
		if (IsKeyDown( GLFW_KEY_R )) camPos += speed * up, changed = true;
		if (IsKeyDown( GLFW_KEY_F )) camPos -= speed * up, changed = true;
		camTarget = camPos + ahead;
		ahead = normalize( camTarget - camPos );
		up = normalize( cross( ahead, right ) );
		right = normalize( cross( up, ahead ) );
		topLeft = camPos + 2 * ahead - aspect * right + up;
		topRight = camPos + 2 * ahead + aspect * right + up;
		bottomLeft = camPos + 2 * ahead - aspect * right - up;
		if (!changed) return false;
		return true;
	}
	float aspect = (float)SCRWIDTH / (float)SCRHEIGHT;
	float3 camPos, camTarget;
	float3 topLeft, topRight, bottomLeft;
};

}