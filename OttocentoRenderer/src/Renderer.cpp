#include "Renderer.h"
#include "Walnut/Random.h"

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
}

//----------------------------------------------------------------------------
void Renderer::Render(const Scene& scene, const Camera& camera)
{
	m_ActiveScene = &scene;
	m_ActiveCamera = &camera;

	if (m_FrameIndex == 1)
		memset(m_AccumulationData, 0, m_FinalImage->GetWidth() * m_FinalImage->GetHeight() * sizeof(glm::vec4));

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
	glm::vec3 color(0.0f);
	float multiplier = 1.0f;

	int bounces = 4;
	for (int i = 0; i < bounces; i++)
	{
		Renderer::HitPayload payload = TraceRay(ray);
		if (payload.HitDistance < 0.0f)
		{
			glm::vec3 skyColor = glm::vec3(0.0f, 0.0f, 0.0f);
			color += skyColor * multiplier;
			break;
		}

		glm::vec3 lightDir;
		float lightIntensity = 0;
		const Sphere& sphere = m_ActiveScene->Spheres[payload.ObjectIndex];
		const Material& material = m_ActiveScene->Materials[sphere.MaterialIndex];
		glm::vec3 sphereColor = material.Albedo;

		// Loop through all lights in the scene.
		for (size_t i = 0; i < m_ActiveScene->Lights.size(); i++)
		{
			if (m_ActiveScene->Lights[i].isActive == true)
			{
				lightDir = glm::normalize(m_ActiveScene->Lights[i].Position);
				lightIntensity += glm::max(glm::dot(payload.WorldNormal, -lightDir), 0.0f); // == cos(angle)
			}
		}
		sphereColor *= lightIntensity;
		color += sphereColor * multiplier;
		multiplier *= 0.5;

		ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
		ray.Direction = glm::reflect(ray.Direction, payload.WorldNormal + material.Roughness * Walnut::Random::Vec3(-0.5f, 0.5f));
	}
	return glm::vec4(color, 1.0f);
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