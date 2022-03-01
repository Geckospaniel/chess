#ifndef VECTOR2_HEADER
#define VECTOR2_HEADER

#include <cstddef>

template <typename T>
struct Vector2
{
	T x;
	T y;

	Vector2() : x(0), y(0) {}
	Vector2(T x, T y) : x(x), y(y) {}

	bool operator!=(const Vector2 <T>& rhs) const { return x != rhs.x || y != rhs.y; }
	bool operator==(const Vector2 <T>& rhs) const { return x == rhs.x && y == rhs.y; }
	bool operator<=(const Vector2 <T>& rhs) const { return x <= rhs.x && y <= rhs.y; }
	bool operator>=(const Vector2 <T>& rhs) const { return x >= rhs.x && y >= rhs.y; }
	bool operator<(const Vector2 <T>& rhs) const { return x < rhs.x && y < rhs.y; }
	bool operator>(const Vector2 <T>& rhs) const { return x > rhs.x && y > rhs.y; }

	template <typename V> Vector2 <T> operator/(const Vector2 <V>& rhs) const { return Vector2 <T> (x / rhs.x, y / rhs.y); }
	template <typename V> Vector2 <T> operator*(const Vector2 <V>& rhs) const { return Vector2 <T> (x * rhs.x, y * rhs.y); }
	template <typename V> Vector2 <T> operator+(const Vector2 <V>& rhs) const { return Vector2 <T> (x + rhs.x, y + rhs.y); }
	template <typename V> Vector2 <T> operator-(const Vector2 <V>& rhs) const { return Vector2 <T> (x - rhs.x, y - rhs.y); }

	Vector2 <T> operator/(const T rhs) const { return Vector2 <T> (x / rhs, y / rhs); }
	Vector2 <T> operator*(const T rhs) const { return Vector2 <T> (x * rhs, y * rhs); }

	template <typename V> Vector2 <T> operator*=(const Vector2 <V>& rhs) { x *= rhs.x; y *= rhs.y; return *this; }
	template <typename V> Vector2 <T> operator/=(const Vector2 <V>& rhs) { x /= rhs.x; y /= rhs.y; return *this; }
	template <typename V> Vector2 <T> operator+=(const Vector2 <V>& rhs) { x += rhs.x; y += rhs.y; return *this; }
	template <typename V> Vector2 <T> operator-=(const Vector2 <V>& rhs) { x /= rhs.x; y /= rhs.y; return *this; }

	template <typename N>
	Vector2 <N> as() const { return Vector2 <N> (static_cast <N> (x), static_cast <N> (y)); }
};

typedef Vector2 <float> Vec2;
typedef Vector2 <size_t> Vec2s;
typedef Vector2 <int> Vec2i;

#endif
