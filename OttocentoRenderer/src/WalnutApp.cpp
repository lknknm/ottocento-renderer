#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"

#include "Camera.h"
#include "Renderer.h"

#include <glm/gtc/type_ptr.hpp>

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
ExampleLayer() : m_Camera(45.0f, 0.1f, 100.0f) 
{
	{	
		Sphere sphere;
		sphere.Position = {0.0f, 0.0f, 0.0f};
		sphere.Albedo = {0.0f, 0.0f, 1.0f};
		sphere.Radius = 0.5f;
		m_Scene.Spheres.push_back(sphere);
	}
	
	{
		Sphere sphere;
		sphere.Position = {1.0f, 0.0f, -5.0f};
		sphere.Albedo = {1.0f, 1.0f, 1.0f};
		sphere.Radius = 0.5f;
		m_Scene.Spheres.push_back(sphere);
	}
	{
		Light light;
		light.Position = {-1.0f, -1.0f, -1.0f};
		light.Radius = 0.5f;
		m_Scene.Lights.push_back(light);
	}
}

//----------------------------------------------------------------------------
virtual void OnUpdate(float ts) override
{
	m_Camera.OnUpdate(ts);
}

//----------------------------------------------------------------------------
virtual void OnUIRender() override
{
	ImGui::Begin("Light Settings");
	ImGui::Text("Light Settings");
	ImGui::Separator();
	for (size_t i = 0; i < m_Scene.Lights.size(); i++)
	{
		ImGui::PushID(i);
		ImGui::DragFloat3("Position", glm::value_ptr(m_Scene.Lights[i].Position), 0.1f);
		ImGui::Separator();
		ImGui::PopID();
	}
	ImGui::End();

	ImGui::Begin("Scene Settings");
	ImGui::Text("Scene Settings");
	ImGui::Separator();
	for (size_t i = 0; i < m_Scene.Spheres.size(); i++)
	{
		ImGui::PushID(i);
		ImGui::Text("Sphere %i", i);
		Sphere& sphere = m_Scene.Spheres[i];
		ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.1f);
		ImGui::DragFloat("Radius", &sphere.Radius);
		ImGui::ColorEdit3("Albedo", glm::value_ptr(sphere.Albedo));

		ImGui::Separator();

		ImGui::PopID();
	}
	ImGui::End();

	ImGui::Begin("Debug");
	ImGui::Text("Last Render: %.3fms", m_LastRenderTime);
	ImGui::End();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("Viewport");

	m_ViewportWidth = ImGui::GetContentRegionAvail().x;
	m_ViewportHeight = ImGui::GetContentRegionAvail().y;

	auto image = m_Renderer.GetFinalImage();
	if (image)
		ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth(), (float)image->GetHeight() }, ImVec2(0,1), ImVec2(1,0));

	ImGui::End();
	ImGui::PopStyleVar();

	Render();
}

//----------------------------------------------------------------------------
void Render()
{
	Timer timer;

	m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
	m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
	m_Renderer.Render(m_Scene, m_Camera);

	m_LastRenderTime = timer.ElapsedMillis();
}
private:
	Renderer m_Renderer;
	Camera m_Camera;
	Scene m_Scene;
	uint32_t* m_ImageData = nullptr;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
	float m_LastRenderTime = 0.0f;
	glm::vec3 LightPos;
};

//----------------------------------------------------------------------------
Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Ottocento Renderer";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}