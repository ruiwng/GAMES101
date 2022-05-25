//
// Created by goksu on 2/25/20.
//
#include "Scene.hpp"
#include <mutex>

#pragma once
struct hit_payload
{
    float tNear;
    uint32_t index;
    Vector2f uv;
    Object* hit_obj;
};

class Renderer
{
public:
    void Render(const Scene& scene);
    void RenderThread();
    bool GetPixelCoord(uint32_t& row, uint32_t& column);
private:
    uint32_t current_row{0};
    uint32_t current_column{0};
    uint32_t scene_width;
    uint32_t scene_height;
    std::mutex coord_mtx;
    uint32_t pixel_processed{0};
    std::mutex log_mtx;

    
};
