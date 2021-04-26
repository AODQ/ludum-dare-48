#include <overlay.hpp>

#include <renderer.hpp> // Texture info
#include <sounds.hpp>

#include <algorithm>

namespace {
}

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
    if (fillPct > 1.0f) fillPct = 1.0f;
    if (fillPct < 0.0f) fillPct = 0.0f;
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

    buttons.emplace("Panic",    ld::Button(x,  50, btnWidth, btnHeight));
    buttons.emplace("BuyMiner", ld::Button(x, 100, btnWidth, btnHeight));
    buttons.emplace("Research", ld::Button(x, 150, btnWidth, btnHeight));
    buttons.emplace("Idle"    , ld::Button(x, 200, btnWidth, btnHeight));
    buttons.emplace("Surface" , ld::Button(x, 250, btnWidth, btnHeight));
    buttons.emplace("Flag"    , ld::Button(x, 300, 32, 32));
    buttons.emplace("FlagCan" , ld::Button(x+32, 300, 32, 32));
}

void ld::Overlay::Instructions()
{
    // Position of the root panel
    uint32_t w = 500;
    uint32_t h = 300;
    uint32_t x = (scrWidth - w) * 0.5f;
    uint32_t y = (scrHeight - h) * 0.5f - 20.0f;
    uint32_t yOffset = y;

    { // Root Menu panel
        uint32_t size = 30;
        DrawRectangle(x, y, w, h, ::BROWN);
        ld::DrawOutlinedCenteredText("Instructions", x+w*0.5f, y-size, size, ::WHITE, ::BLACK);
    }

    { // Back
        uint32_t btnWidth = 55;
        uint32_t btnHeight = 25;
        yOffset += h - btnHeight;
        ld::Button btn(x + w - btnWidth, yOffset, btnWidth, btnHeight);
        btn.Draw("Back", 12);
        if (btn.IsClicked())
        {
            if (!menuStack.empty()) {
                menuState = menuStack.top();
                menuStack.pop();
            } else {
                menuState = MenuState::None;
            }
        }
    }
}

void ld::Overlay::PauseScreen(ld::GameState & game)
{
    uint32_t centerX = 0.5f*scrWidth;
    uint32_t startY = 100;
    uint32_t yOffset = startY;
    // Title
    ld::DrawOutlinedCenteredText("GAME TITLE", centerX, startY, 50, ::WHITE, ::BLACK);

    // Resume
    yOffset += 100;
    const char* pauseText = "PRESS [TAB] TO RESUME";
    ld::DrawOutlinedCenteredText(pauseText, centerX, yOffset, 30, ::WHITE, ::BLACK);

    { // Instructions
        yOffset += 100;
        uint32_t btnWidth = 100;
        uint32_t btnHeight = 50;
        ld::Button btn(centerX-btnWidth*0.5f, yOffset, btnWidth, btnHeight);
        btn.Draw("Instructions", 12);
        if (btn.IsClicked())
        {
            menuStack.push(menuState);
            menuState = MenuState::Instructions;
        }
    }

    { // Restart
        yOffset += 100;
        uint32_t btnWidth = 100;
        uint32_t btnHeight = 50;
        ld::Button btn(centerX-btnWidth*0.5f, yOffset, btnWidth, btnHeight);
        btn.Draw("Restart", 20);
        if (btn.IsClicked())
        {
            menuState = ld::Overlay::MenuState::None;
            game.Restart();
        }
    }

    { // audio
        yOffset += 100;
        uint32_t btnWidth = 100;
        uint32_t btnHeight = 50;
        {
          ld::Button btn(centerX-btnWidth, yOffset, btnWidth, btnHeight);
          btn.Draw("Mute Sfx", 20);
          if (btn.IsClicked())
          {
              ld::ToggleMuteSound();
          }
        }

        {
          ld::Button btn(centerX, yOffset, btnWidth, btnHeight);
          btn.Draw("Audio", 20);
          if (btn.IsClicked())
          {
              ld::ToggleMuteMedia();
          }
        }
    }

}

void ld::Overlay::GameOverScreen(ld::GameState & game)
{
    ::DrawRectangle(0, 0, scrWidth, scrHeight, ::Fade(::BLACK, 0.8f));

    ld::DrawOutlinedCenteredText("Game Over!", scrWidth*0.5f, scrHeight*0.5f-100, 50, ::RED, ::WHITE);

    ld::DrawOutlinedCenteredText("You ran out of food!", scrWidth*0.5f, scrHeight*0.5f, 30, ::WHITE, ::BLACK);

    uint32_t btnWidth = 100;
    uint32_t btnHeight = 50;
    ld::Button replayBtn((scrWidth-btnWidth)*0.5f, scrHeight*0.5f + 100, btnWidth, btnHeight);

    replayBtn.Draw("Replay", 20);
    if (replayBtn.IsClicked())
    {
        menuState = ld::Overlay::MenuState::None;
        game.Restart();
    }
}

void ld::Overlay::ResearchMenu(ld::GameState & game)
{
    // Position of the root panel
    uint32_t w = 520;
    uint32_t h = 375;
    uint32_t x = (scrWidth - w) * 0.5f;
    uint32_t y = (scrHeight - h) * 0.5f - 20.0f;

    // Root Menu panel
    {
        uint32_t size = 30;
        DrawRectangle(x, y, w, h, ::Fade(::DARKGRAY, 0.7f));
        ld::DrawOutlinedCenteredText("Upgrades", x+w*0.5f, y-size, size, ::WHITE, ::BLACK);
    }

    { // -- Upgrade Buttons
        uint32_t btnWidth = 30;
        uint32_t btnHeight = 30;
        const uint32_t numButtons = Idx(ld::ResearchType::Size);
        uint32_t padding = h / numButtons;
        uint32_t offset = (padding-btnHeight)*0.5f + 15;

        uint32_t fontSize = 20;
        ld::DrawOutlinedText("Equipment Upgrades", x + 5, y + offset, fontSize, ::WHITE, ::BLACK);
        offset += fontSize + 10;
        std::vector<ld::Button> upgBtns;
        for (size_t i = 0; i < 3; ++i)
        {
            upgBtns.push_back(ld::Button(x + 5, y + offset, btnWidth, btnHeight));
            offset += btnHeight + 2;
        }

        offset += 20;
        ld::DrawOutlinedText("Passive Upgrades", x + 5, y + offset, fontSize, ::WHITE, ::BLACK);

        offset += fontSize + 10;
        for (size_t i = 3; i < numButtons; ++i)
        {
            upgBtns.push_back(ld::Button(x + 5, y + offset, btnWidth, btnHeight));
            offset += btnHeight + 2;
        }

        std::vector<::Color> researchColor =
        {
            ::RED,      // pickaxe
            ::DARKBLUE, // Armor
            ::RAYWHITE, // Speed
            ::GREEN,    // Food
            ::SKYBLUE,  // Cargo
            ::PURPLE,   // Vision
        };

        std::string desc = "Hover over each button to learn more";
        for (size_t i = 0; i < numButtons; ++i)
        {
            int cost = ld::researchInfoLookup[i].cost + (game.researchItems[i].level * 25);
            if (upgBtns[i].IsHovered())
            {
                desc = ld::researchInfoLookup[i].desc;
            }
            if (
                   upgBtns[i].IsClicked()
                && game.researchItems[i].level < ld::researchInfoLookup[i].maxLevel
            ) {
                if (game.gold >= cost)
                {
                    game.gold -= cost;
                    game.researchItems[i].level++;
                }
            }

            upgBtns[i].DrawTexture("", ld::TextureGet(ld::TextureType::Upgrades), 0, i, ::WHITE, true);

            //uint32_t barWidth = (w-btnWidth-20) / maxUpgrades;
            uint32_t barWidth = 30;
            uint32_t barPosX = upgBtns[i].xPos+upgBtns[i].width + 10;
            uint32_t barPosY = upgBtns[i].yPos;
            uint32_t maxUpgrades = ld::researchInfoLookup[i].maxLevel;
            ::DrawRectangle(barPosX, barPosY, barWidth*game.researchItems[i].level, upgBtns[i].height, Fade(researchColor[i], 0.5f));
            for (size_t j = 0; j < maxUpgrades; ++j)
            {
                // Make the next level bar a button that will upgrade
                if (j == game.researchItems[i].level)
                {
                    ld::Button upgBtn(barPosX, barPosY, barWidth, upgBtns[i].height);
                    upgBtn.Draw(std::to_string(cost).c_str(), 5, ::Fade(researchColor[i], 0.15f));
                    if (upgBtn.IsHovered())
                    {
                        desc = ld::researchInfoLookup[i].desc;
                    }
                    if (upgBtn.IsClicked())
                    {
                        if (
                               (game.researchItems[i].level < ld::researchInfoLookup[i].maxLevel)
                            && (game.gold >= cost)
                        ) {
                            game.gold -= cost;
                            game.researchItems[i].level++;
                        }
                    }
                }
                else
                {
                    ::DrawRectangleLines(barPosX, barPosY, barWidth, upgBtns[i].height, ::DARKGRAY);
                }
                barPosX += barWidth;
            }

            fontSize = 20;
            uint32_t alignTextPos = upgBtns[i].xPos + upgBtns[i].width + 20 + (barWidth)*10;
            const char* barText = ::TextFormat("Level:%i %s", game.researchItems[i].level, game.researchItems[i].name.c_str());
            ld::DrawOutlinedText(barText, alignTextPos, barPosY + 0.5f*(upgBtns[i].height-fontSize), fontSize, researchColor[i], ::BLACK);
        }

        // Description of upgrade
        ld::DrawOutlinedCenteredText(desc.c_str(), x+w*0.5f, y + offset, 20, ::WHITE, ::BLACK);
    }

    { // -- Close
        uint32_t btnWidth = 70;
        uint32_t btnHeight = 35;
        ld::Button closeBtn(x + 0.5f*(w-btnWidth), y+h-btnHeight*0.5f, 70, 35);
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
        uint32_t width = 150;
        uint32_t height = 30;
        uint32_t fontSize = 20;

        const char* text = ::TextFormat("Gold: %i", game.gold);
        ld::DrawBar(text, xPos, yPos, width, height, fontSize, ::GOLD, 1.0f);
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
    auto panicBtn    = buttons.at("Panic");

    panicBtn.Draw("Panic Return", 10, ::Fade(::RED, 0.5));
    if (panicBtn.IsClicked()) {
      for (auto & miner : game.minerGroup.miners)
        miner.surfaceMiner();
    }


    auto buyMinerBtn = buttons.at("BuyMiner");
    auto researchBtn = buttons.at("Research");
    auto idleBtn     = buttons.at("Idle");
    auto surfBtn     = buttons.at("Surface");

    const char* hireText = ::TextFormat("Hire: %i Gold", game.minerCost);
    buyMinerBtn.Draw(hireText, 10, ::Fade(::LIGHTGRAY, 0.5f));
    researchBtn.Draw("Upgrades", 10, ::Fade(::LIGHTGRAY, 0.5f));

    std::vector<int32_t> idleMiners;
    for (auto miner : game.minerGroup.miners) {
        if (miner.aiState == ld::Miner::AiState::Idling) {
            idleMiners.push_back(miner.minerId);
        }
    }

    static size_t cycle = 0;
    std::string idleText = "Idle: " + std::to_string(idleMiners.size());
    idleBtn.Draw(idleText.c_str(), 10, ::Fade(::LIGHTGRAY, 0.5f));
    bool wasIdle = game.minerSelection >= 0;
    if (
        (::IsKeyPressed(KEY_SPACE) || idleBtn.IsClicked())
        && !idleMiners.empty()
    ) {
        // wrap cycle first in case it corresponds to an invalid index within the list
        cycle = (cycle+1) % idleMiners.size();
        game.minerSelection = idleMiners.at(cycle);

        if (wasIdle && idleMiners.size() == 1) {
          game.minerSelection = -1;
        }
    }

    surfBtn.Draw("Surface", 10, ::Fade(::LIGHTGRAY, 0.5f));
    if (surfBtn.IsClicked()) {
      game.camera.y = -618;
      game.camera.yVelocity = 0.0f;
    }

    // -- flag
    ::DrawRectangle(
      scrWidth-100+32,
      300, 32, 32, ::Fade(::RED, (game.targetX>=0?0.5f:0.0f))
    );
    auto &  flagCan = buttons.at("FlagCan");
    if (game.targetX>=0) {
      flagCan
        .DrawTexture(
          "",
          ld::TextureGet(ld::TextureType::Flag),
          0, 0,
          { 255, 255, 255, (game.targetX>=0) ? (uint8_t)(255) : (uint8_t)(0) },
          true
        );
    }
    bool thisFrame = false;
    if (flagCan.IsClicked()) {
      game.targetX = -1;
      game.targetY = -1;

      game.targetActive = 0;
      game.minerSelection = -1;
      thisFrame = true;
    }

    ::DrawRectangle(scrWidth-100, 300, 32, 32, ::Fade(::LIGHTGRAY, 0.5f));
    auto &  flag = buttons.at("Flag");
    flag
      .DrawTexture(
        "",
        ld::TextureGet(ld::TextureType::Flag),
        0, 0,
        { 255, 255, 255, (game.targetX>=0) ? (uint8_t)(128) : (uint8_t)(255) },
        true
      );

    if (flag.IsClicked()) {
      game.targetX = -1;
      game.targetY = -1;

      game.targetActive = 1;
      thisFrame = true;
      game.minerSelection = -1;
    }

    if (
        !thisFrame && game.targetActive
     && ::IsMouseButtonPressed(MOUSE_LEFT_BUTTON)
    ) {
      game.targetX = game.mineChasm.limitX(::GetMousePosition().x / 32);
      game.targetY =
        game.mineChasm.limitY((::GetMousePosition().y + game.camera.y) / 32);
      game.targetActive = 0;
    }
}

// Update any gamestate changes from button usage
void ld::Overlay::Update(ld::GameState & game)
{
    // End game when we're out of food, don't have miners, and out of gold
    if (game.food <= 0
        && game.minerGroup.miners.empty()
    ) {
        menuState = ld::Overlay::MenuState::GameOver;
    }


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

void ld::Overlay::MinerInfo(ld::GameState & game, ld::Miner & miner)
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
            case Miner::AiState::Fighting:
                action += "Fighting";
                break;
            case Miner::AiState::Dying:
                action += "Dying";
                break;
            case Miner::AiState::Idling:
                action += "Idling";
                break;
            case Miner::AiState::Traversing:
                action += "Traversing";
                break;
            case Miner::AiState::Mining:
                action += "Mining";
                break;
            case Miner::AiState::Surfaced:
                action += "Surfaced";
                break;
            case Miner::AiState::Inventorying:
                action += "Inventorying";
                break;
            default:
                action = "N/A BROKE!";
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
            if (equip.level > 0) {
                tint = ::WHITE;
            }

            ld::Button itemBtn(xOffset, y+padding, btnSize, btnSize);
            itemBtn.DrawTexture(
                std::to_string(equip.level).c_str(),
                ld::TextureGet(ld::TextureType::Misc),
                i, 1, tint
            );
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

                ld::Button itemBtn(xOffset, y+padding, btnSize, btnSize);
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

    { // -- net value for buying upgrades
        padding += 15;
        std::string valueText = "Net Value: " + std::to_string(miner.netValue);
        ld::DrawOutlinedCenteredText(valueText.c_str(), x+w*0.5f, y+padding, 10, ::WHITE);
    }

    padding+=20;

    if (
        miner.aiState != ld::Miner::AiState::Surfaced
     && miner.aiState != ld::Miner::AiState::Fighting
    ) { // -- Cancel current action
        int btnWidth = 60;
        int btnHeight = 25;
        ld::Button btn(x, y+padding, btnWidth, btnHeight);
        btn.Draw("Return", 7, ::WHITE);
        if (btn.IsClicked())
        {
          miner.surfaceMiner();
        }
    }

    { // -- Kill
        int btnWidth = 60;
        int btnHeight = 25;
        ld::Button btn(x + (w-btnWidth), y+padding, btnWidth, btnHeight);
        // sorry this is shit
        static int32_t hasClickedKill = 0;
        btn.Draw(hasClickedKill > 0 ? "Confirm" : "Kill", 7, ::RED);

        if (btn.IsClicked() && hasClickedKill == 0)
          hasClickedKill = 1;

        if (btn.IsClicked() && hasClickedKill == 2)
        {
          hasClickedKill = 0; // clicked off screen
          miner.kill();
          game.minerSelection = -1;
        }

        if (!btn.IsHovered()) {
          hasClickedKill = 0;
        }

        if (hasClickedKill == 1) {
          if (!::IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            hasClickedKill = 2;
          }
        }
    }

    // Gui sliders
    //::GuiSliderBar((::Rectangle) { }
}

void ld::Overlay::Draw(ld::GameState & game)
{
    // if game paused, then that should override every other menu
    if (game.isPaused && menuState != MenuState::Pause) {
        if (!menuStack.empty() && menuStack.top() != MenuState::Pause) {
            // Hacky, but checks that we weren't coming from a pause state
            // to prevent pushing another pause menu on top causing it to be
            // stuck on the pause and other pause related menus like instruction
            menuStack.push(menuState);
            menuState = MenuState::Pause;
        }
        else if (menuStack.empty()) {
            menuStack.push(menuState);
            menuState = MenuState::Pause;
        }
    }
    else if (!game.isPaused && menuState == MenuState::Pause) {
        if (!menuStack.empty()) {
            menuState = menuStack.top();
            menuStack.pop();
        } else {
            menuState = MenuState::None;
        }
    }

    switch (menuState)
    {
        case ld::Overlay::MenuState::Title:
            break;
            // TODO Reenable upon submission
            //TitleScreen();
            //return; // return to Avoid rendering resources
        case ld::Overlay::MenuState::Research:
            ResearchMenu(game);
            ResourceMenu(game);
            break;
        case ld::Overlay::MenuState::GameOver:
            GameOverScreen(game);
            break;
        case ld::Overlay::MenuState::Instructions:
            Instructions();
            break;
        case ld::Overlay::MenuState::Pause:
            PauseScreen(game);
            break;
        case ld::Overlay::MenuState::None:
        default:
            ResourceMenu(game);
            break;
    }

    if (game.minerSelection >= 0 && !game.isPaused)
    {
        bool found = false;
        // find miner & check if still alive
        for (auto & miner : game.minerGroup.miners) {
          if (miner.minerId == game.minerSelection) {
            found = true;
            MinerInfo(game, miner);
            break;
          }
        }

        if (!found) {
          game.minerSelection = 0;
        }
    }

    // Cursor should be drawn last
    if (game.showCursor)
    {
        auto mousePos = ::GetMousePosition();
        // Clamp mouse
        ::SetMousePosition(
            std::clamp(mousePos.x, 0.0f, static_cast<float>(scrWidth)),
            std::clamp(mousePos.y, 0.0f, static_cast<float>(scrHeight))
        );
        ::DrawCircleV(GetMousePosition(), 2, RED);
    }
}
