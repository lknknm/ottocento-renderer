#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"
#include "Walnut/ImGui/imnodes.h"

#include "Camera.h"
#include "Renderer.h"

#include <glm/gtc/type_ptr.hpp>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <dinput.h>

#include "imgui_internal.h"

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
ExampleLayer() : m_Camera(45.0f, 0.1f, 100.0f) 
{
	Material& RedMat = m_Scene.Materials.emplace_back(); RedMat.Name = "RedMat"; RedMat.Albedo = {1.0f, 0.0f, 0.0f}; RedMat.Roughness = 0.35f;
	Material& BlackMat = m_Scene.Materials.emplace_back(); BlackMat.Albedo = {1.0f, 1.0f, 1.0f}; BlackMat.Roughness = 0.35f; BlackMat.Metallic = true;
	Material& WhiteMat = m_Scene.Materials.emplace_back(); WhiteMat.Albedo = {1.0f, 1.0f, 1.0f}; WhiteMat.Roughness = 0.2f; WhiteMat.Metallic = true;
	{	
		Sphere sphere;
		sphere.Position = {0.0f, 0.0f, 0.0f};
		sphere.Radius = 1.0f;
		sphere.MaterialIndex = 2;
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
		Sphere sphere;
		sphere.Position = {0.0f, 0.0f, -2.1f};
		sphere.Radius = 0.5f;
		sphere.MaterialIndex = 2;
		m_Scene.Spheres.push_back(sphere);
	}
	{
		Light light;
		light.Position = {-0.8f, -0.8f, -1.6f};
		light.lightColor = {0.0f, 0.2f, 0.8f};
		light.isActive = true;
		light.Intensity = 31.8f;
		m_Scene.Lights.push_back(light);
	}
	{
		Light light;
		light.Position = {0.0f, 1.0f, 1.0f};
		light.lightColor = {1.0f, 0.0f, 0.0f};
		light.isActive = true;
		light.Intensity = 0.7f;
		m_Scene.Lights.push_back(light);
	}
	{
		Light light;
		light.Position = {7.2f, 14.6f, -16.7f};
		light.lightColor = {1.0f, 1.0f, 1.0f};
		light.isActive = true;
		light.Intensity = 15.0f;
		m_Scene.Lights.push_back(light);
	}
	SetupImGuiStyle();
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
	//----------------------------------------------------------------------------
	ImGui::Begin(ICON_MS_PUBLIC);
	ImGui::ColorEdit3("Background Color", glm::value_ptr(m_Scene.SkyColor.Albedo)); 
	if (ImGui::IsItemEdited()) { m_Renderer.ResetFrameIndex(); }
	ImGui::End();
	
	//----------------------------------------------------------------------------
	// Principled BRDF Node Editor - i.e. Material - configuration
	ImGui::Begin(ICON_MS_RADIO_BUTTON_CHECKED);
	ImNodes::BeginNodeEditor();
	for (size_t i = 0; i < m_Scene.Materials.size(); i++)
	{
		ImGui::PushItemWidth(144);
		ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(0x83, 0x18, 0x43, 255));
		ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(0x83, 0x18, 0x43, 255));
		ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(0x83, 0x18, 0x43, 255));
		ImNodes::PushColorStyle(ImNodesCol_NodeBackground, IM_COL32(0x17, 0x17, 0x17, 255));
		ImNodes::PushColorStyle(ImNodesCol_NodeBackgroundHovered, IM_COL32(0x17, 0x17, 0x17, 255));
		ImNodes::PushColorStyle(ImNodesCol_NodeBackgroundSelected, IM_COL32(0x17, 0x17, 0x17, 255));
		
		Material& material = m_Scene.Materials[i];
		ImNodes::SetNodeGridSpacePos(i, ImVec2(100, 100 + i*200));
		ImNodes::SetNodeDraggable(i, true);
		ImNodes::BeginNode(i);
		ImNodes::BeginNodeTitleBar();
		ImGui::Text("%s | Principled BRDF", material.Name.c_str());
		ImNodes::EndNodeTitleBar();
				     
		// Here we overwrite window value
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImRect backup = window->WorkRect;  
		window->WorkRect.Min.x = window->DC.CursorPos.x;
		window->WorkRect.Max.x = window->WorkRect.Min.x + 144;

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Base Color");
		ImGui::SameLine(0, 32);
		if (ImGui::ColorButton("Default Value", ImVec4(material.Albedo.r, material.Albedo.g, material.Albedo.b, 1), 0, ImVec2(144,16)))
			ImGui::OpenPopup("hi-picker");
		if (ImGui::BeginPopup("hi-picker"))
		{
			ImGui::ColorPicker3("##ColorPicker", glm::value_ptr(material.Albedo));
			if (ImGui::IsItemEdited()) { m_Renderer.ResetFrameIndex(); }
			ImGui::EndPopup();
		}

		// Roughness config
		ImGui::Text("Roughness");
		ImGui::SameLine(0, 30);
		ImGui::DragFloat( "##roughness", &material.Roughness, 0.001f, 0.0f, 1.0f); 
		if (ImGui::IsItemEdited()) { m_Renderer.ResetFrameIndex(); }
		
		// Metallic config
		ImGui::Text("Metallic");
		ImGui::SameLine(0, 50);
		ImGui::Checkbox("##metallic", &material.Metallic);
		if (ImGui::IsItemEdited()) { m_Renderer.ResetFrameIndex(); }
		
		ImGui::NewLine();
		if (ImGui::CollapsingHeader("Emission"))
		{
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Base Color");
			ImGui::SameLine(0, 32);
			if (ImGui::ColorButton("Default Value", ImVec4(material.EmissionColor.r,
																	material.EmissionColor.g,
																	material.EmissionColor.b,
																	1),
																	0,
																	ImVec2(144,20)))
				ImGui::OpenPopup("hi-picker");
			if (ImGui::BeginPopup("hi-picker"))
			{
				ImGui::ColorPicker3("##ColorPicker", glm::value_ptr(material.EmissionColor));
				if (ImGui::IsItemEdited()) { m_Renderer.ResetFrameIndex(); }
				ImGui::EndPopup();
			}
		
			ImGui::Text("Power");
			ImGui::SameLine(0, 60);
			ImGui::DragFloat( "##power", &material.EmissionPower, 0.001f, 0.0f, 1.0f); 
			if (ImGui::IsItemEdited()) { m_Renderer.ResetFrameIndex(); }
		}
		window->WorkRect = backup;  // recover correct value
		ImNodes::EndNode();
	}
	ImNodes::EndNodeEditor();
	
	ImGui::End();
	//----------------------------------------------------------------------------

	//----------------------------------------------------------------------------
	ImGui::Begin(ICON_MS_VIDEOCAM);

	ImGui::DragFloat3("Position", glm::value_ptr(m_Camera.m_Position), 0.1f);
	if (ImGui::IsItemEdited()) { m_Renderer.ResetFrameIndex(); }

	
	ImGui::DragFloat("VFOV", (&m_Camera.m_VerticalFOV), 0.1f, 1.0f, 90.0f, "%.3f");
	if (ImGui::IsItemEdited()) { m_Renderer.ResetFrameIndex(); m_Camera.RecalculateView(); m_Camera.RecalculateProjection(); }

	if (ImGui::CollapsingHeader("Depth of Field"))
	{
		ImGui::Checkbox("On", &m_Camera.m_DepthOfFieldOn);
		if (ImGui::IsItemEdited()) { m_Renderer.ResetFrameIndex(); m_Camera.RecalculateView(); m_Camera.RecalculateProjection(); }
		
		ImGui::Text("Angle");
		ImGui::SameLine(0, 51);
		ImGui::DragFloat("##angle", (&m_Camera.m_DefocusAngle), 0.1f, 0.0f, 90.0f, "%.3f");
		if (ImGui::IsItemEdited()) { m_Renderer.ResetFrameIndex(); m_Camera.RecalculateView(); m_Camera.RecalculateProjection(); }
		
		ImGui::Text("Distance");
		ImGui::SameLine(0, 32);
		ImGui::DragFloat("##dist", (&m_Camera.m_FocusDist), 0.1f, 0.0f, 90.0f, "%.3f");
		if (ImGui::IsItemEdited()) { m_Renderer.ResetFrameIndex(); m_Camera.RecalculateView(); m_Camera.RecalculateProjection(); }
	}
	
	// ImGui::Text("Camera Forward X: %f", m_Camera.GetDirection().x);
	// ImGui::Text("Camera Forward Y: %f", m_Camera.GetDirection().y);
	// ImGui::Text("Camera Forward Z: %f", m_Camera.GetDirection().z);
	
	ImGui::End();
	//----------------------------------------------------------------------------
	
	//----------------------------------------------------------------------------
	ImGui::Begin(ICON_MS_LIGHTBULB);
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
	//----------------------------------------------------------------------------

	//----------------------------------------------------------------------------
	ImGui::Begin(ICON_MS_VISIBILITY);
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
	//----------------------------------------------------------------------------

	//----------------------------------------------------------------------------
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("Viewport");

	m_ViewportWidth = ImGui::GetContentRegionAvail().x;
	m_ViewportHeight = ImGui::GetContentRegionAvail().y;

	auto image = m_Renderer.GetFinalImage();
	if (image)
		ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth(), (float)image->GetHeight() }, ImVec2(0,1), ImVec2(1,0));
	ImGui::End();
	//----------------------------------------------------------------------------
	
	ImGui::PopStyleVar();
	bool* p_open;
	FrameCounterOverlay(p_open, m_LastRenderTime);
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

	
void SetupImGuiStyle()
{
	// Purple Comfy style by RegularLunar from ImThemes
	ImGuiStyle& style = ImGui::GetStyle();
	style.Alpha = 1.0f;
	style.DisabledAlpha = 0.10f;
	style.WindowPadding = ImVec2(8.0f, 8.0f);
	style.WindowRounding = 10.0f;
	style.WindowBorderSize = 0.0f;
	style.WindowMinSize = ImVec2(30.0f, 30.0f);
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_Right;
	style.ChildRounding = 5.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 10.0f;
	style.PopupBorderSize = 0.0f;
	style.FramePadding = ImVec2(5.0f, 3.5f);
	style.FrameRounding = 5.0f;
	style.FrameBorderSize = 0.0f;
	style.ItemSpacing = ImVec2(5.0f, 4.0f);
	style.ItemInnerSpacing = ImVec2(5.0f, 5.0f);
	style.CellPadding = ImVec2(4.0f, 2.0f);
	style.IndentSpacing = 5.0f;
	style.ColumnsMinSpacing = 5.0f;
	style.ScrollbarSize = 15.0f;
	style.ScrollbarRounding = 9.0f;
	style.GrabMinSize = 15.0f;
	style.GrabRounding = 5.0f;
	style.TabRounding = 5.0f;
	style.TabBorderSize = 0.0f;
	style.TabMinWidthForCloseButton = 0.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

	style.Colors[ImGuiCol_Text]				        = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled]		        = ImVec4(1.0f, 1.0f, 1.0f, 0.3f);
	style.Colors[ImGuiCol_WindowBg]			        = ImVec4(0.05f, 0.05f, 0.05f, 1.0f);
	style.Colors[ImGuiCol_ChildBg]			        = ImVec4(1.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_PopupBg]			        = ImVec4(0.05f, 0.05f, 0.05f, 1.0f);
	style.Colors[ImGuiCol_Border]			        = ImVec4(0.50f, 0.30f, 1.0f, 0.50f);
	style.Colors[ImGuiCol_BorderShadow]		        = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_FrameBg]			        = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered]	        = ImVec4(0.35f, 0.40f, 0.50f, 0.50f);
	style.Colors[ImGuiCol_FrameBgActive]	        = ImVec4(0.50f, 0.30f, 1.0f, 0.50f);
	style.Colors[ImGuiCol_TitleBg]				    = ImVec4(0.05f, 0.05f, 0.05f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive]		    = ImVec4(0.05f, 0.05f, 0.05f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed]		    = ImVec4(0.25f, 0.25f, 0.25f, 0.0f);
	style.Colors[ImGuiCol_MenuBarBg]				= ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_ScrollbarBg]				= ImVec4(0.15f, 0.15f, 0.15f, 0.0f);
	style.Colors[ImGuiCol_ScrollbarGrab]			= ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered]		= ImVec4(0.23f, 0.23f, 0.23f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive]		= ImVec4(0.30f, 0.30f, 0.30f, 1.0f);
	style.Colors[ImGuiCol_CheckMark]				= ImVec4(0.50f, 0.30f, 1.0f, 0.50f);
	style.Colors[ImGuiCol_SliderGrab]				= ImVec4(0.50, 0.30f, 1.0f, 0.50f);
	style.Colors[ImGuiCol_SliderGrabActive]			= ImVec4(0.50f, 0.30f, 1.0f, 0.50f);
	style.Colors[ImGuiCol_Button]					= ImVec4(0.50f, 0.50f, 0.5f, 0.50f);
	style.Colors[ImGuiCol_ButtonHovered]			= ImVec4(0.50f, 0.30f, 1.0f, 0.50f);
	style.Colors[ImGuiCol_ButtonActive]				= ImVec4(0.50f, 0.30f, 1.0f, 0.50f);
	style.Colors[ImGuiCol_Header]					= ImVec4(0.05f, 0.05f, 0.05f, 0.0f);
	style.Colors[ImGuiCol_HeaderHovered]			= ImVec4(0.05f, 0.05f, 0.05f, 0.0f);
	style.Colors[ImGuiCol_HeaderActive]				= ImVec4(0.05f, 0.05f, 0.05f, 0.0f);
	style.Colors[ImGuiCol_Separator]				= ImVec4(0.50f, 0.30f, 1.0f, 0.50f);
	style.Colors[ImGuiCol_SeparatorHovered]			= ImVec4(0.50f, 0.30f, 1.0f, 0.50f);
	style.Colors[ImGuiCol_SeparatorActive]			= ImVec4(0.50f, 0.30f, 1.0f, 0.50f);
	style.Colors[ImGuiCol_ResizeGrip]				= ImVec4(0.50f, 0.30f, 1.0f, 0.50f);
	style.Colors[ImGuiCol_ResizeGripHovered]		= ImVec4(0.50f, 0.30f, 1.0f, 0.50f);
	style.Colors[ImGuiCol_ResizeGripActive]			= ImVec4(0.50f, 0.30f, 1.0f, 0.50f);
	style.Colors[ImGuiCol_Tab]						= ImVec4(0.05f, 0.05f, 0.05f, 0.50f);
	style.Colors[ImGuiCol_TabHovered]				= ImVec4(0.30f, 0.30f, 0.3f, 0.50f);
	style.Colors[ImGuiCol_TabActive]				= ImVec4(0.25f, 0.25f, 0.25f, 0.50f);
	style.Colors[ImGuiCol_TabUnfocused]				= ImVec4(0.0f, 0.45f, 1.0f, 0.0f);
	style.Colors[ImGuiCol_TabUnfocusedActive]		= ImVec4(0.25f, 0.25f, 0.25f, 0.0f);
	style.Colors[ImGuiCol_PlotLines]				= ImVec4(0.30f, 0.30f, 0.30f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered]			= ImVec4(0.50f, 0.30f, 1.0f, 0.50f);
	style.Colors[ImGuiCol_PlotHistogram]			= ImVec4(0.50f, 0.30f, 1.0f, 0.50f);
	style.Colors[ImGuiCol_PlotHistogramHovered]		= ImVec4(0.70f, 0.70f, 0.90f, 0.50f);
	style.Colors[ImGuiCol_TableHeaderBg]			= ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
	style.Colors[ImGuiCol_TableBorderStrong]		= ImVec4(0.50f, 0.30f, 1.0f, 0.50f);
	style.Colors[ImGuiCol_TableBorderLight]			= ImVec4(0.50f, 0.30f, 1.0f, 0.3f);
	style.Colors[ImGuiCol_TableRowBg]				= ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_TableRowBgAlt]			= ImVec4(1.0f, 1.0f, 1.0f, 0.035f);
	style.Colors[ImGuiCol_TextSelectedBg]			= ImVec4(0.50f, 0.30f, 1.0f, 0.50f);
	style.Colors[ImGuiCol_DragDropTarget]			= ImVec4(1.0f, 1.0f, 0.0f, 0.90f);
	style.Colors[ImGuiCol_NavHighlight]				= ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingHighlight]	= ImVec4(1.0f, 1.0f, 1.0f, 0.71f);
	style.Colors[ImGuiCol_NavWindowingDimBg]		= ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	style.Colors[ImGuiCol_ModalWindowDimBg]			= ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

	// Draws vertical text. The position is the bottom left of the text rect.
inline void AddTextVertical(ImDrawList* DrawList, const char *text, ImVec2 pos, ImU32 text_color)
{
    pos.x = IM_ROUND(pos.x);
    pos.y = IM_ROUND(pos.y);
    ImFont *font = GImGui->Font;
    const ImFontGlyph *glyph;
    char c;
    ImGuiContext& g = *GImGui;
    ImVec2 text_size = ImGui::CalcTextSize(text);
    while ((c = *text++))
    {
        glyph = font->FindGlyph(c);
        if (!glyph) continue;

        DrawList->PrimReserve(6, 4);
        DrawList->PrimQuadUV(
                ImVec2(pos.x + glyph->Y0, pos.y - glyph->X0),
                ImVec2(glyph->Y0, -glyph->X1),
                ImVec2(glyph->Y1, -glyph->X1),
                ImVec2(glyph->Y1, -glyph->X0),

                ImVec2(glyph->U0, glyph->V0),
                ImVec2(glyph->U1, glyph->V0),
                ImVec2(glyph->U1, glyph->V1),
                ImVec2(glyph->U0, glyph->V1),
                    text_color);
        pos.y -= glyph->AdvanceX;
    }
}
	
//----------------------------------------------------------------------------
static void FrameCounterOverlay(bool* p_open, float RenderTime)
{
    static int location = 0;
    ImGuiIO& io = ImGui::GetIO();
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    if (location >= 0)
    {
        const float PAD = 14.0f;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
        ImVec2 work_size = viewport->WorkSize;
        ImVec2 window_pos, window_pos_pivot;
        window_pos.x = (location & 1) ? (work_pos.x + work_size.x - 1) : (work_pos.x + 1);
        window_pos.y = (location & 2) ? (work_pos.y + work_size.y - 18) : (work_pos.y + 18);
        window_pos_pivot.x = (location & 1) ? 1.0f : 0.0f;
        window_pos_pivot.y = (location & 2) ? 1.0f : 0.0f;
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        window_flags |= ImGuiWindowFlags_NoMove;
    }
    else if (location == -2)
    {
        // Center window
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        window_flags |= ImGuiWindowFlags_NoMove;
    }
    ImGui::SetNextWindowBgAlpha(0.00f); // Transparent background
    if (ImGui::Begin("Example: Simple overlay", p_open, window_flags))
            ImGui::Text("Last Render: %.3f ms", RenderTime);
    ImGui::End();
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
	spec.Width = 1280;
	spec.Height = 720;
	
	Walnut::Application* app = new Walnut::Application(spec);
	std::shared_ptr<ExampleLayer> exampleLayer = std::make_shared<ExampleLayer>();
	app->PushLayer(exampleLayer);
	app->SetMenubarCallback([app, exampleLayer]()
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
