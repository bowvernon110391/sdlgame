#include "imgui_impl_bowie.h"
#include "../Texture2d.h"
#include "../Shader.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define FONT_TEXTURE_UNIFORM_LOC	0
#define PROJ_MAT_UNIFORM_LOC		1

static Texture2D* fontTexture = 0;
static Shader* fontShader = 0;
static glm::mat4 projMat;
static GLuint vboHandle;
static GLuint iboHandle;

Texture2D* getFontTexture() {
	return fontTexture;
}

bool ImGui_ImplBowie_Init() {
	SDL_Log("Initializing bowie's IMGUI Implementation...");

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

void ImGui_ImplBowie_NewFrame() {
	// for now, do nothing
	//SDL_Log("Imgui_Bowie_NewFrame()");
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
			return true;
		}
	}
	return true;
}

// create font shader for our implementation
static void ImGui_ImplBowie_CreateFontShader() {
	const char* vertShader = " \
	uniform mat4 matProj;\n \
	\
	attribute vec2 position;\n \
	attribute vec2 uv;\n \
	attribute vec4 color;\n \
	\
	varying vec4 vColor;\n \
	varying vec2 vTexcoord;\n \
	\
	void main() {\n \
		vColor = color;\n \
		vTexcoord = uv;\n \
		gl_Position = matProj * vec4(position.xy, 0, 1);\n \
	}\n \
	";

	const char* fragShader = "\
	#ifdef GL_ES\n \
	precision mediump float;\n \
	#endif\n \
	\
	uniform sampler2D tex0;\n \
	\
	varying vec4 vColor;\n \
	varying vec2 vTexcoord;\n \
	\
	void main() {\n \
		gl_FragColor = vColor * texture2D(tex0, vTexcoord);\n \
	}\n \
	";

	if (!fontShader) {
		fontShader = Shader::loadShaderFromSources(vertShader, strlen(vertShader), fragShader, strlen(fragShader));

		// setup some uniform
		if (fontShader) {
			fontShader->pushUniformLocation("tex0", FONT_TEXTURE_UNIFORM_LOC);
			fontShader->pushUniformLocation("matProj", PROJ_MAT_UNIFORM_LOC);
			SDL_Log("Font shader loaded");
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

	glViewport(0, 0, fb_width, fb_height);

	// setup shader
	fontShader->use();
	// enable texturing
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	// projection matrix
	glUniformMatrix4fv(fontShader->getUniformLocation(PROJ_MAT_UNIFORM_LOC), 1, false, glm::value_ptr(projMat));
	// texture unit
	glUniform1i(fontShader->getUniformLocation(FONT_TEXTURE_UNIFORM_LOC), 0);

	// setup buffers
	glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboHandle);

	glEnableVertexAttribArray(ATTRIB_POS_LOC);
	glEnableVertexAttribArray(ATTRIB_COL_LOC);
	glEnableVertexAttribArray(ATTRIB_UV_LOC);

	glVertexAttribPointer(ATTRIB_POS_LOC, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
	glVertexAttribPointer(ATTRIB_UV_LOC, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
	glVertexAttribPointer(ATTRIB_COL_LOC, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, col));

	
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
