/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 05/06/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#include "ThemeParser.hpp"
#include "QtCore/qeasingcurve.h"
#include "QtCore/qnamespace.h"
#include "ThemeInterface.hpp"

#include <algorithm>
#include <fstream>
#include <optional>
#include <stdexcept>

#include <pugixml.hpp>

#include <QPainter>

#include <string>
#include <type_traits>

namespace
{
    constexpr std::array<std::tuple<std::string_view, int>, 6> _render_hints_opts = {
        {{ "antialias", QPainter::RenderHint::Antialiasing },
         { "lossless", QPainter::RenderHint::LosslessImageRendering },
         { "non-cosmetic", QPainter::RenderHint::NonCosmeticBrushPatterns },
         { "smooth", QPainter::RenderHint::SmoothPixmapTransform },
         { "text-antialias", QPainter::RenderHint::TextAntialiasing },
         { "vertical", QPainter::RenderHint::VerticalSubpixelPositioning }}
    };

    enum class BrushFormats : int64_t
    {
        Regular,
        LinearGradient,
        ConicalGradient,
        RadialGradient
    };

    constexpr std::array<std::tuple<std::string_view, BrushFormats>, 4> _brush_formats_opts = {
        {{ "regular", BrushFormats::Regular },
         { "linear-gradient", BrushFormats::LinearGradient },
         { "conical-gradient", BrushFormats::ConicalGradient },
         { "radial-gradient", BrushFormats::RadialGradient }}
    };

    constexpr std::array<std::tuple<std::string_view, Qt::BrushStyle>, 15> _brush_pattern_opts = {
        {{ "no-brush", Qt::BrushStyle::NoBrush },
         { "solid", Qt::BrushStyle::SolidPattern },
         { "dense-1", Qt::BrushStyle::Dense1Pattern },
         { "dense-2", Qt::BrushStyle::Dense2Pattern },
         { "dense-3", Qt::BrushStyle::Dense3Pattern },
         { "dense-4", Qt::BrushStyle::Dense4Pattern },
         { "dense-5", Qt::BrushStyle::Dense5Pattern },
         { "dense-6", Qt::BrushStyle::Dense6Pattern },
         { "dense-7", Qt::BrushStyle::Dense7Pattern },
         { "hor", Qt::BrushStyle::HorPattern },
         { "ver", Qt::BrushStyle::VerPattern },
         { "cross", Qt::BrushStyle::CrossPattern },
         { "b-diag", Qt::BrushStyle::BDiagPattern },
         { "f-diag", Qt::BrushStyle::FDiagPattern },
         { "diag-cross", Qt::BrushStyle::DiagCrossPattern }}
    };

    constexpr std::array<std::tuple<std::string_view, QGradient::Spread>, 3> _brush_spread_opts = {
        {{ "pad", QGradient::PadSpread },
         { "repeat", QGradient::RepeatSpread },
         { "reflect", QGradient::ReflectSpread }}
    };

    constexpr std::array<std::tuple<std::string_view, Qt::PenStyle>, 6> _pen_style_opts = {
        {{ "solid", Qt::SolidLine },
         { "dash", Qt::DashLine },
         { "dot", Qt::DotLine },
         { "dash-dot", Qt::DashDotLine },
         { "dash-dot-dot", Qt::DashDotDotLine },
         { "custom", Qt::CustomDashLine }}
    };

    constexpr std::array<std::tuple<std::string_view, Qt::PenCapStyle>, 3> _pen_cap_style_opts = {
        {{ "square", Qt::SquareCap },
         { "flat", Qt::FlatCap },
         { "round", Qt::RoundCap }}
    };

    constexpr std::array<std::tuple<std::string_view, Qt::PenJoinStyle>, 4> _pen_join_style_opts = {
        {{ "bevel", Qt::BevelJoin },
         { "miter", Qt::MiterJoin },
         { "round", Qt::RoundJoin },
         { "svg-miter", Qt::SvgMiterJoin }}
    };

    constexpr std::array<std::tuple<std::string_view, QEasingCurve::Type>, 46> _easing_curve_opts = {
        {{ "linear", QEasingCurve::Linear },
         { "in-quad", QEasingCurve::InQuad },
         { "out-quad", QEasingCurve::OutQuad },
         { "in-out-quad", QEasingCurve::InOutQuad },
         { "out-in-quad", QEasingCurve::OutInQuad },
         { "in-cubic", QEasingCurve::InCubic },
         { "out-cubic", QEasingCurve::OutCubic },
         { "in-out-cubic", QEasingCurve::InOutCubic },
         { "out-in-cubic", QEasingCurve::OutInCubic },
         { "in-quart", QEasingCurve::InQuart },
         { "out-quart", QEasingCurve::OutQuart },
         { "in-out-quart", QEasingCurve::InOutQuart },
         { "out-in-quart", QEasingCurve::OutInQuart },
         { "in-quint", QEasingCurve::InQuint },
         { "out-quint", QEasingCurve::OutQuint },
         { "in-out-quint", QEasingCurve::InOutQuint },
         { "out-in-quint", QEasingCurve::OutInQuint },
         { "in-sine", QEasingCurve::InSine },
         { "out-sine", QEasingCurve::OutSine },
         { "in-out-sine", QEasingCurve::InOutSine },
         { "out-in-sine", QEasingCurve::OutInSine },
         { "in-expo", QEasingCurve::InExpo },
         { "out-expo", QEasingCurve::OutExpo },
         { "in-out-expo", QEasingCurve::InOutExpo },
         { "out-in-expo", QEasingCurve::OutInExpo },
         { "in-circ", QEasingCurve::InCirc },
         { "out-circ", QEasingCurve::OutCirc },
         { "in-out-circ", QEasingCurve::InOutCirc },
         { "out-in-circ", QEasingCurve::OutInCirc },
         { "in-elastic", QEasingCurve::InElastic },
         { "out-elastic", QEasingCurve::OutElastic },
         { "in-out-elastic", QEasingCurve::InOutElastic },
         { "out-in-elastic", QEasingCurve::OutInElastic },
         { "in-back", QEasingCurve::InBack },
         { "out-back", QEasingCurve::OutBack },
         { "in-out-back", QEasingCurve::InOutBack },
         { "out-in-back", QEasingCurve::OutInBack },
         { "in-bounce", QEasingCurve::InBounce },
         { "out-bounce", QEasingCurve::OutBounce },
         { "in-out-bounce", QEasingCurve::InOutBounce },
         { "out-in-bounce", QEasingCurve::OutInBounce },
         { "in-curve", QEasingCurve::InCurve },
         { "out-curve", QEasingCurve::OutCurve },
         { "sine-curve", QEasingCurve::SineCurve },
         { "cosine-curve", QEasingCurve::CosineCurve }}
    };
} // namespace

struct theme::ThemeParser::Impl
{
    theme::ThemeParser *parser { nullptr };

    void parseColors(const pugi::xml_node &root);
    void parseBrushes(const pugi::xml_node &root);
    void parsePens(const pugi::xml_node &root);
    void parseAnimations(const pugi::xml_node &root);

    static auto findNode(const pugi::xml_node &node, const char *name)
    {
        return node.find_child([&name](const pugi::xml_node &n) -> bool {
            if (strcmp(n.name(), name) == 0)
                return true;
            return false;
        });
    }

    template <typename T>
    auto _to_integer(const std::string &str) -> std::optional<T>
    {
        try
        {
            static_assert(std::is_same<T, double>::value || std::is_same<T, int>::value || std::is_same<T, float>::value);

            std::size_t er_pos;
            T ret;

            if constexpr (std::is_same_v<T, double>)
                ret = std::stod(str, &er_pos);
            else if constexpr (std::is_same_v<T, float>)
                ret = std::stof(str, &er_pos);
            else
                ret = std::stoi(str, &er_pos);

            if (er_pos != str.size())
                return std::nullopt;

            return ret;

        } catch (C_UNUSED const std::exception &ex)
        {
            return std::nullopt;
        }
    }

    auto parseDouble(const char *p) -> double
    {
        if (p == nullptr)
            throw std::runtime_error("invalid empty string conversion");

        auto ret = _to_integer<double>(p);

        if (!ret.has_value())
            throw std::runtime_error("invalid floating point integer");

        return *ret;
    }

    auto parseInteger(const char *p) -> int
    {
        if (p == nullptr)
            throw std::runtime_error("invalid empty string conversion");

        auto ret = _to_integer<int>(p);

        if (!ret.has_value())
            throw std::runtime_error("invalid integer");

        return *ret;
    }

    auto getColor(const char *p) const -> QColor
    {
        using namespace std::string_literals;
        auto iter = parser->scheme.colors.find(p);
        if (iter == parser->scheme.colors.end())
            throw std::runtime_error("color '"s + p + "' is not declared");
        return iter->second;
    }

    auto getBrush(const char *p) const -> QBrush
    {
        using namespace std::string_literals;
        auto iter = parser->scheme.brushes.find(p);
        if (iter == parser->scheme.brushes.end())
            throw std::runtime_error("brush '"s + p + "' is not declared");
        return iter->second;
    }
};

theme::ThemeParser::ThemeParser() :
    _impl { std::make_unique<Impl>() }
{
    _impl->parser = this;
}

theme::ThemeParser::~ThemeParser() = default;

void theme::ThemeParser::loadTheme(const std::string &file)
{
    using namespace std::string_literals;
    namespace xml = pugi;

    scheme.colors.clear();
    scheme.brushes.clear();
    scheme.pens.clear();

    xml::xml_document doc;

    const auto result = doc.load_file(file.c_str());

    if (result.status != xml::xml_parse_status::status_ok)
        throw std::runtime_error(result.description());

    const auto &root = doc.first_child();
    if (strcmp(root.name(), "theme") != 0)
        throw std::runtime_error("parent node is not 'theme'");

    const auto &name = root.find_attribute([](const xml::xml_attribute &att) -> bool {
        return strcmp(att.name(), "name") == 0;
    });

    if (name == xml::xml_attribute())
        throw std::runtime_error("theme does not have a name");

    const auto &renders = P_IMPL()->findNode(root, "render");

    if (renders == xml::xml_node())
        throw std::runtime_error("theme does not have renders");

    themeScheme = name.value();

    for (const auto &render : renders.children())
    {
        if (strcmp(render.name(), "type") == 0)
        {
            const auto hint = render.first_child().value();
            auto renderHint
                = std::find_if(
                    _render_hints_opts.begin(),
                    _render_hints_opts.end(),
                    [&hint](const auto &v) -> bool {
                        if (std::get<0>(v) == hint)
                            return true;
                        return false;
                    });

            if (renderHint == _render_hints_opts.end())
                throw std::runtime_error("invalid render hint: " + std::string(hint));

            renderHints |= std::get<1>(*renderHint);
        }
        else
            throw std::runtime_error("unknown render node name: " + std::string(render.name()));
    }

    P_IMPL()->parseColors(root);
    P_IMPL()->parseBrushes(root);
    P_IMPL()->parsePens(root);
    P_IMPL()->parseAnimations(root);
}

void theme::ThemeParser::Impl::parseColors(const pugi::xml_node &root)
{
    using namespace std::string_literals;
    namespace xml = pugi;

    auto parseColorNode = [this](const auto &node, auto &var, auto &&funcF, auto &&funcI, const std::string &color_name) {
        if (node != xml::xml_node() && node.first_child() != xml::xml_node())
        {
            const auto v = _to_integer<int>(node.first_child().value());
            if (v.has_value())
                (var.*funcI)(*v);
            else
            {
                const auto i = _to_integer<float>(node.first_child().value());
                if (i.has_value())
                    (var.*funcF)(*i);
                else
                    throw std::runtime_error("color node is not valid in: "s + color_name);
            }
        }
    };

    const auto &colorsNode = findNode(root, "colors");
    if (colorsNode != xml::xml_node())
    {
        for (const auto &color : colorsNode.children())
        {
            if (strcmp(color.name(), "color") != 0)
                throw std::runtime_error("color node '"s + color.name() + "' is not recognized as valid"s);

            const auto &colorNameAttr = color.attribute("name");
            if (colorNameAttr == xml::xml_attribute())
                throw std::runtime_error("color has no name");

            const std::string colorName = colorNameAttr.value();

            const auto &rNode = findNode(color, "r");
            const auto &gNode = findNode(color, "g");
            const auto &bNode = findNode(color, "b");
            const auto &aNode = findNode(color, "a");

            QColor qColor { 0, 0, 0, 255 };

            parseColorNode(rNode, qColor, &QColor::setRedF, &QColor::setRed, colorName);
            parseColorNode(gNode, qColor, &QColor::setGreenF, &QColor::setGreen, colorName);
            parseColorNode(bNode, qColor, &QColor::setBlueF, &QColor::setBlue, colorName);
            parseColorNode(aNode, qColor, &QColor::setAlphaF, &QColor::setAlpha, colorName);

            if (parser->scheme.colors.contains(colorName.c_str()))
                throw std::runtime_error(colorName + " already exists");

            parser->scheme.colors.insert({ colorName.c_str(), qColor });
        }
    }
}

void theme::ThemeParser::Impl::parseBrushes(const pugi::xml_node &root)
{
    using namespace std::string_literals;
    namespace xml = pugi;

    auto parseRegularBrush = [this](const auto &node, const std::string &name) -> void {
        const auto &colorNode = findNode(node, "color");
        if (colorNode == xml::xml_node())
            throw std::runtime_error("brush '"s + name + "' doesn't has a color node");
        const auto &color = colorNode.child_value();
        if (color == nullptr)
            throw std::runtime_error("brush '"s + name + "' doesn't have a valid color node");

        Qt::BrushStyle brushStyle = Qt::BrushStyle::NoBrush;
        const auto &patternNode   = findNode(node, "pattern");
        if (patternNode != xml::xml_node())
        {
            const auto &pattern = patternNode.child_value();
            if (pattern == nullptr)
                throw std::runtime_error("brush '"s + name + "' doesn't have a valid color node");

            auto patternIter = std::find_if(_brush_pattern_opts.begin(),
                _brush_pattern_opts.end(),
                [&pattern](const auto &item) {
                    if (pattern == std::get<0>(item))
                        return true;
                    return false;
                });
            if (patternIter == _brush_pattern_opts.end())
                throw std::runtime_error("brush '"s + name + "' pattern '"s + pattern + "' does not exist"s);

            brushStyle = std::get<1>(*patternIter);
        }

        parser->scheme.brushes.insert({ name.c_str(), QBrush(getColor(color), brushStyle) });
    };

    auto parseLinearGradientBrush = [this](const auto &node, const std::string &name) -> void {
        QPointF start { 0.0, 0.0 }, end { 0.0, 1.0 };

        if (const auto &pointsNode = findNode(node, "points"); pointsNode != xml::xml_node())
        {
            if (const auto &xStartNode = findNode(pointsNode, "x-start"); xStartNode != xml::xml_node())
                start.setX(parseDouble(xStartNode.child_value()));

            if (const auto &yStartNode = findNode(pointsNode, "y-start"); yStartNode != xml::xml_node())
                start.setY(parseDouble(yStartNode.child_value()));

            if (const auto &xEndNode = findNode(pointsNode, "x-end"); xEndNode != xml::xml_node())
                end.setX(parseDouble(xEndNode.child_value()));

            if (const auto &yEndNode = findNode(pointsNode, "y-end"); yEndNode != xml::xml_node())
                end.setY(parseDouble(yEndNode.child_value()));
        }

        QLinearGradient::Spread spread { QLinearGradient::Spread::PadSpread };

        if (const auto &spreadNode = findNode(node, "spread"); spreadNode != xml::xml_node())
        {
            const auto &spreadValue = spreadNode.child_value();

            auto iter = std::find_if(_brush_spread_opts.begin(), _brush_spread_opts.end(), [&spreadValue](const auto &item) {
                if (std::string(spreadValue) == std::get<0>(item))
                    return true;
                return false;
            });

            if (iter == _brush_spread_opts.end())
                throw std::runtime_error("spread '"s + spreadValue + "' is not valid"s);

            spread = std::get<1>(*iter);
        }

        QLinearGradient gd(start, end);
        gd.setCoordinateMode(QGradient::CoordinateMode::ObjectMode);
        gd.setSpread(spread);

        if (const auto &colorsPositionsNode = findNode(node, "colors"); colorsPositionsNode != xml::xml_node())
        {
            for (const auto &color : colorsPositionsNode.children())
            {
                if (strcmp(color.name(), "color") != 0)
                    throw std::runtime_error("node '"s + color.name() + "' is not a valid node in the linear gradient colors node"s);

                const auto &colorNameAttr = color.attribute("name");
                if (colorNameAttr == xml::xml_attribute())
                    throw std::runtime_error("color position in the linear gradient '" + name + "' has no color name");

                gd.setColorAt(parseDouble(color.child_value()), getColor(colorNameAttr.value()));
            }
        }

        parser->scheme.brushes.insert({ name.c_str(), gd });
    };

    auto parseConicalGradientBrush = [this](const auto &node, const std::string &name) -> void {
        QPointF center { 0.0, 0.0 };
        qreal angle { 0.0 };

        if (const auto &centerNode = findNode(node, "center"); centerNode != xml::xml_node())
        {
            if (const auto &xNode = findNode(centerNode, "x"); xNode != xml::xml_node())
                center.setX(parseDouble(xNode.child_value()));

            if (const auto &yNode = findNode(centerNode, "y"); yNode != xml::xml_node())
                center.setY(parseDouble(yNode.child_value()));
        }

        if (const auto &angleNode = findNode(node, "angle"); angleNode != xml::xml_node())
            angle = parseDouble(angleNode.child_value());

        QConicalGradient cg(center, angle);
        cg.setCoordinateMode(QGradient::CoordinateMode::ObjectMode);
        if (const auto &colorsPositionsNode = findNode(node, "colors"); colorsPositionsNode != xml::xml_node())
        {
            for (const auto &color : colorsPositionsNode.children())
            {
                if (strcmp(color.name(), "color") != 0)
                    throw std::runtime_error("node '"s + color.name() + "' is not a valid node in the conical gradient colors node"s);

                const auto &colorNameAttr = color.attribute("name");
                if (colorNameAttr == xml::xml_attribute())
                    throw std::runtime_error("color position in the conical gradient '" + name + "' has no color name");

                cg.setColorAt(parseDouble(color.child_value()), getColor(colorNameAttr.value()));
            }
        }

        parser->scheme.brushes.insert({ name.c_str(), cg });
    };

    auto parseRadialGradientBrush = [this](const auto &node, const std::string &name) -> void {
        QPointF center { 0.0, 0.0 }, focal { 0.0, 0.0 };
        qreal radius { 0.0 }, focalRadius { 0.0 };

        if (const auto &centerNode = findNode(node, "center"); centerNode != xml::xml_node())
        {
            if (const auto &xNode = findNode(centerNode, "x"); xNode != xml::xml_node())
                center.setX(parseDouble(xNode.child_value()));

            if (const auto &yNode = findNode(centerNode, "y"); yNode != xml::xml_node())
                center.setY(parseDouble(yNode.child_value()));
        }

        if (const auto &focalNode = findNode(node, "focal"); focalNode != xml::xml_node())
        {
            if (const auto &xNode = findNode(focalNode, "x"); xNode != xml::xml_node())
                focal.setX(parseDouble(xNode.child_value()));

            if (const auto &yNode = findNode(focalNode, "y"); yNode != xml::xml_node())
                focal.setY(parseDouble(yNode.child_value()));
        }

        if (const auto &radiusNode = findNode(node, "radius"); radiusNode != xml::xml_node())
            radius = parseDouble(radiusNode.child_value());

        if (const auto &focalRadiusNode = findNode(node, "focal-radius"); focalRadiusNode != xml::xml_node())
            focalRadius = parseDouble(focalRadiusNode.child_value());

        QRadialGradient rg(center, radius, focal, focalRadius);
        rg.setCoordinateMode(QGradient::CoordinateMode::ObjectMode);
        if (const auto &colorsPositionsNode = findNode(node, "colors"); colorsPositionsNode != xml::xml_node())
        {
            for (const auto &color : colorsPositionsNode.children())
            {
                if (strcmp(color.name(), "color") != 0)
                    throw std::runtime_error("node '"s + color.name() + "' is not a valid node in the radial gradient colors node"s);

                const auto &colorNameAttr = color.attribute("name");
                if (colorNameAttr == xml::xml_attribute())
                    throw std::runtime_error("color position in the radial gradient '" + name + "' has no color name");

                rg.setColorAt(parseDouble(color.child_value()), getColor(colorNameAttr.value()));
            }
        }

        parser->scheme.brushes.insert({ name.c_str(), rg });
    };

    const auto brushesNode = findNode(root, "brushes");
    if (brushesNode != xml::xml_node())
    {
        for (const auto &brush : brushesNode.children())
        {
            if (strcmp(brush.name(), "brush") != 0)
                throw std::runtime_error("brush node '"s + brush.name() + "' is not recognized as a valid");

            const auto &brushNameAttr = brush.attribute("name");
            if (brushNameAttr == xml::xml_attribute())
                throw std::runtime_error("brush has no name");

            const std::string brushName = brushNameAttr.value();

            const auto &brushFormatAttr = brush.attribute("format");
            if (brushFormatAttr == xml::xml_attribute())
                throw std::runtime_error("brush '" + brushName + "' does not have format");

            const std::string brushFormat = brushFormatAttr.value();

            auto format = std::find_if(_brush_formats_opts.begin(), _brush_formats_opts.end(),
                [&brushFormat](const auto &f) -> bool {
                    if (brushFormat == std::get<0>(f))
                        return true;
                    return false;
                });

            if (format == _brush_formats_opts.end())
            {
                std::string message = "brush format '"s + brushFormat;
                message.append("'in brush: '"s + brushName + "' does not have format"s);
                throw std::runtime_error(message);
            }

            switch (std::get<1>(*format))
            {
                case BrushFormats::Regular:
                    parseRegularBrush(brush, brushName);
                    break;
                case BrushFormats::LinearGradient:
                    parseLinearGradientBrush(brush, brushName);
                    break;
                case BrushFormats::ConicalGradient:
                    parseConicalGradientBrush(brush, brushName);
                    break;
                case BrushFormats::RadialGradient:
                    parseRadialGradientBrush(brush, brushName);
                    break;
            }
        }
    }
}

void theme::ThemeParser::Impl::parsePens(const pugi::xml_node &root)
{
    using namespace std::string_literals;
    namespace xml = pugi;

    const auto &pensNode = findNode(root, "pens");

    for (const auto &pen : pensNode.children())
    {
        if (strcmp(pen.name(), "pen") != 0)
            throw std::runtime_error(pen.name() + "is not a valid node"s);

        const auto &nameAttr  = pen.attribute("name");
        const auto &widthAttr = pen.attribute("width");
        const auto &brushAttr = pen.attribute("brush");

        if (nameAttr == xml::xml_attribute())
            throw std::runtime_error("pen without name");

        const std::string penName { nameAttr.value() };

        const auto width = [&widthAttr, this]() -> qreal {
            if (widthAttr == xml::xml_attribute())
                return 1.0;
            return parseDouble(widthAttr.value());
        }();

        QList<qreal> patternList;
        Qt::PenStyle penStyle = Qt::PenStyle::NoPen;

        if (const auto &styleNode = findNode(pen, "style"); styleNode != xml::xml_node())
        {
            auto styleIter = std::find_if(
                _pen_style_opts.begin(),
                _pen_style_opts.end(),
                [&styleNode](const auto &item) {
                    if (std::string(styleNode.child_value()) == std::get<0>(item))
                        return true;
                    return false;
                });

            if (styleIter == _pen_style_opts.end())
                throw std::runtime_error("pen '"s + penName + "' has an invalid pen style");

            penStyle = std::get<1>(*styleIter);

            if (penStyle == Qt::PenStyle::CustomDashLine)
            {
                const auto &customStylePatternNode = findNode(pen, "patterns");
                if (customStylePatternNode == xml::xml_node())
                    throw std::runtime_error("pen '"s + penName + "' has custom pen style but no patterns node was specified");

                for (const auto &pattern : customStylePatternNode.children())
                {
                    if (strcmp(pattern.name(), "pattern") != 0)
                        throw std::runtime_error("pen '"s + penName + "' invalid patterns inner node");

                    patternList.emplace_back(parseDouble(pattern.child_value()));
                }

                if (patternList.size() % 2)
                    throw std::runtime_error("custom pattern for pen '"s + penName + "' must be an even number");

                if (patternList.isEmpty())
                    throw std::runtime_error("pen '"s + penName + "' custom pattern style without a pattern");
            }
        }

        Qt::PenCapStyle capStyle = Qt::PenCapStyle::FlatCap;
        if (const auto &capNode = findNode(pen, "cap"); capNode != xml::xml_node())
        {
            auto capIter = std::find_if(
                _pen_cap_style_opts.begin(),
                _pen_cap_style_opts.end(),
                [&capNode](const auto &item) {
                    if (std::string(capNode.child_value()) == std::get<0>(item))
                        return true;
                    return false;
                });

            if (capIter == _pen_cap_style_opts.end())
                throw std::runtime_error("pen '"s + penName + "' has an invalid pen cap");

            capStyle = std::get<1>(*capIter);
        }

        qreal miterLimit { 0 };
        Qt::PenJoinStyle joinStyle = Qt::PenJoinStyle::BevelJoin;
        if (const auto &joinNode = findNode(pen, "join"); joinNode != xml::xml_node())
        {
            auto joinIter = std::find_if(
                _pen_join_style_opts.begin(),
                _pen_join_style_opts.end(),
                [&joinNode](const auto &item) {
                    if (std::string(joinNode.child_value()) == std::get<0>(item))
                        return true;
                    return false;
                });

            if (joinIter == _pen_join_style_opts.end())
                throw std::runtime_error("pen '"s + penName + "' has an invalid pen join");

            joinStyle = std::get<1>(*joinIter);
            if (joinStyle == Qt::PenJoinStyle::MiterJoin)
            {
                if (const auto &miterLimitNode = findNode(pen, "miter-limit"); miterLimitNode != xml::xml_node())
                {
                    miterLimit = parseDouble(miterLimitNode.child_value());
                }
            }
        }

        if (parser->scheme.pens.contains(penName.c_str()))
            throw std::runtime_error("pen '"s + penName + "' already declared");

        parser->scheme.pens.insert({ penName.c_str(), [&]() -> QPen {
                                        QPen qPen(getBrush(brushAttr.value()), width, penStyle, capStyle, joinStyle);
                                        if (penStyle == Qt::PenStyle::CustomDashLine)
                                            qPen.setDashPattern(patternList);
                                        if (joinStyle == Qt::PenJoinStyle::MiterJoin)
                                            qPen.setMiterLimit(miterLimit);
                                        return qPen;
                                    }() });
    }
}

void theme::ThemeParser::Impl::parseAnimations(const pugi::xml_node &root)
{
    using namespace std::string_literals;
    namespace xml     = pugi;
    const auto &anims = findNode(root, "animations");
    if (anims == xml::xml_node())
        return;

    for (const auto &anim : anims.children())
    {
        if (strcmp(anim.name(), "animation") != 0)
            throw std::runtime_error("node '"s + anim.name() + "' is not a valid animations child node"s);

        const auto &name = anim.attribute("name");
        if (name == xml::xml_attribute())
            throw std::runtime_error("animation node does not have the attribute name"s);

        const auto &curve    = findNode(anim, "easing-curve");
        const auto &duration = findNode(anim, "duration");

        if (curve == xml::xml_node())
            throw std::runtime_error("animation with name '"s + name.name() + "' does not have the easing curve"s);

        if (duration == xml::xml_node())
            throw std::runtime_error("animation with name '"s + name.name() + "' does not have the duration"s);

        if (parser->uiElements.animations.contains(name.value()))
            throw std::runtime_error("animation name '"s + name.value() + "is already registered"s);

        parser->uiElements.animations.insert({
            name.value(),
            CENTAUR_THEME_INTERFACE_NAMESPACE::ThemeAnimation {
                                                               .easingCurve = [&](const char *p) -> QEasingCurve {
                                                               auto iter = std::find_if(
                                                               _easing_curve_opts.begin(),
                                                               _easing_curve_opts.end(),
                                                               [&](const auto &item) {
                                                               if (strcmp(p, std::get<0>(item).data()) == 0)
                                                               {
                                                               return true;
                                                               }
                                                               return false;
                                                               });

                                                               if (iter == _easing_curve_opts.end())
                                                               throw std::runtime_error("easing curve name '"s + p + "' is not valid");
                                                               return std::get<1>(*iter);
                                                               }(curve.first_child().name()),
                                                               .duration = parseInteger(duration.first_child().value())}
        });
    }
}
