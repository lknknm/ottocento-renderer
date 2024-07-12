#pragma once
#include "Camera.h"
#include "Ray.h"
#include "Scene.h"
#include <glm/glm.hpp>
#include <Walnut/Image.h>
#include <memory>


class Renderer
{
public:
	struct HitPayload
	{
		float HitDistance;
		glm::vec3 WorldPosition;
		glm::vec3 WorldNormal;

		double u;
		double v;
		int ObjectIndex;
	};

	struct Settings
	{
		bool Accumulate = true;
		int bounces = 5;
	};

	Renderer() = default;
	void OnResize(uint32_t width, uint32_t height);
	void Render(const Scene& scene, const Camera& camera);
	void ResetFrameIndex() { m_FrameIndex = 1; }
	Settings& GetSettings() { return m_Settings; }
	std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }

private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	Settings m_Settings;

	std::vector<uint32_t> m_ImageHorizontalIter, m_ImageVerticalIter;

	const Scene* m_ActiveScene = nullptr;
	const Camera* m_ActiveCamera = nullptr;

	uint32_t* m_ImageData = nullptr;
	glm::vec4* m_AccumulationData = nullptr;
	uint32_t m_FrameIndex = 1;

	glm::vec4 PerPixel(uint32_t x, uint32_t y); // RayGen per pixel
	HitPayload TraceRay(const Ray& ray);
	HitPayload ClosestHit(const Ray& ray, float hitDistance, int objectIndex);
	HitPayload Miss(const Ray& ray);
	float ggxDistribution(Material material, float nDotHalfVec);
	float geometryGGX(Material material, float dotProduct);
	glm::vec3 schlickFresnel(float cosTheta, glm::vec3 F0);
};