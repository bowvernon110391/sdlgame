#include "Renderer.h"
#include "Camera.h"
#include "SceneData.h"
#include "Mesh.h"
#include "Material.h"
#include "Shader.h"
#include "ShaderData.h"
#include "RenderPass.h"
#include "ColorDepthPass.h"
#include "DebugDrawPass.h"

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

//#define DEBUG_VA

Renderer::Renderer() {
	m_camera = 0;
	m_scene = 0;
	lastAttribFlags = 0;

	initDebugData();
	createPasses();
}

Renderer::~Renderer() {
	if (m_camera) {
		delete m_camera;
		m_camera = 0;
	}

	if (m_scene) {
		delete m_scene;
		m_scene = 0;
	}

	// iterate over it?
	auto it = passes.begin();
	while (it != passes.end()) {
		delete (*it).second;
		it++;
	}

	destroyDebugData();
}

RenderPass* Renderer::addPass(const std::string& name, RenderPass* p)
{
	if (p) {
		// parent-child relation
		p->renderer = this;
		
		// add to passes
		passes.insert(std::make_pair(name, p));

		// also insert to ordered pass and reorder
		orderedPasses.push_back(p);

		// reorder
		std::sort(
			orderedPasses.begin(),
			orderedPasses.end(),
			[](const RenderPass* a, const RenderPass* b) {
				return a->priority < b->priority;
			});
	}
	return p;
}

void Renderer::createPasses() {
	// the most important, teh depth+color pass
	addPass("color_depth", new ColorDepthPass(5));
	// debug pass (optional)
	addPass("debug", new DebugDrawPass(6));
	
}

void Renderer::initDebugData() {
	// set color
	debugColor = glm::vec4(1.0, 0.0, 0.0, 0.1);
	// generate cube, with size 1 perhaps?
	// genereat buffers
	glGenBuffers(1, &vboDebug);
	glGenBuffers(1, &iboDebug);

	glBindBuffer(GL_ARRAY_BUFFER, vboDebug);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboDebug);

	// fill our vb
	vbDebug.clear();
	ibDebug.clear();

	vbDebug.resize(24);
	ibDebug.resize(36);

	float half = 0.5f;
	float verts[] = {
		// xyz uv
		// front
		-half, -half, half,	// 0
		 half, -half, half,	// 1
		 half,  half, half,	// 2
		-half,  half, half,	// 3

		// back
		 half, -half,-half,	// 4
		-half, -half,-half,	// 5
		-half,  half,-half,	// 6
		 half,  half,-half,	// 7
	};

	// index em (lines)
	uint16_t indices[24] = {
		0,1, 1,2, 2,3, 3,0,
		4,5, 5,6, 6,7, 7,4,
		1,4, 2,7, 6,3, 5,0
	};

	vbDebug.insert(vbDebug.end(), &verts[0], &verts[24]);
	ibDebug.insert(ibDebug.end(), &indices[0], &indices[24]);

	glBufferData(GL_ARRAY_BUFFER, vbDebug.size() * sizeof(float), &vbDebug[0], GL_STREAM_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ibDebug.size() * sizeof(unsigned short), &ibDebug[0], GL_STREAM_DRAW);

	// init shader
	const char* vs = "\
		attribute vec3 position; \n\
		uniform mat4 m_model_view_projection; \n\
		varying vec3 col; \n\
		void main() { \n\
			col = max(position, vec3(0.1,0.1,0.1)); \n\
			gl_Position = m_model_view_projection * vec4(position, 1.0); \n\
		} \n\
		";

	const char* fs = "\
	 	#ifdef GL_ES	\n\
		precision mediump float;	\n\
		#endif	\n\
		uniform vec4 material_diffuse; \n\
		varying vec3 col; \n\
		void main() { \n\
			gl_FragColor = vec4(col, 1.0) * material_diffuse; \n\
		} \n\
		";

	debugShader = Shader::fromMemory(vs, strlen(vs), fs, strlen(fs));
}

void Renderer::generateDebugData(const AABB& b) {
	// just recreate the vertbuffer
	assert(vbDebug.size() != 24);

	const glm::vec3& min = b.min;
	const glm::vec3& max = b.max;

	// make sure to follow the format
	vbDebug[0] = min.x;	vbDebug[1] = min.y; vbDebug[2] = max.z;
	vbDebug[3] = max.x;	vbDebug[4] = min.y; vbDebug[5] = max.z;
	vbDebug[6] = max.x;	vbDebug[7] = max.y; vbDebug[8] = max.z;
	vbDebug[9] = min.x;	vbDebug[10] = max.y; vbDebug[11] = max.z;
	
	vbDebug[12] = max.x;	vbDebug[13] = min.y; vbDebug[14] = min.z;
	vbDebug[15] = min.x;	vbDebug[16] = min.y; vbDebug[17] = min.z;
	vbDebug[18] = min.x;	vbDebug[19] = max.y; vbDebug[20] = min.z;
	vbDebug[21] = max.x;	vbDebug[22] = max.y; vbDebug[23] = min.z;

	glBindBuffer(GL_ARRAY_BUFFER, vboDebug);
	glBufferData(GL_ARRAY_BUFFER, vbDebug.size() * sizeof(float), &vbDebug[0], GL_STREAM_DRAW);
}

void Renderer::destroyDebugData() {
	vbDebug.clear();
	ibDebug.clear();
	glDeleteBuffers(1, &vboDebug);
	glDeleteBuffers(1, &iboDebug);
	delete debugShader;
}

Renderer* Renderer::useCamera(Camera* cam) {
	if (m_camera && cam) {
		delete m_camera;
	}
	m_camera = cam;
	return this;
}

Renderer* Renderer::useSceneData(SceneData* sd) {
	if (m_scene && sd) {
		delete m_scene;
	}
	m_scene = sd;
	return this;
}

Renderer* Renderer::setViewport(int x, int y, int w, int h) {
#ifdef _DEBUG
	SDL_Log("RENDERER_SET_VIEWPORT: %d %d %d %d\n", x, y, w, h);
#endif // _DEBUG

	viewport[0] = x;
	viewport[1] = y;
	viewport[2] = w;
	viewport[3] = h;

	if (m_camera)
		m_camera->setAspect((float)w / (float)h);

	return this;
}

void Renderer::draw(std::vector<AbstractRenderObject*>& objs, float dt)
{
	// compute camera and scene data
	m_scene->updateLightsUniformData(m_camera->getPosition());

	// just pass it to each pass that is interested in it
	// TODO: MAKE IT CONFIGURABLE PER OBJECT, EACH OBJECT COULD BE IN MULTIPLE PASS

	// for each render pass, SHOULD GRAB DATA FROM SCENE NODE (BETTER!)
	for (RenderPass* p : orderedPasses) {
		// 1st step, clear each pass
		p->clear();

		// 2nd step, put each obj in its respective render pass?
		// TODO: BETTER TO SUPPLY A SCENEGRAPH, SO EACH PASS COULD GRAB ITS OWN OBJECTS
		for (AbstractRenderObject* o : objs) {
			p->processRenderObject(o);
		}

		// 3rd step, draw each pass
		p->draw(dt);
	}
}

void Renderer::setupVertexState(const Shader* shd) {
	// disable all shits, then enable the relevant
	for (int i = Shader::AttribLoc::position; i < Shader::AttribLoc::custom_attribute; i++) {
		if (shd->attributeFlags & (1 << i)) {
			glEnableVertexAttribArray(shd->getAttribLocation(i));
		}
	}
}


void Renderer::setupVertexArray(const Shader* shd, const Mesh* m)
{
#ifdef DEBUG_VA
	SDL_Log("VERTEX_ARRAY: shd_attrib_flags(%d), mesh_vtx_format(%d)\n", shd->attributeFlags, m->vertexFormat);
#endif // DEBUG_VA

	// now just call glVertexAttribArray on all possible shiets
	size_t offset = 0;
	int a_loc = -1;

	// gotta follow mesh vertex format
	// position
	a_loc = shd->getAttribLocation(Shader::AttribLoc::position);
	if ((m->vertexFormat & VF_XYZ) && a_loc >= 0) {
#ifdef DEBUG_VA
		SDL_Log("VERTEX_ARRAY[%d]: POSITION @ %d bytes\n", a_loc, offset);
#endif // DEBUG

		glVertexAttribPointer(a_loc, 3, GL_FLOAT, false, m->strideLength, (void*)offset);
	}
	offset += ((m->vertexFormat & VF_XYZ)) * sizeof(float) * 3;

	// normal
	a_loc = shd->getAttribLocation(Shader::AttribLoc::normal);
	if ((m->vertexFormat & VF_NORMAL) && a_loc >= 0) {
#ifdef DEBUG_VA
		SDL_Log("VERTEX_ARRAY[%d]: NORMAL @ %d bytes\n", a_loc, offset);
#endif // DEBUG
		
		glVertexAttribPointer(a_loc, 3, GL_FLOAT, false, m->strideLength, (void*)offset);
	}
	offset += ((m->vertexFormat & VF_NORMAL) >> 1) * sizeof(float) * 3;

	// uv
	a_loc = shd->getAttribLocation(Shader::AttribLoc::uv);
	if ((m->vertexFormat & VF_UV) && a_loc >= 0) {
#ifdef DEBUG_VA
		SDL_Log("VERTEX_ARRAY[%d]: UV @ %d bytes\n", a_loc, offset);
#endif // DEBUG

		glVertexAttribPointer(a_loc, 2, GL_FLOAT, false, m->strideLength, (void*)offset);
	}
	offset += ((m->vertexFormat & VF_UV) >> 2) * sizeof(float) * 2;

	// tangent, bitangent
	if (m->vertexFormat & VF_TANGENT) {
		// tangent first
		a_loc = shd->getAttribLocation(Shader::AttribLoc::tangent);
		if (a_loc >= 0) {
#ifdef DEBUG_VA
			SDL_Log("VERTEX_ARRAY[%d]: TANGENT @ %d bytes\n", a_loc, offset);
#endif // DEBUG
			glVertexAttribPointer(a_loc, 3, GL_FLOAT, false, m->strideLength, (void*)offset);
		}

		// then bitangent
		a_loc = shd->getAttribLocation(Shader::AttribLoc::bitangent);
		if (a_loc >= 0) {
#ifdef DEBUG_VA
			SDL_Log("VERTEX_ARRAY[%d]: BITANGENT @ %d bytes\n", a_loc, offset+sizeof(float)*3);
#endif // DEBUG
			glVertexAttribPointer(a_loc, 3, GL_FLOAT, false, m->strideLength, (void*)(offset + sizeof(float) * 3));
		}
	}
	offset += ((m->vertexFormat & VF_TANGENT) >> 3) * sizeof(float) * 6;

	// second uv
	a_loc = shd->getAttribLocation(Shader::AttribLoc::uv2);
	if ((m->vertexFormat & VF_UV2) && a_loc >= 0) {
#ifdef DEBUG_VA
		SDL_Log("VERTEX_ARRAY[%d]: UV2 @ %d bytes\n", a_loc, offset);
#endif // DEBUG
		glVertexAttribPointer(a_loc, 2, GL_FLOAT, false, m->strideLength, (void*)offset);
	}
	offset += ((m->vertexFormat & VF_UV2) >> 4) * sizeof(float) * 2;
}

// just draw something?


//void Renderer::draw(const std::vector<BaseRenderObject*>& data) {
//	// update scene data? and camera too ofc
//	if (m_scene && m_camera) {
//		m_scene->updateLightsUniformData(m_camera->getPosition());
//	}
//
//	// 1st pass, the color + Depth pass
//	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
//	glDepthMask(GL_TRUE);
//	glEnable(GL_DEPTH_TEST);
//	glDisable(GL_BLEND);
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//	// prepare pass state?
//	// first, gotta build a queue, then sort them all?
//	// nope, just iterate shit
//
//	// enable depth write
//
//	auto it = data.begin();
//	while (it != data.end()) {
//		BaseRenderObject* obj = *it;
//		// draw it, for every submesh it has
//		this->draw(*obj);
//
//		// increment
//		++it;
//	}
//
//	// 2nd pass, debug (only active sometime)
//	glDepthMask(GL_FALSE);
//	glEnable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
//	// draw debug too?
//	if (drawDebug) {
//		AABB tmp;
//		glm::mat4 mvp;
//		//glm::vec4 boxColor = glm::vec4(1.0f, 1.0f, 0.0f, 0.1f);
//
//		int a_pos = debugShader->getAttribLocation(Shader::AttribLoc::position);
//		assert(a_pos >= 0);
//
//		// compute mvp (aabb already in world space)
//		mvp = m_camera->getProjectionMatrix() * m_camera->getViewMatrix();
//
//		// bind buffer and shader
//		debugShader->bind();
//
//		// set uniforms
//		glUniformMatrix4fv(
//			debugShader->getUniformLocation(Shader::UniformLoc::m_model_view_projection), 
//			1, 
//			false, 
//			glm::value_ptr(mvp));
//
//		glBindBuffer(GL_ARRAY_BUFFER, vboDebug);
//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboDebug);
//
//		// enable vertex position?
//		glEnableVertexAttribArray(a_pos);
//
//		auto it = data.begin();
//		while (it != data.end()) {
//			// grab our bbox onleh
//			BaseRenderObject* obj = *it;
//
//			glUniform4fv(
//				debugShader->getUniformLocation(Shader::UniformLoc::material_diffuse),
//				1,
//				glm::value_ptr(debugColor));
//
//			if (obj->data->getBoundingBox(tmp)) {
//				// it can give us bounding box, now draw it
//				generateDebugData(tmp);
//
//				// point our data to updated buffer?
//				glVertexAttribPointer(a_pos, 3, GL_FLOAT, false, 3 * sizeof(float), 0);
//
//				// draw call
//				glDrawElements(GL_TRIANGLES, ibDebug.size(), GL_UNSIGNED_SHORT, 0);
//			}
//
//			++it;
//		}
//	}
//}

/// <summary>
/// draw a single object?
/// </summary>
/// <param name="ro"></param>
//void Renderer::draw(const BaseRenderObject& ro) {
//	// for every submeshes draw call it
//	const BaseRenderObjectData* rod = ro.data;
//	
//	
//	//SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "GL_ERROR: CODE = %d\n", glGetError());
//	//SDL_assert(glGetError() != GL_INVALID_OPERATION);
//
//	for (int i = 0; i < ro.mesh->subMeshes.size(); i++) {
//		// setup shader per submeshes
//		const Shader* sh = ro.mat->mats[i]->sh;
//		const ShaderData* shd = ro.mat->mats[i]->shData;
//		const Mesh::SubMesh* mesh = &ro.mesh->subMeshes[i];
//
//		// bind shader
//		sh->bind();
//		//SDL_assert(glGetError() == GL_NO_ERROR);
//
//		// bind buffer
//		ro.mesh->bind();
//
//		// enable vertex attributes based on shader attrib flags
//		setupVertexState(sh);
//		//SDL_assert(glGetError() == GL_NO_ERROR);
//		// set pointer too
//		setupVertexArray(sh, ro.mesh);
//		//SDL_assert(glGetError() == GL_NO_ERROR);
//		
//		// set per-pass data
//		this->setPassData(sh);
//		//SDL_assert(glGetError() == GL_NO_ERROR);
//		// TODO: GRAB ALL SCENE AND CAMERA DATA, INJECT SHADER USING THEM
//		// set per-material data
//		this->setMaterialData(sh, shd);
//		//SDL_assert(glGetError() == GL_NO_ERROR);
//		// set per-object data
//		// TODO: GRAB RENDER OBJECT DATA, INJECT SHADER USING THEM
//		this->setInstanceData(sh, rod);
//		//SDL_assert(glGetError() == GL_NO_ERROR);
//
//		// draw call
//		// TODO: JUST CALL GLDRAWELEMENTS HERE, ALL HARD JOB IS DONE
//		glDrawElements(GL_TRIANGLES, mesh->elemCount, GL_UNSIGNED_SHORT, (const void*)mesh->idxBegin);
//		//SDL_assert(glGetError() == GL_NO_ERROR);
//	}
//}