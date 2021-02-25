
#include "Game.h"
#include <math.h>
#include "Helper.h"

// include imgui?
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
//#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_bowie.h"

Game::Game():
App(40, "Game Test"),
speed(0.f), 
angle(0.0f),
animate(true) {
    cube = nullptr;
	simple = nullptr;
}

Game::~Game() {
}

void Game::onInit() {
    SDL_Log("Renderer : %s", glGetString(GL_RENDERER));
    SDL_Log("Vendor : %s", glGetString(GL_VENDOR));
    SDL_Log("Version : %s", glGetString(GL_VERSION));
    SDL_Log("GLSL Ver : %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

	// init matrices
	computeProjection();

	view = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	model = glm::mat4(1.0f);

	// log it?
	float *m;
	
	m = glm::value_ptr(proj);
	SDL_Log("perspective @ %.2f, %.2f:", glm::degrees(45.0f), iWidth / (float)iHeight);
	for (int i=0; i<4; i++) {
		SDL_Log("%.2f %.2f %.2f %.2f", m[0 + i], m[4 + i], m[8 + i], m[12 + i]);
	}

	m = glm::value_ptr(view);
	SDL_Log("view:");
	for (int i=0; i<4; i++) {
		SDL_Log("%.2f %.2f %.2f %.2f", m[0 + i], m[4 + i], m[8 + i], m[12 + i]);
	}

	m = glm::value_ptr(model);
	SDL_Log("model:");
	for (int i=0; i<4; i++) {
		SDL_Log("%.2f %.2f %.2f %.2f", m[0 + i], m[4 + i], m[8 + i], m[12 + i]);
	}

	glm::mat4 mvp = proj * view * model; // glm::mul( glm::mul(proj, view), model);
	m = glm::value_ptr(mvp);
	SDL_Log("MVP:");
	for (int i=0; i<4; i++) {
		SDL_Log("%.2f %.2f %.2f %.2f", m[0 + i], m[4 + i], m[8 + i], m[12 + i]);
	}

	// create cube
	cube = Mesh::createTexturedQuad(2.5f);
	if (cube) {
		if (cube->createBufferObjects()) {
			SDL_Log("Buffer ids: %u %u", cube->vbo, cube->ibo);
		}
	}

	simple = Shader::loadShaderFromFile("shaders/shader.vert", "shaders/shader.frag");

	SDL_assert(simple != NULL);

	if (simple) {
		/*simple->pushUniformLocation("matProj", 0);
		simple->pushUniformLocation("matView", 1);
		simple->pushUniformLocation("matModel", 2);

		simple->pushUniformLocation("tex0", 3);*/

		simple->pushUniformLocation("time", 0);
		simple->pushUniformLocation("matProj", 1);
		simple->pushUniformLocation("matView", 2);
		simple->pushUniformLocation("matModel", 3);
		simple->pushUniformLocation("tex0", 4);
	}

	tex = Texture2D::loadFromFile("textures/crate.jpg");

	if (!tex) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error Loading texture!");
	}

	SDL_assert(tex != NULL);
	SDL_Log("Texture loaded.");


	// set opengl state
	glEnable(GL_DEPTH_TEST);
	glClearColor(.2f, .5f, .3f, .1f);

#if defined(CLEAR_DEPTH_GL_CORE)
	glClearDepth(1.0);
#else
	glClearDepthf(1.0f);
#endif

	glEnableVertexAttribArray(ATTRIB_POS_LOC);
	glEnableVertexAttribArray(ATTRIB_COL_LOC);
	glEnableVertexAttribArray(ATTRIB_UV_LOC);

	// init imgui
	initImGui();
}

void Game::onDestroy() {
    // do clean up here rather than at destructor
    // if (simple) delete simple;
	if (cube) delete cube;
	if (simple) delete simple;
	if (tex) delete tex;

	// cleanup imgui
	destroyImGui();
}

void Game::onUpdate(float dt) {
	if (animate)
		angle += speed * dt;
}

/* Render function */
void Game::onRender(float dt) {
    float newAngle = angle + speed * dt;
	if (!animate)
		newAngle = angle;
    
    float sA = sin(newAngle);
    float cA = cos(newAngle);
    float aA = ((sA + cA) * .5f);

	glm::vec3 axis = glm::vec3(sA, cA, aA);

	/*model = glm::translate(glm::mat4(1.0f), glm::vec3(sA * 0.2f, cA * 0.2f, -2.0f + aA * 0.2f) );
	model = glm::rotate(model, newAngle, axis);*/
	model = glm::rotate(glm::mat4(1.0f), newAngle, axis);
    // do render here
    glViewport(0, 0, iWidth, iHeight);
    //glClearColor(sA, cA, aA, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// bind shader and set uniforms?
	simple->use();
	
	glUniform1f(simple->getUniformLocation(0), newAngle);

	glUniformMatrix4fv(simple->getUniformLocation(1), 1, GL_FALSE, glm::value_ptr(proj));
	glUniformMatrix4fv(simple->getUniformLocation(2), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(simple->getUniformLocation(3), 1, GL_FALSE, glm::value_ptr(model));

	// use texture
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(simple->getUniformLocation(4), 0);

	tex->use();

	// use buffer
	cube->use();

	glVertexAttribPointer(ATTRIB_POS_LOC, 3, GL_FLOAT, false, cube->strideLength, (void*)0);
	glVertexAttribPointer(ATTRIB_COL_LOC, 3, GL_FLOAT, false, cube->strideLength, (void*)12);
	glVertexAttribPointer(ATTRIB_UV_LOC, 2, GL_FLOAT, false, cube->strideLength, (void*)24);


	for (int i=0; i<cube->subMeshes.size(); i++) {
		Mesh::SubMesh &s = cube->subMeshes[i];

		glDrawElements(GL_TRIANGLES, s.elemCount, GL_UNSIGNED_SHORT, (void*)s.idxBegin);
	}

	// disable depth test?
	glDisable(GL_DEPTH_TEST);

	// render imgui stuffs
	beginRenderImGui();

	ImGui::Begin("GUI Test", NULL, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Render speed: %d FPS", fps);
	ImGui::SliderFloat("Cube Speed", &speed, 0.0f, 10.0f, "%.2f");
	ImGui::Checkbox("Animate?", &animate);
	ImGui::End();

	// spawn render data
	endRenderImGui();

	// blit to screen
	//ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	ImDrawData* data = ImGui::GetDrawData();
	ImGui_ImplBowie_RenderDrawData(data);
}

/* handle event */
void Game::onEvent(SDL_Event *e) {
	// handle imgui event first
	ImGui_ImplSDL2_ProcessEvent(e);

    if (e->type == SDL_QUIT) {
        this->setRunFlag(false);
		return;
    } else if (e->type == SDL_KEYDOWN) {
        switch (e->key.keysym.sym) {
            case SDLK_ESCAPE:
            case SDLK_AC_BACK:
                this->setRunFlag(false);
            break;
        }
		return;
	}
	else if (e->type == SDL_WINDOWEVENT) {
		if (e->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
			// store new window size and recreate projection matrix
			this->iWidth = e->window.data1;
			this->iHeight = e->window.data2;

			SDL_Log("Window resized to: (%d x %d)", this->iWidth, this->iHeight);

			computeProjection();
		}
	}
}

void Game::computeProjection() {
	//proj = glm::perspective(55.0f, iWidth / (float)iHeight, .01f, 1000.0f);
	//proj = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, -10.f, 10.f);
	proj = glm::perspectiveFov(glm::radians(55.0f), (float)iWidth, (float)iHeight, .1f, 100.0f);
}

void Game::initImGui() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	//const char* glsl_version = "#version 100"; // (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	//SDL_Log("Initializing ImGui with GLSL Version: %s", glsl_version);

	ImGui_ImplSDL2_InitForOpenGL(wndApp, glCtx);
	ImGui_ImplBowie_Init();
	//ImGui_ImplOpenGL3_Init(glsl_version);
}

void Game::destroyImGui() {
	//ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplBowie_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

void Game::beginRenderImGui() {
	//ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplBowie_NewFrame();
	ImGui_ImplSDL2_NewFrame(wndApp);
	ImGui::NewFrame();
}

void Game::endRenderImGui() {
	ImGui::Render();	// generate render data, before rendering for real
}