#include "ui/UISystem.hpp"

#include <tracy/Tracy.hpp>

namespace Optikos
{
bool UISystem::rem_widget(const uint32_t idx)
{
    auto it = widgets.find(idx);
    if (it != widgets.end())
    {
        widgets.erase(idx);
        return true;
    }

    LOG_ERROR("[rem_widget] widget with idx: " + std::to_string(idx) + " not removed", "log");
    return false;
}

void UISystem::render(Optikos::IRenderQueue& renderQueue)
{
    ZoneScopedN("UISystem::render");

    for (const auto& [id, widget] : widgets)
    {
        widget->render(renderQueue);
    }
}

void UISystem::checkIfClicked(double x, double y, int action)
{
    for (const auto& [id, widget] : widgets)
    {
        widget->handleClick(x, y, action); /* stub we check all clicks so can be overhead*/

        if (widgets.find(id) ==
            widgets.end()) /* if widget delete him self (button widget for example) */
            break;
        //    if (widget->handleClick(x, y, action))
        //         return;
    }
}

void UISystem::checkIfHover(double x, double y)
{
    for (const auto& [id, widget] : widgets)
    {
        if (!widget->wantsHoverEvents()) continue;

        if (widget->isInside(x, y))
            widget->handleHover(x, y);
        else
            widget->resetHover();

        widget->handleDrag(x, y);
    }
}

bool UISystem::isInside(const IWidget& widget, double x, double y)
{
    Vec2 widgetPos = widget.getPosition();
    if (widgetPos.x <= x && x <= widgetPos.x + widget.getWidth() && widgetPos.y <= y &&
        y <= widgetPos.y + widget.getHeight())
        return true;

    return false;
}

void UISystem::expandWidgets(int width, int height)
{
    for (auto& [id, widget] : widgets)
    {
        switch (widget->isExpand())
        {
            case ExpandMode::None:
                break;
            case ExpandMode::Width:
                widget->resize(width, widget->getHeight());
                break;
            case ExpandMode::Height:
                widget->resize(widget->getWidth(), height);
                break;
            case ExpandMode::Both:
                widget->resize(width, height);
                break;
            default:
                LOG_WARN("[expandWidgets] Invalid expand mode: " +
                             std::to_string(static_cast<int>(widget->isExpand())),
                         "log");
                break;
        }
    }
}

void UISystem::passInput(unsigned int codepoint)
{
    for (const auto& [id, widget] : widgets)
    {
        if (!widget->wantsGetInput()) continue;

        widget->passInput(codepoint);
    }
}

}  // namespace Optikos