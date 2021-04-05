#ifndef __IMGUI_IMPL_BOWIE__
#define __IMGUI_IMPL_BOWIE__

#include <SDL.h>
#include "imgui.h"
#include "../Texture2d.h"

typedef struct _TouchData {
	_TouchData() {
		cX = cY = 0;
		buttonDown[0] = false;
		buttonDown[1] = false;
		buttonDown[2] = false;
	}
	int cX, cY;
	bool buttonDown[3];
} TouchData;

// just some basic interface
IMGUI_IMPL_API	bool ImGui_ImplBowie_Init(SDL_Window *wnd);
IMGUI_IMPL_API	void ImGui_ImplBowie_Shutdown();
IMGUI_IMPL_API	void ImGui_ImplBowie_NewFrame();
IMGUI_IMPL_API	void ImGui_ImplBowie_RenderDrawData(ImDrawData* draw_data);

// unique per our implementation?
IMGUI_IMPL_API	bool ImGui_ImplBowie_CreateFontsTexture();
IMGUI_IMPL_API	void ImGui_ImplBowie_DestroyFontsTexture();
IMGUI_IMPL_API	void ImGui_ImplBowie_CreateDeviceObjects();
IMGUI_IMPL_API	void ImGui_ImplBowie_DestroyDeviceObjects();
IMGUI_IMPL_API  bool ImGui_ImplBowie_ProcessEvent(SDL_Event* e);

IMGUI_IMPL_API TouchData* ImGui_ImplBowie_GetTouchData();
IMGUI_IMPL_API void ImGui_ImplBowie_InjectTouchHandler();


Texture2D* getFontTexture();


#endif	// __IMGUI_IMPL_BOWIE__