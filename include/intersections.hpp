#pragma once

#include "geometry.hpp"
#include "queries.hpp"
#include "utils.hpp"

#include <cmath>
#include <optional>

namespace geometry::intersections
{
    /*
    * Класс для поиска пересечений между двумя фигурами
    *
    * Требуется организовать возможность нахождения пересечений только для следующих комбинаций фигур:
    *    - Line   & Line
    *    - Line   & Circle
    *    - Circle & Circle
    *
    * Для всех остальных требуется выбросить исключение std::logic_error
    */
    class IntersectionVisitor
    {
    public:
        std::optional<Point2D> operator()(const Line& line1, 
            const Line& line2) const
        {
            /* Решение соответствующей системы линейных уравнений
            задаётся тремя числами */
            double num1 = 
                (line2.end.x - line2.start.x) * (line2.start.y - line1.start.y) - 
                (line2.end.y - line2.start.y) * (line2.start.x - line1.start.x);

            double num2 = 
                (line1.end.x - line1.start.x) * (line2.start.y - line1.start.y) - 
                (line1.end.y - line1.start.y) * (line2.start.x - line1.start.x);
            
            double den = 
                (line1.end.y - line1.start.y) * (line2.end.x - line2.start.x) - 
                (line1.end.x - line1.start.x) * (line2.end.y - line2.start.y);
            
            // Случай расположения вдоль одной прямой
            if (DblEquals{}(den, 0) && 
                (DblEquals{}(num1, 0) || DblEquals{}(num2, 0)))
            {
                // Начало l2 в l1 <=> конец l1 в l2
                if (!DblGreater{}(line2.start.x, line1.end.x) && 
                    !DblLess{}(line2.start.x, line1.start.x) && 
                    !DblGreater{}(line2.start.y, line1.end.y) && 
                    !DblLess{}(line2.start.y, line1.start.y))
                {
                    return (line2.start + line1.end) / 2;
                }
                
                // Начало l1 в l2 <=> конец l2 в l1
                if (!DblGreater{}(line1.start.x, line2.end.x) && 
                    !DblLess{}(line1.start.x, line2.start.x) && 
                    !DblGreater{}(line1.start.y, line2.end.y) && 
                    !DblLess{}(line1.start.y, line2.start.y))
                {
                    return (line1.start + line2.end) / 2;
                }
                
                // Отсутствие пересечения
                return std::nullopt;
            }
            
            // Случай параллельности/коллинеарности
            if (DblEquals{}(den, 0)) return std::nullopt;

            double scale1 = num1 / den;
            double scale2 = num2 / den;

            // Случай пересечения отрезков
            if (!DblLess{}(scale1, 0) && !DblGreater{}(scale1, 1) && 
                !DblLess{}(scale2, 0) && !DblGreater{}(scale2, 1))
            {
                return line1.start + (line1.end - line1.start) * scale1;
            }

            // Отрезки непараллельны и не пересекаются
            return std::nullopt;
        }
        
        std::optional<Point2D> operator()(const Line& line, 
            const Circle& circle) const
        {
            // Линия полностью находится в круге
            if (DblLess{}(line.start.DistanceTo(circle.centre_p), circle.radius) && 
                DblLess{}(line.end.DistanceTo(circle.centre_p), circle.radius))
            {
                return std::nullopt;
            }
            
            double dist = queries::DistanceToPoint(circle.centre_p, line);

            // Линия полностью вне круга
            if (DblGreater{}(dist, circle.radius))
            {
                return std::nullopt;
            }

            Point2D out_norm;
            double l;
            {
                /* Сначала находим точку пересечения с 
                перпендикуляром к отрезку */
                Point2D line_vec = line.end - line.start;
                Point2D centre_vec = circle.centre_p - line.start;

                double scale = line_vec.Dot(centre_vec) / 
                    line_vec.Dot(line_vec);
                
                out_norm = line.start + line_vec * scale;
                l = (out_norm - circle.centre_p).Length();

                /* Теперь находим пересечение 
                перпендикуляра с окружностью */
                out_norm = (out_norm - circle.centre_p) * 
                    circle.radius / l;
            }

            /* Особый случай - прямая отрезка проходит через 
            центр окружности */
            if (DblEquals{}(out_norm.Dot(out_norm), 0))
            {
                Point2D out = line.end - circle.centre_p;
                return out * circle.radius / out.Length();
            }
            
            /* Длина отрезка м/у точками пересечения 
            с окружностью и перпендикуляром */
            double h = std::sqrt(std::clamp(circle.radius * circle.radius - l * l, 
                0.0, circle.radius * circle.radius));
            
            double x_prev = out_norm.x, y_prev = out_norm.y;

            Point2D out;
            out.x = out_norm.x * l - out_norm.y * h;
            out.y = out_norm.x * h + out_norm.y * l;
            out = out / circle.radius + circle.centre_p;

            // Существует только другая точка пересечения
            if (DblGreater{}((out - line.start).Length() / 
                (line.end - line.start).Length(), 1.0))
            {
                out = out_norm;
                out.x = out_norm.x * l + out_norm.y * h;
                out.y = -out_norm.x * h + out_norm.y * l;
                out = out / circle.radius + circle.centre_p;
            }
            
            return out;
        }
        
        std::optional<Point2D> operator()(const Circle& circle, 
            const Line& line) const
        {
            return operator()(line, circle);
        }

        std::optional<Point2D> operator()(const Circle& circle1, 
            const Circle& circle2) const
        {
            double dist = circle1.centre_p.DistanceTo(circle2.centre_p);
            
            // Случай полного перекрытия кругов
            if (DblLess{}(dist, std::abs(circle1.radius - circle2.radius)))
            {
                return std::nullopt;
            }
            
            // Случай непересекающихся окружностей
            if (DblGreater{}(dist, circle1.radius + circle2.radius))
            {
                return std::nullopt;
            }

            // Длина отрезка м/у центрами внутри области пересечения кругов
            double l = (dist * dist + circle1.radius * circle1.radius - 
                (circle2.radius * circle2.radius)) / 2 / dist;
            
            // Длина отрезка радикальной оси внутри области пересечения кругов
            double h = std::sqrt(std::clamp(circle1.radius * circle1.radius - l * l, 
                0.0, circle1.radius * circle1.radius));
            
            Point2D axis_v = circle2.centre_p - circle1.centre_p;
            Point2D out = axis_v * l / dist;

            out.x -= axis_v.y * h / dist;
            out.y += axis_v.x * h / dist;

            return out + circle1.centre_p;
        }

        template <typename Shape1, typename Shape2>
        std::optional<Point2D> operator()(const Shape1&, 
            const Shape2&) const
        {
            throw std::logic_error("Unsupported shapes");
            return std::nullopt;
        }
    };

    inline std::optional<Point2D> GetIntersectPoint(const Shape& shape1, 
        const Shape& shape2)
    {
        return std::visit(IntersectionVisitor{}, shape1, shape2);
    }
}  // namespace geometry::intersections