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
      int32_t minerSelection = -1;
      bool isPaused = false;

      int32_t foodEatTimer = 60*5;

      Camera camera;

      MinerGroup minerGroup;
      MobGroup mobGroup;
      MineChasm mineChasm;

      Miner * getSelectedMiner() {
        Miner * ret = nullptr;
        bool found = false;
        // find miner & check if still alive
        for (auto & miner : minerGroup.miners) {
          if (miner.minerId == minerSelection) {
            found = true;
            ret = &miner;
            break;
          }
        }

        if (!found) {
          minerSelection = 0;
        }

        return ret;
      }
  };
}
