
#include "Game.h"
#include <math.h>
#include "Helper.h"

// include imgui?
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
//#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_bowie.h"

#define PROJECTION_PERSPECTIVE	0
#define PROJECTION_ORTHOGONAL	1

#define	TIME_UNIFORM_LOC	0
#define	MAT_MODELVIEW_UNIFORM_LOC	(TIME_UNIFORM_LOC+1)
#define	MAT_PROJECTION_UNIFORM_LOC	(MAT_MODELVIEW_UNIFORM_LOC+1)
#define MAT_MVP_UNIFORM_LOC	(MAT_PROJECTION_UNIFORM_LOC+1)
#define	TEXTURE0_UNIFORM_LOC	(MAT_MVP_UNIFORM_LOC+1)
#define	SCALE_UNIFORM_LOC	(TEXTURE0_UNIFORM_LOC+1)
#define MAT_NORMAL_UNIFORM_LOC	(SCALE_UNIFORM_LOC+1)

#include <vector>
#include "Light.h"
#include "DebugLightShader.h"
#include "BoxShader.h"

using std::vector;

static vector<Light> lights;
static GShader::DebugLightShader* lightShader;
static GShader::BoxShader* boxShader;

Game::Game():
App(40, "Game Test"),
speed(0.f), 
angle(0.0f),
animate(true),
perspectiveFOV(55.0f),
orthoRange(10.0f),
projectionType(PROJECTION_PERSPECTIVE)
{
    cube = nullptr;
	simple = nullptr;
	
	bgColor[0] = .2f;
	bgColor[1] = .5f;
	bgColor[2] = .72f;
	bgColor[3] = 1.f;
}

Game::~Game() {
}

void Game::onInit() {
    SDL_Log("Renderer : %s", glGetString(GL_RENDERER));
    SDL_Log("Vendor : %s", glGetString(GL_VENDOR));
    SDL_Log("Version : %s", glGetString(GL_VERSION));
    SDL_Log("GLSL Ver : %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

	view = glm::lookAt(glm::vec3(2, 5, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

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
	//cube = Mesh::createTexturedQuad(2.5f);
	cube = Mesh::createUnitBox();
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

		simple->pushUniformLocation("time", TIME_UNIFORM_LOC);
		simple->pushUniformLocation("matModelview", MAT_MODELVIEW_UNIFORM_LOC);
		simple->pushUniformLocation("matMVP", MAT_MVP_UNIFORM_LOC);
		/*simple->pushUniformLocation("matView", 2);
		simple->pushUniformLocation("matModel", 3);*/
		simple->pushUniformLocation("tex0", TEXTURE0_UNIFORM_LOC);

		simple->pushUniformLocation("scale", SCALE_UNIFORM_LOC);
		simple->pushUniformLocation("matNormal", MAT_NORMAL_UNIFORM_LOC);

		SDL_Log("Light uniform loc: %d %d %d", 
			simple->getUniformLocation("point.pos"),
			simple->getUniformLocation("point.color"),
			simple->getUniformLocation("point.radAttenInfluence")
			);

		// position it somewhere above?
		simple->use();
		glUniform3f(simple->getUniformLocation("point.pos"), -10.f, 5.f, 1.5f);
		glUniform3f(simple->getUniformLocation("point.color"), 1.f, .85f, .92f);
		glUniform3f(simple->getUniformLocation("point.radAttenInfluence"), 55.f, .1f, .2f);
	}

	// load debug light shader
	lightShader = new GShader::DebugLightShader;
	lightShader->loadFromFile("shaders/lightdebug.vert", "shaders/lightdebug.frag");
	lightShader->setUniformLocs();

	// load box shader
	boxShader = new GShader::BoxShader;
	boxShader->loadFromFile("shaders/box.vert", "shaders/box.frag");
	boxShader->setUniformLocs();

	// set ambient color
	boxShader->ambientColor = glm::vec3(0.12f, 0.1f, 0.25f);

	// add one light?
	Light l;
	l.pos = glm::vec3(0.5f, 2.5f, 0.1f);
	l.color = glm::vec3(1.f, .82f, .293f);
	l.attenuation = glm::vec3(0.52f, 0.0015f, 0.125f);
	lights.push_back(l);

	l.pos = glm::vec3(2.0f, 1.0f, -1.2f);
	l.color = glm::vec3(.8f, .1f, .97f);
	l.attenuation = glm::vec3(0.84f, .001f, .125f);
	lights.push_back(l);

	tex = Texture2D::loadFromFile("textures/crate.jpg");

	if (!tex) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error Loading texture!");
	}

	SDL_assert(tex != NULL);
	SDL_Log("Texture loaded.");


	// set opengl state
	glEnable(GL_DEPTH_TEST);
	glClearColor(.2f, .5f, .3f, .1f);

	// clear depth based on available function
	if (glClearDepth) {
		glClearDepth(1.0);
	}
	else if (glClearDepthf) {
		glClearDepthf(1.0f);
	}

	glEnableVertexAttribArray(ATTRIB_POS_LOC);
	glEnableVertexAttribArray(ATTRIB_NORMAL_LOC);
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
	if (lightShader) delete lightShader;
	if (boxShader) delete boxShader;

	// cleanup imgui
	destroyImGui();
}

static glm::vec3 findDominantAxis(const glm::vec3 axis) {
	float ax = fabs(axis.x);
	float ay = fabs(axis.y);
	float az = fabs(axis.z);

	if (ax < ay && ax < az) {
		return glm::vec3(1, 0, 0);
	}
	else if (ay < ax && ay < az) {
		return glm::vec3(0, 1, 0);
	}
	else {
		return glm::vec3(0, 0, 1);
	}
}

void Game::onUpdate(float dt) {
	if (animate) {
		// update angle
		angle += speed * dt;

		// rotate light along y axis
		float rotation = speed * dt;

		for (int i = 0; i < lights.size(); i++) {
			Light& l = lights[i];

			glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), rotation, findDominantAxis(l.pos));

			glm::vec4 newPos = glm::vec4(l.pos.x, l.pos.y, l.pos.z, 1.0f);
			newPos = rotate * newPos;

			l.pos = glm::vec3(newPos.x, newPos.y, newPos.z);
		}
	}
}

/* Render function */
void Game::onRender(float dt) {
    float newAngle = angle + speed * dt;
	if (!animate)
		newAngle = angle;

	computeProjection();
    
    float sA = sin(newAngle);
    float cA = cos(newAngle);
    float aA = ((sA + cA) * .5f);

	glm::vec3 axis = glm::vec3(sA, cA, aA);
	
	model = glm::rotate(glm::mat4(1.0f), newAngle, axis);
    // do render here
    glViewport(0, 0, iWidth, iHeight);
    //glClearColor(sA, cA, aA, 1.0f);
	glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// bind shader and set uniforms?
	/*simple->use();
	
	glUniform1f(simple->getUniformLocation(0), newAngle);*/

	glm::mat4 modelView = view * model;
	glm::mat4 mvp = proj * modelView;


	/*glUniformMatrix4fv(simple->getUniformLocation(MAT_MODELVIEW_UNIFORM_LOC), 1, GL_FALSE, glm::value_ptr(modelView));
	glUniformMatrix4fv(simple->getUniformLocation(MAT_MVP_UNIFORM_LOC), 1, GL_FALSE, glm::value_ptr(mvp));*/
	/*glUniformMatrix4fv(simple->getUniformLocation(2), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(simple->getUniformLocation(3), 1, GL_FALSE, glm::value_ptr(model));*/
	//glUniformMatrix3fv(simple->getUniformLocation(MAT_NORMAL_UNIFORM_LOC), 1, GL_FALSE, glm::value_ptr(normalMatrix));

	glm::vec3 scale = glm::vec3(2.5f, .25f, 3.2f);
	//glUniform3fv(simple->getUniformLocation(SCALE_UNIFORM_LOC), 1, glm::value_ptr(scale));

	// use texture
	/*glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(simple->getUniformLocation(TEXTURE0_UNIFORM_LOC), 0);

	tex->use();*/
	//glDisable(GL_TEXTURE_2D);

	// setup box shader
	boxShader->prepareState();
	boxShader->use();
	boxShader->setTransformData(mvp, modelView, scale);
	boxShader->setAmbientColor();
	boxShader->setTextureData(tex);
	
	// loop over active lights
	for (int i = 0; i < lights.size(); i++) {
		boxShader->setLightingData(i, lights[i], view);
	}

	// use buffer
	cube->use();

	glVertexAttribPointer(ATTRIB_POS_LOC, 3, GL_FLOAT, false, cube->strideLength, (void*)0);
	glVertexAttribPointer(ATTRIB_NORMAL_LOC, 3, GL_FLOAT, false, cube->strideLength, (void*)12);
	glVertexAttribPointer(ATTRIB_UV_LOC, 2, GL_FLOAT, false, cube->strideLength, (void*)24);


	for (int i=0; i<cube->subMeshes.size(); i++) {
		Mesh::SubMesh &s = cube->subMeshes[i];

		glDrawElements(GL_TRIANGLES, s.elemCount, GL_UNSIGNED_SHORT, (void*)s.idxBegin);
	}

	// draw lights?
	// setup shader
	lightShader->prepareState();
	lightShader->use();

	// setup buffer

	for (int i = 0; i < lights.size(); i++) {
		const Light& l = lights[i];
		lightShader->setDrawData(l, view, proj);

		cube->use();
		glVertexAttribPointer(ATTRIB_POS_LOC, 3, GL_FLOAT, false, cube->strideLength, (void*)0);

		for (int i = 0; i < cube->subMeshes.size(); i++) {
			Mesh::SubMesh& s = cube->subMeshes[i];

			glDrawElements(GL_TRIANGLES, s.elemCount, GL_UNSIGNED_SHORT, (void*)s.idxBegin);
		}
	}


	// disable depth test?
	glDisable(GL_DEPTH_TEST);

	static bool showColorPicker = false;
	static const int commentLength = 256;
	static char comment[commentLength] = { 0 };

	// render imgui stuffs
	beginRenderImGui();

	// draw text on corner?
	ImGui::Begin("Screen", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs 
		| ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground
		| ImGuiWindowFlags_NoScrollbar
	);
	ImGui::SetWindowPos(ImVec2(0, 0));
	ImGui::SetWindowSize(ImVec2(iWidth, iHeight));
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "FPS: %03d", fps);
	// check if imgui want text
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantTextInput) {
		ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.2f, 1.0f), "Text Input NEEDED!");
	}
	else {
		ImGui::TextColored(ImVec4(.2f, .5f, 1.0f, 1.0f), "Text Input NOT NEEDED!");
	}
	// check if keyboard is shown
	if (SDL_HasScreenKeyboardSupport) {
		ImGui::Text("Device has screen keyboard!");
	}
	else {
		ImGui::Text("No screen keyboard detected!");
	}
	
	if (SDL_IsScreenKeyboardShown(wndApp)) {
		ImGui::Text("Screen Keyboard shown");
	}
	else {
		ImGui::Text("Screen Keyboard hidden");
	}
	ImGui::End();

	ImGui::Begin("App Config", NULL, ImGuiWindowFlags_AlwaysAutoResize);
	//ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Render speed: %d FPS", fps);
	ImGui::SliderFloat("Cube Speed", &speed, 0.0f, 10.0f, "%.2f");
	ImGui::Checkbox("Animate?", &animate);
	ImGui::Checkbox("Change Bg Col", &showColorPicker);

	// test multiline
	/*ImGui::Separator();
	ImGui::InputTextMultiline("Comment:", comment, commentLength);*/

	// use perspective or orthogonal?
	ImGui::Separator();
	ImGui::Text("Projection Type:");
	if (ImGui::RadioButton("Perspective", &projectionType, PROJECTION_PERSPECTIVE)) {
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Orthogonal", &projectionType, PROJECTION_ORTHOGONAL)) {
	}

	if (projectionType == PROJECTION_PERSPECTIVE) {
		// show slider fov
		ImGui::SliderFloat("FOV", &perspectiveFOV, 45.0f, 150.0f, "%.2f Deg");
	}
	else if (projectionType == PROJECTION_ORTHOGONAL) {
		// show slider range
		ImGui::SliderFloat("Range", &orthoRange, 1.0f, 20.0f, "%.2f");
	}	
	// print debug info?
	ImGui::TextColored(ImVec4(.82f, 1.f, 0, 1), "glClearDepth: 0x%x", glClearDepth);
	ImGui::TextColored(ImVec4(0, 1.f, .82f, 1), "glClearDepthf: 0x%x", glClearDepthf);
	ImGui::End();

	if (showColorPicker) {
		ImGui::Begin("Change BG Color", &showColorPicker, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::ColorPicker4("Bg Color", &bgColor[0]);
		ImGui::End();
	}

	// spawn render data
	endRenderImGui();

	// blit to screen
	//ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	ImDrawData* data = ImGui::GetDrawData();
	ImGui_ImplBowie_RenderDrawData(data);
}

/* handle event */
void Game::onEvent(SDL_Event *e) {
	SDL_SetHint(SDL_HINT_IME_INTERNAL_EDITING, "0");
	// override imgui sdl event processing
	if (!ImGui_ImplBowie_ProcessEvent(e))
		ImGui_ImplSDL2_ProcessEvent(e);

	ImGuiIO& io = ImGui::GetIO();
	
    if (e->type == SDL_QUIT) {
        this->setRunFlag(false);
		return;
    } else if (e->type == SDL_KEYDOWN || e->type == SDL_KEYUP) {
        switch (e->key.keysym.sym) {
            case SDLK_ESCAPE:
            case SDLK_AC_BACK:
                this->setRunFlag(false);
            return;
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
	if (projectionType == PROJECTION_PERSPECTIVE)
		proj = glm::perspectiveFov(glm::radians(perspectiveFOV), (float)iWidth, (float)iHeight, .1f, 100.0f);
	else if (projectionType == PROJECTION_ORTHOGONAL) {
		float aspect = (float)iWidth / (float)iHeight;
		float hor = orthoRange * 0.5f * aspect;
		float vert = orthoRange * 0.5f;
		proj = glm::ortho(-hor, hor, -vert, vert, .1f, 100.0f);
	}
}

void Game::initImGui() {
	// set proper scale
	float ddpi, hdpi, vdpi, scale = 1.0f;
	if (SDL_GetDisplayDPI(0, &ddpi, &hdpi, &vdpi) == -1) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "No DPI Info available!");
	}
	else {
		SDL_Log("DPI Info (d, h, v): %.2f, %.2f, %.2f", ddpi, hdpi, vdpi);
		scale = (int) (vdpi / 96.0f);
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImFontConfig cfg;

	cfg.SizePixels = 13.0f * scale;
	io.Fonts->AddFontDefault(&cfg);

	ImGui::StyleColorsDark();
	ImGui::GetStyle().ScaleAllSizes(scale);

	//const char* glsl_version = "#version 100"; // (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	//SDL_Log("Initializing ImGui with GLSL Version: %s", glsl_version);

	ImGui_ImplSDL2_InitForOpenGL(wndApp, glCtx);
	ImGui_ImplBowie_Init(wndApp);
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