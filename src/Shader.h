#ifndef __MUH_SHADER_H__
#define __MUH_SHADER_H__

#include <glad/glad.h>
#include <vector>
#include "Helper.h"

#define ATTRIB_POS_LOC          0
#define ATTRIB_COL_LOC          1
#define ATTRIB_NORMAL_LOC       2
#define ATTRIB_UV_LOC           3
#define ATTRIB_UV2_LOC          4
#define ATTRIB_TANGENT_LOC      5
#define ATTRIB_BITANGENT_LOC    6

#define	ATTRIB_CUSTOM			7

class Shader
{
protected:
    GLint programId;

    std::vector<int> uniformLoc;
public:

    Shader(int progId = 0) : programId(progId) {
    }

    virtual ~Shader() {
        if (programId)
            glDeleteProgram(programId);
    }

    // state preparation
    virtual void setUniformLocs() {}
    virtual void prepareState() {}

    // bind program
    void use() {
        glUseProgram(programId);
    }

    // get attribute location by name
    int getAttribLocation(const char* name) {
        use();
        return glGetAttribLocation(programId, name);
    }

    // get uniform location by name
    int getUniformLocation(const char* name) {
        use();
        return glGetUniformLocation(programId, name);
    }

    // push uniform location to specific index
    void pushUniformLocation(const char* name, int id) {
        // first, gotta check if i <= size, which means we need to resize
        if (uniformLoc.size() <= id) {
            uniformLoc.resize(id + 1);
        }

        // just overwrite whatever's in there
        uniformLoc[id] = getUniformLocation(name);
    }

    int getUniformLocation(int arrayIdx) {
        SDL_assert(uniformLoc.size() > arrayIdx);

        return uniformLoc[arrayIdx];
    }

    // compile one type of shader
    static GLuint compileShader(GLenum shaderType, const char* src, int srcLen) {
        if (!src || srcLen <= 0) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Invalid source");
            return 0;
        }

        // create shader
#ifdef _DEBUG
		SDL_Log("Generating shader id...");
#endif

        GLuint shd = glCreateShader(shaderType);
        if (shd == 0) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot generate shader %d", shaderType);
            return 0;
        }

#ifdef _DEBUG
		SDL_Log("Generated shader id: %u", shd);
#endif

        // load source
#ifdef _DEBUG
		SDL_Log("loading source...");
#endif
        glShaderSource(shd, 1, &src, &srcLen);

        // compile
#ifdef _DEBUG
		SDL_Log("Compiling shader...");
#endif
        glCompileShader(shd);

        // check compile status
#ifdef _DEBUG
		SDL_Log("Checking compile status...");
#endif
        GLint compiled;
        glGetShaderiv(shd, GL_COMPILE_STATUS, &compiled);

        if (!compiled) {
            GLint infoLen = 0;

            glGetShaderiv(shd, GL_INFO_LOG_LENGTH, &infoLen);

            if (infoLen > 1) {
                char* infoLog = new char[infoLen];

                glGetShaderInfoLog(shd, infoLen, NULL, infoLog);

                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error compiling shader: %s", infoLog);

                delete [] infoLog;
            }

            // failed, delete shader
            glDeleteShader(shd);

			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed compiling shader! unknow reason: %d %d", compiled, infoLen);
            return 0;
        }

#ifdef _DEBUG
		SDL_Log("Final shader id: %u", shd);
#endif

        return shd;
    }

    // compile shader from source inputs
    static bool compileShader(const char* vs, const char* fs, int vsLen, int fsLen, int *shaderId) {
        if (!shaderId) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "(!) Compiling shader without program Id (iot)");
            return false;
        }

        int vsId = compileShader(GL_VERTEX_SHADER, vs, vsLen);

        if (!vsId) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "(!) Failed Compiling Vertex Shader!");
            return false;
        }

        int fsId = compileShader(GL_FRAGMENT_SHADER, fs, fsLen);

        if (!fsId) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "(!) Failed Compiling Fragment Shader!");
            return false;
        }

        // create program object nao
        GLuint progId = glCreateProgram();

        if (!progId) {
            return false;
        }

        glAttachShader(progId, vsId);
        glAttachShader(progId, fsId);

        // set common attrib here
        setCommonAttribLocation(progId);

        // link it?
        glLinkProgram(progId);

        // check link status
        GLint linked;

        glGetProgramiv(progId, GL_LINK_STATUS, &linked);

        if (!linked) {
            GLint infoLen = 0;
            
            glGetProgramiv(progId, GL_INFO_LOG_LENGTH, &infoLen);

            if (infoLen > 1) {
                char *infoLog = new char[infoLen];

                glGetProgramInfoLog(progId, infoLen, NULL, infoLog);
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "(!) Program linking failed: %s", infoLog);

                delete [] infoLog;
            }

            glDeleteProgram(progId);
            return false;
        }

        *shaderId = progId;

		// delete shaders
		glDeleteShader(vsId);
		glDeleteShader(fsId);

        return true;
    }

    // set common attribute that we use
    static void setCommonAttribLocation(GLint progId) {
        if (!progId)
            return;

        glBindAttribLocation(progId, ATTRIB_POS_LOC, "position");
        glBindAttribLocation(progId, ATTRIB_COL_LOC, "color");
        glBindAttribLocation(progId, ATTRIB_NORMAL_LOC, "normal");
        glBindAttribLocation(progId, ATTRIB_UV_LOC, "uv");
        glBindAttribLocation(progId, ATTRIB_UV2_LOC, "uv2");
        glBindAttribLocation(progId, ATTRIB_TANGENT_LOC, "tangent");
        glBindAttribLocation(progId, ATTRIB_BITANGENT_LOC, "bitangent");
    }

    // create shader based on source?
    static Shader *loadShaderFromSources(const char* vs, const int vsLen, const char* fs, const int fsLen) {
        int progId = 0;
        if(!compileShader(vs, fs, vsLen, fsLen, &progId)) {
			SDL_Log("Failed compiling shader. Reason? Dunno");
            return NULL;
        }

        // success. make shader
        return new Shader(progId);
    }

    // load from file
    bool loadFromFile(const char* vsFilename, const char* fsFilename) {
        using namespace Helper;

        size_t vsLen;
        char* vs = readFileContent(vsFilename, &vsLen);

        if (!vs) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error loading vertex shader source: %s", vsFilename);
            return false;
        }
        // realloc
        vs = (char*)realloc(vs, vsLen + 1);
        vs[vsLen++] = 0;

        size_t fsLen;
        char* fs = readFileContent(fsFilename, &fsLen);

        if (!fs) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error loading fragment shader source: %s", fsFilename);
            delete[] vs;

            return false;
        }
        // realloc
        fs = (char*)realloc(fs, fsLen + 1);
        fs[fsLen++] = 0;

        // compile
        SDL_Log("Compiling... %s - %s", vsFilename, fsFilename);
        bool result = Shader::compileShader(vs, fs, vsLen, fsLen, &this->programId);

        if (result) {
            SDL_Log("Compiled: %s - %s", vsFilename, fsFilename);
        }

        //cleanup
        delete[] vs;
        delete[] fs;
        return result;
    }

    // create shader based on filename?
    static Shader *loadShaderFromFile(const char* vsFilename, const char* fsFilename) {
        using namespace Helper;

        size_t vsLen;
        char *vs = readFileContent(vsFilename, &vsLen);

		if (!vs) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error loading vertex shader source: %s", vsFilename);
			return NULL;
		}

		char *vsCorrected = new char[vsLen + 1];
		memcpy(vsCorrected, vs, vsLen);
		vsCorrected[vsLen] = 0;

		delete [] vs;

        size_t fsLen;
        char *fs = readFileContent(fsFilename, &fsLen);

		if (!fs) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error loading fragment shader source: %s", fsFilename);
			delete [] vs;

			return NULL;
		}

		char *fsCorrected = new char[fsLen + 1];
		memcpy(fsCorrected, fs, fsLen);
		fsCorrected[fsLen] = 0;

		delete [] fs;

#ifdef DUMP_SHADER
		SDL_Log(vsCorrected);
		SDL_Log(fsCorrected);
#endif

        Shader *result = loadShaderFromSources(vsCorrected, (int)vsLen + 1, fsCorrected, (int)fsLen + 1);

		delete [] vsCorrected;
		delete [] fsCorrected;

		return result;
    }
};


#endif