#pragma once
#include "Texture2d.h"
#include "Resource.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Shader;
class RenderPass;
/// <summary>
/// Base data for material (aka Shader data)
/// </summary>
class ShaderData : public Resource {
public:
	ShaderData() {
		// set default value?
		diffuseColor = glm::vec4(1, 1, 1, 1);
		specularColor = glm::vec4(1, 1, 1, 1);
		emissionColor = glm::vec4(0, 0, 0, 0);
		glossiness = 0.5f;	// half glossy
		fresnel0 = 0.05;	// common dielectric?
	}

	ShaderData* fillTextureSlot(int slot, Texture2D* t) {
		if (slot >= texture.size()) {
			texture.resize(slot + 1);
		}
		texture[slot] = t;
		return this;
	}

	ShaderData* setDiffuse(const glm::vec4& d) {
		diffuseColor = d;
		return this;
	}
	
	ShaderData* setSpecular(const glm::vec4& s) {
		specularColor = s;
		return this;
	}
	
	ShaderData* setEmission(const glm::vec4& e) {
		emissionColor = e;
		return this;
	}

	ShaderData* setGlossiness(float a) {
		glossiness = a;
		return this;
	}

	ShaderData* setFresnel0(float f) {
		fresnel0 = f;
		return this;
	}

	Texture2D* getTexture(int slot) {
		if (slot >= texture.size())
			return 0;
		return texture[slot];
	}

	virtual const char* type() { return "SHADER_DATA"; }

	virtual void setupShader(Shader* s, RenderPass* r);
	
	std::vector<Texture2D*> texture;
	glm::vec4 diffuseColor;
	glm::vec4 specularColor;
	glm::vec4 emissionColor;
	float glossiness, fresnel0;
};