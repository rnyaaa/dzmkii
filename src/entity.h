#ifndef _ENTITY_H
#define _ENTITY_H

#include "transform.h"

struct Entity
{
    int line_of_sight = 6;
    Transform transform;
};

#endif // ENTITY_H
