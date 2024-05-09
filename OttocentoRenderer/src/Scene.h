#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Material
{
    glm::vec3 Albedo{ 1.0f };
    float Roughness = 1.0f;
    float Metallic = 0.0f;
};

struct Sphere
{
    glm::vec3 Position{0.0f};
    float Radius = 0.5f;
    bool isVisible = true;
    int MaterialIndex = 0;
};

struct Light
{
    glm::vec3 Position{-1.0f};
    float Radius = 0.5f;
    float Intensity = 0.5f;
    bool isActive = false;
};

struct Scene
{
    std::vector<Sphere> Spheres;
    std::vector<Light> Lights;
    std::vector<Material> Materials;
};