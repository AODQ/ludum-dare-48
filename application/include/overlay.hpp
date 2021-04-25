#pragma once

#include <button.hpp>
#include <gamestate.hpp>
#include <miner.hpp>

namespace ld {

    void DrawCenteredText(const char* text, uint32_t xPos, uint32_t yPos, uint32_t fontSize, ::Color color);
    void DrawBar(const char* text, uint32_t xPos, uint32_t yPos, uint32_t width, uint32_t height, uint32_t fontSize, ::Color color, float fillPct = 1.0f);

    struct Overlay
    {
        enum class MenuState
        {
            Title = 0,
            Pause = 1,
            Research,
            GameOver,
            None,
        } menuState;

        Overlay(uint32_t w, uint32_t h)
            : scrWidth(w), scrHeight(h)
        {
            InitButtons();
        }

        void Update(ld::GameState & game);
        void Draw(ld::GameState & game);
        void MinerInfo(ld::Miner & miner);
        void TitleScreen();
        void InitButtons();
        void PauseScreen();
        void GameOverScreen();
        void ResearchMenu(ld::GameState & game);
        void ResourceMenu(const ld::GameState & game);

        uint32_t scrWidth, scrHeight;
        ButtonGroup buttons;

        // current gold/food that interpolates to game state's gold
        int32_t currentGold = 0;
        int32_t currentFood = 0;
    };
}
