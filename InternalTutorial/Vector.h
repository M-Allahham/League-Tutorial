#pragma once
#include <string>
#include <DirectxMath.h>
#define _USE_MATH_DEFINES
#include <math.h>
#define NULLVECTOR4 Vector4(0, 0, 0, 0)
#define NULLVECTOR3 Vector3(0, 0, 0)
#define NULLVECTOR2 Vector2(0, 0)

struct Vector4
{
	float x, y, z, w;

	Vector4()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
		w = 0.0f;
	}

	Vector4(const float xa, const float ya, const float za, const float wa)
	{
		x = xa;
		y = ya;
		z = za;
		w = wa;
	}

	Vector4 operator +(const float A) const
	{
		return Vector4(A + x, A + y, A + z, A + w);
	}

	Vector4 operator +(const Vector4& A) const
	{
		return Vector4(A.x + x, A.y + y, A.z + z, A.w + w);
	}

	Vector4 operator *(const float A) const
	{
		return Vector4(A * x, A * y, A * z, A * w);
	}

	Vector4 operator *(const Vector4& A) const
	{
		return Vector4(A.x * x, A.y * y, A.z * z, A.w * w);
	}

	Vector4 operator -(const float A) const
	{
		return Vector4(A * x, A * y, A * z, A * w);
	}

	Vector4 operator -(const Vector4& A) const
	{
		return Vector4(A.x - x, A.y - y, A.z - z, A.w - w);
	}

	Vector4 operator /(const float A) const
	{
		return Vector4(A / x, A / y, A / z, A / w);
	}

	Vector4 operator /(const Vector4& A) const
	{
		return Vector4(A.x / x, A.y / y, A.z / z, A.w / w);
	}

	float DotProduct(const Vector4& vec) const
	{
		return x * vec.x + y * vec.y + z * vec.z + w * vec.w;
	}

	float LengthSquared() const
	{
		return x * x + y * y + z * z + w * w;
	}

	float Length() const
	{
		return static_cast<float>(sqrtf(LengthSquared()));
	}

	Vector4 PerpendicularTo() const
	{
		return Vector4(z, w, -x, -y);
	}

	Vector4 Normalize()
	{
		const float length = this->Length();
		if (length != 0.0f)
		{
			const float inv = 1.0f / length;
			x *= inv;
			y *= inv;
			z *= inv;
			w *= inv;
		}
		return Vector4(x, y, z, w);
	}

	float DistanceTo(const Vector4& A) const
	{
		const float out = sqrtf(powf(A.x - x, 2) + powf(A.y - y, 2) + powf(A.z - z, 2) + powf(A.w - w, 2));
		return out < 0 ? out * -1 : out;
	}

	operator std::string() const
	{
		return std::string(
			"[x: " + std::to_string(x) + " y: " + std::to_string(y) + " z: " + std::to_string(z) + " W: " +
			std::to_string(w) + "]");
	}
}typedef Vector4;


struct Vector2
{
	float x, y;

	Vector2()
	{
		x = 0.0f;
		y = 0.0f;
	}

	Vector2(const float xa, const float ya)
	{
		x = xa;
		y = ya;
	}

	/// <summary> 
	  /// Returns a position that runs from the source position to the target position with the length of the range specified.
	  /// </summary>
	inline Vector2 Extend(Vector2 target, float range)
	{
		return *this + (target - *this).Normalize() * range;
	}

	static float RadianToDegree(double angle)
	{
		return (float)(angle * (180.0 / M_PI));
	}

	static bool Close(float a, float b, float eps)
	{
		if (abs(eps) < FLT_EPSILON)
		{
			eps = (float)1e-9;
		}
		return abs(a - b) <= eps;
	}

	inline float Polar()
	{
		if (Close(this->x, 0, 0))
		{
			if (this->y > 0)
			{
				return 90;
			}
			return this->y < 0 ? 270 : 0;
		}

		auto theta = RadianToDegree(atan((this->y) / this->x));
		if (this->x < 0)
		{
			theta = theta + 180;
		}
		if (theta < 0)
		{
			theta = theta + 360;
		}
		return theta;
	}

	inline float AngleBetween(Vector2 p2)
	{
		auto theta = this->Polar() - p2.Polar();
		if (theta < 0)
		{
			theta = theta + 360;
		}
		if (theta > 180)
		{
			theta = 360 - theta;
		}
		return theta;
	}

	inline Vector2 Rotated(float angle) const
	{
		auto c = cos(angle);
		auto s = sin(angle);

		return Vector2((float)(this->x * c - this->y * s), (float)(this->y * c + this->x * s));
	}


	Vector2 operator +(const float A) const
	{
		return Vector2(A + x, A + y);
	}

	Vector2 operator +(const Vector2& A) const
	{
		return Vector2(A.x + x, A.y + y);
	}

	Vector2 operator *(const float A) const
	{
		return Vector2(A * x, A * y);
	}

	Vector2 operator *(const Vector2& A) const
	{
		return Vector2(A.x * x, A.y * y);
	}

	Vector2 operator -(const float A) const
	{
		return Vector2(A * x, A * y);
	}

	Vector2 operator -(const Vector2& A) const
	{
		return Vector2(A.x - x, A.y - y);
	}

	Vector2 operator /(const float A) const
	{
		return Vector2(A / x, A / y);
	}

	Vector2 operator /(const Vector2& A) const
	{
		return Vector2(A.x / x, A.y / y);
	}

	float DotProduct(const Vector2& vec) const
	{
		return x * vec.x + y * vec.y;
	}

	float LengthSquared() const
	{
		return x * x + y * y;
	}

	float Length() const
	{
		return static_cast<float>(sqrtf(LengthSquared()));
	}

	Vector2 PerpendicularTo() const
	{
		return Vector2(y, -x);
	}

	Vector2 Normalize()
	{
		const float length = this->Length();
		if (length != 0.0f)
		{
			const float inv = 1.0f / length;
			x *= inv;
			y *= inv;
		}
		return Vector2(x, y);
	}

	static Vector2 CoordinatesAtAngle(float angle, const float& distance)
	{
		static double pi{ 3.14159265358979323846 };

		angle = static_cast<float>(pi) * angle / 180.0f;
		const Vector2 point{ static_cast<float>(distance * cos(angle)), static_cast<float>(distance * sin(angle)) };

		return point;
	}

	bool IsValid() const
	{
		return this->Length() > 0;
	}

	float DistanceTo(const Vector2& A, bool squared = true) const
	{
		const float dist = powf(A.x - x, 2) + powf(A.y - y, 2);
		return squared ? sqrtf(dist) : dist;
	}

	operator std::string() const
	{
		return std::string("[x: " + std::to_string(x) + " y: " + std::to_string(y) + "]");
	}
}typedef Vector2;

struct Vector3
{
	float x, y, z;

	Vector3()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}

	Vector3(const float xa, const float ya, const float za)
	{
		x = xa;
		y = ya;
		z = za;
	}

	inline float Distance(Vector2 segmentStart, Vector2 segmentEnd, bool squared = false)
	{
		Vector2 point = this->To2D();
		auto a =
			abs((segmentEnd.y - segmentStart.y) * point.x - (segmentEnd.x - segmentStart.x) * point.y +
				segmentEnd.x * segmentStart.y - segmentEnd.y * segmentStart.x);
		return (squared ? a * a : a) / segmentStart.DistanceTo(segmentEnd, squared);
	}

	static float RadianToDegree(double angle)
	{
		return (float)(angle * (180.0 / M_PI));
	}

	static bool Close(float a, float b, float eps)
	{
		if (abs(eps) < FLT_EPSILON)
		{
			eps = (float)1e-9;
		}
		return abs(a - b) <= eps;
	}

	inline float Polar()
	{
		if (Close(this->x, 0, 0))
		{
			if (this->y > 0)
			{
				return 90;
			}
			return this->y < 0 ? 270 : 0;
		}

		auto theta = RadianToDegree(atan((this->y) / this->x));
		if (this->x < 0)
		{
			theta = theta + 180;
		}
		if (theta < 0)
		{
			theta = theta + 360;
		}
		return theta;
	}

	inline float AngleBetween(Vector3 p2)
	{
		auto theta = this->Polar() - p2.Polar();
		if (theta < 0)
		{
			theta = theta + 360;
		}
		if (theta > 180)
		{
			theta = 360 - theta;
		}
		return theta;
	}

	inline Vector3 Rotated(float angle) const
	{
		auto c = cos(angle);
		auto s = sin(angle);

		return Vector3((float)(this->x * c - this->y * s), (float)(this->y * c + this->x * s), (float)(this->z * c - this->z * s));
	}

	/// <summary> 
	  /// Returns a position that runs from the source position to the target position with the length of the range specified.
	  /// </summary>
	inline Vector3 Extend(Vector3 target, float range)
	{
		return *this + (target - *this).Normalize() * range;
	}

	Vector3 operator +(const float A) const
	{
		return Vector3(A + x, A + y, A + z);
	}

	Vector3 operator +(const Vector3& A) const
	{
		return Vector3(A.x + x, A.y + y, A.z + z);
	}


	Vector3 operator *(const float A) const
	{
		return Vector3(A * x, A * y, A * z);
	}

	bool operator ==(const Vector3& A) const
	{
		return x == A.x && y == A.y && z == A.z;
	}

	static Vector3 zero()
	{
		return Vector3{ 0,0,0 };
	}

	Vector3 operator *(const Vector3& A) const
	{
		return Vector3(A.x * x, A.y * y, A.z * z);
	}

	Vector3 operator -(const float A) const
	{
		return Vector3(A * x, A * y, A * z);
	}

	Vector3 operator -(const Vector3& A) const
	{
		return Vector3(A.x - x, A.y - y, A.z - z);
	}

	Vector3 operator /(const float A) const
	{
		return Vector3(A / x, A / y, A / z);
	}

	Vector3 operator /(const Vector3& A) const
	{
		return Vector3(A.x / x, A.y / y, A.z / z);
	}

	Vector3 CrossProduct(const Vector3& vec) const
	{
		return Vector3(y * vec.z - z * vec.y, z * vec.x - x * vec.z, x * vec.y - y * vec.x);
	}

	Vector3 Multiply(float a)
	{
		return Vector3(x * a, y * a, z * a);
	}

	inline Vector2 To2D() const
	{
		return Vector2(this->x, this->y);
	}

	float DotProduct(const Vector3& vec) const
	{
		return x * vec.x + y * vec.y + z * vec.z;
	}

	float LengthSquared() const
	{
		return x * x + y * y + z * z;
	}

	float LengthSquared2D() const
	{
		return x * x + z * z;
	}

	float Length() const
	{
		return static_cast<float>(sqrtf(LengthSquared()));
	}

	bool IsValid() const
	{
		return Length() > 0;
	}

	float Length2D() const
	{
		return static_cast<float>(sqrtf(LengthSquared2D()));
	}


	Vector3 PerpendicularTo() const
	{
		return Vector3(z, y, -x);
	}


	Vector3 Normalize()
	{
		const float length = this->Length2D();
		if (length != 0.0f)
		{
			const float inv = 1.0f / length;
			x *= inv;
			y *= inv;
			z *= inv;
		}
		return Vector3(x, y, z);
	}

	float DistanceTo(const Vector3& A, bool squared = true) const
	{
		const float dist = powf(A.x - x, 2) + powf(A.y - y, 2) + powf(A.z - z, 2);
		//	const float out = sqrtf(powf(A.x - x, 2) + powf(A.y - y, 2) + powf(A.z - z, 2));
		return squared ? sqrtf(dist) : dist;
	}

	operator std::string() const
	{
		return std::string("[x: " + std::to_string(x) + " y: " + std::to_string(y) + " z: " + std::to_string(z) + "]");
	}
}typedef Vector3;

inline Vector3 To3D(Vector2 vec)
{
	return Vector3(vec.x, vec.y, 0.f);
}