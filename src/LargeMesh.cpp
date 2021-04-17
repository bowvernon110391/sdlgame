#include "LargeMesh.h"
#include "Helper.h"

LargeMesh::LargeMesh()
{
    mesh_count = node_count = 0;
    vertex_size = 0;
    vertex_format = 0;
    meshes.clear();
    nodes.clear();
}

LargeMesh::~LargeMesh()
{
    // clean up
    for (auto m : meshes)
        delete m;
    meshes.clear();
    nodes.clear();
}

LargeMesh* LargeMesh::loadLMFFromMemory(const char* buf, size_t buflen)
{
    return nullptr;
}

LargeMesh* LargeMesh::loadLMFFromFile(const char* filename)
{
    char* buf;
    size_t buflen;

    buf = Helper::readFileContent(filename, &buflen);

    LargeMesh* lm = loadLMFFromMemory(buf, buflen);

    delete[] buf;

    return lm;
}

char* LargeMesh::parseHeader(const char* buf)
{
    return nullptr;
}

char* LargeMesh::parseNode(const char* buf, KDTreeNode& n)
{
    return nullptr;
}

char* LargeMesh::parseMesh(const char* buf, Mesh* m)
{
    return nullptr;
}
