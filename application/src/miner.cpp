#include <miner.hpp>

ld::MinerGroup ld::MinerGroup::Initialize() {
  ld::MinerGroup self;

  self.miners.push_back({.xPosition = 400}); // push a default

  self.surfacedMiners = {};
  self.chasmMiners = { 0 }; // put default miner in the chasm

  return self;
}

void ld::MinerGroup::Update() {
  auto & self = *this;

  // TODO update AI

  // TODO update mining / fighting

  // simple dumb ai right now

  // TODO update durability

  // TODO update animation state
  for (auto & miner : self.miners) {
    miner.animationIdx = (miner.animationIdx + 5) % (4 * 60);
  }

  // TODO update surfacing

  // TODO update inventoy
}
