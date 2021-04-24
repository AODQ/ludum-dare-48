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

  // render surface TODO
  { // -- render chasm miners
    ::DrawTextureV(
      ld::TextureGet(ld::TextureType::SurfacedFg)
    , ::Vector2{0.0f, -612.0f - state.camera.y}
    , Color { 255, 255, 255, 255 }
    );
  }

  // render surfaced miners TODO

  { // -- render foreground rocks

    // TODO only render in view instead of all

    for (size_t it = 0; it < state.mineChasm.rocks.size(); ++ it) {
      auto const & rock = state.mineChasm.rocks[it];

      int32_t
        x = it % state.mineChasm.columns,
        y = it / state.mineChasm.columns
      ;

      ::DrawTextureRec(
        ld::TextureGet(ld::TextureType::Rock)
      , ::Rectangle {
          .x = static_cast<float>(rock.tier) * 32.0f,
          .y = static_cast<float>(rock.type) * 32.0f,
          .width = 32.0f,
          .height = 32.0f,
        }
      , ::Vector2{x*32.0f, y*32.0f - state.camera.y}
      , Color { 255, 255, 255, 255 }
      );
    }
  }

  // render gems TODO

  { // -- render miners
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
    }
  }

  // render enemies TODO
}
