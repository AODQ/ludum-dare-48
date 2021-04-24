#include <sounds.hpp>

#include <enum.hpp>

#include <raylib.h>

#include <array>

namespace { std::array<::Sound, Idx(ld::SoundType::Size)> sounds; }

::Music stream = LoadMusicStream("resources/audio.xm");

void ld::SoundInitialize()
{
  ::InitAudioDevice();

  ::SetMasterVolume(100.0f);

  TraceLog(LOG_INFO, "audio ready? %d", ::IsAudioDeviceReady());


  sounds[Idx(ld::SoundType::RockHit)] =
    ::LoadSound("resources/rock-break1.ogg");

  stream = ::LoadMusicStream("resources/audio.xm");
  ::PlayMusicStream(stream);
}

void ld::SoundShutdown()
{
  // TODO
  ::CloseAudioDevice();
}

void ld::SoundUpdate()
{
  ::UpdateMusicStream(stream);
}

void ld::SoundPlay(ld::SoundType const type, float const volume)
{
  /* ::PlaySoundMulti(sounds[Idx(type)]); */
  ::PlaySound(sounds[Idx(type)]);
}
