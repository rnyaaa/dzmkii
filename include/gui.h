#ifndef _GUI_H
#define _GUI_H
#include "geometry.h"
#include "model.h"
#include "renderer.h"

struct GUI
{
    AArect2i selection; 
    Transform selection_rect_transform;
    Model selection_rect_model; 
    int x_start, y_start, x_curr, y_curr;
    GUI();
    AArect2f selection_to_worldspace(Camera camera);
};

#endif // _GUI_H
