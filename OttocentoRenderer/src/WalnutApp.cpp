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
	Material& RedMat = m_Scene.Materials.emplace_back(); RedMat.Albedo = {1.0f, 0.0f, 0.0f}; RedMat.Roughness = 0.75f;
	Material& BlackMat = m_Scene.Materials.emplace_back(); BlackMat.Albedo = {0.0f, 0.0f, 0.0f}; BlackMat.Roughness = 0.2f; BlackMat.Metallic = true;

	Material& LightMaterial = m_Scene.Materials.emplace_back(); 
	LightMaterial.Albedo = {0.8f, 0.5f, 0.2f}; 
	LightMaterial.Roughness = 0.1f; 
	LightMaterial.EmissionColor = LightMaterial.Albedo; 
	LightMaterial.EmissionPower = 2.0f;

	{	
		Sphere sphere;
		sphere.Position = {0.0f, 0.0f, 0.0f};
		sphere.Radius = 1.0f;
		sphere.MaterialIndex = 0;
		m_Scene.Spheres.push_back(sphere);
	}
	
	{
		Sphere sphere;
		sphere.Position = {0.0f, -101.0f, 0.0f};
		sphere.Radius = 100.0f;
		sphere.MaterialIndex = 1;
		m_Scene.Spheres.push_back(sphere);
	}
	{
		Light light;
		light.Position = {-1.0f, -1.0f, -1.0f};
		light.Radius = 0.5f;
		m_Scene.Lights.push_back(light);
	}
	{
		Light light;
		light.Position = {0.0f, 1.0f, 1.0f};
		light.Radius = 0.2f;
		light.isActive = true;
		m_Scene.Lights.push_back(light);
	}
}

//----------------------------------------------------------------------------
virtual void OnUpdate(float ts) override
{
	if (m_Camera.OnUpdate(ts))
		m_Renderer.ResetFrameIndex();
}

//----------------------------------------------------------------------------
virtual void OnUIRender() override
{
	ImGui::Begin("Environment");
	ImGui::ColorEdit3("Background Color", glm::value_ptr(m_Scene.SkyColor.Albedo)); 
	if (ImGui::IsItemEdited()) { m_Renderer.ResetFrameIndex(); }
	ImGui::End();

	ImGui::Begin("Light Settings");
	ImGui::Text("Light Settings");
	ImGui::Separator();
	for (size_t i = 0; i < m_Scene.Lights.size(); i++)
	{
		ImGui::PushID(i);

		ImGui::Checkbox("Active", &m_Scene.Lights[i].isActive); 
		if (ImGui::IsItemEdited()) { m_Renderer.ResetFrameIndex(); }
		
		ImGui::ColorEdit3("Light Color", glm::value_ptr(m_Scene.Lights[i].lightColor)); 
		if (ImGui::IsItemEdited()) { m_Renderer.ResetFrameIndex(); }

		ImGui::DragFloat3("Position", glm::value_ptr(m_Scene.Lights[i].Position), 0.1f);
		if (ImGui::IsItemEdited()) { m_Renderer.ResetFrameIndex(); }
		
		ImGui::DragFloat("Intensity", (&m_Scene.Lights[i].Intensity), 0.1f, 0.0f, 5000.0f, "%.3f");
		if (ImGui::IsItemEdited()) { m_Renderer.ResetFrameIndex(); }

		ImGui::Separator();
		ImGui::PopID();
	}
	if (ImGui::Button("Add Light"))
	{
		Light light;
		light.Position = {1.0f, 1.0f, 0.8f};
		light.Radius = 0.5f;
		m_Scene.Lights.push_back(light);
	}
	ImGui::End();

	ImGui::Begin("Scene Settings");
	ImGui::Text("Scene Settings");
	ImGui::Separator();
	for (size_t i = 0; i < m_Scene.Spheres.size(); i++)
	{
		ImGui::PushID(i);
		ImGui::Text("Sphere %i", i);
		ImGui::Checkbox("Visible", &m_Scene.Spheres[i].isVisible); if (ImGui::IsItemEdited()) { m_Renderer.ResetFrameIndex(); }
		Sphere& sphere = m_Scene.Spheres[i];

		ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.1f); 
		if (ImGui::IsItemEdited()) { m_Renderer.ResetFrameIndex(); }

		ImGui::DragFloat("Radius", &sphere.Radius); 
		if (ImGui::IsItemEdited()) { m_Renderer.ResetFrameIndex(); }

		ImGui::DragInt("Material", &sphere.MaterialIndex, 1.0f, 0, (int)m_Scene.Materials.size() - 1); 
		if (ImGui::IsItemEdited()) { m_Renderer.ResetFrameIndex(); }

		ImGui::Separator();
		ImGui::PopID();
	}
	if (ImGui::Button("Add Sphere"))
	{
		Sphere sphere;
		sphere.Position = {1.0f, 0.0f, -5.0f};
		sphere.MaterialIndex = 1;
		sphere.Radius = 0.5f;
		m_Scene.Spheres.push_back(sphere);
	}
	ImGui::End();

	ImGui::Separator();
	ImGui::Begin("Material Settings");
	ImGui::Text("Material Settings");
	ImGui::Separator();
	for (size_t i = 0; i < m_Scene.Materials.size(); i++)
	{
		ImGui::PushID(i);

		ImGui::Text("Material %i", i);
		Material& material = m_Scene.Materials[i];

		ImGui::ColorEdit3("Albedo", glm::value_ptr(material.Albedo)); 
		if (ImGui::IsItemEdited()) { m_Renderer.ResetFrameIndex(); }

		ImGui::DragFloat("Roughness", &material.Roughness, 0.05f, 0.0f, 1.0f); 
		if (ImGui::IsItemEdited()) { m_Renderer.ResetFrameIndex(); }

		ImGui::Checkbox("Metallic", &material.Metallic); 
		if (ImGui::IsItemEdited()) { m_Renderer.ResetFrameIndex(); }

		ImGui::ColorEdit3("Emission Color", glm::value_ptr(material.EmissionColor)); 
		if (ImGui::IsItemEdited()) { m_Renderer.ResetFrameIndex(); }

		ImGui::DragFloat("Emission Power", &material.EmissionPower, 0.05f, 0.0f, FLT_MAX); 
		if (ImGui::IsItemEdited()) { m_Renderer.ResetFrameIndex(); }
		
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