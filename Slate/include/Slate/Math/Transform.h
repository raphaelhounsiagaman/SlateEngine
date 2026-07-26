#pragma once

#include "Vector.h"
#include "Quaternion.h"

namespace Slate
{
	struct Transform2D
	{
		Vector2f Position{ 0.0f, 0.0f };
		Vector2f Scale{ 1.0f, 1.0f };
		float Rotation{ 0.0f };
	};

	struct Transform3D
	{
		Quaternion Rotation{ 0.0f, 0.0f, 0.0f, 1.0f };
		Vector3f Position{ 0.0f, 0.0f, 0.0f };
		Vector3f Scale{ 1.0f, 1.0f, 1.0f };
	};

}
