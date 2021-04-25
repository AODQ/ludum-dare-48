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
    buttons.emplace("Research", ld::Button(x, 150, btnWidth, btnHeight, 5, ::Fade(::LIGHTGRAY, 0.5f)));
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

void ld::Overlay::ResearchMenu(ld::GameState & game)
{
    // Position of the root panel
    uint32_t w = 500;
    uint32_t h = 300;
    uint32_t x = (scrWidth - w) * 0.5f;
    uint32_t y = (scrHeight - h) * 0.5f;

    // Root Menu panel
    DrawRectangle(x, y, w, h, ::Fade(::DARKGRAY, 0.7f));
    ld::DrawCenteredText("Research", x+w*0.5f, y, 20, ::BLACK);

    // Upgrade Buttons
    uint32_t btnWidth = 75;
    uint32_t btnHeight = 75;
    const uint32_t numButtons = 4;
    uint32_t padding = w / numButtons;
    uint32_t xOffset = x + (padding-btnWidth)*0.5f; // Increment with padding as we add more elements

    std::vector<ld::Button> upgBtns;
    for (size_t i = 0; i < numButtons; ++i)
    {
        upgBtns.push_back(ld::Button(xOffset, y + 100, btnWidth, btnHeight, 15, ::WHITE));
        xOffset+=padding;
    }

    std::vector<std::string> descriptions =
    {
        "Increase pickaxe power and durability",
        "Increase armor defense and durability",
        "Increase food to energy ratio",
        "Increase vision distance in fog of war",
    };

    // TODO replace with texture icons
    std::vector<std::string> icon =
    {
        "Pickaxe",
        "Armor",
        "Food",
        "Vision",
    };

    std::string desc = "Hover over each button to learn more";
    for (size_t i = 0; i < numButtons; ++i)
    {
        upgBtns[i].Draw(icon[i].c_str());
        if (upgBtns[i].IsHovered())
        {
            desc = descriptions[i];
        }
        if (upgBtns[i].IsClicked())
        {
            // Decrement gold
            // Increase level
        }
    }
    // Description of upgrade
    ld::DrawCenteredText(desc.c_str(), x+w*0.5f, y + 200, 20, ::BLACK);

    // -- Close
    ld::Button closeBtn(x + 0.5f*(w-btnWidth), y+h-30, 50, 30, 15, ::WHITE);
    closeBtn.Draw("Close");
    if (closeBtn.IsClicked())
    {
        menuState = MenuState::None;
    }
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
    auto researchBtn = buttons.at("Research");
    buyMinerBtn.Draw("Hire Miner");
    researchBtn.Draw("Research");

}

// Update any gamestate changes from button usage
void ld::Overlay::Update(ld::GameState & game)
{
    auto buyMinerBtn = buttons.at("BuyMiner");
    if (buyMinerBtn.IsClicked())
    {
        if (game.gold >= static_cast<int32_t>(game.minerCost))
        {
            game.gold -= game.minerCost;
            game.minerGroup.addMiner();
        }
    }

    auto researchBtn = buttons.at("Research");
    if (researchBtn.IsClicked())
    {
        menuState = MenuState::Research;
    }

}

void ld::Overlay::MinerInfo(ld::Miner & miner)
{
    // Position of the root panel
    uint32_t w = 120;
    uint32_t h = 200;
    uint32_t x = scrWidth - w - 20;
    uint32_t y = scrHeight - h - 20;

    // Root Menu panel
    DrawRectangle(x, y, w, h, ::Fade(::DARKGRAY, 0.8f));

    // Increment as we go to get padding between the last element
    uint32_t padding = 0;
    { // -- Energy
        const char* energyText = ::TextFormat("Energy: %i/%i", miner.energy, miner.maxEnergy);
        ld::DrawBar(energyText, x, y+padding, 100, 20, 10, ::GREEN, static_cast<float>(miner.energy) / static_cast<float>(miner.maxEnergy));
    }

    { // -- Weight
        padding += 20;
        const char* cargo = ::TextFormat("Weight: %i/%i", miner.currentCargoCapacity, miner.cargoCapacity);
        ld::DrawBar(cargo, x, y+padding, 100, 20, 10, ::BLUE, static_cast<float>(miner.currentCargoCapacity) / static_cast<float>(miner.cargoCapacity));
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
                action += "Traversing";
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
        padding += 20;
        ::DrawText(action.c_str(), x, y+padding, 10, ::BLACK);
    }

    { // -- Equipment
        padding+=20;
        ::DrawText("Equipment:", x, y+padding, 20, ::BLACK);

        uint32_t xOffset = 0;
        padding+=20;
        for (auto equip : miner.inventory) {
            // TODO display texture and darken if not owned
            ::Color color = ::DARKGRAY;
            if (equip.owns) {
                color = ::WHITE;
            }

            ld::Button itemBtn(x+xOffset, y+padding, 30, 30, 10, color);
            // TODO replace with texture
            itemBtn.Draw("TODO");
            xOffset += 30;
        }
    }

    { // -- Cargo
        padding+=35;
        ::DrawText("Cargo:", x, y+padding, 20, ::BLACK);
        uint32_t xOffset = 0;
        uint32_t cargoValue = 0;

        padding+=20;
        for (size_t it = 0u; it < miner.cargo.size(); ++ it) {
            // TODO display texture
            ld::Button itemBtn(x+xOffset, y+padding, 30, 30, 10, ::WHITE);
            // Display count of item
            itemBtn.Draw(std::to_string(miner.cargo[it].ownedUnits).c_str());
            xOffset += 30;
            cargoValue += miner.cargo[it].ownedUnits* ld::valuableInfoLookup[it].value;
        }
        std::string valueText = "Total Value: " + std::to_string(cargoValue);
        padding += 30;
        ::DrawText(valueText.c_str(), x, y+padding, 10, ::BLACK);
    }

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
            break;
            // TODO Reenable upon submission
            //TitleScreen();
            //return; // return to Avoid rendering resources
        case ld::Overlay::MenuState::Research:
            ResearchMenu(game);
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

    if (game.minerSelection >= 0)
    {
        bool found = false;
        // find miner & check if still alive
        for (auto & miner : game.minerGroup.miners) {
          if (miner.minerId == game.minerSelection) {
            found = true;
            MinerInfo(miner);
            break;
          }
        }

        if (!found) {
          game.minerSelection = 0;
        }
    }

    ResourceMenu(game);
}
