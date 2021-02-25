#ifndef __HELPER_H__
#define __HELPER_H__

#include <SDL.h>

namespace Helper {
	char* readFileContent(const char* filename, size_t *filesize = NULL);
}


#endif