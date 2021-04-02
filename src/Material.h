#pragma once

#include <vector>

class Shader;
class ShaderData;
template<typename Resource>
class ResourceManager;
/// <summary>
/// just a holder for a shader + shader data
/// </summary>
class Material {
public:
	Material() { sh = nullptr; shData = nullptr; }
	Material(Shader* s, ShaderData* sd) {
		sh = s;
		shData = sd;
	}

	Material* withShader(Shader* s) { sh = s; return this; }
	Material* withData(ShaderData* sd) { shData = sd; return this; }

	Shader* sh;
	ShaderData* shData;
};

/// <summary>
/// just a holder of a list of Material
/// </summary>
class MaterialSet {
	friend class ResourceManager<MaterialSet>;
protected:
public:
	MaterialSet() {}
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