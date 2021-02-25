#ifndef __MUH_TEX2D__
#define __MUH_TEX2D__

#include <glad/glad.h>
#include <cstddef>

class Texture2D {
public:
	GLuint texId;
	int width, height;
	unsigned char* texData;
	GLint format, minFilter, magFilter, wrapS, wrapT;
	bool retainPixelData, ownPixelData;

	Texture2D() {
		retainPixelData = false;
		ownPixelData = false;

		texId = 0;
		width = height = 0;
		texData = NULL;
		format = GL_RGBA;
		minFilter = GL_LINEAR;
		magFilter = GL_LINEAR;
		wrapS = GL_CLAMP_TO_EDGE;
		wrapT = GL_CLAMP_TO_EDGE;
	}

	virtual ~Texture2D() {
		if (texData) delete[] texData;
		destroyHandle();
	}

	bool createHandle() {
		glGenTextures(1, &texId);
		if (!texId)
			return false;

		return true;
	}

	virtual void generateMipMap() {}

	bool upload(bool retainPixelData = false) {
		if (width && height) {
			if (!texId)
				if (!createHandle())
					return false;

			// generate rgba by default, don't ask don't tell?
			glBindTexture(GL_TEXTURE_2D, texId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);

			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, texData);

			generateMipMap();

			// free texture data
			if (!retainPixelData && ownPixelData) {
				delete[] texData;
				texData = NULL;
			}

			return true;
		}

		return false;
	}

	void use() {
		glBindTexture(GL_TEXTURE_2D, texId);
	}

	void destroyHandle() {
		if (texId) {
			glDeleteTextures(1, &texId);
		}
	}

	static Texture2D* loadFromFile(const char* filename,
		GLint minFilter = GL_TEXTURE_MIN_FILTER, GLint magFilter = GL_TEXTURE_MAG_FILTER,
		GLint wrapS = GL_CLAMP_TO_EDGE, GLint wrapT = GL_CLAMP_TO_EDGE);

	static Texture2D* loadFromMemory(const char* buf, int bufSize,
		GLint minFilter = GL_TEXTURE_MIN_FILTER, GLint magFilter = GL_TEXTURE_MAG_FILTER,
		GLint wrapS = GL_CLAMP_TO_EDGE, GLint wrapT = GL_CLAMP_TO_EDGE);
};

#endif