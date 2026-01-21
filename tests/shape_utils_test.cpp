#include <gtest/gtest.h>
#include "shape_utils.hpp"
#include "visualization.hpp"

TEST(ShapeUtils_Tests, Collisions)
{
    using namespace geometry;
    using namespace utils;

    std::vector<Shape> shapes;
    shapes.emplace_back(Triangle{Point2D{1, 2}, Point2D{3, 4}, Point2D{0, 0}});
    shapes.emplace_back(Rectangle{Point2D{5, 3}, 3, 11});
    shapes.emplace_back(Circle{Point2D{7, 2}, 2});
    shapes.emplace_back(RegularPolygon{Point2D{2, 11}, 1, 15});
    shapes.emplace_back(Polygon{std::vector{Point2D{0, -1}, Point2D{2, -2}, 
        Point2D{0, 0}}});
    
    // Проверка
    // visualization::Draw(shapes);

    std::vector<std::pair<Shape, Shape>> result = 
        FindAllCollisions(shapes);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(FindAllCollisions({}).size(), 0);
}

TEST(ShapeUtils_Tests, HighestShape)
{
    using namespace geometry;
    using namespace utils;

    std::vector<Shape> shapes;
    shapes.emplace_back(Triangle{Point2D{1, 2}, Point2D{3, 4}, Point2D{-1, 9}}); // h = 9
    shapes.emplace_back(Rectangle{Point2D{5, 3}, 5, 8}); // h = 3 + 8 = 11
    shapes.emplace_back(Circle{Point2D{7, 2}, 2}); // h = 2 + 2 = 6
    shapes.emplace_back(RegularPolygon{Point2D{2, 11}, 1, 15}); // h = 11 + 1 = 12
    shapes.emplace_back(Polygon{std::vector{Point2D{0, -1}, Point2D{2, -2}, 
        Point2D{1, -10}, Point2D{0, 0}}}); // h = 0

    std::optional maybe_index = 
        FindHighestShape(shapes);
    
    if ((bool)maybe_index)
    {
        EXPECT_EQ(*maybe_index, 3);
    }
    else
    {
        EXPECT_EQ(!maybe_index, false);
    }

    EXPECT_EQ(!FindHighestShape({}), true);
}

TEST(ShapeUtils_Tests, LowestShape)
{
    using namespace geometry;
    using namespace utils;

    std::vector<Shape> shapes;
    shapes.emplace_back(Triangle{Point2D{1, 2}, Point2D{3, 4}, Point2D{-1, 9}}); // h = 9
    shapes.emplace_back(Rectangle{Point2D{5, 3}, 5, 8}); // h = 3 + 8 = 11
    shapes.emplace_back(Circle{Point2D{7, 2}, 2}); // h = 2 + 2 = 4
    shapes.emplace_back(RegularPolygon{Point2D{2, 11}, 1, 15}); // h = 11 + 1 = 12
    shapes.emplace_back(Polygon{std::vector{Point2D{0, -1}, Point2D{2, -2}, 
        Point2D{1, -10}, Point2D{0, 0}}}); // h = 0

    std::optional maybe_index = 
        FindLowestShape(shapes);
    
    if ((bool)maybe_index)
    {
        EXPECT_EQ(*maybe_index, 4);
    }
    else
    {
        EXPECT_EQ(!maybe_index, false);
    }

    EXPECT_EQ(!FindLowestShape({}), true);
}