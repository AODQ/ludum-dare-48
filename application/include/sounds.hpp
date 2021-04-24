#pragma once

namespace ld {
  enum class SoundType {
    RockHit,
    Size,
  };


  void SoundInitialize();
  void SoundShutdown();

  void SoundUpdate();

  void SoundPlay(SoundType const, float const volume = 1.0f);
}
