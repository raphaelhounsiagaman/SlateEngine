#pragma once

#include "Transform.h"

namespace Slate
{

	struct Matrix4x4
	{
		float Values[4][4]{ 0 };

		static Matrix4x4 Identity();
		static Matrix4x4 Translation(const Vector3f& translation);
		static Matrix4x4 Scale(const Vector3f& scale);
		static Matrix4x4 Rotation(const Quaternion& rotation);

		static Matrix4x4 World(const Transform3D& transform);
		static Matrix4x4 View(const Transform3D& cameraTransform);

		static Matrix4x4 Perspective(
			float verticalFieldOfView,
			float aspectRatio,
			float nearPlaneMetres,
			float farPlaneMetres
		);

		Matrix4x4 operator*(const Matrix4x4& other) const;

		const float* Data() const {	return &Values[0][0]; }
	};

}
