#include "visualization.hpp"
#include "geometry.hpp"

#include <print>
#include <matplot/matplot.h>

namespace geometry::visualization
{
    template <class... Fs>
    struct Multilambda : Fs...
    {
        using Fs::operator()...;
    };

    auto DrawConfig()
    {
        using namespace geometry;
        using namespace matplot;

        // Disable gnuplot warnings
        auto fig = figure(false);
        fig->backend()->run_command("unset warnings");
        fig->ioff();
        fig->size(900, 900);

        hold(on);     // Multiple plots mode
        axis(equal);  // Square view
        grid(on);     // Enable grid by default
        return fig;
    }

    void Draw(std::span<geometry::Shape> shapes)
    {
        using namespace geometry;
        using namespace matplot;

        const figure_handle& fig_handle = DrawConfig();

        for (const auto& [index, shape] : 
            std::ranges::views::enumerate(shapes))
        {
            /**
             * @brief Для каждой фигуры примените `std::visit` с помощью мульти-лямбд (Multilambda),
             *    которая обрабатывает каждый возможный тип фигуры отдельно.
             *    Внутри каждой лямбды:
             *    - Вызовите метод `.Lines()` у фигуры — он возвращает структуру с двумя векторами:
             *      `.x` и `.y`, содержащими координаты точек для отрисовки.
             *    - Передайте эти координаты в функцию `plot(lines.x, lines.y)`.
             *    - Настройте внешний вид линии: установите толщину `.line_width(2)` и задайте цвет `.color()`:
             *        • Line      → "yellow"
             *        • Triangle  → "blue"
             *        • Rectangle → "green"
             *        • RegularPolygon → "magenta"
             *        • Circle    → "red"
             *        • Polygon   → "cyan"
             * 
             */

            // Add shape number
            const Point2D centre = shape.visit([]<typename Shape>(const Shape& shape) { return shape.Centre(); });
            auto t = text(centre.x, centre.y, std::to_string(index));
            t->font_size(14);

            std::visit([]<typename Shape>(const Shape& shape)
                {
                    plot(shape.Lines().x, shape.Lines().y);
                }, shape);

            if (std::holds_alternative<Line>(shape)) t->color("yellow");
            else if (std::holds_alternative<Triangle>(shape)) t->color("blue");
            else if (std::holds_alternative<Rectangle>(shape)) t->color("green");
            else if (std::holds_alternative<RegularPolygon>(shape)) t->color("magenta");
            else if (std::holds_alternative<Rectangle>(shape)) t->color("red");
            else if (std::holds_alternative<Polygon>(shape)) t->color("cyan");
            else t->color("black");
        }

        // Display plot
        fig_handle->show();
    }

    void Draw(std::span<const triangulation::DelaunayTriangle> triangles)
    {
        using namespace geometry;
        using namespace matplot;
        
        const figure_handle& fig_handle = DrawConfig();

        for (const auto& [index, d_triangle] : 
            std::ranges::views::enumerate(triangles))
        {
            const geometry::Triangle tri{d_triangle.a, d_triangle.b, d_triangle.c};
            const auto lines = tri.Lines();
            plot(lines.x, lines.y)->line_width(2).color("cyan");

            // Add triangle number
            const auto centre = tri.Centre();
            auto t = text(centre.x, centre.y, std::to_string(index));
            t->font_size(14);
            t->color("black");
        }

        // Display plot
        fig_handle->show();
    }
}  // namespace geometry::visualization