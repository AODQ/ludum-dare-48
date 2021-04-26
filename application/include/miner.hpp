#pragma once

#include <enum.hpp>
#include <items.hpp>

#include <raylib.h>

#include <array>
#include <string>
#include <vector>

namespace ld { struct GameState; }

namespace ld {

  int32_t PickLevelDamage(int32_t level);

  struct Miner {
      enum class PurchaseState {
        BuyHighest,
        Conserve,
        Size,
      } purchaseState;

      // Keep running track of how much miner contributed so that
      // he will purchase only what he's contributed
      uint32_t netValue = 0;

      int32_t minerId;
      // position is in texels, not tiles
      int32_t xPosition = 0, yPosition = 0;
      int32_t prevXPosition = 0, prevYPosition = 0;
      uint8_t alpha = 255;
      int32_t maxEnergy = 1'000;
      int32_t energy = 500;
      int32_t foodToEnergyRatio = 100;
      std::array<ld::Item, Idx(ld::ItemType::Size)> inventory = {{
        { .type = ld::ItemType::Pickaxe, .owns = false, .durability = 10, .level = 0 },
        { .type = ld::ItemType::Armor,   .owns = false, .durability = 10, .level = 0 },
        { .type = ld::ItemType::Speed,   .owns = false, .durability = 10, .level = 0 },
      }};

      uint32_t cargoCapacity = 50u;
      uint32_t currentCargoCapacity = 0u;

      void damageEquipment(ld::ItemType type);

      bool wantsToSurface() const {
        return energy <= (currentCargoCapacity > 0 ? (100 + yPosition/5) : 20);
      }

      void moveTowards(int32_t x, int32_t y);

      int32_t animationFrames() const;
      bool animationFinishesThisFrame();
      void reduceEnergy(int32_t units);
      void kill();

      std::array<ld::Valuable, Idx(ld::ValuableType::Size)> cargo = {{
        {
          .type = ld::ValuableType::Stone,
          .ownedUnits = 0,
        }, {
          .type = ld::ValuableType::Food,
          .ownedUnits = 0,
        }, {
          .type = ld::ValuableType::Tin,
          .ownedUnits = 0,
        }, {
          .type = ld::ValuableType::Ruby,
          .ownedUnits = 0,
        }, {
          .type = ld::ValuableType::Emerald,
          .ownedUnits = 0,
        }, {
          .type = ld::ValuableType::Sapphire,
          .ownedUnits = 0,
        },
      }};

      // this syncs with the miner.png file vertically
      enum class AnimationState {
        Travelling,
        Mining,
        Fighting,
        Idling,
        Dying,
      };

      AnimationState animationState = AnimationState::Idling;

      // this syncs with the miner.png file horizontally, scaled by 60Hz
      int32_t animationIdx = 0;

      void applyAnimationState(ld::Miner::AnimationState const state) {
        auto & self = *this;
        if (self.animationState != state) {
          self.animationState = state;
          self.animationIdx = 0;
        }
      }

      enum class AiState {
        Attacking,
        Dying,
        Fighting,
        Idling,
        Inventorying,
        Mining,
        Surfaced,
        Traversing,
      };

      enum class AiStateSurfaced {
        Surfacing,
        MovingToBase,
        DumpingMaterial,
        PurchasingUpgrades,
        BoughtFromMine,
        BackToMine,
      };

      void chooseNewTarget(ld::GameState & state);
      void surfaceMiner();

      // struct, not a union, in order to preserve memory to chain commands
      // like mine -> mineTraversing
      struct AiStateInternal {
        struct Mining {
          int32_t targetRockId = 0;
        };

        struct Attacking {
          int32_t targetEnemyId = 0;
        };

        struct Traversing {
          std::array<::Vector2, 4> path;
          size_t pathIdx;
          size_t pathSize;

          int32_t targetTileX = 0, targetTileY = 0;
          int32_t targetPosOffX = 0, targetPosOffY = 0;

          bool wantsToSurface = false;
        };

        struct Idling {
          int32_t waitTimer = -1;
        };

        struct Inventorying {
          int32_t waitTimer = -1;
        };

        struct Dying {
        };

        struct Fighting {
          bool hasSwung = false; // to allow only 1 slime hit in multi combat
        };

        struct Surfaced {
          AiStateSurfaced state = AiStateSurfaced::Surfacing;

          int32_t waitTimer = -1;
          int32_t targetX, targetY;
          bool hasPurchasedFood = false;
        };

        Attacking      attacking;
        Inventorying   inventorying;
        Dying          dying;
        Idling         idling;
        Fighting       fighting;
        Mining         mining;
        Surfaced       surfaced;
        Traversing     traversing;
      };

      AiState aiState;
      AiStateInternal aiStateInternal;

      void resetToTraversal() {
        aiState = ld::Miner::AiState::Traversing;
        aiStateInternal.traversing.pathIdx = 0;
        aiStateInternal.traversing.pathSize = 0;
      }

      bool isSurfaced = true;
  };

  struct MinerGroup {
      std::vector<ld::Miner> miners;

      static MinerGroup Initialize();

      static void Update(ld::GameState & state);

      void addMiner();
  };
}
