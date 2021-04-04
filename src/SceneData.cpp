#include "SceneData.h"
#include "Shader.h"
#include "RenderPass.h"
#include <algorithm>

/// <summary>
/// temporary querypos to support lightCostFunction
/// </summary>
static glm::vec3 queryPos;
/// <summary>
/// compare the cost of a light, smaller cost = higher priority
/// </summary>
/// <param name="a"></param>
/// <param name="b"></param>
/// <returns></returns>
static bool lightCostFunction(const Light& a, const Light& b) {
	return a.distanceCost(queryPos) < b.distanceCost(queryPos);
}

void SceneData::setupData(const Shader* s, const RenderPass* rp)
{
	// set ambient color?
	int u_loc = SU_LOC(s, scene_ambient_color);
	if (u_loc >= 0) {
		glUniform4fv(u_loc, 1, glm::value_ptr(ambientColor));
	}

	// set sun direction
	u_loc = SU_LOC(s, sun_direction);
	if (u_loc >= 0) {
		glUniform3fv(u_loc, 1, glm::value_ptr(sunDirection));
	}

	// sun color (diffuse)
	u_loc = SU_LOC(s, sun_diffuse_color);
	if (u_loc >= 0) {
		glUniform4fv(u_loc, 1, glm::value_ptr(sunDiffuseColor));
	}

	// sun color (specular)
	u_loc = SU_LOC(s, sun_specular_color);
	if (u_loc >= 0) {
		glUniform4fv(u_loc, 1, glm::value_ptr(sunSpecularColor));
	}

	// sun intensity
	u_loc = SU_LOC(s, sun_intensity);
	if (u_loc >= 0) {
		glUniform4fv(u_loc, 1, glm::value_ptr(sunIntensity));
	}

	// point light's active count
	u_loc = SU_LOC(s, num_active_point_light);
	if (u_loc >= 0) {
		glUniform1i(u_loc, activePointLightCount);
	}

	// point light's diffuse
	u_loc = SU_LOC(s, light_diffuse_color);
	if (u_loc >= 0) {
		glUniform4fv(u_loc, 1, &lightDiffuseColor[0]);
	}

	// point light's specular
	u_loc = SU_LOC(s, light_specular_color);
	if (u_loc >= 0) {
		glUniform4fv(u_loc, 1, &lightSpecularColor[0]);
	}

	// point light's attenuation
	u_loc = SU_LOC(s, light_falloff);
	if (u_loc >= 0) {
		glUniform4fv(u_loc, 1, &lightFalloff[0]);
	}
}

/// <summary>
/// update our active lights data
/// </summary>
/// <param name="queryPos"></param>
void SceneData::updateLightsUniformData(const glm::vec3& pos)
{
	// update point lights?
	activePointLightCount = glm::min((int) pointLights.size(), MAX_POINT_LIGHTS);

	// sort
	queryPos = pos;
	std::sort(pointLights.begin(), pointLights.end(), lightCostFunction);

	// zero out data
	memset(&lightPosition, 0, sizeof(lightPosition));
	memset(&lightDiffuseColor, 0, sizeof(lightDiffuseColor));
	memset(&lightSpecularColor, 0, sizeof(lightSpecularColor));
	memset(&lightFalloff, 0, sizeof(lightFalloff));

	// now just set available data
	for (int i = 0; i < activePointLightCount; i++) {
		const Light& l = pointLights[i];

		memcpy(&lightPosition[i * 3], glm::value_ptr(l.position), sizeof(float) * 3);
		memcpy(&lightDiffuseColor[i * 4], glm::value_ptr(l.diffuseColor), sizeof(float) * 4);
		memcpy(&lightSpecularColor[i * 4], glm::value_ptr(l.specularColor), sizeof(float) * 4);
		memcpy(&lightFalloff[i * 4], glm::value_ptr(l.attenuation), sizeof(float) * 4);
	}
}

/// <summary>
/// get active point light data up to MAX_LIGHT
/// </summary>
/// <param name="MAX_LIGHT">how many were requested</param>
/// <param name="result">result to store to, padded</param>
/// <param name="pos">query position (usually camera)</param>
/// <return>number of active light</return>
int SceneData::getActivePointLightsData(int MAX_LIGHT, std::vector<Light*>& result, const glm::vec3& pos)
{
	int nLight = glm::min((int) this->pointLights.size(), MAX_LIGHT);

	// sort the light?
	queryPos = pos;
	std::sort(pointLights.begin(), pointLights.end(), lightCostFunction);

	// clear buffer
	result.clear();

	// for each light
	for (int i = 0; i < nLight; i++) {
		result.push_back(&pointLights[i]);
	}

	// pad the data
	for (int i = nLight; i < MAX_LIGHT; i++) {
		result.push_back(&nullLight);
	}

	return nLight;
}
