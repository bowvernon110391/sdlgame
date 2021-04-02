#pragma once

#include <vector>

class Shader;
class ShaderData;
/// <summary>
/// just a holder for a shader + shader data
/// </summary>
class Material {
public:
	Material(Shader* s, ShaderData* sd) {
		sh = s;
		shData = sd;
	}

	Shader* sh;
	ShaderData* shData;
};

/// <summary>
/// just a holder of a list of Material
/// </summary>
class MaterialSet {
protected:
	MaterialSet() {}
public:
	~MaterialSet() {}

	std::vector<Material*> mats;

	static MaterialSet* create() {
		return new MaterialSet();
	}

	MaterialSet *addMaterial(Material* m) {
		mats.push_back(m);
		return this;
	}
};