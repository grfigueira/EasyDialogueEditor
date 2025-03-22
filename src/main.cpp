/******************************************************************************
    Created by Guilherme Figueira, 2025

    My contacts, feel free to reach out:
    - Github: https://github.com/grfigueira
    - LinkedIn: https://www.linkedin.com/in/grfigueira/
 ******************************************************************************/

 #include "node_editor.h"

 #include <imgui.h>
 #include <imgui_impl_sdl2.h>
 #include <imgui_impl_opengl3.h>
 #include <imgui_internal.h>
 #include <imnodes.h>
 #include <SDL2/SDL.h>
 #if defined(IMGUI_IMPL_OPENGL_ES2)
 #include <SDL2/SDL_opengles2.h>
 #else
 #include <SDL2/SDL_opengl.h>
 #endif

#include "Utils.h"
#include "RobotoFont.hpp"
#include <iostream>


 
 int main(int, char**)
 {
     if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
     {
         printf("Error: %s\n", SDL_GetError());
         return -1;
     }
 
     // Decide GL+GLSL versions
 #if defined(IMGUI_IMPL_OPENGL_ES2)
     // GL ES 2.0 + GLSL 100
     const char* glsl_version = "#version 100";
     SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
     SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
     SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
     SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
 #elif defined(__APPLE__)
     // GL 3.2 Core + GLSL 150
     const char* glsl_version = "#version 150";
     SDL_GL_SetAttribute(
         SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
     SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
     SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
     SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
 #else
     // GL 3.0 + GLSL 130
     const char* glsl_version = "#version 130";
     SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
     SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
     SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
     SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
 #endif
 
     // Create window with graphics context
     SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
     SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
     SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
     SDL_WindowFlags window_flags =
         (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
     SDL_Window* window = SDL_CreateWindow(
         "EasyDialogueEditor v0.7 [beta]",
         SDL_WINDOWPOS_CENTERED,
         SDL_WINDOWPOS_CENTERED,
         1440,
         900,
         window_flags);
     if (!window) {
         printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
         return -1;
     }
 
     SDL_GLContext gl_context = SDL_GL_CreateContext(window);
     SDL_GL_MakeCurrent(window, gl_context);
     SDL_GL_SetSwapInterval(1); // Enable vsync
 
     // Setup Dear ImGui context
     IMGUI_CHECKVERSION();
     ImGui::CreateContext();
     ImGuiIO& io = ImGui::GetIO();
     io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
     io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
     io.IniFilename = nullptr;

     ImFontConfig font_cfg;
     font_cfg.FontDataOwnedByAtlas = false;
     io.Fonts->AddFontFromMemoryTTF((void*)_acRoboto_Medium, sizeof(_acRoboto_Medium), 16.0f, &font_cfg);
 
     ImNodes::CreateContext();
     ede::NodeEditorInitialize();
 
     // Setup Dear ImGui style
     ImGui::StyleColorsDark();
 
     // Setup Platform/Renderer backends
     ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
     ImGui_ImplOpenGL3_Init(glsl_version);
 
     ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
     
     bool done = false;
     bool hasRootSpawned = false;
     ImGui::LoadIniSettingsFromMemory(DEFAULT_INI);
	 ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
     
     // Main loop
     while (!done)
     {
		 if (!ImGui::GetIO().BackendPlatformUserData) {
			 std::cout << "Warning: BackendPlatformUserData is null!" << std::endl;
		 }

		 // You can also check the context
		 if (!ImGui::GetCurrentContext()) {
			 std::cout << "Warning: ImGui context is null!" << std::endl;
		 }
         SDL_Event event;

         /*************************************************************
         *                   Process SDL events
         *************************************************************/

         while (SDL_PollEvent(&event))
         {
             ImGui_ImplSDL2_ProcessEvent(&event);
             if (event.type == SDL_QUIT)
                 done = true;
             if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                 event.window.windowID == SDL_GetWindowID(window))
                 done = true;
			 if (event.type == SDL_KEYDOWN)
			 {
				 if ((event.key.keysym.mod & KMOD_CTRL) && event.key.keysym.sym == SDLK_x)
				 {
                     ede::FileDialogs::ExportDialogueJsonFile();
				 }
				 if ((event.key.keysym.mod & KMOD_CTRL) && event.key.keysym.sym == SDLK_s)
				 {
					 ede::FileDialogs::SaveStateJson();
				 }
				 if (((event.key.keysym.mod & KMOD_CTRL) && event.key.keysym.sym == SDLK_r))
				 {
					 ImGui::LoadIniSettingsFromMemory(DEFAULT_INI);
					 ImGui::MarkIniSettingsDirty();
                     ede::marked_for_UI_reset = false;
				 }
                 if ((event.key.keysym.mod & KMOD_CTRL) && event.key.keysym.sym == SDLK_o) 
                 {
                     ede::FileDialogs::LoadStateJson();
                 }
				 if ((event.key.keysym.mod & KMOD_CTRL) && event.key.keysym.sym == SDLK_n)
				 {
					 ede::ShowNewFilePopup();
				 }
			 }
         }
 
         // Start the Dear ImGui frame
         ImGui_ImplOpenGL3_NewFrame();
         ImGui_ImplSDL2_NewFrame();
         ImGui::NewFrame();

         if (ImNodes::IsEditorHovered() && ImGui::GetIO().MouseWheel != 0)
         {
             float zoom = ImNodes::EditorContextGetZoom() + ImGui::GetIO().MouseWheel * 0.1f;
             ImNodes::EditorContextSetZoom(zoom, ImGui::GetMousePos());
         }
 
         // EasyDialogEditor "entry point"
         ede::NodeEditorShow();
         
         if (!hasRootSpawned) {
             hasRootSpawned = true;
             ede::InitializeConversation();
         }
 
         // Rendering
         ImGui::Render();
         glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
         glClearColor(
             clear_color.x * clear_color.w,
             clear_color.y * clear_color.w,
             clear_color.z * clear_color.w,
             clear_color.w);
         glClear(GL_COLOR_BUFFER_BIT);
         ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
         SDL_GL_SwapWindow(window);

         if (ede::marked_for_UI_reset) {
			 ImGui::LoadIniSettingsFromMemory(DEFAULT_INI);
			 ImGui::MarkIniSettingsDirty();
			 ede::marked_for_UI_reset = false;
         }
     }
 
     // Cleanup
     ImGui::PopStyleVar(1);
     ImGui_ImplOpenGL3_Shutdown();
     ImGui_ImplSDL2_Shutdown();
     ede::NodeEditorShutdown();
     ImNodes::DestroyContext();
     ImGui::DestroyContext();
 
     SDL_GL_DeleteContext(gl_context);
     SDL_DestroyWindow(window);
     SDL_Quit();
 
     return 0;
 }