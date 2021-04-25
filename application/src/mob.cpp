#include <mob.hpp>

#include <gamestate.hpp>

#include <algorithm>

void ld::MobGroup::Update(ld::GameState & state)
{
  for (auto & slime : state.mobGroup.slimes) {
    slime.animationIdx = (slime.animationIdx + 5) % (60*5);

    // TODO add to FOW if this is detected
  }

  for (auto & cloud : state.mobGroup.poisonClouds) {
    cloud.animationIdx = (cloud.animationIdx + 5) % (60*2);

    // TODO add to FOW if this is detected
  }
}

ld::MobGroup ld::MobGroup::Initialize()
{
  MobGroup self;

  return self;
}
