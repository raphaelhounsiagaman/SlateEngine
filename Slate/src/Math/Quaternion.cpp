#include "Slate/Math/Quaternion.h"

#include <cmath>
#include <stdexcept>

namespace Slate
{

	Quaternion Quaternion::FromAxisAngle(
		const Vector3f& axis, 
		float angleRadians
	)
	{

		const float axisLengthSquared =
			axis.X * axis.X +
			axis.Y * axis.Y +
			axis.Z * axis.Z;

		if (axisLengthSquared == 0.0f)
		{
			throw std::invalid_argument(
				"Cannot create a quaternion from a zero-length axis."
			);
		}

		const float inverseAxisLength =
			1.0f / std::sqrt(axisLengthSquared);

		const float halfAngle = angleRadians * 0.5f;
		const float sinHalfAngle = std::sin(halfAngle);


		return Quaternion
		{
			axis.X * inverseAxisLength * sinHalfAngle,
			axis.Y * inverseAxisLength * sinHalfAngle,
			axis.Z * inverseAxisLength * sinHalfAngle,
			std::cos(halfAngle)
		};
	}

	Quaternion Quaternion::Normalized() const
	{
		const float lengthSquared =
			x * x +
			y * y +
			z * z +
			w * w;

		if (lengthSquared == 0.0f)
		{
			throw std::invalid_argument(
				"Cannot normalize a zero-length quaternion."
			);
		}

		const float inverseLength = 1.0f / std::sqrt(lengthSquared);

		return Quaternion
		{
			x * inverseLength,
			y * inverseLength,
			z * inverseLength,
			w
		};
	}

	Quaternion Quaternion::operator*(const Quaternion& other) const
	{
		return Quaternion
		{
			w * other.x + x * other.w + y * other.z - z * other.y,
			w * other.y - x * other.z + y * other.w + z * other.x,
			w * other.z + x * other.y - y * other.x + z * other.w,
			w * other.w - x * other.x - y * other.y - z * other.z
		};
	}

}



