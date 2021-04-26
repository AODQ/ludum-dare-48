
#pragma once

#include <button.hpp>

// fwd
namespace ld { struct GameState; }
struct Texture;
using Texture2D = Texture;

namespace ld {

  // -- textures

  enum class TextureType {
    Rock,
    Gem,
    Miner,
    SurfacedFg,
    Misc,
    RockDamage,
    Cargo,
    Upgrades,
    Flag,
    Size,
  };

  ::Texture2D const & TextureGet(ld::TextureType const);

  // -- mine renderer
  void RenderScene(ld::GameState const & state);

  void RenderOverlay(
    ld::ButtonGroup const & buttonGroup
  );

  // -- initialize/shutdown
  void RenderInitialize();
  void RenderShutdown();
}

