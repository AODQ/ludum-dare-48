#include <miner.hpp>

#include <gamestate.hpp>
#include <pathfinder.hpp>
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

  for (uint32_t i = 0; i < speed; ++ i) {
    self.xPosition -= ld::sgn(self.xPosition - x);
    self.yPosition -= ld::sgn(self.yPosition - y);
  }
}

int32_t ld::Miner::animationFrames() const
{
  auto & self = *this;
  return self.animationState == ld::Miner::AnimationState::Idling ? 3 : 4;
}

bool ld::Miner::animationFinishesThisFrame()
{
  auto & self = *this;
  return
    (self.animationIdx + 5) % (self.animationFrames() * 60) < self.animationIdx
  ;
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
  miner.aiState = ld::Miner::AiState::Traversing;
  auto & state = miner.aiStateInternal.traversing;
  state.pathSize = 0;
  state.pathIdx = 0;

  ld::pathFind(
    gameState,
    state.path, state.pathSize,
    miner.xPosition / 32, miner.yPosition / 32,
    targetTileX, targetTileY,
    state.wantsToSurface ? false : true // can mine
  );
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

  // reset back to traversing as before
  if (rock.isMined()) {
    miner.aiState = ld::Miner::AiState::Traversing;
    return;
  }

  miner.applyAnimationState(ld::Miner::AnimationState::Mining);

  if (miner.animationFinishesThisFrame()) {
    miner.reduceEnergy(10);
    rock.receiveDamage(-5);
    UpdateMinerInventory(miner, rock);
    ld::SoundPlay(ld::SoundType::RockHit);

    if (miner.currentCargoCapacity >= miner.cargoCapacity) {
      miner.aiState = ld::Miner::AiState::Traversing;
      miner.aiStateInternal.traversing.wantsToSurface = true;
      miner.aiStateInternal.traversing.pathSize = 0;
      miner.aiStateInternal.traversing.pathIdx  = 0;
      miner.aiStateInternal.traversing.targetTileX = miner.xPosition/32;
      miner.aiStateInternal.traversing.targetTileY = 0;
    }
  }
}

void UpdateMinerAiTraversing(ld::Miner & miner, ld::GameState & gameState)
{
  auto & state = miner.aiStateInternal.traversing;

  if (state.pathIdx >= state.pathSize) {

    MinerPickLocation(
      miner,
      state.targetTileX,
      state.targetTileY,
      gameState
    );

    // if no path was selected just leave
    if (state.pathIdx >= state.pathSize) {
      miner.aiState = ld::Miner::AiState::Traversing;
      miner.aiStateInternal.traversing.wantsToSurface = true;
      miner.aiStateInternal.traversing.pathSize = 0;
      miner.aiStateInternal.traversing.pathIdx  = 0;
      miner.aiStateInternal.traversing.targetTileX = miner.xPosition/32;
      miner.aiStateInternal.traversing.targetTileY = 0;
      return;
    }
  }

  auto & path = state.path[state.pathIdx];

  miner.applyAnimationState(ld::Miner::AnimationState::Travelling);
  miner.moveTowards(
    path.x*32.0f + state.targetPosOffX,
    path.y*32.0f + state.targetPosOffY
  );

  if (miner.animationFinishesThisFrame()) {
    miner.reduceEnergy(5);
  }

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
      8.0f,
      rect
    )
  ) {
    state.pathIdx += 1;

    if (state.wantsToSurface && miner.yPosition < 48.0f) {
      // surface
      miner.aiState = ld::Miner::AiState::Surfaced;
      miner.aiStateInternal.surfaced.state =
        ld::Miner::AiStateSurfaced::Surfacing;
      miner.aiStateInternal.surfaced.waitTimer = -1;
    }

    state.targetPosOffX = ::GetRandomValue(0, 16);
    state.targetPosOffY = ::GetRandomValue(0, 16);

    auto rockId = gameState.mineChasm.rockId(path.x, path.y);
    auto & rock = gameState.mineChasm.rock(rockId);
    if (!rock.isMined()) {
      miner.aiState = ld::Miner::AiState::Mining;
      miner.aiStateInternal.mining.targetRockId = rockId;
    }

    if (state.targetTileX == path.x && state.targetTileY == path.y) {

      // should just leave
      if (miner.energy <= 200) {
        miner.aiState = ld::Miner::AiState::Traversing;
        miner.aiStateInternal.traversing.pathSize = 0;
        miner.aiStateInternal.traversing.pathIdx  = 0;
        miner.aiStateInternal.traversing.wantsToSurface = true;
        miner.aiStateInternal.traversing.targetTileX = miner.xPosition/32;
        miner.aiStateInternal.traversing.targetTileY = 0;
        return;
      }
      state.targetTileX =
        gameState.mineChasm.limitX(
          state.targetTileX + ::GetRandomValue(-10, +10)
        );

      state.targetTileY =
        gameState.mineChasm.limitY(
          state.targetTileY + ::GetRandomValue(-10, +10)
        );
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

        state.targetX = ::GetRandomValue(50, 180);
        state.targetY = ::GetRandomValue(-120, -80);
      }

      state.waitTimer -= 1;
    break;
    case ld::Miner::AiStateSurfaced::MovingToBase:
      miner.moveTowards(state.targetX, state.targetY);

      if (miner.animationFinishesThisFrame()) {
        miner.reduceEnergy(5);
      }

      if (
          miner.xPosition == state.targetX
       && miner.yPosition == state.targetY
      ) {
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
        UpdateMinerCargo(miner);
        gameState.gold += ld::valuableInfoLookup[Idx(cargo.type)].value;
        gameState.notifGroup.AddNotif(
          ld::NotifType::ItemSold, miner.xPosition, miner.yPosition
        );
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
        gameState.notifGroup.AddNotif(
          ld::NotifType::FoodGot, miner.xPosition, miner.yPosition
        );
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

  // draw
  if (miner.minerId == gameState.minerSelection) {
    if (::IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
    {
      auto mousePos = ::GetMousePosition();
      miner.animationIdx = 0;
      miner.animationState = ld::Miner::AnimationState::Travelling;
      miner.aiState = ld::Miner::AiState::Traversing;
      miner.aiStateInternal.traversing.pathSize = 0;
      miner.aiStateInternal.traversing.pathIdx = 0;
      miner.aiStateInternal.traversing.targetTileX = mousePos.x / 32.0f;
      miner.aiStateInternal.traversing.targetTileY =
        (mousePos.y + gameState.camera.y) / 32.0f;
      miner.aiStateInternal.traversing.wantsToSurface = false;
    }
  }

}

} // -- namespace

void ld::MinerGroup::Update(ld::GameState & state) {
  auto & self = state.minerGroup;

  // Update miner stats based on upgrades
  for (int64_t i = 0; i < self.miners.size(); ++ i) {
    auto & miner = self.miners[i];
    miner.cargoCapacity = state.MaxCargoCapacity();
    miner.speed = state.MinerSpeed();
    // Weapons
    // Armor
  }

  // selecting a miner via mouse click
  if (::IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    auto mousePos = ::GetMousePosition();
    for (int64_t i = 0; i < self.miners.size(); ++ i) {
      auto & miner = self.miners[i];
      if(
        ::CheckCollisionPointCircle(
          mousePos,
          ::Vector2 {
            miner.xPosition + 8.0f,
            miner.yPosition + 8.0f - state.camera.y,
          },
          9.0f
        )
      ) {
        state.minerSelection = miner.minerId;
        break;
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
      case ld::Miner::AiState::Dying:
        if (miner.animationFinishesThisFrame()) {
          self.miners.erase(self.miners.begin() + i);
          i -= 1;
          continue;
        }
      break;
    }
  }

  for (auto & miner : self.miners) {
    miner.animationIdx =
      (miner.animationIdx + 5) % (miner.animationFrames() * 60);
  }

  // update fog of war
  for (auto & miner : self.miners) {
    if (miner.aiState == ld::Miner::AiState::Surfaced) { continue; }

    int32_t const minBoundsX = state.mineChasm.limitX(miner.xPosition/32 - 3);
    int32_t const minBoundsY = state.mineChasm.limitY(miner.yPosition/32 - 3);
    int32_t const maxBoundsX = state.mineChasm.limitX(miner.xPosition/32 + 3);
    int32_t const maxBoundsY = state.mineChasm.limitY(miner.yPosition/32 + 3);

    for (int32_t x = minBoundsX; x < maxBoundsX; ++ x)
    for (int32_t y = minBoundsY; y < maxBoundsY; ++ y) {
      auto & fow = state.mineChasm.rockFow[state.mineChasm.rockId(x, y)];

      float len =
        std::clamp(
            (3*::sqrt(2.0f))
          - static_cast<float>(
              ::sqrt(
                (miner.xPosition/32 - x)*(miner.xPosition/32 - x)
              + (miner.yPosition/32 - y)*(miner.yPosition/32 - y)
              )
            )
          , 0.0f, (3*::sqrt(2.0f))
        )
      ;

      fow = std::clamp(len/(3.0f), fow, 1.0f);
    }
  }
}

void ld::MinerGroup::addMiner()
{
  auto & self = *this;

  auto id = self.miners.size();

  self.miners.push_back(
    ld::Miner {
      .minerId = static_cast<int32_t>(id),
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
