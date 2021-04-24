#pragma once

#include <raylib.h>
#include <string>
#include <array>

struct Item {
    std::string name;
    uint32_t value;
};

struct Miner {
    uint32_t xPosition = 0, yPosition = 0;
    uint32_t width = 10, height = 10;
    uint32_t energy = 100;
    std::array<Item, 9> inventory;

    ::Rectangle GetBounds() {
        ::Rectangle bounds = {
            static_cast<float>(xPosition),
            static_cast<float>(yPosition),
            static_cast<float>(width),
            static_cast<float>(height),
        };
        return bounds;
    }
};
