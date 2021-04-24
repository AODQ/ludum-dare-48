#include <mob.hpp>

#include <gamestate.hpp>

#include <algorithm>

void ld::MobGroup::Update(ld::GameState & state)
{
  for (auto & slime : state.mobGroup.slimes) {
    slime.animationIdx = (slime.animationIdx + 5) % (60*5);
  }

  for (auto & cloud : state.mobGroup.poisonClouds) {
    cloud.animationIdx = (cloud.animationIdx + 5) % (60*2);
  }
}

ld::MobGroup ld::MobGroup::Initialize()
{
  MobGroup self;

  return self;
}
