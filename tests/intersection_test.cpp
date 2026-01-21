#include <gtest/gtest.h>
#include "intersections.hpp"

TEST(Intersections_Tests, LineLine)
{
    using namespace geometry;
    using namespace intersections;

    // Отрезки на одной прямой
    {
        Line l1{Point2D{0, 0}, Point2D{2, 3}};
        Line l2{Point2D{1, 1.5}, Point2D{4, 6}};

        std::optional maybe_intersection = 
            GetIntersectPoint(l1, l2);

        if ((bool)maybe_intersection)
        {
            EXPECT_EQ(*maybe_intersection == 
                (Point2D{1, 1.5} + Point2D{2, 3}) / 2, 
                true);
        }
        else
        {
            EXPECT_EQ((bool)maybe_intersection, 
                true);
        }

        maybe_intersection = 
            GetIntersectPoint(l2, l1);

        if ((bool)maybe_intersection)
        {
            EXPECT_EQ(*maybe_intersection == 
                (Point2D{1, 1.5} + Point2D{2, 3}) / 2, 
                true);
        }
        else
        {
            EXPECT_EQ((bool)maybe_intersection, 
                true);
        }
    }

    // Параллельные отрезки
    {
        Line l1{Point2D{0, 0}, Point2D{2, 3}};
        Line l2{Point2D{1, 0}, Point2D{3, 3}};

        EXPECT_EQ(!GetIntersectPoint(l1, l2), 
            true);
        EXPECT_EQ(!GetIntersectPoint(l2, l1), 
            true);
    }

    // Пересекающиеся отрезки
    {
        Line l1{Point2D{0, 10}, Point2D{12, -1}};
        Line l2{Point2D{1, 0}, Point2D{8, 9}};

        std::optional maybe_intersection = 
            GetIntersectPoint(l1, l2);

        if ((bool)maybe_intersection)
        {
            EXPECT_EQ(*maybe_intersection == 
                (Point2D{948, 981} / 185), 
                true);
        }
        else
        {
            EXPECT_EQ((bool)maybe_intersection, 
                true);
        }

        maybe_intersection = 
            GetIntersectPoint(l2, l1);

        if ((bool)maybe_intersection)
        {
            EXPECT_EQ(maybe_intersection.value() == 
                (Point2D{948, 981} / 185), 
                true);
        }
        else
        {
            EXPECT_EQ((bool)maybe_intersection, 
                true);
        }
    }

    // Пересекающиеся концами отрезки
    {
        Line l1{Point2D{2, 9}, Point2D{8, 22}};
        Line l2{Point2D{-1, -10}, Point2D{2, 9}};

        std::optional maybe_intersection = 
            GetIntersectPoint(l1, l2);

        if ((bool)maybe_intersection)
        {
            EXPECT_EQ(*maybe_intersection == 
                (Point2D{2, 9}), true);
        }
        else
        {
            EXPECT_EQ((bool)maybe_intersection, 
                true);
        }

        maybe_intersection = 
            GetIntersectPoint(l2, l1);

        if ((bool)maybe_intersection)
        {
            EXPECT_EQ(*maybe_intersection == 
                (Point2D{2, 9}), true);
        }
        else
        {
            EXPECT_EQ((bool)maybe_intersection, 
                true);
        }
    }

    // Непараллельные непересекающиеся отрезки
    {
        Line l1{Point2D{4, 7}, Point2D{8, 3}};
        Line l2{Point2D{-4, -3}, Point2D{1, 2}};

        std::optional maybe_intersection = 
            GetIntersectPoint(l1, l2);

        EXPECT_EQ(!maybe_intersection, 
            true);
        
        maybe_intersection = 
            GetIntersectPoint(l2, l1);

        EXPECT_EQ(!maybe_intersection, 
            true);
    }
}

TEST(Intersections_Tests, CircleCircle)
{
    using namespace geometry;
    using namespace intersections;

    // Одна точка пересечения, вложенные круги
    {
        Circle c1{Point2D{3, 4}, 5};
        Circle c2{Point2D{5, 4}, 3};

        std::optional maybe_intersection = 
            GetIntersectPoint(c1, c2);

        if ((bool)maybe_intersection)
        {
            EXPECT_EQ(*maybe_intersection == 
                (Point2D{8, 4}), true);
        }
        else
        {
            EXPECT_EQ((bool)maybe_intersection, 
                true);
        }

        maybe_intersection = 
            GetIntersectPoint(c2, c1);

        if ((bool)maybe_intersection)
        {
            EXPECT_EQ(*maybe_intersection == 
                (Point2D{8, 4}), true);
        }
        else
        {
            EXPECT_EQ((bool)maybe_intersection, 
                true);
        }
    }

    // Одна точка пересечения, касающиеся круги
    {
        Circle c1{Point2D{3, 4}, 5};
        Circle c2{Point2D{10, 4}, 2};

        std::optional maybe_intersection = 
            GetIntersectPoint(c1, c2);

        if ((bool)maybe_intersection)
        {
            EXPECT_EQ(*maybe_intersection == 
                (Point2D{8, 4}), true);
        }
        else
        {
            EXPECT_EQ((bool)maybe_intersection, 
                true);
        }

        maybe_intersection = 
            GetIntersectPoint(c2, c1);

        if ((bool)maybe_intersection)
        {
            EXPECT_EQ(*maybe_intersection == 
                (Point2D{8, 4}), true);
        }
        else
        {
            EXPECT_EQ((bool)maybe_intersection, 
                true);
        }
    }

    // Пересекающиеся круги
    {
        Circle c1{Point2D{2, 10}, 5};
        Circle c2{Point2D{7, 9}, 1};

        std::optional maybe_intersection = 
            GetIntersectPoint(c1, c2);

        if ((bool)maybe_intersection)
        {
            EXPECT_EQ(*maybe_intersection == 
                (Point2D{7, 10}), true);
        }
        else
        {
            EXPECT_EQ((bool)maybe_intersection, 
                true);
        }

        /* Просто поменять местами нельзя из-за 
        выбора самой первой точки пересечения по 
        ходу часовой стрелки */
        c1 = {Point2D{12, 10}, 5};
        c2 = {Point2D{7, 9}, 1};

        maybe_intersection = 
            GetIntersectPoint(c2, c1);

        if ((bool)maybe_intersection)
        {
            EXPECT_EQ(maybe_intersection.value() == 
                (Point2D{7, 10}), true);
        }
        else
        {
            EXPECT_EQ((bool)maybe_intersection, 
                true);
        }
    }

    // Непересекающиеся круги
    {
        Circle c1{Point2D{1, 3}, 8};
        Circle c2{Point2D{14, 4}, 1};

        std::optional maybe_intersection = 
            GetIntersectPoint(c1, c2);

        EXPECT_EQ(!maybe_intersection, true);

        maybe_intersection = 
            GetIntersectPoint(c2, c1);

        EXPECT_EQ(!maybe_intersection, true);
    }
}

TEST(Intersections_Tests, CircleLine)
{
    using namespace geometry;
    using namespace intersections;

    // Одна точка пересечения, против часовой стрелки
    {
        Circle circle{Point2D{3, 4}, 5};
        Line line{Point2D{1, 9}, Point2D{5, 9}};

        std::optional maybe_intersection = 
            GetIntersectPoint(circle, line);

        if ((bool)maybe_intersection)
        {
            EXPECT_EQ(*maybe_intersection == 
                (Point2D{3, 9}), true);
        }
        else
        {
            EXPECT_EQ((bool)maybe_intersection, 
                true);
        }

        maybe_intersection = 
            GetIntersectPoint(line, circle);

        if ((bool)maybe_intersection)
        {
            EXPECT_EQ(*maybe_intersection == 
                (Point2D{3, 9}), true);
        }
        else
        {
            EXPECT_EQ((bool)maybe_intersection, 
                true);
        }
    }

    // Одна точка пересечения, по часовой стрелке
    {
        Circle circle{Point2D{1, 4}, 5};
        Line line{Point2D{2, 2}, {2, -2}};

        std::optional maybe_intersection = 
            GetIntersectPoint(circle, line);

        if ((bool)maybe_intersection)
        {
            EXPECT_EQ((*maybe_intersection).x == 2, true);
            EXPECT_EQ((*maybe_intersection).y < 0, true);
        }
        else
        {
            EXPECT_EQ((bool)maybe_intersection, 
                true);
        }

        maybe_intersection = 
            GetIntersectPoint(line, circle);

        if ((bool)maybe_intersection)
        {
            EXPECT_EQ((*maybe_intersection).x == 2, true);
            EXPECT_EQ((*maybe_intersection).y < 0, true);
        }
        else
        {
            EXPECT_EQ((bool)maybe_intersection, 
                true);
        }
    }

    // Пересечение в двух точках
    {
        Circle circle{Point2D{0, 0}, 5};
        Line line{Point2D{-1, 6}, Point2D{6, -1}};

        std::optional maybe_intersection = 
            GetIntersectPoint(circle, line);

        if ((bool)maybe_intersection)
        {
            EXPECT_EQ(*maybe_intersection == 
                (Point2D{0, 5}), true);
        }
        else
        {
            EXPECT_EQ((bool)maybe_intersection, 
                true);
        }

        maybe_intersection = 
            GetIntersectPoint(circle, line);

        if ((bool)maybe_intersection)
        {
            EXPECT_EQ(maybe_intersection.value() == 
                (Point2D{0, 5}), true);
        }
        else
        {
            EXPECT_EQ((bool)maybe_intersection, 
                true);
        }
    }

    // Нет пересечений
    {
        Circle circle{Point2D{0, 0}, 8};
        Line line{Point2D{-1, 100}, Point2D{100, -1}};

        std::optional maybe_intersection = 
            GetIntersectPoint(circle, line);

        EXPECT_EQ(!maybe_intersection, true);

        maybe_intersection = 
            GetIntersectPoint(line, circle);

        EXPECT_EQ(!maybe_intersection, true);
    }
}