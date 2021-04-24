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

  self.xPosition -= ld::sgn(self.xPosition - x);
  self.yPosition -= ld::sgn(self.yPosition - y);
}

namespace {
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
    miner.aiStateInternal.mining.targetRockId += 1;
    return;
  }

  // TODO path to the rock
  miner.xPosition = state.mineChasm.rockPositionX(rockId)*32.0f;
  miner.yPosition = state.mineChasm.rockPositionY(rockId)*32.0f - 8.0f;

  if (miner.animationState != ld::Miner::AnimationState::Mining) {
    miner.animationState = ld::Miner::AnimationState::Mining;
    miner.animationIdx = 0;
  }

  if ((miner.animationIdx + 5) % (4 * 60) < miner.animationIdx) {
    rock.receiveDamage(-1);
    UpdateMinerInventory(miner, rock);
    ld::SoundPlay(ld::SoundType::RockHit);

    if (miner.currentCargoCapacity >= miner.cargoCapacity) {
      miner.aiState = ld::Miner::AiState::Traversing;
      miner.aiStateInternal.traversing.wantsToSurface = true;
      miner.aiStateInternal.traversing.waitTimer = -1;
      miner.aiStateInternal.traversing.targetTileX = 20;
      miner.aiStateInternal.traversing.targetTileY = 0;
    }
  }
}

void UpdateMinerAiTraversing(ld::Miner & miner, ld::GameState & gameState) {
  auto & state = miner.aiStateInternal.traversing;
  miner.moveTowards(state.targetTileX, state.targetTileY);

  // clamp to game bounds
  miner.xPosition = std::clamp(miner.xPosition, 0, 30*32+16);
  miner.yPosition = std::clamp(miner.yPosition, 0, 80*32+16);

  if (miner.animationState != ld::Miner::AnimationState::Travelling) {
    miner.animationState = ld::Miner::AnimationState::Travelling;
    miner.animationIdx = 0;
  }

  if (
    miner.xPosition == state.targetTileX && miner.yPosition == state.targetTileY
  ) {
    if (state.waitTimer < 0) {
      state.waitTimer = 30 * 1; // 0.5 second wait
      miner.animationState = ld::Miner::AnimationState::Idling;
      miner.animationIdx = 0;
    }

    if (state.waitTimer == 0) {
      // transition
      if (state.wantsToSurface) {
        // surfacing miner
        miner.aiState = ld::Miner::AiState::Surfaced;
        miner.aiStateInternal.surfaced.state =
          ld::Miner::AiStateSurfaced::Surfacing;
        miner.aiStateInternal.surfaced.waitTimer = -1;
      }
    }

    state.waitTimer -= 1;
  }
}

void UpdateMinerAiSurfaced(ld::Miner & miner, ld::GameState & gameState) {
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
        state.waitTimer = 40;
      }

    } break;
    case ld::Miner::AiStateSurfaced::PurchasingUpgrades:
      state.state = ld::Miner::AiStateSurfaced::BackToMine;
      miner.animationState = ld::Miner::AnimationState::Travelling;
      miner.animationIdx = 0;
    break;
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

void UpdateMinerAiIdling(ld::Miner & miner, ld::GameState & /*gameState*/)
{
  miner.animationState = ld::Miner::AnimationState::Idling;

  miner.animationIdx = 0;
  miner.animationState = ld::Miner::AnimationState::Travelling;
  miner.aiState = ld::Miner::AiState::Mining;
  miner.aiStateInternal.mining = {};
}

} // -- namespace

void ld::MinerGroup::Update(ld::GameState & state) {
  auto & self = state.minerGroup;

  // TODO update AI

  // TODO update mining / fighting

  // simple dumb ai right now
  for (auto & miner : self.miners) {

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
      .aiState = ld::Miner::AiState::Surfaced,
      .aiStateInternal = {
        .surfaced = {
          .state = ld::Miner::AiStateSurfaced::Surfacing,
          .waitTimer = -1,
        },
      },
    }
  );
}
