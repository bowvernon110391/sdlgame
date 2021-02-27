#include "Texture2d.h"
#include "Helper.h"

#define	STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

Texture2D* Texture2D::loadFromFile(const char* filename,
	GLint minFilter, GLint magFilter,
	GLint wrapS, GLint wrapT) {
	size_t bufSize;
	const char* buf = Helper::readFileContent(filename, &bufSize);

	Texture2D* tex = loadFromMemory(buf, bufSize, minFilter, magFilter, wrapS, wrapT);
	if (tex) {
		delete[] buf;
	}

	return tex;
}

Texture2D* Texture2D::loadFromMemory(const char* buf, int bufSize,
	GLint minFilter, GLint magFilter,
	GLint wrapS, GLint wrapT) {
	// first, check the buffer?
	if (buf && bufSize) {
		// init libspng context
		bool failed = false;
		unsigned char* decodedImage = NULL;
		int w, h, bpp;

		// decode using stb, vertically flipped for opengl
		stbi_set_flip_vertically_on_load(1);
		decodedImage = stbi_load_from_memory((const unsigned char*)buf, bufSize, &w, &h, &bpp, 0);
		failed = decodedImage == NULL;

		// if we failed, clean up
		if (failed) {
			if (decodedImage)
				stbi_image_free(decodedImage);
			return NULL;
		}
		else {
			// we spawn the real shit
			Texture2D* tex = new Texture2D();

			tex->width = w;
			tex->height = h;
			tex->texData = decodedImage;
			tex->format = bpp == 4 ? GL_RGBA : GL_RGB;

			if (tex->upload()) {
				stbi_image_free(decodedImage);
				if (!tex->ownPixelData || !tex->retainPixelData) {
					// zero out
					tex->texData = NULL;
				}
				return tex;
			}

			// we failed
			delete tex;
			return NULL;
		}
	}
	return NULL;
}
