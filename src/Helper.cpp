#include "Helper.h"

char *readFileContent(const char* filename, size_t *outFilesize) {
    SDL_RWops *io = SDL_RWFromFile(filename, "rb");
    if (!io) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed opening '%s' : %s", filename, SDL_GetError());
        return NULL;
    }

    int64_t filesize = SDL_RWsize(io);
    char *bytes = new char[filesize];

    if (!bytes) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed allocating %u bytes of memory when reading file '%s'", 
            filesize, filename);
        return NULL;
    }

    // gotta keep reading until we got all we need
    int64_t total_read = 0, read_ret = 1;

    while (total_read < filesize && read_ret != 0) {
        read_ret = SDL_RWread(io, &bytes[total_read], 1, (filesize-total_read));
        total_read += read_ret;
    }

    if (total_read != filesize) {
        SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "[%s] : Try to read %d bytes, only got %d. Bailing...", filename, filesize, total_read);
        delete [] bytes;
        return NULL;
    }

    if (outFilesize) {
        *outFilesize = filesize;
    }

    return bytes;
}