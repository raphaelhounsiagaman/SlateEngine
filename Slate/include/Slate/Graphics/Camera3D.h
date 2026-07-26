#pragma once

#include "Slate/Math/Math.h"

namespace Slate
{
	struct Camera3D
	{
		Transform3D Transform{};
		float FOV = 60.0f * 3.1415926535f / 180.0f;
		float NearPlane = 0.1f;
		float FarPlane = 1000.0f;
	};
}

