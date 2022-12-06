//#pragma once
//#include "../imgui/imgui.h"
//#include "../imgui/imgui.h"
//#include "../imgui/imgui_impl_glfw.h"
//#include "../imgui/imgui_impl_opengl3.h"
//#include<glm/glm.hpp>
//
//class Gui {
//public:
//	Gui(GLFWwindow* window) {
//		const char* glsl_version = "#version 330";
//		IMGUI_CHECKVERSION();
//		ImGui::CreateContext();
//		ImGui::StyleColorsLight();
//		ImGui_ImplGlfw_InitForOpenGL(window, true);
//		ImGui_ImplOpenGL3_Init(glsl_version);
//	}
//	~Gui() {
//		ImGui_ImplOpenGL3_Shutdown();
//		ImGui_ImplGlfw_Shutdown();
//		ImGui::DestroyContext();
//	}
//	void window(Model& model) {
//		ImGui_ImplOpenGL3_NewFrame();
//		ImGui_ImplGlfw_NewFrame();
//		ImGui::NewFrame();
//
//		ImGui::Begin("Info");
//
//		ImGui::Text("Vertices: %d", model.validVertices);
//		ImGui::Text("Edges: %d", model.validEdges);
//		ImGui::Text("Faces: %d", model.validFaces);
//		bool button = ImGui::Button("Collapse Edge");
//		if (button) {
//			model.randomCollapse();
//		}
//		ImGui::End();
//
//		ImGui::Render();
//	}
//
//	void render() {
//		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
//	}
//};
