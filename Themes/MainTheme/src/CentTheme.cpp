/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 12/06/23.
// Copyright (c) 2023 Ricardo Romero.  All rights reserved.
//

#include "CentTheme.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QEventTransition>
#include <QFocusFrame>
#include <QGraphicsEffect>
#include <QLineEdit>
#include <QMargins>
#include <QPainterPath>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScrollBar>
#include <QStyle>
#include <QStyleOption>
#include <QTableWidget>
#include <QTreeWidget>
#include <QWindow>
#include <algorithm>
#include <qdrawutil.h>
#include <qnamespace.h>

#include "ThemeInterface.hpp"

namespace
{
    const char *const _hover_background_property        = "hover-background";
    const char *const _hover_border_property            = "hover-border";
    const char *const _focus_background_property        = "focus-background";
    const char *const _focus_border_property            = "focus-border";
    const char *const _hover_u_checkbox_bg_property     = "hover-u-checkbox-background";
    const char *const _hover_u_checkbox_border_property = "hover-u-checkbox-border";
    const char *const _hover_c_checkbox_bg_property     = "hover-c-checkbox-background";
    const char *const _hover_c_checkbox_border_property = "hover-c-checkbox-border";
} // namespace

helper::AnimationBase::AnimationBase(QWidget *parent, const CENTAUR_THEME_INTERFACE_NAMESPACE::AnimationInformation &animInfo) :
    widget { parent }
{
    setupAnimation(animInfo);
    stateMachine.reserve(4);
}

helper::AnimationBase::~AnimationBase()
{

    std::for_each(stateMachine.begin(), stateMachine.end(), [](const QStateMachine *pState) { delete pState; });
}

void helper::AnimationBase::setupAnimation(const CENTAUR_THEME_INTERFACE_NAMESPACE::AnimationInformation &animInfo)
{
    using namespace Qt::StringLiterals;
    namespace theme = CENTAUR_THEME_INTERFACE_NAMESPACE;

    auto _warning = [this]() -> void {
        qWarning() << u"animation can not be applied to \'"_s + widget->objectName() + u"\' due to a lack of step inconsistency"_s;
    };

    for (const auto &ifo : animInfo) {
        bool redrawWidget = false;

        /// \note BY NOW, animationSteps assumes it contains at least one
        /// element an as most two. Assuming only a pair of "Applications"
        /// Understanding that an application is Hover in and Hover out or Focus
        /// in or focus out

        if (ifo.animationSteps.empty())
            continue; // no need to continue

        const auto &[st0_apply, st0_step] = *ifo.animationSteps.begin();
        theme::ThemeAnimation *startStep { nullptr };
        theme::ThemeAnimation *endStep { nullptr };

        QEvent::Type startEvent {};
        QEvent::Type endEvent {};

        QString property;

        QState *s0 { nullptr };
        QState *s1 { nullptr };

        switch (st0_apply) {
            case theme::AnimationApplication::CheckBoxFocusIn:
                {
                    // Next animationSteps iterator must be HoverOut
                    const auto &[st1_apply, st1_step] = *std::next(ifo.animationSteps.begin());
                    if (st1_apply != theme::AnimationApplication::CheckBoxFocusOut) {
                        _warning();
                        continue;
                    }
                    startStep = st0_step;
                    endStep   = st1_step;

                    startEvent = QEvent::Type::Enter;
                    endEvent   = QEvent::Type::Leave;

                    s0 = new QState;
                    s1 = new QState;

                    property = "hover-";
                }
                break;
            case theme::AnimationApplication::HoverIn:
                {
                    // Next animationSteps iterator must be HoverOut
                    const auto &[st1_apply, st1_step] = *std::next(ifo.animationSteps.begin());
                    if (st1_apply != theme::AnimationApplication::HoverOut) {
                        _warning();
                        continue;
                    }
                    startStep = st0_step;
                    endStep   = st1_step;

                    startEvent = QEvent::Type::Enter;
                    endEvent   = QEvent::Type::Leave;

                    s0 = new QState;
                    s1 = new QState;

                    property = "hover-";
                }
                break;
            case theme::AnimationApplication::CheckBoxFocusOut:
                {
                    const auto &[st1_apply, st1_step] = *std::next(ifo.animationSteps.begin());
                    if (st1_apply != theme::AnimationApplication::CheckBoxFocusIn) {
                        _warning();
                        continue;
                    }

                    startStep = st1_step;
                    endStep   = st0_step;

                    startEvent = QEvent::Type::Enter;
                    endEvent   = QEvent::Type::Leave;

                    s0 = new QState;
                    s1 = new QState;

                    property = "hover-";
                }
                break;
            case theme::AnimationApplication::HoverOut:
                {

                    // Next animationSteps iterator must be HoverIn
                    const auto &[st1_apply, st1_step] = *std::next(ifo.animationSteps.begin());
                    if (st1_apply != theme::AnimationApplication::HoverIn) {
                        _warning();
                        continue;
                    }

                    startStep = st1_step;
                    endStep   = st0_step;

                    startEvent = QEvent::Type::Enter;
                    endEvent   = QEvent::Type::Leave;

                    s0 = new QState;
                    s1 = new QState;

                    property = "hover-";
                }
                break;
            case theme::AnimationApplication::FocusIn:
                {

                    // Next animationSteps iterator must be FocusOut
                    const auto &[st1_apply, st1_step] = *std::next(ifo.animationSteps.begin());
                    if (st1_apply != theme::AnimationApplication::FocusOut) {
                        _warning();
                        continue;
                    }
                    startStep = st0_step;
                    endStep   = st1_step;

                    startEvent = QEvent::Type::FocusIn;
                    endEvent   = QEvent::Type::FocusOut;

                    property = "focus-";

                    s0 = new QState;
                    s1 = new QState;
                }
                break;
            case theme::AnimationApplication::FocusOut:
                {
                    // Next animationSteps iterator must be FocusIn
                    const auto &[st1_apply, st1_step] = *std::next(ifo.animationSteps.begin());
                    if (st1_apply != theme::AnimationApplication::FocusIn) {
                        _warning();
                        continue;
                    }

                    startStep = st1_step;
                    endStep   = st0_step;

                    startEvent = QEvent::Type::FocusIn;
                    endEvent   = QEvent::Type::FocusOut;

                    property = "focus-";

                    s0 = new QState;
                    s1 = new QState;
                }
                break;
        }

        switch (ifo.element) {
            case theme::AnimationElement::Border:
                property += "border";
                redrawWidget = true;
                // The widget does not havc this internal property
                widget->setProperty(qPrintable(property),
                    ifo.start); // and the property needs to be created
                break;
            case theme::AnimationElement::Background:
                property += "background";
                redrawWidget = true;
                // The widget does not havc this internal property
                widget->setProperty(qPrintable(property),
                    ifo.start); // and the property needs to be created
                break;
            case theme::AnimationElement::uCheckBoxBackground:
                property += "u-checkbox-background";
                redrawWidget = true;
                widget->setProperty(qPrintable(property), ifo.start);
                break;
            case theme::AnimationElement::uCheckBoxBorder:
                property += "u-checkbox-border";
                redrawWidget = true;
                widget->setProperty(qPrintable(property), ifo.start);
                break;
            case theme::AnimationElement::cCheckBoxBackground:
                property += "c-checkbox-background";
                redrawWidget = true;
                widget->setProperty(qPrintable(property), ifo.start);
                break;
            case theme::AnimationElement::cCheckBoxBorder:
                property += "c-checkbox-border";
                redrawWidget = true;
                widget->setProperty(qPrintable(property), ifo.start);
                break;
        }

        s0->assignProperty(widget, qPrintable(property), ifo.start);
        s1->assignProperty(widget, qPrintable(property), ifo.end);

        auto *e0 = new QEventTransition(widget, startEvent);
        auto *e1 = new QEventTransition(widget, endEvent);

        e0->addAnimation([this, property, startStep, redrawWidget]() -> QPropertyAnimation * {
            auto *propAnim = new QPropertyAnimation(widget, qPrintable(property));
            propAnim->setEasingCurve(startStep->easingCurve);
            propAnim->setDuration(startStep->duration);

            if (redrawWidget) {
                QObject::connect(propAnim, &QPropertyAnimation::valueChanged, widget,
                    [this](C_UNUSED const QVariant &value) { widget->repaint(); });
            }

            return propAnim;
        }());

        e1->addAnimation([this, property, endStep, redrawWidget]() -> QPropertyAnimation * {
            auto *propAnim = new QPropertyAnimation(widget, qPrintable(property));
            propAnim->setEasingCurve(endStep->easingCurve);
            propAnim->setDuration(endStep->duration);

            if (redrawWidget) {
                QObject::connect(propAnim, &QPropertyAnimation::valueChanged, widget,
                    [this](C_UNUSED const QVariant &value) { widget->repaint(); });
            }

            return propAnim;
        }());

        s0->addTransition(e0);
        s1->addTransition(e1);

        e0->setTargetState(s1);
        e1->setTargetState(s0);

        auto *machine = new QStateMachine;
        machine->addState(s0);
        machine->addState(s1);
        machine->setInitialState(s0);
        machine->start();

        stateMachine.emplace_back(machine);
    }
}

CentTheme::CentTheme(CENTAUR_THEME_INTERFACE_NAMESPACE::ThemeConstants *themeConstants,
    CENTAUR_THEME_INTERFACE_NAMESPACE::ColorScheme *colorScheme,
    CENTAUR_THEME_INTERFACE_NAMESPACE::UIElements *uiElements, QPainter::RenderHints renderHints) :
    m_renderHints { renderHints },
    m_themeConstants { themeConstants },
    m_colorScheme { colorScheme },
    m_uiElements { uiElements }

{
}

CentTheme::~CentTheme() = default;

auto CentTheme::initFontFromInfo(const QFont &font_p, const cen::theme::FontTextLayout &fifo) noexcept -> QFont
{
    QFont font = font_p;

    font.setCapitalization(fifo.style.caps);
    if (!fifo.style.fontName.isEmpty())
        font.setFamily(fifo.style.fontName);

    font.setItalic(fifo.style.italic);
    font.setKerning(fifo.style.kerning);
    font.setUnderline(fifo.style.underline);

    if (fifo.style.size > 0)
        font.setPointSizeF(fifo.style.size);

    font.setWeight(static_cast<QFont::Weight>(fifo.style.weight));

    if (fifo.style.wordSpacing > -1)
        font.setWordSpacing(fifo.style.wordSpacing);

    font.setStretch(fifo.style.stretchFactor);

    if (fifo.style.letterSpacing > -1)
        font.setLetterSpacing(fifo.style.spacingType, fifo.style.letterSpacing);

    return font;
}

auto CentTheme::getPushButtonInformation(const QWidget *widget) const -> CENTAUR_THEME_INTERFACE_NAMESPACE::PushButtonInformation &
{
    auto &pbi = [this, widget]() -> auto & {
        const auto &iter = m_uiElements->pushButtonOverride.find(widget->objectName());
        if (iter != m_uiElements->pushButtonOverride.end())
            return iter->second;
        return m_uiElements->pushButtonInformation;
    }();

    return pbi;
}

auto CentTheme::getToolButtonInformation(const QWidget *widget) const -> CENTAUR_THEME_INTERFACE_NAMESPACE::ToolButtonInformation &
{
    auto &tbi = [this, widget]() -> auto & {
        const auto &iter = m_uiElements->toolButtonOverride.find(widget->objectName());
        if (iter != m_uiElements->toolButtonOverride.end())
            return iter->second;
        return m_uiElements->toolButtonInformation;
    }();

    return tbi;
}

auto CentTheme::getLineEditInformation(const QWidget *widget) const -> CENTAUR_THEME_INTERFACE_NAMESPACE::LineEditInformation &
{
    auto &lei = [this, widget]() -> auto & {
        const auto &iter = m_uiElements->lineEditOverride.find(widget->objectName());
        if (iter != m_uiElements->lineEditOverride.end())
            return iter->second;
        return m_uiElements->lineEditInformation;
    }();

    return lei;
}

auto CentTheme::getComboBoxInformation(const QWidget *widget) const -> CENTAUR_THEME_INTERFACE_NAMESPACE::ComboBoxInformation &
{
    auto &cbi = [this, widget]() -> auto & {
        const auto &iter = m_uiElements->comboBoxOverride.find(widget->objectName());
        if (iter != m_uiElements->comboBoxOverride.end())
            return iter->second;
        return m_uiElements->comboBoxInformation;
    }();

    return cbi;
}

auto CentTheme::getProgressBarInformation(const QWidget *widget) const -> CENTAUR_THEME_INTERFACE_NAMESPACE::ProgressBarInformation &
{
    const auto &iter = m_uiElements->progressBarOverride.find(widget->objectName());
    if (iter != m_uiElements->progressBarOverride.end())
        return iter->second;
    return m_uiElements->progressBarInformation;
}

auto CentTheme::getHeaderInformation(const QWidget *widget, Qt::Orientation orientation) const
    -> CENTAUR_THEME_INTERFACE_NAMESPACE::HeaderInformation &
{
    switch (orientation) {
        case Qt::Horizontal:
            {
                const auto &iter = m_uiElements->horizontalHeaderOverride.find(widget->objectName());
                if (iter != m_uiElements->horizontalHeaderOverride.end())
                    return iter->second;
                return m_uiElements->horizontalHeaderInformation;
            }

        case Qt::Vertical:
            {
                const auto &iter = m_uiElements->verticalHeaderOverride.find(widget->objectName());
                if (iter != m_uiElements->verticalHeaderOverride.end())
                    return iter->second;
                return m_uiElements->verticalHeaderInformation;
            }
    }
}

auto CentTheme::getTableViewInformation(const QWidget *widget) const -> CENTAUR_THEME_INTERFACE_NAMESPACE::TableViewInformation &
{
    const auto &iter = m_uiElements->tableViewOverride.find(widget->objectName());
    if (iter != m_uiElements->tableViewOverride.end())
        return iter->second;
    return m_uiElements->tableViewInformation;
}

auto CentTheme::getCheckBoxInformation(const QWidget *widget) const -> CENTAUR_THEME_INTERFACE_NAMESPACE::CheckBoxInformation &
{
    const QString objectName = widget == nullptr ? "" : widget->objectName();

    const auto &iter = m_uiElements->checkBoxOverride.find(objectName);
    if (iter != m_uiElements->checkBoxOverride.end())
        return iter->second;
    return m_uiElements->checkBoxInformation;
}

auto CentTheme::getGroupBoxInformation(const QWidget *widget) const -> CENTAUR_THEME_INTERFACE_NAMESPACE::GroupBoxInformation &
{
    const QString objectName = widget == nullptr ? "" : widget->objectName();

    const auto &iter = m_uiElements->groupBoxOverride.find(objectName);
    if (iter != m_uiElements->groupBoxOverride.end())
        return iter->second;
    return m_uiElements->groupBoxInformation;
}

auto CentTheme::getCheckBoxInformationState(const QStyleOption *option, CENTAUR_THEME_INTERFACE_NAMESPACE::CheckBoxInformation *state)
    -> CENTAUR_THEME_INTERFACE_NAMESPACE::CheckBoxInformation::CheckElementState &
{
    return [&]() -> CENTAUR_THEME_INTERFACE_NAMESPACE::CheckBoxInformation::CheckElementState & {
        if (!(option->state & QStyle::State_Enabled)) {
            return state->disabled.normal;
        }
        return option->state & QStyle::State_HasFocus
                   ? state->enabled.focus
                   : (option->state & QStyle::State_MouseOver ? state->enabled.hover : state->enabled.normal);
    }();
}

auto CentTheme::getPushButtonDefaultState(const QStyleOptionButton *option,
    CENTAUR_THEME_INTERFACE_NAMESPACE::PushButtonInformation *element) -> CENTAUR_THEME_INTERFACE_NAMESPACE::ElementState &
{
    return option->state & QStyle::State_HasFocus
               ? element->defaultButton.focus
               : (option->state & QStyle::State_MouseOver ? element->defaultButton.hover : element->defaultButton.normal);
}

auto CentTheme::getElementState(const QStyleOption *option, CENTAUR_THEME_INTERFACE_NAMESPACE::UIElementBasis *element, bool *validSunken)
    -> CENTAUR_THEME_INTERFACE_NAMESPACE::ElementState &
{
    const auto state = option->state.toInt();
    return [&]() -> CENTAUR_THEME_INTERFACE_NAMESPACE::ElementState & {
        if (!(state & QStyle::State_Enabled)) {
            return element->disabled.normal;
        }

        if (state & QStyle::State_Sunken && element->enabled.pressed.brush != Qt::NoBrush) {
            if (validSunken != nullptr)
                *validSunken = true;
            return element->enabled.pressed;
        }
        else {
            if (validSunken != nullptr)
                *validSunken = false;
        }

        return state & QStyle::State_HasFocus
                   ? element->enabled.focus
                   : (state & QStyle::State_MouseOver ? element->enabled.hover : element->enabled.normal);
    }();
}

auto CentTheme::getComboBoxState(const QStyleOptionComboBox *option, CENTAUR_THEME_INTERFACE_NAMESPACE::ComboBoxInformation *element)
    -> std::tuple<CENTAUR_THEME_INTERFACE_NAMESPACE::ElementState &, QPen, int>
{
    return { [&]() -> CENTAUR_THEME_INTERFACE_NAMESPACE::ElementState & {
                if (!(option->state & QStyle::State_Enabled)) {
                    return element->disabled.normal;
                }
                return option->state & QStyle::State_HasFocus
                           ? element->enabled.focus
                           : (option->state & QStyle::State_MouseOver ? element->enabled.hover : element->enabled.normal);
            }(),
        element->dropArrowPen, element->dropArrowSize };
}

void CentTheme::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter,
    const QWidget *widget) const
{
    painter->setRenderHints(m_renderHints);

    switch (element) {
        case PE_Frame: return;
        case PE_FrameDefaultButton: C_FALLTHROUGH;
        case PE_FrameDockWidget: break;
        case PE_FrameFocusRect: painter->fillRect(option->rect, QColor(0, 0, 255)); return;
        case PE_FrameGroupBox: C_FALLTHROUGH;
        case PE_FrameLineEdit: C_FALLTHROUGH;
        case PE_FrameMenu: C_FALLTHROUGH;
        case PE_FrameStatusBarItem: C_FALLTHROUGH;
        case PE_FrameTabWidget: C_FALLTHROUGH;
        case PE_FrameWindow: C_FALLTHROUGH;
        case PE_FrameButtonBevel: C_FALLTHROUGH;
        case PE_FrameButtonTool: C_FALLTHROUGH;
        case PE_FrameTabBarBase: C_FALLTHROUGH;
        case PE_PanelButtonCommand: C_FALLTHROUGH;
        case PE_PanelButtonBevel: C_FALLTHROUGH;
        case PE_PanelButtonTool: C_FALLTHROUGH;
        case PE_PanelMenuBar: C_FALLTHROUGH;
        case PE_PanelToolBar: break;
        case PE_PanelLineEdit:
            drawEditLinePanel(option, painter, qobject_cast<const QLineEdit *>(widget));
            return;
        case PE_IndicatorArrowDown: C_FALLTHROUGH;
        case PE_IndicatorArrowLeft: C_FALLTHROUGH;
        case PE_IndicatorArrowRight: C_FALLTHROUGH;
        case PE_IndicatorArrowUp: C_FALLTHROUGH;
        case PE_IndicatorBranch: C_FALLTHROUGH;
        case PE_IndicatorButtonDropDown: break;
        case PE_IndicatorItemViewItemCheck:
            drawCheckBoxIndicator(option, painter, widget, false);
            return;
        case PE_IndicatorCheckBox:
            drawCheckBox(qstyleoption_cast<const QStyleOptionButton *>(option), painter, qobject_cast<const QCheckBox *>(widget));
            return;

        case PE_IndicatorDockWidgetResizeHandle: break;
        case PE_IndicatorHeaderArrow:
            {
                const auto *header = qobject_cast<const QHeaderView *>(widget);
                auto opt           = *qstyleoption_cast<const QStyleOptionHeader *>(option);

                auto headerInformation = getHeaderInformation(header, header->orientation());

                opt.palette.setColor(QPalette::ColorRole::Text, headerInformation.fontPen.color());

                const auto state = opt.state.toInt();
                if (state & QStyle::State_MouseOver)
                    opt.palette.setColor(QPalette::ColorRole::Text, headerInformation.hoverPen.color());

                if (state & QStyle::State_Sunken)
                    opt.palette.setColor(QPalette::ColorRole::Text, headerInformation.sunkenPen.color());

                QProxyStyle::drawPrimitive(element, &opt, painter, widget);
            }
            return;
        case PE_IndicatorMenuCheckMark: C_FALLTHROUGH;
        case PE_IndicatorProgressChunk: C_FALLTHROUGH;
        case PE_IndicatorRadioButton: C_FALLTHROUGH;
        case PE_IndicatorSpinDown: C_FALLTHROUGH;
        case PE_IndicatorSpinMinus: C_FALLTHROUGH;
        case PE_IndicatorSpinPlus: C_FALLTHROUGH;
        case PE_IndicatorSpinUp: C_FALLTHROUGH;
        case PE_IndicatorToolBarHandle: C_FALLTHROUGH;
        case PE_IndicatorToolBarSeparator: C_FALLTHROUGH;
        case PE_PanelTipLabel: C_FALLTHROUGH;
        case PE_IndicatorTabTear: break;
        case PE_PanelScrollAreaCorner:
            // TODO: THIS
            painter->fillRect(option->rect, Qt::green);
            return;
        case PE_Widget: break;
        case PE_IndicatorColumnViewArrow: C_FALLTHROUGH;
        case PE_IndicatorItemViewItemDrop: break;
        case PE_PanelItemViewItem:
            drawTableViewItem(qstyleoption_cast<const QStyleOptionViewItem *>(option), painter, widget);
            return;
        case PE_PanelItemViewRow:
            drawTableViewItemRow(qstyleoption_cast<const QStyleOptionViewItem *>(option), painter, widget);
            return;
        case PE_PanelStatusBar: C_FALLTHROUGH;
        case PE_IndicatorTabClose: break;
        case PE_PanelMenu: drawPanelMenu(option, painter); return;
        case PE_IndicatorTabTearRight: C_FALLTHROUGH;
        case PE_CustomBase: break;
    }
    // qDebug() << "ELE" << element;
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

void CentTheme::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    painter->setRenderHints(m_renderHints);

    switch (element) {
        case CE_PushButton: drawPushButton(qstyleoption_cast<const QStyleOptionButton *>(option), painter, widget); return;
        case CE_PushButtonBevel:
            /// Intentionally ignored
            break;
        case CE_PushButtonLabel: drawPushButtonText(qstyleoption_cast<const QStyleOptionButton *>(option), painter, widget); return;
        case CE_CheckBox:
            drawCheckBox(qstyleoption_cast<const QStyleOptionButton *>(option), painter, qobject_cast<const QCheckBox *>(widget));
            return;
        case CE_CheckBoxLabel:
            drawCheckBoxLabel(qstyleoption_cast<const QStyleOptionButton *>(option), painter, qobject_cast<const QCheckBox *>(widget));
            return;
        case CE_RadioButton: C_FALLTHROUGH;
        case CE_RadioButtonLabel: C_FALLTHROUGH;
        case CE_TabBarTab: C_FALLTHROUGH;
        case CE_TabBarTabShape: C_FALLTHROUGH;
        case CE_TabBarTabLabel: break;
        case CE_ProgressBar:
            drawProgressBarContents(qstyleoption_cast<const QStyleOptionProgressBar *>(option), painter,
                qobject_cast<const QProgressBar *>(widget));
            return;
        case CE_ProgressBarGroove: C_FALLTHROUGH;
        case CE_ProgressBarLabel: C_FALLTHROUGH;
        case CE_ProgressBarContents: return;

        case CE_MenuItem: drawMenuItem(qstyleoption_cast<const QStyleOptionMenuItem *>(option), painter, widget); return;
        case CE_MenuHMargin: return;
        case CE_MenuScroller: break;
        case CE_MenuVMargin: return;

        case CE_MenuTearoff: break;
        case CE_MenuEmptyArea: drawMenuEmptyArea(option, painter); return;
        case CE_MenuBarItem: C_FALLTHROUGH;
        case CE_MenuBarEmptyArea: break;
        case CE_ToolButtonLabel:
            drawToolButtonLabel(qstyleoption_cast<const QStyleOptionToolButton *>(option), painter, widget);
            return;
        case CE_Header:
            drawHeaderView(qstyleoption_cast<const QStyleOptionHeader *>(option), painter, qobject_cast<const QHeaderView *>(widget));
            return;
        case CE_HeaderSection:
            drawHeaderSection(qstyleoption_cast<const QStyleOptionHeader *>(option), painter,
                qobject_cast<const QHeaderView *>(widget));
            return;
        case CE_HeaderLabel:
            drawHeaderLabel(qstyleoption_cast<const QStyleOptionHeader *>(option), painter, qobject_cast<const QHeaderView *>(widget));
            return;

        case CE_ToolBoxTab: C_FALLTHROUGH;
        case CE_SizeGrip: C_FALLTHROUGH;
        case CE_Splitter: C_FALLTHROUGH;
        case CE_RubberBand: C_FALLTHROUGH;
        case CE_DockWidgetTitle: C_FALLTHROUGH;
        case CE_ScrollBarAddLine: C_FALLTHROUGH;
        case CE_ScrollBarSubLine: C_FALLTHROUGH;
        case CE_ScrollBarAddPage: C_FALLTHROUGH;
        case CE_ScrollBarSubPage: C_FALLTHROUGH;
        case CE_ScrollBarSlider: C_FALLTHROUGH;
        case CE_ScrollBarFirst: C_FALLTHROUGH;
        case CE_ScrollBarLast: break;
        case CE_FocusFrame:
            if (const auto *frame = qobject_cast<const QFocusFrame *>(widget); frame != nullptr) {
                // Avoid drawing focus frame
                return;
            }
            break;
        case CE_ComboBoxLabel: return;
        case CE_ToolBar: C_FALLTHROUGH;
        case CE_ToolBoxTabShape: C_FALLTHROUGH;
        case CE_ToolBoxTabLabel: C_FALLTHROUGH;
        case CE_ColumnViewGrip: break;
        case CE_HeaderEmptyArea: drawHeaderEmptyArea(option, painter, qobject_cast<const QHeaderView *>(widget)); return;

        case CE_ItemViewItem:
            drawTableView(qstyleoption_cast<const QStyleOptionViewItem *>(option), painter, widget);
            return;
        case CE_ShapedFrame:

            if (qobject_cast<const QTableView *>(widget) != nullptr || qobject_cast<const QTableWidget *>(widget) != nullptr) {
                drawTableFrameBackground(option, painter, widget);
                return;
            }
            // painter->fillRect(option->rect, Qt::blue);
            return;

        case CE_CustomBase: break;
    }
    // qDebug() << element;
    QProxyStyle::drawControl(element, option, painter, widget);
}

void CentTheme::drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, QPainter *painter,
    const QWidget *widget) const
{
    painter->setRenderHints(m_renderHints);
    switch (control) {

        case CC_Dial: C_FALLTHROUGH;

        case CC_MdiControls: C_FALLTHROUGH;
        case CC_CustomBase: C_FALLTHROUGH;
        case CC_ScrollBar: C_FALLTHROUGH;
        case CC_SpinBox: C_FALLTHROUGH;
        case CC_Slider: break;
        case CC_ComboBox:
            drawComboBox(qstyleoption_cast<const QStyleOptionComboBox *>(option), painter, qobject_cast<const QComboBox *>(widget));
            return;
        case CC_GroupBox:
            drawGroupBox(qstyleoption_cast<const QStyleOptionGroupBox *>(option), painter, widget);
            return;

        case CC_ToolButton:
            drawToolButton(qstyleoption_cast<const QStyleOptionToolButton *>(option), painter, widget);
            return;
        case CC_TitleBar: return;
    }
    // qDebug() << "CE" << control;
    QProxyStyle::drawComplexControl(control, option, painter, widget);
}

void CentTheme::drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text,
    QPalette::ColorRole textRole) const
{
    QProxyStyle::drawItemText(painter, rect, flags, pal, enabled, text, textRole);
}

void CentTheme::drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const
{
    QProxyStyle::drawItemPixmap(painter, rect, alignment, pixmap);
}

QSize CentTheme::sizeFromContents(QStyle::ContentsType type, const QStyleOption *option, const QSize &size,
    const QWidget *widget) const
{
    switch (type) {

        case CT_PushButton: C_FALLTHROUGH;
        case CT_CheckBox: C_FALLTHROUGH;
        case CT_RadioButton: C_FALLTHROUGH;
        case CT_ToolButton: C_FALLTHROUGH;
        case CT_ComboBox: C_FALLTHROUGH;
        case CT_Splitter: C_FALLTHROUGH;
        case CT_ProgressBar: break;
        case CT_MenuItem:
            {
                const auto *opt = qstyleoption_cast<const QStyleOptionMenuItem *>(option);
                auto _size      = QProxyStyle::sizeFromContents(type, option, size, widget);
                if ((opt != nullptr) && opt->menuItemType == QStyleOptionMenuItem::Separator) {
                    _size.setHeight(std::min(m_uiElements->menuInformation.separatorHeight, _size.height()));
                }
                else {
                    _size.setHeight(std::max(m_uiElements->menuInformation.itemHeight, _size.height()));
                }
                return _size;
            }
        case CT_MenuBarItem: C_FALLTHROUGH;
        case CT_MenuBar: C_FALLTHROUGH;
        case CT_Menu: C_FALLTHROUGH;
        case CT_TabBarTab: C_FALLTHROUGH;
        case CT_Slider: C_FALLTHROUGH;
        case CT_ScrollBar: C_FALLTHROUGH;
        case CT_LineEdit: C_FALLTHROUGH;
        case CT_SpinBox: C_FALLTHROUGH;
        case CT_SizeGrip: C_FALLTHROUGH;
        case CT_TabWidget: C_FALLTHROUGH;
        case CT_DialogButtons: C_FALLTHROUGH;
        case CT_HeaderSection: C_FALLTHROUGH;
        case CT_GroupBox: C_FALLTHROUGH;
        case CT_MdiControls: C_FALLTHROUGH;
        case CT_CustomBase: break;
        case CT_ItemViewItem:
            {
                auto _size = QProxyStyle::sizeFromContents(type, option, size, widget);

                const QString objectName = widget == nullptr ? "" : widget->objectName();
                const auto newHeight     = [&]() -> int {
                    auto iter = m_themeConstants->treeItemHeight.find(objectName);
                    if (iter == m_themeConstants->treeItemHeight.end()) {
                        iter = m_themeConstants->treeItemHeight.find("*");
                        if (iter != m_themeConstants->treeItemHeight.end())
                            return iter->second;
                        return _size.height();
                    }
                    return iter->second;
                }();

                if (newHeight > -1)
                    _size.setHeight(newHeight);
                return _size;
            }
    }
    // qDebug() << type;
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

QRect CentTheme::subElementRect(QStyle::SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    switch (element) {
        case SE_PushButtonContents: C_FALLTHROUGH;
        case SE_PushButtonFocusRect: C_FALLTHROUGH;
        case SE_CheckBoxIndicator: C_FALLTHROUGH;
        case SE_CheckBoxContents: C_FALLTHROUGH;
        case SE_CheckBoxFocusRect: C_FALLTHROUGH;
        case SE_CheckBoxClickRect: C_FALLTHROUGH;
        case SE_RadioButtonIndicator: C_FALLTHROUGH;
        case SE_RadioButtonContents: C_FALLTHROUGH;
        case SE_RadioButtonFocusRect: C_FALLTHROUGH;
        case SE_RadioButtonClickRect: C_FALLTHROUGH;
        case SE_ComboBoxFocusRect: C_FALLTHROUGH;
        case SE_ProgressBarGroove: C_FALLTHROUGH;
        case SE_ProgressBarContents: C_FALLTHROUGH;
        case SE_ProgressBarLabel: C_FALLTHROUGH;
        case SE_SliderFocusRect: C_FALLTHROUGH;
        case SE_ToolBoxTabContents: C_FALLTHROUGH;
        case SE_HeaderLabel: C_FALLTHROUGH;
        case SE_HeaderArrow: C_FALLTHROUGH;
        case SE_TabWidgetTabBar: C_FALLTHROUGH;
        case SE_TabWidgetTabPane: C_FALLTHROUGH;
        case SE_TabWidgetTabContents: C_FALLTHROUGH;
        case SE_TabWidgetLeftCorner: C_FALLTHROUGH;
        case SE_TabWidgetRightCorner: C_FALLTHROUGH;
        case SE_ItemViewItemCheckIndicator: C_FALLTHROUGH;
        case SE_TabBarTearIndicator: C_FALLTHROUGH;
        case SE_TreeViewDisclosureItem: break;
        case SE_LineEditContents:
            {
                auto state = getElementState(option, &getLineEditInformation(widget));
                auto rect  = QProxyStyle::subElementRect(element, option, widget)
                                .marginsRemoved(state.fi.margins)
                                .marginsRemoved(state.fi.padding);

                return visualRect(option->direction, option->rect, rect);
            }
        case SE_FrameContents: C_FALLTHROUGH;
        case SE_DockWidgetCloseButton: C_FALLTHROUGH;
        case SE_DockWidgetFloatButton: C_FALLTHROUGH;
        case SE_DockWidgetTitleBarText: C_FALLTHROUGH;
        case SE_DockWidgetIcon: C_FALLTHROUGH;
        case SE_CheckBoxLayoutItem: C_FALLTHROUGH;
        case SE_ComboBoxLayoutItem: C_FALLTHROUGH;
        case SE_DateTimeEditLayoutItem: C_FALLTHROUGH;
        case SE_LabelLayoutItem: C_FALLTHROUGH;
        case SE_ProgressBarLayoutItem: C_FALLTHROUGH;
        case SE_PushButtonLayoutItem: C_FALLTHROUGH;
        case SE_RadioButtonLayoutItem: C_FALLTHROUGH;
        case SE_SliderLayoutItem: C_FALLTHROUGH;
        case SE_SpinBoxLayoutItem: C_FALLTHROUGH;
        case SE_ToolButtonLayoutItem: C_FALLTHROUGH;
        case SE_FrameLayoutItem: C_FALLTHROUGH;
        case SE_GroupBoxLayoutItem: C_FALLTHROUGH;
        case SE_TabWidgetLayoutItem: C_FALLTHROUGH;
        case SE_ItemViewItemDecoration: C_FALLTHROUGH;
        case SE_ItemViewItemText: C_FALLTHROUGH;
        case SE_ItemViewItemFocusRect: C_FALLTHROUGH;
        case SE_TabBarTabLeftButton: C_FALLTHROUGH;
        case SE_TabBarTabRightButton: C_FALLTHROUGH;
        case SE_TabBarTabText: C_FALLTHROUGH;
        case SE_ShapedFrameContents: C_FALLTHROUGH;
        case SE_ToolBarHandle: C_FALLTHROUGH;
        case SE_TabBarScrollLeftButton: C_FALLTHROUGH;
        case SE_TabBarScrollRightButton: C_FALLTHROUGH;
        case SE_TabBarTearIndicatorRight: C_FALLTHROUGH;
        case SE_PushButtonBevel: C_FALLTHROUGH;
        case SE_CustomBase: break;
    }
    // qDebug() << "SER" << element;

    return QProxyStyle::subElementRect(element, option, widget);
}

QRect CentTheme::subControlRect(QStyle::ComplexControl cc, const QStyleOptionComplex *opt, QStyle::SubControl sc,
    const QWidget *widget) const
{
    if (cc == CC_GroupBox) {

        auto &gbi = getGroupBoxInformation(widget);

        QRect rect = QProxyStyle::subControlRect(cc, opt, sc, widget);

        rect = rect.marginsRemoved(gbi.fi.margins);

        if ((sc == SC_GroupBoxFrame || sc == SC_GroupBoxContents)
            && gbi.headerHeight > -1) {
            rect.setTop(gbi.headerHeight);
        }
        else if (sc == SC_GroupBoxLabel || sc == SC_GroupBoxCheckBox) {
            const auto &height         = gbi.headerHeight;
            const auto &indicatorWidth = gbi.indicatorWidth;

            if (height > -1) {
                rect.setRight(opt->rect.right());
                rect.setTop(opt->rect.top());
                rect.setHeight(height);
                rect.setLeft(rect.left() + indicatorWidth);
            }
            else
                rect.moveTo(rect.left() + indicatorWidth, rect.top());
        }
        return rect;
    }
    return QProxyStyle::subControlRect(cc, opt, sc, widget);
}

QRect CentTheme::itemTextRect(const QFontMetrics &fm, const QRect &r, int flags, bool enabled, const QString &text) const
{
    return QProxyStyle::itemTextRect(fm, r, flags, enabled, text);
}

QRect CentTheme::itemPixmapRect(const QRect &r, int flags, const QPixmap &pixmap) const
{
    return QProxyStyle::itemPixmapRect(r, flags, pixmap);
}

QStyle::SubControl CentTheme::hitTestComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option,
    const QPoint &pos, const QWidget *widget) const
{
    return QProxyStyle::hitTestComplexControl(control, option, pos, widget);
}

int CentTheme::styleHint(QStyle::StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    if (hint == QStyle::StyleHint::SH_Table_GridLineColor) {
        auto &tvi  = getTableViewInformation(widget);
        auto brush = tvi.gridLinesPen.brush();
        if (brush.style() == Qt::BrushStyle::SolidPattern) {
            // Only return if brush style is a solid pattern
            return static_cast<int>(brush.color().rgba());
        }
    }

    if (hint == QStyle::StyleHint::SH_ScrollBar_Transient)
        return 1;

    return QProxyStyle::styleHint(hint, option, widget, returnData);
}

int CentTheme::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    return QProxyStyle::pixelMetric(metric, option, widget);
}

int CentTheme::layoutSpacing(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2, Qt::Orientation orientation,
    const QStyleOption *option, const QWidget *widget) const
{
    return QProxyStyle::layoutSpacing(control1, control2, orientation, option, widget);
}

QIcon CentTheme::standardIcon(QStyle::StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const
{
    return QProxyStyle::standardIcon(standardIcon, option, widget);
}

QPixmap CentTheme::standardPixmap(QStyle::StandardPixmap standardPixmap, const QStyleOption *opt, const QWidget *widget) const
{
    return QProxyStyle::standardPixmap(standardPixmap, opt, widget);
}

QPixmap CentTheme::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt) const
{
    return QProxyStyle::generatedIconPixmap(iconMode, pixmap, opt);
}

QPalette CentTheme::standardPalette() const { return QProxyStyle::standardPalette(); }

void CentTheme::polish(QWidget *widget)
{
#ifdef Q_OS_MAC
    widget->setAttribute(Qt::WA_MacShowFocusRect, false);
#endif

    const auto *className = widget->metaObject()->className();

    CENTAUR_THEME_INTERFACE_NAMESPACE::AnimationInformation animInfo;
    if (strcmp(className, "QPushButton") == 0) {
        // Get the information
        auto &pbi = getPushButtonInformation(widget);
        animInfo  = pbi.animations;
    }
    else if (strcmp(className, "QLineEdit") == 0) {
        auto &lei = getLineEditInformation(widget);
        animInfo  = lei.animations;

        bool paletteMods = false;
        auto palette     = widget->palette();

        if (lei.textColor.isValid() && widget->isEnabled()) {
            palette.setColor(QPalette::Text, lei.textColor);
            paletteMods = true;
        }

        if (lei.disableTextColor.isValid() && !widget->isEnabled()) {
            palette.setColor(QPalette::Text, lei.disableTextColor);
            paletteMods = true;
        }

        if (lei.placeHolderColor.isValid()) {
            palette.setColor(QPalette::PlaceholderText, lei.placeHolderColor);
            paletteMods = true;
        }

        if (paletteMods)
            widget->setPalette(palette);

        const auto state = [&]() {
            return widget->isEnabled() ? lei.enabled.normal : lei.disabled.normal;
        }();

        widget->setFont(initFontFromInfo(widget->font(), state.fontInformation));
    }
    else if (strcmp(className, "QComboBox") == 0) {
        auto &cbi = getComboBoxInformation(widget);
        animInfo  = cbi.animations;
    }
    else if (strcmp(className, "QProgressBar") == 0) {
        auto &pbi = getProgressBarInformation(widget);

        if (pbi.applyGlowEffect) {
            const QPointer<QGraphicsDropShadowEffect> effect = new QGraphicsDropShadowEffect(widget);
            effect->setColor(pbi.effectInformation.glowColor);
            effect->setBlurRadius(pbi.effectInformation.blurRadius);
            effect->setXOffset(pbi.effectInformation.xOffset);
            effect->setYOffset(pbi.effectInformation.yOffset);
            widget->setGraphicsEffect(effect);
        }
    }
    else if (strcmp(className, "QHeaderView") == 0) {
        widget->setAttribute(Qt::WA_Hover, true);
    }
    else if (strcmp(className, "QCheckBox") == 0) {
        auto &cbi = getCheckBoxInformation(widget);
        widget->setAttribute(Qt::WA_Hover, true);
        animInfo = cbi.animations;
    }
    else if (strcmp(className, "QTableWidget") == 0
             || strcmp(className, "QTableView") == 0
             || strcmp(className, "QTreeWidget") == 0
             || strcmp(className, "QTreeView") == 0) {
        auto &tvi = getTableViewInformation(widget);
        // widget->setAttribute(Qt::WA_Hover, true);

        widget->setMouseTracking(tvi.mouseOverItem);

        auto *tableView = qobject_cast<QTableView *>(widget);
        if (tableView != nullptr)
            tableView->setShowGrid(tvi.gridLines);

        auto palette = widget->palette();
        palette.setBrush(QPalette::ColorGroup::All, QPalette::ColorRole::Base, tvi.backgroundBrush);
        palette.setBrush(QPalette::ColorGroup::Disabled, QPalette::ColorRole::Base, tvi.disableBackgroundBrush);
        widget->setPalette(palette);

        if (tableView != nullptr && tvi.gridLinesPen.style() != Qt::PenStyle::NoPen) {
            // Only apply the grid lines style if it's valid
            qobject_cast<QTableView *>(widget)->setGridStyle(tvi.gridLinesPen.style());
        }
    }
    else if (strcmp(className, "QToolButton") == 0) {
        auto &tbi = getToolButtonInformation(widget);
        widget->setAttribute(Qt::WA_Hover, true);
        animInfo = tbi.animations;
    }

    if (!animInfo.empty()) {
        auto iter = m_widgetAnimations.find(widget);
        if (iter == m_widgetAnimations.end())
            m_widgetAnimations.insert({ widget, std::make_unique<helper::AnimationBase>(widget, animInfo) });
    }

    QProxyStyle::polish(widget);
}

void CentTheme::polish(QPalette &pal)
{
    QProxyStyle::polish(pal);

    pal.setColor(QPalette::ColorRole::Window, QColor(0, 0, 0));
}

void CentTheme::polish(QApplication *app) { QProxyStyle::polish(app); }

void CentTheme::unpolish(QWidget *widget)
{
    if (auto btnAnim = m_widgetAnimations.find(widget); btnAnim != m_widgetAnimations.end()) {
        m_widgetAnimations.erase(btnAnim);
    }
    QProxyStyle::unpolish(widget);
}

void CentTheme::unpolish(QApplication *app) { QProxyStyle::unpolish(app); }

void CentTheme::drawFrameAnimation(QPainter *painter, const QWidget *widget, const cen::theme::ElementState &state, QRect &widgetRect, bool isSunken)
{
    const CENTAUR_THEME_INTERFACE_NAMESPACE::FrameInformation &frameInformation = state.fi;

    widgetRect = widgetRect.marginsRemoved(state.fi.margins);

    QPen pen     = state.pen;
    QBrush brush = state.brush;

    painter->save();
    {
        if (widget->isEnabled() & !isSunken) {
            if (widget->hasFocus()) {
                const auto focusBackgroundPropertyAnimation = widget->property(_focus_background_property);
                if (focusBackgroundPropertyAnimation.isValid())
                    brush.setColor(focusBackgroundPropertyAnimation.value<QColor>());

                const auto focusBorderPropertyAnimation = widget->property(_focus_border_property);
                if (focusBorderPropertyAnimation.isValid())
                    pen.setColor(focusBorderPropertyAnimation.value<QColor>());
            }
            else {
                const auto hoverBackgroundPropertyAnimation = widget->property(_hover_background_property);
                if (hoverBackgroundPropertyAnimation.isValid())
                    brush.setColor(hoverBackgroundPropertyAnimation.value<QColor>());

                const auto hoverBorderPropertyAnimation = widget->property(_hover_border_property);
                if (hoverBorderPropertyAnimation.isValid())
                    pen.setColor(hoverBorderPropertyAnimation.value<QColor>());
            }
        }

        painter->setBrush(brush);
        painter->setPen(pen);

        drawFrame(painter, frameInformation, widgetRect);
    }

    painter->restore();
}

void CentTheme::drawFrame(QPainter *painter, const cen::theme::FrameInformation &frameInformation, const QRect &widgetRect)
{
    static constexpr const int g_angle270 = 270;
    static constexpr const int g_angle180 = 180;
    static constexpr const int g_angle90  = 90;
    if (qFuzzyCompare(frameInformation.borderRadiusX, 0.0) || qFuzzyCompare(frameInformation.borderRadiusY, 0.0)) {
        if (frameInformation.borderRadiusTopLeft > 0.0 || frameInformation.borderRadiusBottomLeft > 0.0
            || frameInformation.borderRadiusTopRight > 0.0 || frameInformation.borderRadiusBottomRight > 0.0) {
            const qreal widgetX = widgetRect.x();
            const qreal widgetY = widgetRect.y();
            const qreal width   = widgetRect.width();
            const qreal height  = widgetRect.height();

            QPainterPath path;
            if (frameInformation.borderRadiusTopLeft > 0.0) {
                path.moveTo(widgetX + frameInformation.borderRadiusTopLeft, widgetY);
                path.arcTo(widgetX, widgetY, widgetX + (frameInformation.borderRadiusTopLeft * 2),
                    widgetY + (frameInformation.borderRadiusTopLeft * 2), g_angle90, g_angle90);
            }
            else
                path.moveTo(widgetX, widgetY);

            if (frameInformation.borderRadiusBottomLeft > 0) {
                path.lineTo(widgetX, widgetY + height - frameInformation.borderRadiusBottomLeft);
                path.arcTo(widgetX, widgetY + height - (frameInformation.borderRadiusBottomLeft * 2),
                    frameInformation.borderRadiusBottomLeft * 2, frameInformation.borderRadiusBottomLeft * 2, g_angle180, g_angle90);
            }
            else
                path.lineTo(widgetX, widgetY + height);

            if (frameInformation.borderRadiusBottomRight > 0) {
                path.lineTo(widgetX - frameInformation.borderRadiusBottomRight, widgetY + height);
                path.arcTo(widgetX + width - (frameInformation.borderRadiusBottomRight * 2),
                    widgetY + height - (frameInformation.borderRadiusBottomRight * 2), frameInformation.borderRadiusBottomRight * 2,
                    frameInformation.borderRadiusBottomRight * 2, g_angle270, g_angle90);
            }
            else
                path.lineTo(widgetX + width, widgetY + height);

            if (frameInformation.borderRadiusTopRight > 0) {
                path.lineTo(widgetX + width, widgetY + frameInformation.borderRadiusTopRight);
                path.arcTo(widgetX + width - (frameInformation.borderRadiusTopRight * 2), widgetY,
                    frameInformation.borderRadiusTopRight * 2, frameInformation.borderRadiusTopRight * 2, 0, g_angle90);
            }
            else
                path.lineTo(widgetX + width, widgetY);

            path.closeSubpath();
            painter->drawPath(path);
        }
        else
            painter->drawRect(widgetRect);
    }
    else {
        const qreal radiusX = frameInformation.borderRadiusX > 0.0 ? frameInformation.borderRadiusX
                                                                   : std::min(widgetRect.width(), widgetRect.height()) / 2.0;
        const qreal radiusY = frameInformation.borderRadiusY > 0.0 ? frameInformation.borderRadiusY
                                                                   : std::min(widgetRect.width(), widgetRect.height()) / 2.0;

        painter->drawRoundedRect(widgetRect, radiusX, radiusY);
    }
}

void CentTheme::drawPushButton(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget) const
{
    const auto *button = qobject_cast<const QPushButton *>(widget);

    auto &pbi = getPushButtonInformation(widget);

    bool validSunken = false;
    auto state       = [&]() -> CENTAUR_THEME_INTERFACE_NAMESPACE::ElementState       &{
        if (button->isDefault() && button->isEnabled()) {
            return getPushButtonDefaultState(option, std::addressof(pbi));
        }
        return getElementState(option, std::addressof(pbi), &validSunken);
    }();

    QRect rect = option->rect;
    CentTheme::drawFrameAnimation(painter, button, state, rect, validSunken);

    painter->save();
    {
        const auto &fifo = state.fontInformation;

        const QFont font = initFontFromInfo(QApplication::font(), fifo);

        painter->setFont(font);
        painter->setPen(state.fontPen);

        rect = rect.marginsRemoved(state.fi.padding);
        painter->drawText(rect, option->text, fifo.opts);
    }
    painter->restore();
}

void CentTheme::drawPushButtonText(C_UNUSED const QStyleOptionButton *option, C_UNUSED QPainter *painter,
    C_UNUSED const QWidget *widget) const
{
}

void CentTheme::drawEditLinePanel(const QStyleOption *option, QPainter *painter, const QLineEdit *widget) const
{
    auto &state = getElementState(option, &getLineEditInformation(widget));

    QRect rect = option->rect;

    CentTheme::drawFrameAnimation(painter, widget, state, rect);
}

void CentTheme::drawComboBox(const QStyleOptionComboBox *option, QPainter *painter, const QComboBox *widget) const
{
    static constexpr int internalMargin = 7;
    auto [ui, dropPen, dropSize]        = getComboBoxState(option, &getComboBoxInformation(widget));

    QRect rect = option->rect;

    CentTheme::drawFrameAnimation(painter, widget, ui, rect);

    QRect dropRect = rect;

    dropRect.setLeft(rect.right() - dropSize);
    rect.setWidth(rect.width() - dropSize);

    painter->save();
    {
        painter->setPen(dropPen);
        painter->drawLine(dropRect.left(), dropRect.top() + 1, dropRect.left(), dropRect.bottom() - 1);

        dropRect = dropRect.marginsRemoved(QMargins(internalMargin, 0, internalMargin, 0));

        if (dropRect.width() > dropRect.height()) {
            const int diff = dropRect.width() - dropRect.height();

            dropRect.setLeft(dropRect.left() + (diff / 2));
            dropRect.setWidth(dropRect.height());
        }
        else {
            const int diff = dropRect.height() - dropRect.width();

            dropRect.setTop(dropRect.top() + (diff / 2));
            dropRect.setHeight(dropRect.width());
        }

        painter->drawPixmap(dropRect, (option->state & QStyle::State_Enabled) ? m_comboBoxDownArrow : m_comboBoxDownArrowGray);

        painter->setPen(ui.fontPen);
        painter->setFont(initFontFromInfo(QApplication::font(), ui.fontInformation));

        rect = rect.marginsRemoved(ui.fi.padding);

        painter->drawText(rect, option->currentText, ui.fontInformation.opts);
    }
    painter->restore();
}

void CentTheme::drawPanelMenu(const QStyleOption *option, QPainter *painter) const
{
    painter->save();
    {
        painter->fillRect(option->rect, m_uiElements->menuInformation.panelBrush);
    }
    painter->restore();
}

void CentTheme::drawMenuEmptyArea(const QStyleOption *option, QPainter *painter) const
{
    painter->save();
    {
        painter->fillRect(option->rect, m_uiElements->menuInformation.emptyAreaBrush);
    }
    painter->restore();
}

void CentTheme::drawMenuItem(const QStyleOptionMenuItem *option, QPainter *painter, C_UNUSED const QWidget *widget) const
{
    painter->save();
    {
        // qDebug() << option->state << option->icon << option->checkType <<
        // option->menuItemType << option->reservedShortcutWidth << option->rect
        // << option->menuRect << widget;

        QRect itemRect = option->rect;
        itemRect.setLeft(itemRect.left() + m_uiElements->menuInformation.leftPadding);
        bool item = false;
        QTextOption options {};
        QIcon::Mode iconMode { QIcon::Mode::Normal };
        QIcon::State iconState { QIcon::State::On };

        const auto &state = option->state.toInt();

        switch (option->menuItemType) {
            case QStyleOptionMenuItem::Separator:
                {
                    if (m_uiElements->menuInformation.separatorBrush != Qt::NoBrush)
                        painter->fillRect(option->menuRect, m_uiElements->menuInformation.separatorBrush);

                    const int yPos = option->rect.top() + option->rect.height() / 2;

                    painter->setPen(m_uiElements->menuInformation.separatorPen);
                    painter->drawLine(option->menuRect.left(), yPos, option->menuRect.right(), yPos);
                }
                break;
            case QStyleOptionMenuItem::Normal: C_FALLTHROUGH;
            case QStyleOptionMenuItem::DefaultItem: item = true; C_FALLTHROUGH;
            case QStyleOptionMenuItem::SubMenu:
                {
                    if (state & QStyle::State_Selected && state & QStyle::State_Enabled) {
                        painter->setPen(m_uiElements->menuInformation.selectedPen);
                        painter->setFont(initFontFromInfo(QApplication::font(), m_uiElements->menuInformation.selectedFont));
                        options  = m_uiElements->menuInformation.selectedFont.opts;
                        iconMode = QIcon::Mode::Selected;
                    }
                    else if (!(state & QStyle::State_Enabled)) {
                        painter->setPen(m_uiElements->menuInformation.disabledPen);
                        painter->setFont(initFontFromInfo(QApplication::font(), m_uiElements->menuInformation.enabledFont));
                        options  = m_uiElements->menuInformation.enabledFont.opts;
                        iconMode = QIcon::Mode::Normal;
                    }
                    else {
                        painter->setPen(m_uiElements->menuInformation.enabledPen);
                        painter->setFont(initFontFromInfo(QApplication::font(), m_uiElements->menuInformation.disabledFont));
                        options  = m_uiElements->menuInformation.disabledFont.opts;
                        iconMode = QIcon::Mode::Disabled;
                    }

                    if (!item) { }
                }
                break;
            case QStyleOptionMenuItem::Scroller: C_FALLTHROUGH;
            case QStyleOptionMenuItem::TearOff: C_FALLTHROUGH;
            case QStyleOptionMenuItem::Margin: C_FALLTHROUGH;
            case QStyleOptionMenuItem::EmptyArea: break;
        }

        if (state & QStyle::State_Selected) {
            painter->fillRect(option->menuRect, m_uiElements->menuInformation.selectedBrush);
        }

        switch (option->checkType) {
            case QStyleOptionMenuItem::NotCheckable: iconState = QIcon::State::On; break;
            case QStyleOptionMenuItem::Exclusive: C_FALLTHROUGH;
            case QStyleOptionMenuItem::NonExclusive:
                if (option->checked) {
                    iconState = QIcon::State::On;
                }
                else {
                    iconState = QIcon::State::Off;
                }

                {
                    QRect checkArrowRect = itemRect;
                    checkArrowRect.setWidth(m_themeConstants->menuItemImage.width());
                    if (state & QStyle::State_Enabled) {
                        painter->drawPixmap(checkArrowRect, m_menuCheck.pixmap(m_themeConstants->menuItemImage));
                    }
                    else {
                        painter->drawPixmap(checkArrowRect, m_menuCheckGray.pixmap(m_themeConstants->menuItemImage));
                    }
                }

                itemRect.setLeft(itemRect.left() + m_themeConstants->menuItemImage.width());

                break;
        }

        if (!option->icon.isNull()) {
            int size       = 0;
            QRect iconRect = itemRect;
            if (option->maxIconWidth > itemRect.height()) {
                const int diff = iconRect.width() - iconRect.height();

                iconRect.setLeft(iconRect.left() + (diff / 2));
                iconRect.setWidth(iconRect.height());

                size = iconRect.height();
            }
            else {
                const int diff = iconRect.height() - option->maxIconWidth;

                iconRect.setTop(iconRect.top() + (diff / 2));
                iconRect.setWidth(option->maxIconWidth);
                iconRect.setHeight(option->maxIconWidth);

                size = option->maxIconWidth;
            }

            painter->drawPixmap(iconRect, option->icon.pixmap(QSize(size, size), iconMode, iconState));

            itemRect.setLeft(itemRect.left() + option->maxIconWidth);
        }

        if (!option->text.isEmpty())
            painter->drawText(itemRect, static_cast<int>(options.alignment()) | Qt::TextShowMnemonic, option->text);
    }
    painter->restore();
}

void CentTheme::drawProgressBarContents(const QStyleOptionProgressBar *option, QPainter *painter, const QProgressBar *widget) const
{
    auto &pbi = getProgressBarInformation(widget);

    const QPen pen { Qt::NoPen };

    const QRect rect = option->rect.marginsRemoved(pbi.fi.margins);

    const qreal pos = static_cast<qreal>(option->progress) / static_cast<qreal>(option->maximum - option->minimum);

    QRect barRect = rect;

    if (widget->orientation() == Qt::Horizontal) {
        const auto width = static_cast<qreal>(rect.width());
        if (!option->invertedAppearance) {
            barRect.setWidth(static_cast<int>(width * pos));
        }
        else {
            barRect.setLeft(barRect.left() + static_cast<int>(width * (1 - pos)));
        }
    }
    else {
        const auto height = static_cast<qreal>(rect.height());
        if (!option->invertedAppearance) {
            barRect.setTop(barRect.top() + static_cast<int>(height * (1 - pos)));
        }
        else {
            barRect.setHeight(static_cast<int>(height * pos));
        }
    }

    painter->save();
    {
        painter->setPen(pen);
        if (!(option->state & QStyle::State_Enabled)) {
            auto *effect = widget->graphicsEffect();
            if (effect != nullptr)
                effect->setEnabled(false);

            painter->setBrush(pbi.disableBarBrush);

            CentTheme::drawFrame(painter, pbi.fi, rect);

            painter->setBrush(pbi.disableGrooveBrush);
            CentTheme::drawFrame(painter, pbi.fi, barRect);
        }
        else {
            auto *effect = widget->graphicsEffect();
            if (effect != nullptr && !effect->isEnabled())
                effect->setEnabled(true);

            painter->setBrush(pbi.enabledBarBrush);
            CentTheme::drawFrame(painter, pbi.fi, rect);

            painter->setBrush(pbi.enabledGrooveBrush);
            CentTheme::drawFrame(painter, pbi.fi, barRect);
        }
    }
    painter->restore();
}

void CentTheme::drawHeaderView(const QStyleOptionHeader *option, QPainter *painter, const QHeaderView *widget) const
{
    if (widget == nullptr)
        return;

    const auto state              = option->state.toInt();
    const auto &headerInformation = getHeaderInformation(widget, widget->orientation());
    painter->fillRect(option->rect, headerInformation.backgroundBrush);
    if (state & QStyle::State_Enabled) {
        painter->fillRect(option->rect, headerInformation.backgroundBrush);

        if (state & QStyle::State_MouseOver)
            painter->fillRect(option->rect, headerInformation.hoverBrush);

        if (state & QStyle::State_Sunken)
            painter->fillRect(option->rect, headerInformation.sunkenBrush);
    }
    else
        painter->fillRect(option->rect, headerInformation.disableBackgroundBrush);

    drawControl(CE_HeaderSection, option, painter, widget);

    QStyleOptionHeader opt = *option;
    opt.rect               = subElementRect(SE_HeaderLabel, option, widget);
    if (!option->text.isEmpty())
        drawControl(CE_HeaderLabel, &opt, painter, widget);

    if (opt.sortIndicator != QStyleOptionHeader::None) {
        opt.rect = subElementRect(SE_HeaderArrow, option, widget);
        drawPrimitive(PE_IndicatorHeaderArrow, &opt, painter, widget);
    }
}

void CentTheme::drawHeaderSection(const QStyleOptionHeader *option, QPainter *painter, const QHeaderView *widget) const
{
    if (widget == nullptr) {
        return;
    }

    const auto &headerInformation = getHeaderInformation(widget, widget->orientation());
    if (!headerInformation.showSectionLines) {
        return;
    }

    painter->save();
    {
        // Turning the antialiasing off, draws a single pixel line
        painter->setRenderHints(QPainter::RenderHint::Antialiasing, false);

        if (widget->isEnabled()) {
            painter->setPen(headerInformation.sectionLinesPen);
        }
        else {
            painter->setPen(headerInformation.disableSectionLinesPen);
        }

        const QRect rect = option->rect.marginsRemoved(headerInformation.sectionLinesMargins);
        if (widget->orientation() == Qt::Orientation::Horizontal) {
            painter->drawLine(rect.topRight(), rect.bottomRight());
        }
        else {
            painter->drawLine(rect.bottomLeft(), rect.bottomRight());
        }
    }
    painter->restore();
}

void CentTheme::drawHeaderLabel(const QStyleOptionHeader *option, QPainter *painter, const QHeaderView *widget) const
{
    if (widget == nullptr) {
        return;
    }

    const auto state              = option->state.toInt();
    const auto &headerInformation = getHeaderInformation(widget, widget->orientation());

    painter->save();
    {
        const QTextOption *textOpt = nullptr;
        if (!widget->isEnabled()) {
            painter->setPen(headerInformation.disableFontPen);
            painter->setFont(initFontFromInfo(QApplication::font(), headerInformation.disableFont));
            textOpt = &headerInformation.disableFont.opts;
        }
        else {
            bool opt = false;
            if (state & QStyle::State_MouseOver) {
                opt = true;
                painter->setPen(headerInformation.hoverPen);
                painter->setFont(initFontFromInfo(QApplication::font(), headerInformation.hoverFont));
                textOpt = &headerInformation.hoverFont.opts;
            }

            if (state & QStyle::State_Sunken) {
                opt = true;
                painter->setPen(headerInformation.sunkenPen);
                painter->setFont(initFontFromInfo(QApplication::font(), headerInformation.sunkenFont));
                textOpt = &headerInformation.sunkenFont.opts;
            }

            if (!opt) {
                painter->setPen(headerInformation.fontPen);
                painter->setFont(initFontFromInfo(QApplication::font(), headerInformation.font));
                textOpt = &headerInformation.font.opts;
            }
        }

        const QFontMetrics metrics = painter->fontMetrics();
        const auto elidedText      = metrics.elidedText(option->text, Qt::ElideRight, option->rect.width(), 0);
        painter->drawText(option->rect, elidedText, *textOpt);
    }
    painter->restore();
}

void CentTheme::drawHeaderEmptyArea(const QStyleOption *option, QPainter *painter, const QHeaderView *widget) const
{
    if (widget == nullptr) {
        return;
    }

    const auto &headerInformation = getHeaderInformation(widget, widget->orientation());

    const auto state = option->state.toInt();
    if (state & QStyle::State_Enabled) {
        painter->fillRect(option->rect, headerInformation.emptyAreaBrush);
        return;
    }

    painter->fillRect(option->rect, headerInformation.disableEmptyAreaBrush);
}

void CentTheme::drawTableFrameBackground(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    auto &tvi = getTableViewInformation(widget);
    painter->fillRect(option->rect, tvi.paneBackgroundBrush);
}

void CentTheme::drawTableView(const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget) const
{
    const auto state    = option->state.toInt();
    const auto features = option->features.toInt();

    auto &tvi = getTableViewInformation(widget);

    const QRect iconRect = subElementRect(SE_ItemViewItemDecoration, option, widget);
    const QRect textRect = subElementRect(SE_ItemViewItemText, option, widget).marginsRemoved(tvi.itemMargins);

    drawPrimitive(PE_PanelItemViewItem, option, painter, widget);

    if (features & QStyleOptionViewItem::HasCheckIndicator) {
        QStyleOptionViewItem opt = *option;

        opt.rect  = subElementRect(SE_ItemViewItemCheckIndicator, option, widget);
        opt.state = opt.state & ~QStyle::State_HasFocus;

        switch (option->checkState) {
            case Qt::Unchecked: opt.state |= QStyle::State_Off; break;
            case Qt::PartiallyChecked: opt.state |= QStyle::State_NoChange; break;
            case Qt::Checked: opt.state |= QStyle::State_On; break;
        }
        drawPrimitive(QStyle::PE_IndicatorItemViewItemCheck, &opt, painter, widget);
    }

    QIcon::Mode mode = QIcon::Normal;
    if (!(state & QStyle::State_Enabled)) {
        mode = QIcon::Disabled;
    }
    else if (state & QStyle::State_Selected) {
        mode = QIcon::Selected;
    }

    const QIcon::State iconState = option->state & QStyle::State_Open ? QIcon::On : QIcon::Off;
    option->icon.paint(painter, iconRect, option->decorationAlignment, mode, iconState);

    painter->save();
    {
        const QTextOption *textOpt = nullptr;
        if (!(option->state & QStyle::State_Enabled)) {
            painter->setPen(tvi.disabledPen);
            painter->setFont(initFontFromInfo(QApplication::font(), tvi.disabledFont));
            textOpt = &tvi.disabledFont.opts;
        }
        else if (state & QStyle::State_HasFocus && state & QStyle::State_Selected) {
            painter->setPen(tvi.focusPen);
            painter->setFont(initFontFromInfo(QApplication::font(), tvi.focusFont));
            textOpt = &tvi.focusFont.opts;
        }
        else if (!(state & QStyle::State_HasFocus) && (state & QStyle::State_Selected) && !(state & QStyle::State_MouseOver)) {
            painter->setPen(tvi.selectedPen);
            painter->setFont(initFontFromInfo(QApplication::font(), tvi.selectedFont));
            textOpt = &tvi.selectedFont.opts;
        }
        else if (state & QStyle::State_MouseOver) {
            painter->setPen(tvi.hoverPen);
            painter->setFont(initFontFromInfo(QApplication::font(), tvi.hoverFont));
            textOpt = &tvi.hoverFont.opts;
        }
        else {
            painter->setPen(tvi.fontPen);
            painter->setFont(initFontFromInfo(QApplication::font(), tvi.fontFont));
            textOpt = &tvi.fontFont.opts;
        }

        const QFontMetrics metrics = painter->fontMetrics();
        const auto elidedText      = metrics.elidedText(option->text, Qt::ElideRight, textRect.width(), 0);
        painter->drawText(textRect, elidedText, *textOpt);
    }
    painter->restore();
}

void CentTheme::drawTableViewItemRow(const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget) const
{
    const auto state    = option->state.toInt();
    const auto features = option->features.toInt();

    if ((state & QStyle::State_Enabled) == 0) {
        // Use the default background brush
        return;
    }

    auto &tvi = getTableViewInformation(widget);

    if (state & QStyle::State_HasFocus) {
        painter->fillRect(option->rect, tvi.itemFocusBrush);
        return;
    }

    // Draw the row background
    if (!(option->state & QStyle::State_MouseOver)) {

        if (state & QStyle::State_Selected) {
            painter->fillRect(option->rect, tvi.itemSelectedBrush);
            return;
        }

        if (features & QStyleOptionViewItem::Alternate) {
            painter->fillRect(option->rect, tvi.itemAltBackgroundBrush);
        }
        else {
            painter->fillRect(option->rect, tvi.itemBackgroundBrush);
        }
    }
    else {
        painter->fillRect(option->rect, tvi.itemHoverBrush);
    }
}

void CentTheme::drawTableViewItem(const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget) const
{
    drawTableViewItemRow(option, painter, widget);
}

void CentTheme::drawCheckBox(const QStyleOptionButton *option, QPainter *painter, const QCheckBox *widget) const
{
    // const auto state               = option->state.toInt();
    auto &cbi                      = getCheckBoxInformation(widget);
    auto &checkBoxInformationState = getCheckBoxInformationState(option, &cbi);

    painter->setBrush(checkBoxInformationState.widgetBrush);
    painter->setPen(checkBoxInformationState.widgetPen);
    drawFrame(painter, checkBoxInformationState.widgetFrame, option->rect);

    QStyleOptionButton boxOpt = *option;

    boxOpt.rect = subElementRect(SE_CheckBoxIndicator, option, widget);
    drawCheckBoxIndicator(&boxOpt, painter, widget, true);

    QStyleOptionButton contentsOpts = *option;

    contentsOpts.rect = subElementRect(SE_CheckBoxContents, option, widget);
    drawCheckBoxLabel(&contentsOpts, painter, widget);
}

void CentTheme::drawCheckBoxLabel(const QStyleOptionButton *option, QPainter *painter, const QCheckBox *widget) const
{
    const auto state = option->state.toInt();
    auto &cbi        = getCheckBoxInformation(widget);

    auto &checkBoxInformationState = getCheckBoxInformationState(option, &cbi);

    if (state & QStyle::State_On) {
        painter->setPen(checkBoxInformationState.checkedFontPen);
        painter->setFont(initFontFromInfo(QApplication::font(), checkBoxInformationState.checkedFont));
        painter->drawText(option->rect, option->text, checkBoxInformationState.checkedFont.opts);
    }
    else if (state & QStyle::State_Off) {
        painter->setPen(checkBoxInformationState.uncheckedFontPen);
        painter->setFont(initFontFromInfo(QApplication::font(), checkBoxInformationState.uncheckedFont));
        painter->drawText(option->rect, option->text, checkBoxInformationState.uncheckedFont.opts);
    }
    else if (state & QStyle::State_NoChange) {
        painter->setPen(checkBoxInformationState.undefinedFontPen);
        painter->setFont(initFontFromInfo(QApplication::font(), checkBoxInformationState.undefinedFont));
        painter->drawText(option->rect, option->text, checkBoxInformationState.undefinedFont.opts);
    }
}

void CentTheme::drawCheckBoxIndicator(const QStyleOption *option, QPainter *painter, const QWidget *widget, bool withAnimations) const
{
    static constexpr QMargins checkMargins { 3, 2, 2, 2 };

    const auto state = option->state.toInt();
    auto &cbi        = getCheckBoxInformation(widget);

    auto &checkBoxInformationState = getCheckBoxInformationState(option, &cbi);

    QRect rect = option->rect;

    // Make it square relative to the minimum value between width or height
    if (rect.width() > rect.height()) {
        if ((rect.height() % 2) != 0) {
            rect.setHeight(rect.height() - 1);
        }
        rect.setWidth(rect.height());
    }
    else {
        if ((rect.width() % 2) != 0) {
            rect.setWidth(rect.width() - 1);
        }
        rect.setHeight(rect.width());
    }

    if (withAnimations && state & QStyle::State_Enabled && widget != nullptr) {

        if (state & QStyle::State_On) {
            const auto hoverBackgroundPropertyAnimation = widget->property(_hover_c_checkbox_bg_property);
            if (hoverBackgroundPropertyAnimation.isValid())
                checkBoxInformationState.checkedBoxBrush.setColor(hoverBackgroundPropertyAnimation.value<QColor>());

            const auto hoverBorderPropertyAnimation = widget->property(_hover_c_checkbox_border_property);
            if (hoverBorderPropertyAnimation.isValid())
                checkBoxInformationState.checkedBoxPen.setColor(hoverBorderPropertyAnimation.value<QColor>());
        }
        else if (state & QStyle::State_NoChange) {
            const auto hoverBackgroundPropertyAnimation = widget->property(_hover_c_checkbox_bg_property);
            if (hoverBackgroundPropertyAnimation.isValid())
                checkBoxInformationState.undefinedBoxBrush.setColor(hoverBackgroundPropertyAnimation.value<QColor>());

            const auto hoverBorderPropertyAnimation = widget->property(_hover_c_checkbox_border_property);
            if (hoverBorderPropertyAnimation.isValid())
                checkBoxInformationState.undefinedBoxPen.setColor(hoverBorderPropertyAnimation.value<QColor>());
        }
        else if (state & QStyle::State_Off) {

            const auto hoverBackgroundPropertyAnimation = widget->property(_hover_u_checkbox_bg_property);
            if (hoverBackgroundPropertyAnimation.isValid())
                checkBoxInformationState.uncheckedBoxBrush.setColor(hoverBackgroundPropertyAnimation.value<QColor>());

            const auto hoverBorderPropertyAnimation = widget->property(_hover_u_checkbox_border_property);
            if (hoverBorderPropertyAnimation.isValid())
                checkBoxInformationState.uncheckedBoxPen.setColor(hoverBorderPropertyAnimation.value<QColor>());
        }
    }

    rect = rect.marginsRemoved(checkBoxInformationState.widgetFrame.margins);

    painter->save();
    {
        if (state & QStyle::State_Off) {

            rect = rect.marginsRemoved(checkBoxInformationState.uncheckedBoxFrame.margins);
            painter->setBrush(checkBoxInformationState.uncheckedBoxBrush);
            painter->setPen(checkBoxInformationState.uncheckedBoxPen);
            drawFrame(painter, checkBoxInformationState.uncheckedBoxFrame, rect);
        }
        else if (state & QStyle::State_On) {
            rect = rect.marginsRemoved(checkBoxInformationState.checkedBoxFrame.margins);
            painter->setBrush(checkBoxInformationState.checkedBoxBrush);
            painter->setPen(checkBoxInformationState.checkedBoxPen);
            drawFrame(painter, checkBoxInformationState.checkedBoxFrame, rect);

            rect = rect.marginsRemoved(checkMargins);

            painter->setPen(checkBoxInformationState.checkedBoxIndicatorPen);

            static constexpr signed Y_N_FACTOR = 33;
            static constexpr signed Y_D_FACTOR = 50;

            painter->drawLine(rect.left(), rect.top() + ((rect.height() * Y_N_FACTOR) / Y_D_FACTOR), rect.left() + rect.width() / 2,
                rect.bottom() - 1);

            painter->drawLine(rect.left() + rect.width() / 2, rect.bottom() - 1, rect.right(), rect.top() + (rect.height() / 4) - 1);
        }
        else if (state & QStyle::State_NoChange) {
            rect = rect.marginsRemoved(checkBoxInformationState.undefinedBoxFrame.margins);
            painter->setBrush(checkBoxInformationState.undefinedBoxBrush);
            painter->setPen(checkBoxInformationState.undefinedBoxPen);
            drawFrame(painter, checkBoxInformationState.undefinedBoxFrame, rect);

            rect = rect.marginsRemoved(checkMargins);

            painter->setPen(checkBoxInformationState.undefinedBoxIndicatorPen);
            painter->drawLine(rect.left(), rect.top() + (rect.height() / 2), rect.right(), rect.top() + (rect.height() / 2));
        }
    }
    painter->restore();
}

void CentTheme::drawToolButton(const QStyleOptionToolButton *option, QPainter *painter, const QWidget *widget) const
{
    auto &tbi = getToolButtonInformation(widget);

    bool validSunken = false;
    auto state       = [&]() -> CENTAUR_THEME_INTERFACE_NAMESPACE::ElementState       &{
        return getElementState(option, std::addressof(tbi), &validSunken);
    }();

    QRect buttonRect             = option->rect;
    const QRect menuRect         = subControlRect(CC_ToolButton, option, SC_ToolButtonMenu, widget).marginsRemoved(state.fi.margins);
    const QRect buttonRectActive = subControlRect(CC_ToolButton, option, SC_ToolButton, widget).marginsRemoved(state.fi.margins);

    CentTheme::drawFrameAnimation(painter, widget, state, buttonRect, validSunken);

    auto labelOption = *option;
    labelOption.rect = buttonRectActive;
    drawControl(CE_ToolButtonLabel, &labelOption, painter, widget);

    if (option->features.testFlag(QStyleOptionToolButton::ToolButtonFeature::MenuButtonPopup)) {

        const auto drawPixmap = [&](const QPixmap &pixmap, const QRect &rect) {
            const qreal scale   = pixmap.devicePixelRatio();
            const QRect aligned = alignedRect(
                QGuiApplication::layoutDirection(),
                QFlag(Qt::AlignCenter),
                pixmap.size() / scale,
                rect);
            const QRect inter = aligned.intersected(aligned);
            painter->drawPixmap(
                inter.x(),
                inter.y(),
                pixmap,
                inter.x() - aligned.x(),
                inter.y() - aligned.y(),
                qRound(inter.width() * scale),
                qRound(inter.height() * scale));
        };

        const auto btnState = option->state.toInt();

        static constexpr int _adjusted_margin = -5;

        QPixmap pixmap;
        QRect pixmapRect = menuRect;
        pixmapRect.setTop(menuRect.bottom() - menuRect.width());
        pixmapRect.adjust(_adjusted_margin, 0, 0, 0);
        QSize pmSize = pixmapRect.size();
        if (!m_toolButtonDownArrow.isNull()) {
            const QIcon::State iconState = btnState & State_On ? QIcon::On : QIcon::Off;
            const QIcon::Mode mode       = [&btnState]() -> QIcon::Mode {
                if (!(btnState & State_Enabled)) {
                    return QIcon::Disabled;
                }
                if (btnState & State_MouseOver && btnState & State_AutoRaise) {
                    return QIcon::Active;
                }
                return QIcon::Normal;
            }();

            pixmap = m_toolButtonDownArrow.pixmap(
                menuRect.size().boundedTo(pmSize),
                mode,
                iconState);

            pmSize = pixmap.size() / pixmap.devicePixelRatio();

            drawPixmap(pixmap,
                QStyle::visualRect(option->direction, menuRect, pixmapRect));
        }
    }
}

void CentTheme::drawToolButtonLabel(const QStyleOptionToolButton *option, QPainter *painter, const QWidget *widget) const
{
    auto &tbi = getToolButtonInformation(widget);

    auto elementState = [&]() -> CENTAUR_THEME_INTERFACE_NAMESPACE::ElementState & {
        return getElementState(option, std::addressof(tbi));
    }();

    const auto features = option->features.toInt();
    const auto state    = option->state.toInt();

    const auto drawArrow = [&]() {
        const QStyle::PrimitiveElement primitiveElement = [&type = option->arrowType]() {
            switch (type) {
                case Qt::NoArrow: C_FALLTHROUGH;
                case Qt::UpArrow: return QStyle::PE_IndicatorArrowUp;
                case Qt::DownArrow: return QStyle::PE_IndicatorArrowDown;
                case Qt::LeftArrow: return QStyle::PE_IndicatorArrowLeft;
                case Qt::RightArrow: return QStyle::PE_IndicatorArrowRight;
            }
        }();
        const QStyleOption arrowOption = *qstyleoption_cast<const QStyleOption *>(option);
        drawPrimitive(primitiveElement, &arrowOption, painter, widget);
    };

    const auto drawPixmap = [&](const QPixmap &pixmap, const QRect &rect) {
        const qreal scale   = pixmap.devicePixelRatio();
        const QRect aligned = alignedRect(
            QGuiApplication::layoutDirection(),
            QFlag(Qt::AlignCenter),
            pixmap.size() / scale,
            rect);
        const QRect inter = aligned.intersected(aligned);
        painter->drawPixmap(
            inter.x(),
            inter.y(),
            pixmap,
            inter.x() - aligned.x(),
            inter.y() - aligned.y(),
            qRound(inter.width() * scale),
            qRound(inter.height() * scale));
    };

    if (((!(features & QStyleOptionToolButton::Arrow) && option->icon.isNull())
            && !option->text.isEmpty())
        || option->toolButtonStyle == Qt::ToolButtonTextOnly) {

        painter->setPen(QPen(Qt::black));
        painter->drawText(option->rect, Qt::AlignCenter, option->text);
    }
    else {
        QPixmap pixmap;
        QSize pmSize = option->iconSize;
        if (!option->icon.isNull()) {
            const QIcon::State iconState = state & State_On ? QIcon::On : QIcon::Off;

            const QIcon::Mode mode = [&state]() -> QIcon::Mode {
                if (!(state & State_Enabled)) {
                    return QIcon::Disabled;
                }
                if (state & State_MouseOver && state & State_AutoRaise) {
                    return QIcon::Active;
                }
                return QIcon::Normal;
            }();

            pixmap = option->icon.pixmap(
                option->rect.size().boundedTo(option->iconSize),
                mode,
                iconState);

            pmSize = pixmap.size() / pixmap.devicePixelRatio();
        }

        static constexpr int _hc_hh = 4; // From Qt source code
        if (option->toolButtonStyle != Qt::ToolButtonIconOnly) {
            QRect pixmapRect = option->rect;
            QRect textRect   = pixmapRect;
            if (option->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
                pixmapRect.setHeight(pmSize.height() + _hc_hh);
                textRect.adjust(0, pixmapRect.height() - 1, 0, 0);
                if (features & QStyleOptionToolButton::Arrow) {
                    drawArrow();
                }
                else {
                    drawPixmap(pixmap,
                        QStyle::visualRect(option->direction, option->rect, pixmapRect));
                }
            }
            else {
                pixmapRect.setWidth(pmSize.width() + _hc_hh);
                textRect.adjust(pixmapRect.width(), 0, 0, 0);
                if (features & QStyleOptionToolButton::Arrow) {
                    drawArrow();
                }
                else {
                    drawPixmap(pixmap,
                        QStyle::visualRect(option->direction, option->rect, pixmapRect));
                }
            }

            painter->save();
            {

                painter->setPen(elementState.fontPen);
                painter->setFont(initFontFromInfo(QApplication::font(), elementState.fontInformation));
                const QFontMetrics fontMetrics = painter->fontMetrics();

                const QString elideText = fontMetrics.elidedText(option->text,
                    Qt::TextElideMode::ElideRight,
                    textRect.width(),
                    0);
                painter->drawText(textRect, elideText, elementState.fontInformation.opts);
            }
            painter->restore();
        }
        else {
            if (features & QStyleOptionToolButton::Arrow) {
                drawArrow();
            }
            else {
                drawPixmap(pixmap, option->rect);
            }
        }
    }
}

void CentTheme::drawGroupBox(const QStyleOptionGroupBox *option, QPainter *painter, const QWidget *widget) const
{
    auto &gbi = getGroupBoxInformation(widget);

    // Under this style the flat feature will be drawn exactly the same as non-flat group boxes
    const QRect labelRect    = subControlRect(CC_GroupBox, option, SC_GroupBoxLabel, widget);
    const QRect checkBoxRect = subControlRect(CC_GroupBox, option, SC_GroupBoxCheckBox, widget);
    const QRect frameRect    = subControlRect(CC_GroupBox, option, SC_GroupBoxFrame, widget);

    QRect labelFrameRect = option->rect;
    labelFrameRect.setHeight(frameRect.top());

    QRect indicatorFrame = labelFrameRect;
    indicatorFrame.setRight(gbi.indicatorWidth);

    painter->save();
    {
        painter->setPen(Qt::NoPen);

        painter->setBrush(gbi.contentsBrush);
        drawFrame(painter, gbi.fi, frameRect);

        painter->setBrush(gbi.headerBrush);
        drawFrame(painter, gbi.fi, labelFrameRect);

        const auto state = option->state.toInt();
        if (state & State_Enabled) {
            painter->fillRect(indicatorFrame, gbi.indicatorBrush);
            painter->setPen(gbi.pen);
            painter->setFont(initFontFromInfo(QApplication::font(), gbi.font));
        }
        else {
            painter->fillRect(indicatorFrame, gbi.disabledIndicatorBrush);
            painter->setPen(gbi.disabledPen);
            painter->setFont(initFontFromInfo(QApplication::font(), gbi.disabledFont));
        }

        const QFontMetrics fontMetrics = painter->fontMetrics();
        const QString elideText        = fontMetrics.elidedText(option->text,
                   Qt::TextElideMode::ElideRight,
                   labelRect.width(),
                   0);

        painter->drawText(
            labelRect,
            elideText,
            state & State_Enabled
                ? gbi.font.opts
                : gbi.disabledFont.opts);

        if (option->subControls.testFlag(SC_GroupBoxCheckBox)) {
            QStyleOptionButton checkBox;
            checkBox.QStyleOption::operator=(*option);
            checkBox.rect = checkBoxRect;
            drawPrimitive(PE_IndicatorCheckBox, &checkBox, painter, widget);
        }
    }
    painter->restore();
    // painter->fillRect(contentsRect, Qt::blue);
}
