#include "Slate/Math/Matrix.h"

#include <cmath>
#include <stdexcept>

namespace Slate
{
	Matrix4x4 Matrix4x4::Identity()
	{
		Matrix4x4 result{};

		result.Values[0][0] = 1.0f;
		result.Values[1][1] = 1.0f;
		result.Values[2][2] = 1.0f;
		result.Values[3][3] = 1.0f;

		return result;
	}

	Matrix4x4 Matrix4x4::Translation(const Vector3f& translation)
	{
		Matrix4x4 result = Identity();

		result.Values[3][0] = translation.X;
		result.Values[3][1] = translation.Y;
		result.Values[3][2] = translation.Z;

		return result;
	}

	Matrix4x4 Matrix4x4::Scale(const Vector3f& scale)
	{
		Matrix4x4 result{};

		result.Values[0][0] = scale.X;
		result.Values[1][1] = scale.Y;
		result.Values[2][2] = scale.Z;
		result.Values[3][3] = 1.0f;

		return result;
	}

	Matrix4x4 Matrix4x4::Rotation(const Quaternion& rotation)
	{
		const float lengthSquared =
			rotation.x * rotation.x +
			rotation.y * rotation.y +
			rotation.z * rotation.z +
			rotation.w * rotation.w;

		if (lengthSquared == 0.0f)
		{
			return Identity();
		}

		const float inverseLength = 1.0f / std::sqrt(lengthSquared);
		const float x = rotation.x * inverseLength;
		const float y = rotation.y * inverseLength;
		const float z = rotation.z * inverseLength;
		const float w = rotation.w * inverseLength;

		Matrix4x4 result = Identity();

		result.Values[0][0] = 1.0f - 2.0f * (y * y + z * z);
		result.Values[0][1] = 2.0f * (x * y + z * w);
		result.Values[0][2] = 2.0f * (x * z - y * w);

		result.Values[1][0] = 2.0f * (x * y - z * w);
		result.Values[1][1] = 1.0f - 2.0f * (x * x + z * z);
		result.Values[1][2] = 2.0f * (y * z + x * w);

		result.Values[2][0] = 2.0f * (x * z + y * w);
		result.Values[2][1] = 2.0f * (y * z - x * w);
		result.Values[2][2] = 1.0f - 2.0f * (x * x + y * y);

		return result;
	}

	Matrix4x4 Matrix4x4::World(const Transform3D& transform)
	{
		return
			Scale(transform.Scale) *
			Rotation(transform.Rotation) *
			Translation(transform.Position);
	}

	Matrix4x4 Matrix4x4::View(const Transform3D& cameraTransform)
	{
		const Quaternion inverseRotation
		{
			-cameraTransform.Rotation.x,
			-cameraTransform.Rotation.y,
			-cameraTransform.Rotation.z,
			cameraTransform.Rotation.w
		};

		const Vector3f inversePosition
		{
			-cameraTransform.Position.X,
			-cameraTransform.Position.Y,
			-cameraTransform.Position.Z
		};

		return Translation(inversePosition) * Rotation(inverseRotation);
	}

	Matrix4x4 Matrix4x4::Perspective(
		float verticalFieldOfView,
		float aspectRatio,
		float nearPlane,
		float farPlane)
	{
		if (verticalFieldOfView <= 0.0f || verticalFieldOfView >= 3.1415926535f)
		{
			throw std::invalid_argument("The vertical field of view must be between zero and pi radians.");
		}

		if (aspectRatio <= 0.0f)
		{
			throw std::invalid_argument("The aspect ratio must be greater than zero.");
		}

		if (nearPlane <= 0.0f || farPlane <= nearPlane)
		{
			throw std::invalid_argument("The clipping planes must satisfy 0 < nearPlane < farPlane.");
		}

		const float verticalScale = 1.0f / std::tan(verticalFieldOfView * 0.5f);
		const float depthScale = farPlane / (farPlane - nearPlane);

		Matrix4x4 result{};

		result.Values[0][0] = verticalScale / aspectRatio;
		result.Values[1][1] = verticalScale;
		result.Values[2][2] = depthScale;
		result.Values[2][3] = 1.0f;
		result.Values[3][2] = -nearPlane * depthScale;

		return result;
	}

	Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other) const
	{
		Matrix4x4 result{};

		for (unsigned int row = 0; row < 4; row++)
		{
			for (unsigned int column = 0; column < 4; column++)
			{
				for (unsigned int element = 0; element < 4; element++)
				{
					result.Values[row][column] +=
						Values[row][element] *
						other.Values[element][column];
				}
			}
		}

		return result;
	}

}
