/*
	GUI
	part of the ToyGraph project

	创建：2022.12.7

*/
#pragma once
#include <ToyGraph/Scene/Scene.h>
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"
#include<glm/glm.hpp>
/*
	每个Scene可以实例化一个GUI
	调用方式参考VCloudScene.cpp
	
	TODO: 析构的时候会有bug，好像与键盘key的buffer存在访问冲突
*/
class GUI 
{
protected:
	Scene* scene;
	ImGuiContext* mcontex;
public:
	GUI(Scene*pscene,GLFWwindow*window) {
		scene = pscene;
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 130");
	};
	~GUI() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		
		ImGui::DestroyContext();
	};
	
	virtual void render() 
	{
		
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin("Scene control");
		scene->setGUI();
		//ImGui::TextColored(ImVec4(1, 1, 0, 1), "Other controls");
	/*	if (ImGui::DragFloat3("Light Position",  ) 
		ImGui::InputFloat3("Camera Position", &(scene.camera->getPosition), 7);
		ImGui::ColorEdit3("Light color", (float*)&scene.lightColor);
		ImGui::ColorEdit3("Fog color", (float*)&scene.fogColor);
		ImGui::SliderFloat("Camera speed", &scene.cam->MovementSpeed, 0.0, SPEED * 3.0);*/

		//ImGui::Checkbox("Wireframe mode", &scene.wireframe);

		//if (ImGui::Button("Generate seed"))
		//	scene.seed = genRandomVec3();
		//ImGui::SameLine();
		//ImGui::Text("Generate a new seed");
		/*ImGui::SameLine();
		if (ImGui::Button("Use default seed"));*/
			//scene.seed = glm::vec3(0.0, 0.0, 0.0);
		//ImGui::Text("Scene average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();

		//actual drawing
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	};
	
private:
};

