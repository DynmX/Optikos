#include "ui/sdk/label.hpp"

#include <tracy/Tracy.hpp>

namespace Optikos
{
Label::Label(std::string text, Vec2 position, Color color)
    : Widget(DEFAULT_LABEL_WIDTH, DEFAULT_LABEL_HEIGHT, position, true, color)
{
    m_text = text;

    Vec2 size = TextFont::getInstance().getSizeText(m_text, m_fontName);
    m_width   = static_cast<uint32_t>(size.x);
    m_height  = static_cast<uint32_t>(size.y);

    updateData();
}

Label::Label(std::string text, Vec2 position, uint32_t width, uint32_t height, Color color)
    : Widget(width, height, position, true, color)
{
    m_text = text;

    Vec2 size = TextFont::getInstance().getSizeText(m_text, m_fontName);

    updateData();
}

void Label::render(IRenderQueue& renderQueue)
{
    ZoneScopedN("Label::render");

    DrawCommand cmd;
    cmd.vertices    = getVertices();
    cmd.indices     = getIndices();
    cmd.shaderId    = 0;
    cmd.textureId   = TextFont::getInstance().getAtlasTextureId(m_fontName);
    cmd.textureMode = TEXTURE_FONT;
    renderQueue.submit(std::move(cmd));
}

void Label::updateData()
{
    ZoneScopedN("Label::updateData");

    if (!m_dirty) return;

    m_data = TextFont::getInstance().generateTextQuads(m_text, m_position, m_width, m_height,
                                                       m_clip, m_fontName, m_color);

    m_dirty = false;
}

const std::vector<Vertex>& Label::getVertices() const
{
    return m_data.vertices;
}

const std::vector<unsigned int>& Label::getIndices() const
{
    return m_data.indices;
}

void Label::setPosition(Vec2 pos)
{
    m_position = pos;
    m_dirty    = true;
    updateData();
}

void Label::handleEvent()
{
    return; /* stub */
}
bool Label::handleClick(double x, double y, int action)
{
    (void) action;
    (void) x;
    (void) y;
    return false; /* stub */
}

void Label::handleHover(double x, double y)
{
    (void) x;
    (void) y;
    return; /* stub */
}
void Label::resetHover()
{
    return; /* stub */
}
bool Label::wantsHoverEvents() const
{
    return false; /* stub */
}
void Label::resize(int width, int height)
{
    m_width  = width;
    m_height = height;
    m_dirty  = true;
    updateData();
}

void Label::setText(const std::string& text)
{
    ZoneScopedN("Label::setText");

    m_text  = text;
    m_dirty = true;
    // updateData();
}

void Label::setFont(const std::string& font)
{
    m_fontName = font;
    m_dirty    = true;
    updateData();
}

void Label::setColor(const Color& color)
{
    m_color = color;
    m_dirty = true;
    updateData();
}

}  // namespace Optikos
