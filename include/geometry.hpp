#pragma once

#include "utils.hpp"

#include <format>

#include <cmath>
#include <numbers>

#include <array>
#include <vector>
#include <span>
#include <ranges>
#include <algorithm>

#include <variant>

namespace geometry
{
    /*
    * Добавьте к методам класса Point2D и Lines2DDyn все необходимые атрибуты и спецификаторы
    * Важно: Возвращаемый тип и принимаемые аргументы менять не нужно
    */
    struct Point2D
    {
        double x, y;

        constexpr Point2D() : x(0), y(0) {}
        constexpr Point2D(double x, double y) : x(x), y(y) {}

        // Comparison
        constexpr bool operator<(const Point2D& other) const
        {
            return DblLess{}(x, other.x) && 
                DblLess{}(y, other.y);
        }

        constexpr bool operator==(const Point2D& other) const
        {
            return DblEquals{}(x, other.x) && 
                DblEquals{}(y, other.y);
        }

        // Binary maths operators
        constexpr Point2D operator+(const Point2D& other) const
        {
            return {x + other.x, y + other.y};
        }

        constexpr Point2D operator-(const Point2D& other) const
        {
            return {x - other.x, y - other.y};
        }

        Point2D operator*(double value) { return {x * value, y * value}; }
        Point2D operator/(double value) { return {x / value, y / value}; }

        Point2D operator*=(double value) { return *this * value; }
        Point2D operator/=(double value) { return *this / value; }

        // Binary geometry operations
        double Dot(const Point2D& other) { return x * other.x + y * other.y; }
        double Cross(const Point2D& other) { return x * other.y - y * other.x; }
        double Length() { return std::sqrt(x * x + y * y); }
        double DistanceTo(const Point2D& other) const { return (*this - other).Length(); }

        Point2D Normalize()
        {
            const double len = Length();
            return (len > 0) ? Point2D{x / len, y / len} : Point2D{0, 0};
        }
    };

    template <size_t N>
    struct Lines2D
    {
        std::array<double, N> x;
        std::array<double, N> y;
    };

    struct Lines2DDyn
    {
        std::vector<double> x;
        std::vector<double> y;

        void Reserve(size_t n)
        {
            x.reserve(n);
            y.reserve(n);
        }

        void PushBack(Point2D p)
        {
            x.push_back(p.x);
            y.push_back(p.y);
        }

        void PushBack(double px, double py)
        {
            x.push_back(px);
            y.push_back(py);
        }

        Point2D Front() { return { x.front(), y.front() }; }
        Point2D Back() { return { x.back(), y.back() }; }
    };

    struct BoundingBox
    {
        double min_x, min_y, max_x, max_y;

        constexpr BoundingBox() noexcept : 
            min_x(0), min_y(0), max_x(0), max_y(0)
        {}

        constexpr BoundingBox(double min_x, double min_y, 
            double max_x, double max_y) noexcept : 
            min_x(min_x), min_y(min_y), max_x(max_x), max_y(max_y)
        {}

        constexpr bool operator==(const BoundingBox& other)
        {
            return DblEquals{}(min_x, other.min_x) && 
                DblEquals{}(min_y, other.min_y) && 
                DblEquals{}(max_x, other.max_x) && 
                DblEquals{}(max_y, other.max_y);
        }

        [[nodiscard]] constexpr bool Overlaps(const BoundingBox& other) const noexcept
        {
            return !(!DblGreater{}(max_x, other.min_x) || 
                !DblLess{}(min_x, other.max_x) || 
                !DblGreater{}(max_y, other.min_y) || 
                !DblLess{}(min_y, other.max_y));
        }

        [[nodiscard]] constexpr double Width() const noexcept { return max_x - min_x; }
        [[nodiscard]] constexpr double Height() const noexcept { return max_y - min_y; }
        [[nodiscard]] constexpr Point2D Centre() const noexcept { return {(min_x + max_x) / 2, (min_y + max_y) / 2}; }};

    struct Line
    {
        using SupportsDistanceTag = Line;

        Point2D start, end;

        constexpr Line(Point2D start, Point2D end) noexcept : 
            start(start), end(end)
        {}

        [[nodiscard]] constexpr double Length() noexcept { return start.DistanceTo(end); }
        [[nodiscard]] constexpr Point2D Direction() noexcept { return (end - start).Normalize(); }

        [[nodiscard]] constexpr BoundingBox BoundBox() const noexcept
        {
            return { std::min(start.x, end.x), std::min(start.y, end.y), 
                std::max(start.x, end.x), std::max(start.y, end.y) };
        }

        [[nodiscard]] constexpr double Height() const noexcept { return std::max(start.y, end.y); }
        [[nodiscard]] constexpr Point2D Centre() const noexcept { return (start + end) / 2.0; }

        [[nodiscard]] constexpr std::array<Point2D, 2> Vertices() const noexcept
        {
            return {Point2D{start.x, start.y}, {end.x, end.y}};
        }

        [[nodiscard]] constexpr Lines2D<2> Lines() const noexcept
        {
            return {{start.x, end.x}, {start.y, end.y}};
        }
    };

    struct Triangle
    {
        Point2D a, b, c;

        constexpr Triangle(Point2D a, Point2D b, Point2D c) noexcept : a(a), b(b), c(c) {}

        [[nodiscard]] constexpr double Area() const noexcept { return std::abs((b - a).Cross(c - a)) / 2.0; }
        [[nodiscard]] constexpr BoundingBox BoundBox() const noexcept {
            return {std::min({a.x, b.x, c.x}), std::min({a.y, b.y, c.y}), 
                    std::max({a.x, b.x, c.x}), std::max({a.y, b.y, c.y})};
        }
        [[nodiscard]] constexpr std::array<Point2D, 3> Vertices() const noexcept { return {a, b, c}; }
        [[nodiscard]] constexpr double Height() const noexcept { return std::max({a.y, b.y, c.y}); }
        [[nodiscard]] constexpr Point2D Centre() const noexcept { return (a + b + c) / 3.0; }

        [[nodiscard]] constexpr Lines2D<4> Lines() const noexcept { return {{a.x, b.x, c.x, a.x}, {a.y, b.y, c.y, a.y}}; }
    };

    struct Rectangle
    {
        Point2D bottom_left;
        double width, height;

        constexpr Rectangle(Point2D bottom_left, double width, double height) noexcept : 
            bottom_left(bottom_left), width(width), height(height)
        {}

        [[nodiscard]] constexpr Point2D TopRight() const noexcept
        {
            return {bottom_left.x + width, bottom_left.y + height};
        }
        [[nodiscard]] constexpr BoundingBox BoundBox() const noexcept
        {
            return {bottom_left.x, bottom_left.y, bottom_left.x + width, bottom_left.y + height};
        }
        [[nodiscard]] constexpr std::array<Point2D, 4> Vertices() const noexcept
        {
            return {bottom_left,
                    {bottom_left.x + width, bottom_left.y},
                    {bottom_left.x + width, bottom_left.y + height},
                    {bottom_left.x, bottom_left.y + height}};
        }

        [[nodiscard]] constexpr double Height() const noexcept { return bottom_left.y + height; }
        [[nodiscard]] constexpr Point2D Centre() const noexcept { return bottom_left + (Point2D{width, height} / 2.0); }

        [[nodiscard]] constexpr Lines2D<5> Lines() const noexcept
        {
            return {{bottom_left.x, bottom_left.x, bottom_left.x + width, bottom_left.x + width, bottom_left.x},
                    {bottom_left.y, bottom_left.y + height, bottom_left.y + height, bottom_left.y, bottom_left.y}};
        }
    };

    struct RegularPolygon
    {
        Point2D centre_p;
        double radius;
        int sides;

        constexpr RegularPolygon(Point2D centre, double radius, int sides)
            : centre_p(centre), radius(radius), sides(sides)
        {}

        std::vector<Point2D> Vertices() const
        {
            std::vector<Point2D> points;
            points.reserve(sides);

            for (int i = 0; i < sides; ++i)
            {
                const double angle = 2 * std::numbers::pi * i / sides;
                points.emplace_back(centre_p.x + radius * std::cos(angle), 
                    centre_p.y + radius * std::sin(angle));
            }
            return points;
        }

        [[nodiscard]] constexpr BoundingBox BoundBox() const noexcept
        {
            return {centre_p.x - radius, centre_p.y - radius, centre_p.x + radius, centre_p.y + radius};
        }

        [[nodiscard]] constexpr double Height() const noexcept { return centre_p.y + radius; }
        [[nodiscard]] constexpr Point2D Centre() const noexcept { return centre_p; }

        [[nodiscard]] Lines2DDyn Lines() const
        {
            auto verts = Vertices();
            Lines2DDyn lines;
            lines.Reserve(verts.size() + 1);

            for (const auto& p : verts)
            {
                lines.PushBack(p);
            }

            lines.PushBack(lines.Front());
            return lines;
        }
    };

    struct Circle
    {
        using SupportsDistanceTag = Circle;

        Point2D centre_p;
        double radius;

        constexpr Circle(Point2D centre, double radius) noexcept : 
            centre_p(centre), radius(radius)
        {}

        [[nodiscard]] constexpr BoundingBox BoundBox() const noexcept
        {
            return { centre_p.x - radius, centre_p.y - radius, 
                centre_p.x + radius, centre_p.y + radius };
        }

        [[nodiscard]] constexpr double Height() const noexcept { return centre_p.y + radius; }
        [[nodiscard]] constexpr Point2D Centre() const noexcept { return centre_p; }

        [[nodiscard]] std::vector<Point2D> Vertices(size_t N = 30) const
        {
            std::vector<Point2D> points;
            points.reserve(N);

            for (auto i : std::ranges::views::iota(0u, N))
            {
                const double angle = 2 * std::numbers::pi * i / N;
                points.emplace_back(centre_p.x + radius * std::cos(angle), centre_p.y + radius * std::sin(angle));
            }
            return points;
        }

        [[nodiscard]] Lines2DDyn Lines(size_t N = 100) const
        {
            Lines2DDyn lines;
            lines.Reserve(N + 1);

            for (auto i : std::ranges::views::iota(0u, N))
            {
                double angle = 2 * std::numbers::pi * i / N;
                lines.PushBack(centre_p.x + radius * std::cos(angle), centre_p.y + radius * std::sin(angle));
            }

            lines.PushBack(lines.Front());
            return lines;
        }
    };

    class Polygon
    {
    public:
        Polygon(std::vector<Point2D> points) noexcept : 
            points_(std::move(points))
        {
            CalculateBoundBox();
        }

        [[nodiscard]] constexpr BoundingBox BoundBox() const noexcept { return bounding_box_; }
        [[nodiscard]] constexpr double Height() const noexcept
        {
            const auto box = BoundBox();
            return box.max_y;
        }

        [[nodiscard]] constexpr Point2D Centre() const noexcept
        {
            const BoundingBox box = BoundBox();
            return Point2D{ (box.min_x + box.max_x) / 2.0, 
                (box.min_y + box.max_y) / 2.0 };
        }

        [[nodiscard]] std::span<const Point2D> Vertices() const noexcept { return points_; }

        [[nodiscard]] Lines2DDyn Lines() const
        {
            Lines2DDyn lines;
            lines.Reserve(points_.size() + 1);

            for (const auto& p : points_) lines.PushBack(p);
            
            if (lines.Front() != lines.Back())
            {
                lines.PushBack(lines.Front());
            }
            
            return lines;
        }

    private:
        void CalculateBoundBox()
        {
            double min_x = points_[0].x, max_x = points_[0].x;
            double min_y = points_[0].y, max_y = points_[0].y;

            for (const auto& p : points_)
            {
                if (p.x < min_x) min_x = p.x;
                if (p.x > max_x) max_x = p.x;
                if (p.y < min_y) min_y = p.y;
                if (p.y > max_y) max_y = p.y;
            }

            bounding_box_ = BoundingBox{min_x, min_y, max_x, max_y};
        }

        std::vector<Point2D> points_;
        BoundingBox bounding_box_;
    };

    using Shape = std::variant<Line, Triangle, Rectangle, RegularPolygon, Circle, Polygon>;
}  // namespace geometry

template <>
struct std::formatter<geometry::Point2D>
{
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Point2D& p, 
        FormatContext& ctx) const
    {
        return format_to(ctx.out(), 
            "({:.2f}; {:.2f})", p.x, p.y);
    }
};

template<>
struct std::formatter<std::vector<geometry::Point2D>>
{
    bool use_new_line = false;

    constexpr auto parse(std::format_parse_context& ctx)
    {
        using namespace std::string_view_literals;

        using It = std::format_parse_context::const_iterator;
        It from = ctx.begin();
        It to = ctx.end();
        const std::string_view fmt_spec{from, to};
        
        if (from != to && 
            fmt_spec.starts_with("new_line"sv))
        {
            use_new_line = true;
            from += "new_line"sv.size();
        }

        if (from != to && *from != '}')
        {
            throw std::format_error("Invalid format");
        }

        return from;
    }

    auto format(const std::vector<geometry::Point2D>& v, 
        std::format_context& ctx) const
    {
        if (use_new_line)
        {
            for (const geometry::Point2D& p : v)
            {
                ctx.out() = std::format_to(ctx.out(), 
                    "\n\t{}", p);
            }

            if (v.size())
            {
                ctx.out() = std::format_to(ctx.out(), "\n");
            }
        }
        else
        {
            std::string_view sep = ""sv;
            for (const geometry::Point2D& p : v)
            {
                ctx.out() = std::format_to(ctx.out(), 
                    "{}{}", sep, p);
                sep = " "sv;
            }
        }
        
        return std::format_to(ctx.out(), "");
    }
};

template <>
struct std::formatter<geometry::Line>
{
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Line& l, 
        FormatContext& ctx) const
    {
        return std::format_to(ctx.out(), 
            "Line({}, {})", l.start, l.end);
    }
};

template <>
struct std::formatter<geometry::Circle>
{
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Circle& c, 
        FormatContext& ctx) const
    {
        return std::format_to(ctx.out(), 
            "Circle(centre={}, r={:.2f})", 
            c.centre_p, c.radius);
    }
};

template <>
struct std::formatter<geometry::Rectangle>
{
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Rectangle& r, 
        FormatContext& ctx) const
    {
        return std::format_to(ctx.out(), 
            "Rectangle(bottom_left={}, w={:.2f}, h={:.2f})", 
            r.bottom_left, r.width, r.height);
    }
};

template <>
struct std::formatter<geometry::RegularPolygon>
{
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::RegularPolygon& p, 
        FormatContext& ctx) const
    {
        return std::format_to(ctx.out(), 
            "RegularPolygon(centre={}, r={:.2f}, sides={})", 
            p.centre_p, p.radius, p.sides);
    }
};

template <>
struct std::formatter<geometry::Triangle>
{
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Triangle& t, 
        FormatContext& ctx) const
    {
        return std::format_to(ctx.out(), 
            "Triangle({}, {}, {})", 
            t.a, t.b, t.c);
    }
};

template <>
struct std::formatter<geometry::Polygon>
{
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Polygon& poly, 
        FormatContext& ctx) const
    {
        ctx.out() = std::format_to(ctx.out(), "Polygon[{} points]: [", 
            poly.Vertices().size());

        for (const geometry::Point2D& p : poly.Vertices())
        {
            ctx.out() = std::format_to(ctx.out(), "{} ", p);
        }

        return std::format_to(ctx.out(), "]");
    }
};

template <>
struct std::formatter<geometry::Shape>
{
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Shape& shape, 
        FormatContext& ctx) const
    {
        return std::visit([&ctx]<typename Shape>(const Shape& shape)
            {
                return std::formatter<Shape>{}.format(shape, ctx);
            }, shape);
    }
};