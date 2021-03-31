
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
#include "LightmapShader.h"

#include "btBulletDynamicsCommon.h"

using std::vector;

static vector<Light> lights;
static GShader::DebugLightShader* lightShader;
static GShader::BoxShader* boxShader;
static GShader::LightmapShader* lightmapShader;

static Texture2D** sceneTextures;
static Texture2D* lightmapTexture;

static bool dragStart = false;

static bool renderInterpolation = true;

static int dragPos[2] = {};
static int lastDragPos[2] = {};
static float camRotHorz, camRotVert;
static float camRateHorz = 0.01f, camRateVert = 0.005f;
static float camDist;

static int accelId=-1, gyroId=-1;
static float accelerometer[6] = {};
static float gyro[6] = {};
SDL_Sensor* accelSensor = NULL, * gyroSensor = NULL;

static bool followTilt = false;
static bool camUseTilt = false;
static float tiltAngle;

// physical world?
btDynamicsWorld* world;
btBroadphaseInterface* broadphase;
btCollisionConfiguration* collisionConfig;
btCollisionDispatcher* collisionDispatch;
btConstraintSolver* solver;

btAlignedObjectArray<btCollisionShape*> colShapes;
btAlignedObjectArray<btRigidBody*> bodies;

btRigidBody* ground = NULL;

void initSensors() {
	// init all data
	accelId = -1;
	gyroId = -1;
	memset(&accelerometer[0], 0, sizeof(accelerometer));
	memset(&gyro[0], 0, sizeof(gyro));
	accelSensor = gyroSensor = NULL;
	
	int numSensors = SDL_NumSensors();

	for (int i = 0; i < numSensors; i++) {
		SDL_SensorType type = SDL_SensorGetDeviceType(i);
		if (type == SDL_SENSOR_ACCEL && accelId < 0) {
			accelId = SDL_SensorGetDeviceInstanceID(i);

			accelSensor = SDL_SensorOpen(i);
		}

		if (type == SDL_SENSOR_GYRO && gyroId < 0) {
			gyroId = SDL_SensorGetDeviceInstanceID(i);

			gyroSensor = SDL_SensorOpen(i);
		}
	}
}

void destroySensors() {
	if (accelSensor) {
		SDL_SensorClose(accelSensor);
	}

	if (gyroSensor) {
		SDL_SensorClose(gyroSensor);
	}

	SDL_QuitSubSystem(SDL_INIT_SENSOR);
}

void pollSensors() {
	SDL_SensorUpdate();

	/*if (accelSensor) {
		if (SDL_SensorGetData(accelSensor, accelerometer, 6) < 0)
			SDL_LogError(SDL_LOG_CATEGORY_INPUT, "Accelerometer error %s", SDL_GetError());
		else
			SDL_Log("Accelerometer polled!");
	}

	if (gyroSensor) {
		if (SDL_SensorGetData(gyroSensor, gyro, 6) < 0)
			SDL_LogError(SDL_LOG_CATEGORY_INPUT, "Gyroscope error %s", SDL_GetError());
		else
			SDL_Log("Gyroscope polled!");
	}*/
}

void groundPreTick(btDynamicsWorld* wrld, btScalar dt) {
	btRigidBody* groundObj = (btRigidBody*)wrld->getWorldUserInfo();
	if (groundObj) {
		//SDL_Log("Do something with the kinematic here! %.2f", dt);

		// integrate some shit
		btVector3 linvel(0, 0, 0);
		float rotSpeed = 0.5;
		btVector3 angvel(0, rotSpeed, 0);

		btTransform target, current;

		// grab current, and predict target
		groundObj->getMotionState()->getWorldTransform(current);
		//current = groundObj->getWorldTransform();
		// compute target
		//btTransformUtil::integrateTransform(current, linvel, angvel, dt, target);
		// set it
		//groundObj->getMotionState()->setWorldTransform(target);

		// compute real shit
		// compute the tilted box direction?
		float cosVert = glm::cos(camRotVert);
		float sinVert = glm::sin(camRotVert);
		btVector3 axis(
			glm::sin(camRotHorz) * cosVert,
			sinVert,
			glm::cos(camRotHorz) * cosVert
		);

		btQuaternion qRot(axis, btRadians(-tiltAngle));
		current.setRotation(qRot);
		ground->getMotionState()->setWorldTransform(current);
	}
}

bool initPhysicsWorld() {
	srand(time(0));

	colShapes.clear();
	bodies.clear();
	// set gravity
	world->setGravity(btVector3(0, -9.8, 0));

	// create big box
	btCollisionShape* groundShape = new btBoxShape(btVector3(2.5f, .25f, 2.5f));
	colShapes.push_back(groundShape);

	btTransform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(btVector3(0, 0, 0));

	btScalar mass(0.f);
	btVector3 localInertia(0, 0, 0);
	btMotionState* motionState = new btDefaultMotionState(groundTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, groundShape, localInertia);
	btRigidBody* b = new btRigidBody(rbInfo);

	b->setUserIndex(-1);
	b->setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT);
	b->forceActivationState(DISABLE_DEACTIVATION);

	if (b->isStaticObject()) {
		SDL_Log("Added as STATIC");
	}
	else {
		SDL_Log("Added as NON-STATIC");
	}
	b->setAngularVelocity(btVector3(200, 100, 300));
	b->setFriction(0.4f);
	world->addRigidBody(b);
	bodies.push_back(b);

	ground = b;

	world->setInternalTickCallback(groundPreTick, ground, true);

	// add several bodies
	{
		btCollisionShape* boxShape = new btBoxShape(btVector3(.5f, .5f, .5f));
		colShapes.push_back(boxShape);

		btTransform boxTransform;
		btScalar mass(1.0f);
		
		for (int i = 0; i < 15; i++) {
			boxTransform.setIdentity();
			boxTransform.setOrigin(btVector3(glm::linearRand(-1.0f, 1.0f), 4.f + i*1.1f, glm::linearRand(-1.0f, 1.0f)));
			btMotionState* motion = new btDefaultMotionState(boxTransform);
			btVector3 localInertia;
			boxShape->calculateLocalInertia(mass, localInertia);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motion, boxShape, localInertia);

			btRigidBody* b = new btRigidBody(rbInfo);
			b->setFriction(glm::gaussRand(0.6f, 0.4f));
			world->addRigidBody(b);
			bodies.push_back(b);
		}
	}

	return true;
}

void destroyPhysicsData() {
	for (int i = 0; i < colShapes.size(); i++) {
		delete colShapes[i];
	}
	colShapes.clear();
	bodies.clear();
}

Game::Game():
App(40, "Game Test"),
speed(0.f), 
lightSpeed(0.1f),
angle(0.0f),
animate(true),
perspectiveFOV(57.0f),
orthoRange(10.0f),
projectionType(PROJECTION_PERSPECTIVE)
{
    cube = nullptr;
	scene = nullptr;
	
	bgColor[0] = .2f;
	bgColor[1] = .5f;
	bgColor[2] = .72f;
	bgColor[3] = 1.f;
}

Game::~Game() {
}

void Game::onInit() {
	// setup camera
	camRotHorz = 0.0f;
	camRotVert = glm::radians(45.0f);
	camDist = 10.0f;

	// follow tilt is false
	followTilt = false;
	tiltAngle = 0.0f;	// relative to view position ofc

	// init sensor
	initSensors();

    SDL_Log("Renderer : %s", glGetString(GL_RENDERER));
    SDL_Log("Vendor : %s", glGetString(GL_VENDOR));
    SDL_Log("Version : %s", glGetString(GL_VERSION));
    SDL_Log("GLSL Ver : %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

	// checking precision
	if (glGetShaderPrecisionFormat) {
		GLint range[2], precision;
		glGetShaderPrecisionFormat(GL_FRAGMENT_SHADER, GL_MEDIUM_FLOAT, range, &precision);
		SDL_Log("MEDIUMP: precision(%d), range(%d to %d)", precision, range[0], range[1]);

		glGetShaderPrecisionFormat(GL_FRAGMENT_SHADER, GL_LOW_FLOAT, range, &precision);
		SDL_Log("LOWP: precision(%d), range(%d to %d)", precision, range[0], range[1]);
	}

	view = glm::lookAt(glm::vec3(0, 0, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

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

	scene = Mesh::loadBCFFromFile("meshes/scene.bcf");
	if (scene) {
		scene->createBufferObjects();
	}

	SDL_Log("Mesh Data (%s): vtxformat(%d), vbsize(%d), idxsize(%d), submeshes(%d), bytespervertex(%d)",
		scene->name, scene->vertexFormat, scene->vertexBufferSize, scene->indexBufferSize, 
		scene->subMeshes.size(), scene->strideLength);

	// load debug light shader
	lightShader = new GShader::DebugLightShader;
	lightShader->loadFromFile("shaders/lightdebug.vert", "shaders/lightdebug.frag");
	lightShader->setUniformLocs();

	SDL_Log("Test: lookup(texture2, viewport_dimension) = %d, %d\n", Shader::getUniformId("texture2"), Shader::getUniformId("viewport_dimension"));

	// load box shader
	boxShader = new GShader::BoxShader;
	boxShader->loadFromFile("shaders/box.vert", "shaders/box.frag");
	boxShader->setUniformLocs();

	// load lightmap shader
	lightmapShader = new GShader::LightmapShader;
	lightmapShader->loadFromFile("shaders/lightmap.vert", "shaders/lightmap.frag");
	lightmapShader->setUniformLocs();

	sceneTextures = new Texture2D * [3];
	sceneTextures[0] = Texture2D::loadFromFile("textures/cliff.jpg", GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR, GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T, true);
	sceneTextures[1] = Texture2D::loadFromFile("textures/grass.jpg", GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR, GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T, true);
	sceneTextures[2] = Texture2D::loadFromFile("textures/gravel.jpg", GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR, GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T, true);
	
	lightmapTexture = Texture2D::loadFromFile("textures/lightmap.png", GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR, GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T, true);

	// set ambient color
	boxShader->ambientColor = glm::vec3(0.12f, 0.1f, 0.25f);

	// add one light?
	lights.clear();

	Light l;
	l.pos = glm::vec3(-1.5f, 5.5f, 21.1f);
	l.color = glm::vec3(1.f, .82f, .293f);
	l.attenuation = glm::vec3(1.f, 0.0015f, 0.00125f);
	lights.push_back(l);

	l.pos = glm::vec3(22.0f, 7.0f, -1.2f);
	l.color = glm::vec3(.8f, .41f, .97f);
	l.attenuation = glm::vec3(1.f, .001f, .0015f);
	lights.push_back(l);

	tex = Texture2D::loadFromFile("textures/crate.jpg", GL_LINEAR_MIPMAP_LINEAR,
		GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);

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
	glEnableVertexAttribArray(ATTRIB_UV2_LOC);

	// enable msaa x4
	glEnable(GL_MULTISAMPLE);

	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	// init imgui
	initImGui();

	// init physics
	collisionConfig = new btDefaultCollisionConfiguration();
	collisionDispatch = new btCollisionDispatcher(collisionConfig);
	broadphase = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver();
	world = new btDiscreteDynamicsWorld(collisionDispatch, broadphase, solver, collisionConfig);

	initPhysicsWorld();
}

void Game::onDestroy() {
    // do clean up here rather than at destructor
    // if (simple) delete simple;
	if (scene) delete scene;
	if (cube) delete cube;
	if (tex) delete tex;
	if (lightShader) delete lightShader;
	if (boxShader) delete boxShader;
	if (lightmapShader) delete lightmapShader;

	if (lightmapTexture) delete lightmapTexture;
	if (sceneTextures[0]) delete sceneTextures[0];
	if (sceneTextures[1]) delete sceneTextures[1];
	if (sceneTextures[2]) delete sceneTextures[2];
	delete[] sceneTextures;

	// cleanup imgui
	destroyImGui();

	// destroy physics
	delete world;
	delete solver;
	delete broadphase;
	delete collisionDispatch;
	delete collisionConfig;

	destroyPhysicsData();

	destroySensors();
}

static void updateTilt(SDL_DisplayOrientation ori) {
	tiltAngle = 0.0f;
	if (!accelSensor) {
		followTilt = false;
	}
	else if (followTilt) {
		// do computation?
		glm::vec3 accelDir(accelerometer[0], accelerometer[1], accelerometer[2]);

		// rotate if necessary
		if (ori == SDL_ORIENTATION_LANDSCAPE) {
			SDL_Log("LANDSCAPE RIGHT!");
			// -90 degree rotate
			float tmp = accelDir.x;
			accelDir.x = accelDir.y;
			accelDir.y = -tmp;
		}
		else if (ori == SDL_ORIENTATION_LANDSCAPE_FLIPPED) {
			// 90 degree rotate
			SDL_Log("LANDSCAPE LEFT!");
			float tmp = accelDir.x;
			accelDir.x = -accelDir.y;
			accelDir.y = tmp;
		}

		SDL_Log("upVector: %.2f %.2f", accelDir.x, -accelDir.y);
        tiltAngle = glm::degrees(glm::atan(accelDir.x, -accelDir.y));
	}
}

void Game::onUpdate(float dt) {
	pollSensors();
	updateTilt(getScreenOrientation());

	if (animate) {
		// update angle
		angle += speed * dt;

		for (int i = 0; i < lights.size(); i++) {
			Light& l = lights[i];

			float rotation = lightSpeed * l.pos.length() * dt;

			glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0,1,0));

			glm::vec4 newPos = glm::vec4(l.pos.x, l.pos.y, l.pos.z, 1.0f);
			newPos = rotate * newPos;

			l.pos = glm::vec3(newPos.x, newPos.y, newPos.z);
		}
		
		world->stepSimulation(dt, 0, dt);

		// reset position if it's over the limit
		for (int j = 0; j < world->getNumCollisionObjects(); j++) {
			btCollisionObject* colObj = world->getCollisionObjectArray()[j];
			btRigidBody* b = btRigidBody::upcast(colObj);

			btTransform trans;
			b->getMotionState()->getWorldTransform(trans);

			btVector3 pos = trans.getOrigin();

			if (pos.y() < -10.0f) {
				// reset position
				btVector3 newPos(
					btClamped<float>(pos.x(), -1.0f, 1.0f),
					10.0f,
					btClamped<float>(pos.z(), -1.0f, 1.0f)
				);

				btVector3 vel = b->getLinearVelocity();
				vel.setX(0);
				vel.setZ(0);

				trans.setOrigin(newPos);
				b->clearForces();
				b->clearGravity();
				b->setLinearVelocity(vel);
				//b->setAngularVelocity(btVector3(3, 10, 2));
				b->setWorldTransform(trans);
			}
		}
	}

	// handle rotation
	if (dragStart) {
		// rotate camera matrix in its local x and y
		float yRot = dragPos[1] - lastDragPos[1];
		float xRot = dragPos[0] - lastDragPos[0];

		// snap last drag to current
		lastDragPos[0] = dragPos[0];
		lastDragPos[1] = dragPos[1];

		// do the heavy computation
		//glm::vec3 yAxis = glm::vec3(glm::transpose(view) * glm::vec4(0, 1, 0, 0));
		//glm::vec3 xAxis = glm::vec3(glm::transpose(view) * glm::vec4(1, 0, 0, 0));

		//// rotate it along yAxis for xRot
		//view = glm::rotate(view, glm::radians(xRot), yAxis);
		//view = glm::rotate(view, glm::radians(yRot), xAxis);
		camRotHorz += -xRot * camRateHorz;
		camRotVert += yRot * camRateVert;

		// clamp vertical rotation
		const float half_pi = 3.14f * 0.5f;
		camRotVert = glm::fclamp(camRotVert, -half_pi, half_pi);
	}
}

void Game::computeCameraMatrix() {
	// update view
	float cx, cy, cz;
	
	cy = glm::sin(camRotVert) * camDist;

	float scaleHorz = glm::cos(camRotVert);
	cz = glm::cos(camRotHorz) * scaleHorz * camDist;
	cx = glm::sin(camRotHorz) * scaleHorz * camDist;

	view = glm::lookAt(glm::vec3(cx, cy, cz), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	// let's tilt
	if (camUseTilt) {
		// compute rotation matrix
		glm::mat4 camZrot = glm::rotate(glm::mat4(1.0f), glm::radians(tiltAngle), glm::vec3(0, 0, 1));
		view = camZrot * view;
	}
}

/* Render function */
void Game::onRender(float dt) {
	//SDL_Log("render dt: %.4f", dt);
	// only interpolate if we're not paused
	if (renderInterpolation && animate)
		world->stepSimulation(dt, 10, (btScalar)1.0/(btScalar)iTickRate);

    float newAngle = angle + speed * dt;
	if (!animate)
		newAngle = angle;

	computeProjection();
	computeCameraMatrix();
    
    // do render here
    glViewport(0, 0, iWidth, iHeight);
    //glClearColor(sA, cA, aA, 1.0f);
	glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// setup box shader
	boxShader->prepareState();
	boxShader->use();
	//boxShader->setTransformData(mvp, modelView, scale);
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

	// draw each cube
	for (int j = 0; j < world->getNumCollisionObjects(); j++) {
		btCollisionObject* colObj = world->getCollisionObjectArray()[j];
		btRigidBody* b = btRigidBody::upcast(colObj);
		btTransform trans;
		b->getMotionState()->getWorldTransform(trans);
		
		btScalar m[16];

		trans.getOpenGLMatrix(m);
		glm::mat4 model = glm::make_mat4(m);
		glm::mat4 modelView = view * model;
		glm::mat4 mvp = proj * modelView;

		/*SDL_Log("=======================");
		SDL_Log("Object %d", j);
		float *md = glm::value_ptr(model);
		for (int c = 0; c < 4; c++) {
			SDL_Log("%.2f %.2f %.2f %.2f", md[c+0], md[c+4], md[c+8], md[c+12]);
		}*/

		btBoxShape* box = (btBoxShape*)b->getCollisionShape();
		btVector3 half = box->getHalfExtentsWithMargin();

		glm::vec3 scale(half.x() * 2, half.y() * 2, half.z() * 2);

		// set draw data
		boxShader->setTransformData(mvp, modelView, scale);

		// draw call
		for (int i = 0; i < cube->subMeshes.size(); i++) {
			Mesh::SubMesh& s = cube->subMeshes[i];

			//glDrawElements(GL_TRIANGLES, s.elemCount, GL_UNSIGNED_SHORT, (void*)s.idxBegin);
		}
	}

	// draw scene test
	glEnable(GL_CULL_FACE);
	lightmapShader->prepareState();
	lightmapShader->use();

	glm::mat4 mvp = proj * view * glm::mat4(1.0f);

	lightmapShader->setTexture1(lightmapTexture);
	lightmapShader->setTransformData(mvp);

	scene->use();

	glVertexAttribPointer(ATTRIB_POS_LOC, 3, GL_FLOAT, false, scene->strideLength, (void*)0);
	glVertexAttribPointer(ATTRIB_NORMAL_LOC, 3, GL_FLOAT, false, scene->strideLength, (void*)12);
	glVertexAttribPointer(ATTRIB_UV_LOC, 2, GL_FLOAT, false, scene->strideLength, (void*)24);
	glVertexAttribPointer(ATTRIB_UV2_LOC, 2, GL_FLOAT, false, scene->strideLength, (void*)32);

	// draw call
	for (int i = 0; i < scene->subMeshes.size(); i++) {
		const Mesh::SubMesh& s = scene->subMeshes[i];
		lightmapShader->setTexture0(sceneTextures[i]);
		glDrawElements(GL_TRIANGLES, s.elemCount, GL_UNSIGNED_SHORT, (void*)s.idxBegin);
	}
	glDisable(GL_CULL_FACE);

	// draw lights?
	// setup shader
	lightShader->prepareState();
	lightShader->use();

	// setup buffer
	cube->use();
	glVertexAttribPointer(ATTRIB_POS_LOC, 3, GL_FLOAT, false, cube->strideLength, (void*)0);

	// draw each light
	for (int i = 0; i < lights.size(); i++) {
		const Light& l = lights[i];
		lightShader->setDrawData(l, view, proj);


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
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "FPS: %03d, Tilt: %.2f deg", fps, tiltAngle);
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

	// sensor data?
	ImGui::TextColored(ImVec4(1, 0, 0, 1), "Accel[%d]: %.2f %.2f %.2f | %.2f %.2f %.2f",
		accelId, accelerometer[0], accelerometer[1], accelerometer[2],
		accelerometer[3], accelerometer[4], accelerometer[5]
	);

	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Gyro[%d]: %.2f %.2f %.2f | %.2f %.2f %.2f",
		gyroId, gyro[0], gyro[1], gyro[2],
		gyro[3], gyro[4], gyro[5]
	);


	ImGui::End();

	ImGui::Begin("App Config", NULL, ImGuiWindowFlags_AlwaysAutoResize);
	//ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Render speed: %d FPS", fps);
	ImGui::SliderFloat("Light Speed", &lightSpeed, 0.0f, 2.0f, "%.2f");
	ImGui::SliderFloat("Cam Dist", &camDist, 1.0f, 20.0f, "%.2f m");
	ImGui::Checkbox("Animate?", &animate);
	ImGui::Checkbox("Interpolate", &renderInterpolation);
	ImGui::Checkbox("Change Bg Col", &showColorPicker);

	ImGui::Checkbox("Use tilt", &followTilt);
	ImGui::SameLine();
	ImGui::Checkbox("Cam tilt", &camUseTilt);

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
		ImGui::SameLine();
		ImGui::ColorPicker3("Ambient", glm::value_ptr(boxShader->ambientColor));
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
	// override imgui sdl event processing
	bool canHandleInput = true;
	if (!ImGui_ImplBowie_ProcessEvent(e)) {
		canHandleInput = !ImGui_ImplSDL2_ProcessEvent(e);
	}

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

			int wx, wy;
			SDL_GetWindowPosition(wndApp, &wx, &wy);

			SDL_Log("Window resized to: (%d x %d) @ pos(%d, %d)", 
				this->iWidth, this->iHeight,
				wx, wy
			);

			computeProjection();
		}
		if (e->window.event == SDL_WINDOWEVENT_RESIZED) {
			SDL_Log("Window resized to %dx%d", e->window.data1, e->window.data2);
		}
		if (e->window.event == SDL_WINDOWEVENT_MOVED) {
			SDL_Log("Window moved to %d, %d", e->window.data1, e->window.data2);
		}
	}
	else if (!io.WantCaptureMouse) {
		if (e->type == SDL_MOUSEBUTTONDOWN) {
			if (e->button.button == SDL_BUTTON_LEFT) {
				dragStart = true;

				lastDragPos[0] = dragPos[0] = e->button.x;
				lastDragPos[1] = dragPos[1] = e->button.y;
			}
		}

		if (e->type == SDL_MOUSEBUTTONUP) {
			dragStart = false;
		}

		if (e->type == SDL_MOUSEMOTION && dragStart) {
			dragPos[0] = e->motion.x;
			dragPos[1] = e->motion.y;
		}

		if (e->type == SDL_MOUSEWHEEL) {
			camDist += e->wheel.y * -0.1f;
		}
	} 

	// handle tilt
	if (e->type == SDL_SENSORUPDATE) {
		//SDL_Log("GOT SENSOR EVENT!");
		SDL_Sensor* sensor = SDL_SensorFromInstanceID(e->sensor.which);
		float* data = &e->sensor.data[0];
		Uint32 timestamp = e->sensor.timestamp;
		
		if (sensor == accelSensor) {
			/*SDL_Log("FOR ACCEL @ %d! %.2f %.2f %.2f : %.2f %.2f %.2f", 
				timestamp,
				data[0], data[1], data[2], data[3], data[4], data[5]
				);*/
			// copy data
			memcpy(&accelerometer[0], data, sizeof(accelerometer));
		}
		else if (sensor == gyroSensor) {
			/*SDL_Log("FOR GYRO @ %d! %.2f %.2f %.2f : %.2f %.2f %.2f",
				timestamp,
				data[0], data[1], data[2], data[3], data[4], data[5]
			);*/
			// copy data
			memcpy(&gyro[0], data, sizeof(gyro));
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