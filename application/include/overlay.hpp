#pragma once

#include <button.hpp>
#include <gamestate.hpp>
#include <miner.hpp>
#include <stack>

namespace ld {

    /* Draw a colored text with black outlines */
    void DrawOutlinedText(const char* text, uint32_t xPos, uint32_t yPos, uint32_t fontSize, ::Color mainColor, ::Color outlineColor);
    void DrawOutlinedCenteredText(const char* text, uint32_t xPos, uint32_t yPos, uint32_t fontSize, ::Color color, ::Color outline = ::BLACK);
    void DrawCenteredText(const char* text, uint32_t xPos, uint32_t yPos, uint32_t fontSize, ::Color color);
    void DrawBar(const char* text, uint32_t xPos, uint32_t yPos, uint32_t width, uint32_t height, uint32_t fontSize, ::Color color, float fillPct = 1.0f);

    struct Overlay
    {
        enum class MenuState
        {
            None = 0,
            Title,
            Pause,
            Research,
            GameOver,
            Instructions,
        } menuState;

        std::stack<MenuState> menuStack;

        Overlay(uint32_t w, uint32_t h)
            : scrWidth(w), scrHeight(h)
        {
            InitButtons();
        }

        void Update(ld::GameState & game);
        void Draw(ld::GameState & game);
        void MinerInfo(ld::Miner & miner);
        void TitleScreen();
        void Instructions();
        void InitButtons();
        void PauseScreen(ld::GameState & game);
        void GameOverScreen(ld::GameState & game);
        void ResearchMenu(ld::GameState & game);
        void ResourceMenu(ld::GameState & game);

        uint32_t scrWidth, scrHeight;
        ButtonGroup buttons;

        // current gold/food that interpolates to game state's gold
        int32_t currentGold = 0;
        int32_t currentFood = 0;
    };
}
