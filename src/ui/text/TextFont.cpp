#include "ui/text/TextFont.hpp"

#include <tracy/Tracy.hpp>

namespace Optikos
{
TextFont::TextFont()
{
    if (FT_Init_FreeType(&m_library))
    {
        LOG_DEBUG("inside [FT_Init_FreeType] error occured", "log");
        return;
    }
}

TextFont::~TextFont()
{
    for (auto& [name, atlas] : m_Atlases)
    {
        if (atlas.face)
        {
            FT_Done_Face(atlas.face);
            atlas.face = nullptr;
        }
    }

    m_Atlases.clear();
    FT_Done_FreeType(m_library);
}

void TextFont::loadFont(std::string_view fontPath, std::string fontName, float fontSize)
{
    auto [it, inserted] = m_Atlases.try_emplace(fontName);

    if (!inserted)
    {
        LOG_DEBUG("Font '" + fontName + "' already loaded", "log");
        return;
    }

    Atlas& atlas   = it->second;
    atlas.fontSize = fontSize;

    std::string pathStr(fontPath);
    if (FT_New_Face(m_library, pathStr.c_str(), 0, &atlas.face))
    {
        LOG_DEBUG("Failed to load font: " + pathStr, "log");
        m_Atlases.erase(it);
        return;
    }

    generateAtlas(fontName, atlas.fontSize);
}

void TextFont::generateAtlas(std::string fontName, float fontSize)
{
    auto it = m_Atlases.find(fontName);
    if (it == m_Atlases.end())
    {
        LOG_DEBUG("Font '" + fontName + "' not created", "log");
        return;
    }

    auto& atlas = it->second;
    if (!atlas.face)
    {
        LOG_DEBUG("Face not initialized in generateAtlas", "log");
        return;
    }

    FT_Set_Pixel_Sizes(atlas.face, 0, static_cast<FT_UInt>(fontSize));

    atlas.fontSize  = fontSize;
    atlas.atlasSize = CalculateAtlasSize(fontSize);

    atlas.atlasData.assign(atlas.atlasSize * atlas.atlasSize, 0);

    int xpos         = 0;
    int ypos         = 0;
    int maxRowHeight = 0;

    for (unsigned char c = DEFAULT_CHAR_START; c <= DEFAULT_CHAR_END; c++)
    {
        if (FT_Load_Char(atlas.face, c, FT_LOAD_RENDER))
        {
            continue;
        }

        auto& bitmap = atlas.face->glyph->bitmap;

        if (xpos + (int) bitmap.width >= static_cast<int>(atlas.atlasSize))
        {
            xpos = 0;
            ypos += maxRowHeight + 1;
            maxRowHeight = 0;
        }

        if (ypos + (int) bitmap.rows >= static_cast<int>(atlas.atlasSize))
        {
            LOG_ERROR("Font atlas is full! Increase atlas size or decrease font size.", "log");
            break;
        }

        for (int row = 0; row < (int) bitmap.rows; ++row)
        {
            for (int col = 0; col < (int) bitmap.width; ++col)
            {
                int atlasIdx              = (ypos + row) * atlas.atlasSize + (xpos + col);
                int glyphIdx              = row * bitmap.pitch + col;
                atlas.atlasData[atlasIdx] = bitmap.buffer[glyphIdx];
            }
        }

        Character character{xpos,
                            ypos,
                            (int) bitmap.width,
                            (int) bitmap.rows,
                            atlas.face->glyph->bitmap_left,
                            atlas.face->glyph->bitmap_top,
                            (int) (atlas.face->glyph->advance.x >> 6)};

        atlas.characters[c] = character;

        if ((int) bitmap.rows > maxRowHeight) maxRowHeight = (int) bitmap.rows;
        xpos += bitmap.width + 1;
    }
    LOG_TRACE(
        "New font atlas created with name: " + fontName + " and size: " + std::to_string(fontSize),
        "log");
}

RenderData TextFont::generateTextQuads(const std::string& text, const Vec2& position,
                                       const uint32_t& width, const uint32_t& height,
                                       const Clip& clip, const std::string& fontName,
                                       const Color& textColor)
{
    ZoneScopedN("TextFont::generateTextQuads");

    RenderData data;
    data.vertices.reserve(text.size() * 4);
    data.indices.reserve(text.size() * 6);

    auto it = m_Atlases.find(fontName);
    if (it == m_Atlases.end())
    {
        LOG_DEBUG("Font '" + fontName + "' not found. Call loadFont() first.", "log");
        return data;
    }

    Atlas& atlas = it->second;

    float textLength = 0;
    float maxAscent  = 0;
    float maxDecent  = 0;
    for (const auto& symbol : text)
    {
        auto charIt = atlas.characters.find(symbol);
        if (charIt != atlas.characters.end())
        {
            const Character& ch = charIt->second;
            textLength += ch.advance;

            maxAscent = std::max(maxAscent, (float) ch.bearing_y);
            maxDecent = std::max(maxDecent, (float) ch.height - (float) ch.bearing_y);
        }
    }

    float textHeight = maxAscent + maxDecent;
    float xpos       = position.x + (width - textLength) / 2.0f;
    float yBaseline  = position.y + (height - textHeight) / 2.0f + maxAscent;

    unsigned int offset = 0;

    for (const auto& symbol : text)
    {
        auto charIt = atlas.characters.find(symbol);
        if (charIt == atlas.characters.end())
        {
            LOG_DEBUG("Character '" + std::string(1, symbol) + "' not found in atlas", "log");
            continue;
        }

        const Character& ch = charIt->second;

        float x = xpos + ch.bearing_x;
        float y = yBaseline - ch.bearing_y;
        float w = static_cast<float>(ch.width);
        float h = static_cast<float>(ch.height);

        float u1 = static_cast<float>(ch.atlasX) / atlas.atlasSize;
        float v1 = static_cast<float>(ch.atlasY) / atlas.atlasSize;
        float u2 = u1 + static_cast<float>(ch.width) / atlas.atlasSize;
        float v2 = v1 + static_cast<float>(ch.height) / atlas.atlasSize;

        // ZoneScopedN("insert");
        // TODO: main problem of label overhead when too much insert calls.

        data.vertices.emplace_back(x, y, textColor.r, textColor.g, textColor.b, textColor.a, u1, v1,
                                   clip.xMin, clip.xMax, clip.yMin, clip.yMax);
        data.vertices.emplace_back(x + w, y, textColor.r, textColor.g, textColor.b, textColor.a, u2,
                                   v1, clip.xMin, clip.xMax, clip.yMin, clip.yMax);
        data.vertices.emplace_back(x, y + h, textColor.r, textColor.g, textColor.b, textColor.a, u1,
                                   v2, clip.xMin, clip.xMax, clip.yMin, clip.yMax);
        data.vertices.emplace_back(x + w, y + h, textColor.r, textColor.g, textColor.b, textColor.a,
                                   u2, v2, clip.xMin, clip.xMax, clip.yMin, clip.yMax);

        data.indices.insert(data.indices.end(), {offset + 0, offset + 1, offset + 2, offset + 1,
                                                 offset + 3, offset + 2});

        offset += 4;

        xpos += ch.advance;
    }

    return data;
}

// TODO: add trace debug
unsigned int TextFont::getAtlasSize(std::string fontName) const
{
    auto it = m_Atlases.find(fontName);
    return (it != m_Atlases.end()) ? it->second.atlasSize : 0;
}

Vec2 TextFont::getSizeText(const std::string& text, std::string fontName)
{
    auto it = m_Atlases.find(fontName);
    if (it == m_Atlases.end())
    {
        LOG_DEBUG("Font '" + fontName + "' not found. Call loadFont() first.", "log");
        return {-1, -1};  // Not found -1
    }

    Atlas& atlas      = it->second;
    float  textLength = 0;
    float  maxAscent  = 0;
    float  maxDecent  = 0;
    for (const auto& symbol : text)
    {
        auto charIt = atlas.characters.find(symbol);
        if (charIt != atlas.characters.end())
        {
            const Character& ch = charIt->second;
            textLength += ch.advance;

            maxAscent = std::max(maxAscent, (float) ch.bearing_y);
            maxDecent = std::max(maxDecent, (float) ch.height - (float) ch.bearing_y);
        }
    }

    return {textLength, maxAscent + maxDecent};
}

int TextFont::getPosText(double startText, const std::string& text, std::string fontName)
{
    if (startText <= 0) return 0;

    auto it = m_Atlases.find(fontName);
    if (it == m_Atlases.end())
    {
        LOG_DEBUG("Font '" + fontName + "' not found. Call loadFont() first.", "log");
        return -1;  // Not found -1
    }

    Atlas& atlas       = it->second;
    float  textLength  = 0;
    int    newPosition = 0;
    for (const auto& symbol : text)
    {
        auto charIt = atlas.characters.find(symbol);
        if (charIt != atlas.characters.end())
        {
            const Character& ch = charIt->second;
            textLength += ch.advance;

            if (startText <= textLength - ch.advance / 2) return newPosition;
            newPosition++;
        }
    }
    return newPosition;
}

unsigned int TextFont::getAtlasTextureId(std::string fontName) const
{
    auto it = m_Atlases.find(fontName);
    return (it != m_Atlases.end()) ? it->second.atlasTextureId : 0;
}

void TextFont::setAtlasTextureId(unsigned int id, std::string fontName)
{
    auto it = m_Atlases.find(fontName);
    if (it != m_Atlases.end())
    {
        it->second.atlasTextureId = id;
    }
}

const std::vector<unsigned char>& TextFont::getAtlasData(std::string fontName) const
{
    static const std::vector<unsigned char> empty;

    auto it = m_Atlases.find(fontName);
    return (it != m_Atlases.end()) ? it->second.atlasData : empty;
}

unsigned int TextFont::CalculateAtlasSize(float fontSize)
{
    /* Calculate texture atlas size for font rendering.
       Example: 313 pixels → 512 pixels
          312 in binary:  100111000
          After filling:  111111111 (= 511)
          Add 1:          512
    */
    unsigned int pixels =
        static_cast<int>(sqrt(fontSize * fontSize * (DEFAULT_CHAR_END - DEFAULT_CHAR_START)));
    unsigned int atlasSize = pixels - 1;
    atlasSize |= atlasSize >> 1;
    atlasSize |= atlasSize >> 2;
    atlasSize |= atlasSize >> 4;
    atlasSize |= atlasSize >> 8;
    atlasSize |= atlasSize >> 16;
    return atlasSize + 1;
}

}  // namespace Optikos