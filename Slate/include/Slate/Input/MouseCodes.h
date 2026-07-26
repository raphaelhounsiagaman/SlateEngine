#pragma once

#include <cstdint>

namespace Slate
{
	enum class MouseButton : std::uint8_t
	{
		Left = 0,
		Right,
		Middle,
		X1,
		X2,

		Count
	};
}
