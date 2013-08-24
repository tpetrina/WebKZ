#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

enum
{
	YAW = 0,
	PITCH,
	ROLL,
};

class vec3_t
{
public:
	float x, y, z;

	vec3_t() : x(0), y(0), z(0) { }
	vec3_t(const vec3_t& v) : x(v.x), y(v.y), z(v.z) { }
	vec3_t(const float& s) : x(s), y(s), z(s) { }
	vec3_t(const float& x, const float& y, const float& z) : x(x), y(y), z(z) { }
	vec3_t(const float* p) : x(p[0]), y(p[1]), z(p[2]) { }

	float& operator[](int i) { return (&x)[i]; }
	const float& operator[](int i) const { return (&x)[i]; }

	void Clear() {x = 0; y = 0; z = 0;}

	vec3_t& operator=(const vec3_t& v) { x = v.x; y = v.y; z = v.z; return *this; }
	vec3_t& operator+=(const vec3_t& v) { x += v.x; y += v.y; z += v.z; return *this; }
	vec3_t& operator-=(const vec3_t& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
	vec3_t& operator*=(const float& s) { x *= s; y *= s; z *= s; return *this; }
	vec3_t& operator*=(const vec3_t& v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
	vec3_t& operator/=(const float& s) { x /= s; y /= s; z /= s; return *this; }
	vec3_t& operator/=(const vec3_t& v) { x /= v.x; y /= v.y; z /= v.z; return *this; }

	operator vec3_t*() { return (vec3_t*)&x; }
	operator const vec3_t*() const { return this; }
};

inline vec3_t operator+(const vec3_t& v) { return v; }
inline vec3_t operator+(const vec3_t& v1, const vec3_t& v2) { return vec3_t(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z); }
inline vec3_t operator-(const vec3_t& v) { return vec3_t(-v.x, -v.y, -v.z); }
inline vec3_t operator-(const vec3_t& v1, const vec3_t& v2) { return vec3_t(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z); }
inline vec3_t operator*(const float& s, const vec3_t& v) { return vec3_t(s * v.x, s * v.y, s * v.z); }
inline vec3_t operator*(const vec3_t& v, const float& s) { return vec3_t(v.x * s, v.y * s, v.z * s); }
inline vec3_t operator*(const vec3_t& v1, const vec3_t& v2) { return vec3_t(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z); }
inline vec3_t operator/(const float& s, const vec3_t& v) { return vec3_t(s / v.x, s / v.y, s / v.z); }
inline vec3_t operator/(const vec3_t& v, const float& s) { return vec3_t(v.x / s, v.y / s, v.z / s); }
inline vec3_t operator/(const vec3_t& v1, const vec3_t& v2) { return vec3_t(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z); }

inline bool operator==(const vec3_t& v1, const vec3_t& v2) { return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z; }
inline bool operator!=(const vec3_t& v1, const vec3_t& v2) { return v1.x != v2.x || v1.y != v2.y || v1.z != v2.z; }
inline bool operator<(const vec3_t& v1, const vec3_t& v2) { return v1.x < v2.x && v1.y < v2.y && v1.z < v2.z; }
inline bool operator<=(const vec3_t& v1, const vec3_t& v2) { return v1.x <= v2.x && v1.y <= v2.y && v1.z <= v2.z; }
inline bool operator>(const vec3_t& v1, const vec3_t& v2) { return v1.x > v2.x && v1.y > v2.y && v1.z > v2.z; }
inline bool operator>=(const vec3_t& v1, const vec3_t& v2) { return v1.x >= v2.x && v1.y >= v2.y && v1.z >= v2.z; }

inline float DotProduct(const vec3_t& v1, const vec3_t& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }
inline float Length(const vec3_t& v) { return sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }
inline float LengthSqr(const vec3_t& v) { return v.x * v.x + v.y * v.y + v.z * v.z; }
inline float Distance(const vec3_t& v1, const vec3_t& v2) { return Length(v1-v2); }
inline float DistanceSqr(const vec3_t& v1, const vec3_t& v2) { return LengthSqr(v1-v2); }
