#include "imgui_impl_bowie.h"
#include "../Texture2d.h"
#include "../Shader.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

static Texture2D* fontTexture = 0;
static Shader* fontShader = 0;
static ShaderData* fontShaderData = 0;
static glm::mat4 projMat;
static GLuint vboHandle;
static GLuint iboHandle;
static bool showVirtualKeyboard = false;
static SDL_Window* wndApp = 0;
static TouchData touchData;
static bool useTouchScreen = false;
static int width = 0;
static int height = 0;

Texture2D* getFontTexture() {
	return fontTexture;
}

TouchData* ImGui_ImplBowie_GetTouchData() {
	return &touchData;
}

bool ImGui_ImplBowie_Init(SDL_Window *wnd) {
	SDL_Log("Initializing bowie's IMGUI Implementation...");

	std::string platform = SDL_GetPlatform();
	if (platform == "Android" || platform == "iOS") {
		useTouchScreen = true;
	}

	wndApp = wnd;

	SDL_GetWindowSize(wnd, &width, &height);

	ImGuiIO& io = ImGui::GetIO();
	io.BackendRendererName = "imgui_impl_bowie";

	// create device objects here!
	SDL_Log("Creating device objects...");
	ImGui_ImplBowie_CreateDeviceObjects();

	return (NULL != fontTexture) && (NULL != fontShader);
}

void ImGui_ImplBowie_Shutdown() {
	ImGui_ImplBowie_DestroyDeviceObjects();
}

void ImGui_ImplBowie_InjectTouchHandler() {
	if (useTouchScreen) {
		auto& io = ImGui::GetIO();

		io.MousePos = ImVec2(touchData.cX, touchData.cY);
		io.MouseDown[0] = touchData.buttonDown[0];
	}
}

void ImGui_ImplBowie_NewFrame() {
	static bool lastKeyboardShown = false;

	// track the keyboard shown flag
	bool keyboardShown = SDL_IsScreenKeyboardShown(wndApp);

	// for now, do nothing
	//SDL_Log("Imgui_Bowie_NewFrame()");
	// show virtual keyboard when needed
	ImGuiIO& io = ImGui::GetIO();

	// gotta check some shiet
	if (io.WantTextInput && !keyboardShown) {
		// text input wanted, keyboard not shown, show it
		SDL_StartTextInput();
	}

	// maybe no longer needed
	// or still in text mode when not needed
	if (!io.WantTextInput && (keyboardShown || SDL_IsTextInputActive())) {
		SDL_StopTextInput();
	}

	// stop text input if now keyboard is hidden but last time it was shown
	if (!keyboardShown && lastKeyboardShown) {
		ImGui::SetWindowFocus(nullptr);
	}

	// track last keyboard shown value
	lastKeyboardShown = keyboardShown;

	// show log status?
	static int backspaceCounter = 0;
	static int returnCounter = 0;
	bool specialHandleMode = io.WantTextInput && SDL_IsScreenKeyboardShown(wndApp);

	if (specialHandleMode) {
		//SDL_Log("BS: %d", io.KeysDown[SDL_SCANCODE_BACKSPACE]);
		// gotta check if we're doing more than one frame already
		if (io.KeysDown[SDL_SCANCODE_BACKSPACE]) {
			////SDL_Log("BACKSPACE IS ON WITH COUNTER %d", backspaceCounter);
			if (++backspaceCounter > 1) {
				//SDL_Log("BACKSPACE OFF @ %d", backspaceCounter);
				backspaceCounter = 0;
				io.KeysDown[SDL_SCANCODE_BACKSPACE] = false;
			}
		}

		// handle return key separately
		if (io.KeysDown[SDL_SCANCODE_RETURN]) {
			if (++returnCounter > 1) {
				returnCounter = 0;
				io.KeysDown[SDL_SCANCODE_RETURN] = false;
			}
		}
	}
}

bool ImGui_ImplBowie_CreateFontsTexture() {
	if (!fontTexture) {
		// grab texture data from imgui
		ImGuiIO& io = ImGui::GetIO();

		unsigned char* pixels;
		int w, h;

		io.Fonts->GetTexDataAsRGBA32(&pixels, &w, &h);

		fontTexture = new Texture2D();
		fontTexture->width = w;
		fontTexture->height = h;
		fontTexture->texData = pixels;
		fontTexture->useMipmap = false;
		fontTexture->minFilter = GL_NEAREST;

		if (!fontTexture->upload()) {
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed creating font texture!");
			delete fontTexture;
			fontTexture = NULL;
			return false;
		}
		else {
			SDL_Log("Font texture created.");
			fontTexture->texData = NULL;
			// set font texture data
			io.Fonts->SetTexID((ImTextureID)(intptr_t)fontTexture->texId);
			fontShaderData = (new ShaderData())->fillTextureSlot(0, fontTexture);
			return true;
		}
	}
	return true;
}

// create font shader for our implementation
static void ImGui_ImplBowie_CreateFontShader() {
	const char* vertShader = R"(
uniform mat4 m_projection;

attribute vec2 position;
attribute vec2 uv;
attribute vec4 color;

varying vec4 vColor;
varying vec2 vTexcoord;

void main() {
    vColor = color;
    vTexcoord = uv;
    gl_Position = m_projection * vec4(position.xy, 0, 1);
}
    )";

	const char* fragShader =
	        "#ifdef GL_ES\n"
            "precision mediump float;\n"
            "#endif\n"
            "uniform sampler2D texture0;\n"
            "varying vec4 vColor;\n"
            "varying vec2 vTexcoord;\n"
            "void main() {\n"
            "vec4 color = texture2D(texture0, vTexcoord);\n"
            "gl_FragColor = vColor * color;\n"
            "}\n";

	SDL_Log("Font Shader: %s", fragShader);

	if (!fontShader) {
		//fontShader = Shader::loadShaderFromSources(vertShader, strlen(vertShader), fragShader, strlen(fragShader));
		fontShader = Shader::fromMemory(vertShader, strlen(vertShader), fragShader, strlen(fragShader));

		// setup some uniform
		if (fontShader) {
			SDL_Log("Font shader loaded");
			fontShader->printDebug();
		}
		else {
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed loading font shader!");
		}
	}
}

static void ImGui_ImplBowie_DestroyFontShader() {
	if (fontShader) {
		delete fontShader;
		fontShader = 0;
	}
}

void ImGui_ImplBowie_CreateDeviceObjects() {
	// just create texture and shader
	ImGui_ImplBowie_CreateFontsTexture();
	ImGui_ImplBowie_CreateFontShader();

	// init vbo and ibo
	glGenBuffers(1, &vboHandle);
	glGenBuffers(1, &iboHandle);
}

// cleanup functions
void ImGui_ImplBowie_DestroyFontsTexture() {
	if (fontTexture) {
		delete fontTexture;
		fontTexture = 0;
	}
	if (fontShaderData) {
		delete fontShaderData;
		fontShaderData = 0;
	}
}

void ImGui_ImplBowie_DestroyDeviceObjects() {
	ImGui_ImplBowie_DestroyFontsTexture();
	ImGui_ImplBowie_DestroyFontShader();

	if (vboHandle) {
		glDeleteBuffers(1, &vboHandle); 
		vboHandle = 0;
	}

	if (iboHandle) {
		glDeleteBuffers(1, &iboHandle);
		iboHandle = 0;
	}
}

// drawing function!
// prepare rendering
static void Imgui_ImplBowie_SetupRenderState(int fb_width, int fb_height, ImDrawData* draw_data) {
	// setup orthogonal matrix
	projMat = glm::ortho(0.0f, (float)fb_width, (float)fb_height, 0.0f, -1.0f, 1.0f);

	// setup gl state
	glEnable(GL_BLEND);
	glBlendEquation(GL_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_SCISSOR_TEST);
	glEnable(GL_TEXTURE_2D);

	glViewport(0, 0, fb_width, fb_height);

	// setup shader
	fontShader->bind();
	fontShaderData->setupShader(fontShader, nullptr);	// setupData per material
	// projection matrix
	glUniformMatrix4fv(fontShader->getUniformLocation(Shader::UniformLoc::m_projection), 1, false, glm::value_ptr(projMat));
	
	// setup buffers
	glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboHandle);

	int position_loc = fontShader->getAttribLocation(Shader::AttribLoc::position);
	int color_loc = fontShader->getAttribLocation(Shader::AttribLoc::color);
	int uv_loc = fontShader->getAttribLocation(Shader::AttribLoc::uv);

	glEnableVertexAttribArray(position_loc);
	glEnableVertexAttribArray(color_loc);
	glEnableVertexAttribArray(uv_loc);

	glVertexAttribPointer(position_loc, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
	glVertexAttribPointer(color_loc, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, col));
	glVertexAttribPointer(uv_loc, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));

	
}

void ImGui_ImplBowie_RenderDrawData(ImDrawData* draw_data) {
	int fbWidth = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
	int fbHeight = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);

	if (fbWidth <= 0 || fbHeight <= 0)
		return;

	// for now, just log some shit perhaps?
	//SDL_Log("Imgui_Bowie_RenderDrawData(): w = %d, h = %d, list = %d", fbWidth, fbHeight, draw_data->CmdListsCount);

	Imgui_ImplBowie_SetupRenderState(fbWidth, fbHeight, draw_data);

	// now render them all?
	for (int n = 0; n < draw_data->CmdListsCount; n++) {
		const ImDrawList* cmd_list = draw_data->CmdLists[n];

		// buffer vertex data
		glBufferData(
			GL_ARRAY_BUFFER, 
			(GLsizeiptr)(cmd_list->VtxBuffer.Size * sizeof(ImDrawVert)), 
			(const GLvoid*)cmd_list->VtxBuffer.Data, 
			GL_STREAM_DRAW
		);
		// buffer index data
		glBufferData(
			GL_ELEMENT_ARRAY_BUFFER, 
			(GLsizeiptr)(cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx)), 
			(const GLvoid*)cmd_list->IdxBuffer.Data, 
			GL_STREAM_DRAW
		);

		// draw command
		for (int cmdIdx = 0; cmdIdx < cmd_list->CmdBuffer.Size; cmdIdx++) {
			const ImDrawCmd* pCmd = &cmd_list->CmdBuffer[cmdIdx];

			if (pCmd->UserCallback != NULL) {
				// draw command
				if (pCmd->UserCallback == ImDrawCallback_ResetRenderState) {
					Imgui_ImplBowie_SetupRenderState(fbWidth, fbHeight, draw_data);
				}
				else {
					// custom callback
					pCmd->UserCallback(cmd_list, pCmd);
				}
			}
			else {
				// draw data
				// do scissor test (might show incorrect though)
				ImVec4 clipRect = pCmd->ClipRect;

				//SDL_Log("Cliprect: %.2f %.2f %.2f %.2f", clipRect.x, clipRect.y, clipRect.z, clipRect.w);

				// only draw if within view
				if (clipRect.x < fbWidth && clipRect.y < fbHeight && clipRect.z >= 0 && clipRect.w >= 0) {
					// do scissor test
					glScissor(clipRect.x, (int)(fbHeight - clipRect.w), (int)(clipRect.z - clipRect.x), (int)(clipRect.w - clipRect.y));
					// bind texture data
					glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pCmd->TextureId);
					// draw elements
					glDrawElements(
						GL_TRIANGLES,
						pCmd->ElemCount,
						GL_UNSIGNED_SHORT,
						(const GLvoid*)(pCmd->IdxOffset * sizeof(ImDrawIdx))
					);
				}
			}
		}
	}

	// disable blending
	glDisable(GL_BLEND);
	glDisable(GL_SCISSOR_TEST);
}

bool ImGui_ImplBowie_ProcessEvent(SDL_Event* e) {
	ImGuiIO& io = ImGui::GetIO();
	bool specialHandleMode = io.WantTextInput && SDL_IsScreenKeyboardShown(wndApp);

	// check backspace key status
	// now handle specific event here
	switch (e->type) {
	case SDL_KEYUP:
		// prevent imgui to handle backspace + enter release
		if (specialHandleMode) {
			if (e->key.keysym.scancode == SDL_SCANCODE_BACKSPACE 
				|| e->key.keysym.scancode == SDL_SCANCODE_RETURN) {
				return true;
			}
		}
		break;
	case SDL_KEYDOWN:
		// handle special text input for mobile devices
		if (specialHandleMode) {
			// okay, if it's return key, add character
			/*if (e->key.keysym.scancode == SDL_SCANCODE_RETURN) {
				io.AddInputCharacter('\n');
				return true;
			}*/

			// if it's backspace, do something else
			if (e->key.keysym.scancode == SDL_SCANCODE_BACKSPACE
				|| e->key.keysym.scancode == SDL_SCANCODE_RETURN) {
				//SDL_Log("BACKSPACE oldstate %d", io.KeysDown[SDL_SCANCODE_BACKSPACE]);
				//SDL_Log("Forcing BACKSPACE!");
				io.KeysDown[e->key.keysym.scancode] = true;
				return true;
			}
		}
		break;
	case SDL_FINGERDOWN:
		touchData.buttonDown[0] = true;
		touchData.cX = e->tfinger.x * width;
		touchData.cY = e->tfinger.y * height;
		break;
	case SDL_FINGERMOTION:
		touchData.cX = e->tfinger.x * width;
		touchData.cY = e->tfinger.y * height;
		break;
	case SDL_FINGERUP:
		touchData.buttonDown[0] = false;
		touchData.cX = e->tfinger.x * width;
		touchData.cY = e->tfinger.y * height;
		break;
	}
	
	return false;
}