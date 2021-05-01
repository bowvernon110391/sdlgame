
#include "Game.h"
#include <math.h>
#include "Helper.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/rotate_vector.hpp>

// include imgui?
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_bowie.h"

#include "Renderer.h"
#include "Camera.h"
#include "SceneData.h"
#include "Shader.h"
#include "Mesh.h"
#include "LargeMesh.h"
#include "Material.h"
#include "ResourceManager.h"
#include "MeshObject.h"
#include "LargeMeshObject.h"
#include "AABBTree.h"
#include "ShaderSource.h"
#include "ShaderInstanceManager.h"

Game::Game() {
	cam_horzRot = 0;
	cam_vertRot = -45.0f;
	cam_dist = 5.0f;
}

Game::~Game() {

}
static LargeMeshObject* lmo;
void Game::onInit() {
	// debug print
	glEnable(GL_MULTISAMPLE);
	srand(SDL_GetTicks());
	initImGui();

	m_renderer = (new Renderer)
		->useCamera((new Camera())
			->setPosition(glm::vec3(0, 7, 5))
			->setTarget(glm::vec3(0))
			->setFov(80.0f)
			->usePerspective(true)
			->setClipDistance(0.2f, 500.0f)
		)
		->useSceneData(
			(new SceneData())
		)
		->setViewport(0, 0, iWidth, iHeight);

	// initalize managers
	shaderMgr = new ResourceManager<Shader>(loadShader, "shaders");
	shaderDataMgr = new ResourceManager<ShaderData>(loadBasicShaderData);
	materialMgr = new ResourceManager<Material>(loadBasicMaterial);
	matsetMgr = new ResourceManager<MaterialSet>(loadMaterialSet);
	meshMgr = new ResourceManager<Mesh>(loadMesh, "meshes");
	largeMeshMgr = new ResourceManager<LargeMesh>(loadLargeMesh, "meshes");
	textureMgr = new ResourceManager<Texture2D>(loadTexture, "textures");
	sourceMgr = new ResourceManager<ShaderSource>(loadShaderSource, "shaders", new ShaderSource(""));
	shdInstanceMgr = new ShaderInstanceManager();

	// load a source
	sourceMgr->load("plain.glsl");

	ShaderSource* src = sourceMgr->get("plain.glsl");
	//src->debugPrint();

	ShaderKey keys[] = {
		ShaderKey(src, LightType::UNLIT, OpacityType::OPAQUE, GeomType::STATIC),
		ShaderKey(src, LightType::UNLIT, OpacityType::ALPHA_CLIP, GeomType::STATIC),
		ShaderKey(src, LightType::AMBIENT, OpacityType::OPAQUE, GeomType::STATIC),
		ShaderKey(src, LightType::AMBIENT, OpacityType::ALPHA_CLIP, GeomType::STATIC)
	};

	for (int i = 0; i < 4; i++) {
		const Shader* shd = shdInstanceMgr->getShader(keys[i]);
	}

	// add a cube manually
	//meshMgr->put("cube", Mesh::createUnitBox()->createBufferObjects());
	//meshMgr->load("weirdcube.bcf");
	//meshMgr->load("weirdsphere.bcf");
	//meshMgr->load("sphere.bcf");

	//// load a shader
	//shaderMgr->load("box");
	//shaderMgr->load("plain");
	//shaderMgr->load("spheremap");
	//shaderMgr->load("phong");

	//// load textures
	//textureMgr->load("env.jpg");
	//textureMgr->load("env2.jpg");
	//textureMgr->load("env3.jpg");
	//textureMgr->load("road_on_grass.png")->withWrap(GL_REPEAT, GL_REPEAT);
	//textureMgr->load("trimsheet_01.png");
	//textureMgr->load("ibl_spec.jpg");
	//textureMgr->load("ibl_diff.jpg");
}

void Game::spawnRandomObject() {
	MeshObject* mo = new MeshObject(
		meshMgr->getRandom(),
		matsetMgr->getRandom()
	);

	mo->setPosition(glm::sphericalRand(5.f) * glm::vec3(1, 1, 1) + glm::vec3(0, 0, 0))
		->setRotation(
			glm::angleAxis(
				glm::radians(
					glm::gaussRand(0.0f, 45.0f)
				), glm::normalize(glm::vec3(
					glm::gaussRand(0.0f, 1.0f),
					glm::gaussRand(0.0f, 1.0f),
					glm::gaussRand(0.0f, 1.0f)
				)))
		);
	renderObjs.push_back(mo);

	// add to tree?
	tree->insert(new AABBNode(mo));
}

void Game::clearTrees()
{
	// delete all in objects?
	for (int i = 1; i < renderObjs.size(); i++) {
		delete renderObjs.at(i);
	}
	renderObjs.resize(1);

	delete tree;
	tree = new AABBTree();
}

void Game::debugPrint(AABBNode* n)
{
	if (!n) 
		return;

	char tmp[256];
	sprintf(tmp, "%X: %s%s", (size_t)n, n->isRoot() ? "[ROOT]" : "", n->isLeaf() ? "[LEAF]" : "");

	if (ImGui::TreeNode(n, "%s", tmp)) {
		// print aabb
		const AABB& b = n->bbox;
		ImGui::Text("aabb(%.2f %.2f %.2f | %.2f %.2f %.2f)", b.min.x, b.min.y, b.min.z, b.max.x, b.max.y, b.max.z);

		// button to select?
		if (ImGui::Button("SELECT")) {
			tree->selected = n;
		}
		
		if (n->canRotate()) {
			ImGui::SameLine();
			if (ImGui::Button("CHK")) {
				// do something
				SDL_Log("SHOULD ROTATE HERE!");
				tree->setDebugBox(n->potentialRotatedBox());
			}

			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.5f, .0f, .0f, 1.f));
			if (ImGui::Button("ROT")) {
				// do something here
				n->rotate();
			}
			ImGui::PopStyleColor();
		}

		// child render too
		if (!n->isLeaf()) {
			debugPrint(n->left);
			debugPrint(n->right);
		}

		ImGui::TreePop();
	}
}

void Game::onDestroy() {
	// debug first
	SDL_Log("++SHADERS++\n");
	shaderMgr->printDebug();
	SDL_Log("++SHADERS_DATA++\n");
	shaderDataMgr->printDebug();
	SDL_Log("++MATERIALS++\n");
	materialMgr->printDebug();
	SDL_Log("++MATERIAL_SETS++\n");
	matsetMgr->printDebug();
	SDL_Log("++MESHES++\n");
	meshMgr->printDebug();
	SDL_Log("++LARGE_MESHES++\n");
	largeMeshMgr->printDebug();
	SDL_Log("++TEXTURES++\n");
	textureMgr->printDebug();
	SDL_Log("++SHADER_SOURCES++\n");
	sourceMgr->printDebug();
	SDL_Log("__SHADER_INSTANCES__\n");
	shdInstanceMgr->printDebug();

	destroyImGui();
	// delete renderer
	delete m_renderer;
	// delete resource managers
	delete shaderMgr;
	delete shaderDataMgr;
	delete meshMgr;
	delete largeMeshMgr;
	delete materialMgr;
	delete matsetMgr;
	delete textureMgr;
	delete sourceMgr;
	delete shdInstanceMgr;
	// delete tree
	delete tree;

	// delete render objects (unmanaged)
	for (auto it = renderObjs.begin(); it != renderObjs.end(); ++it) { 
		// print bbox first?
		delete* it;
	};
}

void Game::onUpdate(float dt) {
}

void Game::onRender(float dt) {
	// compute camera
	cam_vertRot = glm::clamp(cam_vertRot, -89.0f, 89.0f);
	glm::vec3 camPos = glm::vec3(0.0f, 0.0f, cam_dist);

	camPos = glm::rotate(camPos, glm::radians(cam_vertRot), glm::vec3(1, 0, 0));
	camPos = glm::rotate(camPos, glm::radians(cam_horzRot), glm::vec3(0, 1, 0));

	//SDL_Log("Camera pos: %.4f, %.4f, %.4f\n", camPos.x, camPos.y, camPos.z);
	m_renderer->getCamera()->setPosition(camPos);
	
	// clear depth and color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 3d renderer
	m_renderer->draw(this->renderObjs, dt);

	// 2d rendering
	glDisable(GL_DEPTH_TEST);
	beginRenderImGui();

	{
		static int value = 20;
		static glm::vec4 color(0.5f, 0.2f, 1.0f, 1.0f);
		glClearColor(color.r, color.g, color.b, color.a);

		auto& io = ImGui::GetIO();
		std::string title;
		if (io.WantCaptureMouse) {
			title += "MOUSE_NEEDED";
		} else {
			title += "MOUSE_CLEAR";
		}

		// window app config
		ImGui::Begin("App Config", NULL, ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "FPS: %d", fps);
			ImGui::SameLine();
			ImGui::Text("%s (%.2f, %.2f)", title.c_str(), io.MousePos.x, io.MousePos.y);
			ImGui::SameLine();
			if (ImGui::Button("QUIT")) {
			    this->setRunFlag(false);
			}

		ImGui::End();

		//ImGui::ShowMetricsWindow();
	}

	endRenderImGui();
	ImDrawData* data = ImGui::GetDrawData();
	ImGui_ImplBowie_RenderDrawData(data);
}

void Game::onEvent(SDL_Event* e) {
	static int last_x=0, last_y=0;
	static bool dragging = false;
	static bool is_multigesturing = false;
	static int fingercount = 0;
	
	int cur_x, cur_y;

    auto& io = ImGui::GetIO();
    int* viewport = m_renderer->getViewport();

	bool canHandleInput = true;
	if (!ImGui_ImplBowie_ProcessEvent(e)) {
		canHandleInput = !ImGui_ImplSDL2_ProcessEvent(e);
	}

	// let's print out event
	bool interesting = false;
	std::string eventName;
	switch (e->type) {
	    case SDL_FINGERDOWN:
	        interesting = true;
	        eventName = "FINGER_DOWN";
	        break;
	    case SDL_FINGERMOTION:
	        interesting = true;
	        eventName = "FINGER_MOTION";
	        break;
	    case SDL_FINGERUP:
	        interesting = true;
	        eventName = "FINGER_UP";
	        break;
	    case SDL_MULTIGESTURE:
	        interesting = true;
	        eventName = "MULTI_GESTURE";
	        break;
	    case SDL_MOUSEBUTTONDOWN:
	        interesting = true;
	        eventName = "MOUSE_DOWN";
	        break;
	    case SDL_MOUSEMOTION:
	        interesting = true;
	        eventName = "MOUSE_MOVE";
	        break;
	    case SDL_MOUSEBUTTONUP:
	        interesting = true;
	        eventName = "MOUSE_UP";
	        break;
	}
	/*if (interesting) {
	    SDL_Log("INTERESTING_EVENT: %s, is_multigesturing: %s, ImGuiWantMouse: %s\n",
	    		eventName.c_str(),
	    		is_multigesturing ? "true" : "false",
	    		io.WantCaptureMouse ? "true" : "false");
	}*/

	switch (e->type) {
    case SDL_MULTIGESTURE:
        if (io.WantCaptureMouse) break;
        is_multigesturing = true;
        fingercount = 2;
        dragging = false;
        // now just update scale?
        cam_dist -= e->mgesture.dDist * 10.0f;
        break;
	case SDL_QUIT:
		this->setRunFlag(false);
		return;
	case SDL_WINDOWEVENT:
		if (e->window.event == SDL_WINDOWEVENT_RESIZED) {
			SDL_Log("Window resize: %d x %d\n", e->window.data1, e->window.data2);
			m_renderer->setViewport(0, 0, e->window.data1, e->window.data2);
		}
		break;
	// mimic dragging for finger down
	case SDL_FINGERDOWN:
		if (io.WantCaptureMouse || is_multigesturing) break;
		++fingercount;
		if (fingercount >= 2) {
		    is_multigesturing = true;
		    break;
		}
		dragging = true;
		cur_x = last_x = e->tfinger.x * viewport[2];
		cur_y = last_y = e->tfinger.y * viewport[3];
		break;
	case SDL_MOUSEBUTTONDOWN:
		if (io.WantCaptureMouse) break;
		dragging = true;
		cur_x = last_x = e->button.x;
		cur_y = last_y = e->button.y;
		break;

	// mimic mouse motion
	case SDL_FINGERMOTION:
		if (io.WantCaptureMouse || is_multigesturing) {
			break;
		}

		cur_x = e->tfinger.x * viewport[2];
		cur_y = e->tfinger.y * viewport[3];
		if (dragging) {
			cam_horzRot -= (float)(cur_x - last_x) * 0.5f;
			cam_vertRot -= (float)(cur_y - last_y) * 0.25f;
		}
		last_x = cur_x;
		last_y = cur_y;
		break;
	case SDL_MOUSEMOTION:
		if (io.WantCaptureMouse) break;
		cur_x = e->motion.x;
		cur_y = e->motion.y;
		if (dragging) {
			cam_horzRot -= (float)(cur_x - last_x);
			cam_vertRot -= (float)(cur_y - last_y);
		}
		last_x = cur_x;
		last_y = cur_y;
		break;
	case SDL_MOUSEWHEEL:
		if (io.WantCaptureMouse) break;
		cam_dist -= e->wheel.y * 0.2f;
		break;

	// mimic button up
	case SDL_FINGERUP:
	    --fingercount;
	    if (fingercount <0 ) fingercount = 0;
	    dragging = is_multigesturing = false;
	    break;
	case SDL_MOUSEBUTTONUP:
		if (io.WantCaptureMouse) break;
		dragging = false;
		break;

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
		scale = (float)((int) (vdpi / 96.0f));
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImFontConfig cfg;

	cfg.SizePixels = 13.0f * scale;
	io.Fonts->AddFontDefault(&cfg);

	ImGui::StyleColorsDark();
	ImGui::GetStyle().ScaleAllSizes(scale);

	ImGui_ImplSDL2_InitForOpenGL(wndApp, glCtx);
	ImGui_ImplBowie_Init(wndApp);
}

void Game::destroyImGui() {
	ImGui_ImplBowie_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

void Game::beginRenderImGui() {
	ImGui_ImplBowie_NewFrame();
	ImGui_ImplSDL2_NewFrame(wndApp);
	ImGui_ImplBowie_InjectTouchHandler();
	ImGui::NewFrame();
}

void Game::endRenderImGui() {
	ImGui::Render();	// generate render data, before rendering for real
}