#include <metal_stdlib>
#define TILES_PER_SIDE 64
#define TILES_PER_CHUNK (64 * 64)
#define TILE_DIM     (64.0/100.0)
#define TILE_ORIGIN  float2(0.0, 0.0)
#define CHUNK_SIZE 100.0
#define NUM_OCTAVES 5


using namespace metal;

struct Vertex
{
    float3 position;
    float3 normal;
    float3 tangent;
    float3 bitangent;
    float3 color;
    float2 uv;
};

struct ChunkUniforms
{
    float4x4 model_matrix;
    int32_t chunk_index;
};

struct CameraData
{
    float4x4 view_matrix;
    float4x4 projection_matrix;
};

struct GlobalUniforms
{
    CameraData camera;
    float3 sun_dir;
    int no_lights;
    int debug_texture;
    int LOS_ON;
    float u_time;
};

struct TerrainUniforms 
{
    uint8_t texture_indices[3 * 3 * 64 * 64];    
    uint8_t los_indices[3 * 3 * 64 * 64]; 
};

struct v2f
{
    float4 position [[position]];
    float4 world_position;
    float4 local_position;
};

v2f vertex vertexMain( 
        uint vertex_id [[ vertex_id ]],
        constant GlobalUniforms &global_uniforms [[ buffer(0) ]],
        device const Vertex *vertices [[ buffer(1) ]],
        constant ChunkUniforms  &local_uniforms  [[ buffer(2) ]]
    )
{
    v2f o;
    o.local_position = float4(vertices[vertex_id].position, 1.0);
    o.world_position = local_uniforms.model_matrix * float4(vertices[vertex_id].position, 1.0);
    o.position = global_uniforms.camera.projection_matrix * global_uniforms.camera.view_matrix * o.world_position;
    return o;
};

uint8_t get_chunk_index(float2 pos, int32_t chunk_index)
{
    int32_t ret = chunk_index;

    if (pos.x < 0)
    {
        if(chunk_index % 3 != 0)
        {
            ret -= 1;
        }
    }
    else if (pos.x > CHUNK_SIZE)
    {
        if(chunk_index % 3 != 2)
        {
            ret += 1;
        }
    }

    if (pos.y < 0)
    {
        if(chunk_index > 2)
        {
            ret -= 3;
        }
    }
    else if (pos.y > CHUNK_SIZE)
    {
        if(chunk_index < 6)
        {
            ret += 3;
        }
    }

    return ret;
}

uint16_t tile_index_from_pos(float2 pos, int32_t chunk_index)
{
    if (pos.x < 0)
    {
        pos.x += CHUNK_SIZE;
    }
    if (pos.y < 0)
    {
        pos.y += CHUNK_SIZE;
    }
    if (pos.x > CHUNK_SIZE)
    {
        pos.x -= CHUNK_SIZE;
    }
    if (pos.y > CHUNK_SIZE)
    {
        pos.y -= CHUNK_SIZE;
    }
    float tile_width = CHUNK_SIZE / TILES_PER_SIDE;
    int x = floor(pos.x / tile_width);
    int y = floor(pos.y / tile_width);
    return TILES_PER_CHUNK * chunk_index + (y * TILES_PER_SIDE + x);
}

int32_t get_los_index(float2 pos, uint8_t const constant *los_index, bool LOS_ON, int32_t chunk_index)
{
    if(LOS_ON)
    {
         return los_index[tile_index_from_pos(pos, chunk_index)];
    }
    else
    {
         return 2;
    }
};

float random(float2 pos)
{
    return fract(sin(dot(pos, float2(12.9898, 78.233))) * 43758.5453123);
}

float noise(float2 pos)
{
    float2 i = floor(pos);
    float2 f = fract(pos);

    float a = random(i);
    float b = random(i + float2(1.0, 0.0));
    float c = random(i + float2(0.0, 1.0));
    float d = random(i + float2(1.0, 1.0));

    float2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

float fbm(float2 pos)
{
    float v = 0.0;
    float a = 0.5;

    float2 shift = {100.0};
    // Rotate to reduce 'axial bias' (TODO: research)

    float2x2 rot = float2x2(cos(0.5), sin(0.5), -sin(0.5), cos(0.50));

    for(int i = 0; i < NUM_OCTAVES; i++)
    {
        v += a * noise(pos);
        pos = rot * pos * 2.0 + shift;
        a *= 0.5;
    }
    return v;
}


half4 fragment fragmentMain( 
        v2f in [[stage_in]],
        constant GlobalUniforms &global_uniforms [[ buffer(0) ]],
        constant TerrainUniforms &terrain_uniforms [[ buffer(1) ]],
        constant ChunkUniforms  &local_uniforms  [[ buffer(2) ]]
    )
{
    float2 pos = in.local_position.xy;
    uint8_t chunk_index = local_uniforms.chunk_index;
    chunk_index = get_chunk_index(pos, chunk_index);

    uint8_t los = get_los_index(pos, &terrain_uniforms.los_indices[0], global_uniforms.LOS_ON, chunk_index); 

    if(los == 0) 
    {
        pos = in.world_position.xy / 16.0;
        float3 color = {0.0};

        float2 q = {0.0};
        q.x = fbm(pos + 0.0 * global_uniforms.u_time);
        q.y = fbm(pos + float2(1.0, 1.0));

        float2 r = {0.0};
        r.x = fbm(pos + 1.0*q + float2(1.7, 9.2) + 0.15*global_uniforms.u_time);
        r.y = fbm(pos + 1.0*q + float2(8.3, 2.8) + 0.126*global_uniforms.u_time);

        float f = fbm(pos + r);

        color = mix(float3(0.101961/64, 0.619608/64, 0.666667/64),
                    float3(0.666667/32, 0.666667/32, 0.498039/32),
                    clamp((f*f)*4.0, 0.0, 1.0));

        color = mix(color, 
                    float3(0.0, 0.0, 0.164706/16),
                    clamp(length(q), 0.0, 1.0));

        color = mix(color,
                    float3(0.666667/8, 1.0/8, 1.0/8),
                    clamp(abs(r.x), 0.0, 1.0));

        return half4(color.x/4, color.y/4, color.z/4, 1.0);
    }
    else if(los < 255)
    {
        pos = in.world_position.xy / 16.0;
        float4 color = {0.0};

        float2 q = {0.0};
        q.x = fbm(pos + 0.0 * global_uniforms.u_time);
        q.y = fbm(pos + float2(1.0, 1.0));

        float2 r = {0.0};
        r.x = fbm(pos + 1.0*q + float2(1.7, 9.2) + 0.15*global_uniforms.u_time);
        r.y = fbm(pos + 1.0*q + float2(8.3, 2.8) + 0.126*global_uniforms.u_time);

        float f = fbm(pos + r);

        color = mix(float4(0.101961, 0.319608, 0.666667, 0.0),
                    float4(0.666667, 0.666667, 0.498039, 0.1),
                    clamp((f*f)*2.0, 0.0, 1.0));

        color = mix(color, 
                    float4(0.0, 0.0, 0.164706, 0.0),
                    clamp(length(q * 2.0), 0.0, 1.0));

        color = mix(color,
                    float4(0.666667, 1.0, 1.0, 0.1),
                    clamp(abs(r.x * 2.0), 0.0, 1.0));


        return half4(
                // base color
                color.x/2, color.y/2, color.z/2, 
                    mix(
                        // mix with 0 alpha in case of fading
                        (1- (color.x + color.y + color.z)/3) / 2,
                        0.0,
                        clamp(
                            los-100,
                            0,
                            100)/100
                        )
                    );
    }
    discard_fragment();
    return half4{0.0, 0.0, 0.0, 0.0};
}


//oid main() {
//   vec2 st = gl_FragCoord.xy/u_resolution.xy*3.;
//   // st += st * abs(sin(u_time*0.1)*3.0);
//   vec3 color = vec3(0.0);
//
//   vec2 q = vec2(0.);
//   q.x = fbm( st + 0.00*u_time);
//   q.y = fbm( st + vec2(1.0));
//
//   vec2 r = vec2(0.);
//   r.x = fbm( st + 1.0*q + vec2(1.7,9.2)+ 0.15*u_time );
//   r.y = fbm( st + 1.0*q + vec2(8.3,2.8)+ 0.126*u_time);
//
//   float f = fbm(st+r);
//
//   color = mix(vec3(0.101961,0.619608,0.666667),
//               vec3(0.666667,0.666667,0.498039),
//               clamp((f*f)*4.0,0.0,1.0));
//
//   color = mix(color,
//               vec3(0,0,0.164706),
//               clamp(length(q),0.0,1.0));
//
//   color = mix(color,
//               vec3(0.666667,1,1),
//               clamp(length(r.x),0.0,1.0));
//
//   gl_FragColor = vec4((f*f*f+.6*f*f+.5*f)*color,1.);
//
//
