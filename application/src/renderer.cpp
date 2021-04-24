
#include <camera.hpp>
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
  uint32_t shaderChasm_locationRockTypeTier;
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

        uniform mat4 mvp;

        out vec2 fragTexCoord;

        const vec2 uvCoords[4] = vec2[] (
          vec2(0.0, 1.0)
        , vec2(0.0, 0.0)
        , vec2(1.0, 0.0)
        , vec2(1.0, 1.0)
        );

        void main() {
          fragTexCoord = uvCoords[gl_VertexID % 4];
          fragTexCoord.y = 1.0f - fragTexCoord.y;

          gl_Position = mvp * vec4(vertexPosition, 1.0f);
        }
      ),
      LD_SHADER(

        precision mediump float;

        uniform sampler2D samplerRock;
        uniform ivec2 unfRockTypeTier;

        in vec2 fragTexCoord;

        out vec4 finalColor;

        const ivec2 samplerRockDimensions = ivec2(4, 4);

        void main() {
          vec2 offset = vec2(unfRockTypeTier) / vec2(samplerRockDimensions);
          const vec2 samplerInvDim = vec2(1.0f) / vec2(samplerRockDimensions);

          vec4 texelColor =
            texture(samplerRock, offset + samplerInvDim*fragTexCoord);

          finalColor = texelColor;
        }
      )
    );

  shaderChasm_locationRockTexture =
    GetShaderLocation(shaderChasm, "samplerRock");

  shaderChasm_locationRockTypeTier =
    GetShaderLocation(shaderChasm, "unfRockTypeTier");
}


void ld::RenderShutdown()
{
  // TODO cleanup
}

void ld::RenderScene(
  ld::MineChasm const & chasm
, ld::Camera & camera
) {
  std::array<int32_t, 2> rockTypeTier;

  for (size_t it = 0; it < chasm.rocks.size(); ++ it) {
    auto const & rock = chasm.rocks[it];
    int32_t x = it % chasm.columns, y = it / chasm.columns;

    rockTypeTier[0] = static_cast<float>(rock.type);
    rockTypeTier[1] = static_cast<float>(rock.tier);

    ::DrawTextureRec(
      ld::TextureGet(ld::TextureType::Rock)
    , ::Rectangle {
        .x = static_cast<float>(rock.type) * 32.0f,
        .y = static_cast<float>(rock.tier) * 32.0f,
        .width = 32.0f,
        .height = 32.0f,
      }
    , ::Vector2{x*32.0f, y*32.0f - camera.y}
    , Color { 255, 255, 255, 255 }
    );
  }
}
