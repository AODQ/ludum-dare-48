#include <scene.hpp>

#include <raylib.h>

#define assert(cnd) \
  if (!(cnd)) { \
    TraceLog( \
      LOG_ERROR, \
      "assertion fail: %s@%u: `%s`", __FILE__, __LINE__, #cnd \
    ); \
  }

bool IsSame(ObjectId const & x, ObjectId const & y) {
  return x.type == y.type && x.id == y.id;
}

bool IsSameType(ObjectId const & x, ObjectId const & y) {
  return x.type == y.type;
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
