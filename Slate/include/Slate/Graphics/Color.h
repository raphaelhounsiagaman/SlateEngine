#pragma once

#include <array>
#include <cstdint>

namespace Slate
{
	struct Color
	{
		Color() = default;
		Color(
			uint8_t r,
			uint8_t g,
			uint8_t b,
			uint8_t a = 255)
			: R(r), G(g), B(b), A(a) {}

		uint8_t R{ 255 };
		uint8_t G{ 255 };
		uint8_t B{ 255 };
		uint8_t A{ 255 };

		std::array<float, 4> GetFloatArray() const
		{
			return {
				R / 255.0f,
				G / 255.0f,
				B / 255.0f,
				A / 255.0f
			};
		}

		static const Color WHITE;
		static const Color BLACK;

		static const Color RED;
		static const Color GREEN;
		static const Color BLUE;
		static const Color YELLOW;
		static const Color MAGENTA;
		static const Color CYAN;

	};
}

