#include "noise.h"
#include <stdlib.h>

f32 Noise::perlin(v2f position)
{
    return 
        noise_scale * std::powf(
                            perlin_source.octave2D(position.x * perlin_scale, position.y * perlin_scale, octaves),
                           powr);
};

f32 Noise::voronoi(v2f position)
{
    // credit: u/Clayman8000 
    // https://www.reddit.com/r/proceduralgeneration/comments/4u0s1i/comment/d5lx1my/
    v2f closest_grid {std::floor(position.x / noise_scale), std::floor(position.y / noise_scale)};

    f32 frac_dist_x = position.x - closest_grid.x;
    f32 frac_dist_y = position.y - closest_grid.y;

    f32 min_dist = noise_scale * 4;

    for(int i = -1; i < 1; i++)
    {
        for(int j = -1; j < 1; j++)
        {
            v2f hashedpos = {
                                perlin(
                                        v2f{closest_grid.x - position.x, closest_grid.y + position.y}
                                        ), 
                                perlin(
                                        v2f{closest_grid.x + position.x, closest_grid.y - position.y}
                                        )};
            v2f currp = v2f {i + hashedpos.x - frac_dist_x, j + hashedpos.y - frac_dist_y};
            f32 distance = (currp.x * currp.x + currp.y * currp.y);

            if(distance < min_dist)
            {
                min_dist = distance;
            }
        }
    }
    return min_dist;
}
