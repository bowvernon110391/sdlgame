#ifndef __IMGUI_IMPL_BOWIE__
#define __IMGUI_IMPL_BOWIE__

#include "imgui.h"
#include "../Texture2d.h"

// just some basic interface
IMGUI_IMPL_API	bool ImGui_ImplBowie_Init();
IMGUI_IMPL_API	void ImGui_ImplBowie_Shutdown();
IMGUI_IMPL_API	void ImGui_ImplBowie_NewFrame();
IMGUI_IMPL_API	void ImGui_ImplBowie_RenderDrawData(ImDrawData* draw_data);

// unique per our implementation?
IMGUI_IMPL_API	bool ImGui_ImplBowie_CreateFontsTexture();
IMGUI_IMPL_API	void ImGui_ImplBowie_DestroyFontsTexture();
IMGUI_IMPL_API	void ImGui_ImplBowie_CreateDeviceObjects();
IMGUI_IMPL_API	void ImGui_ImplBowie_DestroyDeviceObjects();

Texture2D* getFontTexture();


#endif	// __IMGUI_IMPL_BOWIE__