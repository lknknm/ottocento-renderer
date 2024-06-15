#pragma once

#include <glm/glm.hpp>
#include <vector>

class Camera
{
public:
	Camera(float verticalFOV, float nearClip, float farClip);

	bool OnUpdate(float ts);
	void OnResize(uint32_t width, uint32_t height);

	const glm::mat4& GetProjection() const { return m_Projection; }
	const glm::mat4& GetInverseProjection() const { return m_InverseProjection; }
	const glm::mat4& GetView() const { return m_View; }
	const glm::mat4& GetInverseView() const { return m_InverseView; }
	
	const glm::vec3& GetPosition() const { return m_Position; }
	const glm::vec3& GetDirection() const { return m_ForwardDirection; }

	const std::vector<glm::vec3>& GetRayDirections() const { return m_RayDirections; }
	const glm::vec3 DefocusDiskSample() const;
	
	glm::vec3 m_Position{0.0f, 0.0f, 0.0f};
	float GetRotationSpeed();

	// Depth of Field Implementation
	bool m_DepthOfFieldOn = false;
	float m_DefocusAngle = 10;  // Variation angle of rays through each pixel
	float m_FocusDist = 1.0;    // Distance from camera lookfrom point to plane of perfect focus
	
private:
	void RecalculateProjection();
	void RecalculateView();
	void RecalculateRayDirections();

	glm::mat4 m_Projection{ 1.0f };
	glm::mat4 m_View{ 1.0f };
	glm::mat4 m_InverseProjection{ 1.0f };
	glm::mat4 m_InverseView{ 1.0f };
	glm::vec3 m_ForwardDirection{0.0f, 0.0f, 0.0f};
	glm::vec3 m_UpVector{0.0f, 1.0f, 0.0f};

	float m_VerticalFOV = 45.0f;
	float m_NearClip = 0.1f;
	float m_FarClip = 100.0f;
	
	// Cached ray directions
	std::vector<glm::vec3> m_RayDirections;

	glm::vec2 m_LastMousePosition{ 0.0f, 0.0f };

	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
	
	// Depth of Field Implementation. Calculate the camera defocus disk basis vectors.
	glm::vec3 m_PixelDeltaU{0.0f};        // Offset to pixel to the right
	glm::vec3 m_PixelDeltaV{0.0f};        // Offset to pixel below
	glm::vec3 u, v, w;     
	glm::vec3 m_DefocusDiskU;       // Defocus disk horizontal radius
	glm::vec3 m_DefocusDiskV;       // Defocus disk vertical radius
	float m_DefocusRadius;
	//double m_DefocusRadius = m_FocusDist * tan(glm::radians(m_DefocusAngle / 2));
};
