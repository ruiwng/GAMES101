//
// Created by goksu on 2/25/20.
//

#include <fstream>
#include <thread>
#include "Scene.hpp"
#include "Renderer.hpp"


inline float deg2rad(const float& deg) { return deg * M_PI / 180.0; }

const float EPSILON = 0.00001;

// The main render function. This where we iterate over all pixels in the image,
// generate primary rays and cast these rays into the scene. The content of the
// framebuffer is saved to a file.
bool Renderer::GetPixelCoord(uint32_t& row, uint32_t& column) {
    coord_mtx.lock();
    bool found = current_row < scene_height;
    if(found) {
        row = current_row;
        column = current_column;
        if(++current_column >= scene_width) {
            ++current_row;
            current_column = 0;
        }
    }
    coord_mtx.unlock();
    return found;
}

void Renderer::Render(const Scene& scene)
{
    scene_width = scene.width;
    scene_height = scene.height;
    std::vector<Vector3f> framebuffer(scene.width * scene.height);

    float scale = tan(deg2rad(scene.fov * 0.5));
    float imageAspectRatio = scene.width / (float)scene.height;
    Vector3f eye_pos(278, 273, -800);

    // change the spp value to change sample ammount
    int spp = 1;
    std::cout << "SPP: " << spp << "\n";
    int threadCount = 3;
    std::cout << "Thread Count: " << threadCount << "\n";
    auto renderThread = [&]() {
        uint32_t row, column;
        while(GetPixelCoord(row, column)) {
            float x = (2 * (column + 0.5) / (float)scene.width - 1) *
                      imageAspectRatio * scale;
            float y = (1 - 2 * (row + 0.5) / (float)scene.height) * scale;

            Vector3f dir = normalize(Vector3f(-x, y, 1));
            for (int k = 0; k < spp; k++){
                framebuffer[row * scene.width + column] += scene.castRay(Ray(eye_pos, dir), 0) / spp;  
            }
            log_mtx.lock();
            ++pixel_processed;
            UpdateProgress(pixel_processed / float(scene.width * scene.height));
            log_mtx.unlock();
        }
    };
    std::vector<std::thread> threadArray;
    for(int i = 0; i < threadCount; ++i) {
        threadArray.push_back(std::thread(renderThread));
    }
    for(int i = 0; i < threadCount; ++i) {
        threadArray[i].join();
    }
    UpdateProgress(1.f);

    // save framebuffer to file
    FILE* fp = fopen("binary.ppm", "wb");
    (void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
    for (auto i = 0; i < scene.height * scene.width; ++i) {
        static unsigned char color[3];
        color[0] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].x), 0.6f));
        color[1] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].y), 0.6f));
        color[2] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].z), 0.6f));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);    
}
