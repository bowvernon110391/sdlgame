
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

Game::Game() {
	cam_horzRot = 30;
	cam_vertRot = -45.0f;
	cam_dist = 10.0f;
}

Game::~Game() {

}
static LargeMeshObject* lmo;
void Game::onInit() {
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
		->useSceneData((new SceneData())
			->setSunDiffuseColor(glm::vec4(1.0f))
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

	// add a cube manually
	meshMgr->put("cube", Mesh::createUnitBox()->createBufferObjects());
	meshMgr->load("weirdcube.bcf");
	meshMgr->load("weirdsphere.bcf");

	// load a shader
	shaderMgr->load("box");
	shaderMgr->load("plain");
	shaderMgr->load("spheremap");

	// load textures
	textureMgr->load("env.jpg");
	textureMgr->load("env2.jpg");
	textureMgr->load("env3.jpg");
	textureMgr->load("road_on_grass.png")->withWrap(GL_REPEAT, GL_REPEAT);
	textureMgr->load("trimsheet_01.png");

	// add shader data
	shaderDataMgr->load("reflect_env")
		->fillTextureSlot(0, textureMgr->get("env.jpg"));
	shaderDataMgr->load("reflect_env2")
		->fillTextureSlot(0, textureMgr->get("env2.jpg"));
	shaderDataMgr->load("reflect_env3")
		->fillTextureSlot(0, textureMgr->get("env3.jpg"));
	shaderDataMgr->load("rally_track_01")
		->fillTextureSlot(0, textureMgr->get("road_on_grass.png"))
		->setShininess(50.1f)
		->setSpecular(glm::vec4(0.1f));
	shaderDataMgr->load("trimsheet_01")
		->fillTextureSlot(0, textureMgr->get("trimsheet_01.png"))
		->setShininess(1.0f);
	shaderDataMgr->load("debug")
		->setDiffuse(glm::vec4(1.f, 0.f, 0.f, 1.f));


	// make a material
	materialMgr->load("reflect_env")
		->withShader(shaderMgr->get("spheremap"))
		->withData(shaderDataMgr->get("reflect_env"));
	materialMgr->load("reflect_env2")
		->withShader(shaderMgr->get("spheremap"))
		->withData(shaderDataMgr->get("reflect_env2"));
	materialMgr->load("reflect_env3")
		->withShader(shaderMgr->get("spheremap"))
		->withData(shaderDataMgr->get("reflect_env3"));
	materialMgr->load("rally_track_01")
		->withShader(shaderMgr->get("box"))
		->withData(shaderDataMgr->get("rally_track_01"));
	materialMgr->load("trimsheet_01")
		->withShader(shaderMgr->get("plain"))
		->withData(shaderDataMgr->get("trimsheet_01"));
	materialMgr->load("debug")
		->withShader(m_renderer->getDebugShader())
		->withData(shaderDataMgr->get("debug"));

	// now add material set (a combination of material basically)
	matsetMgr->load("reflect_env")
		->addMaterial(materialMgr->get("reflect_env"));
	matsetMgr->load("reflect_env2")
		->addMaterial(materialMgr->get("reflect_env2"));
	matsetMgr->load("reflect_env3")
		->addMaterial(materialMgr->get("reflect_env3"));
	matsetMgr->load("rally_track_01")
		->addMaterial(materialMgr->get("rally_track_01"))
		->addMaterial(materialMgr->get("trimsheet_01"));
	matsetMgr->load("debug")
		->addMaterial(materialMgr->get("debug"));
	// test to create object
	/*for (int i = 0; i < 15; i++) {
		MeshObject* mo = new MeshObject(
			meshMgr->getRandom(), 
			matsetMgr->getRandom()
		);

		mo->setPosition(glm::sphericalRand(10.0f) * glm::vec3(1, 0.2, 1) + glm::vec3(0, 4, 0))
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
	}*/

	glEnable(GL_MULTISAMPLE);

	// test
	LargeMesh* lm = largeMeshMgr->load("rally_track_01.lmf");

	lmo = new LargeMeshObject(lm, matsetMgr->get("rally_track_01"));
	lmo->debug_draw = true;
	renderObjs.push_back(lmo);
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

		ImGui::Begin("App Config", NULL, ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "FPS: %d", fps);
			ImGui::SameLine();
			ImGui::Text("%s", title.c_str());
			ImGui::SameLine();
			if (ImGui::Button("QUIT")) {
			    this->setRunFlag(false);
			}

			// debug draw
			ImGui::Checkbox("Draw Active Mesh Only", &lmo->debug_draw);
			// active mesh?
			ImGui::SliderInt("active_mesh", &lmo->active_mesh, 0, lmo->lm->mesh_count - 1);

			// debug text
			if (ImGui::CollapsingHeader("Debugging")) {
				ImGui::Checkbox("Draw Debug?", &m_renderer->drawDebug);
				ImGui::ColorEdit4("Box Color", glm::value_ptr(m_renderer->debugColor));
			}

			if (ImGui::CollapsingHeader("Color+Depth Pass")) {
				RenderPass* rp = m_renderer->getPass("color_depth");
				if (rp) {
					if (ImGui::Button("See Cmd Buffer")) {
						rp->generateDebugString();
					}

					ImGui::SameLine();
					// set random matset
					if (ImGui::Button("Randomize Matset")) {
						// reset all object's matset, except the last one
						for (int i = 0; i < renderObjs.size() - 1; i++) {
							((MeshObject*)renderObjs[i])->ms = matsetMgr->getRandom();
						}
						//rp->generateDebugString();
					}

					// spawn multiple text colored, alternated color
					ImVec4 colors[2] = {
						ImVec4(1,1,0,1), ImVec4(0,1,0,1)
					};

					std::string txt = rp->debugString;	// copy string
					std::string delim = "\n";
					std::string line;
					size_t pos;
					int cnt = 0;
					while ((pos = txt.find(delim)) != std::string::npos) {
						line = txt.substr(0, pos);
						txt.erase(0, pos + delim.length());

						ImGui::TextColored(colors[cnt++ % 2], line.c_str(), "");
					}
				}
			}
			
			if (ImGui::CollapsingHeader("Background Settings")) {
				ImGui::ColorPicker4("Color", glm::value_ptr(color));
			}

			if (ImGui::CollapsingHeader("Sun Settings")) {
				// put sun settings here?
				ImGui::ColorEdit4("Sun Diffuse", glm::value_ptr(m_renderer->getSceneData()->sunDiffuseColor));
				ImGui::ColorEdit4("Sun Specular", glm::value_ptr(m_renderer->getSceneData()->sunSpecularColor));
				//ImGui::ColorPicker4("Sun Intensity", glm::value_ptr(m_renderer->getSceneData()->sunIntensity));
				ImGui::ColorEdit4("Sun Intensity", glm::value_ptr(m_renderer->getSceneData()->sunIntensity));
				// renormalize at the end?
				ImGui::DragFloat3("Sun Direction", glm::value_ptr(m_renderer->getSceneData()->sunDirection));
				
				// only renormalize after we dont receive input
				if (!io.WantCaptureMouse) {
					m_renderer->getSceneData()->setSunDirection( glm::normalize(m_renderer->getSceneData()->sunDirection));
				}
			}

			if (ImGui::CollapsingHeader("Scene Settings")) {
				ImGui::ColorEdit4("Scene Ambient", glm::value_ptr(m_renderer->getSceneData()->ambientColor));
			}

		ImGui::End();
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