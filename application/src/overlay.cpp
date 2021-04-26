#include <overlay.hpp>
#include <renderer.hpp> // Texture info

typedef enum {
    eStorageScore = 0,
    eStorageHiScore = 1,
} StorageData;

void ld::DrawOutlinedText(const char* text, uint32_t xPos, uint32_t yPos, uint32_t fontSize, ::Color mainColor, ::Color outlineColor)
{
    int offset = 1;
    // Place a slightly offset text on each quadrant
    ::DrawText(text, xPos-offset, yPos-offset, fontSize, outlineColor);
    ::DrawText(text, xPos-offset, yPos+offset, fontSize, outlineColor);
    ::DrawText(text, xPos+offset, yPos-offset, fontSize, outlineColor);
    ::DrawText(text, xPos+offset, yPos+offset, fontSize, outlineColor);

    ::DrawText(text, xPos, yPos, fontSize, mainColor);
}

void ld::DrawOutlinedCenteredText(const char* text, uint32_t xPos, uint32_t yPos, uint32_t fontSize, ::Color color, ::Color outline)
{
    int textWidth = ::MeasureText(text, fontSize);
    ld::DrawOutlinedText(text, xPos - 0.5f*textWidth, yPos + 0.5*fontSize, fontSize, color, outline);
}

void ld::DrawCenteredText(const char* text, uint32_t xPos, uint32_t yPos, uint32_t fontSize, ::Color color)
{
    int textWidth = ::MeasureText(text, fontSize);
    ::DrawText(text, xPos - 0.5f*textWidth, yPos + 0.5*fontSize, fontSize, color);
}

void ld::DrawBar(const char* text, uint32_t xPos, uint32_t yPos, uint32_t width, uint32_t height, uint32_t fontSize, ::Color color, float fillPct)
{
    int textWidth = ::MeasureText(text, fontSize);
    ::DrawRectangle     (xPos, yPos, width*fillPct, height, color);
    ::DrawRectangleLines(xPos, yPos, width        , height, DARKGRAY);
    //::DrawText(text, xPos + 0.5f*(width-textWidth), yPos + 0.5*(height-fontSize), fontSize, BLACK);
    ld::DrawOutlinedText(text, xPos + 0.5f*(width-textWidth), yPos + 0.5*(height-fontSize), fontSize, ::WHITE, ::BLACK);
}


void ld::Overlay::InitButtons()
{
    uint32_t x = scrWidth - 100;
    uint32_t btnWidth = 70;
    uint32_t btnHeight = 50;

    buttons.emplace("BuyMiner", ld::Button(x, 100, btnWidth, btnHeight, ::Fade(::LIGHTGRAY, 0.5f)));
    buttons.emplace("Research", ld::Button(x, 150, btnWidth, btnHeight, ::Fade(::LIGHTGRAY, 0.5f)));
    buttons.emplace("Idle"    , ld::Button(x, 200, btnWidth, btnHeight, ::Fade(::LIGHTGRAY, 0.5f)));
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
    ::DrawRectangle(0, 0, scrWidth, scrHeight, ::Fade(::DARKGRAY, 0.9f));
    ld::DrawCenteredText("Game Over!", scrWidth*0.5f, scrHeight*0.5f, 30, ::WHITE);
}

void ld::Overlay::ResearchMenu(ld::GameState & game)
{
    // Position of the root panel
    uint32_t w = 500;
    uint32_t h = 300;
    uint32_t x = (scrWidth - w) * 0.5f;
    uint32_t y = (scrHeight - h) * 0.5f;

    // Root Menu panel
    {
        uint32_t size = 30;
        DrawRectangle(x, y, w, h, ::Fade(::DARKGRAY, 0.7f));
        ld::DrawOutlinedCenteredText("Research", x+w*0.5f, y-size, size, ::WHITE, ::BLACK);
    }

    { // -- Upgrade Buttons
        uint32_t btnWidth = 30;
        uint32_t btnHeight = 30;
        const uint32_t numButtons = Idx(ld::ResearchType::Size);
        uint32_t padding = h / numButtons;
        uint32_t offset = (padding-btnHeight)*0.5f + 15;

        std::vector<ld::Button> upgBtns;
        for (size_t i = 0; i < numButtons; ++i)
        {
            upgBtns.push_back(ld::Button(x + 5, y + offset, btnWidth, btnHeight, ::WHITE));
            offset += btnHeight + 2;
        }

        std::vector<::Color> researchColor =
        {
            ::RED,      // pickaxe
            ::DARKBLUE, // Armor
            ::GREEN,    // Food
            ::GOLD,     // Cargo
            ::VIOLET,   // Vision
            ::RAYWHITE, // Speed
        };

        std::string desc = "Hover over each button to learn more";
        for (size_t i = 0; i < numButtons; ++i)
        {
            int cost = ld::researchInfoLookup[i].cost + (game.researchItems[i].level * 25);
            if (upgBtns[i].IsHovered())
            {
                desc = ld::researchInfoLookup[i].desc;
            }
            if (upgBtns[i].IsClicked())
            {
                if (game.gold >= cost)
                {
                    game.gold -= cost;
                    game.researchItems[i].level++;
                }
            }

            upgBtns[i].DrawTexture("", ld::TextureGet(ld::TextureType::Upgrades), 0, i, ::WHITE, true);

            // Draw upgrade bars
            uint32_t maxUpgrades = 10;
            //uint32_t barWidth = (w-btnWidth-20) / maxUpgrades;
            uint32_t barWidth = 30;
            uint32_t barPosX = upgBtns[i].xPos+upgBtns[i].width + 10;
            uint32_t barPosY = upgBtns[i].yPos;
            ::DrawRectangle(barPosX, barPosY, barWidth*game.researchItems[i].level, upgBtns[i].height, Fade(researchColor[i], 0.5f));
            for (size_t j = 0; j < maxUpgrades; ++j)
            {
                ::DrawRectangleLines(barPosX, barPosY, barWidth, upgBtns[i].height, researchColor[i]);
                barPosX += barWidth;
            }
            const char* barText = ::TextFormat("%s Level:%i, Cost:%iG", game.researchItems[i].name.c_str(), game.researchItems[i].level, cost);
            ld::DrawOutlinedText(barText, barPosX + 10, barPosY + 0.5f*(upgBtns[i].height-10), 10, researchColor[i], ::BLACK);
        }

        // Description of upgrade
        ld::DrawOutlinedCenteredText(desc.c_str(), x+w*0.5f, y + 230, 20, ::WHITE, ::BLACK);
    }

    { // -- Close
        uint32_t btnWidth = 70;
        uint32_t btnHeight = 35;
        ld::Button closeBtn(x + 0.5f*(w-btnWidth), y+h-btnHeight*0.5f, 70, 35, ::WHITE);
        closeBtn.Draw("Close", 20);
        if (closeBtn.IsClicked())
        {
            menuState = MenuState::None;
        }
    }
}

void ld::Overlay::TitleScreen()
{
    uint32_t xPos = scrWidth / 2.0f;
    uint32_t yPos = 100;
    uint32_t width = 50;
    DrawCenteredText("Game Title", xPos, yPos, 100, BLACK);

    ld::Button playBtn(xPos - 0.5*width, yPos + 250, 100, 50);
    playBtn.Draw("PLAY", 35);
    if (playBtn.IsClicked())
    {
        menuState = MenuState::None;
    }
}

void ld::Overlay::ResourceMenu(ld::GameState & game)
{
    currentGold -= ld::sgn(currentGold - game.gold);

    // Gold supply bar
    {
        uint32_t xPos = 50;
        uint32_t yPos = scrHeight - 50;
        uint32_t width = 200;
        uint32_t height = 30;
        uint32_t fontSize = 20;

        float fillPct = game.gold < 100 ? game.gold / 100.0f : 1.0f;
        const char* text = ::TextFormat("Gold: %i", game.gold);
        ld::DrawBar(text, xPos, yPos, width, height, fontSize, ::GOLD, fillPct);
    }

    currentFood -= ld::sgn(currentFood - game.food);

    // Food supply bar
    {
        uint32_t xPos = 275;
        uint32_t yPos = scrHeight - 50;
        uint32_t width = 200;
        uint32_t height = 30;
        uint32_t fontSize = 20;

        const char* text = ::TextFormat("Food: %i/%i", game.food, game.MaxFood());

        ::DrawRectangle(
          xPos, yPos, width, height,
          Color{100, 40, 55, 255}
        );

        ::DrawRectangle(
          xPos, yPos - height/2 + 8,
          width * (static_cast<float>(game.foodEatTimer) / game.MaxFoodEatTimer()), 5,
          BLUE
        );

        ld::DrawBar(text, xPos, yPos, width, height, fontSize, ::RED, static_cast<float>(game.food)/static_cast<float>(game.MaxFood()));
    }

    // Resource related buttons
    auto buyMinerBtn = buttons.at("BuyMiner");
    auto researchBtn = buttons.at("Research");
    auto idleBtn     = buttons.at("Idle");
    const char* hireText = ::TextFormat("Hire: %i Gold", game.minerCost);
    buyMinerBtn.Draw(hireText);
    researchBtn.Draw("Research");

    std::vector<int32_t> idleMiners;
    for (auto miner : game.minerGroup.miners) {
        if (miner.aiState == ld::Miner::AiState::Idling) {
            idleMiners.push_back(miner.minerId);
        }
    }

    static size_t cycle = 0;
    std::string idleText = "Idle: " + std::to_string(idleMiners.size());
    idleBtn.Draw(idleText.c_str(), 10);
    if (
        (::IsKeyPressed(KEY_SPACE) || idleBtn.IsClicked())
        && !idleMiners.empty()
    ) {
        // wrap cycle first in case it corresponds to an invalid index within the list
        cycle = (cycle+1) % idleMiners.size();
        game.minerSelection = idleMiners.at(cycle);
    }
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
    uint32_t h = 260;
    uint32_t x = scrWidth - w - 20;
    uint32_t y = scrHeight - h - 20;

    // Root Menu panel
    ::DrawRectangle(x, y, w, h, ::Fade(::DARKGRAY, 0.7f));

    // Increment as we go to get padding between the last element
    uint32_t padding = 0;

    { // -- Panel title
        uint32_t titleSize = 20;
        ld::DrawOutlinedCenteredText(
            "Miner Info", x+0.5f*w, y-titleSize, titleSize, ::WHITE, ::BLACK
        );
    }

    { // -- Current action
        padding += 15;
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
        ld::DrawOutlinedText(action.c_str(), x, y+padding, 10, ::WHITE, ::BLACK);
    }

    { // -- Energy
        padding += 15;
        const char* energyText = ::TextFormat("Energy: %i/%i", miner.energy, miner.maxEnergy);
        ld::DrawBar(energyText, x, y+padding, w, 20, 10, ::GREEN, static_cast<float>(miner.energy) / static_cast<float>(miner.maxEnergy));
    }

    { // -- Weight
        padding += 20;
        const char* cargo = ::TextFormat("Weight: %i/%i", miner.currentCargoCapacity, miner.cargoCapacity);
        ld::DrawBar(cargo, x, y+padding, w, 20, 10, ::BLUE, static_cast<float>(miner.currentCargoCapacity) / static_cast<float>(miner.cargoCapacity));
    }

    { // -- Equipment
        padding+=20;
        ld::DrawOutlinedCenteredText("Equipment:", x+w*0.5f, y+padding, 10, ::WHITE);

        padding+=20;
        uint32_t btnSize = 30;
        float xPadding = w / 3.0f;
        uint32_t xOffset = x + 0.5f*(xPadding - btnSize);

        int i = 0;
        for (auto equip : miner.inventory) {
            ::Color tint = ::DARKGRAY;
            if (equip.owns) {
                tint = ::WHITE;
            }

            ld::Button itemBtn(xOffset, y+padding, btnSize, btnSize, tint);
            itemBtn.DrawTexture("", ld::TextureGet(ld::TextureType::Misc), i, 1, tint);
            xOffset += xPadding;

            i++;
        }
    }

    { // -- Cargo
        padding+=35;
        ld::DrawOutlinedCenteredText("Cargo:", x+w*0.5f, y+padding, 10, ::WHITE);
        uint32_t cargoValue = 0;

        uint32_t btnSize = 30;
        padding+=20;
        float xPadding = static_cast<float>(w) / 3.0f;

        for (uint32_t row = 0u; row < 2; ++row) {

            uint32_t xOffset = x + 0.5f*(xPadding - btnSize);

            for (uint32_t col = 0u; col < 3; ++col) {
                size_t it = row * 3 + col;

                ld::Button itemBtn(xOffset, y+padding, btnSize, btnSize, ::WHITE);
                // Display count of item
                auto itemCount = miner.cargo[it].ownedUnits;
                Color tint = itemCount > 0
                    ? ::WHITE
                    : ::Fade(::DARKGRAY, 0.5f)
                ;

                itemBtn.DrawTexture(
                    std::to_string(miner.cargo[it].ownedUnits).c_str(),
                    ld::TextureGet(ld::TextureType::Cargo), row, col, tint
                );

                xOffset += xPadding;
                cargoValue += miner.cargo[it].ownedUnits* ld::valuableInfoLookup[it].value;
            }

            padding += btnSize;
        }

        std::string valueText = "Cargo Value: " + std::to_string(cargoValue);
        ld::DrawOutlinedCenteredText(valueText.c_str(), x+w*0.5f, y+padding, 10, ::WHITE);
    }

    { // -- Cancel current action
        padding+=20;
        int btnWidth = 60;
        int btnHeight = 25;
        ld::Button btn(x + (w-btnWidth)*0.5f, y+padding, btnWidth, btnHeight, ::WHITE);
        btn.Draw("Set Idle", 7);
        if (btn.IsClicked())
        {
            miner.aiState = ld::Miner::AiState::Idling;
        }
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

    // Cursor should be drawn last
    if (game.showCursor)
    {
        ::DrawCircleV(GetMousePosition(), 2, RED);
    }
}
