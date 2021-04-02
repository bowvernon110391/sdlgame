
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
#include "RenderObjectData.h"
#include "BaseRenderObject.h"
#include "Shader.h"
#include "Mesh.h"
#include "Material.h"
#include "ResourceManager.h"

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
	shaderMgr->load("lightmap");

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
		->setSpecular(glm::vec4(glm::vec3(0.4f), 1.0f));
	shaderDataMgr->load("grass")
		->fillTextureSlot(0, textureMgr->get("grass.jpg"))
		->setShininess(5.f)
		->setSpecular(glm::vec4(glm::vec3(0.2f), 1.0f));
	shaderDataMgr->load("gravel")
		->fillTextureSlot(0, textureMgr->get("gravel.jpg"))
		->setShininess(82.0f)
		->setSpecular(glm::vec4(0.8f, 0.6f, 0.7f, 1.0f));
	shaderDataMgr->load("crate")
		->fillTextureSlot(0, textureMgr->get("crate.jpg"))
		->setShininess(8.0f)
		->setSpecular(glm::vec4(glm::vec3(0.5f), 1.0f));

	// add material
	materialMgr->load("box_cliff")
		->withShader(shaderMgr->get("box"))
		->withData(shaderDataMgr->get("cliff"));
	materialMgr->load("box_grass")
		->withShader(shaderMgr->get("box"))
		->withData(shaderDataMgr->get("grass"));
	materialMgr->load("box_gravel")
		->withShader(shaderMgr->get("box"))
		->withData(shaderDataMgr->get("gravel"));
	materialMgr->load("box_crate")
		->withShader(shaderMgr->get("box"))
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

	// now create random objects
	for (int i = 0; i < 30; i++) {
		glm::vec3 pos = glm::sphericalRand(8.0f);
		pos.y *= 0.2f;
		pos.y += 3.5f;
		BaseRenderObjectData* rod = (new RenderObjectData())
			->usePosition(pos)
			->useRotation(glm::angleAxis(
				glm::radians(
					glm::gaussRand(0.0f, 45.0f)
				), glm::normalize(glm::vec3(
					glm::gaussRand(0.0f, 1.0f),
					glm::gaussRand(0.0f, 1.0f),
					glm::gaussRand(0.0f, 1.0f)
				)))
			);

		renderObjs.push_back(new BaseRenderObject(meshMgr->getRandom(), rod, matsetMgr->getRandom()));
	}

	// spawn the level
	renderObjs.push_back(new BaseRenderObject(
		meshMgr->load("export_test.bcf"),
		new RenderObjectData(),
		matsetMgr->load("scene_matset")
		->addMaterial(materialMgr->load("scene_mat")
			->withShader(shaderMgr->get("box"))
			->withData(shaderDataMgr->load("island")
				->fillTextureSlot(0, textureMgr->get("road.png"))
				->setSpecular(glm::vec4(0.2, 0.2, 0.2, 1.0))
				->setShininess(2.5f)
			)
		)
	));

	//Mesh* cube = Mesh::createUnitBox()->createBufferObjects();
	//meshes.push_back(cube);

	//Shader* cubeShader = Shader::fromFile("shaders/box.vert", "shaders/box.frag");
	//shaders.push_back(cubeShader);

	//Texture2D* tex = Texture2D::loadFromFile("textures/crate.jpg", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT, true);
	//textures.push_back(tex);
	//tex = Texture2D::loadFromFile("textures/cliff.jpg", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT, true);
	//textures.push_back(tex);
	//tex = Texture2D::loadFromFile("textures/grass.jpg", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT, true);
	//textures.push_back(tex);
	//tex = Texture2D::loadFromFile("textures/gravel.jpg", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT, true);
	//textures.push_back(tex);

	//ShaderData* shData = ((new ShaderData())
	//	->fillTextureSlot(0, textures[0])
	//	->setShininess(glm::linearRand(0.0f, 255.0f))
	//	);
	//shaderDatas.push_back(shData);
	//shData = ((new ShaderData())
	//	->fillTextureSlot(0, textures[1])
	//	->setShininess(glm::linearRand(0.0f, 255.0f))
	//	);
	//shaderDatas.push_back(shData);
	//shData = ((new ShaderData())
	//	->fillTextureSlot(0, textures[2])
	//	->setShininess(glm::linearRand(0.0f, 255.0f))
	//	);
	//shaderDatas.push_back(shData);
	//shData = ((new ShaderData())
	//	->fillTextureSlot(0, textures[3])
	//	->setShininess(glm::linearRand(0.0f, 255.0f))
	//	);
	//shaderDatas.push_back(shData);

	//Material* mat = new Material(cubeShader, shaderDatas[0]);
	//mats.push_back(mat);
	//mat = new Material(cubeShader, shaderDatas[1]);
	//mats.push_back(mat);
	//mat = new Material(cubeShader, shaderDatas[2]);
	//mats.push_back(mat);
	//mat = new Material(cubeShader, shaderDatas[3]);
	//mats.push_back(mat);

	//MaterialSet* cms = MaterialSet::create()->addMaterial(mats[0]);
	//matsets.push_back(cms);
	//cms = MaterialSet::create()->addMaterial(mats[1]);
	//matsets.push_back(cms);
	//cms = MaterialSet::create()->addMaterial(mats[2]);
	//matsets.push_back(cms);
	//cms = MaterialSet::create()->addMaterial(mats[3]);
	//matsets.push_back(cms);

	//// now create some random cube?
	//srand(SDL_GetTicks());
	//for (int i = 0; i < 20; i++) {
	//	BaseRenderObjectData* rod = (new RenderObjectData())
	//		->usePosition(glm::ballRand(5.0f))
	//		->useRotation( glm::angleAxis(
	//			glm::radians(
	//				glm::gaussRand(0.0f, 45.0f)
	//			), glm::normalize(glm::vec3(
	//				glm::gaussRand(0.0f, 1.0f),
	//				glm::gaussRand(0.0f, 1.0f),
	//				glm::gaussRand(0.0f, 1.0f)
	//			)) ) 
	//		);

	//	BaseRenderObject* box = new BaseRenderObject(cube, rod, matsets[rand() % matsets.size()]);
	//	renderObjs.push_back(box);
	//}
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
	for (auto it = renderObjs.begin(); it != renderObjs.end(); ++it) delete* it;
	/*for (auto it = shaders.begin(); it != shaders.end(); ++it) delete* it;
	for (auto it = meshes.begin(); it != meshes.end(); ++it) delete* it;
	for (auto it = shaderDatas.begin(); it != shaderDatas.end(); ++it) delete* it;
	for (auto it = textures.begin(); it != textures.end(); ++it) delete* it;
	for (auto it = mats.begin(); it != mats.end(); ++it) delete* it;
	for (auto it = matsets.begin(); it != matsets.end(); ++it) delete* it;*/

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

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// 3d rendering
	glEnable(GL_DEPTH_TEST);
	m_renderer->draw(this->renderObjs);

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