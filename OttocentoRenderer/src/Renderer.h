#pragma once
#include "Camera.h"
#include "Ray.h"
#include <glm/glm.hpp>
#include <Walnut/Image.h>
#include <memory>

class Renderer
{
public:
	Renderer() = default;
	void OnResize(uint32_t width, uint32_t height);
	void Render(const Camera& camera);

	std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage;  }
private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;

	glm::vec4 TraceRay(const Ray& ray);
};