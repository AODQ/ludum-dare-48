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
    ::LoadTexture("resources/surfaced-foreground.png");

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
  // render background TODO

  if (state.camera.y < 0.0f)
  { // -- render surface
    ::DrawTextureV(
      ld::TextureGet(ld::TextureType::SurfacedFg)
    , ::Vector2{0.0f, -612.0f - state.camera.y}
    , Color { 255, 255, 255, 255 }
    );
  }

  // render surfaced miners TODO

  { // -- render foreground rocks & gems

    // TODO only render in view instead of all

    float const limit = state.mineChasm.rocks.size() / 30;

    int32_t begIt = std::clamp((state.camera.y + 0.0f)   / 32.0f, 0.0f, limit);
    int32_t endIt = std::clamp((state.camera.y + 650.0f) / 32.0f, 0.0f, limit);

    for (int32_t it = begIt*30; it < endIt*30; ++ it) {
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

      if (!rock.isMined() && rock.gem != ld::RockGemType::Empty) {

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
        , Color { fow, fow, fow, 255 }
        );
      }
    }
  }

  // render gems TODO

  { // -- render miners
    // TODO optimize
    for (auto & miner : state.minerGroup.miners) {
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
            miner.aiState == ld::Miner::AiState::MineTraversing
         || miner.aiState == ld::Miner::AiState::Mining
        ) {
          auto & aiState = miner.aiStateInternal.mineTraversing;
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

  { // -- render mobs
    // TODO optimize
    for (auto & slime : state.mobGroup.slimes) {
      constexpr std::array<::Vector2, Idx(ld::RockGemType::Size)> offsets = {
        ::Vector2 { 4, 0, },
        ::Vector2 { 4, 1, },
        ::Vector2 { 4, 2, },
        ::Vector2 { 4, 1, },
        ::Vector2 { 4, 0, },
      };

      uint8_t const fow = state.mineChasm.fowU8(slime);

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
      , Color { fow, fow, fow, 255 }
      );
    }

    for (auto & cloud : state.mobGroup.poisonClouds) {
      constexpr std::array<::Vector2, Idx(ld::RockGemType::Size)> offsets = {
        ::Vector2 { 4, 3, },
        ::Vector2 { 4, 4, },
      };

      uint8_t const fow = state.mineChasm.fowU8(cloud);

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
      , Color { fow, fow, fow, 255 }
      );
    }
  }
}
