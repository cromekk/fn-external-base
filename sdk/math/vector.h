#pragma once

#include <cmath>

struct Vector2
{
    double x{ 0.0 };
    double y{ 0.0 };

    constexpr Vector2( ) noexcept = default;

    constexpr Vector2( double x,double y ) noexcept
        : x( x ),y( y )
    {}
};

struct Vector3
{
    double x{ 0.0 };
    double y{ 0.0 };
    double z{ 0.0 };

    constexpr Vector3( ) noexcept = default;

    constexpr Vector3( double x,double y,double z ) noexcept
        : x( x ),y( y ),z( z )
    {}

    constexpr double dot( const Vector3& other ) const noexcept
    {
        return x * other.x + y * other.y + z * other.z;
    }

    double distance( const Vector3& other ) const noexcept
    {
        return std::hypot(
            other.x - x,
            other.y - y,
            other.z - z
        );
    }

    constexpr Vector3 operator-( const Vector3& other ) const noexcept
    {
        return {
            x - other.x,
            y - other.y,
            z - other.z
        };
    }
};