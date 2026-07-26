#pragma once

#include <array> 

namespace Slate
{
	struct Color
	{
		Color() = default;
		Color(unsigned short r, unsigned short g, unsigned short b, unsigned short a = 255)
			: R(r), G(g), B(b), A(a) {}

		void FromFloats(float r, float g, float b, float a = 1.0f)
		{
			R = static_cast<unsigned short>(r * 255);
			G = static_cast<unsigned short>(g * 255);
			B = static_cast<unsigned short>(b * 255);
			A = static_cast<unsigned short>(a * 255);
		}
			

		unsigned short R{ 255 };
		unsigned short G{ 255 };
		unsigned short B{ 255 };
		unsigned short A{ 255 };

		std::array<unsigned short, 4> GetUIntArray() const
		{
			return {
				R,
				G,
				B,
				A
			};
		}

		std::array<float, 4> GetFloatArray() const
		{
			return {
				R / 255.0f,
				G / 255.0f,
				B / 255.0f,
				A / 255.0f
			};
		}

		// Predefined Colors
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

