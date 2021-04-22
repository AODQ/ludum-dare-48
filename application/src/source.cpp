/* pulcher | aodq.net */

#include <raylib.h>

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

struct ObjectId {
  uint32_t type = 0u, id = 0u;
};

bool IsSame(ObjectId const & x, ObjectId const & y) {
  return x.type == y.type && x.id == y.id;
}

bool IsSameType(ObjectId const & x, ObjectId const & y) {
  return x.type == y.type;
}

struct ObjectAllocator {
  std::vector<uint8_t> dataHeap;
  std::vector<ObjectId> objects;
  std::vector<uint32_t> freeHandles = {};
};

struct ObjectImplementation {
  ObjectAllocator objectStorage;

  std::function<
    void(
      uint8_t * const data
    , std::vector<ObjectId> const & objects
    )
  > updateAll = {};

  std::function<
    void(
      uint8_t const * const data
    , std::vector<ObjectId> const & objects
    )
  > renderAll = {};
};

struct Scene {
  std::vector<ObjectImplementation> objects;
};

#define assert(cnd) \
  if (!(cnd)) { \
    TraceLog( \
      LOG_ERROR, \
      "assertion fail: %s@%u: `%s`", __FILE__, __LINE__, #cnd \
    ); \
  }

template <typename T>
T & Scene_ObjectLookup(Scene & scene, ObjectId const & object)
{
  assert(static_cast<size_t>(T::type) < scene.objects.size());

  auto const & objectStorage =
    scene.objects[static_cast<size_t>(T::type)].objectStorage
  ;

  assert(IsSame(objectStorage.objects[object.id], object));

  T * internalData = reinterpret_cast<T *>(objectStorage.dataHeap.data());

  return internalData + object.id;
}

template <typename T>
ObjectId Scene_ObjectAdd(Scene & scene, T const & data)
{
  assert(static_cast<size_t>(T::type) < scene.objects.size());

  auto & objectStorage =
    scene.objects[static_cast<size_t>(T::type)].objectStorage
  ;

  T * objectDataToAllocate = nullptr;
  size_t objectHandleToFill = 0ul;

  // if there is an empty handle, write over its memory
  if (!objectStorage.freeHandles.empty()) {

    objectHandleToFill = objectStorage.freeHandles.back();
    objectStorage.freeHandles.pop_back();

    objectDataToAllocate =
      reinterpret_cast<T *>(objectStorage.dataHeap.data()) + objectHandleToFill
    ;
  }
  else { // otherwise 'allocate' memory for it

    objectHandleToFill = objectStorage.objects.size();
    objectStorage.objects.push_back({});

    for (size_t i = 0; i < sizeof(T); ++ i)
      objectStorage.dataHeap.emplace_back(0);

    objectDataToAllocate =
      reinterpret_cast<T *>(objectStorage.dataHeap.data()) + objectHandleToFill
    ;
  }

  assert(objectDataToAllocate);

  // write object & its data
  objectStorage.objects[objectHandleToFill] = {
    .type = T::type,
    .id = objectHandleToFill,
  };

  ::memcpy(
    objectDataToAllocate
  , reinterpret_cast<void const *>(&data)
  , sizeof(T)
  );

  return objectStorage.objects[objectHandleToFill];
}

template <typename T>
void Scene_ObjectImplementationAdd(
  Scene & scene
, ObjectImplementation const & impl
) {
  while (scene.objects.size() <= static_cast<size_t>(T::type))
    scene.objects.push_back({});

  scene.objects[static_cast<size_t>(T::type)] = impl;
}

void Scene_Update(Scene & scene)
{
  for (auto & objectImpl : scene.objects) {
    if (objectImpl.updateAll) {
      objectImpl.updateAll(
        reinterpret_cast<uint8_t *>(objectImpl.objectStorage.dataHeap.data())
      , objectImpl.objectStorage.objects
      );
    }
  }
}

void Scene_Render(Scene & scene)
{
  for (auto & objectImpl : scene.objects) {
    if (objectImpl.renderAll) {
      objectImpl.renderAll(
        reinterpret_cast<uint8_t const *>(
          objectImpl.objectStorage.dataHeap.data()
        )
      , objectImpl.objectStorage.objects
      );
    }
  }
}

struct Object_Circle {
  static constexpr size_t type = 2;

  uint32_t xPosition = 0, yPosition = 0;
  Color color;
};

struct Object_Rectangle {
  static constexpr size_t type = 0;

  uint32_t xPosition = 0, yPosition = 0;
  Color color;
};

void Entry() {
  InitWindow(640, 480, "whatever");
  SetTargetFPS(60);

  TraceLog(LOG_INFO, "Initializing scene");

  Scene scene;

  TraceLog(LOG_INFO, "adding circle object implementation");
  Scene_ObjectImplementationAdd<Object_Circle>(
    scene
  , ObjectImplementation {
      .objectStorage = {},
      .updateAll =
        [](
          uint8_t * const dataRaw
        , std::vector<ObjectId> const & objects
        ) -> void {
          Object_Circle * const data =
            reinterpret_cast<Object_Circle * const>(dataRaw);

          for (auto const & objectId : objects) {
            Object_Circle & circle = data[objectId.id];
            circle.xPosition =
              250 + objectId.id*100
            + static_cast<int32_t>(sin(GetTime() * objectId.id*5) * 50.0)
            ;
            circle.yPosition =
              250 + static_cast<int32_t>(cos(GetTime()) * 50.0 + objectId.id*25)
            ;
          }
        }
    , .renderAll =
        [](
          uint8_t const * const dataRaw
        , std::vector<ObjectId> const & objects
        ) -> void {
          Object_Circle const * const data =
            reinterpret_cast<Object_Circle const * const>(dataRaw);

          for (auto const & objectId : objects) {
            Object_Circle const & circle = data[objectId.id];
            DrawCircleV(
              Vector2{
                static_cast<float>(circle.xPosition),
                static_cast<float>(circle.yPosition)
              }
            , 50
            , circle.color
            );
          }
        }
    }
  );

  Scene_ObjectImplementationAdd<Object_Rectangle>(
    scene
  , ObjectImplementation {
      .objectStorage = {},
      .updateAll =
        [](
          uint8_t * const dataRaw
        , std::vector<ObjectId> const & objects
        ) -> void {
          Object_Rectangle * const data =
            reinterpret_cast<Object_Rectangle * const>(dataRaw);

          for (auto const & objectId : objects) {
            Object_Rectangle & self = data[objectId.id];
            self.xPosition =
              50 + (objectId.id % 20)*25
            + static_cast<int32_t>(sin(GetTime()) * 50.0)
            ;
            self.yPosition =
              50 + static_cast<int32_t>(objectId.id / 20)*25
            + static_cast<int32_t>(cos(GetTime()) * 50.0)
            ;
          }
        }
    , .renderAll =
        [](
          uint8_t const * const dataRaw
        , std::vector<ObjectId> const & objects
        ) -> void {
          Object_Rectangle const * const data =
            reinterpret_cast<Object_Rectangle const * const>(dataRaw);

          for (auto const & objectId : objects) {
            Object_Rectangle const & self = data[objectId.id];
            DrawRectangleV(
              Vector2{
                static_cast<float>(self.xPosition),
                static_cast<float>(self.yPosition)
              }
            , Vector2{50, 50}
            , Color {
                static_cast<uint8_t>((objectId.id % 10)/10.0f * 255.0f),
                static_cast<uint8_t>((objectId.id % 40)/40.0f * 255.0f),
                static_cast<uint8_t>((objectId.id % 80)/80.0f * 255.0f),
                255,
              }
            );
          }
        }
    }
  );

  TraceLog(LOG_INFO, "adding circle object");

  Scene_ObjectAdd(
    scene,
    Object_Circle { .xPosition = 50, .yPosition = 50, .color = YELLOW }
  );

  Scene_ObjectAdd(
    scene,
    Object_Circle { .xPosition = 50, .yPosition = 50, .color = VIOLET }
  );

  Scene_ObjectAdd(
    scene,
    Object_Circle { .xPosition = 50, .yPosition = 50, .color = LIME }
  );

  for (int i = 0; i < 200; ++ i) {
    Scene_ObjectAdd(
      scene,
      Object_Rectangle { .xPosition = 0, .yPosition = 0, .color = GRAY }
    );
  }

  TraceLog(LOG_INFO, "entering loop");
  while (!WindowShouldClose()) {

    Scene_Update(scene);

    BeginDrawing();
      ClearBackground(RAYWHITE);

      Scene_Render(scene);
    EndDrawing();
  }

    TraceLog(LOG_INFO, "Closing window\n");
  CloseWindow();
}

int main()
{
  try {
    Entry();
  } catch (std::exception & e) {
    TraceLog(LOG_ERROR, "EXCEPTION CAUGHT!!!");
    TraceLog(LOG_ERROR, "EXCEPTION: %s", e.what());
  }

  return 0;
}
