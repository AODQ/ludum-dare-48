#include <overlay.hpp>

typedef enum {
    eStorageScore = 0,
    eStorageHiScore = 1,
} StorageData;

void ld::Overlay::InitButtons()
{
    uint32_t x = scrWidth - 100;
    uint32_t btnWidth = 70;
    uint32_t btnHeight = 50;

    buttons.emplace("BuyMiner", ld::Button(x, 100, btnWidth, btnHeight, 5));

    buttons.emplace("BluePrints", ld::Button(x, 150, btnWidth, btnHeight, 5));
}

void ld::Overlay::PauseScreen()
{
    int fontSize = 30;

    int score = ::LoadStorageValue(eStorageScore);
    int hiScore = ::LoadStorageValue(eStorageHiScore);

    const char* scoreText = ::TextFormat("Score: %i\t Hi-Score: %i", score, hiScore);
    int scoreWidth = ::MeasureText(scoreText, fontSize);
    ::DrawText(scoreText, 0.5f*(scrWidth-scoreWidth), 130, fontSize, BLACK);

    const char* pauseText = "PRESS [TAB] TO RESUME";
    int pauseWidth = ::MeasureText(pauseText, fontSize);
    ::DrawText(pauseText, 0.5f*(scrWidth-pauseWidth), 170, fontSize, GRAY);
}

void ld::Overlay::GameOverScreen()
{

}

void ld::Overlay::BlueprintsMenu(ld::GameState & game)
{
    // Update gamestate current available level of equipment
}

void ld::Overlay::ResourceMenu(const ld::GameState & game)
{
    // Gold supply bar
    {
        uint32_t xPos = 50;
        uint32_t yPos = scrHeight - 50;
        uint32_t width = 200;
        uint32_t height = 30;
        uint32_t fontSize = 20;

        const char* text = TextFormat("Gold: %i", game.gold);
        int textWidth = ::MeasureText(text, fontSize);
        ::DrawRectangle     (xPos, yPos, width, height, GOLD);
        ::DrawRectangleLines(xPos, yPos, width, height, DARKGRAY);
        ::DrawText(text, xPos + 0.5f*(width-textWidth), yPos + 0.5*(height-fontSize), fontSize, BLACK);
    }

    // Food supply bar
    {
        uint32_t xPos = 275;
        uint32_t yPos = scrHeight - 50;
        uint32_t width = 200;
        uint32_t height = 30;
        uint32_t fontSize = 20;

        const char* text = TextFormat("Food %i/%i", game.food, game.maxFood);
        int textWidth = ::MeasureText(text, fontSize);
        ::DrawRectangle     (xPos, yPos, width*((float)game.food/game.maxFood), height, RED);
        ::DrawRectangleLines(xPos, yPos, width, height, DARKGRAY);
        ::DrawText(text, xPos + 0.5f*(width-textWidth), yPos + 0.5*(height-fontSize), fontSize, BLACK);
    }

    // Resource related buttons
    auto buyMinerBtn = buttons.at("BuyMiner");
    auto bluePrintBtn = buttons.at("BluePrints");
    buyMinerBtn.Draw("Buy Miner");
    bluePrintBtn.Draw("Buy Blueprint");

    // Miner info
}

// Update any gamestate changes from button usage
void ld::Overlay::Update(ld::GameState & game)
{
    auto buyMinerBtn = buttons.at("BuyMiner");
    if (buyMinerBtn.IsClicked())
    {
        game.gold -= game.minerCost;
        game.minerGroup.miners.push_back({
            .xPosition = 300
        });
        game.minerGroup.surfacedMiners.push_back(
            game.minerGroup.miners.size() - 1
        );
    }

    auto bluePrintBtn = buttons.at("BluePrints");
    if (bluePrintBtn.IsClicked())
    {
        menuState = MenuState::Blueprint;
    }

}

void ld::Overlay::Draw(const ld::GameState & game)
{
    ResourceMenu(game);

    // TODO
    //switch (menuState)
    //{
    //    case MenuState::Blueprint:
    //        BlueprintsMenu(game);
    //        break;
    //}

    // Paused
    if (game.isPaused) {
        PauseScreen();
    }

    // Gameover
}
