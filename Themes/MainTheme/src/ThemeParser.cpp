/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 05/06/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#include "ThemeParser.hpp"
#include "ThemeInterface.hpp"

#include <algorithm>
#include <memory>
#include <stdexcept>

#include <fmt/core.h>

#include <QApplication>
#include <QDomDocument>
#include <QFile>
#include <QPainter>

#include <string>

#define NODE_ITERATOR(node, root) for (auto node = (root).firstChild(); !node.isNull(); node = node.nextSibling())
#define NODE_ELEMENT(node, var) \
    if (!(node).isElement())    \
        continue;               \
    const auto &(var) = node.toElement();

#define NODE_ATTRIBUTE(node, var) \
    if (!(node).isAttr())         \
        return;                   \
    const auto &(var) = node.toAttr();

#define NODE_ATTRIBUTE_VAL(node, var) \
    if (!(node).isAttr())             \
        return {};                    \
    const auto &(var) = node.toAttr();

namespace
{
    constexpr int maxWeightValue   = 1000;
    constexpr int maxStretchFactor = 4000;

    constexpr std::array<std::tuple<QStringView, QPainter::RenderHint>, 6> _render_hints_opts = {
        {{ u"antialias", QPainter::RenderHint::Antialiasing }, { u"lossless", QPainter::RenderHint::LosslessImageRendering },
         { u"non-cosmetic", QPainter::RenderHint::NonCosmeticBrushPatterns },
         { u"smooth", QPainter::RenderHint::SmoothPixmapTransform }, { u"text-antialias", QPainter::RenderHint::TextAntialiasing },
         { u"vertical", QPainter::RenderHint::VerticalSubpixelPositioning }}
    };

    enum class BrushFormats : int64_t
    {
        Regular,
        LinearGradient,
        ConicalGradient,
        RadialGradient
    };

    constexpr std::array<std::tuple<QStringView, BrushFormats>, 4> _brush_formats_opts = {
        {{ u"regular", BrushFormats::Regular }, { u"linear-gradient", BrushFormats::LinearGradient },
         { u"conical-gradient", BrushFormats::ConicalGradient }, { u"radial-gradient", BrushFormats::RadialGradient }}
    };

    constexpr std::array<std::tuple<QStringView, Qt::BrushStyle>, 15> _brush_pattern_opts = {
        {{ u"no-brush", Qt::BrushStyle::NoBrush }, { u"solid", Qt::BrushStyle::SolidPattern },
         { u"dense-1", Qt::BrushStyle::Dense1Pattern }, { u"dense-2", Qt::BrushStyle::Dense2Pattern },
         { u"dense-3", Qt::BrushStyle::Dense3Pattern }, { u"dense-4", Qt::BrushStyle::Dense4Pattern },
         { u"dense-5", Qt::BrushStyle::Dense5Pattern }, { u"dense-6", Qt::BrushStyle::Dense6Pattern },
         { u"dense-7", Qt::BrushStyle::Dense7Pattern }, { u"hor", Qt::BrushStyle::HorPattern },
         { u"ver", Qt::BrushStyle::VerPattern }, { u"cross", Qt::BrushStyle::CrossPattern },
         { u"b-diag", Qt::BrushStyle::BDiagPattern }, { u"f-diag", Qt::BrushStyle::FDiagPattern },
         { u"diag-cross", Qt::BrushStyle::DiagCrossPattern }}
    };

    constexpr std::array<std::tuple<QStringView, QGradient::Spread>, 3> _brush_spread_opts = {
        {{ u"pad", QGradient::PadSpread }, { u"repeat", QGradient::RepeatSpread }, { u"reflect", QGradient::ReflectSpread }}
    };

    constexpr std::array<std::tuple<QStringView, Qt::PenStyle>, 6> _pen_style_opts = {
        {{ u"solid", Qt::SolidLine }, { u"dash", Qt::DashLine }, { u"dot", Qt::DotLine }, { u"dash-dot", Qt::DashDotLine },
         { u"dash-dot-dot", Qt::DashDotDotLine }, { u"custom", Qt::CustomDashLine }}
    };

    constexpr std::array<std::tuple<QStringView, Qt::PenCapStyle>, 3> _pen_cap_style_opts = {
        {{ u"square", Qt::SquareCap }, { u"flat", Qt::FlatCap }, { u"round", Qt::RoundCap }}
    };

    constexpr std::array<std::tuple<QStringView, Qt::PenJoinStyle>, 4> _pen_join_style_opts = {
        {{ u"bevel", Qt::BevelJoin }, { u"miter", Qt::MiterJoin }, { u"round", Qt::RoundJoin }, { u"svg-miter", Qt::SvgMiterJoin }}
    };

    constexpr std::array<std::tuple<QStringView, QEasingCurve::Type>, 45> _easing_curve_opts = {
        {{ u"linear", QEasingCurve::Linear }, { u"in-quad", QEasingCurve::InQuad }, { u"out-quad", QEasingCurve::OutQuad },
         { u"in-out-quad", QEasingCurve::InOutQuad }, { u"out-in-quad", QEasingCurve::OutInQuad },
         { u"in-cubic", QEasingCurve::InCubic }, { u"out-cubic", QEasingCurve::OutCubic },
         { u"in-out-cubic", QEasingCurve::InOutCubic }, { u"out-in-cubic", QEasingCurve::OutInCubic },
         { u"in-quart", QEasingCurve::InQuart }, { u"out-quart", QEasingCurve::OutQuart },
         { u"in-out-quart", QEasingCurve::InOutQuart }, { u"out-in-quart", QEasingCurve::OutInQuart },
         { u"in-quint", QEasingCurve::InQuint }, { u"out-quint", QEasingCurve::OutQuint },
         { u"in-out-quint", QEasingCurve::InOutQuint }, { u"out-in-quint", QEasingCurve::OutInQuint },
         { u"in-sine", QEasingCurve::InSine }, { u"out-sine", QEasingCurve::OutSine }, { u"in-out-sine", QEasingCurve::InOutSine },
         { u"out-in-sine", QEasingCurve::OutInSine }, { u"in-expo", QEasingCurve::InExpo }, { u"out-expo", QEasingCurve::OutExpo },
         { u"in-out-expo", QEasingCurve::InOutExpo }, { u"out-in-expo", QEasingCurve::OutInExpo },
         { u"in-circ", QEasingCurve::InCirc }, { u"out-circ", QEasingCurve::OutCirc }, { u"in-out-circ", QEasingCurve::InOutCirc },
         { u"out-in-circ", QEasingCurve::OutInCirc }, { u"in-elastic", QEasingCurve::InElastic },
         { u"out-elastic", QEasingCurve::OutElastic }, { u"in-out-elastic", QEasingCurve::InOutElastic },
         { u"out-in-elastic", QEasingCurve::OutInElastic }, { u"in-back", QEasingCurve::InBack },
         { u"out-back", QEasingCurve::OutBack }, { u"in-out-back", QEasingCurve::InOutBack },
         { u"out-in-back", QEasingCurve::OutInBack }, { u"in-bounce", QEasingCurve::InBounce },
         { u"out-bounce", QEasingCurve::OutBounce }, { u"in-out-bounce", QEasingCurve::InOutBounce },
         { u"out-in-bounce", QEasingCurve::OutInBounce }, { u"in-curve", QEasingCurve::InCurve },
         { u"out-curve", QEasingCurve::OutCurve }, { u"sine-curve", QEasingCurve::SineCurve },
         { u"cosine-curve", QEasingCurve::CosineCurve }}
    };

    constexpr std::array<std::tuple<QStringView, QFont::Weight>, 9> _font_weights_opts = {
        {{ u"thin", QFont::Thin }, { u"extra-light", QFont::ExtraLight }, { u"light", QFont::Light }, { u"normal", QFont::Normal },
         { u"medium", QFont::Medium }, { u"demi-bold", QFont::DemiBold }, { u"bold", QFont::Bold },
         { u"extra-bold", QFont::ExtraBold }, { u"black", QFont::Black }}
    };

    constexpr std::array<std::tuple<QStringView, QFont::Capitalization>, 5> _font_caps_opts = {
        {{ u"mixed-case", QFont::MixedCase }, { u"all-uppercase", QFont::AllUppercase }, { u"all-lowercase", QFont::AllLowercase },
         { u"small-caps", QFont::SmallCaps }, { u"capitalize", QFont::Capitalize }}
    };

    constexpr std::array<std::tuple<QStringView, QFont::Stretch>, 10> _font_stretch_opts = {
        {{ u"any", QFont::AnyStretch }, { u"ultra-condensed", QFont::UltraCondensed }, { u"extra-condensed", QFont::ExtraCondensed },
         { u"condensed", QFont::Condensed }, { u"semi-condensed", QFont::SemiCondensed }, { u"unstretched", QFont::Unstretched },
         { u"semi-expanded", QFont::SemiExpanded }, { u"expanded", QFont::Expanded }, { u"extra-expanded", QFont::ExtraExpanded },
         { u"ultra-expanded", QFont::UltraExpanded }}
    };

    constexpr std::array<std::tuple<QStringView, Qt::Alignment>, 4> _font_horz_alignment_opts = {
        {{ u"left", Qt::AlignLeft }, { u"right", Qt::AlignRight }, { u"center", Qt::AlignHCenter }, { u"justify", Qt::AlignJustify }}
    };

    constexpr std::array<std::tuple<QStringView, Qt::Alignment>, 4> _font_vert_alignment_opts = {
        {{ u"top", Qt::AlignTop }, { u"bottom", Qt::AlignBottom }, { u"center", Qt::AlignVCenter },
         { u"baseline", Qt::AlignBaseline }}
    };

    constexpr std::array<std::tuple<QStringView, QTextOption::WrapMode>, 5> _font_word_wrap_opts = {
        {{ u"none", QTextOption::NoWrap }, { u"word", QTextOption::WordWrap }, { u"manual", QTextOption::ManualWrap },
         { u"anywhere", QTextOption::WrapAnywhere }, { u"at-word-or-anywhere", QTextOption::WrapAtWordBoundaryOrAnywhere }}
    };

    constexpr std::array<std::tuple<QStringView, QTextOption::Flag>, 6> _font_opts_flags_opts = {
        {{ u"trailing-spaces", QTextOption::IncludeTrailingSpaces }, { u"tabs-and-spaces", QTextOption::ShowTabsAndSpaces },
         { u"line-and-paragraph-separators", QTextOption::ShowLineAndParagraphSeparators },
         { u"document-terminator", QTextOption::ShowDocumentTerminator },
         { u"space-for-line-and-paragraph-separators", QTextOption::AddSpaceForLineAndParagraphSeparators },
         { u"suppress-colors", QTextOption::SuppressColors }}
    };

} // namespace

struct theme::ThemeParser::Impl
{
    theme::ThemeParser *parser { nullptr };

    auto parseRender(const QDomElement &element) -> void;
    auto parseThemeConstants(const QDomElement &element) -> void;
    auto parseColors(const QDomElement &element) -> void;
    auto parseBrushes(const QDomElement &element) -> void;
    auto parsePens(const QDomElement &element) -> void;
    auto parseFonts(const QDomElement &element) -> void;
    auto parseAnimations(const QDomElement &element) -> void;
    auto parseFrames(const QDomElement &element) -> void;
    auto parsePushButtonInformation(const QDomElement &element) -> void;
    auto parseLineEditInformation(const QDomElement &element) -> void;
    auto parseComboBoxInformation(const QDomElement &element) -> void;
    auto parseMenuInformation(const QDomElement &element) -> void;
    auto parseProgressBarInformation(const QDomElement &element) -> void;
    auto parseHeaderViewInformation(const QDomElement &element) -> void;
    auto parseTableViewInformation(const QDomElement &element) -> void;
    auto parseCheckBoxInformation(const QDomElement &element) -> void;
    auto parseToolButtonInformation(const QDomElement &element) -> void;
    auto parseGroupBoxInformation(const QDomElement &element) -> void;
    auto parseCDialog(const QDomElement &element) -> void;
    auto parseTitleBar(const QDomElement &element) -> void;

    C_NODISCARD auto parseStates(const QDomElement &element) const -> CENTAUR_THEME_INTERFACE_NAMESPACE::Elements;
    C_NODISCARD auto parseElementState(const QDomElement &element) const -> CENTAUR_THEME_INTERFACE_NAMESPACE::ElementState;
    C_NODISCARD auto parseAnimationData(const QDomElement &element) const -> CENTAUR_THEME_INTERFACE_NAMESPACE::AnimationInformation;

    C_NODISCARD inline auto getColor(const QString &ptr) const -> const QColor &
    {
        using namespace Qt::Literals::StringLiterals;
        auto iter = parser->scheme.colors.find(ptr);
        if (iter == parser->scheme.colors.end())
            throw std::runtime_error(fmt::format("color '{}' is not declared", qPrintable(ptr)));
        return iter->second;
    }

    C_NODISCARD inline auto getBrush(const QString &ptr) const -> const QBrush &
    {
        using namespace Qt::Literals::StringLiterals;
        auto iter = parser->scheme.brushes.find(ptr);
        if (iter == parser->scheme.brushes.end())
            throw std::runtime_error(fmt::format("brush '{}' is not declared", qPrintable(ptr)));
        return iter->second;
    }

    C_NODISCARD inline auto getPen(const QString &ptr) const -> const QPen &
    {
        using namespace Qt::Literals::StringLiterals;
        auto iter = parser->scheme.pens.find(ptr);
        if (iter == parser->scheme.pens.end())
            throw std::runtime_error(fmt::format("pen '{}' is not declared", qPrintable(ptr)));
        return iter->second;
    }

    C_NODISCARD inline auto getFont(const QString &ptr) const -> const CENTAUR_THEME_INTERFACE_NAMESPACE::FontTextLayout &
    {
        using namespace Qt::Literals::StringLiterals;
        auto iter = parser->scheme.fonts.find(ptr);
        if (iter == parser->scheme.fonts.end())
            throw std::runtime_error(fmt::format("font '{}' is not declared", qPrintable(ptr)));
        return iter->second;
    }

    C_NODISCARD inline auto getFrameInformation(const QString &ptr) const
        -> const CENTAUR_THEME_INTERFACE_NAMESPACE::FrameInformation &
    {
        using namespace Qt::Literals::StringLiterals;
        auto iter = parser->uiElements.frames.find(ptr);
        if (iter == parser->uiElements.frames.end())
            throw std::runtime_error(fmt::format("frame '{}' is not declared", qPrintable(ptr)));
        return iter->second;
    }

    C_NODISCARD inline auto getAnimation(const QString &ptr) const -> CENTAUR_THEME_INTERFACE_NAMESPACE::ThemeAnimation *
    {
        using namespace Qt::Literals::StringLiterals;
        auto iter = parser->uiElements.animations.find(ptr);
        if (iter == parser->uiElements.animations.end())
            throw std::runtime_error(fmt::format("animation '{}' is not declared", qPrintable(ptr)));
        return std::addressof(iter->second);
    }

public:
    QStringList errors;
};

theme::ThemeParser::ThemeParser() :
    _impl { new Impl } { _impl->parser = this; }

theme::ThemeParser::~ThemeParser() = default;

QStringList theme::ThemeParser::getErrors() { return P_IMPL()->errors; }

void theme::ThemeParser::loadTheme(const std::string &file)
{
    using namespace Qt::Literals::StringLiterals;

    scheme.colors.clear();
    scheme.brushes.clear();
    scheme.pens.clear();

    uiElements.animations.clear();
    uiElements.frames.clear();
    uiElements.pushButtonOverride.clear();
    uiElements.lineEditOverride.clear();
    uiElements.comboBoxOverride.clear();
    uiElements.progressBarOverride.clear();

    QDomDocument document("theme");

    QFile stream(QString::fromStdString(file));
    if (!stream.open(QIODevice::ReadOnly))
        throw std::runtime_error(fmt::format("The file {} could not be opened: {}", file, qPrintable(stream.errorString())));

    const auto &results = document.setContent(&stream);

    if (!results) {
        stream.close();
        throw std::runtime_error(fmt::format("The XML Document has some errors: {} (col: {}; line: {})",
            qPrintable(results.errorMessage), results.errorColumn, results.errorLine));
    }
    stream.close();

    const auto &root = document.documentElement();

    if (root.tagName() != "theme")
        throw std::runtime_error("document elements is not named 'theme'");

    if (root.attribute("name").isEmpty())
        throw std::runtime_error("theme does not have a name");

    NODE_ITERATOR(child, root)
    {
        if (!child.isElement())
            continue;

        const auto &childElement = child.toElement();

        if (childElement.tagName() == "render") {
            P_IMPL()->parseRender(childElement);
        }
        else if (childElement.tagName() == "constants") {
            P_IMPL()->parseThemeConstants(childElement);
        }
        else if (childElement.tagName() == "colors") {
            P_IMPL()->parseColors(childElement);
        }
        else if (childElement.tagName() == "brushes") {
            P_IMPL()->parseBrushes(childElement);
        }
        else if (childElement.tagName() == "pens") {
            P_IMPL()->parsePens(childElement);
        }
        else if (childElement.tagName() == "fonts") {
            P_IMPL()->parseFonts(childElement);
        }
        else if (childElement.tagName() == "animations") {
            P_IMPL()->parseAnimations(childElement);
        }
        else if (childElement.tagName() == "ui-elements") {
            NODE_ITERATOR(uiElementsNode, child)
            {
                NODE_ELEMENT(uiElementsNode, uiElement)
                const auto nodeName = uiElement.tagName();

                if (nodeName == "frames") {
                    P_IMPL()->parseFrames(uiElement);
                }
                else if (nodeName == "buttons") {
                    P_IMPL()->parsePushButtonInformation(uiElement);
                }
                else if (nodeName == "line-editors") {
                    P_IMPL()->parseLineEditInformation(uiElement);
                }
                else if (nodeName == "menus") {
                    P_IMPL()->parseMenuInformation(uiElement);
                }
                else if (nodeName == "combo-boxes") {
                    P_IMPL()->parseComboBoxInformation(uiElement);
                }
                else if (nodeName == "progress-bar") {
                    P_IMPL()->parseProgressBarInformation(uiElement);
                }
                else if (nodeName == "headers") {
                    P_IMPL()->parseHeaderViewInformation(uiElement);
                }
                else if (nodeName == "tables") {
                    P_IMPL()->parseTableViewInformation(uiElement);
                }
                else if (nodeName == "check-boxes") {
                    P_IMPL()->parseCheckBoxInformation(uiElement);
                }
                else if (nodeName == "tool-buttons") {
                    P_IMPL()->parseToolButtonInformation(uiElement);
                }
                else if (nodeName == "groupbox") {
                    P_IMPL()->parseGroupBoxInformation(uiElement);
                }
                else {
                    P_IMPL()->errors.emplace_back(u"node '%1' in <ui-elements> node is not valid"_s.arg(nodeName));
                }
            }
        }
        else if (childElement.tagName() == "cui-elements") {
            NODE_ITERATOR(cuiElementsNode, child)
            {
                NODE_ELEMENT(cuiElementsNode, cuiElement)
                const auto nodeName = cuiElement.tagName();

                if (nodeName == "c-dialogs") {
                    P_IMPL()->parseCDialog(cuiElement);
                }
                else if (nodeName == "title-bar") {
                    P_IMPL()->parseTitleBar(cuiElement);
                }
            }
        }
        else {
            P_IMPL()->errors.emplace_back(u"node in '%1' node is not valid"_s.arg(childElement.tagName()));
        }
    }
}

auto theme::ThemeParser::Impl::parseRender(const QDomElement &element) -> void
{

    NODE_ITERATOR(node, element)
    {
        NODE_ELEMENT(node, typeNode)

        if (typeNode.nodeName() == "type") {
            const auto *renderHint = std::find_if(_render_hints_opts.begin(), _render_hints_opts.end(),
                [hint = typeNode.text()](const auto &value) -> bool { return static_cast<bool>(std::get<0>(value) == hint); });

            if (renderHint == _render_hints_opts.end()) {
                errors.emplace_back(QString("invalid render hint: %1").arg(typeNode.text()));
            }
            else {
                parser->renderHints.setFlag(std::get<1>(*renderHint));
            }
        }
        else
            errors.emplace_back(QString("render node %1 is not valid").arg(typeNode.nodeName()));
    }
}

auto theme::ThemeParser::Impl::parseThemeConstants(const QDomElement &element) -> void
{
    using namespace Qt::Literals::StringLiterals;

    NODE_ITERATOR(constantsNode, element)
    {
        NODE_ELEMENT(constantsNode, constantElement)

        const QString &tagName = constantElement.tagName();
        if (tagName == "menu-item-image") {
            const auto widthElement  = constantElement.firstChildElement("width");
            const auto heightElement = constantElement.firstChildElement("height");

            if (!widthElement.isNull()) {
                parser->constants.menuItemImage.setWidth(widthElement.text().toInt());
            }

            if (!heightElement.isNull()) {
                parser->constants.menuItemImage.setHeight(heightElement.text().toInt());
            }
        }
        else if (tagName == "tree-item") {
            const QString name = [&]() {
                auto val = constantElement.attribute("name", "*");
                if (val.isEmpty()) {
                    // in case the attribute is set as: name=""
                    return u"*"_s;
                }
                return val;
            }();

            const auto heightElement = constantElement.firstChildElement("height");

            if (!heightElement.isNull()) {
                auto parsedIntStatus = false;
                auto heightValue     = heightElement.text().toInt(&parsedIntStatus);
                if (!parsedIntStatus) {
                    errors.emplace_back(u"%1 can't be parsed as a valid integer for the tree-item->height node"_s.arg(heightElement.text()));
                    heightValue = -1;
                }
                parser->constants.treeItemHeight[name] = heightValue;
            }
        }
        else {
            errors.emplace_back(u"node %1 in constants is not valid"_s.arg(tagName));
        }
    }
}

auto theme::ThemeParser::Impl::parseColors(const QDomElement &element) -> void
{
    using namespace Qt::Literals::StringLiterals;

    NODE_ITERATOR(colorsNode, element)
    {
        NODE_ELEMENT(colorsNode, colorNode)

        const auto &attrs = colorNode.attributes();

        const QString colorName = [&, this]() -> QString {
            for (int idx = 0; idx < attrs.length(); ++idx) {
                NODE_ATTRIBUTE_VAL(attrs.item(idx), attribute)

                if (attribute.name() == "name") {
                    return attribute.value();
                }
                errors.emplace_back(u"color attribute '%1' is not recognize"_s.arg(attribute.name()));
            }
            return {};
        }();

        const QString &value = colorNode.text();
        auto iter            = parser->scheme.colors.find(value);
        if (iter != parser->scheme.colors.end()) {
            errors.emplace_back(u"color name '%1' is already registered"_s.arg(value));
        }
        else {
            const QColor newColor = QColor::fromString(value);
            if (!newColor.isValid()) {
                errors.emplace_back(u"color value '%1' for color name '%2' is already registered"_s.arg(value, colorName));
            }
            else {
                parser->scheme.colors.insert({ colorName, newColor });
            }
        }
    }
}

auto theme::ThemeParser::Impl::parseBrushes(const QDomElement &element) -> void
{
    using namespace Qt::Literals::StringLiterals;

    auto parseRegularBrush = [this](const auto &node, const QString &name, const QString &colorName) -> void {
        Qt::BrushStyle brushStyle = Qt::BrushStyle::SolidPattern;
        NODE_ITERATOR(brushNode, node)
        {
            NODE_ELEMENT(brushNode, childElement)

            if (childElement.tagName() == "pattern") {
                const QString pattern = childElement.text();

                auto patternIter = std::find_if(_brush_pattern_opts.begin(), _brush_pattern_opts.end(),
                    [&pattern](const auto &item) { return static_cast<bool>(pattern == std::get<0>(item)); });

                if (patternIter == _brush_pattern_opts.end()) {
                    errors.emplace_back(u"brush '%1' pattern in '%2' is not valid"_s.arg(pattern, name));
                }
                else {
                    brushStyle = std::get<1>(*patternIter);
                }
            }
            else {
                errors.emplace_back(u"brush child node '%1' for the brush '%2' and, the solid pattern is not valid"_s.arg(
                    childElement.tagName(), name));
            }
        }

        parser->scheme.brushes.insert({ name, QBrush(getColor(colorName), brushStyle) });
    };

    auto parseLinearGradientBrush = [this](const auto &node, const QString &name) -> void {
        QPointF start { 0.0, 0.0 };
        QPointF end { 0.0, 1.0 };

        QLinearGradient linearGradient(start, end);
        linearGradient.setCoordinateMode(QGradient::CoordinateMode::ObjectMode);
        linearGradient.setSpread(QLinearGradient::Spread::PadSpread);

        NODE_ITERATOR(brushChild, node)
        {
            NODE_ELEMENT(brushChild, brushChildElement)
            const QString &nodeName = brushChildElement.tagName();

            if (nodeName == "points") {
                NODE_ITERATOR(pointsNode, brushChild)
                {
                    NODE_ELEMENT(pointsNode, pointsChildElement)
                    const QString &pointsNodeName = pointsChildElement.tagName();

                    if (pointsNodeName == "x-start")
                        start.setX(pointsChildElement.text().toDouble());

                    if (pointsNodeName == "y-start")
                        start.setY(pointsChildElement.text().toDouble());

                    if (pointsNodeName == "x-end")
                        end.setX(pointsChildElement.text().toDouble());

                    if (pointsNodeName == "y-end")
                        end.setY(pointsChildElement.text().toDouble());
                }

                linearGradient.setStart(start);
                linearGradient.setFinalStop(end);
            }
            else if (nodeName == "spread") {
                const auto &spreadValue = brushChildElement.text();

                auto iter = std::find_if(_brush_spread_opts.begin(), _brush_spread_opts.end(),
                    [&spreadValue](const auto &item) { return static_cast<bool>(spreadValue == std::get<0>(item)); });

                if (iter == _brush_spread_opts.end()) {
                    errors.emplace_back(u"spread '%1' is not valid in brush %2"_s.arg(spreadValue, name));
                }
                else {
                    linearGradient.setSpread(std::get<1>(*iter));
                }
            }
            else if (nodeName == "colors") {
                NODE_ITERATOR(nodeColor, brushChild)
                {
                    NODE_ELEMENT(nodeColor, color)
                    if (color.tagName() == "color") {
                        QString colorName;
                        for (int idx = 0; idx < color.attributes().length(); ++idx) {
                            NODE_ATTRIBUTE(color.attributes().item(idx), attr)
                            const QString attrName  = attr.name();
                            const QString attrValue = attr.value();

                            if (attrName == "name") {
                                colorName = attrValue;
                            }
                            else {
                                errors.emplace_back(
                                    u"attribute '%1' in color position for linear-gradient in '%2' is not recognize"_s.arg(
                                        attrName, name));
                            }
                        }

                        if (colorName.isEmpty()) {
                            errors.emplace_back(
                                u"no color name for in color position for linear-gradient in '%1' is not recognize"_s.arg(name));
                        }
                        else {
                            linearGradient.setColorAt(color.text().toDouble(), getColor(colorName));
                        }
                    }
                    else {
                        errors.emplace_back(
                            u"node '%1' is not a valid node in the linear-gradient colors node in '%2'"_s.arg(nodeName, name));
                    }
                }
            }
            else
                errors.emplace_back(u"node '%1' in brush '%2' is not valid"_s.arg(nodeName, name));
        }

        if (auto iter = parser->scheme.brushes.find(name); iter != parser->scheme.brushes.end()) {
            errors.emplace_back(u"brush '%1' is already registered"_s.arg(name));
        }
        else {
            parser->scheme.brushes.insert({ name, linearGradient });
        }
    };

    auto parseConicalGradientBrush = [this](const auto &node, const QString &name) -> void {
        QConicalGradient conicalGradient({}, 0.0);
        conicalGradient.setCoordinateMode(QGradient::CoordinateMode::ObjectMode);

        NODE_ITERATOR(conicalChildNodes, node)
        {
            NODE_ELEMENT(conicalChildNodes, conicalChild)
            const QString &nodeName = conicalChild.tagName();
            if (nodeName == "center") {
                QPointF center { 0.0, 0.0 };
                NODE_ITERATOR(centerChildNode, conicalChild)
                {
                    NODE_ELEMENT(centerChildNode, centerChild)

                    if (centerChild.tagName() == "x") {
                        center.setX(centerChild.text().toDouble());
                    }
                    else if (centerChild.tagName() == "y") {
                        center.setY(centerChild.text().toDouble());
                    }
                    else {
                        errors.emplace_back(
                            u"node '%1' in center node for brush '%2' is not recognized"_s.arg(centerChild.tagName(), name));
                    }
                }
                conicalGradient.setCenter(center);
            }
            else if (nodeName == "angle") {
                conicalGradient.setAngle(conicalChild.text().toDouble());
            }
            else if (nodeName == "colors") {
                NODE_ITERATOR(nodeColor, conicalChild)
                {
                    NODE_ELEMENT(nodeColor, color)
                    if (color.tagName() == "color") {
                        QString colorName;
                        for (int idx = 0; idx < color.attributes().length(); ++idx) {
                            NODE_ATTRIBUTE(color.attributes().item(idx), attr)
                            const QString attrName  = attr.name();
                            const QString attrValue = attr.value();

                            if (attrName == "name") {
                                colorName = attrValue;
                            }
                            else {
                                errors.emplace_back(
                                    u"attribute '%1' in color position for conical-gradient in '%2' is not recognize"_s.arg(
                                        attrName, name));
                            }
                        }

                        if (colorName.isEmpty()) {
                            errors.emplace_back(
                                u"no color name for in color position for conical-gradient in '%1' is not recognize"_s.arg(name));
                        }
                        else {
                            conicalGradient.setColorAt(color.text().toDouble(), getColor(colorName));
                        }
                    }
                    else {
                        errors.emplace_back(
                            u"node '%1' is not a valid node in the conical-gradient colors node in '%2'"_s.arg(nodeName, name));
                    }
                }
            }
            else
                errors.emplace_back(u"node '%1' in brush '%2' is not valid"_s.arg(nodeName, name));
        }

        if (auto iter = parser->scheme.brushes.find(name); iter != parser->scheme.brushes.end()) {
            errors.emplace_back(u"brush '%1' is already registered"_s.arg(name));
        }
        else {
            parser->scheme.brushes.insert({ name, conicalGradient });
        }
    };

    auto parseRadialGradientBrush = [this](const auto &node, const QString &name) -> void {
        QRadialGradient radialGradient({}, 0.0, {}, 0.0);
        radialGradient.setCoordinateMode(QGradient::CoordinateMode::ObjectMode);

        NODE_ITERATOR(radialChildNode, node)
        {
            NODE_ELEMENT(radialChildNode, radialChild)
            const QString &nodeName = radialChild.tagName();

            if (nodeName == "center") {
                QPointF center { 0.0, 0.0 };
                NODE_ITERATOR(centerChildNode, radialChild)
                {
                    NODE_ELEMENT(centerChildNode, centerChild)

                    if (centerChild.tagName() == "x") {
                        center.setX(centerChild.text().toDouble());
                    }
                    else if (centerChild.tagName() == "y") {
                        center.setY(centerChild.text().toDouble());
                    }
                    else {
                        errors.emplace_back(
                            u"node '%1' in center node for brush '%2' is not recognized"_s.arg(centerChild.tagName(), name));
                    }
                }
                radialGradient.setCenter(center);
            }
            else if (nodeName == "focal") {
                QPointF focal { 0.0, 0.0 };
                NODE_ITERATOR(focalChildNode, radialChild)
                {
                    NODE_ELEMENT(focalChildNode, focalChild)

                    if (focalChild.tagName() == "x") {
                        focal.setX(focalChild.text().toDouble());
                    }
                    else if (focalChild.tagName() == "y") {
                        focal.setY(focalChild.text().toDouble());
                    }
                    else {
                        errors.emplace_back(
                            u"node '%1' in focal node for brush '%2' is not recognized"_s.arg(focalChild.tagName(), name));
                    }
                }
                radialGradient.setFocalPoint(focal);
            }
            else if (nodeName == "radius") {
                radialGradient.setRadius(radialChild.text().toDouble());
            }
            else if (nodeName == "focal-radius") {
                radialGradient.setFocalRadius(radialChild.text().toDouble());
            }
            else if (nodeName == "colors") {
                NODE_ITERATOR(nodeColor, radialChild)
                {
                    NODE_ELEMENT(nodeColor, color)
                    if (color.tagName() == "color") {
                        QString colorName;
                        for (int idx = 0; idx < color.attributes().length(); ++idx) {
                            NODE_ATTRIBUTE(color.attributes().item(idx), attr)
                            const QString attrName  = attr.name();
                            const QString attrValue = attr.value();

                            if (attrName == "name") {
                                colorName = attrValue;
                            }
                            else {
                                errors.emplace_back(
                                    u"attribute '%1' in color position for radial-gradient in '%2' is not recognize"_s.arg(
                                        attrName, name));
                            }
                        }

                        if (colorName.isEmpty()) {
                            errors.emplace_back(
                                u"no color name for in color position for radial-gradient in '%1' is not recognize"_s.arg(name));
                        }
                        else {
                            radialGradient.setColorAt(color.text().toDouble(), getColor(colorName));
                        }
                    }
                    else {
                        errors.emplace_back(
                            u"node '%1' is not a valid node in the radial-gradient colors node in '%2'"_s.arg(nodeName, name));
                    }
                }
            }
            else
                errors.emplace_back(u"node '%1' in brush '%2' is not valid"_s.arg(nodeName, name));
        }

        if (auto iter = parser->scheme.brushes.find(name); iter != parser->scheme.brushes.end()) {
            errors.emplace_back(u"brush '%1' is already registered"_s.arg(name));
        }
        else {
            parser->scheme.brushes.insert({ name, radialGradient });
        }
    };

    NODE_ITERATOR(brushNode, element)
    {
        NODE_ELEMENT(brushNode, brushElement)

        if (brushElement.tagName() != "brush") {
            errors.emplace_back(u"element '%1' in the brushes node is not valid"_s.arg(brushElement.tagName()));
            continue;
        }

        QString brushName;
        QString brushFormat;
        QString brushColor;
        for (int idx = 0; idx < brushElement.attributes().length(); ++idx) {
            NODE_ATTRIBUTE(brushElement.attributes().item(idx), attr)
            const QString name  = attr.name();
            const QString value = attr.value();

            if (name == "name") {
                brushName = value;
            }
            else if (name == "format") {
                brushFormat = value;

                if (!brushColor.isEmpty() && brushFormat != "regular") {
                    errors.emplace_back(
                        u"attribute 'color' in brush will be ignored because the attribute is not valid in a non-regular brush"_s);
                }
            }
            else if (name == "color") {
                brushColor = value;
                if (!brushFormat.isEmpty() && brushFormat != "regular") {
                    errors.emplace_back(
                        u"attribute 'color' in brush will be ignored because the attribute is not valid in a non-regular brush"_s);
                }
            }
            else
                errors.emplace_back(u"attribute '%1' in brush is not valid"_s.arg(name));
        }

        if (brushName.isEmpty()) {
            errors.emplace_back(u"brush without name is not valid"_s);
            continue;
        }

        const auto *format = std::find_if(_brush_formats_opts.begin(), _brush_formats_opts.end(),
            [&brushFormat](const auto &frmt) -> bool { return static_cast<bool>(brushFormat == std::get<0>(frmt)); });

        if (format == _brush_formats_opts.end()) {
            errors.emplace_back(u"brush format '%1' in brush '%2' is not recognize"_s.arg(brushFormat, brushName));
        }
        else {

            switch (std::get<1>(*format)) {
                case BrushFormats::Regular: parseRegularBrush(brushElement, brushName, brushColor); break;
                case BrushFormats::LinearGradient: parseLinearGradientBrush(brushElement, brushName); break;
                case BrushFormats::ConicalGradient: parseConicalGradientBrush(brushElement, brushName); break;
                case BrushFormats::RadialGradient: parseRadialGradientBrush(brushElement, brushName); break;
            }
        }
    }
}

auto theme::ThemeParser::Impl::parsePens(const QDomElement &element) -> void
{
    using namespace Qt::Literals::StringLiterals;
    NODE_ITERATOR(penNode, element)
    {
        QPen pen { Qt::NoBrush, 1.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin };
        NODE_ELEMENT(penNode, penElement)

        if (penElement.tagName() != "pen") {
            errors.emplace_back(u"element '%1' in the pens node is not valid"_s.arg(penElement.tagName()));
            continue;
        }

        QString penName;
        QString penWidth;
        QString penBrush;
        for (int idx = 0; idx < penElement.attributes().length(); ++idx) {
            NODE_ATTRIBUTE(penElement.attributes().item(idx), attr)
            const QString name  = attr.name();
            const QString value = attr.value();

            if (name == "name") {
                penName = value;
            }
            else if (name == "width") {
                penWidth = value;
            }
            else if (name == "brush") {
                penBrush = value;
            }
            else
                errors.emplace_back(u"attribute '%1' in pen is not valid"_s.arg(name));
        }

        if (penName.isEmpty()) {
            errors.emplace_back(u"pen without name is not valid"_s);
            continue;
        }

        if (!penWidth.isEmpty()) {
            pen.setWidthF(penWidth.toDouble());
        }

        if (!penBrush.isEmpty()) {
            pen.setBrush(getBrush(penBrush));
        }

        NODE_ITERATOR(penChildNode, penNode)
        {
            NODE_ELEMENT(penChildNode, penChildElement)

            if (penChildElement.tagName() == "style") {
                const auto &styleName = penChildElement.text();
                const auto *styleIter = std::find_if(_pen_style_opts.begin(), _pen_style_opts.end(),
                    [&styleName](const auto &item) { return static_cast<bool>(styleName == std::get<0>(item)); });

                if (styleIter == _pen_style_opts.end()) {
                    errors.emplace_back(u"pen '%1' has an invalid pen style"_s.arg(penName));
                    continue;
                }
                pen.setStyle(std::get<1>(*styleIter));

                if (pen.style() == Qt::PenStyle::CustomDashLine) {
                    QList<qreal> patternList;
                    NODE_ITERATOR(patternsChildNode, penChildElement)
                    {
                        NODE_ELEMENT(patternsChildNode, patternChildElement)

                        if (patternChildElement.tagName() == "pattern") {
                            patternList.emplace_back(patternChildElement.text().toDouble());
                        }
                        else {
                            errors.emplace_back(u"node '%1' in patterns child node for pen '%2' is not recognized"_s.arg(
                                patternChildElement.tagName(), penName));
                        }
                    }

                    if (patternList.isEmpty()) {
                        errors.emplace_back(u"pen '%1' custom pattern style without a pattern"_s.arg(penName));
                    }
                    else {
                        if (patternList.size() % 2) {
                            errors.emplace_back(
                                u"custom pattern for pen '%1' must be an even number, a pattern with value 1.0 will be added"_s.arg(
                                    penName));
                            patternList.emplace_back(1.0);
                        }

                        pen.setDashPattern(patternList);
                    }
                }
            }
            else if (penChildElement.tagName() == "cap") {
                const auto &capName = penChildElement.text();
                const auto *capIter = std::find_if(_pen_cap_style_opts.begin(), _pen_cap_style_opts.end(),
                    [&capName](const auto &item) { return static_cast<bool>(capName == std::get<0>(item)); });

                if (capIter == _pen_cap_style_opts.end())
                    errors.emplace_back(u"pen '%1' has an invalid pen cap: %2"_s.arg(penName, capName));

                pen.setCapStyle(std::get<1>(*capIter));
            }
            else if (penChildElement.tagName() == "join") {
                const auto &joinName = penChildElement.text();
                const auto *joinIter = std::find_if(_pen_join_style_opts.begin(), _pen_join_style_opts.end(),
                    [&joinName](const auto &item) { return static_cast<bool>(joinName == std::get<0>(item)); });

                if (joinIter == _pen_join_style_opts.end()) {
                    errors.emplace_back(u"pen '%1' has an invalid pen join"_s.arg(penName));
                    continue;
                }

                pen.setJoinStyle(std::get<1>(*joinIter));
            }
            else if (penChildElement.tagName() == "miter-limit") {
                pen.setMiterLimit(penChildElement.text().toDouble());
            }
            else
                errors.emplace_back(u"node '%1' in pen '%2' is not valid"_s.arg(penChildElement.tagName(), penName));
        }

        if (parser->scheme.pens.contains(penName)) {
            errors.emplace_back(u"pen '%1' is already registered"_s.arg(penName));
        }
        else {
            parser->scheme.pens.insert({ penName, pen });
        }
    }
}

auto theme::ThemeParser::Impl::parseFonts(const QDomElement &element) -> void
{
    using namespace Qt::Literals::StringLiterals;
    NODE_ITERATOR(fontNode, element)
    {
        const auto iErrors = errors.size();
        CENTAUR_THEME_INTERFACE_NAMESPACE::FontTextLayout ftl;

        NODE_ELEMENT(fontNode, fontElement)

        QString fontLayoutName;
        for (int idx = 0; idx < fontElement.attributes().length(); ++idx) {
            NODE_ATTRIBUTE(fontElement.attributes().item(idx), attr)
            const QString name  = attr.name();
            const QString value = attr.value();

            if (name == "name") {
                fontLayoutName = value;
            }
            else
                errors.emplace_back(u"attribute '%1' in font is not valid"_s.arg(name));
        }

        NODE_ITERATOR(fontChildNode, fontNode)
        {
            NODE_ELEMENT(fontChildNode, fontChildElement)
            const QString nodeName = fontChildElement.tagName();

            if (nodeName == "options") {
                NODE_ITERATOR(fontOptionsNode, fontChildElement)
                {
                    NODE_ELEMENT(fontOptionsNode, fontOptionElement)
                    const QString optionName  = fontOptionElement.tagName();
                    const QString optionValue = fontOptionElement.text();

                    if (optionName == "vertical-alignment") {
                        const auto *vertIter = std::find_if(_font_vert_alignment_opts.begin(), _font_vert_alignment_opts.end(),
                            [&optionValue](
                                const auto &value) -> bool { return static_cast<bool>(std::get<0>(value) == optionValue); });

                        if (vertIter == _font_vert_alignment_opts.end())
                            errors.emplace_back(u"invalid vertical alignment value: %1"_s.arg(optionName));

                        ftl.opts.setAlignment(ftl.opts.alignment() | std::get<1>(*vertIter));
                    }
                    else if (optionName == "horizontal-alignment") {
                        const auto *horzIter = std::find_if(_font_horz_alignment_opts.begin(), _font_horz_alignment_opts.end(),
                            [&optionValue](
                                const auto &value) -> bool { return static_cast<bool>(std::get<0>(value) == optionValue); });

                        if (horzIter == _font_horz_alignment_opts.end())
                            errors.emplace_back(u"invalid horizontal alignment value: %1"_s.arg(optionName));

                        ftl.opts.setAlignment(ftl.opts.alignment() | std::get<1>(*horzIter));
                    }
                    else if (optionName == "wrap-mode") {
                        const auto *wrapIter = std::find_if(
                            _font_word_wrap_opts.begin(), _font_word_wrap_opts.end(), [&optionValue](const auto &value) -> bool {
                                return static_cast<bool>(std::get<0>(value) == optionValue);
                            });

                        if (wrapIter == _font_word_wrap_opts.end())
                            errors.emplace_back(u"invalid wrap-mode value %1"_s.arg(optionName));

                        ftl.opts.setWrapMode(std::get<1>(*wrapIter));
                    }
                    else if (optionName == "flags") {
                        QTextOption::Flags flags;

                        NODE_ITERATOR(optionFlagsNode, fontChildElement)
                        {
                            NODE_ELEMENT(optionFlagsNode, optionFlagsElement)

                            if (optionFlagsElement.tagName() != "flag") {
                                errors.emplace_back(
                                    u"'%1' is not recognize as a valid flag node for flags"_s.arg(optionFlagsElement.tagName()));
                            }

                            const auto flagValue = optionFlagsElement.text();

                            const auto *flagIter = std::find_if(
                                _font_opts_flags_opts.begin(), _font_opts_flags_opts.end(), [&flagValue](const auto &value) -> bool {
                                    return static_cast<bool>(std::get<0>(value) == flagValue);
                                });

                            if (flagIter == _font_opts_flags_opts.end())
                                errors.emplace_back(u"invalid flag value %1"_s.arg(nodeName));

                            flags.setFlag(std::get<1>(*flagIter));
                        }

                        ftl.opts.setFlags(flags);
                    }
                    else
                        errors.emplace_back(u"font options '%1' node %2 is not valid"_s.arg(fontLayoutName, optionName));
                }
            }
            else if (nodeName == "style") {
                for (int idx = 0; idx < fontChildElement.attributes().length(); ++idx) {
                    NODE_ATTRIBUTE(fontChildElement.attributes().item(idx), attr)
                    const auto attributeName = attr.name();

                    if (attributeName == "face") {
                        ftl.style.fontName = attr.value();
                    }
                    else if (attributeName == "size") {
                        const QString value = attr.value();
                        if (!value.isEmpty()) {
                            if (value.back() == '%') {
                                const auto ptSize    = QApplication::font().pointSizeF();
                                const auto newPtSize = value.first(value.size() - 1).toDouble();

                                ftl.style.size = ptSize * (newPtSize / 100.0);
                            }
                            else
                                ftl.style.size = attr.value().toDouble();
                        }
                    }
                    else if (attributeName == "italic") {
                        const int value  = attr.value().toInt();
                        ftl.style.italic = value != 0;
                    }
                    else if (attributeName == "kerning") {
                        const int value   = attr.value().toInt();
                        ftl.style.kerning = value != 0;
                    }
                    else if (attributeName == "underline") {
                        const int value     = attr.value().toInt();
                        ftl.style.underline = value != 0;
                    }
                    else
                        errors.emplace_back(u"font '%1 style attribute '%2' is not valid"_s.arg(fontLayoutName, attributeName));
                }

                NODE_ITERATOR(fontStyleNode, fontChildElement)
                {
                    NODE_ELEMENT(fontStyleNode, fontStyleElement)
                    const QString styleName = fontStyleElement.tagName();
                    const QString nodeValue = fontStyleElement.text();

                    if (styleName == "weight") {
                        bool intOk       = true;
                        const auto value = nodeValue.toInt(&intOk);
                        if (intOk) {
                            // Fix values between 1 and 1000, just as the Qt specification notes

                            ftl.style.weight = std::max(1, std::min(maxWeightValue, value));
                        }
                        else {
                            const auto *weightIter = std::find_if(_font_weights_opts.begin(), _font_weights_opts.end(),
                                [&nodeValue](const auto &val) -> bool { return static_cast<bool>(std::get<0>(val) == nodeValue); });

                            if (weightIter == _font_weights_opts.end()) {
                                errors.emplace_back(u"invalid weight value %1 in font %2"_s.arg(nodeValue, fontLayoutName));
                            }
                            else {
                                ftl.style.weight = static_cast<int>(std::get<1>(*weightIter));
                            }
                        }
                    }
                    else if (styleName == "capitalization") {
                        const auto *capsIter = std::find_if(_font_caps_opts.begin(), _font_caps_opts.end(),
                            [&nodeValue](const auto &value) -> bool { return static_cast<bool>(std::get<0>(value) == nodeValue); });

                        if (capsIter == _font_caps_opts.end()) {
                            errors.emplace_back(u"invalid caps value %1 in font %2"_s.arg(nodeValue, fontLayoutName));
                        }
                        else {
                            ftl.style.caps = std::get<1>(*capsIter);
                        }
                    }
                    else if (styleName == "stretch-factor") {
                        bool intOk       = true;
                        const auto value = nodeValue.toInt(&intOk);
                        if (intOk) {
                            // Fix values between 1 and 4000, just as the Qt specification notes

                            ftl.style.stretchFactor = std::max(1, std::min(maxStretchFactor, value));
                        }
                        else {
                            const auto *stretchIter = std::find_if(_font_stretch_opts.begin(), _font_stretch_opts.end(),
                                [&nodeValue](const auto &val) -> bool { return static_cast<bool>(std::get<0>(val) == nodeValue); });

                            if (stretchIter == _font_stretch_opts.end()) {
                                errors.emplace_back(u"invalid stretch value %1 in font %2"_s.arg(nodeValue, fontLayoutName));
                            }
                            else {
                                ftl.style.stretchFactor = static_cast<int>(std::get<1>(*stretchIter));
                            }
                        }
                    }
                    else if (styleName == "letter-spacing") {
                        const auto &factAttr = fontStyleElement.attribute("factor", "");
                        if (factAttr.isEmpty()) {
                            ftl.style.spacingType = QFont::SpacingType::PercentageSpacing;
                        }
                        else {
                            if (factAttr == "absolute") {
                                ftl.style.spacingType = QFont::SpacingType::AbsoluteSpacing;
                            }
                            else if (factAttr == "percentage") {
                                ftl.style.spacingType = QFont::SpacingType::PercentageSpacing;
                            }
                            else {
                                errors.emplace_back(
                                    u"font '%1' 'letter-spacing' node has an factor: '%2'"_s.arg(fontLayoutName, factAttr));
                            }
                        }

                        ftl.style.letterSpacing = nodeValue.toDouble();
                    }
                    else if (styleName == "word-spacing") {
                        ftl.style.wordSpacing = nodeValue.toDouble();
                    }
                    else
                        errors.emplace_back(u"font style '%1', node '%2' is not valid"_s.arg(fontLayoutName, styleName));
                }
            }
            else
                errors.emplace_back(u"Node %1 in font is not valid"_s.arg(nodeName));
        }

        if (iErrors == errors.size()) {
            const auto &iter = parser->scheme.fonts.find(fontLayoutName);
            if (iter != parser->scheme.fonts.end()) {
                errors.emplace_back(u"font name '%1' is already declared"_s.arg(fontLayoutName));
            }
            else {
                parser->scheme.fonts.insert({ fontLayoutName, ftl });
            }
        }
    }
}

auto theme::ThemeParser::Impl::parseAnimations(const QDomElement &element) -> void
{
    using namespace Qt::Literals::StringLiterals;
    NODE_ITERATOR(animationsNode, element)
    {
        const auto iErrors = errors.size();
        CENTAUR_THEME_INTERFACE_NAMESPACE::ThemeAnimation themeAnimation;

        NODE_ELEMENT(animationsNode, animationElement)

        QString animationName;
        for (int idx = 0; idx < animationElement.attributes().length(); ++idx) {
            NODE_ATTRIBUTE(animationElement.attributes().item(idx), attr)
            const QString name  = attr.name();
            const QString value = attr.value();

            if (name == "name") {
                animationName = value;
            }
            else
                errors.emplace_back(u"attribute '%1' in animation is not valid"_s.arg(name));
        }

        NODE_ITERATOR(animationsChildNode, animationElement)
        {
            NODE_ELEMENT(animationsChildNode, animChildElement)
            const QString nodeName  = animChildElement.tagName();
            const QString nodeValue = animChildElement.text();

            if (nodeName == "easing-curve") {
                const auto *iter = std::find_if(_easing_curve_opts.begin(), _easing_curve_opts.end(),
                    [&nodeValue](const auto &item) { return static_cast<bool>(nodeValue == std::get<0>(item)); });
                if (iter == _easing_curve_opts.end()) {
                    errors.emplace_back(u"easing curve name '%1' is not valid in animation '%2'"_s.arg(nodeValue, animationName));
                }
                else {
                    themeAnimation.easingCurve = std::get<1>(*iter);
                }
            }
            else if (nodeName == "duration") {
                themeAnimation.duration = nodeValue.toInt();
            }
            else
                errors.emplace_back(u"node '%1' in animation '%2' is not valid"_s.arg(nodeName, animationName));
        }

        if (iErrors == errors.size()) {
            const auto &iter = parser->uiElements.animations.find(animationName);
            if (iter != parser->uiElements.animations.end()) {
                errors.emplace_back(u"animation name '%1' is already declared"_s.arg(animationName));
            }
            else {
                parser->uiElements.animations.insert({ animationName, themeAnimation });
            }
        }
    }
}

auto theme::ThemeParser::Impl::parseFrames(const QDomElement &element) -> void
{
    using namespace Qt::Literals::StringLiterals;

    NODE_ITERATOR(framesNode, element)
    {
        const auto iErrors = errors.size();
        CENTAUR_THEME_INTERFACE_NAMESPACE::FrameInformation fifo;

        NODE_ELEMENT(framesNode, framesElement)

        QString frameName;
        for (int idx = 0; idx < framesElement.attributes().length(); ++idx) {
            NODE_ATTRIBUTE(framesElement.attributes().item(idx), attr)
            const QString name  = attr.name();
            const QString value = attr.value();

            if (name == "name") {
                frameName = value;
            }
            else
                errors.emplace_back(u"attribute '%1' in frame is not valid"_s.arg(name));
        }

        NODE_ITERATOR(frameChildNode, framesElement)
        {
            NODE_ELEMENT(frameChildNode, frameChildElement)
            const auto nodeName  = frameChildElement.tagName();
            const auto nodeValue = frameChildElement.text();

            if (nodeName == "border-radius-x") {
                fifo.borderRadiusX = nodeValue.toDouble();
            }
            else if (nodeName == "border-radius-y") {
                fifo.borderRadiusY = nodeValue.toDouble();
            }
            else if (nodeName == "border-radius-top-left") {
                fifo.borderRadiusTopLeft = nodeValue.toDouble();
            }
            else if (nodeName == "border-radius-top-right") {
                fifo.borderRadiusTopRight = nodeValue.toDouble();
            }
            else if (nodeName == "border-radius-bottom-left") {
                fifo.borderRadiusBottomLeft = nodeValue.toDouble();
            }
            else if (nodeName == "border-radius-bottom-right") {
                fifo.borderRadiusBottomRight = nodeValue.toDouble();
            }
            else if (nodeName == "border-left-pen") {
                if (!nodeValue.isEmpty())
                    fifo.leftBorderPen = getPen(nodeValue);
            }
            else if (nodeName == "border-right-pen") {
                if (!nodeValue.isEmpty())
                    fifo.rightBorderPen = getPen(nodeValue);
            }
            else if (nodeName == "border-bottom-pen") {
                if (!nodeValue.isEmpty())
                    fifo.bottomBorderPen = getPen(nodeValue);
            }
            else if (nodeName == "border-top-pen") {
                if (!nodeValue.isEmpty())
                    fifo.topBorderPen = getPen(nodeValue);
            }
            else if (nodeName == "margins") {
                NODE_ITERATOR(marginsChildNode, frameChildNode)
                {
                    NODE_ELEMENT(marginsChildNode, marginChildElement)
                    const auto marginNodeName  = marginChildElement.tagName();
                    const auto marginNodeValue = marginChildElement.text();

                    if (marginNodeName == "bottom") {
                        fifo.margins.setBottom(marginNodeValue.toInt());
                    }
                    else if (marginNodeName == "top") {
                        fifo.margins.setTop(marginNodeValue.toInt());
                    }
                    else if (marginNodeName == "right") {
                        fifo.margins.setRight(marginNodeValue.toInt());
                    }
                    else if (marginNodeName == "left") {
                        fifo.margins.setLeft(marginNodeValue.toInt());
                    }
                    else {
                        errors.emplace_back(u"frame name '%1' has an invalid margin name: '%2'"_s.arg(frameName, marginNodeName));
                    }
                }
            }
            else if (nodeName == "padding") {
                NODE_ITERATOR(paddingsChildNode, frameChildNode)
                {
                    NODE_ELEMENT(paddingsChildNode, paddingChildElement)
                    const auto paddingNodeName  = paddingChildElement.tagName();
                    const auto paddingNodeValue = paddingChildElement.text();

                    if (paddingNodeName == "bottom") {
                        fifo.padding.setBottom(paddingNodeValue.toInt());
                    }
                    else if (paddingNodeName == "top") {
                        fifo.padding.setTop(paddingNodeValue.toInt());
                    }
                    else if (paddingNodeName == "right") {
                        fifo.padding.setRight(paddingNodeValue.toInt());
                    }
                    else if (paddingNodeName == "left") {
                        fifo.padding.setLeft(paddingNodeValue.toInt());
                    }
                    else {
                        errors.emplace_back(u"frame name '%1' has an invalid margin name: '%2'"_s.arg(frameName, paddingNodeName));
                    }
                }
            }
            else {
                errors.emplace_back(u"node '%1' in frame '%2' was not recognized"_s.arg(nodeName, frameName));
            }
        }
        if (iErrors == errors.size()) {
            if (parser->uiElements.frames.contains(frameName)) {
                errors.emplace_back(u"frame name '%1' is already registered"_s.arg(frameName));
            }
            else {
                parser->uiElements.frames.insert({ frameName, fifo });
            }
        }
    }
}

auto theme::ThemeParser::Impl::parseElementState(const QDomElement &element) const -> CENTAUR_THEME_INTERFACE_NAMESPACE::ElementState
{
    using namespace Qt::Literals::StringLiterals;

    CENTAUR_THEME_INTERFACE_NAMESPACE::ElementState elementState;

    NODE_ITERATOR(stateNode, element)
    {
        if (!stateNode.isElement())
            continue;
        const auto &stateElement = stateNode.toElement();

        const auto nodeName  = stateElement.tagName();
        const auto nodeValue = stateElement.text();

        if (nodeName == "frame") {
            elementState.fi = getFrameInformation(nodeValue);
        }
        else if (nodeName == "frame-brush") {
            elementState.brush = getBrush(nodeValue);
        }
        else if (nodeName == "frame-pen") {
            elementState.pen = getPen(nodeValue);
        }
        else if (nodeName == "font") {
            elementState.fontInformation = getFont(nodeValue);
        }
        else if (nodeName == "font-pen") {
            elementState.fontPen = getPen(nodeValue);
        }
        else
            throw std::runtime_error(qPrintable(u"element state '%1' is not recognize"_s.arg(nodeName)));
    }

    return elementState;
}

auto theme::ThemeParser::Impl::parseAnimationData(const QDomElement &element) const
    -> CENTAUR_THEME_INTERFACE_NAMESPACE::AnimationInformation
{
    using namespace Qt::Literals::StringLiterals;

    CENTAUR_THEME_INTERFACE_NAMESPACE::AnimationInformation animationInformation;

    NODE_ITERATOR(animationNode, element)
    {
        CENTAUR_THEME_INTERFACE_NAMESPACE::Animation anim;

        if (!animationNode.isElement())
            continue;
        const auto &animationElement = animationNode.toElement();

        // animationElement: <animation> element
        const auto hint = animationElement.attribute("hint", "color_");
        for (int idx = 0; idx < animationElement.attributes().length(); ++idx) {
            if (!animationElement.attributes().item(idx).isAttr())
                continue;

            const auto &attr = animationElement.attributes().item(idx).toAttr();

            const QString name  = attr.name();
            const QString value = attr.value();

            if (name == "to") {
                if (value.toLower() == "border") {
                    anim.element = CENTAUR_THEME_INTERFACE_NAMESPACE::AnimationElement::Border;
                }
                else if (value.toLower() == "background") {
                    anim.element = CENTAUR_THEME_INTERFACE_NAMESPACE::AnimationElement::Background;
                }
                else if (value.toLower() == "unchecked-background") {
                    anim.element = CENTAUR_THEME_INTERFACE_NAMESPACE::AnimationElement::uCheckBoxBackground;
                }
                else if (value.toLower() == "unchecked-border") {
                    anim.element = CENTAUR_THEME_INTERFACE_NAMESPACE::AnimationElement::uCheckBoxBorder;
                }
                else if (value.toLower() == "checked-background") {
                    anim.element = CENTAUR_THEME_INTERFACE_NAMESPACE::AnimationElement::cCheckBoxBackground;
                }
                else if (value.toLower() == "checked-border") {
                    anim.element = CENTAUR_THEME_INTERFACE_NAMESPACE::AnimationElement::cCheckBoxBorder;
                }
                else
                    throw std::runtime_error(qPrintable(u"'to' attribute with value '%1' in animation is not valid"_s.arg(value)));
            }
            else if (name == "hint") {
                // hint is parsed before, in order to parse the 'start' and 'end' attributes
                // correctly if hint is declared after this attributes example:
                // <... start="0.0" end="1.0" hint="decimal"> </...>
                // In this case 'start' and 'end' will be parsed as colors
            }
            else if (name == "start") {
                if (hint == "color") {
                    anim.start = getColor(value);
                }
                else if (hint == "integer") {
                    anim.start = value.toInt();
                }
                else if (hint == "decimal") {
                    anim.start = value.toDouble();
                }
                else
                    throw std::runtime_error(qPrintable(u"'start' attribute with value '%1' in animation is not valid"_s.arg(value)));
            }
            else if (name == "end") {
                if (hint == "color") {
                    anim.end = getColor(value);
                }
                else if (hint == "integer") {
                    anim.end = value.toInt();
                }
                else if (hint == "decimal") {
                    anim.end = value.toDouble();
                }
                else
                    throw std::runtime_error(qPrintable(u"'start' attribute with value '%1' in animation is not valid"_s.arg(value)));
            }
            else
                throw std::runtime_error(qPrintable(u"attribute '%1' in animation is not valid"_s.arg(name)));
        }

        NODE_ITERATOR(animationStateNode, animationElement)
        {
            if (!animationStateNode.isElement())
                continue;
            const auto &animationStateElement = animationStateNode.toElement();

            const auto &nodeName  = animationStateElement.tagName();
            const auto &nodeValue = animationStateElement.text();

            CENTAUR_THEME_INTERFACE_NAMESPACE::AnimationApplication apply {};
            if (nodeName == "hover-in") {
                if (!anim.animationSteps.empty()
                    && !anim.animationSteps.contains(CENTAUR_THEME_INTERFACE_NAMESPACE::AnimationApplication::HoverOut))
                    throw std::runtime_error("'hover-in' can be only paired with 'hover-out'");

                apply = CENTAUR_THEME_INTERFACE_NAMESPACE::AnimationApplication::HoverIn;
            }
            else if (nodeName == "hover-out") {
                if (!anim.animationSteps.empty()
                    && !anim.animationSteps.contains(CENTAUR_THEME_INTERFACE_NAMESPACE::AnimationApplication::HoverIn))
                    throw std::runtime_error("'hover-out' can be only paired with 'hover-in'");

                apply = CENTAUR_THEME_INTERFACE_NAMESPACE::AnimationApplication::HoverOut;
            }
            else if (nodeName == "focus-in") {
                if (!anim.animationSteps.empty()
                    && !anim.animationSteps.contains(CENTAUR_THEME_INTERFACE_NAMESPACE::AnimationApplication::FocusOut))
                    throw std::runtime_error("'focus-in' can be only paired with 'focus-out'");

                apply = CENTAUR_THEME_INTERFACE_NAMESPACE::AnimationApplication::FocusIn;
            }
            else if (nodeName == "focus-out") {
                if (!anim.animationSteps.empty()
                    && !anim.animationSteps.contains(CENTAUR_THEME_INTERFACE_NAMESPACE::AnimationApplication::FocusIn))
                    throw std::runtime_error("'focus-out' can be only paired with 'focus-in'");

                apply = CENTAUR_THEME_INTERFACE_NAMESPACE::AnimationApplication::FocusOut;
            }
            else if (nodeName == "checkbox-hover-in") {
                if (!anim.animationSteps.empty()
                    && !anim.animationSteps.contains(CENTAUR_THEME_INTERFACE_NAMESPACE::AnimationApplication::CheckBoxFocusOut))
                    throw std::runtime_error("'checkbox-unchecked-hover-out' can be only paired with 'checkbox-unchecked-hover-in'");

                apply = CENTAUR_THEME_INTERFACE_NAMESPACE::AnimationApplication::CheckBoxFocusIn;
            }
            else if (nodeName == "checkbox-hover-out") {
                if (!anim.animationSteps.empty()
                    && !anim.animationSteps.contains(CENTAUR_THEME_INTERFACE_NAMESPACE::AnimationApplication::CheckBoxFocusIn))
                    throw std::runtime_error("'checkbox-unchecked-hover-out' can be only paired with 'checkbox-unchecked-hover-in'");

                apply = CENTAUR_THEME_INTERFACE_NAMESPACE::AnimationApplication::CheckBoxFocusOut;
            }
            else
                throw std::runtime_error(qPrintable(u"node '%1' in animation is not valid"_s.arg(nodeName)));

            anim.animationSteps[apply] = getAnimation(nodeValue);
        }

        animationInformation.emplace_back(anim);
    }

    return animationInformation;
}

auto theme::ThemeParser::Impl::parseStates(const QDomElement &element) const -> CENTAUR_THEME_INTERFACE_NAMESPACE::Elements
{
    using namespace Qt::Literals::StringLiterals;

    CENTAUR_THEME_INTERFACE_NAMESPACE::Elements elements;

    NODE_ITERATOR(stateNode, element)
    {
        if (!stateNode.isElement())
            return {};
        const auto &stateElement = stateNode.toElement();

        const auto nodeName = stateElement.tagName();

        if (nodeName == "state-normal") {
            elements.normal = parseElementState(stateElement);
        }
        else if (nodeName == "state-focus") {
            elements.focus = parseElementState(stateElement);
        }
        else if (nodeName == "state-hover") {
            elements.hover = parseElementState(stateElement);
        }
        else if (nodeName == "state-pressed") {
            elements.pressed = parseElementState(stateElement);
        }
        else
            throw std::runtime_error(qPrintable(u"state name '%1' is not valid"_s.arg(nodeName)));
    }

    return elements;
}

auto theme::ThemeParser::Impl::parsePushButtonInformation(const QDomElement &element) -> void
{
    using namespace Qt::Literals::StringLiterals;

    NODE_ITERATOR(buttonNode, element)
    {
        CENTAUR_THEME_INTERFACE_NAMESPACE::PushButtonInformation pbi;

        NODE_ELEMENT(buttonNode, buttonElement)

        const QString buttonName     = buttonElement.attribute("name");
        const QString buttonCopyFrom = buttonElement.attribute("copy");

        if (!buttonCopyFrom.isEmpty()) {
            if (buttonName.isEmpty()) {
                errors.emplace_back(u"can not copy button information because the button node doesn't have a name"_s);
                continue; // We will not continue parsing because that action will probably override
                          // the default button action information
            }
            auto buttonInformationIter = parser->uiElements.pushButtonOverride.find(buttonCopyFrom);
            if (buttonInformationIter == parser->uiElements.pushButtonOverride.end()) {
                errors.emplace_back(u"can not copy button information because '%1' is not declared"_s.arg(buttonCopyFrom));
            }
            else {
                pbi = buttonInformationIter->second;
            }
        }

        NODE_ITERATOR(btnNode, buttonElement)
        {
            NODE_ELEMENT(btnNode, btnElement)
            const auto nodeName = btnElement.tagName();

            if (nodeName == "default") {
                pbi.defaultButton = parseStates(btnElement);
            }
            else if (nodeName == "enabled") {
                pbi.enabled = parseStates(btnElement);
            }
            else if (nodeName == "disabled") {
                pbi.disabled = parseStates(btnElement);
            }
            else if (nodeName == "animations") {
                pbi.animations = parseAnimationData(btnElement);
            }
            else
                errors.emplace_back(u"node '%1' in button is not valid"_s.arg(nodeName));
        }

        if (!buttonName.isEmpty()) {
            parser->uiElements.pushButtonOverride[buttonName] = pbi;
        }
        else
            parser->uiElements.pushButtonInformation = pbi;
    }
}

auto theme::ThemeParser::Impl::parseLineEditInformation(const QDomElement &element) -> void
{
    using namespace Qt::Literals::StringLiterals;

    NODE_ITERATOR(editorNode, element)
    {
        CENTAUR_THEME_INTERFACE_NAMESPACE::LineEditInformation lei;

        NODE_ELEMENT(editorNode, editorElement)

        if (editorElement.tagName() != "editor") {
            errors.emplace_back(u"node '%1' in editors is not recognizable"_s.arg(editorElement.tagName()));
            continue;
        }

        const auto editorName       = editorElement.attribute("name");
        const auto textColor        = editorElement.attribute("text-color");
        const auto disableColor     = editorElement.attribute("disable-color");
        const auto placeHolderColor = editorElement.attribute("placeholder-color");

        lei.textColor        = getColor(textColor);
        lei.disableTextColor = getColor(disableColor);
        lei.placeHolderColor = getColor(placeHolderColor);

        NODE_ITERATOR(widgetNode, editorElement)
        {
            NODE_ELEMENT(widgetNode, widgetElement)

            const auto nodeName = widgetElement.tagName();
            if (nodeName == "enabled") {
                lei.enabled = parseStates(widgetElement);
            }
            else if (nodeName == "disabled") {
                lei.disabled = parseStates(widgetElement);
            }
            else if (nodeName == "animations") {
                lei.animations = parseAnimationData(widgetElement);
            }
            else
                errors.emplace_back(u"node '%1' in line-edit is not valid"_s.arg(nodeName));
        }

        if (!editorName.isEmpty()) {
            parser->uiElements.lineEditOverride[editorName] = lei;
        }
        else {
            parser->uiElements.lineEditInformation = lei;
        }
    }
}

auto theme::ThemeParser::Impl::parseComboBoxInformation(const QDomElement &element) -> void
{
    using namespace Qt::Literals::StringLiterals;

    NODE_ITERATOR(comboNode, element)
    {
        CENTAUR_THEME_INTERFACE_NAMESPACE::ComboBoxInformation cbi;

        NODE_ELEMENT(comboNode, comboElement)

        if (comboElement.tagName() != "combo-box") {
            errors.emplace_back(u"node '%1' in combo-boxes is not recognizable"_s.arg(comboElement.tagName()));
            continue;
        }

        const auto comboName = comboElement.attribute("name");
        cbi.dropArrowSize    = comboElement.attribute("drop-width", "0").toInt();
        cbi.dropArrowPen     = getPen(comboElement.attribute("drop-pen"));

        NODE_ITERATOR(widgetNode, comboElement)
        {
            NODE_ELEMENT(widgetNode, widgetElement)

            const auto nodeName = widgetElement.tagName();
            if (nodeName == "enabled") {
                cbi.enabled = parseStates(widgetElement);
            }
            else if (nodeName == "disabled") {
                cbi.disabled = parseStates(widgetElement);
            }
            else if (nodeName == "animations") {
                cbi.animations = parseAnimationData(widgetElement);
            }
            else
                errors.emplace_back(u"node '%1' in combo-box is not valid"_s.arg(nodeName));
        }

        if (!comboName.isEmpty()) {
            parser->uiElements.comboBoxOverride[comboName] = cbi;
        }
        else
            parser->uiElements.comboBoxInformation = cbi;
    }
}

auto theme::ThemeParser::Impl::parseMenuInformation(const QDomElement &element) -> void
{
    using namespace Qt::Literals::StringLiterals;

    for (int idx = 0; idx < element.attributes().length(); ++idx) {
        NODE_ATTRIBUTE(element.attributes().item(idx), attr)
        const auto attrName  = attr.name();
        const auto attrValue = attr.value();

        if (attrName == "item-height") {
            parser->uiElements.menuInformation.itemHeight = attrValue.toInt();
        }

        else if (attrName == "separator-height") {
            parser->uiElements.menuInformation.separatorHeight = attrValue.toInt();
        }

        else if (attrName == "left-padding") {
            parser->uiElements.menuInformation.leftPadding = attrValue.toInt();
        }
        else
            errors.emplace_back(u"attribute '%1' for the menus node is not valid"_s.arg(attrName));
    }

    NODE_ITERATOR(menusNode, element)
    {
        NODE_ELEMENT(menusNode, menuElement)

        const auto nodeName  = menuElement.tagName();
        const auto nodeValue = menuElement.text();

        if (nodeName == "panel-brush") {
            parser->uiElements.menuInformation.panelBrush = getBrush(nodeValue);
        }
        else if (nodeName == "empty-area-brush") {
            parser->uiElements.menuInformation.emptyAreaBrush = getBrush(nodeValue);
        }
        else if (nodeName == "separator-pen") {
            if (!nodeValue.isEmpty())
                parser->uiElements.menuInformation.separatorPen = getPen(nodeValue);
        }
        else if (nodeName == "separator-brush") {
            if (!nodeValue.isEmpty())
                parser->uiElements.menuInformation.separatorBrush = getBrush(nodeValue);
        }
        else if (nodeName == "selected-brush") {
            if (!nodeValue.isEmpty())
                parser->uiElements.menuInformation.selectedBrush = getBrush(nodeValue);
        }
        else if (nodeName == "selected-pen") {
            if (!nodeValue.isEmpty())
                parser->uiElements.menuInformation.selectedPen = getPen(nodeValue);
        }
        else if (nodeName == "enabled-pen") {
            if (!nodeValue.isEmpty())
                parser->uiElements.menuInformation.enabledPen = getPen(nodeValue);
        }
        else if (nodeName == "disabled-pen") {
            if (!nodeValue.isEmpty())
                parser->uiElements.menuInformation.disabledPen = getPen(nodeValue);
        }
        else if (nodeName == "selected-font") {
            if (!nodeValue.isEmpty())
                parser->uiElements.menuInformation.selectedFont = getFont(nodeValue);
        }
        else if (nodeName == "disabled-font") {
            if (!nodeValue.isEmpty())
                parser->uiElements.menuInformation.disabledFont = getFont(nodeValue);
        }
        else if (nodeName == "enabled-font") {
            if (!nodeValue.isEmpty())
                parser->uiElements.menuInformation.enabledFont = getFont(nodeValue);
        }
        else
            errors.emplace_back(u"node '%1' for menus is not valid"_s.arg(nodeName));
    }
}

auto theme::ThemeParser::Impl::parseProgressBarInformation(const QDomElement &element) -> void
{
    using namespace Qt::Literals::StringLiterals;

    NODE_ITERATOR(barNode, element)
    {
        NODE_ELEMENT(barNode, barElement)

        if (barElement.tagName() != "bar") {
            errors.emplace_back(u"node '%1' in progress-bar is not recognizable"_s.arg(barElement.tagName()));
            continue;
        }

        CENTAUR_THEME_INTERFACE_NAMESPACE::ProgressBarInformation pbi;
        const auto barName = barElement.attribute("name");

        pbi.applyGlowEffect = static_cast<bool>(barElement.attribute("glow-effect", "0").toInt());

        NODE_ITERATOR(widgetNode, barElement)
        {
            NODE_ELEMENT(widgetNode, widgetElment)

            const auto nodeName  = widgetElment.tagName();
            const auto nodeValue = widgetElment.text();

            if (nodeName == "effect") {
                if (!pbi.applyGlowEffect) {
                    errors.emplace_back(u"node effect was not specified in the parent attributes"_s);
                }
                else {
                    NODE_ITERATOR(effectNode, widgetElment)
                    {
                        NODE_ELEMENT(effectNode, effectElement)
                        const auto effectNodeName  = effectElement.tagName();
                        const auto effectNodeValue = effectElement.text();

                        if (effectNodeName == "x-offset") {
                            pbi.effectInformation.xOffset = effectNodeValue.toDouble();
                        }
                        else if (effectNodeName == "y-offset") {
                            pbi.effectInformation.yOffset = effectNodeValue.toDouble();
                        }
                        else if (effectNodeName == "blur-radius") {
                            pbi.effectInformation.blurRadius = effectNodeValue.toDouble();
                        }
                        else if (effectNodeName == "effect-color") {
                            pbi.effectInformation.glowColor = getColor(effectNodeValue);
                        }
                        else {
                            errors.emplace_back(u"node '%1' in progress bar effect node is not valid"_s.arg(effectNodeName));
                        }
                    }
                }
            }
            else if (nodeName == "frame") {
                pbi.fi = getFrameInformation(nodeValue);
            }
            else if (nodeName == "disabled-groove-brush") {
                pbi.disableGrooveBrush = getBrush(nodeValue);
            }
            else if (nodeName == "disable-bar-brush") {
                pbi.disableBarBrush = getBrush(nodeValue);
            }
            else if (nodeName == "groove-brush") {
                pbi.enabledGrooveBrush = getBrush(nodeValue);
            }
            else if (nodeName == "bar-brush") {
                pbi.enabledBarBrush = getBrush(nodeValue);
            }
            else
                errors.emplace_back(u"node '%1' in progress bar node is not valid"_s.arg(nodeName));
        }

        if (!barName.isEmpty()) {
            parser->uiElements.progressBarOverride[barName] = pbi;
        }
        else {
            parser->uiElements.progressBarInformation = pbi;
        }
    }
}

auto theme::ThemeParser::Impl::parseHeaderViewInformation(const QDomElement &element) -> void
{
    using namespace Qt::Literals::StringLiterals;

    NODE_ITERATOR(headerNode, element)
    {
        NODE_ELEMENT(headerNode, headerElement)

        if (headerElement.tagName() != "header") {
            errors.emplace_back(u"node '%1' in headers is not recognizable"_s.arg(headerElement.tagName()));
            continue;
        }

        Qt::Orientation orientation {};
        CENTAUR_THEME_INTERFACE_NAMESPACE::HeaderInformation headerInformation {};
        const auto headerName        = headerElement.attribute("name");
        const auto headerOrientation = headerElement.attribute("orientation");

        if (headerOrientation.isEmpty()) {
            errors.emplace_back(u"header orientation must be set"_s);
            continue;
        }

        if (headerOrientation == "vertical") {
            orientation = Qt::Orientation::Vertical;
        }
        else if (headerOrientation == "horizontal") {
            orientation = Qt::Orientation::Horizontal;
        }
        else {
            errors.emplace_back(u"header orientation '%1' must be either 'vertical' or 'horizontal'"_s.arg(headerOrientation));
            continue;
        }

        NODE_ITERATOR(widgetNode, headerElement)
        {
            NODE_ELEMENT(widgetNode, widgetElement)

            const auto nodeName  = widgetElement.tagName();
            const auto nodeValue = widgetElement.text();

            if (nodeName == "empty-area-brush") {
                headerInformation.emptyAreaBrush = getBrush(nodeValue);
            }
            else if (nodeName == "empty-area-disable-brush") {
                headerInformation.disableEmptyAreaBrush = getBrush(nodeValue);
            }
            else if (nodeName == "background-brush") {
                headerInformation.backgroundBrush = getBrush(nodeValue);
            }
            else if (nodeName == "hover-brush") {
                headerInformation.hoverBrush = getBrush(nodeValue);
            }
            else if (nodeName == "sunken-brush") {
                headerInformation.sunkenBrush = getBrush(nodeValue);
            }
            else if (nodeName == "disable-background-brush") {
                headerInformation.disableBackgroundBrush = getBrush(nodeValue);
            }
            else if (nodeName == "section-lines") {
                const auto visibleAttributeValue = widgetElement.attribute("visible", "false").toLower();

                headerInformation.showSectionLines
                    = visibleAttributeValue == "true"
                          ? true
                          : (visibleAttributeValue == "1"
                                  ? true
                                  : (visibleAttributeValue == "false" ? false
                                      : visibleAttributeValue == "0"
                                          ? false
                                          : (static_cast<void>(errors.emplace_back(
                                                 u"header section lines visibility value of '%1' is unrecognizable and will be defaulted to 'false'"_s
                                                     .arg(visibleAttributeValue))),
                                              false)));

                if (headerInformation.showSectionLines) {
                    headerInformation.sectionLinesPen        = getPen(widgetElement.attribute("pen"));
                    headerInformation.disableSectionLinesPen = getPen(widgetElement.attribute("disable-pen"));

                    const auto marginsList = widgetElement.elementsByTagName("margins");

                    if (marginsList.size()) {
                        const auto marginChildList = marginsList.at(0).childNodes();
                        for (int idx = 0; idx < marginChildList.length(); ++idx) {
                            const auto marginsNode = marginChildList.at(idx);
                            NODE_ELEMENT(marginsNode, marginsElement)
                            const auto marginName  = marginsElement.tagName();
                            const auto marginValue = marginsElement.text();
                            if (marginName == "left") {
                                headerInformation.sectionLinesMargins.setLeft(marginValue.toInt());
                            }
                            else if (marginName == "right") {
                                headerInformation.sectionLinesMargins.setRight(marginValue.toInt());
                            }
                            else if (marginName == "bottom") {
                                headerInformation.sectionLinesMargins.setBottom(marginValue.toInt());
                            }
                            else if (marginName == "top") {
                                headerInformation.sectionLinesMargins.setTop(marginValue.toInt());
                            }
                            else {
                                errors.emplace_back(u"margin name '%1' is unrecognizable"_s.arg(marginName));
                            }
                        }
                    }
                }
            }
            else if (nodeName == "disable-font-pen") {
                headerInformation.disableFontPen = getPen(nodeValue);
            }
            else if (nodeName == "disable-font") {
                headerInformation.disableFont = getFont(nodeValue);
            }
            else if (nodeName == "font-pen") {
                headerInformation.fontPen = getPen(nodeValue);
            }
            else if (nodeName == "font") {
                headerInformation.font = getFont(nodeValue);
            }
            else if (nodeName == "hover-pen") {
                headerInformation.hoverPen = getPen(nodeValue);
            }
            else if (nodeName == "hover-font") {
                headerInformation.hoverFont = getFont(nodeValue);
            }
            else if (nodeName == "sunken-pen") {
                headerInformation.sunkenPen = getPen(nodeValue);
            }
            else if (nodeName == "sunken-font") {
                headerInformation.sunkenFont = getFont(nodeValue);
            }
            else
                errors.emplace_back(u"node '%1' in header is not valid"_s.arg(nodeName));
        }
        switch (orientation) {
            case Qt::Horizontal:
                if (headerName.isEmpty()) {
                    parser->uiElements.horizontalHeaderInformation = headerInformation;
                }
                else
                    parser->uiElements.horizontalHeaderOverride[headerName] = headerInformation;
                break;
            case Qt::Vertical:
                if (headerName.isEmpty()) {
                    parser->uiElements.verticalHeaderInformation = headerInformation;
                }
                else
                    parser->uiElements.verticalHeaderOverride[headerName] = headerInformation;
                break;
        }
    }
}

auto theme::ThemeParser::Impl::parseTableViewInformation(const QDomElement &element) -> void
{
    using namespace Qt::Literals::StringLiterals;

    bool mainTableInformationSet = false;

    NODE_ITERATOR(tableNode, element)
    {
        NODE_ELEMENT(tableNode, tableElement)

        if (tableElement.tagName() != "table") {
            errors.emplace_back(u"node '%1' in tables is not recognizable"_s.arg(tableElement.tagName()));
            continue;
        }

        CENTAUR_THEME_INTERFACE_NAMESPACE::TableViewInformation tvi;
        const auto tableName     = tableElement.attribute("name");
        const auto copyFrom      = tableElement.attribute("copy");
        const auto mouseOverItem = tableElement.attribute("mouse-over-item", "false");
        const auto gridLines     = tableElement.attribute("grid-lines", "false");
        const auto paneBrush     = tableElement.attribute("pane-brush");

        if (!copyFrom.isEmpty()) {
            bool copyError    = false;
            const auto copied = [&]() -> CENTAUR_THEME_INTERFACE_NAMESPACE::TableViewInformation {
                if (copyFrom == "*") {
                    if (!mainTableInformationSet) {
                        errors.emplace_back(u"Can not copy the global table information because is not set"_s);
                        copyError = true;
                        return {};
                    }
                    return parser->uiElements.tableViewInformation;
                }

                auto iter = parser->uiElements.tableViewOverride.find(copyFrom);
                if (iter == parser->uiElements.tableViewOverride.end()) {
                    copyError = true;
                    errors.emplace_back(u"Can not copy table information because '%1' was not found"_s.arg(copyFrom));
                    return {};
                }
                return iter->second;
            }();

            if (copyError) {
                // Avoid continue because it might overwrite the main data
                continue;
            }

            if (tableName.isEmpty()) {
                errors.emplace_back(u"Can not copy table information because the destination must be set"_s);
                continue;
            }

            tvi = copied;
        }

        tvi.mouseOverItem
            = mouseOverItem == "true" || mouseOverItem == "1"
              || ((
                  !(mouseOverItem == "false") && !(mouseOverItem == "0")
                  && (static_cast<void>(errors.emplace_back(
                          u"'mouse-over-item' value of '%1' is unrecognizable and will be defaulted to 'false'"_s.arg(mouseOverItem))),
                      false)));

        tvi.gridLines
            = gridLines == "true"
                  ? true
                  : (gridLines == "1"
                          ? true
                          : (gridLines == "false" ? false
                              : gridLines == "0"
                                  ? false
                                  : (static_cast<void>(errors.emplace_back(
                                         u"'grid-lines' value of '%1' is unrecognizable and will be defaulted to 'false'"_s.arg(
                                             gridLines))),
                                      false)));

        if (!paneBrush.isEmpty())
            tvi.paneBackgroundBrush = getBrush(paneBrush);

        NODE_ITERATOR(widgetNode, tableElement)
        {
            NODE_ELEMENT(widgetNode, widgetElement)

            const auto nodeName  = widgetElement.tagName();
            const auto nodeValue = widgetElement.text();

            if (nodeName == "grid-line-pen") {
                tvi.gridLinesPen = getPen(nodeValue);
            }
            else if (nodeName == "widget-background-brush") {
                tvi.backgroundBrush = getBrush(nodeValue);
            }
            else if (nodeName == "item-background-brush") {
                tvi.itemBackgroundBrush = getBrush(nodeValue);
            }
            else if (nodeName == "item-alternate-background-brush") {
                tvi.itemAltBackgroundBrush = getBrush(nodeValue);
            }
            else if (nodeName == "widget-disabled-background-brush") {
                tvi.disableBackgroundBrush = getBrush(nodeValue);
            }
            else if (nodeName == "item-hover-brush") {
                tvi.itemHoverBrush = getBrush(nodeValue);
            }
            else if (nodeName == "item-selected-brush") {
                tvi.itemSelectedBrush = getBrush(nodeValue);
            }
            else if (nodeName == "item-focus-brush") {
                tvi.itemFocusBrush = getBrush(nodeValue);
            }
            else if (nodeName == "disabled-font") {
                tvi.disabledFont = getFont(nodeValue);
            }
            else if (nodeName == "disabled-font-pen") {
                tvi.disabledPen = getPen(nodeValue);
            }
            else if (nodeName == "font") {
                tvi.fontFont = getFont(nodeValue);
            }
            else if (nodeName == "font-pen") {
                tvi.fontPen = getPen(nodeValue);
            }
            else if (nodeName == "hover-font") {
                tvi.hoverFont = getFont(nodeValue);
            }
            else if (nodeName == "hover-pen") {
                tvi.hoverPen = getPen(nodeValue);
            }
            else if (nodeName == "focus-font") {
                tvi.focusFont = getFont(nodeValue);
            }
            else if (nodeName == "focus-pen") {
                tvi.focusPen = getPen(nodeValue);
            }
            else if (nodeName == "selected-font") {
                tvi.selectedFont = getFont(nodeValue);
            }
            else if (nodeName == "selected-pen") {
                tvi.selectedPen = getPen(nodeValue);
            }
            else if (nodeName == "item-margins") {
                const auto marginsList = widgetElement.elementsByTagName("margins");

                if (marginsList.size()) {
                    const auto marginChildList = marginsList.at(0).childNodes();
                    for (int idx = 0; idx < marginChildList.length(); ++idx) {
                        const auto marginsNode = marginChildList.at(idx);
                        NODE_ELEMENT(marginsNode, marginsElement)
                        const auto marginName  = marginsElement.tagName();
                        const auto marginValue = marginsElement.text();
                        if (marginName == "left") {
                            tvi.itemMargins.setLeft(marginValue.toInt());
                        }
                        else if (marginName == "right") {
                            tvi.itemMargins.setRight(marginValue.toInt());
                        }
                        else if (marginName == "bottom") {
                            tvi.itemMargins.setBottom(marginValue.toInt());
                        }
                        else if (marginName == "top") {
                            tvi.itemMargins.setTop(marginValue.toInt());
                        }
                        else {
                            errors.emplace_back(u"margin name '%1' is unrecognizable"_s.arg(marginName));
                        }
                    }
                }
            }
            else
                errors.emplace_back(u"node '%1' in table is not valid"_s.arg(nodeName));
        }
        if (tableName.isEmpty()) {
            mainTableInformationSet                 = true;
            parser->uiElements.tableViewInformation = tvi;
        }
        else {
            parser->uiElements.tableViewOverride[tableName] = tvi;
        }
    }
}

auto theme::ThemeParser::Impl::parseCheckBoxInformation(const QDomElement &element) -> void
{
    using namespace Qt::Literals::StringLiterals;

    auto parseElementState
        = [&, this](const QDomElement &state) -> CENTAUR_THEME_INTERFACE_NAMESPACE::CheckBoxInformation::CheckElementState {
        CENTAUR_THEME_INTERFACE_NAMESPACE::CheckBoxInformation::CheckElementState ces;

        NODE_ITERATOR(elementNode, state)
        {
            NODE_ELEMENT(elementNode, elementState)

            const auto tagName  = elementState.tagName();
            const auto tagValue = elementState.text();

            if (tagName == "widget-frame") {
                ces.widgetFrame = getFrameInformation(tagValue);
            }
            else if (tagName == "widget-frame-brush") {
                ces.widgetBrush = getBrush(tagValue);
            }
            else if (tagName == "widget-frame-pen") {
                ces.widgetPen = getPen(tagValue);
            }
            else if (tagName == "checked-box-frame") {
                ces.checkedBoxFrame = getFrameInformation(tagValue);
            }
            else if (tagName == "checked-box-frame-brush") {
                ces.checkedBoxBrush = getBrush(tagValue);
            }
            else if (tagName == "checked-box-frame-pen") {
                ces.checkedBoxPen = getPen(tagValue);
            }
            else if (tagName == "checked-box-indicator-pen") {
                ces.checkedBoxIndicatorPen = getPen(tagValue);
            }
            else if (tagName == "unchecked-box-frame") {
                ces.uncheckedBoxFrame = getFrameInformation(tagValue);
            }
            else if (tagName == "unchecked-box-frame-brush") {
                ces.uncheckedBoxBrush = getBrush(tagValue);
            }
            else if (tagName == "unchecked-box-frame-pen") {
                ces.uncheckedBoxPen = getPen(tagValue);
            }
            else if (tagName == "unchecked-box-indicator-pen") {
                ces.uncheckedBoxIndicatorPen = getPen(tagValue);
            }
            else if (tagName == "undefined-box-frame") {
                ces.undefinedBoxFrame = getFrameInformation(tagValue);
            }
            else if (tagName == "undefined-box-frame-brush") {
                ces.undefinedBoxBrush = getBrush(tagValue);
            }
            else if (tagName == "undefined-box-frame-pen") {
                ces.undefinedBoxPen = getPen(tagValue);
            }
            else if (tagName == "undefined-box-indicator-pen") {
                ces.undefinedBoxIndicatorPen = getPen(tagValue);
            }
            else if (tagName == "unchecked-font") {
                ces.uncheckedFont = getFont(tagValue);
            }
            else if (tagName == "unchecked-font-pen") {
                ces.uncheckedFontPen = getPen(tagValue);
            }
            else if (tagName == "checked-font") {
                ces.checkedFont = getFont(tagValue);
            }
            else if (tagName == "checked-font-pen") {
                ces.checkedFontPen = getPen(tagValue);
            }
            else if (tagName == "undefined-font") {
                ces.undefinedFont = getFont(tagValue);
            }
            else if (tagName == "undefined-font-pen") {
                ces.undefinedFontPen = getPen(tagValue);
            }
        }

        return ces;
    };

    auto parseElement = [&](const QDomElement &elem) -> CENTAUR_THEME_INTERFACE_NAMESPACE::CheckBoxInformation::CheckElements {
        CENTAUR_THEME_INTERFACE_NAMESPACE::CheckBoxInformation::CheckElements checkElements;

        NODE_ITERATOR(elementNode, elem)
        {
            NODE_ELEMENT(elementNode, elementElem)
            const QString tagName = elementElem.tagName();

            if (tagName == "state-normal") {
                checkElements.normal = parseElementState(elementElem);
            }
            else if (tagName == "state-hover") {
                checkElements.hover = parseElementState(elementElem);
            }
            else if (tagName == "state-focus") {
                checkElements.focus = parseElementState(elementElem);
            }
            else
                throw std::runtime_error(qPrintable(u"check-box node '%1' is not recognizable"_s.arg(tagName)));
        }

        return checkElements;
    };

    NODE_ITERATOR(boxNode, element)
    {
        CENTAUR_THEME_INTERFACE_NAMESPACE::CheckBoxInformation cbi;

        NODE_ELEMENT(boxNode, boxElement)

        if (boxElement.tagName() != "box") {
            errors.emplace_back(u"node '%1' in check-boxes is not recognizable"_s.arg(boxElement.tagName()));
            continue;
        }

        const auto boxName = boxElement.attribute("name");

        NODE_ITERATOR(widgetNode, boxElement)
        {
            NODE_ELEMENT(widgetNode, widgetElement)

            const auto nodeName = widgetElement.tagName();
            if (nodeName == "enabled") {
                cbi.enabled = parseElement(widgetElement);
            }
            else if (nodeName == "disabled") {
                cbi.disabled = parseElement(widgetElement);
            }
            else if (nodeName == "animations") {
                cbi.animations = parseAnimationData(widgetElement);
            }
            else
                errors.emplace_back(u"node '%1' in box is not valid"_s.arg(nodeName));
        }

        if (!boxName.isEmpty()) {
            parser->uiElements.checkBoxOverride[boxName] = cbi;
        }
        else {
            parser->uiElements.checkBoxInformation = cbi;
        }
    }
}

auto theme::ThemeParser::Impl::parseToolButtonInformation(const QDomElement &element) -> void
{
    using namespace Qt::Literals::StringLiterals;

    NODE_ITERATOR(buttonNode, element)
    {
        CENTAUR_THEME_INTERFACE_NAMESPACE::ToolButtonInformation tbi;

        NODE_ELEMENT(buttonNode, buttonElement)

        const QString buttonName     = buttonElement.attribute("name");
        const QString buttonCopyFrom = buttonElement.attribute("copy");

        if (!buttonCopyFrom.isEmpty()) {
            if (buttonName.isEmpty()) {
                errors.emplace_back(u"can not copy tool-button information because the tool-button node doesn't have a name"_s);
                continue; // We will not continue parsing because that action will probably override
                          // the default button action information
            }
            auto buttonInformationIter = parser->uiElements.toolButtonOverride.find(buttonCopyFrom);
            if (buttonInformationIter == parser->uiElements.toolButtonOverride.end()) {
                errors.emplace_back(u"can not copy tool-button information because '%1' is not declared"_s.arg(buttonCopyFrom));
            }
            else {
                tbi = buttonInformationIter->second;
            }
        }

        NODE_ITERATOR(btnNode, buttonElement)
        {
            NODE_ELEMENT(btnNode, btnElement)
            const auto nodeName = btnElement.tagName();

            if (nodeName == "enabled") {
                tbi.enabled = parseStates(btnElement);
            }
            else if (nodeName == "disabled") {
                tbi.disabled = parseStates(btnElement);
            }
            else if (nodeName == "animations") {
                tbi.animations = parseAnimationData(btnElement);
            }
            else
                errors.emplace_back(u"node '%1' in tool-button is not valid"_s.arg(nodeName));
        }

        if (!buttonName.isEmpty()) {
            parser->uiElements.toolButtonOverride[buttonName] = tbi;
        }
        else
            parser->uiElements.toolButtonInformation = tbi;
    }
}

auto theme::ThemeParser::Impl::parseGroupBoxInformation(const QDomElement &element) -> void
{
    using namespace Qt::Literals::StringLiterals;
    NODE_ITERATOR(groupNode, element)
    {
        CENTAUR_THEME_INTERFACE_NAMESPACE::GroupBoxInformation gbi;

        NODE_ELEMENT(groupNode, groupElement)

        const QString name = groupElement.attribute("name");

        NODE_ITERATOR(boxNode, groupElement)
        {
            NODE_ELEMENT(boxNode, boxElement)

            const QString nodeName  = boxElement.tagName();
            const QString nodeValue = boxElement.text();

            if (nodeName == "frame") {
                gbi.fi = getFrameInformation(nodeValue);
            }
            else if (nodeName == "contents-brush") {
                gbi.contentsBrush = getBrush(nodeValue);
            }
            else if (nodeName == "header-brush") {
                gbi.headerBrush = getBrush(nodeValue);
            }
            else if (nodeName == "header-indicator-width") {
                bool intOk         = true;
                gbi.indicatorWidth = nodeValue.toInt(&intOk);
                if (!intOk) {
                    errors.emplace_back(u"indicator with in the group box could not be parser (%1) as a valid integer"_s.arg(nodeValue));
                    gbi.indicatorWidth = 0; // Default to zero in case of error
                }
            }
            else if (nodeName == "header-height") {
                bool intOk       = true;
                gbi.headerHeight = nodeValue.toInt(&intOk);
                if (!intOk) {
                    errors.emplace_back(u"indicator height with in the group box could not be parser as a valid integer (%1)"_s.arg(nodeValue));
                    gbi.headerHeight = -1; // Default to -1 in case of error, this will force the theme to use the default
                }
            }
            else if (nodeName == "header-indicator-brush") {
                gbi.indicatorBrush = getBrush(nodeValue);
            }
            else if (nodeName == "header-indicator-brush-disabled") {
                gbi.disabledIndicatorBrush = getBrush(nodeValue);
            }
            else if (nodeName == "font") {
                gbi.font = getFont(nodeValue);
            }
            else if (nodeName == "font-pen") {
                gbi.pen = getPen(nodeValue);
            }
            else if (nodeName == "font-disabled") {
                gbi.disabledFont = getFont(nodeValue);
            }
            else if (nodeName == "font-pen-disable") {
                gbi.disabledPen = getPen(nodeValue);
            }
            else {
                errors.emplace_back(u"node '%1' in groupbox is not recognized"_s.arg(nodeName));
            }
        }

        if (!name.isEmpty()) {
            parser->uiElements.groupBoxOverride[name] = gbi;
        }
        else {
            parser->uiElements.groupBoxInformation = gbi;
        }
    }
}

auto theme::ThemeParser::Impl::parseCDialog(const QDomElement &element) -> void
{
    using namespace Qt::Literals::StringLiterals;
    NODE_ITERATOR(dialogNode, element)
    {
        CENTAUR_THEME_INTERFACE_NAMESPACE::DialogInformation difo;

        NODE_ELEMENT(dialogNode, dialogElement)

        if (dialogElement.tagName() != "c-dialog") {
            errors.emplace_back(u"node '%1' in c-dialog is not recognizable"_s.arg(dialogElement.tagName()));
            continue;
        }

        const auto name = dialogElement.attribute("name");

        NODE_ITERATOR(uiNode, dialogElement)
        {
            NODE_ELEMENT(uiNode, uiElement)

            const QString nodeName  = uiElement.tagName();
            const QString nodeValue = uiElement.text();

            if (nodeName == "frame") {
                difo.frameInformation = getFrameInformation(nodeValue);
            }
            else if (nodeName == "background-brush") {
                difo.backgroundBrush = getBrush(nodeValue);
            }
            else if (nodeName == "border-pen" && !nodeValue.isEmpty()) {
                difo.borderPen = getPen(nodeValue);
            }
            else {
                errors.emplace_back(u"node '%1' is not recognized in the c-dialog information"_s.arg(nodeName));
            }
        }

        if (name.isEmpty()) {
            parser->uiElements.dialogInformation = difo;
        }
        else
            parser->uiElements.dialogOverride[name] = difo;
    }
}

auto theme::ThemeParser::Impl::parseTitleBar(const QDomElement &element) -> void
{
    using namespace Qt::Literals::StringLiterals;
    NODE_ITERATOR(titleNode, element)
    {
        CENTAUR_THEME_INTERFACE_NAMESPACE::TitleBarInformation tbfo;

        NODE_ELEMENT(titleNode, titleElement)

        if (titleElement.tagName() != "bar") {
            errors.emplace_back(u"node '%1' in title-bar is not recognizable"_s.arg(titleElement.tagName()));
            continue;
        }

        const auto name = titleElement.attribute("name");

        NODE_ITERATOR(uiNode, titleElement)
        {
            NODE_ELEMENT(uiNode, uiElement)

            const QString nodeName  = uiElement.tagName();
            const QString nodeValue = uiElement.text();

            if (nodeName == "frame") {
                tbfo.frameInformation = getFrameInformation(nodeValue);
            }
            else if (nodeName == "background-brush") {
                tbfo.backgroundBrush = getBrush(nodeValue);
            }
            else if (nodeName == "font") {
                tbfo.font = getFont(nodeValue);
            }
            else if (nodeName == "font-pen") {
                tbfo.fontPen = getPen(nodeValue);
            }
            else {
                errors.emplace_back(u"node '%1' is not recognized in the title-bar->bar information"_s.arg(nodeName));
            }
        }

        if (name.isEmpty()) {
            parser->uiElements.titleBarInformation = tbfo;
        }
        else
            parser->uiElements.titleBarOverride[name] = tbfo;
    }
}
