#include <stdint.h>
#include <vector>
#include <optional>
#include <cmath>
#include "logger.h"
#ifndef _GEOMETRY_H
#define _GEOMETRY_H

template <typename T>
T clamp(T x, T min, T max)
{
    if (x < min) x = min;
    else if (x > max) x = max;
    return x;
}

template <typename T>
struct v2
{
    union { T x; T u; };
    union { T y; T v; };

    inline T magnitudeSq() const
    {
        return x * x + y * y;
    }

    inline T magnitude() const
    {
        return std::sqrt(this->magnitudeSq());
    }

    inline T distanceSqFrom(const v2 &other) const
    {
        return (*this - other).magnitudeSq();
    }

    inline T distanceFrom(const v2 &other) const
    {
        return (*this - other).magnitude();
    }

    inline T dot(const v2 &other) const
    {
        return this->x * other.x + this->y * other.y;
    }

    inline v2 normalized() const
    {
        return (*this) / this->magnitude();
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

    v2 operator/(const T& divisor) const
    {
        return v2 { this->x / divisor, this->y / divisor };
    }

    v2 operator*(const T& factor) const
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

    inline double magnitudeSq()
    {
        return x * x + y * y + z * z;
    }

    inline double magnitude()
    {
        return std::sqrt(this->magnitudeSq());
    }

    inline double distanceSqFrom(const v3 &other) const
    {
        return (*this - other).magnitudeSq();
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

template <typename T>
struct v4
{
    union { T x; T r; };
    union { T y; T g; };
    union { T z; T b; };
    union { T w; T a; };
};

typedef v2<uint32_t> v2u;
typedef v3<uint32_t> v3u;
typedef v4<uint32_t> v4u;
typedef v2<int32_t>  v2i;
typedef v3<int32_t>  v3i;
typedef v4<int32_t>  v4i;
typedef v2<float>    v2f;
typedef v3<float>    v3f;
typedef v4<float>    v4f;
typedef v2<double>   v2d;
typedef v3<double>   v3d;
typedef v4<double>   v4d;


template <typename T> struct OBRect2D;
template <typename T> struct AARect2D;
template <typename T> struct Circle2D;

typedef OBRect2D<uint32_t> OBrect2u;
typedef OBRect2D<int32_t>  OBrect2i;
typedef OBRect2D<float>    OBrect2f;
typedef OBRect2D<double>   OBrect2d;

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
struct OBRect2D
{
    v2<T> pos;      
    v2<T> dim;   
    T rotation;  

    v2<T> rotatePoint(const v2<T> &point, T angle) const
    {
        T s = std::sin(angle);
        T c = std::cos(angle);

        // Translate point back to origin
        v2<T> p = point - v2<T>{pos.x + dim.x / 2, pos.y + dim.y/2};

        // Rotate point
        v2<T> rotated{
            p.x * c + p.y * s,
            p.x * s - p.y * c
        };

        // Translate point back to its original location
        return rotated + pos;
    }

    std::array<v2<T>, 4> getVertices() const
    {
    //      // Vertices assuming no rotation, centered around pos
    //  std::array<v2<T>, 4> vertices = {
    //      v2<T>{pos.x - dim.x / 2, pos.y - dim.y / 2}, // Bottom-left
    //      v2<T>{pos.x - dim.x / 2, pos.y + dim.y / 2}, // Top-left
    //      v2<T>{pos.x + dim.x / 2, pos.y + dim.y / 2}, // Top-right
    //      v2<T>{pos.x + dim.x / 2, pos.y - dim.y / 2}  // Bottom-right
    //  };

    //  // Rotate all vertices
    //  for (auto &vertex : vertices)
    //  {
    //      vertex = rotatePoint(vertex, rotation);
    //  }

    //  return vertices;
        std::array<v2<T>,5> vertices = {
            v2<T>{pos.x, pos.y}, 
           v2<T>{pos.x, pos.y + dim.y}, 
            v2<T>{pos.x + dim.x, pos.y + dim.y}, 
            v2<T>{pos.x + dim.x, pos.y},
            v2<T>{pos.x + (dim.x /2), pos.y + (dim.y/2)}
        };

        for (auto &vertex : vertices)
        {
            vertex = rotatePoint(vertex, rotation);
        }

        v2<T> to_center = v2<T>{pos.x + (dim.x / 2), pos.y + (dim.y / 2)} - vertices[4];
        
        std::array<v2<T>, 4> fixed_vertices = {
            vertices[0] + to_center,
            vertices[1] + to_center,
            vertices[2] + to_center,
            vertices[3] + to_center
        };
        return fixed_vertices;
    }

    bool containsPoint(const v2<T> &point) const
    {
        v2<T> localPoint = rotatePoint(point, -rotation);

        v2<T> halfDim = dim / 2;

        return (localPoint.x > pos.x - halfDim.x && localPoint.x < pos.x + halfDim.x &&
                localPoint.y > pos.y - halfDim.y && localPoint.y < pos.y + halfDim.y);
    }


    bool collidesWith(Circle2D<T> circle) const
    {
        v2<T> rotated_circ_pos = rotatePoint(circle.pos, -rotation);
        Circle2D<T> rotated_circle = {
            rotated_circ_pos,
            circle.radius
        };

        v2<T> rect_min = pos;  
        v2<T> rect_max = pos + dim;

        v2<T> closest_point_on_rect = {
            std::max(rect_min.x, std::min(rotated_circle.pos.x, rect_max.x)),
            std::max(rect_min.y, std::min(rotated_circle.pos.y, rect_max.y))
        };

        T distance_to_circle = closest_point_on_rect.distanceFrom(rotated_circle.pos);

        return distance_to_circle <= rotated_circle.radius;
    }

    std::optional<OBRect2D<T>> getIntersectingRect(const OBRect2D &other) const
    {
        return std::nullopt;  // Return optional if no collision
    }
};

template<typename T>
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

    std::array<v2<T>, 4> getVertices() const
    {
        return {
            v2<T> {pos.x, pos.y},
            v2<T> {pos.x, pos.y + dim.y},
            v2<T> {pos.x + dim.x, pos.y + dim.y},
            v2<T> {pos.x + dim.x, pos.y}
        };
    }

    bool containsPoint(const v2<T> &other) const
    {
        return other.x > this->pos.x && other.x < this->pos.x + dim.x 
            && other.y > this->pos.u && other.y < this->pos.y + dim.y;
    }
};

template <typename T>
struct LineSegment2D
{
    v2<T> p1, p2;

    void print()
    {
        Log::verbose("%f %f - %f %f", p1.x, p1.y, p2.x, p2.y);
    }

    T collidesWith(Circle2D<T> circ) const
    {
           // Step 1: Get the direction of the line segment
        v2<T> d = p2 - p1; // Direction vector of the line
        v2<T> f = p1 - circ.pos; // Vector from circle center to p1

        T a = d.dot(d);
        T b = 2 * f.dot(d);
        T c = f.dot(f) - (circ.radius * circ.radius);

        // Step 2: Solve the quadratic equation for t (parametric distance along the line)
        T discriminant = b * b - 4 * a * c;

        if (discriminant < 0) {
            // No intersection
            return false;
        } else {
            // Discriminant is non-negative, so there are potential intersections
            discriminant = std::sqrt(discriminant);

            // Two possible solutions for t
            T t1 = (-b - discriminant) / (2 * a);
            T t2 = (-b + discriminant) / (2 * a);

            // Check if the intersection points lie on the line segment (0 <= t <= 1)
            if (t1 >= 0 && t1 <= 1) 
            {
                return std::min(t1, (T) 1.0 - t1);
            }
            if (t2 >= 0 && t2 <= 1) 
            {
                return std::min(t2, (T) 1.0 - t2);
            }

            return -INFINITY;
        }
    }
};

template <typename T>
struct Circle2D
{
    v2<T> pos;
    T radius;

    bool collidesWith(AARect2D<T> rect) const
    {
        return rect.collidesWith(*this);
    }

    bool collidesWith(OBRect2D<T> rect)
    {
        return rect.collidesWith(*this);
    }
};


std::vector<v2f> drawCircle(v2f origin, int n_points, int radius);

#endif // _GEOMETRY_H
