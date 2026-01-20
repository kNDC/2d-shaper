#include <gtest/gtest.h>

#include "geometry.hpp"

#include <sstream>
#include <print>

TEST(GeometryFormat_Tests, PointFormatter)
{
    using namespace geometry;
    using namespace std::string_literals;

    Point2D p{ 0.1, 3.888 };

    std::stringstream oss{};
    std::print(oss, "text{}text", p);
    EXPECT_EQ(oss.str(), "text(0.10; 3.89)text"s);
}

TEST(GeometryFormat_Tests, PointVecFormatter)
{
    using namespace geometry;
    using namespace std::string_literals;
    using namespace std::string_view_literals;

    std::vector<Point2D> vec;
    vec.emplace_back(0.0, 1.0);
    vec.emplace_back(1.0, 0.0);
    vec.emplace_back(2.0, 1.0);
    vec.emplace_back(3.0, 4.0);

    // {}
    {
        std::stringstream oss{};
        std::print(oss, "text{}text"sv, vec);

        EXPECT_EQ(oss.str(), 
            "text(0.00; 1.00) (1.00; 0.00) (2.00; 1.00) (3.00; 4.00)text"s);
    }

    // {}, пустой вектор
    {
        std::stringstream oss{};
        std::print(oss, "text{}text"sv, 
            std::vector<Point2D>{});

        EXPECT_EQ(oss.str(), 
            "texttext"s);
    }

    // {:new_line}
    {
        std::stringstream oss{};
        std::print(oss, "text{:new_line}text", vec);

        EXPECT_EQ(oss.str(), 
            "text\n"
            "\t(0.00; 1.00)\n"
            "\t(1.00; 0.00)\n"
            "\t(2.00; 1.00)\n"
            "\t(3.00; 4.00)\n"
            "text"s);
    }

    // {:new_line}, пустой текст
    {
        std::stringstream oss{};
        std::print(oss, "text{:new_line}text", 
            std::vector<Point2D>{});

        EXPECT_EQ(oss.str(), 
            "texttext"s);
    }

    /* {:invalid_format} - не скомпилируется
    {
        std::stringstream oss{};

        EXPECT_THROW(std::print(oss, "text{:invalid_format}text", vec), 
            std::format_error);
    }*/
}