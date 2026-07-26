#pragma once

#include "Slate/Math/Math.h"

namespace Slate
{
	struct Camera3D
	{
		Transform3D Transform{};
		float VerticalFieldOfViewRadians =
			60.0f * 3.1415926535f / 180.0f;
		float NearPlaneMetres = 0.1f;
		float FarPlaneMetres = 1000.0f;
	};
}

