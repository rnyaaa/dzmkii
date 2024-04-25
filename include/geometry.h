#include <stdint.h>
#include <vector>
#include <optional>
#include <cmath>
#include "logger.h"
#ifndef _GEOMETRY_H
#define _GEOMETRY_H

template <typename T>
struct v2
{
    union { T x; T u; };
    union { T y; T v; };

    inline double magnitude()
    {
        return std::sqrt(x * x + y * y);
    }

    inline double distanceFrom(const v2 &other) const
    {
        return (*this - other).magnitude();
    }

    bool operator<(const v2& other) const {
        // Compare x values first
        if (x < other.x) return true;
        if (x > other.x) return false;
        // If x values are equal, compare y values
        return y < other.y;
    }

    v2 operator+(const v2& other) const 
    {
        return v2 { this->x + other.x, this->y + other.y, };
    }

    v2 operator-(const v2& other) const 
    {
        return v2 { this->x - other.x, this->y - other.y, };
    }

    v2 operator/(const T& divisor)
    {
        return v2 { this->x / divisor, this->y / divisor };
    }

    v2 operator*(const T& factor)
    {
        return v2 { this->x * factor, this->y * factor };
    }
};

template <typename T>
struct v3
{
    union { T x; T r; };
    union { T y; T g; };
    union { T z; T b; };

    inline double magnitude()
    {
        std::sqrt(x * x + y * y + z * z);
    }

    inline double distanceFrom(const v3 &other) const
    {
        return (*this - other).magnitude();
    }

    bool operator<(const v3& other) const 
    {
        // Compare x values first
        if (x < other.x) return true;
        if (x > other.x) return false;
        // If x values are equal, compare y values
        return z < other.z;
    }

    v3 operator+(const v3& other) const 
    {
        return v3 {
            this->x + other.x,
            this->y + other.y,
            this->z + other.z,
        };
    }

    v3 operator-(const v3& other) const 
    {
        return v3 {
            this->x - other.x,
            this->y - other.y,
            this->z - other.z,
        };
    }    

    v3 operator/(const T& divisor)
    {
        return v3 { this->x / divisor, this->y / divisor, this->z / divisor };
    }

    v3 operator*(const T& factor)
    {
        return v3 { this->x * factor, this->y * factor, this->z * factor };
    }
};

typedef v2<uint32_t> v2u;
typedef v3<uint32_t> v3u;
typedef v2<int32_t>  v2i;
typedef v3<int32_t>  v3i;
typedef v2<float>    v2f;
typedef v3<float>    v3f;
typedef v2<double>   v2d;
typedef v3<double>   v3d;

template <typename T> struct AARect2D;
template <typename T> struct Circle2D;

typedef AARect2D<uint32_t> AArect2u;
typedef AARect2D<int32_t>  AArect2i;
typedef AARect2D<float>    AArect2f;
typedef AARect2D<double>   AArect2d;

typedef Circle2D<uint32_t> circ2u;
typedef Circle2D<int32_t>  circ2i;
typedef Circle2D<float>    circ2f;
typedef Circle2D<double>   circ2d;

// TODO: Maybe provide conversions between different template types;

template <typename T>
struct AARect2D
{
    v2<T> pos;
    v2<T> dim;
    
    bool collidesWith(Circle2D<T> circle)
    {
        v2<T> rect_center {
            pos.x + dim.x / 2,
            pos.y + dim.y / 2 
        };

        if (rect_center.distanceFrom(circle.pos) <= circle.radius)
            return true;

        v2<T> c_to_r = 
            (rect_center - circle.pos) / (rect_center - circle.pos).magnitude();

        v2<T> closest_to_center = (c_to_r * circle.radius + circle.pos);

        if (closest_to_center.x >= pos.x && closest_to_center.x <= pos.x + dim.x
            && closest_to_center.y >= pos.y && closest_to_center.y <= pos.y + dim.y)
        {
            return true;
        }

        return false;
    }

    std::optional<AARect2D<T>> getIntersectingRect(const AARect2D &other) const
    {
        v2<T> tl = { 
            std::max(
                    std::min(this->pos.x, other.pos.x + other.dim.x), 
                    std::min(this->pos.x + this->dim.x, other.pos.x)
                ),
            std::max(
                    std::min(this->pos.y, other.pos.y + other.dim.y), 
                    std::min(this->pos.y + this->dim.y, other.pos.y)
                )
        };
        v2<T> br = { 
            std::min(
                    std::max(this->pos.x, other.pos.x + other.dim.x), 
                    std::max(this->pos.x + this->dim.x, other.pos.x)
                ),
            std::min(
                    std::max(this->pos.y, other.pos.y + other.dim.y), 
                    std::max(this->pos.y + this->dim.y, other.pos.y)
                )
        };

        AARect2D<T> ret = { tl, br - tl };

        // Check if degenerate
        if (ret.dim.x <= 0 || ret.dim.y <= 0)
        {
            return std::nullopt;
        }

        return ret;
    }
};

template <typename T>
struct Circle2D
{
    v2<T> pos;
    T radius;

    void collidesWith(AARect2D<T> rect)
    {
        return rect.collidesWith(this);
    }
};


std::vector<v2f> drawCircle(v2f origin, int n_points, int radius);

#endif // _GEOMETRY_H
