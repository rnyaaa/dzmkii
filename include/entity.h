#ifndef _ENTITY_H
#define _ENTITY_H

#include "common.h"

#define MAX_ENTITIES 1000
struct MoveSpeed
{
    u32 speed;
};

struct LineOfSight
{
    u32 los;
};

struct Health
{
    u32 hp;
};

struct Team
{
    u32 team;
};

#endif // ENTITY_H
