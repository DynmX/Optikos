#ifndef LABEL_H
#define LABEL_H

#include "ui/Widget.hpp"
#include "ui/text/TextFont.hpp"

uint32_t constexpr DEFAULT_LABEL_WIDTH  = 100;
uint32_t constexpr DEFAULT_LABEL_HEIGHT = 100;

namespace Optikos
{
class Label : public Widget
{
   public:
    Label(std::string text, Vec2 position, Color color = {255, 255, 255, 255});
    Label(std::string text, Vec2 position, uint32_t width, uint32_t height,
          Color color = {255, 255, 255, 255});

    void                             render(IRenderQueue& renderQueue) override;
    void                             updateData() override;
    const std::vector<Vertex>&       getVertices() const override;
    const std::vector<unsigned int>& getIndices() const override;
    void                             handleEvent() override;
    bool                             handleClick(double x, double y, int action) override;
    void                             handleHover(double x, double y) override;
    void                             resetHover() override;
    bool                             wantsHoverEvents() const override;
    void                             resize(int width, int height) override;
    void                             setPosition(Vec2 pos) override;

    void setText(const std::string& text);
    void setFont(const std::string& font);
    void setColor(const Color& color);

   private:
    RenderData  m_data;
    std::string m_text;
    std::string m_fontName = DEFAULT0_FONT;
    bool        m_dirty    = true;
};

}  // namespace Optikos

#endif /* LABEL_H */