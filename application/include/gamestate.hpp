#pragma once

#include <camera.hpp>
#include <mine.hpp>
#include <miner.hpp>
#include <mob.hpp>

namespace ld {
  struct GameState
  {
      int32_t food = 100;
      int32_t maxFood = 200;
      int32_t gold = 100;
      uint32_t minerCost = 5;
      int32_t selection = 0;
      bool isPaused = false;

      Camera camera;

      MinerGroup minerGroup;
      MobGroup mobGroup;
      MineChasm mineChasm;
  };
}
