#pragma once

#include "Slate/Math/Math.h"

namespace Slate
{
	struct Vertex2D
	{
		// Unit: Metres
		Vector2f Position{};
	};

	struct Vertex3D
	{
		// Unit: Metres
		Vector3f Position{};
		Vector3f Normal{};
	};

}