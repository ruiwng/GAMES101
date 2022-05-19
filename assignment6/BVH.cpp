#include <algorithm>
#include <cassert>
#include "BVH.hpp"

#define SAH_SPLIT_COUNT 12

BVHAccel::BVHAccel(std::vector<Object*> p, int maxPrimsInNode,
                   SplitMethod splitMethod)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), splitMethod(splitMethod),
      primitives(std::move(p))
{
    if (primitives.empty())
        return;

    auto start = std::chrono::system_clock::now();
    if(splitMethod == SplitMethod::NAIVE) {
        root = recursiveBuild(primitives);
    } else {
        root = recursiveBuildSAH(0, primitives.size());
    }
    auto stop = std::chrono::system_clock::now();

    std::cout << "BVH Generation complete:: \n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << " hours\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() << " minutes\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() << " seconds\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << " milliseconds\n";
}


BVHBuildNode* BVHAccel::recursiveBuildSAH(size_t firstPrimOffset, size_t nPrimitives) {
    Bounds3 bbox;
    for(size_t i = 0; i < nPrimitives; ++i) {
        bbox = Union(bbox, primitives[firstPrimOffset + i]->getBounds());
    }
    BVHBuildNode* node = new BVHBuildNode();
    node->left = nullptr;
    node->right = nullptr;
    node->bounds = bbox;
    node->firstPrimOffset = firstPrimOffset;
    node->nPrimitives = nPrimitives;
    if(nPrimitives <= maxPrimsInNode) {
        return node;
    }

    float totalArea = bbox.SurfaceArea();
    float minCost = std::numeric_limits<float>::max();
    size_t minAxis = 0;
    size_t minLeftCount;
    float splitPos;
    Vector3f diagonal = bbox.Diagonal();
    for(size_t axis = 0; axis < 3; ++axis) {
        std::sort(primitives.begin() + firstPrimOffset, primitives.begin() + firstPrimOffset + nPrimitives, 
        [&](Object* lhs, Object* rhs) -> bool {
            return lhs->getBounds().Centroid()[axis] < rhs->getBounds().Centroid()[axis];
        });

        Bounds3 leftBound, rightBound;
        std::vector<Bounds3> leftBounds;
        std::vector<Bounds3> rightBounds;
        for(size_t i = 0; i < nPrimitives; ++i) {
            leftBound = Union(leftBound, primitives[firstPrimOffset + i]->getBounds());
            rightBound = Union(rightBound, primitives[firstPrimOffset + nPrimitives - i - 1]->getBounds());
            leftBounds.push_back(leftBound);
            rightBounds.push_back(rightBound);
        }
        float splitOffset = bbox.pMin[axis];
        float splitStep = diagonal[axis] / SAH_SPLIT_COUNT;
        int objectIndex = firstPrimOffset;
        size_t leftCount, rightCount;
        float leftArea, rightArea;
        for(size_t splitNum = 0; splitNum < SAH_SPLIT_COUNT - 1; ++splitNum) {
            splitOffset += splitStep;
            while((objectIndex < firstPrimOffset + nPrimitives) && primitives[objectIndex]->getBounds().Centroid()[axis] < splitOffset) {
                ++objectIndex;
            }
            leftCount = objectIndex - firstPrimOffset;
            rightCount = nPrimitives - leftCount;
            leftArea = leftCount == 0? 0: leftBounds[leftCount - 1].SurfaceArea();
            rightArea = rightCount == 0? 0: rightBounds[rightCount - 1].SurfaceArea();
            float cost = .125f + (leftCount * leftArea + rightCount * rightArea) / totalArea;
            if(cost < minCost) {
                minCost = cost;
                minAxis = axis;
                splitPos = splitOffset;
                minLeftCount = leftCount;
            }
        }
    }
    std::sort(primitives.begin() + firstPrimOffset, primitives.begin() + firstPrimOffset + nPrimitives, 
        [&](Object* lhs, Object* rhs) -> bool {
            return lhs->getBounds().Centroid()[minAxis] < rhs->getBounds().Centroid()[minAxis];
    });
    if(minLeftCount == nPrimitives) {
        return node;
    }
    node->left = recursiveBuildSAH(firstPrimOffset, minLeftCount);
    node->right = recursiveBuildSAH(firstPrimOffset + minLeftCount, nPrimitives - minLeftCount);
    return node;
}

BVHBuildNode* BVHAccel::recursiveBuild(std::vector<Object*> objects)
{
    BVHBuildNode* node = new BVHBuildNode();

    // Compute bounds of all primitives in BVH node
    Bounds3 bounds;
    for (int i = 0; i < objects.size(); ++i)
        bounds = Union(bounds, objects[i]->getBounds());
    if (objects.size() == 1) {
        // Create leaf _BVHBuildNode_
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->left = nullptr;
        node->right = nullptr;
        return node;
    }
    else if (objects.size() == 2) {
        node->left = recursiveBuild(std::vector{objects[0]});
        node->right = recursiveBuild(std::vector{objects[1]});

        node->bounds = Union(node->left->bounds, node->right->bounds);
        return node;
    }
    else {
        Bounds3 centroidBounds;
        for (int i = 0; i < objects.size(); ++i)
            centroidBounds =
                Union(centroidBounds, objects[i]->getBounds().Centroid());
        int dim = centroidBounds.maxExtent();
        switch (dim) {
        case 0:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().x <
                       f2->getBounds().Centroid().x;
            });
            break;
        case 1:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().y <
                       f2->getBounds().Centroid().y;
            });
            break;
        case 2:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().z <
                       f2->getBounds().Centroid().z;
            });
            break;
        }

        auto beginning = objects.begin();
        auto middling = objects.begin() + (objects.size() / 2);
        auto ending = objects.end();

        auto leftshapes = std::vector<Object*>(beginning, middling);
        auto rightshapes = std::vector<Object*>(middling, ending);

        assert(objects.size() == (leftshapes.size() + rightshapes.size()));

        node->left = recursiveBuild(leftshapes);
        node->right = recursiveBuild(rightshapes);

        node->bounds = Union(node->left->bounds, node->right->bounds);
    }

    return node;
}

Intersection BVHAccel::Intersect(const Ray& ray) const
{
    Intersection isect;
    if (!root)
        return isect;
    if(splitMethod == SplitMethod::NAIVE) {
        isect = BVHAccel::getIntersection(root, ray);
    } else {
        isect = BVHAccel::getIntersectionSAH(root, ray);
    }
    return isect;
}

Intersection BVHAccel::getIntersection(BVHBuildNode* node, const Ray& ray) const
{
    // TODO Traverse the BVH to find intersection
    if(!node->bounds.IntersectP(ray, ray.direction_inv, {int(ray.direction_inv[0] < 0.0), int(ray.direction_inv[1] < 0.0), int(ray.direction_inv[2] < 0.0)})) {
        return Intersection();
    }
    if(node->object) {
        return node->object->getIntersection(ray);
    }
    auto intersectionLeft = getIntersection(node->left, ray);
    auto intersectionRight = getIntersection(node->right, ray);
    if(intersectionLeft.happened && intersectionRight.happened) {
        return intersectionLeft.distance < intersectionRight.distance? intersectionLeft: intersectionRight;
    } else if(intersectionLeft.happened) {
        return intersectionLeft;
    } else {
        return intersectionRight;
    }
}

Intersection BVHAccel::getIntersectionSAH(BVHBuildNode* node, const Ray& ray) const
{
    Intersection isect;
    if(node->isLeaf()) {
        for(int i = 0; i < node->nPrimitives; ++i) {
            auto intersection = primitives[node->firstPrimOffset + i]->getIntersection(ray);
            if(intersection.happened &&(!isect.happened ||(isect.happened && intersection.distance < isect.distance))) {
                isect = intersection;
            }
        }
        return isect;
    }
    std::array<int, 3> dirIsNeg = {int(ray.direction_inv[0] < 0.0), int(ray.direction_inv[1] < 0.0), int(ray.direction_inv[2] < 0.0)};
    float leftRangeMin, leftRangeMax;
    bool isLeftIntersec = node->left->bounds.IntersectP(ray, ray.direction_inv, dirIsNeg, leftRangeMin, leftRangeMax);
    float rightRangeMin, rightRangeMax;
    bool isRightIntersec = node->right->bounds.IntersectP(ray, ray.direction_inv, dirIsNeg, rightRangeMin, rightRangeMax);
    if(isLeftIntersec && isRightIntersec) {
        float secondMin;
        BVHBuildNode *firstNode, *secondNode;
        if(leftRangeMin < rightRangeMin) {
            secondMin = rightRangeMin;
            firstNode = node->left;
            secondNode = node->right;
        } else {
            secondMin = leftRangeMin;
            firstNode = node->right;
            secondNode = node->left;
        }
        auto firstIntersection = getIntersectionSAH(firstNode, ray);
        if(!firstIntersection.happened || (firstIntersection.happened && firstIntersection.distance > secondMin)) {
            auto secondIntersection = getIntersectionSAH(secondNode, ray);
            if(secondIntersection.happened &&(!firstIntersection.happened || (firstIntersection.distance > secondIntersection.distance))) {
                firstIntersection = secondIntersection;
            }
        }
        return firstIntersection;
    } else if(isLeftIntersec) {
        return getIntersectionSAH(node->left, ray);
    } else if(isRightIntersec) {
        return getIntersectionSAH(node->right, ray);
    }
    return isect;
}