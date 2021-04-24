#pragma once

#include <button.hpp>
#include <gamestate.hpp>

namespace ld {

    struct Overlay
    {
        enum class MenuState
        {
            Title,
            Pause,
            GameOver,
            Blueprint,
            None,
        } menuState;

        Overlay(uint32_t w, uint32_t h)
            : scrWidth(w), scrHeight(h)
        {
            InitButtons();
        }

        void Update(ld::GameState & game);
        void Draw(const ld::GameState & game);

        void InitButtons();
        void PauseScreen();
        void GameOverScreen();
        void BlueprintsMenu(ld::GameState & game);
        void ResourceMenu(const ld::GameState & game);

        uint32_t scrWidth, scrHeight;
        ButtonGroup buttons;

        // current gold/food that interpolates to game state's gold
        int32_t currentGold = 0;
        int32_t currentFood = 0;
    };
}
