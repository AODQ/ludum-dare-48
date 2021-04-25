#include <miner.hpp>

#include <gamestate.hpp>
#include <sounds.hpp>

#include <algorithm>

ld::MinerGroup ld::MinerGroup::Initialize() {
  ld::MinerGroup self;

  self.addMiner();

  return self;
}

void ld::Miner::moveTowards(int32_t x, int32_t y)
{
  auto & self = *this;
  self.prevXPosition = self.xPosition;
  self.prevYPosition = self.yPosition;

  for (int i = 0; i < 3; ++ i) {
    self.xPosition -= ld::sgn(self.xPosition - x);
    self.yPosition -= ld::sgn(self.yPosition - y);
  }
}

bool ld::Miner::animationFinishesThisFrame()
{
  auto & self = *this;
  return ((self.animationIdx + 5) % (4 * 60) < self.animationIdx);
}

void ld::Miner::reduceEnergy(int32_t units)
{
  auto & self = *this;
  self.energy = std::max(0, self.energy - std::abs(units));

  if (self.energy == 0) { self.kill(); }
}

void ld::Miner::kill()
{
  auto & self = *this;
  if (self.aiState == ld::Miner::AiState::Dying) { return; }

  self.aiState = ld::Miner::AiState::Dying;
  self.aiStateInternal.dying = {};
  self.animationState = ld::Miner::AnimationState::Dying;
  self.animationIdx = 0;
}

namespace {

void MinerPickLocation(
  ld::Miner & miner,
  int32_t const targetTileX,
  int32_t const targetTileY,
  ld::GameState const & gameState
) {

  miner.aiState = ld::Miner::AiState::MineTraversing;
  auto & state = miner.aiStateInternal.mineTraversing;
  state.path.clear();
  state.pathIdx = 0;

  // *very* naive path finder
  // these miners are DUMB! and very near-sighted!
  /*

       build a cone in a direction to pick

            *
          * *
      X * * *
          * *
            *
  */
  int32_t previousTileX2 = 0;
  int32_t previousTileY2 = 0;
  int32_t previousTileX = static_cast<int32_t>(miner.xPosition / 32.0f);
  int32_t previousTileY = static_cast<int32_t>(miner.yPosition / 32.0f);

  for (size_t i = 0; i < 3ul; ++ i) {

    struct PossLocs {
      int32_t x, y;
    };

    constexpr std::array<PossLocs, 4> directions = {{
      { -1, 0 }, { +1, 0 }, {  0, +1 }, {  0, -1 },
    }};

    std::array<int32_t, 4> pathValue = {{ 0, 0, 0, 0, }};

    if (previousTileX > targetTileX) pathValue[0] = 250;
    if (previousTileX < targetTileX) pathValue[1] = 250;
    if (previousTileY < targetTileY) pathValue[2] = 500;
    if (previousTileY > targetTileY) pathValue[3] = 550;

    if (previousTileX - 1 < 0)
      pathValue[0] = -55000;
    if (previousTileX + 1 >= static_cast<int32_t>(gameState.mineChasm.columns))
      pathValue[1] = -55000;
    if (previousTileY - 1 < 0)
      pathValue[3] = -55000;

    // down is always better
    pathValue[2] += 50;

    // up is always slightly worse
    pathValue[3] += -50;

    for (auto & path : pathValue)
      path += ::GetRandomValue(-20, 100);

    for (size_t directionIt = 0ul; directionIt < 4ul; ++ directionIt) {
      auto const direction = directions[directionIt];
      constexpr std::array<PossLocs, 9> offsets = {{
        { +1, 0 }, { +2,  0 }, { +3,  0 },
                   { +2, +1 }, { +3, +2 },
                   { +2, -1 }, { +3, -2 },
                               { +3, +2 },
                               { +3, -2 },
      }};

      for (auto const offset : offsets) {
        // transform cone into local space
        int32_t pickTileX =
           direction.x == -1 ? -offset.x
        : (direction.x == +1 ?  offset.x
        : (direction.y == -1 ? -offset.y
        :                       offset.y
        ));

        int32_t pickTileY =
           direction.x == -1 ? -offset.y
        : (direction.x == +1 ?  offset.y
        : (direction.y == -1 ? -offset.x
        :                       offset.x
        ));

        // transform cone into global space & select rock
        pathValue[directionIt] +=
          gameState.mineChasm.rockPathValue(
            previousTileX+pickTileX,
            previousTileY+pickTileY
          )
          * (1.0f / (offset.x+offset.y));

        if (
            previousTileX+pickTileX == previousTileX2
         && previousTileY+pickTileY == previousTileY2
        ) {
          pathValue[directionIt] -= 1000;
        }
      }
    }

    TraceLog(LOG_INFO, "path <%d, %d> -> <%d, %d> (left %d) (right %d) (down %d) (up %d)",
      previousTileX, previousTileY,
      targetTileX, targetTileY,
      pathValue[0],
      pathValue[1],
      pathValue[2],
      pathValue[3]
      );

    int32_t selectedPath = -1;
    int32_t selectedPathMaxValue = INT32_MIN;
    for (int32_t pathIt = 0; pathIt < 4; ++ pathIt) {
      if (pathValue[pathIt] > selectedPathMaxValue) {
        selectedPath = pathIt;
        selectedPathMaxValue = pathValue[pathIt];
      }
    }

    if (selectedPath == -1) { break; }

    int32_t newTileX = previousTileX + directions[selectedPath].x;
    int32_t newTileY = previousTileY + directions[selectedPath].y;

    state.path.push_back(
      ::Vector2{static_cast<float>(newTileX), static_cast<float>(newTileY)}
    );

    previousTileX2 = previousTileX;
    previousTileY2 = previousTileY;
    previousTileX = newTileX;
    previousTileY = newTileY;
  }
}


void UpdateMinerCargo(ld::Miner & miner) {
  miner.currentCargoCapacity = 0;
  for (size_t it = 0u; it < miner.cargo.size(); ++ it) {
    miner.currentCargoCapacity +=
        miner.cargo[it].ownedUnits
      * ld::valuableInfoLookup[it].weightUnitMultiplier
    ;
  }
}

void UpdateMinerInventory(ld::Miner & miner, ld::MineRock const & rock)
{
  switch (rock.gem) {
    default:
      miner.cargo[Idx(ld::ValuableType::Stone)].ownedUnits += 1;
    break;
  }

  UpdateMinerCargo(miner);
}

void UpdateMinerAiMining(ld::Miner & miner, ld::GameState & state) {
  auto rockId = miner.aiStateInternal.mining.targetRockId;
  auto & rock = state.mineChasm.rock(rockId);

  if (rock.isMined()) {
    miner.aiState = ld::Miner::AiState::MineTraversing;
    return;
  }

  // TODO path to the rock
  miner.xPosition = state.mineChasm.rockPositionX(rockId)*32.0f;
  miner.yPosition = state.mineChasm.rockPositionY(rockId)*32.0f - 8.0f;

  miner.applyAnimationState(ld::Miner::AnimationState::Mining);

  if (miner.animationFinishesThisFrame()) {
    miner.reduceEnergy(1);
    rock.receiveDamage(-1);
    UpdateMinerInventory(miner, rock);
    ld::SoundPlay(ld::SoundType::RockHit);

    if (miner.currentCargoCapacity >= miner.cargoCapacity) {
      miner.aiState = ld::Miner::AiState::Traversing;
      miner.aiStateInternal.traversing.wantsToSurface = true;
      miner.aiStateInternal.traversing.waitTimer = -1;
      miner.aiStateInternal.traversing.targetTileX = miner.xPosition;
      miner.aiStateInternal.traversing.targetTileY = 0;
    }
  }
}

void UpdateMinerAiTraversing(ld::Miner & miner, ld::GameState & /*gameState*/)
{
  auto & state = miner.aiStateInternal.traversing;
  miner.moveTowards(state.targetTileX, state.targetTileY);

  miner.applyAnimationState(ld::Miner::AnimationState::Travelling);

  // clamp to game bounds
  miner.xPosition = std::clamp(miner.xPosition, 0, 30*32+16);
  miner.yPosition = std::clamp(miner.yPosition, 0, 80*32+16);

  miner.applyAnimationState(ld::Miner::AnimationState::Travelling);

  if (miner.animationFinishesThisFrame()) {
    miner.reduceEnergy(1);
  }

  if (
    miner.xPosition == state.targetTileX && miner.yPosition == state.targetTileY
  ) {
    // transition
    if (state.wantsToSurface) {
      // surfacing miner
      miner.aiState = ld::Miner::AiState::Surfaced;
      miner.aiStateInternal.surfaced.state =
        ld::Miner::AiStateSurfaced::Surfacing;
      miner.aiStateInternal.surfaced.waitTimer = -1;
    }
  }

  // update energy
  miner.reduceEnergy(1);
}

void UpdateMinerAiMineTraversing(ld::Miner & miner, ld::GameState & gameState)
{
  auto & state = miner.aiStateInternal.mineTraversing;

  if (state.pathIdx >= state.path.size()) {

    if (state.hasHitTarget) {
      miner.aiState = ld::Miner::AiState::Traversing;
      miner.aiStateInternal.traversing.wantsToSurface = true;
      miner.aiStateInternal.traversing.waitTimer = -1;
      miner.aiStateInternal.traversing.targetTileX = 20;
      miner.aiStateInternal.traversing.targetTileY = 0;
      return;
    }

    MinerPickLocation(
      miner,
      state.targetTileX,
      state.targetTileY,
      gameState
    );
    state.hasHitTarget = false;

    // if no path was selected just leave
    if (state.path.size() == 0) {
      miner.aiState = ld::Miner::AiState::Traversing;
      miner.aiStateInternal.traversing.wantsToSurface = true;
      miner.aiStateInternal.traversing.waitTimer = -1;
      miner.aiStateInternal.traversing.targetTileX = 20;
      miner.aiStateInternal.traversing.targetTileY = 0;
      return;
    }
  }

  auto & path = state.path[state.pathIdx];

  miner.applyAnimationState(ld::Miner::AnimationState::Travelling);
  miner.moveTowards(path.x*32.0f, path.y*32.0f);

  ::Rectangle rect = {
    .x = path.x*32.0f, .y = path.y*32.0f,
    .width = 32.0f, .height = 32.0f,
  };

  if (
    ::CheckCollisionCircleRec(
      ::Vector2 {
        static_cast<float>(miner.xPosition),
        static_cast<float>(miner.yPosition),
      },
      24.0f,
      rect
    )
  ) {
    state.pathIdx += 1;

    auto rockId = gameState.mineChasm.rockId(path.x, path.y);
    auto & rock = gameState.mineChasm.rock(rockId);
    if (!rock.isMined()) {
      miner.aiState = ld::Miner::AiState::Mining;
      miner.aiStateInternal.mining.targetRockId = rockId;
    }

    if (state.targetTileX == path.x && state.targetTileY == path.y) {
      state.hasHitTarget = true;
    }
  }
}

void UpdateMinerAiSurfaced(ld::Miner & miner, ld::GameState & gameState)
{
  auto & state = miner.aiStateInternal.surfaced;
  switch (state.state) {
    case ld::Miner::AiStateSurfaced::Surfacing:
      miner.xPosition = 700;
      miner.yPosition = -100;

      miner.animationState = ld::Miner::AnimationState::Idling;

      if (state.waitTimer < 0) {
        state.waitTimer = 1*60;
      }

      if (state.waitTimer == 0) {
        state.state = ld::Miner::AiStateSurfaced::MovingToBase;
        miner.animationState = ld::Miner::AnimationState::Travelling;
        miner.animationIdx = 0;
      }

      state.waitTimer -= 1;
    break;
    case ld::Miner::AiStateSurfaced::MovingToBase:
      miner.moveTowards(200, -100);
      if (miner.xPosition == 200 && miner.yPosition == -100) {
        miner.animationState = ld::Miner::AnimationState::Idling;
        miner.animationIdx = 0;
        state.waitTimer = 50;
        state.state = ld::Miner::AiStateSurfaced::DumpingMaterial;
      }
    break;
    case ld::Miner::AiStateSurfaced::DumpingMaterial: {
      state.waitTimer -= 1;

      if (state.waitTimer > 0) { break; }

      // sell item
      state.waitTimer = 20;

      bool hasSold = false;

      for (auto & cargo : miner.cargo) {
        if (cargo.ownedUnits == 0) { continue; }

        hasSold = true;

        cargo.ownedUnits -= 1;
        gameState.gold += ld::valuableInfoLookup[Idx(cargo.type)].value;
      }

      if (!hasSold) {
        state.state = ld::Miner::AiStateSurfaced::PurchasingUpgrades;
        state.waitTimer = 20;
      }

    } break;
    case ld::Miner::AiStateSurfaced::PurchasingUpgrades: {
      state.waitTimer -= 1;
      if (state.waitTimer > 0) { break; }
      state.waitTimer = 20;

      bool readyToContinue = true;

      // be conservative ; don't eat unless there is 100% efficiency
      if (
          readyToContinue
       && miner.energy < miner.maxEnergy - miner.foodToEnergyRatio
       && gameState.food > 0
      ) {
        gameState.food -= 1;
        miner.energy =
          std::min(miner.maxEnergy, miner.energy + miner.foodToEnergyRatio);
        readyToContinue = false;
      }

      if (readyToContinue) {
        state.state = ld::Miner::AiStateSurfaced::BackToMine;
        miner.animationState = ld::Miner::AnimationState::Travelling;
        miner.animationIdx = 0;
      }
    } break;
    case ld::Miner::AiStateSurfaced::BackToMine:
      miner.moveTowards(700, -100);
      if (miner.xPosition == 700 && miner.yPosition == -100) {
        miner.aiState = ld::Miner::AiState::Idling;
        miner.xPosition = ::GetRandomValue(100, 700);
        miner.yPosition = ::GetRandomValue(10, 30);
      }
    break;
  }
}

void UpdateMinerAiIdling(ld::Miner & miner, ld::GameState & gameState)
{
  miner.animationState = ld::Miner::AnimationState::Idling;

  miner.animationIdx = 0;
  miner.animationState = ld::Miner::AnimationState::Travelling;
  miner.aiState = ld::Miner::AiState::MineTraversing;
  miner.aiStateInternal.mineTraversing.path.clear();
  miner.aiStateInternal.mineTraversing.pathIdx = 0;
  miner.aiStateInternal.mineTraversing.targetTileX =
    ::GetRandomValue(0, 29);
  miner.aiStateInternal.mineTraversing.targetTileY =
    ::GetRandomValue(4, 8);
}

} // -- namespace

void ld::MinerGroup::Update(ld::GameState & state) {
  auto & self = state.minerGroup;

  // selecting a miner via mouse click
  if (::IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
  {
      auto mousePos = ::GetMousePosition();
      state.selection = -1;
      for (int64_t i = 0; i < self.miners.size(); ++ i) {
          auto & miner = self.miners[i];
          if(
            ::CheckCollisionPointCircle(
              mousePos,
              ::Vector2 {
                static_cast<float>(miner.xPosition),
                static_cast<float>(miner.yPosition),
              },
              24.0f
            )
          ) {
            state.selection = i;
          }
      }
  }

  // TODO update AI

  // TODO update mining / fighting

  // simple dumb ai right now
  for (int64_t i = 0; i < self.miners.size(); ++ i) {
    auto & miner = self.miners[i];

    switch (miner.aiState) {
      case ld::Miner::AiState::Mining:
        UpdateMinerAiMining(miner, state);
      break;
      case ld::Miner::AiState::Attacking: break;
      case ld::Miner::AiState::Idling:
        UpdateMinerAiIdling(miner, state);
      break;
      case ld::Miner::AiState::Surfaced:
        UpdateMinerAiSurfaced(miner, state);
      break;
      case ld::Miner::AiState::Traversing:
        UpdateMinerAiTraversing(miner, state);
      break;
      case ld::Miner::AiState::MineTraversing:
        UpdateMinerAiMineTraversing(miner, state);
      break;
      case ld::Miner::AiState::Dying:
        if (miner.animationFinishesThisFrame()) {
          self.miners.erase(self.miners.begin() + i);
          i -= 1;
          continue;
        }
      break;
    }
  }

  // TODO update durability

  // TODO update animation state
  for (auto & miner : self.miners) {
    miner.animationIdx = (miner.animationIdx + 5) % (4 * 60);
  }

  // TODO update surfacing

  // TODO update inventoy
}

void ld::MinerGroup::addMiner()
{
  auto & self = *this;

  auto id = self.miners.size();

  self.miners.push_back(
    ld::Miner {
      .minerId = id,
      .xPosition = 700,
      .yPosition = -100,

      // TODO this is for debug
      .aiState = ld::Miner::AiState::Surfaced,
      .aiStateInternal = {
        .surfaced = {
          .state = ld::Miner::AiStateSurfaced::BackToMine,
          .waitTimer = -1,
        },
      },


      // TODO use below on release
      /* .aiState = ld::Miner::AiState::Surfaced, */
      /* .aiStateInternal = { */
      /*   .surfaced = { */
      /*     .state = ld::Miner::AiStateSurfaced::Surfacing, */
      /*     .waitTimer = -1, */
      /*   }, */
      /* }, */
    }
  );
}
