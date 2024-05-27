#ifndef _FACTORY_H
#define _FACTORY_H

#include "entity.h"
#include "geometry.h"
#include "model.h"
#include "transform.h"

struct Factory
{
    void createUnit(v3f pos);
};

#endif // _FACTORY_H
