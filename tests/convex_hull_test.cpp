#include <gtest/gtest.h>
#include "convex_hull.hpp"

TEST(ConvexHull_Tests, ErrorHandling)
{
    using namespace geometry;
    using namespace convex_hull;

    std::vector<Point2D> points{{0, 0}, {1, 1}, {2, 2}};
    std::expected<std::vector<Point2D>, std::logic_error> result;

    EXPECT_NO_THROW(result = GrahamScan(points));
    EXPECT_EQ((bool)result, true);

    points.pop_back();
    EXPECT_NO_THROW(result = GrahamScan(points));
    EXPECT_EQ(!result, true);
    EXPECT_EQ((std::is_same_v<decltype(result.error()), std::logic_error&>), true);
    
    points.pop_back();
    EXPECT_NO_THROW(result = GrahamScan(points));
    EXPECT_EQ(!result, true);
    EXPECT_EQ((std::is_same_v<decltype(result.error()), std::logic_error&>), true);
}