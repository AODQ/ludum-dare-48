#pragma once

#include <button.hpp>
#include <gamestate.hpp>
#include <miner.hpp>

namespace ld {

    void DrawCenteredText(const char* text, uint32_t xPos, uint32_t yPos, uint32_t fontSize, ::Color color);
    void DrawBar(const char* text, uint32_t xPos, uint32_t yPos, uint32_t width, uint32_t height, uint32_t fontSize, ::Color color);

    struct Overlay
    {
        enum class MenuState
        {
            Title = 0,
            Pause = 1,
            Blueprint,
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
        void BlueprintsMenu(ld::GameState & game);
        void ResourceMenu(const ld::GameState & game);

        uint32_t scrWidth, scrHeight;
        ButtonGroup buttons;
    };
}
