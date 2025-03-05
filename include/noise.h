#include "3rdparty/PerlinNoise.hpp"
#include "common.h"
#include "geometry.h"
#ifndef _NOISE_H
#define _NOISE_H

struct Noise 
{
    f32 perlin_scale;
    f32 noise_scale;
    u32 powr;
    u32 octaves;

    u32 seed; 
    const::siv::PerlinNoise perlin_source;

    Noise(f32 perlin_scale, f32 noise_scale, u32 pow, u32 powr, u32 seed);

    f32 perlin(v2f position);
    f32 voronoi(v2f position);
};
//perlin.octave2D((startx + corners[i].x) * perlin_scale, (starty + corners[i].y) * perlin_scale, octaves)

#endif // _NOISE_H
