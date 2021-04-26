#include <renderer.hpp>

#include <enum.hpp>
#include <gamestate.hpp>

#include <raylib.h>

#include <array>

#define LD_SHADER(...) \
  "#version 300 es\n" \
  #__VA_ARGS__

namespace { std::array<::Texture2D, Idx(ld::TextureType::Size)> textures; }
namespace {
}

::Texture2D const & ld::TextureGet(ld::TextureType const type)
{
  return textures[Idx(type)];
}

void ld::RenderInitialize()
{
  // -- load textures
  textures[Idx(ld::TextureType::Rock)] = ::LoadTexture("resources/rocks.png");
  textures[Idx(ld::TextureType::Gem)] = ::LoadTexture("resources/gems.png");
  textures[Idx(ld::TextureType::Miner)] = ::LoadTexture("resources/miner.png");
  textures[Idx(ld::TextureType::SurfacedFg)] =
    ::LoadTexture("resources/surfaced-foreground-new.png");
  textures[Idx(ld::TextureType::Misc)] =
    ::LoadTexture("resources/misc.png");
  textures[Idx(ld::TextureType::RockDamage)] =
    ::LoadTexture("resources/rock-damage.png");
  textures[Idx(ld::TextureType::Cargo)] =
    ::LoadTexture("resources/cargo.png");
  textures[Idx(ld::TextureType::Upgrades)] =
    ::LoadTexture("resources/upgrades.png");
  textures[Idx(ld::TextureType::Flag)] = ::LoadTexture("resources/flag.png");

  // -- load shaders
}

void ld::RenderShutdown()
{
  // TODO cleanup
}

void ld::RenderOverlay(
  ld::ButtonGroup const & buttonGroup
) {
    for (auto pair : buttonGroup)
    {
        pair.second.Draw(pair.first.c_str());
    }
}

void ld::RenderScene(ld::GameState const & state)
{
  if (state.camera.y < 0.0f)
  { // -- render surface
    ::DrawTextureV(
      ld::TextureGet(ld::TextureType::SurfacedFg)
    , ::Vector2{0.0f, -600.0f - state.camera.y}
    , Color { 255, 255, 255, 255 }
    );
  }

  { // -- render foreground rocks & gems

    int32_t begIt = std::max((state.camera.y + 0.0f)   / 32.0f, 0.0f);
    int32_t endIt = std::max((state.camera.y + 650.0f) / 32.0f, 0.0f);

    for (int32_t it = begIt*30; it < endIt*30; ++ it) {
      if (it >= static_cast<int32_t>(state.mineChasm.rocks.size())) break;
      auto const & rock = state.mineChasm.rocks[it];

      int32_t
        x = it % state.mineChasm.columns,
        y = it / state.mineChasm.columns
      ;

      uint8_t fow =
        static_cast<int8_t>(
          std::clamp(
            state.mineChasm.rockFow[it]*255.0f,
            0.0f,
            255.0f
          )
        );

      ::DrawTextureRec(
        ld::TextureGet(ld::TextureType::Rock)
      , ::Rectangle {
          .x = static_cast<float>(rock.tier) * 32.0f,
          .y = static_cast<float>(rock.type) * 32.0f,
          .width = 32.0f,
          .height = 32.0f,
        }
      , ::Vector2{x*32.0f, y*32.0f - state.camera.y}
      , { fow, fow, fow, 255 }
      );

      if (!rock.isMined() && rock.gem != ld::RockGemType::Empty && fow>=40) {

        constexpr std::array<::Vector2, Idx(ld::RockGemType::Size)> offsets = {
          ::Vector2 { 0, 0, }, // none
          ::Vector2 { 0, 0, }, // tin
          ::Vector2 { 0, 1, }, // ruby
          ::Vector2 { 1, 0, }, // emerald
          ::Vector2 { 1, 1, }, // sapphire
        };

        ::DrawTextureRec(
          ld::TextureGet(ld::TextureType::Gem)
        , ::Rectangle {
            .x = offsets[Idx(rock.gem)].x * 32.0f,
            .y = offsets[Idx(rock.gem)].y * 32.0f,
            .width = 32.0f,
            .height = 32.0f,
          }
        , ::Vector2{x*32.0f, y*32.0f - state.camera.y}
        , Color { fow, fow, fow, fow < (uint8_t)(40) ? (uint8_t)(0) : fow }
        );
      }

      if (!rock.isMined() && rock.durability != rock.baseDurability && fow>=40){
        ::DrawTextureRec(
          ld::TextureGet(ld::TextureType::RockDamage)
        , ::Rectangle {
            .x =
              static_cast<int32_t>(
                  4.0f
                * (1.0f - (
                     rock.durability / static_cast<float>(rock.baseDurability)
                  ))
              ) * 32.0f,
            .y = 0.0f,
            .width = 32.0f,
            .height = 32.0f,
          }
        , ::Vector2{x*32.0f, y*32.0f - state.camera.y}
        , Color { fow, fow, fow, fow < (uint8_t)(40) ? (uint8_t)(0) : fow }
        );
      }
    }
  }

  { // -- render mobs
    for (auto & slime : state.mobGroup.slimes) {

      uint8_t const fow = state.mineChasm.fowU8(slime);

      if (
          slime.positionY+8.0f < state.camera.y
       || slime.positionY-8.0f > state.camera.y+600
       || fow < 40
      ) {
        continue;
      }

      constexpr std::array<::Vector2, Idx(ld::RockGemType::Size)> offsets = {
        ::Vector2 { 4, 0, },
        ::Vector2 { 4, 1, },
        ::Vector2 { 4, 2, },
        ::Vector2 { 4, 1, },
        ::Vector2 { 4, 0, },
      };

      ::DrawTextureRec(
        ld::TextureGet(ld::TextureType::Miner)
      , ::Rectangle {
          .x = offsets[slime.animationIdx / 60].x * 16.0f,
          .y = offsets[slime.animationIdx / 60].y * 16.0f,
          .width = 16.0f,
          .height = 16.0f,
        }
      , ::Vector2{
          static_cast<float>(slime.positionX),
          static_cast<float>(slime.positionY) - state.camera.y
        }
        , Color { fow, fow, fow, fow < (uint8_t)(40) ? (uint8_t)(0) : fow }
      );
    }

    for (auto & cloud : state.mobGroup.poisonClouds) {

      uint8_t const fow = state.mineChasm.fowU8(cloud);

      if (
          cloud.positionY+8.0f < state.camera.y
       || cloud.positionY-8.0f > state.camera.y+600
       || fow < 40
      ) {
        continue;
      }

      constexpr std::array<::Vector2, Idx(ld::RockGemType::Size)> offsets = {
        ::Vector2 { 4, 3, },
        ::Vector2 { 4, 4, },
      };

      ::DrawTextureRec(
        ld::TextureGet(ld::TextureType::Miner)
      , ::Rectangle {
          .x = offsets[cloud.animationIdx / 60].x * 16.0f,
          .y = offsets[cloud.animationIdx / 60].y * 16.0f,
          .width = 16.0f,
          .height = 16.0f,
        }
      , ::Vector2{
          static_cast<float>(cloud.positionX),
          static_cast<float>(cloud.positionY) - state.camera.y
        }
        , Color { fow, fow, fow, fow < (uint8_t)(40) ? (uint8_t)(0) : fow }
      );
    }
  }

  { // -- render miners
    for (auto & miner : state.minerGroup.miners) {

      if (
          miner.yPosition+8.0f < state.camera.y
       || miner.yPosition-8.0f > state.camera.y+600
      ) { continue; }

      ::DrawTextureRec(
        ld::TextureGet(ld::TextureType::Miner)
      , ::Rectangle {
          .x = static_cast<float>(miner.animationIdx / 60) * 16.0f,
          .y = static_cast<float>(miner.animationState) * 16.0f,
          .width = 16.0f - 32.0f*(miner.xPosition < miner.prevXPosition),
          .height = 16.0f,
        }
      , ::Vector2{
          static_cast<float>(miner.xPosition),
          static_cast<float>(miner.yPosition) - state.camera.y
        }
      , Color { 255, 255, 255, 255 }
      );

      // circle around highlighted miner
      if (miner.minerId == state.minerSelection) {
        ::DrawCircleLines(
          miner.xPosition + 8.0f,
          miner.yPosition + 8.0f - state.camera.y,
          9.0f,
          ::GREEN
        );

        if (miner.aiState == ld::Miner::AiState::Idling) {
          auto mousePos = ::GetMousePosition();
          ::DrawCircleV(
            ::Vector2{mousePos.x, mousePos.y},
            32.0f,
            { 128, 25, 25, 128 }
          );
        }

        if (
            miner.aiState == ld::Miner::AiState::Traversing
         || miner.aiState == ld::Miner::AiState::Mining
        ) {
          auto & aiState = miner.aiStateInternal.traversing;
          ::DrawCircleV(
            ::Vector2{
              static_cast<float>(aiState.targetTileX)*32.0f+16.0f,
              static_cast<float>(aiState.targetTileY)*32.0f+16.0f - state.camera.y
            },
            8.0f,
            { 25, 25, 80, 200 }
          );

          ::DrawCircleV(
            ::Vector2{
              aiState.path[aiState.pathIdx].x*32.0f+16.0f,
              aiState.path[aiState.pathIdx].y*32.0f+16.0f - state.camera.y
            },
            8.0f,
            { 25, 80, 25, 200 }
          );
        }
      }
    }
  }

  { // -- render notifs
    for (auto & notif : state.notifGroup.notifs) {
      constexpr std::array<::Vector2, Idx(ld::NotifType::Size)> offsets = {
        ::Vector2 { 2, 0, },
        ::Vector2 { 2, 1, },
        ::Vector2 { 1, 0, },
        ::Vector2 { 2, 1, },
        ::Vector2 { 2, 2, },
      };

      ::DrawTextureRec(
        ld::TextureGet(ld::TextureType::Misc)
      , ::Rectangle {
          .x = offsets[Idx(notif.type)].x * 32.0f,
          .y = offsets[Idx(notif.type)].y * 32.0f,
          .width = 32.0f,
          .height = 32.0f,
        }
      , ::Vector2{
          static_cast<float>(notif.positionX),
          static_cast<float>(notif.positionY) - state.camera.y
        }
      , { 255, 255, 255, static_cast<uint8_t>(notif.timer / 120.0f * 255) }
      );
    }
  }

  if (state.targetX >= 0 && state.targetY >= 0)
  { // -- render flag
    ::DrawTexturePro(
      ld::TextureGet(ld::TextureType::Flag)
    , ::Rectangle {
        .x = 0.0f,
        .y = 0.0f,
        .width = 32.0f,
        .height = 32.0f,
      }
    , ::Rectangle {
        .x = state.targetX*32.0f,
        .y = state.targetY*32.0f - state.camera.y,
        .width = 16.0f,
        .height = 16.0f,
      }
    , ::Vector2{0.0f, 0.0f}
    , 0.0f
    , { 255, 255, 255, 255 }
    );
  }

  if (state.targetActive)
  { // -- render flag for mouse
    ::DrawTexturePro(
      ld::TextureGet(ld::TextureType::Flag)
    , ::Rectangle {
        .x = 0.0f,
        .y = 0.0f,
        .width = 32.0f,
        .height = 32.0f,
      }
    , ::Rectangle {
        .x = ::GetMousePosition().x,
        .y = ::GetMousePosition().y,
        .width = 16.0f,
        .height = 16.0f,
      }
    , ::Vector2{0.0f, 0.0f}
    , 0.0f
    , { 255, 255, 255, 255 }
    );
  }
}
