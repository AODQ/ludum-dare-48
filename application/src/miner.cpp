#include <miner.hpp>

#include <gamestate.hpp>

ld::MinerGroup ld::MinerGroup::Initialize() {
  ld::MinerGroup self;

  self.miners.push_back({.xPosition = 400}); // push a default

  self.surfacedMiners = {};
  self.chasmMiners = { 0 }; // put default miner in the chasm

  return self;
}

void ld::MinerGroup::Update(ld::GameState & state) {
  auto & self = state.minerGroup;

  // TODO update AI

  // TODO update mining / fighting

  // simple dumb ai right now
  for (auto & miner : self.miners) {
    auto rockId = miner.aiStateInternal.mining.targetRockId;
    auto & rock = state.mineChasm.rock(rockId);

    if (rock.isMined()) {
      miner.aiStateInternal.mining.targetRockId += 1;
    }

    // TODO path to the rock
    miner.xPosition = state.mineChasm.rockPositionX(rockId)*32.0f;
    miner.yPosition = state.mineChasm.rockPositionY(rockId)*32.0f - 32.0f;
  }

  // TODO update durability

  // TODO update animation state
  for (auto & miner : self.miners) {
    miner.animationIdx = (miner.animationIdx + 5) % (4 * 60);
  }

  // TODO update surfacing

  // TODO update inventoy
}
