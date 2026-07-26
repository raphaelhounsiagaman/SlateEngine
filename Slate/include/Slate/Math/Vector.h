#pragma once

namespace Slate
{

	template<typename T>
	struct Vector2
	{
		T X = 0;
		T Y = 0;

		Vector2 operator+(const Vector2& other) const
		{
			return Vector2{ X + other.X, Y + other.Y };
		}

		Vector2 operator-(const Vector2& other) const
		{
			return Vector2{ X - other.X, Y - other.Y };
		}

		Vector2 operator*(const T scalar) const
		{
			return Vector2{ X * scalar, Y * scalar };
		}

		Vector2 operator/(const T scalar) const
		{
			return Vector2{ X / scalar, Y / scalar };
		}

	};

	using Vector2f = Vector2<float>;
	using Vector2i = Vector2<int>;



	template<typename T>
	struct Vector3
	{
		T X = 0;
		T Y = 0;
		T Z = 0;

		Vector3 operator+(const Vector3& other) const
		{
			return Vector3{ X + other.X, Y + other.Y, Z + other.Z };
		}

		Vector3 operator-(const Vector3& other) const
		{
			return Vector3{ X - other.X, Y - other.Y, Z - other.Z };
		}

		Vector3 operator*(const T scalar) const
		{
			return Vector3{ X * scalar, Y * scalar, Z * scalar };
		}

		Vector3 operator/(const T scalar) const
		{
			return Vector3{ X / scalar, Y / scalar, Z / scalar };
		}

	};

	using Vector3f = Vector3<float>;
	using Vector3i = Vector3<int>;

}