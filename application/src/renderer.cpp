
#include <enum.hpp>
#include <renderer.hpp>

#include <raylib.h>

#include <array>

#define LD_SHADER(...) \
  "#version 300 es\n" \
  #__VA_ARGS__

namespace { std::array<::Texture2D, Idx(ld::TextureType::Size)> textures; }
namespace {
  ::Shader shaderChasm;

  uint32_t shaderChasm_locationRockTexture;
}

::Texture2D const & ld::TextureGet(ld::TextureType const type)
{
  return textures[Idx(type)];
}

void ld::RenderInitialize()
{
  // -- load textures
  textures[Idx(ld::TextureType::Rock)] = ::LoadTexture("resources/rocks.png");

  // -- load shaders
  shaderChasm =
    ::LoadShaderFromMemory(
      LD_SHADER(

        precision mediump float;

        in vec3 vertexPosition;
        in vec2 vertexTexCoord;

        out vec2 fragTexCoord;

        void main() {
          fragTexCoord = vertexTexCoord;
          gl_Position = vec4(vertexPosition, 1.0f);
        }
      ),
      LD_SHADER(

        precision mediump float;

        uniform sampler2D samplerRock;

        in vec2 fragTexCoord;

        out vec4 finalColor;

        void main() {
          vec4 texelColor = texture(samplerRock, fragTexCoord);

          finalColor = texelColor;
          /* finalColor = vec4(fragTexCoord, 0.0f, 1.0f); */
        }
      )
    );

  shaderChasm_locationRockTexture =
    GetShaderLocation(shaderChasm, "samplerRock");
}


void ld::RenderShutdown()
{
  // TODO cleanup
}

void ld::RenderScene(ld::MineChasm const & chasm)
{
  BeginShaderMode(shaderChasm);


  SetShaderValueTexture(
    shaderChasm
  , shaderChasm_locationRockTexture
  , ld::TextureGet(ld::TextureType::Rock)
  );

  for (size_t it = 0; it < chasm.rocks.size(); ++ it) {
    auto const & rock = chasm.rocks[it];
    (void) rock;
    int32_t x = it % chasm.columns, y = it / chasm.columns;

    ::DrawRectangleV(
      ::Vector2{x*32.0f, y*32.0f}
    , ::Vector2{32.0f, 32.0f}
    , ::Color{static_cast<uint8_t>(x), static_cast<uint8_t>(y*5), 50, 255}
    );
  }

  EndShaderMode();
}
