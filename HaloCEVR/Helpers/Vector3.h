#pragma once

struct Vector3
{
	float x;
	float y;
	float z;

	Vector3 Cross(const Vector3& v) const
	{
		Vector3 cross_product;
		cross_product.x = y * v.z - z * v.y;
		cross_product.y = z * v.x - x * v.z;
		cross_product.z = x * v.y - y * v.x;
		return cross_product;
	}

	float Dot(const Vector3& v) const
	{
		return x * v.x + y * v.y + z * v.z;
	}

	Vector3 operator+(const Vector3& v) const
	{
		return { x + v.x, y + v.y, z + v.z };
	}

	Vector3 operator-(const Vector3& v) const
	{
		return { x - v.x, y - v.y, z - v.z };
	}

	Vector3 operator- ()
	{
		return { -x, -y, -z };
	}

	Vector3 operator*(float scalar) const
	{
		return { x * scalar, y * scalar, z * scalar };
	}

	Vector3 operator/(float scalar) const
	{
		return { x / scalar, y / scalar, z / scalar };
	}

	Vector3& operator+=(const Vector3& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	Vector3& operator-=(const Vector3& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}
};
static_assert(sizeof(Vector3) == 0xc);