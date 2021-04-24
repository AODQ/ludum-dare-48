#pragma once

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

struct ObjectId {
  uint32_t type = 0u, id = 0u;
};

bool IsSame(ObjectId const & x, ObjectId const & y);

bool IsSameType(ObjectId const & x, ObjectId const & y);

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

template <typename T>
T & Scene_ObjectLookup(Scene & scene, ObjectId const & object);

template <typename T>
ObjectId Scene_ObjectAdd(Scene & scene, T const & data);

template <typename T>
void Scene_ObjectImplementationAdd(
  Scene & scene
, ObjectImplementation const & impl
);
void Scene_Update(Scene & scene);
void Scene_Render(Scene & scene);

#define assert(cnd) \
  if (!(cnd)) { \
    TraceLog( \
      LOG_ERROR, \
      "assertion fail: %s@%u: `%s`", __FILE__, __LINE__, #cnd \
    ); \
  }

#include <raylib.h>

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
