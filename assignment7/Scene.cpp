//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                pos.obj = objects[k];
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // TO DO Implement Path Tracing Algorithm here
    auto inter = intersect(ray);
    if(!inter.happened) {
        return Vector3f(0.0f, 0.0f, 0.0f);
    }
    // return (inter.normal + Vector3f(1.0f, 1.0f, 1.0f)) * 0.5f; 
    if(inter.obj->hasEmit()) {
        if(depth == 0) {
            return inter.m->m_emission;
        } else {
            return Vector3f(0.0f, 0.0f, 0.0f);
        }
    }
    Vector3f dirRadiance, indirRadiance;
    Intersection posLight;
    float pdfLight;
    sampleLight(posLight, pdfLight);

    Vector3f shadowDir = posLight.coords - inter.coords;
    float shadowDirLength = shadowDir.norm();
    shadowDir = normalize(shadowDir);
    Vector3f shadowPosOffset = dotProduct(inter.normal, shadowDir) >= 0.0f ? (1e-5 * inter.normal) : (-1e-5 * inter.normal);
    Ray shadowRay(inter.coords + shadowPosOffset, shadowDir);
    auto shadowInter = intersect(shadowRay);
    Vector3f rayDir = -normalize(ray.direction);
    if(!(shadowInter.happened && shadowInter.obj != posLight.obj)) {
        float m = std::max(0.0f, dotProduct(-shadowDir, posLight.normal));
        Vector3f bsdf = inter.m->eval(rayDir, shadowRay.direction, inter.normal);
        dirRadiance = posLight.emit * bsdf * std::max(0.0f, dotProduct(shadowDir, inter.normal)) * m / (shadowDirLength * shadowDirLength * pdfLight);
    }

    // russian roulette
    if(get_random_float() < RussianRoulette) {
        Vector3f wo = inter.m->sample(rayDir, inter.normal);
        wo = normalize(wo);
        Vector3f posOffset = 1e-5 * inter.normal;
        Ray secondaryRay(inter.coords + posOffset, wo);
        indirRadiance = castRay(secondaryRay, depth + 1) * inter.m->eval(rayDir, wo, inter.normal) * std::max(0.0f, dotProduct(wo, inter.normal)) / (inter.m->pdf(rayDir, wo, inter.normal) * RussianRoulette);
    }
    
    return dirRadiance + indirRadiance;
}
