
#pragma once

#include <mine.hpp>


// fwd
struct Texture;
using Texture2D = Texture;
namespace ld { struct MineRenderer; }
namespace ld { struct Camera; }

namespace ld {

  // -- textures

  enum class TextureType {
    Rock,
    Size,
  };

  ::Texture2D const & TextureGet(ld::TextureType const);

  // -- mine renderer
  void RenderScene(
    ld::MineChasm const & mineChasm
  , ld::Camera & camera
  );

  // -- initialize/shutdown
  void RenderInitialize();
  void RenderShutdown();
}

