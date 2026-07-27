#pragma once

#include "Color.h"
#include "Slate/Math/Vector.h"

#include <string>

namespace Slate
{
	struct Rectangle2D
	{
		Vector2f Position{ 0, 0 };
		Vector2f Size{ 0, 0 };

		bool Contains(Vector2f point) const
		{
			return
				point.X >= Position.X &&
				point.Y >= Position.Y &&
				point.X <= Position.X + Size.X &&
				point.Y <= Position.Y + Size.Y;
		}
	};

	enum class HorizontalTextAlignment
	{
		Left,
		Center,
		Right
	};

	enum class VerticalTextAlignment
	{
		Top,
		Center,
		Bottom
	};

	struct TextStyle
	{
		std::wstring FontFamily = L"Segoe UI";
		float FontSizePixels = 24.0f;
		Color TextColor = Color::WHITE;
		bool IsBold = false;
		HorizontalTextAlignment HorizontalAlignment =
			HorizontalTextAlignment::Left;
		VerticalTextAlignment VerticalAlignment =
			VerticalTextAlignment::Top;
	};
}
