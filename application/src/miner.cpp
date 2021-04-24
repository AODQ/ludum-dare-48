#include <miner.hpp>

#include <gamestate.hpp>
#include <sounds.hpp>

#include <algorithm>

ld::MinerGroup ld::MinerGroup::Initialize() {
  ld::MinerGroup self;

  self.miners.push_back({.minerId = 0, .xPosition = 400,}); // push a default

  self.surfacedMiners = {};
  self.chasmMiners = { 0 }; // put default miner in the chasm

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
      state.waitTimer = 60 * 5; // 5 second wait
      miner.animationState = ld::Miner::AnimationState::Idling;
      miner.animationIdx = 0;
    }

    if (state.waitTimer == 0) {
      // transition
      if (state.wantsToSurface) {
        // surfacing miner
        miner.aiState = ld::Miner::AiState::Surfaced;
        gameState.minerGroup.TransitionMiner(miner.minerId, false);
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
        state.waitTimer = 5*60;
      }

      if (state.waitTimer == 0) {
        state.state = ld::Miner::AiStateSurfaced::DumpingMaterial;
        miner.animationState = ld::Miner::AnimationState::Travelling;
        miner.animationIdx = 0;
      }

      state.waitTimer -= 1;
    break;
    case ld::Miner::AiStateSurfaced::DumpingMaterial:
      miner.moveTowards(200, -100);
    break;
    default: break;
  }
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
      case ld::Miner::AiState::Idling: break;
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

void ld::MinerGroup::TransitionMiner(size_t minerId, bool isCurrentlySurfaced)
{
  auto & self = *this;

  std::vector<size_t> * originMiners, * destinationMiners;
  if (isCurrentlySurfaced) {
    originMiners = &self.surfacedMiners;
    destinationMiners = &self.chasmMiners;
  } else {
    originMiners = &self.chasmMiners;
    destinationMiners = &self.surfacedMiners;
  }

  size_t surfacedMinerIdx = 0;
  for (; surfacedMinerIdx < originMiners->size(); ++ surfacedMinerIdx) {
    if (self.surfacedMiners[surfacedMinerIdx] == minerId) { break; }
  }

  if (surfacedMinerIdx == self.surfacedMiners.size()) {
    TraceLog(LOG_ERROR, "Trying to transition miner but couldn't find ID");
    return;
  }

  destinationMiners->push_back(minerId);
  originMiners->erase(originMiners->begin() + surfacedMinerIdx);

  // TODO assert that there's no missing miners in any of the containers
}
