#ifndef __MESH_H__
#define __MESH_H__

#include <cstdint>
#include <vector>

#define DEBUG_BCF   1

#define VF_XYZ      (1<<0)
#define VF_NORMAL   (1<<1)
#define VF_UV       (1<<2)
#define VF_TANGENT  (1<<3)
#define VF_UV2      (1<<4)
#define VF_COLOR    (1<<5)

class Mesh
{
public:

    typedef struct SubMesh 
    {
        // this contains the indices begin-end only
        uint16_t idxBegin, elemCount;
        char materialName[32];
    } SubMesh_t;
    
    // raw buffer
    size_t vertexBufferSize;
    void* vertexBuffer;
    size_t indexBufferSize;
    unsigned short* indexBuffer;

    //std::vector<float> vertBuffer; // float buffer containing our mesh vertices data
    //std::vector<uint16_t> idxBuffer;
    std::vector<SubMesh_t> subMeshes;

    uint32_t vertexFormat;
    uint16_t strideLength;  // vertex size per bytes

	uint32_t vbo;
	uint32_t ibo;

    char name[32];

    Mesh():
	strideLength(12), vertexFormat(VF_XYZ), vbo(0), ibo(0), vertexBuffer(0), indexBuffer(0)
	{
    }

    ~Mesh();

	// instantiate buffer
	bool createBufferObjects();

	// bind buffers
	void use();

    // helper shiet
    static Mesh* createTexturedQuad(float w) {
        if (w <= 0.f)
            w = 1.f;

        const float half = .5f * w;

        Mesh* m = new Mesh();

        m->vertexFormat = VF_XYZ | VF_COLOR | VF_UV;
        m->strideLength = sizeof(float) * 8;

        // create 4 vertices
        float verts[] = {
            -half, -half, 0,    1, 0, 0,    0, 0,
            half, -half, 0,     0, 1, 0,    1, 0,
            half, half, 0,      0, 0, 1,    1, 1,
            -half, half, 0,     1, 1, 0,   0, 1
        };

        /*std::vector<float>& v = m->vertBuffer;
        v.insert(v.end(), verts, verts + 32);*/
        m->vertexBuffer = new char[sizeof(verts)];
        m->vertexBufferSize = sizeof(verts);
        memcpy(m->vertexBuffer, verts, sizeof(verts));

        // the indices is simple
        uint16_t indices[] = {
            0, 1, 2,
            0, 2, 3
        };
        
        /*std::vector<uint16_t>& idx = m->idxBuffer;
        idx.insert(idx.end(), indices, indices + 6);*/
        int idxCount = sizeof(indices) / sizeof(indices[0]);
        m->indexBuffer = new unsigned short[idxCount];
        m->indexBufferSize = sizeof(indices);

        SubMesh s;
        strcpy(s.materialName, "QUAD");
        s.idxBegin = 0;
        s.elemCount = idxCount;

        m->subMeshes.push_back(s);

        return m;
    }

    static Mesh* createUnitBox() {
        float w = 1.f;

        // now we create
        float half = .5f * w;

        // create cube with colors
        // 8 vertices
        Mesh *m = new Mesh();

        m->vertexFormat = VF_XYZ | VF_NORMAL | VF_UV;
        m->strideLength = 32;

        // create 24 vertices, front set first, then back
        // 
        float verts[] = {
            // xyz uv
            // front
            -half, -half, half, 0, 0, 1,  0, 0,
             half, -half, half, 0, 0, 1,  1, 0,
             half,  half, half, 0, 0, 1,  1, 1,
            -half,  half, half, 0, 0, 1,  0, 1,

            // back
             half, -half,-half, 0, 0,-1,  0, 0,
            -half, -half,-half, 0, 0,-1,  1, 0,
            -half,  half,-half, 0, 0,-1,  1, 1,
             half,  half,-half, 0, 0,-1,  0, 1,

             // right side
             half, -half, half, 1, 0, 0,  0, 0,
             half, -half, -half, 1, 0, 0,  1, 0,
             half, half, -half, 1, 0, 0,  1, 1,
             half, half, half, 1, 0, 0,  0, 1,

             // left side
             -half, -half, -half, -1, 0, 0,  0, 0,
             -half, -half,  half, -1, 0, 0,  1, 0,
             -half,  half,  half, -1, 0, 0,  1, 1,
             -half,  half, -half, -1, 0, 0,  0, 1,
             
             // top side
             -half,  half,  half, 0, 1, 0,  0, 0,
              half,  half,  half, 0, 1, 0,  1, 0,
              half,  half, -half, 0, 1, 0,  1, 1,
             -half,  half, -half, 0, 1, 0,  0, 1,
             
             // bottom side
             -half, -half, -half, 0, -1, 0,  0, 0,
              half, -half, -half, 0, -1, 0,  1, 0,
              half, -half,  half, 0, -1, 0,  1, 1,
             -half, -half,  half, 0, -1, 0,  0, 1,
        };
        
        /*std::vector<float> &v = m->vertBuffer;
        v.insert(v.end(), verts, verts+(sizeof(verts)/sizeof(float)));*/
        m->vertexBuffer = new char[sizeof(verts)];
        m->vertexBufferSize = sizeof(verts);
        memcpy(m->vertexBuffer, verts, sizeof(verts));

        // index em
        uint16_t indices[36] = {
            0, 1, 2, 0, 2, 3,   // front
            4, 5, 6, 4, 6, 7,   // back
            8, 9, 10, 8, 10, 11,   // right
            12, 13, 14, 12, 14, 15,   // left
            16, 17, 18, 16, 18, 19,   // top            
            20, 21, 22, 20, 22, 23   // bottom
        };

        /*std::vector<uint16_t> &idx = m->idxBuffer;
        idx.insert(idx.end(), indices, indices+36);*/
        int idxCount = sizeof(indices) / sizeof(indices[0]);
        m->indexBufferSize = sizeof(indices);
        m->indexBuffer = new unsigned short[idxCount];
        memcpy(m->indexBuffer, indices, m->indexBufferSize);

        // append submeshes info
        SubMesh_t s;

        s.idxBegin = 0;
        s.elemCount = idxCount;
        strcpy(s.materialName, "BOX");

        m->subMeshes.push_back(s);

        return m;
    }

    /*
    * 
    # write_to_binary, write binary file
    # FORMAT IS AS FOLLOWS:
    # 1b : vtx_format
    # 1b : bytes_per_vertex
    # 2b : vertex_count (max 65535 vertex)
    # 4b : vertex_buffer_size_in_bytes
    # 2b : sub_mesh_count
    # 32b: objname
    # 2b : total_tris
    # --SUB_MESH_DATA_---
    # { 32b: material_name, 2b: begin_at, 2b: total_tri }
    # { vertex_buffer }
    # { index_buffer }
    */
    static Mesh* loadBCFfromMemory(const void* mem, int bufSize) {
        char* ptr = (char*) mem;
        // first, read vertex format
        Mesh* m = new Mesh();

        // 1b: vtx_format
        m->vertexFormat = *(unsigned char*)ptr;
        ++ptr;

        // 1b: bytes_per_vertex
        m->strideLength = *(unsigned char*)ptr;
        ++ptr;

        // 2b: vertex count
        unsigned short vert_count = *(unsigned short*)ptr;
        ptr += 2;

        // 4b: vbuffer size
        unsigned int vb_size = *(unsigned int*)ptr;
        ptr += 4;


    }
};

#endif