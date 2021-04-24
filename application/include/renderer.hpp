
#pragma once

#include <mine.hpp>
#include <button.hpp>

// fwd
struct Texture;
using Texture2D = Texture;
namespace ld { struct Camera; }
namespace ld { struct MineRenderer; }
namespace ld { struct MinerGroup; }
namespace ld { struct Button; }

namespace ld {

  // -- textures

  enum class TextureType {
    Rock,
    Miner,
    SurfacedFg,
    Size,
  };

  ::Texture2D const & TextureGet(ld::TextureType const);

  // -- mine renderer
  void RenderScene(
    ld::MineChasm const & mineChasm
  , ld::MinerGroup const & minerGroup
  , ld::Camera const & camera
  );

  void RenderOverlay(
    ld::ButtonGroup const & buttonGroup
  );

  // -- initialize/shutdown
  void RenderInitialize();
  void RenderShutdown();
}

