#pragma once

#include <raylib.h>
#include <string>
#include <map>

namespace ld
{
    struct Button
    {
        Button(uint32_t x, uint32_t y, uint32_t w, uint32_t h, ::Color c = ::WHITE)
            : xPos(x), yPos(y), width(w), height(h), color(c)
        {
            bounds = {
                static_cast<float>(x),
                static_cast<float>(y),
                static_cast<float>(w),
                static_cast<float>(h)
            };
        }

        void Draw(const char* text, uint32_t fontSize = 10);
        bool IsHovered();
        bool IsPressedDown();
        bool IsClicked();

        uint32_t xPos, yPos;
        uint32_t width, height;
        ::Color color = BLACK;
        ::Rectangle bounds;
    };

    using ButtonGroup = std::map<std::string, ld::Button>;
}

