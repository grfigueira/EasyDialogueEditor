/******************************************************************************
 * Copyright (c) 2025 Guilherme Figueira
 * MIT License - See LICENSE file in the project root for details
 * Contact: g.figueira.2002@gmail.com
 ******************************************************************************/

 #include "node_editor.h"

 #include <imgui.h>
 #include <imgui_impl_sdl2.h>
 #include <imgui_impl_opengl3.h>
 #include <imnodes.h>
 #include <SDL2/SDL.h>
 #if defined(IMGUI_IMPL_OPENGL_ES2)
 #include <SDL2/SDL_opengles2.h>
 #else
 #include <SDL2/SDL_opengl.h>
 #endif
 
 #include <cstdio>
 #include <iostream> 
 #include <thread>

#include "Utils.h"


 
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
         "EasyDialogEditor v0.1.0 [beta]",
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
	 static const char* DEFAULT_INI = R"INI(
[Window][MainDockspace]
Pos=0,0
Size=1440,900
Collapsed=0

[Window][Debug##Default]
Pos=60,60
Size=400,400
Collapsed=0

[Window][Graph Editor]
Pos=0,0
Size=1057,900
Collapsed=0
DockId=0x00000001,0

[Window][Story Graph Info]
Pos=1059,0
Size=381,900
Collapsed=0
DockId=0x00000002,0

[Window][Dear ImGui Demo]
Pos=425,72
Size=550,680
Collapsed=0

[Docking][Data]
DockSpace     ID=0x5F4274ED Window=0x1F1D7494 Pos=0,0 Size=1440,900 Split=X
  DockNode    ID=0x00000003 Parent=0x5F4274ED SizeRef=703,900 Selected=0xEEAF2C9D
  DockNode    ID=0x00000004 Parent=0x5F4274ED SizeRef=735,900 Split=X
    DockNode  ID=0x00000001 Parent=0x00000004 SizeRef=1057,900 CentralNode=1 Selected=0xEEAF2C9D
    DockNode  ID=0x00000002 Parent=0x00000004 SizeRef=381,900 Selected=0xBCDEDAD9
)INI";

     ImGui::LoadIniSettingsFromMemory(DEFAULT_INI);
     
     // Main loop
     while (!done)
     {
         SDL_Event event;
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
                     ede::FileDialogs::ExportJsonFile();
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
     }
 
     // Cleanup
     ImGui_ImplOpenGL3_Shutdown();
     ImGui_ImplSDL2_Shutdown();
     ede::NodeEditorShutdown(); // Make sure this function exists
     ImNodes::DestroyContext();
     ImGui::DestroyContext();
 
     SDL_GL_DeleteContext(gl_context);
     SDL_DestroyWindow(window);
     SDL_Quit();
 
     return 0;
 }