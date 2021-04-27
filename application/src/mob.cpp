#include <mob.hpp>

#include <gamestate.hpp>
#include <pathfinder.hpp>
#include <sounds.hpp>

#include <algorithm>
#include <set>

namespace {
  constexpr uint32_t cloudSpreadFrame = 15 * 60;
  uint32_t cloudSpreadTimer = cloudSpreadFrame;


template <typename T>
void moveTowards(T & thing, int32_t x, int32_t y)
{
  thing.positionX -= ld::sgn(thing.positionX - x);
  thing.positionY -= ld::sgn(thing.positionY - y);
}

} // -- namespace

void ld::MobGroup::Update(ld::GameState & state)
{
  for (
      int32_t slimeIdx = 0;
      slimeIdx < static_cast<int32_t>(state.mobGroup.slimes.size());
      ++ slimeIdx
  ) {

    auto & slime = state.mobGroup.slimes[slimeIdx];

    // kill
    if (slime.health <= 0 && slime.dieTimer < 0) {
      slime.dieTimer = 120;
      ld::SoundPlay(
        ld::SoundType::SlimeDie,
        std::abs(state.camera.y - slime.positionY) * 0.25f // more audible
      );

      // check if still attacking miner
      int32_t minerIdx = -1;
      for (
        ; slime.chasingMinerId >= 0
      && minerIdx < (int32_t)state.minerGroup.miners.size()
        ; ++ minerIdx
      ) {
        if (state.minerGroup.miners[minerIdx].minerId == slime.chasingMinerId)
          { break; }
      }

      // force to -1 to avoid size comparisons
      if (minerIdx >= 0 && minerIdx < (int32_t)state.minerGroup.miners.size()) {
        state.minerGroup.miners[minerIdx].AddUnit(ld::ValuableType::Food, 5);
        state.minerGroup.miners[minerIdx].resetToTraversal();
        slime.chasingMinerId = -1;
      }
    }

    // dying :(
    if (slime.health <= 0 && slime.dieTimer >= 0) {
      slime.dieTimer -= 1;
      slime.alpha = (slime.dieTimer) / 120.0f;

      if (slime.dieTimer <= 0) {
        state.mobGroup.slimes.erase(state.mobGroup.slimes.begin() + slimeIdx);
        -- slimeIdx;
      }
      continue;
    }

    if (slime.sleepTimer > 0) {
      slime.sleepTimer -= 1;

      // allow slime to come to rest
      if (slime.animationIdx > 10) {
        slime.animationIdx = (slime.animationIdx + 5) % (60*5);
      } else {
        slime.animationIdx = 0;
      }
      continue;
    }

    // look for tile near

    if (
        slime.chasingMinerId >= 0
     && slime.pathIdx >= slime.pathSize
     && slime.pathSize != 0
    ) {
      slime.pathIdx = 0;
      slime.pathSize = 0;
      slime.sleepTimer = ::GetRandomValue(0, 20);
      continue;
    }

    if (
        slime.attackTimer < 0 && slime.chasingMinerId < 0
     && slime.pathSize == 0
     && state.minerGroup.miners.size() > 0
    ) {
      uint32_t minerIdx =
        ::GetRandomValue(0, state.minerGroup.miners.size()-1);
      auto const & miner = state.minerGroup.miners[minerIdx];

      slime.pathSize = 0;
      slime.pathIdx = 0;

      // dont even bother if distance is too large
      if (
          std::abs(slime.positionX - miner.xPosition)/32.0f
        + std::abs(slime.positionY - miner.yPosition)/32.0f
        <= 10.0f
      ) {
        slime.pathSize = 0;
        ld::pathFind(
          state,
          slime.path, slime.pathSize,
          slime.positionX/32, slime.positionY/32,
          miner.xPosition/32, miner.yPosition/32,
          nullptr // cannot mine
        );
      }

      if (slime.pathSize == 0) {
        slime.attackTimer = 3;
      } else {
        slime.chasingMinerId = miner.minerId;
      }
    }

    ld::Miner * attackingMiner = nullptr;
    if (slime.chasingMinerId >= 0 && slime.pathSize == 0)
    {
      // find miner, in O(N) but this wont be called often enough to matter
      int32_t minerIdx = -1;
      for (; minerIdx < (int32_t)state.minerGroup.miners.size(); ++ minerIdx) {
        if (state.minerGroup.miners[minerIdx].minerId == slime.chasingMinerId)
          { break; }
      }

      // force to -1 to avoid size comparisons
      if (minerIdx == (int32_t)state.minerGroup.miners.size())
        minerIdx = -1;

      // can either lose focus if miner has died (minerId cant be found),
      // small random chance, or if miner gets too far away
      bool const breakChance = ::GetRandomValue(0, 100) < 5;

      attackingMiner =
        (minerIdx == -1)
          ? nullptr : &state.minerGroup.miners[slime.chasingMinerId]
      ;

      if (
          !attackingMiner
       || attackingMiner->aiState == ld::Miner::AiState::Dying
       || (!slime.inCombat && breakChance)
       || attackingMiner->aiState == ld::Miner::AiState::Surfaced
       || (
              std::abs(slime.positionX - attackingMiner->xPosition)/32.0f
            + std::abs(slime.positionY - attackingMiner->yPosition)/32.0f
            > 12
          )
      ) {
        slime.chasingMinerId = -1;

        if (attackingMiner && slime.inCombat) {
          attackingMiner->resetToTraversal();
          slime.inCombat = false;
        }
        attackingMiner = nullptr;
      } else if(attackingMiner && !slime.inCombat) /* not really needed lol */ {
        slime.pathSize = 0;
        ld::pathFind(
          state,
          slime.path, slime.pathSize,
          slime.positionX/32, slime.positionY/32,
          attackingMiner->xPosition/32, attackingMiner->yPosition/32,
          nullptr // cannot mine
        );
      }
    }

    if (slime.inCombat && !attackingMiner) {
      slime.inCombat = false;
    }

    if (slime.inCombat) {
      slime.animationIdx = (slime.animationIdx + 5) % (60*5);

      auto & miner = *attackingMiner;

      // force into combat state (in case other slimes set this to traversal)
      miner.aiState = ld::Miner::AiState::Fighting;

      if ((slime.animationIdx + 5) % (60 * 5) < slime.animationIdx) {
        miner.reduceEnergy(
          miner.inventory[Idx(ld::ItemType::Armor)].owns
            ? 20 : 100
        );
        miner.damageEquipment(ld::ItemType::Armor);
      }

      if (
          miner.animationFinishesThisFrame()
       && !miner.aiStateInternal.fighting.hasSwung
      ) {
        miner.aiStateInternal.fighting.hasSwung = true;
        slime.health -= 1;

        if (slime.health <= 0) {
          miner.AddUnit(ld::ValuableType::Food, 5);
          slime.animationIdx = 0;
        }

        ld::SoundPlay(
          ld::SoundType::Slime,
          (state.camera.y - slime.positionY) * 0.25f // more audible
        );

        // always break off
        slime.chasingMinerId = -1;
        slime.inCombat = false;
        attackingMiner->resetToTraversal();
      }

      // force miner to face this direction
      miner.prevXPosition =
          slime.positionX > miner.xPosition
        ? miner.xPosition-1 : miner.xPosition+1
      ;
      miner.prevYPosition =
          slime.positionY > miner.yPosition
        ? miner.yPosition-1 : miner.yPosition+1
      ;

      continue;

    } else if (
        attackingMiner
     && attackingMiner->aiState != ld::Miner::AiState::Dying
     && ::CheckCollisionPointCircle(
          ::Vector2 {
            (float)attackingMiner->xPosition,
            (float)attackingMiner->yPosition,
          },
          ::Vector2 {
            (float)slime.positionX,
            (float)slime.positionY,
          },
          48.0f
        )
    ) {
      slime.inCombat = true;
      attackingMiner->aiState = ld::Miner::AiState::Fighting;
    }

    if (slime.attackTimer >= 0)
      slime.attackTimer -= 1;

    if (slime.pathSize > 0) {
      if (slime.pathIdx >= slime.pathSize) {
        slime.pathIdx = 0;
        slime.pathSize = 0;
        continue;
      }

      auto & path = slime.path[slime.pathIdx];
      if (slime.targetTileX == -1 && slime.targetTileY == -1) {
        slime.targetTileX = path.x*32.0f + ::GetRandomValue(0, 16);
        slime.targetTileY = path.y*32.0f + ::GetRandomValue(0, 16);
      }
    }

    struct Offsets {
      int32_t x, y;
    };

    constexpr std::array<Offsets, 4> offsets = {{
      { -1, 0 }, { +1, 0 }, {  0, +1 }, {  0, -1 },
    }};

    auto offsetCheck = ::GetRandomValue(0, 3);
    if (slime.targetTileX == -1 && slime.targetTileY == -1)
    {
      auto conformanceX =
        state.mineChasm.limitX(slime.positionX/32 + offsets[offsetCheck].x);
      auto conformanceY =
        state.mineChasm.limitY(slime.positionY/32 + offsets[offsetCheck].y);
      if (state.mineChasm.rock(conformanceX, conformanceY).isMined()) {
        slime.targetTileX =
          ((slime.positionX/32) + offsets[offsetCheck].x)*32
        + ::GetRandomValue(0, 16)
        ;
        slime.targetTileY =
          ((slime.positionY/32) + offsets[offsetCheck].y)*32
        + ::GetRandomValue(0, 16)
        ;
      }
    }

    if (slime.targetTileX < 0 && slime.targetTileY < 0) { continue; }

    if (slime.targetTileX > 30*32-16)
      slime.targetTileX = 30*32-16;

    for (int i = 0; i < (slime.chasingMinerId == -1 ? 1 : 2); ++ i) {
      moveTowards(slime, slime.targetTileX, slime.targetTileY);
    }

    slime.animationIdx = (slime.animationIdx + 5) % (60*5);

    if (
        slime.positionX == slime.targetTileX
     && slime.positionY == slime.targetTileY
    ) {
      if (::GetRandomValue(0, 100) < (slime.chasingMinerId>=0 ? 5 : 20)) {
        slime.sleepTimer =
          slime.chasingMinerId < 0 ? ::GetRandomValue(1, 3)*60 : 5;
      }

      if (slime.pathSize > 0)
        slime.pathIdx += 1;

      slime.targetTileX = -1;
      slime.targetTileY = -1;
    }

    // TODO add to FOW if this is detected
  }

  // -- tnt ----------------------------------------------
  for (size_t it = 0; it < state.mobGroup.tnts.size(); ++ it) {
    auto & tnt = state.mobGroup.tnts[it];

    if (state.mineChasm.fowU8(tnt) <= 20) { continue; }

    if (tnt.animationIdx >= 0) {
      tnt.animationIdx += 1;

      if (tnt.animationIdx >= 5*30) {
        state.mobGroup.tnts.erase(state.mobGroup.tnts.begin() + it);
        -- it;
        continue;
      }

      struct Offsets {
        int32_t x, y;
      };

      constexpr std::array<Offsets, 13> offsets = {{
        {  0, 0 },
        { -1, 0 }, { -1, 0 }, { +1, 0 }, {  0, +1 },
        { -2, 0 }, { -2, 0 }, { +2, 0 }, {  0, +2 },
        { -1, -1 }, { -1, +1 }, { +1, -1 }, {  +1, +1 },
      }};

      for (auto & offset : offsets) {
        auto conformanceX = state.mineChasm.limitX(tnt.positionX/32) + offset.x;
        auto conformanceY = state.mineChasm.limitY(tnt.positionY/32) + offset.y;
        state.mineChasm.rockFow[
          state.mineChasm.rockId(conformanceX, conformanceY)
        ] = 1.0f;
      }


      // BOOM
      if (tnt.animationIdx != 30*4) { continue; }

      ld::SoundPlay(
        ld::SoundType::Explosion,
        (tnt.positionY - state.camera.y) * 0.05f
      );

      for (auto & miner : state.minerGroup.miners) {
        if (
          ::CheckCollisionPointCircle(
            ::Vector2 {
              (float)miner.xPosition,
              (float)miner.yPosition,
            },
            ::Vector2 {
              (float)tnt.positionX,
              (float)tnt.positionY,
            },
            64.0f
          )
        ) {
          if (miner.inventory[Idx(ld::ItemType::Armor)].owns) {
            miner.inventory[Idx(ld::ItemType::Armor)].owns = false;
            miner.inventory[Idx(ld::ItemType::Armor)].level = 0;
            miner.inventory[Idx(ld::ItemType::Armor)].durability = 0;
          } else {
            miner.kill();
          }
        }
      }

      for (auto & tntChain : state.mobGroup.tnts) {
        if (&tnt == &tntChain) { continue; }
        if (
            tntChain.animationIdx < 0
        &&  ::CheckCollisionPointCircle(
              ::Vector2 {
                (float)tnt.positionX,
                (float)tnt.positionY,
              },
              ::Vector2 {
                (float)tntChain.positionX,
                (float)tntChain.positionY,
              },
              64.0f
          )
        ) {
          tntChain.animationIdx = 30*2;
        }
      }

      for (auto & slime : state.mobGroup.slimes) {
        if (
          ::CheckCollisionPointCircle(
            ::Vector2 {
              (float)slime.positionX,
              (float)slime.positionY,
            },
            ::Vector2 {
              (float)tnt.positionX,
              (float)tnt.positionY,
            },
            64.0f
          )
        ) {
          // this should hopefully let slime detach itself from
          // miners on its next frame
          slime.health = 0;
          -- it;
        }
      }

      for (auto & offset : offsets) {
        auto conformanceX = state.mineChasm.limitX(tnt.positionX/32 + offset.x);
        auto conformanceY = state.mineChasm.limitY(tnt.positionY/32 + offset.y);
        state.mineChasm.rock(conformanceX, conformanceY).receiveDamage(300);
      }
      continue;
    }

    // if mob steps on it
    for (auto & miner : state.minerGroup.miners) {
      if (
        ::CheckCollisionPointCircle(
          ::Vector2 {
            (float)miner.xPosition,
            (float)miner.yPosition,
          },
          ::Vector2 {
            (float)tnt.positionX,
            (float)tnt.positionY,
          },
          48.0f
        )
      ) {
        tnt.animationIdx = 0;
      }
    }

    for (auto & slime : state.mobGroup.slimes) {
      if (
        ::CheckCollisionPointCircle(
          ::Vector2 {
            (float)slime.positionX,
            (float)slime.positionY,
          },
          ::Vector2 {
            (float)tnt.positionX,
            (float)tnt.positionY,
          },
          48.0f
        )
      ) {
        tnt.animationIdx = 0;
      }
    }
  }

  // -- clouds -------------------------------------------

  const auto spread = --cloudSpreadTimer == 0;
  if (spread) cloudSpreadTimer = cloudSpreadFrame;

  std::vector<MobPoisonCloud> toAdd = {};
  for (int32_t it = 0; it < (int32_t)state.mobGroup.poisonClouds.size(); ++ it) {
    auto & cloud = state.mobGroup.poisonClouds[it];
    cloud.animationIdx = (cloud.animationIdx + 5) % (60*2);

    // odd
    if (!cloud.numInstances) {
      state.mobGroup.poisonClouds.erase(
        state.mobGroup.poisonClouds.begin() + it
      );
      state.mineChasm.isPoisoned(cloud.positionX/32, cloud.positionY/32) = 0.0f;
      -- it;
      continue;
    }

    cloud.potency =
      std::clamp(
        0.2f + ((16 - *cloud.numInstances) / 16.0f)*0.8f,
        0.2f, 1.0f
      );
    state.mineChasm.isPoisoned(
      cloud.positionX/32, cloud.positionY/32
    ) = cloud.potency;

    // spread only if fully visible, has been activated & on spread timer
    if (spread && (cloud.activated || state.mineChasm.fowU8(cloud) > 80)) {
      cloud.activated = true;
    } else {
      continue; // iknow i know wtf is this
    }

    if (*cloud.numInstances >= 16) {
      state.mobGroup.poisonClouds.erase(
        state.mobGroup.poisonClouds.begin() + it
      );
      state.mineChasm.isPoisoned(cloud.positionX/32, cloud.positionY/32) = 0.0f;
      -- it;
      continue;
    }

    int32_t nIdx = 0;
    for (auto [rowOff, colOff] : {
      std::tuple
      { 1,  0},
      { 0,  1},
      {-1,  0},
      { 0, -1},
    })
    {
      ++ nIdx ; // fuuuuuuuuuuuck c++ not having iters for ranges

      auto col = (cloud.positionX + colOff*32)/32;
      auto row = (cloud.positionY + rowOff*32)/32;

      if (col < 0 || row < 0 || row >= 250 || col >= 30) continue;

      const auto& rock = state.mineChasm.rock(col, row);

      if (
        rock.tier != ld::RockTier::Mined
        || state.mineChasm.isPoisoned(col, row) > 0.0f
      ) {
        continue;
      }

      state.mineChasm.isPoisoned(col, row) = 0.5f;
      toAdd.push_back({
        .positionX = static_cast<int32_t>(cloud.positionX + colOff*32),
        .positionY = static_cast<int32_t>(cloud.positionY + rowOff*32),
        .numInstances = cloud.numInstances
      });
      (*cloud.numInstances) += 1;
    }
  }

  for (auto & add : toAdd)
    state.mobGroup.poisonClouds.push_back(add);
}

ld::MobGroup ld::MobGroup::Initialize()
{
  MobGroup self;

  return self;
}
