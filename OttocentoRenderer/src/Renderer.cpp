#include "Renderer.h"
#include "Walnut/Random.h"
#include <execution>
# define M_PI           3.14159265358979323846  /* pi */

namespace Utils 
{
	static uint32_t ConvertToRGBA(const glm::vec4& color)
	{
		uint8_t r = (color.r * 255.0f);
		uint8_t g = (color.g * 255.0f);
		uint8_t b = (color.b * 255.0f);
		uint8_t a = (color.a * 255.0f);

		uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
		return result;
	}
}

//----------------------------------------------------------------------------
void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage)
	{
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;
		m_FinalImage->Resize(width, height);
	}
	else
	{
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];
	delete[] m_AccumulationData;
	m_AccumulationData = new glm::vec4[width * height];

	m_ImageHorizontalIter.resize(width);
	m_ImageVerticalIter.resize(height);
	for (uint32_t i = 0; i < width; i++)
		m_ImageHorizontalIter[i] = i;
	for (uint32_t i = 0; i < height; i++)
		m_ImageVerticalIter[i] = i;
	ResetFrameIndex();
}

//----------------------------------------------------------------------------
void Renderer::Render(const Scene& scene, const Camera& camera)
{
	m_ActiveScene = &scene;
	m_ActiveCamera = &camera;

	if (m_FrameIndex == 1)
		memset(m_AccumulationData, 0, m_FinalImage->GetWidth() * m_FinalImage->GetHeight() * sizeof(glm::vec4));

#define MT 1
#if MT
	std::for_each(std::execution::par, m_ImageVerticalIter.begin(), m_ImageVerticalIter.end(),
		[this](uint32_t y)
		{
			std::for_each(std::execution::par, m_ImageHorizontalIter.begin(), m_ImageHorizontalIter.end(),
				[this, y](uint32_t x)
				{
					glm::vec4 color = PerPixel(x, y);
					m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;

					glm::vec4 accumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
					accumulatedColor /= (float)m_FrameIndex;

					accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
					m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor);
				});
		});

#else
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			glm::vec4 color = PerPixel(x, y);
			m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;
			
			glm::vec4 accumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
			accumulatedColor /= (float)m_FrameIndex;

			accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor);
		}
	}
#endif

	m_FinalImage->SetData(m_ImageData);

	if(m_Settings.Accumulate)
		m_FrameIndex++;
	else
		m_FrameIndex = 1;
}

//----------------------------------------------------------------------------
Renderer::HitPayload Renderer::TraceRay(const Ray& ray)
{
	// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
	// where
	// a = ray origin
	// b = ray direction
	// r = radius
	// t = hit/trace distance
	int closestSphere = -1;
	float hitDistance = std::numeric_limits<float>::max();
	
	// loop and render all spheres in the scene. This can be 
	// further expanded for all objects in the scene.
	for (size_t i = 0; i < m_ActiveScene->Spheres.size(); i++)
	{
		const Sphere& sphere = m_ActiveScene->Spheres[i];
		if (sphere.isVisible)
		{
			glm::vec3 origin = ray.Origin - sphere.Position;

			// in a quadratic equation: ax² + bx + c = 0:
			float a = glm::dot(ray.Direction, ray.Direction);
			float b = 2.0f * glm::dot(origin, ray.Direction);
			float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;

			// in a quadratic formula discriminant b² - 4ac:
			float discriminant = b * b - 4.0f * a * c;

			if (discriminant < 0.0f)
				continue;

			// in a quadratic solution for 2 points
			// float t0 = (-b + sqrt(discriminant)) / (2.0f * a);
			float closestT = (-b - sqrt(discriminant)) / (2.0f * a);
			if (closestT > 0.0f && closestT < hitDistance)
			{
				hitDistance = closestT;
				closestSphere = (int)i;
			}
		}
	}

	if (closestSphere < 0)
		return Miss(ray);
	return ClosestHit(ray, hitDistance, closestSphere);
}

//----------------------------------------------------------------------------
// RayGen per pixel function
glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y) 
{
	Ray ray;
	ray.Origin = m_ActiveCamera->GetPosition();
	ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];
	glm::vec3 finalColor(0.0f);
	float contribution = 1.0f;

	int bounces = 5;
	for (int i = 0; i < bounces; i++)
	{
		Renderer::HitPayload payload = TraceRay(ray);
		if (payload.HitDistance < 0.0f)
		{
			glm::vec3 skyColor = glm::vec3(0.0f, 0.0f, 0.0f);
			finalColor += skyColor * contribution;
			break;
		}
 
		glm::vec3 lightDir;
		float lightIntensity = 0;
		const Sphere& sphere = m_ActiveScene->Spheres[payload.ObjectIndex];
		const Material& material = m_ActiveScene->Materials[sphere.MaterialIndex];
		glm::vec3 objectColor = material.Albedo;

		// Loop through all lights in the scene.
		for (size_t i = 0; i < m_ActiveScene->Lights.size(); i++)
		{
			if (m_ActiveScene->Lights[i].isActive == true)
			{
				lightDir = glm::normalize(m_ActiveScene->Lights[i].Position);
				lightIntensity += glm::max(glm::dot(payload.WorldNormal, -lightDir), 0.0f); // == cos(angle)
			}
		}
		
		objectColor *= lightIntensity;
		// finalColor = (DiffuseBRDF + SpecularBRDF) * LightIntensity *nDotL
		finalColor += objectColor * contribution;
		contribution *= 0.5;

		ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
		ray.Direction = glm::reflect(ray.Direction, payload.WorldNormal + material.Roughness * Walnut::Random::Vec3(-0.5f, 0.5f));
	}
	return glm::vec4(finalColor, 1.0f);
}

//----------------------------------------------------------------------------
// back to the original equation, a + bt that gives us the hit position.
// here we are assuming that the closest hit position will be the 
// smaller value on the quadratic solution for t
Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex)
{
	Renderer::HitPayload payload;
	payload.HitDistance = hitDistance;
	payload.ObjectIndex = objectIndex;

	const Sphere& closestSphere = Renderer::m_ActiveScene->Spheres[objectIndex];

	glm::vec3 origin = ray.Origin - closestSphere.Position;
	payload.WorldPosition = origin + ray.Direction * hitDistance;
	payload.WorldNormal = glm::normalize(payload.WorldPosition);

	payload.WorldPosition += closestSphere.Position;
	return payload;
}

//----------------------------------------------------------------------------
Renderer::HitPayload Renderer::Miss(const Ray& ray)
{
	Renderer::HitPayload payload;
	payload.HitDistance = -1.0f;
	return payload;
}

//----------------------------------------------------------------------------
// Normal Distribution Function based on the GGX formula by Trowbridge and Reitz.
// in which D = α² / (π*(dot(normal,halfVector)²*(α² - 1) + 1)²
double Renderer::ggxDistribution(Material material, float nDotHalfVec)
{
	float alpha2 = material.Roughness * material.Roughness * material.Roughness * material.Roughness;
	float normalDistFunction = nDotHalfVec * nDotHalfVec* (alpha2 - 1) + 1;
	double ggxDistribution = alpha2 / (M_PI * normalDistFunction * normalDistFunction);
	return ggxDistribution;
}

//----------------------------------------------------------------------------
// Geometry Function based on the Schlik-GGX by Schlick and Beckman.
// This function is defined as G = dot(normal,view) / dot(normal,view)*(1-K) + K
// where the constant K is defined by the equation K = (α + 1)² / 8.
float Renderer::geometryGGX(Material material, float dotProduct)
{
	float k = (material.Roughness + 1.0f) * (material.Roughness + 1.0f) / 8.0f;
	float denominator = dotProduct * (1 - k) + k;
	return dotProduct / denominator;
}

//----------------------------------------------------------------------------
// Fresnel Function based on the Schlick Approximation
// Defined as F = F0 + (1-F0) * (1 - dot(view, half))5
glm::vec3 Renderer::schlickFresnel(Material material, float viewDocHalfVec)
{
	glm::vec3 F0(0.04f);
	if (material.Metallic)
		F0 = material.Albedo;
	glm::vec3 ret = F0 + (1.0f - F0) * pow(glm::clamp(1.0f - viewDocHalfVec, 0.0f, 1.0f), 5.0f);
	return ret;
}