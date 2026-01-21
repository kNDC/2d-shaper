#include "convex_hull.hpp"
#include "geometry.hpp"
#include "intersections.hpp"
#include "queries.hpp"
#include "shape_utils.hpp"
#include "triangulation.hpp"
#include "visualization.hpp"

#include <iostream>
#include <print>

#include <random>
#include <ranges>

#include <algorithm>

using namespace geometry;

namespace rng = std::ranges;
namespace views = std::ranges::views;

void PrintAllIntersections(const Shape& shape, 
    std::span<const Shape> others)
{
    using namespace queries;
    using namespace intersections;

    std::println("=== Intersections ===");

    if (!DistanceSupported(shape))
    {
        std::println("Пересечения отсутствуют");
        std::println();
        return;
    }
    
    /*
     * Используйте ranges чтобы оставить только фигуры,
     * поддерживающие возможность находить пересечения между собой
     *
     * Затем примените монадический интерфейс для обработки результатов:
     *     - Пересечение найдено в точке A между фигурами B и C
     *     - Фигуры B и C не пересекаются
     */
    std::ranges::for_each(others | views::filter(DistanceSupported), 
        [&shape](const Shape& other)
        {
            GetIntersectPoint(shape, other)
                .transform([&shape, &other](const Point2D& p)
                    {
                        std::println("Пересечение найдено в точке {} между "
                            "фигурами {} и {}", p, shape, other);
                        return p;
                    })
                .or_else([&shape, &other]()
                    {
                        std::println("Фигуры {} и {} не пересекаются", 
                            shape, other);
                        return std::optional<Point2D>{};
                    });
        });
        
    std::println();
}

void PrintDistancesFromPointToShapes(Point2D p, 
    std::span<const Shape> shapes, size_t sample_size = 5)
{
    using namespace queries;
    using namespace std::ranges;

    std::println("=== Distance from Point Test ===");

    using Sample = std::vector<std::reference_wrapper<const Shape>>;
    Sample sm; sm.reserve(sample_size);
    
    sample(shapes, std::back_inserter(sm), 
        sample_size, std::mt19937{std::random_device{}()});
    
    for_each(sm, [&p](const Shape& shape)
        {
            std::println("Расстояние от точки {} до фигуры {} равно {:.2f}", 
                p, shape, DistanceToPoint(p, shape));
        });
    
    std::println();
}

void PerformShapeAnalysis(std::span<const Shape> shapes)
{
    using namespace queries;
    using namespace std::ranges;

    std::println("=== Shape Analysis ===");
    
    {
        std::vector<std::pair<Shape, Shape>> overlap_data = 
            utils::FindAllCollisions(shapes);
        
        std::println("Пересечения фигур:");
        for_each(overlap_data, 
            [](const std::pair<Shape, Shape>& overlap)
            {
                std::println("  {}, {}", 
                    overlap.first, overlap.second);
            });
        std::println();
    }

    {
        std::optional i = utils::FindHighestShape(shapes);

        if ((bool)i)
        {
            std::println("Наивысшая точка находится в фигуре:");
            std::println("{}", shapes[*i]);
            std::println();
        }
    }
    
    // Расстояние между любыми двумя поддерживаемыми фигурами
    {
        using Sample = std::vector<std::reference_wrapper<const Shape>>;

        Sample sm;
        {
            Sample full_sm = shapes | 
                views::filter(DistanceSupported) | 
                to<Sample>();
            
            sm.reserve(2);
            sample(full_sm, std::back_inserter(sm), 
                2, std::mt19937{std::random_device{}()});
        }
        
        if (sm.size() < 2)
        {
            std::println("Недостаточное количество поддерживаемых фигур");
            std::println();
            return;
        }
        
        std::println("Расстояние между {} и {} = {:.2f}",
            sm[0].get(), sm[1].get(), *DistanceBetweenShapes(sm[0], sm[1]));
        std::println();
    }
}

void PerformExtraShapeAnalysis(std::span<const Shape> shapes, 
    const double height_threshold = 50.0, 
    unsigned sample_size = 3)
{
    using namespace queries;
    using namespace std::ranges;

    std::println("=== Shape Extra Analysis ===");

    // Выборка тел высотой не менее пятидесяти
    {
        using Sample = std::vector<std::reference_wrapper<const Shape>>;

        Sample sm;
        {
            Sample full_sm = shapes | 
                views::filter([height_threshold](const Shape& shape)
                {
                    return GetHeight(shape) > height_threshold;
                }) | to<Sample>();
            
            sample(full_sm, std::back_inserter(sm), 
                sample_size, std::mt19937{std::random_device{}()});
        }
        
        if (!sm.size())
        {
            std::println("Отсутствуют фигуры высотой от {} и выше", 
                height_threshold);
        }
        else
        {
            std::println("Выборка фигур (n = {}) высотой от {} и выше", 
                sm.size(), height_threshold);
            for_each(sm, 
                [](const Shape& shape)
                {
                    std::println("{}", shape);
                });
        }

        std::println();
    }

    // Фигуры с наименьшей и наибольшей высотами
    {
        std::optional i_max = utils::FindHighestShape(shapes);
        std::optional i_min = utils::FindLowestShape(shapes);

        if ((bool)i_max)
        {
            std::println("Наивысшая точка находится в фигуре:");
            std::println("{}", shapes[*i_max]);
            std::println();

            std::println("Самая низшая точка находится в фигуре:");
            std::println("{}", shapes[*i_min]);
            std::println();
        }
    }
}

int main()
{
    using namespace visualization;
    using namespace convex_hull;
    using namespace triangulation;
    using namespace std::ranges;

    std::vector<Shape> shapes = utils::ParseShapes("circle 40 50 15; "
        "polygon 25 35 3 3; polygon 35 40 5 8; rectangle 35 20 8 5; "
        "circle 5 40 8; rectangle 15 25 12 7; "
        "line 1 2 3 4; polygon 0 0 2 5; triangle 0 0 1 0 0.5 1; "
        "polygon 0 0 1 2; badshape; circle 0 0 -1");

    std::println("Прочитано {} фигур", shapes.size());
    std::println();
    
    std::println("Высшие точки фигур:");
    for_each(shapes | views::enumerate, 
        [](const std::tuple<size_t, Shape>& datum)
        {
            std::println("{}: высота = {:.2f}", 
                std::get<0>(datum), queries::GetHeight(std::get<1>(datum)));
        });
    std::println();
            
    //
    // Вызываем разработанные функции
    //
    PrintAllIntersections(shapes[0], shapes);

    PrintDistancesFromPointToShapes(Point2D{10.0, 10.0}, shapes);

    PerformShapeAnalysis(shapes);

    PerformExtraShapeAnalysis(shapes);

    // Отрисовка всех геометрических тел
    // Важно: переход к каждому следующему графику по нажатию на Ввод
    geometry::visualization::Draw(shapes);
    
    // Построение выпуклой оболочки алгоритмом Грэхэма
    {
        // Формируем список из вершин всех фигур
        std::vector<Point2D> points = 
            shapes | views::transform([](const Shape& shape)
                {
                    return 
                        std::visit([]<typename Shape_>(const Shape_& shape)
                            {
                                auto vertices = shape.Vertices();
                                return std::vector<Point2D>(vertices.begin(), 
                                    vertices.end());
                            }, shape);
                }) | views::join | to<std::vector<Point2D>>();
        
        // Находим список точек, для построения выпуклой оболочки - convex hull - алгоритмом Грэхема 
        // Создаём из них объект класса `Polygon` и добавляем его в список shapes
        std::vector<Point2D> convex_hull = GrahamScan(points)
            .or_else([](std::logic_error e) -> std::expected<std::vector<Point2D>, std::logic_error>
                {
                    std::println(std::cerr, "Error: {}", e.what());
                    return std::vector<Point2D>{};
                })
            .value();

        // Рисуем все фигуры
        shapes.emplace_back(std::move(Polygon{convex_hull}));
        
        Draw(shapes);
    }

    // после изучения графика - нажмите Enter чтобы продолжить выполнение и построить 3ий график
    {
        std::vector<Point2D> points = {{0, 0}, {10, 0}, {5, 8}, {15, 5}, {2, 12}};

        //
        // Используйте список точек points или свой, чтобы
        // выполнить алгоритм триангуляции Делоне алгоритмом Боуэра-Ватсона
        //
        std::vector<DelaunayTriangle> triangles = 
            DelaunayTriangulation(points)
                .or_else([](std::logic_error e) -> std::expected<std::vector<DelaunayTriangle>, std::logic_error>
                    {
                        std::println(std::cerr, "Error: {}", e.what());
                        return std::vector<DelaunayTriangle>{};
                    })
                .value();
        
        //
        // После успешного завершения алгоритма - выведите результат для проверки
        // используя geometry::visualization::Draw
        //
        Draw(triangles);
    }
}