#pragma once

#include <raylib.h>
#include <string>
#include <map>

namespace ld
{
    struct Button
    {
        Button(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
            : xPos(x), yPos(y), width(w), height(h)
        {
            bounds = {
                static_cast<float>(x),
                static_cast<float>(y),
                static_cast<float>(w),
                static_cast<float>(h)
            };
        }

        void DrawTexture(const char* text, ::Texture2D texture, uint32_t spriteCol, uint32_t spriteRow, ::Color tint = ::WHITE, bool isInteractable = false);
        void Draw(const char* text, uint32_t fontSize = 10, ::Color color = ::WHITE);
        bool IsHovered();
        bool IsPressedDown();
        bool IsClicked();

        uint32_t xPos, yPos;
        uint32_t width, height;
        ::Rectangle bounds;
    };

    using ButtonGroup = std::map<std::string, ld::Button>;
}

