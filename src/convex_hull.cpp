#include "convex_hull.hpp"

namespace geometry::convex_hull
{
    double CrossProduct(Point2D p1, Point2D middle, Point2D p2)
    {
        Point2D new_p1 = p1 - middle;
        Point2D new_p2 = p2 - middle;

        return new_p1.Cross(new_p2);
    }

    std::expected<std::vector<Point2D>, std::logic_error> 
    GrahamScan(std::span<Point2D> points) noexcept
    {
        if (points.size() < 3)
        {
            return std::unexpected(std::logic_error("At least "
                "three points are required for the convex hull"));
        }
        
        Point2D smallest;
        {
            using It = std::span<Point2D>::iterator;

            It smallest_pos = std::min_element(points.begin(), points.end(), 
            [](const Point2D& p1, const Point2D& p2)
            {
                // Наименьший y, при неединственности - наименьший x
                return (p1.y == p2.y) ? p1.x < p2.x : p1.y < p2.y;
            });

            std::swap(*points.begin(), *smallest_pos);
            smallest = points.front();
        }

        std::sort(points.begin() + 1, points.end(), 
            [&smallest](const Point2D& p1, const Point2D& p2)
            {
                double cross = CrossProduct(p1, smallest, p2);
                if (std::abs(cross) < Precision::eps)
                {
                    return smallest.DistanceTo(p1) < 
                        smallest.DistanceTo(p2);
                }

                return cross > 0;
            });

        StackForGrahamScan hull;
        for (const Point2D& new_p : points)
        {
            while (hull.Size() > 1 && 
                !DblLess{}(CrossProduct(hull.NextToTop(), hull.Top(), new_p), 0.0))
            {
                hull.Pop();
            }
            
            hull.Push(new_p);
        }

        return std::vector{hull.Extract()};
    }
}  // namespace geometry::convex_hull