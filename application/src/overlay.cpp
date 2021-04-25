#include <overlay.hpp>

typedef enum {
    eStorageScore = 0,
    eStorageHiScore = 1,
} StorageData;

void ld::DrawCenteredText(const char* text, uint32_t xPos, uint32_t yPos, uint32_t fontSize, ::Color color)
{
    int textWidth = ::MeasureText(text, fontSize);
    ::DrawText(
        text,
        xPos - 0.5f*textWidth,
        yPos + 0.5*fontSize,
        fontSize,
        color
    );
}

void ld::DrawBar(const char* text, uint32_t xPos, uint32_t yPos, uint32_t width, uint32_t height, uint32_t fontSize, ::Color color, float fillPct)
{
    int textWidth = ::MeasureText(text, fontSize);
    ::DrawRectangle     (xPos, yPos, width*fillPct, height, color);
    ::DrawRectangleLines(xPos, yPos, width        , height, DARKGRAY);
    ::DrawText(text, xPos + 0.5f*(width-textWidth), yPos + 0.5*(height-fontSize), fontSize, BLACK);
}


void ld::Overlay::InitButtons()
{
    uint32_t x = scrWidth - 100;
    uint32_t btnWidth = 70;
    uint32_t btnHeight = 50;

    buttons.emplace("BuyMiner", ld::Button(x, 100, btnWidth, btnHeight, 5, ::Fade(::LIGHTGRAY, 0.5f)));
    buttons.emplace("BluePrints", ld::Button(x, 150, btnWidth, btnHeight, 5, ::Fade(::LIGHTGRAY, 0.5f)));
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

void ld::Overlay::BlueprintsMenu(ld::GameState & /*game*/)
{

}

void ld::Overlay::TitleScreen()
{
    uint32_t xPos = scrWidth / 2.0f;
    uint32_t yPos = 100;
    uint32_t width = 50;
    DrawCenteredText("Game Title", xPos, yPos, 100, BLACK);

    ld::Button playBtn(xPos - 0.5*width, yPos + 250, 100, 50, 35);
    playBtn.Draw("PLAY");
    if (playBtn.IsClicked())
    {
        menuState = MenuState::None;
    }
}

void ld::Overlay::ResourceMenu(const ld::GameState & game)
{
    currentGold -= ld::sgn(currentGold - game.gold);

    // Gold supply bar
    {
        uint32_t xPos = 50;
        uint32_t yPos = scrHeight - 50;
        uint32_t width = 200;
        uint32_t height = 30;
        uint32_t fontSize = 20;

        const char* text = ::TextFormat("Gold: %i", game.gold);
        ld::DrawBar(text, xPos, yPos, width, height, fontSize, ::GOLD);
    }

    currentFood -= ld::sgn(currentFood - game.food);
    // Food supply bar
    {
        uint32_t xPos = 275;
        uint32_t yPos = scrHeight - 50;
        uint32_t width = 200;
        uint32_t height = 30;
        uint32_t fontSize = 20;

        const char* text = ::TextFormat("Food: %i/%i", game.food, game.maxFood);
        ld::DrawBar(text, xPos, yPos, width, height, fontSize, ::RED, static_cast<float>(game.food)/static_cast<float>(game.maxFood));
    }

    // Resource related buttons
    auto buyMinerBtn = buttons.at("BuyMiner");
    auto bluePrintBtn = buttons.at("BluePrints");
    buyMinerBtn.Draw("Buy Miner");
    bluePrintBtn.Draw("Buy Blueprint");

}

// Update any gamestate changes from button usage
void ld::Overlay::Update(ld::GameState & game)
{
    auto buyMinerBtn = buttons.at("BuyMiner");
    if (buyMinerBtn.IsClicked())
    {
        game.gold -= game.minerCost;
        game.minerGroup.addMiner();
    }

    auto bluePrintBtn = buttons.at("BluePrints");
    if (bluePrintBtn.IsClicked())
    {
        menuState = MenuState::Blueprint;
    }

}

void ld::Overlay::MinerInfo(ld::Miner & miner)
{
    // Position of the root panel
    uint32_t w = 300;
    uint32_t h = 200;
    uint32_t padding = 20;
    uint32_t x = scrWidth - w - padding;
    uint32_t y = scrHeight - h - padding;

    // Root Menu panel
    DrawRectangle(x, y, w, h, ::Fade(::DARKGRAY, 0.5f));

    { // -- Energy
        const char* energyText = ::TextFormat("Energy: %i/%i", miner.energy, miner.maxEnergy);
        ld::DrawBar(energyText, x, y+20, 100, 20, 10, ::GREEN, static_cast<float>(miner.energy) / static_cast<float>(miner.maxEnergy));
    }

    { // -- Current action
        std::string action = "Action: ";
        switch (miner.aiState)
        {
            case Miner::AiState::Attacking:
                action += "Attacking";
                break;
            case Miner::AiState::Dying:
                action += "Dying";
                break;
            case Miner::AiState::Idling:
                action += "Idling";
                break;
            case Miner::AiState::MineTraversing:
                action += "MineTraversing";
                break;
            case Miner::AiState::Mining:
                action += "Mining";
                break;
            case Miner::AiState::Surfaced:
                action += "Surfaced";
                break;
            case Miner::AiState::Traversing:
                action += "Traversing";
                break;
            default:
                action = "";
                break;
        }
        ::DrawText(action.c_str(), x, y+50, 15, ::BLACK);
    }

    // Inventory

    // Cargo

    // Gui sliders
    //::GuiSliderBar((::Rectangle) { } 
}

void ld::Overlay::Draw(ld::GameState & game)
{
    // if game paused, then that should override every other menu
    //menuState = game.isPaused ? MenuState::Pause : menuState;

    switch (menuState)
    {
        case ld::Overlay::MenuState::Title:
            // TODO Reenable upon submission
            //TitleScreen();
            //return; // return to Avoid rendering resources
        case ld::Overlay::MenuState::Blueprint:
            BlueprintsMenu(game);
            break;
        case ld::Overlay::MenuState::GameOver:
            GameOverScreen();
            break;
        case ld::Overlay::MenuState::Pause:
            PauseScreen();
        case ld::Overlay::MenuState::None:
        default:
            break;
    }

    if (game.isPaused)
    {
        PauseScreen();
    }

    if (game.selection >= 0)
    {
        MinerInfo(game.minerGroup.miners.at(game.selection));
    }

    ResourceMenu(game);
}
