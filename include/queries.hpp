#pragma once

#include "geometry.hpp"

#include <algorithm>
#include <optional>
#include <variant>

namespace geometry::queries
{
    template <class... Fs>
    struct Multilambda : Fs...
    {
        using Fs::operator()...;
    };

    struct PointToShapeDistanceVisitor
    {
        Point2D point;

        explicit PointToShapeDistanceVisitor(const Point2D& p) : point(p) {}

        double operator()(const Line& line) const
        {
            Point2D line_vec = line.end - line.start;
            Point2D point_vec = point - line.start;

            double line_length_sq = line_vec.Dot(line_vec);
            if (line_length_sq == 0)
            {
                return point.DistanceTo(line.start);
            }

            double t = std::clamp(point_vec.Dot(line_vec) / line_length_sq, 0.0, 1.0);
            Point2D projection = line.start + line_vec * t;

            return point.DistanceTo(projection);
        }

        double operator()(const Triangle& triangle) const
        {
            auto vertices = triangle.Vertices();
            double min_distance = std::numeric_limits<double>::max();

            for (size_t i = 0; i < vertices.size(); ++i)
            {
                Line edge{vertices[i], vertices[(i + 1) % vertices.size()]};
                min_distance = std::min(min_distance, (*this)(edge));
            }

            return min_distance;
        }

        double operator()(const Rectangle& rect) const
        {
            auto vertices = rect.Vertices();
            double min_distance = std::numeric_limits<double>::max();

            for (size_t i = 0; i < vertices.size(); ++i)
            {
                Line edge{vertices[i], vertices[(i + 1) % vertices.size()]};
                min_distance = std::min(min_distance, (*this)(edge));
            }

            return min_distance;
        }

        double operator()(const RegularPolygon& polygon) const
        {
            auto vertices = polygon.Vertices();
            double min_distance = std::numeric_limits<double>::max();

            for (size_t i = 0; i < vertices.size(); ++i)
            {
                Line edge{vertices[i], vertices[(i + 1) % vertices.size()]};
                min_distance = std::min(min_distance, (*this)(edge));
            }

            return min_distance;
        }

        double operator()(const Circle& circle) const
        {
            double centre_distance = point.DistanceTo(circle.centre_p);
            return std::max(0.0, centre_distance - circle.radius);
        }

        double operator()(const Polygon& polygon) const
        {
            double min_distance = std::numeric_limits<double>::max();
            for (const auto& p : polygon.Vertices())
            {
                min_distance = std::min(min_distance, point.DistanceTo(p));
            }
            return min_distance;
        }
    };

    struct PointInShapeVisitor
    {
        Point2D point;

        explicit PointInShapeVisitor(const Point2D& p) : 
            point(p)
        {}

        bool operator()(const Line& line) const
        {
            Point2D line_vec = line.end - line.start;
            Point2D point_vec = point - line.start;

            double cross = point_vec.Cross(line_vec);
            if (DblGreater{}(std::abs(cross), 0)) return false;

            double dot = point_vec.Dot(line_vec);
            double line_length_sq = line_vec.Dot(line_vec);

            return dot >= 0 && dot <= line_length_sq;
        }

        bool operator()(const Triangle& triangle) const
        {
            Point2D a = triangle.a;
            Point2D b = triangle.b;
            Point2D c = triangle.c;

            double sign1 = (point - a).Cross(b - a);
            double sign2 = (point - b).Cross(c - b);
            double sign3 = (point - c).Cross(a - c);

            bool has_neg = (sign1 < 0) || (sign2 < 0) || (sign3 < 0);
            bool has_pos = (sign1 > 0) || (sign2 > 0) || (sign3 > 0);

            return !(has_neg && has_pos);
        }

        bool operator()(const Rectangle& rect) const
        {
            return point.x >= rect.bottom_left.x && point.x <= rect.bottom_left.x + rect.width &&
                point.y >= rect.bottom_left.y && point.y <= rect.bottom_left.y + rect.height;
        }

        bool operator()(const RegularPolygon& polygon) const
        {
            std::vector<Point2D> vertices = polygon.Vertices();
            return point_in_polygon_ray_casting(point, vertices);
        }

        bool operator()(const Circle& circle) const
        {
            return point.DistanceTo(circle.centre_p) <= circle.radius;
        }

    private:
        bool point_in_polygon_ray_casting(const Point2D& p, 
            const std::vector<Point2D>& vertices) const
        {
            int intersections = 0;
            size_t n = vertices.size();

            for (size_t i = 0; i < n; ++i)
            {
                Point2D v1 = vertices[i];
                Point2D v2 = vertices[(i + 1) % n];

                if (((v1.y > p.y) != (v2.y > p.y)) && 
                    (p.x < (v2.x - v1.x) * (p.y - v1.y) / (v2.y - v1.y) + v1.x))
                {
                    ++intersections;
                }
            }

            return (intersections % 2) == 1;
        }
    };

    struct ShapeToShapeDistanceVisitor
    {
        std::optional<double> operator()(const Circle& c1, 
            const Circle& c2) const
        {
            double centre_dist = c1.centre_p.DistanceTo(c2.centre_p);
            return std::max(0.0, centre_dist - c1.radius - c2.radius);
        }

        std::optional<double> operator()(const Line& l1, 
            const Line& l2) const
        {
            std::vector<double> distances = 
                {
                    queries::PointToShapeDistanceVisitor{l1.start}(l2), 
                    queries::PointToShapeDistanceVisitor{l1.end}(l2), 
                    queries::PointToShapeDistanceVisitor{l2.start}(l1), 
                    queries::PointToShapeDistanceVisitor{l2.end}(l1)
                };
            
            return *std::ranges::min_element(distances);
        }

        std::optional<double> operator()(const Line& l1, 
            const Circle& c2) const
        {
            return std::max(PointToShapeDistanceVisitor{c2.centre_p}(l1) - 
                c2.radius, 0.0);
        }

        std::optional<double> operator()(const Circle& c1, 
            const Line& l2) const
        {
            return (*this)(l2, c1);
        }

        // fallback for all unsupported combinations
        template <typename Shape1, typename Shape2>
        std::optional<double> operator()(const Shape1&, 
            const Shape2&) const
        {
            return std::nullopt;
        }
    };

    /*
    * Функции-помощники
    */
    inline double DistanceToPoint(const Shape& shape, 
        const Point2D& point)
    {
        return std::visit(PointToShapeDistanceVisitor{point}, shape);
    }

    inline double DistanceToPoint(const Point2D& point, 
        const Shape& shape)
    {
        // Пользуемся симметрией евклидовой меры расстояния
        return DistanceToPoint(shape, point);
    }

    inline BoundingBox GetBoundingBox(const Shape& shape)
    {
        return std::visit([]<typename Shape>(const Shape& shape)
            {
                return shape.BoundBox();
            }, 
            shape);
    }

    inline double GetHeight(const Shape& shape)
    {
        return std::visit([]<typename Shape>(const Shape& shape) 
            {
                return shape.Height();
            }, 
            shape);
    }

    inline bool BoundingBoxesOverlap(const Shape& shape1, 
        const Shape& shape2)
    {
        BoundingBox bb1 = GetBoundingBox(shape1);
        BoundingBox bb2 = GetBoundingBox(shape2);
        return bb1.Overlaps(bb2);
    }

    template <typename T, typename = void>
    struct SupportsDistance
    {
        const static bool type = false;
    };

    template <typename T>
    struct SupportsDistance<T, 
        std::void_t<typename T::SupportsDistanceTag>>
    {
        const static bool type = true;
    };
    
    inline constexpr bool DistanceSupported(const Shape& shape)
    {
        return std::visit([]<typename Shape>(const Shape&)
            {
                return SupportsDistance<Shape>::type;
            }, 
            shape);
    }

    inline std::optional<double> DistanceBetweenShapes(const Shape& shape1, 
        const Shape& shape2)
    {
        return std::visit(ShapeToShapeDistanceVisitor{}, 
            shape1, shape2);
    }
}  // namespace geometry::queries