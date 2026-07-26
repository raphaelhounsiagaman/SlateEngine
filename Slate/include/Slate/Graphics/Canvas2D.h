#pragma once

#include "Color.h"

#include <string>

namespace Slate
{
	struct Rectangle2D
	{
		float X = 0.0f;
		float Y = 0.0f;
		float Width = 0.0f;
		float Height = 0.0f;

		bool Contains(float x, float y) const
		{
			return
				x >= X &&
				y >= Y &&
				x <= X + Width &&
				y <= Y + Height;
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
