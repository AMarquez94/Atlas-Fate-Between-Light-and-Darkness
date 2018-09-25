#pragma once
#include "PxPhysicsAPI.h"

enum FilterGroup {

    Wall = 1 << 0,
    Floor = 1 << 1,
    Player = 1 << 2,
    Enemy = 1 << 3,
    Ignore = 1 << 4,
    Fence = 1 << 5,
    DItem = 1 << 6,
    MovableObject = 1 << 7,
    Button = 1 << 8,
    NonCastShadows = 1 << 9,
    Light = 1 << 10,
    Scenario = Wall | Floor,
    Characters = Player | Enemy,
    All = -1
};

physx::PxFilterFlags CustomFilterShader(
    physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
    physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
    physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize
);

FilterGroup getFilterByName(const std::string & name);