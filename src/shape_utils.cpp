#include "shape_utils.hpp"
#include "queries.hpp"
#include "utils.hpp"

#include <ranges>
#include <functional>

namespace geometry::utils
{
    // Разбивает строку на слова (по пробелам), игнорируя лишние пробелы
    std::vector<std::string_view> SplitIntoWords(std::string_view sv)
    {
        std::vector<std::string_view> words;
        size_t start = 0;
        size_t end = 0;

        while (start < sv.size())
        {
            // Пропускаем пробелы
            while (start < sv.size() && std::isspace(static_cast<unsigned char>(sv[start]))) ++start;
            if (start >= sv.size()) break;
            end = start;
            while (end < sv.size() && !std::isspace(static_cast<unsigned char>(sv[end]))) ++end;
            words.push_back(sv.substr(start, end - start));
            start = end;
        }
        return words;
    }

    // Безопасный парсинг строки в double (без исключений)
    std::optional<double> ParseDouble(std::string_view s)
    {
        double value = 0.0;
        auto result = std::from_chars(s.data(), s.data() + s.size(), value);
        if (result.ec == std::errc{} && result.ptr == s.data() + s.size())
        {
            return value;
        }
        return std::nullopt;
    }

    // Парсит строку в вектор double
    std::optional<std::vector<double>> ParseDoubles(std::string_view s)
    {
        if (s.empty()) return std::nullopt;

        std::vector<std::string_view> tokens = 
            SplitIntoWords(s);

        if (tokens.empty()) return std::nullopt;

        std::vector<double> result;
        result.reserve(tokens.size());

        for (std::string_view token : tokens)
        {
            std::optional<double> num = ParseDouble(token);

            if (!num) return std::nullopt;
            result.push_back(*num);
        }
        return result;
    }

    // Проверяет размер вектора и возвращает его, если совпадает
    std::optional<std::vector<double>> 
    RequireSize(const std::vector<double>& v, size_t expected)
    {
        return (v.size() == expected) 
            ? std::make_optional(v) 
            : std::nullopt;
    }

    // Проверяет, что значение > 0
    std::optional<double> RequirePositive(double x)
    {
        return (x > 0) ? std::make_optional(x) : std::nullopt;
    }

    // Проверяет, что double представляет целое число >= min_value
    std::optional<int> RequireIntegerAtLeast(double d, int min_value)
    {
        int i = static_cast<int>(d);
        if (static_cast<double>(i) == d && 
            i >= min_value) return i;
        return std::nullopt;
    }

    // Конструкторы фигур

    /**
        @brief Создаёт круг из параметров
        @note Пример того как могла бы выглядеть эта функция:
            if (v.size() != 3) return std::nullopt;
            if (v[2] <= 0) return std::nullopt; // радиус должен быть > 0
            return Circle{{v[0], v[1]}, v[2]};
    */
    std::optional<Shape> MakeCircle(const std::vector<double>& v)
    {
        if (v.size() != 3) return std::nullopt;
        if (!DblGreater{}(v.back(), 0)) return std::nullopt;
        
        return Circle{ Point2D{ v[0], v[1] }, v[2] };
    }

    /**
        @brief Создаёт линию из параметров
        @note Пример того как могла бы выглядеть эта функция:
            if (v.size() != 4) return std::nullopt;
            return Line{{v[0], v[1]}, {v[2], v[3]}};
    */
    std::optional<Shape> MakeLine(const std::vector<double>& v)
    {
        if (v.size() != 4) return std::nullopt;
        
        return Line{ Point2D{ v[0], v[1] }, 
            Point2D{ v[2], v[3] } };
    }

    /**
        @brief Создаёт треугольник из параметров
        @note Пример того как могла бы выглядеть эта функция:
            if (v.size() != 6) return std::nullopt;
            return Triangle{{v[0], v[1]}, {v[2], v[3]}, {v[4], v[5]}};
    */
    std::optional<Shape> MakeTriangle(const std::vector<double>& v)
    {
        if (v.size() != 6) return std::nullopt;
        
        return Triangle{ Point2D{ v[0], v[1] }, 
            Point2D{ v[2], v[3] }, Point2D{ v[4], v[5] } };
    }

    /**
        @brief Создаёт прямоугольник из параметров
        @note Пример того как могла бы выглядеть эта функция:
            if (v.size() != 4) return std::nullopt;
            if (v[2] <= 0 || v[3] <= 0) return std::nullopt; // ширина/высота > 0
            return Rectangle{{v[0], v[1]}, v[2], v[3]};
    */
    std::optional<Shape> MakeRectangle(const std::vector<double>& v)
    {
        if (v.size() != 4) return std::nullopt;
        if (!DblGreater{}(v[2], 0)) return std::nullopt;
        if (!DblGreater{}(v[3], 0)) return std::nullopt;
        
        return Rectangle{ Point2D{ v[0], v[1] }, 
            v[2], v[3] };
    }

    /**
        @brief Создаёт правильный многоугольник из параметров
        @note Пример того как могла бы выглядеть эта функция:
            if (v.size() != 4) return std::nullopt;
            if (v[2] <= 0) return std::nullopt; // радиус > 0
            auto sides_opt = parse_double(std::to_string(v[3])); // v[3] — double, но sides — целое
            if (!sides_opt.has_value()) return std::nullopt;
            int sides = static_cast<int>(v[3]);
            if (sides != v[3] || sides < 3) return std::nullopt; // должно быть целым и >=3
            return RegularPolygon{{v[0], v[1]}, v[2], sides};
    */
    std::optional<Shape> MakePolygon(const std::vector<double>& v)
    {
        if (v.size() != 4) return std::nullopt;
        if (!DblGreater{}(v[2], 0)) return std::nullopt;

        // Количество сторон
        if (!DblGreater{}(v[3], 0)) return std::nullopt;
        if (DblGreater{}(std::abs(std::round(v[3]) - v[3]), 
            0)) return std::nullopt;
        int n_edges = (int)std::round(v[3]);
        
        return RegularPolygon{ Point2D{ v[0], v[1] }, 
            v[2], n_edges };
    }

    // Парсинг одной фигуры
    std::optional<Shape> ParseSingleShape(std::string_view token)
    {
        std::vector<std::string_view> parts = 
            SplitIntoWords(token);
        if (parts.empty()) return std::nullopt;

        std::string_view type = parts[0];
        std::string param_str;
        for (auto i : std::views::iota(1u, parts.size()))
        {
            if (!param_str.empty()) param_str += ' ';
            param_str += std::string(parts[i]);
        }

        using ShapeGenerator = 
            std::function<std::optional<Shape>(const std::vector<double>&)>;
        
        // Выбираем конструктор по имени
        decltype([](std::string_view type) -> std::optional<ShapeGenerator>
        {
            if (type == "circle") return MakeCircle;
            if (type == "line") return MakeLine;
            if (type == "triangle") return MakeTriangle;
            if (type == "rectangle") return MakeRectangle;
            if (type == "polygon") return MakePolygon;
            return std::nullopt;
        }) GetMaker;

        //Обратите внимание на код ниже
        return GetMaker(type)
            .and_then([&]<typename Fn>(Fn maker)
                {
                    return ParseDoubles(param_str)
                        .and_then(maker);
                });
    }

    std::vector<Shape> ParseShapes(std::string_view input)
    {
        std::vector<Shape> result;

        // Разделяем по ';'
        size_t start = 0;
        size_t end = 0;
        while (start < input.size())
        {
            end = input.find(';', start);
            if (end == std::string_view::npos) end = input.size();

            std::string_view token = input.substr(start, end - start);
            // Убираем пробелы по краям
            while (!token.empty() && std::isspace(static_cast<unsigned char>(token.front()))) token.remove_prefix(1);
            while (!token.empty() && std::isspace(static_cast<unsigned char>(token.back()))) token.remove_suffix(1);

            if (!token.empty())
            {
                auto shape_opt = ParseSingleShape(token);
                if (shape_opt.has_value())
                {
                    result.push_back(*shape_opt);
                }
            }

            start = end + 1;
        }

        return result;
    }

    std::vector<std::pair<Shape, Shape>> FindAllCollisions(std::span<const Shape> shapes)
    {
        using namespace std::ranges;
        using namespace std::ranges::views;

        size_t n_shapes = shapes.size();
        
        return cartesian_product(views::iota(0, (int)n_shapes), 
            views::iota(0, (int)n_shapes)) | 
            // Отсев членов вне верхнетреугольной области
            views::filter([](const std::tuple<int, int>& ixs)
                {
                    return std::get<1>(ixs) > std::get<0>(ixs);
                }) | 
            // Отсев непересекающихся тел
            views::filter([&shapes](const std::tuple<int, int>& ixs)
                {
                    using namespace queries;
                    return BoundingBoxesOverlap(shapes[std::get<0>(ixs)], 
                        shapes[std::get<1>(ixs)]);
                }) | 
            // От индексов к телам
            views::transform([&shapes](const std::tuple<int, int>& ixs)
                {
                    return std::make_pair(shapes[std::get<0>(ixs)], 
                        shapes[std::get<1>(ixs)]);
                }) | to<std::vector<std::pair<Shape, Shape>>>();
    }

    template <typename Fn>
    std::optional<size_t> 
    FindOptimalShape(std::span<const Shape> shapes, Fn fn)
    {
        using namespace queries;
        using namespace std::ranges;

        if (!shapes.size()) return std::nullopt;

        return [&shapes, &fn]()
            {
                size_t out = 0;
                double opt_height = GetHeight(shapes.front());

                for (const std::tuple<size_t, Shape>& data : 
                    shapes | views::enumerate)
                {
                    double curr_height = GetHeight(std::get<1>(data));
                    bool update = fn(curr_height, 
                        opt_height);
                    
                    out = update ? std::get<0>(data) : out;
                    opt_height = update ? curr_height : opt_height;
                }

                return out;
            }();
    }
    
    std::optional<size_t> 
    FindHighestShape(std::span<const Shape> shapes)
    {
        return FindOptimalShape(shapes, DblGreater{});
    }

    std::optional<size_t> 
    FindLowestShape(std::span<const Shape> shapes)
    {
        return FindOptimalShape(shapes, DblLess{});
    }
}