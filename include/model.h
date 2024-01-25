#include<stdlib.h>
#include<vector>

#include "mesh.h"
#include "transform.h"

#ifndef _MODEL_H
#define _MODEL_H

struct Model
{
    Transform transform;
    std::vector<Mesh> meshes;
};

#endif
