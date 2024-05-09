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
}

//----------------------------------------------------------------------------
void Renderer::Render(const Scene& scene, const Camera& camera)
{
	float aspectRatio = m_FinalImage->GetWidth() / (float)m_FinalImage->GetHeight();

	Ray ray;
	ray.Origin = camera.GetPosition();

	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			ray.Direction = camera.GetRayDirections()[x + y * m_FinalImage->GetWidth()];
			// glm::vec2 coord = { (float)x / (float)m_FinalImage->GetWidth(), (float)y / (float)m_FinalImage->GetHeight()};
			// coord = coord * 2.0f - 1.0f; // Remapping -1 -> 1
			// coord.x *= aspectRatio;

			glm::vec4 color = TraceRay(scene, ray);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
		}
	}
	m_FinalImage->SetData(m_ImageData);
}

//----------------------------------------------------------------------------
glm::vec4 Renderer::TraceRay(const Scene& scene, const Ray& ray)
{
	// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
	// where
	// a = ray origin
	// b = ray direction
	// r = radius
	// t = hit/trace distance

	if (scene.Spheres.size() == 0)
		return glm::vec4(0, 0, 0, 1);

	const Sphere* closestSphere = nullptr;
	float hitDistance = std::numeric_limits<float>::max();
	
	// loop and render all spheres in the scene. This can be 
	// further expanded for all objects in the scene.
	for (const Sphere& sphere : scene.Spheres)
	{
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
			if (closestT < hitDistance)
			{
				hitDistance = closestT;
				closestSphere = &sphere;
			}
		}
	}

	if (closestSphere == nullptr)
		return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	// back to the original equation, a + bt that gives us the hit position.
	// here we are assuming that the closest hit position will be the 
	// smaller value on the quadratic solution for t
	glm::vec3 origin = ray.Origin - closestSphere->Position;
	glm::vec3 hitPoint = origin + ray.Direction * hitDistance;
	glm::vec3 normal = glm::normalize(hitPoint);
	glm::vec3 lightDir;
	float d = 0;

	glm::vec3 sphereColor = closestSphere->Albedo;

	// Loop through all lights in the scene.
	for (size_t i = 0; i < scene.Lights.size(); i++)
	{
		if (scene.Lights[i].isActive == true)
		{
			lightDir = glm::normalize(scene.Lights[i].Position);
			d += glm::max(glm::dot(normal, -lightDir), 0.0f); // == cos(angle)
		}
	}
	sphereColor *= d;
	return glm::vec4(sphereColor, 1);
}
