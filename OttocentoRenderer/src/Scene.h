#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <cvt/wstring>
#include <../stb_image/stb_image.h>

struct SkyColor
{
    glm::vec3 Albedo{ 0.5f };
};

struct Material
{
    std::string Name = "Default Material";
    glm::vec3 Albedo{ 1.0f };
    float Roughness = 1.0f;
    bool Metallic = 0; 
    glm::vec3 EmissionColor{ 0.0f };
	float EmissionPower = 0.0f;
    float AmbientOcclusion;

	glm::vec3 GetEmission() const { return EmissionColor * EmissionPower; }
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
    glm::vec3 lightColor{20.0f};
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
    SkyColor SkyColor;
};
