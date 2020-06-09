#ifndef __MUH_SHADER_H__
#define __MUH_SHADER_H__

#define GL_GLEXT_PROTOTYPES 1

#include <SDL_opengles2.h>

#include <vector>

#include "Helper.h"

#define ATTRIB_POS_LOC          0
#define ATTRIB_COL_LOC          1
#define ATTRIB_NORMAL_LOC       2
#define ATTRIB_UV_LOC           3
#define ATTRIB_UV2_LOC          4
#define ATTRIB_TANGENT_LOC      5
#define ATTRIB_BITANGENT_LOC    6

class Shader
{
protected:
    GLint programId;

    std::vector<int> uniformLoc;
public:
    Shader(int progId = 0): programId(progId) {
    }

    virtual ~Shader() {
        if (programId)
            glDeleteProgram(programId);
    }

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
            return 0;
        }

        // create shader
        GLuint shd = glCreateShader(shaderType);
        if (shd == 0) {
            return 0;
        }

        // load source
        glShaderSource(shd, 1, &src, &srcLen);

        // compile
        glCompileShader(shd);

        // check compile status
        GLint compiled;
        glGetShaderiv(shd, GL_COMPILE_STATUS, &compiled);

        if (!compiled) {
            GLint infoLen = 0;

            glGetShaderiv(shd, GL_INFO_LOG_LENGTH, &infoLen);

            if (infoLen > 1) {
                char* infoLog = new char[infoLen];

                glGetShaderInfoLog(shd, infoLen, NULL, infoLog);

                SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Error compiling shader: %s", infoLog);

                delete [] infoLog;
            }

            // failed, delete shader
            glDeleteShader(shd);
            return 0;
        }

        return shd;
    }

    // compile shader from source inputs
    static bool compileShader(const char* vs, const char* fs, int vsLen, int fsLen, int *shaderId) {
        if (!shaderId) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Compiling shader without program Id (iot)");
            return false;
        }

        int vsId = compileShader(GL_VERTEX_SHADER, vs, vsLen);

        if (!vsId) {
            return false;
        }

        int fsId = compileShader(GL_FRAGMENT_SHADER, fs, fsLen);

        if (!fsId) {
            return false;
        }

        // create program object nao
        auto progId = glCreateProgram();

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
                SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Program linking failed: %s", infoLog);

                delete [] infoLog;
            }

            glDeleteProgram(progId);
            return false;
        }

        *shaderId = progId;
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
            return NULL;
        }

        // success. make shader
        return new Shader(progId);
    }

    // create shader based on filename?
    static Shader *loadShaderFromFile(const char* vsFilename, const char* fsFilename) {
        size_t vsLen;
        char *vs = readFileContent(vsFilename, &vsLen);

        size_t fsLen;
        char *fs = readFileContent(fsFilename, &fsLen);

        return loadShaderFromSources(vs, (int)vsLen, fs, (int)fsLen);
    }
};


#endif