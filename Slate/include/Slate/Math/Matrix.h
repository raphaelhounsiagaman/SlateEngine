#pragma once

#include "Transform.h"

namespace Slate
{

	// Row-major matrix used with row vectors: position * world * view * projection.
	struct Matrix4x4
	{
		float Values[4][4]{};

		static Matrix4x4 Identity();
		static Matrix4x4 Translation(const Vector3f& translation);
		static Matrix4x4 Scale(const Vector3f& scale);
		static Matrix4x4 Rotation(const Quaternion& rotation);

		static Matrix4x4 World(const Transform3D& transform);
		static Matrix4x4 View(const Transform3D& cameraTransform);

		// Creates a left-handed projection matrix. The field of view is in radians.
		static Matrix4x4 Perspective(
			float verticalFieldOfView,
			float aspectRatio,
			float nearPlane,
			float farPlane
		);

		Matrix4x4 operator*(const Matrix4x4& other) const;

		const float* Data() const { return &Values[0][0]; }
	};

}
