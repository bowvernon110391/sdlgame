#include "Game.h"
#include "Shader.h"
#include "ShaderData.h"
#include "Texture2d.h"
#include "Mesh.h"
#include "Material.h"
#include "LargeMesh.h"
#include "ShaderSource.h"

Shader* Game::loadShader(const char* name) {
	std::string vsFilename = std::string(name) + ".vert";
	std::string fsFilename = std::string(name) + ".frag";
	
	return Shader::fromFile(vsFilename.c_str(), fsFilename.c_str());
}

Texture2D* Game::loadTexture(const char* name) {
	// load the texture, do not set anything yet?
	return Texture2D::loadFromFile(name, true)->withFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
}

Mesh* Game::loadMesh(const char* name) {
	return Mesh::loadBCFFromFile(name)->createBufferObjects();
}

ShaderData* Game::loadBasicShaderData(const char* name) {
	// just spawn a basic material
	return new ShaderData();
}

Material* Game::loadBasicMaterial(const char* name) {
	// just spawn empty material
	return new Material();
}

MaterialSet* Game::loadMaterialSet(const char* name) {
	// just return empty matset
	return new MaterialSet();
}

LargeMesh* Game::loadLargeMesh(const char* name)
{
	return LargeMesh::loadLMFFromFile(name)->createBufferObjects();
}

ShaderSource* Game::loadShaderSource(const char* name)
{
	char* buf = Helper::readFileContent(name);

	ShaderSource* src = new ShaderSource(buf);

	delete[] buf;
	return src;
}
