#pragma once

#include <camera.hpp>
#include <mine.hpp>
#include <miner.hpp>
#include <mob.hpp>
#include <notifs.hpp>
#include <research.hpp>

namespace ld {
  struct GameState
  {
      int timeScale = 1;
      bool showCursor = true;
      // below is in tiles
      int32_t targetX = -1, targetY = -1;
      bool targetActive = 0;

      int32_t food = 500;
      int32_t gold = 100;
      uint32_t foodEatTimer = 60 * 5;
      uint32_t minerCost = 5;
      int32_t minerSelection = -1;
      bool isPaused = false;
      Camera camera;
      MinerGroup minerGroup;
      MobGroup mobGroup;
      MineChasm mineChasm;
      NotifGroup notifGroup;

      void Restart() {
        *this = GameState {};

        for (auto & item : researchItems) { item.level = 0; }
        mobGroup   = ld::MobGroup::Initialize();
        mineChasm  = ld::MineChasm::Initialize(mobGroup);
        minerGroup = ld::MinerGroup::Initialize();
      }

      // Upgrade related max values
      int32_t MaxFood() const {
        size_t idx = static_cast<size_t>(ld::ResearchType::Food);
        return 500 + researchItems[idx].level*1000;
      }

      uint32_t MaxFoodEatTimer() const {
        size_t idx = static_cast<size_t>(ld::ResearchType::Food);
        return 60 * (5 + researchItems[idx].level);
      }

      uint32_t MaxMinerSpeed() const {
        size_t idx = static_cast<size_t>(ld::ResearchType::Speed);
        return 1 + researchItems[idx].level;
      }

      uint32_t MaxCargoCapacity() const {
        size_t idx = static_cast<size_t>(ld::ResearchType::Cargo);
        return 5 + 5*researchItems[idx].level;
      }

      uint32_t FoodToEnergyRatio() const {
        size_t idx = static_cast<size_t>(ld::ResearchType::Food);
        return 100 + 10*researchItems[idx].level;
      }

      bool HasArmorUpgrade() const {
        size_t idx = static_cast<size_t>(ld::ResearchType::Armor);
        return researchItems[idx].level > 0;
      }

      std::array<ld::ResearchItem, Idx(ld::ResearchType::Size)> researchItems = {{
        { .type = ld::ResearchType::Pickaxe, .level = 0, .name = "Pickaxe"},
        { .type = ld::ResearchType::Armor  , .level = 0, .name = "Armor"  },
        { .type = ld::ResearchType::Speed  , .level = 0, .name = "Speed"  },
        { .type = ld::ResearchType::Food   , .level = 0, .name = "Food"   },
        { .type = ld::ResearchType::Cargo  , .level = 0, .name = "Cargo"  },
        { .type = ld::ResearchType::Vision , .level = 0, .name = "Vision" },
      }};

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
