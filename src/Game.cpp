
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
#include "Material.h"
#include "ResourceManager.h"
#include "MeshObject.h"

Game::Game() {
	cam_horzRot = 30;
	cam_vertRot = -45.0f;
	cam_dist = 10.0f;
}

Game::~Game() {

}

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
		)
		->setViewport(0, 0, iWidth, iHeight);

	// initalize managers
	shaderMgr = new ResourceManager<Shader>(loadShader, "shaders");
	shaderDataMgr = new ResourceManager<ShaderData>(loadBasicShaderData);
	materialMgr = new ResourceManager<Material>(loadBasicMaterial);
	matsetMgr = new ResourceManager<MaterialSet>(loadMaterialSet);
	meshMgr = new ResourceManager<Mesh>(loadMesh, "meshes");
	textureMgr = new ResourceManager<Texture2D>(loadTexture, "textures");

	// add a cube manually
	meshMgr->put("cube", Mesh::createUnitBox()->createBufferObjects());
	meshMgr->load("weirdcube.bcf");

	// load a shader
	shaderMgr->load("box");
	shaderMgr->load("plain");

	// load textures
	textureMgr->load("cliff.jpg");
	textureMgr->load("grass.jpg");
	textureMgr->load("gravel.jpg");
	textureMgr->load("crate.jpg");
	textureMgr->load("road.png");

	// add shader data
	shaderDataMgr->load("cliff")
		->fillTextureSlot(0, textureMgr->get("cliff.jpg"))
		->setShininess(10.0f)
		->setSpecular(glm::vec4(glm::vec3(1.0f) * glm::gaussRand(0.5f, 0.5f), 0.0f));
	shaderDataMgr->load("grass")
		->fillTextureSlot(0, textureMgr->get("grass.jpg"))
		->setShininess(5.f)
		->setSpecular(glm::vec4(glm::vec3(1.0f) * glm::gaussRand(0.5f, 0.5f), 0.0f));
	shaderDataMgr->load("gravel")
		->fillTextureSlot(0, textureMgr->get("gravel.jpg"))
		->setShininess(82.0f)
		->setSpecular(glm::vec4(glm::vec3(1.0f) * glm::gaussRand(0.5f, 0.5f), 0.0f));
	shaderDataMgr->load("crate")
		->fillTextureSlot(0, textureMgr->get("crate.jpg"))
		->setShininess(8.0f)
		->setSpecular(glm::vec4(glm::vec3(1.0f) * glm::gaussRand(0.5f, 0.5f), 1.0f));

	// add material
	materialMgr->load("box_cliff")
		->withShader(shaderMgr->getRandom())
		->withData(shaderDataMgr->get("cliff"));
	materialMgr->load("box_grass")
		->withShader(shaderMgr->getRandom())
		->withData(shaderDataMgr->get("grass"));
	materialMgr->load("box_gravel")
		->withShader(shaderMgr->getRandom())
		->withData(shaderDataMgr->get("gravel"));
	materialMgr->load("box_crate")
		->withShader(shaderMgr->getRandom())
		->withData(shaderDataMgr->get("crate"));

	// now add material set (a combination of material basically)
	matsetMgr->load("box_cliff")
		->addMaterial(materialMgr->get("box_cliff"));
	matsetMgr->load("box_grass")
		->addMaterial(materialMgr->get("box_grass"));
	matsetMgr->load("box_gravel")
		->addMaterial(materialMgr->get("box_gravel"));
	matsetMgr->load("box_crate")
		->addMaterial(materialMgr->get("box_crate"));

	// test to create object
	for (int i = 0; i < 30; i++) {
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
	}

	renderObjs.push_back(new MeshObject(
		meshMgr->load("export_test.bcf"),
		matsetMgr->load("island")
		->addMaterial(
			materialMgr->load("island")
			->withShader(shaderMgr->get("box"))
			->withData(shaderDataMgr->load("island")
				->fillTextureSlot(0, textureMgr->get("road.png"))
				->setSpecular(glm::vec4( glm::vec3(0.2f) * glm::gaussRand(0.2f, 0.1f) , 0.0 ))
				->setShininess(glm::gaussRand(10.0f, 10.0f))
			)
		)
	));
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
	SDL_Log("++TEXTURES++\n");
	textureMgr->printDebug();

	destroyImGui();
	// delete renderer
	delete m_renderer;
	// delete resource managers
	delete shaderMgr;
	delete shaderDataMgr;
	delete meshMgr;
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
		ImGui::Begin("App Config", NULL, ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "FPS: %d", fps);
			ImGui::SameLine();
			if (ImGui::Button("QUIT")) {
			    this->setRunFlag(false);
			}
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
							renderObjs[i]->ms = matsetMgr->getRandom();
						}
						rp->generateDebugString();
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
	
	int cur_x, cur_y;

	bool canHandleInput = true;
	if (!ImGui_ImplBowie_ProcessEvent(e)) {
		canHandleInput = !ImGui_ImplSDL2_ProcessEvent(e);
	}

	auto& io = ImGui::GetIO();

	switch (e->type) {
	case SDL_QUIT:
		this->setRunFlag(false);
		return;
	case SDL_WINDOWEVENT:
		if (e->window.event == SDL_WINDOWEVENT_RESIZED) {
			SDL_Log("Window resize: %d x %d\n", e->window.data1, e->window.data2);
			m_renderer->setViewport(0, 0, e->window.data1, e->window.data2);
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
		if (io.WantCaptureMouse) break;
		dragging = true;
		last_x = e->button.x;
		last_y = e->button.y;
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
	ImGui::NewFrame();
}

void Game::endRenderImGui() {
	ImGui::Render();	// generate render data, before rendering for real
}