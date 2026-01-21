#pragma once
#include "geometry.hpp"

#include <format>

#include <vector>
#include <set>

#include <expected>

#include <stdexcept>

#include <algorithm>

namespace geometry::triangulation
{
    struct DelaunayTriangle
    {
        Point2D a, b, c;

        DelaunayTriangle(Point2D a, Point2D b, Point2D c) : a(a), b(b), c(c) {}

        bool ContainsPoint(const Point2D &p) const
        {
            Point2D center = Circumcenter();
            double radius = Circumradius();

            return !DblGreater{}(center.DistanceTo(p), radius);
        }

        Point2D Circumcenter() const
        {
            double d = 2 * (a.x * (b.y - c.y) + 
                b.x * (c.y - a.y) + c.x * (a.y - b.y));
            
            if (DblEquals{}(std::abs(d), 0))
            {
                return {(a.x + b.x + c.x) / 3, (a.y + b.y + c.y) / 3};
            }

            double ux = ((a.x * a.x + a.y * a.y) * (b.y - c.y) + 
                (b.x * b.x + b.y * b.y) * (c.y - a.y) +
                (c.x * c.x + c.y * c.y) * (a.y - b.y)) / d;

            double uy = ((a.x * a.x + a.y * a.y) * (c.x - b.x) + 
                (b.x * b.x + b.y * b.y) * (a.x - c.x) + 
                (c.x * c.x + c.y * c.y) * (b.x - a.x)) / d;

            return {ux, uy};
        }

        double Circumradius() const
        {
            Point2D center = Circumcenter();
            return center.DistanceTo(a);
        }

        bool SharesEdge(const DelaunayTriangle& other) const
        {
            std::vector<Point2D> these_points = {a, b, c};
            std::vector<Point2D> other_points = {other.a, other.b, other.c};

            int shared_count = 0;

            for (const Point2D& p1 : these_points)
            {
                for (const Point2D &p2 : other_points)
                {
                    if (DblEquals{}(std::abs(p1.x - p2.x), 0) && 
                        DblEquals{}(std::abs(p1.y - p2.y), 0))
                    {
                        ++shared_count;
                        break;
                    }
                }
            }

            return shared_count == 2;
        }

        std::vector<Point2D> vertices() const { return {a, b, c}; }
    };

    struct Edge
    {
        Point2D p1, p2;

        Edge(Point2D p1, Point2D p2) : p1(p1), p2(p2)
        {
            if (p1.x > p2.x || (p1.x == p2.x && p1.y > p2.y))
            {
                std::swap(this->p1, this->p2);
            }
        }

        bool operator<(const Edge& other) const
        {
            return DblLess{}(p1.x, other.p1.x) || 
                DblLess{}(p1.y, other.p1.y) || 
                DblLess{}(p2.x, other.p2.x) || 
                DblLess{}(p2.y, other.p2.y);
        }

        bool operator==(const Edge& other) const
        {
            return p1 == other.p1 && p2 == other.p2;
        }
    };

    inline std::expected<std::vector<DelaunayTriangle>, std::logic_error>
    DelaunayTriangulation(std::span<const Point2D> points) noexcept
    {
        if (points.size() < 3)
        {
            return std::unexpected(std::logic_error("At least "
                "three points are required for triangulation"));
        }

        auto [minX, maxX] =
            std::minmax_element(points.begin(), points.end(), 
                [](const Point2D &a, const Point2D &b) { return a.x < b.x; });
        auto [minY, maxY] =
            std::minmax_element(points.begin(), points.end(), 
                [](const Point2D &a, const Point2D &b) { return a.y < b.y; });

        double dx = maxX->x - minX->x;
        double dy = maxY->y - minY->y;
        double dmax = std::max(dx, dy);

        Point2D center = {(minX->x + maxX->x) / 2, (minY->y + maxY->y) / 2};

        Point2D super1 = {center.x - 20 * dmax, center.y - dmax};
        Point2D super2 = {center.x, center.y + 20 * dmax};
        Point2D super3 = {center.x + 20 * dmax, center.y - dmax};

        std::vector<DelaunayTriangle> triangles;
        triangles.emplace_back(super1, super2, super3);

        for (const Point2D& point : points)
        {
            std::vector<DelaunayTriangle> bad_triangles;
            std::set<Edge> polygon;

            for (const DelaunayTriangle& triangle : 
                triangles)
            {
                if (triangle.ContainsPoint(point))
                {
                    bad_triangles.push_back(triangle);

                    Edge e1{triangle.a, triangle.b};
                    Edge e2{triangle.b, triangle.c};
                    Edge e3{triangle.c, triangle.a};

                    if (!polygon.erase(e1)) polygon.insert(e1);
                    if (!polygon.erase(e2)) polygon.insert(e2);
                    if (!polygon.erase(e3)) polygon.insert(e3);
                }
            }

            std::erase_if(triangles, 
                [&bad_triangles](const DelaunayTriangle& t)
                {
                    return std::find_if(bad_triangles.begin(), bad_triangles.end(), 
                        [&t](const DelaunayTriangle &bad)
                        {
                            return DblEquals{}(std::abs(t.a.x - bad.a.x), 0) && 
                                DblEquals{}(std::abs(t.a.y - bad.a.y), 0) && 
                                DblEquals{}(std::abs(t.b.x - bad.b.x), 0) && 
                                DblEquals{}(std::abs(t.b.y - bad.b.y), 0) && 
                                DblEquals{}(std::abs(t.c.x - bad.c.x), 0) && 
                                DblEquals{}(std::abs(t.c.y - bad.c.y), 0);
                        }) != bad_triangles.end();
                });

            for (const Edge &edge : polygon)
            {
                triangles.emplace_back(edge.p1, edge.p2, point);
            }
        }

        std::erase_if(triangles, [&super1, &super2, &super3](const DelaunayTriangle& t)
            {
                return (t.a == super1) || (t.a == super2) || (t.a == super3) || 
                    (t.b == super1) || (t.b == super2) || (t.b == super3) || 
                    (t.c == super1) || (t.c == super2) || (t.c == super3);
            });

        return triangles;
    }
}  // namespace geometry::triangulation

template <>
struct std::formatter<geometry::triangulation::DelaunayTriangle>
{
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::triangulation::DelaunayTriangle& t, 
        FormatContext& ctx) const
    {
        return std::format_to(ctx.out(), "DelaunayTriangle({}, {}, {})", 
            t.a, t.b, t.c);
    }
};
