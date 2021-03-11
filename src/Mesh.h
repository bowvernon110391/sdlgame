#ifndef __MESH_H__
#define __MESH_H__

#include <cstdint>
#include <vector>

#define VF_XYZ      1
#define VF_UV       (1<<1)
#define VF_UV2      (1<<2)
#define VF_NORMAL   (1<<3)
#define VF_COLOR    (1<<4)

class Mesh
{
public:

    typedef struct SubMesh 
    {
        // this contains the indices begin-end only
        uint16_t idxBegin, elemCount;
    } SubMesh_t;
    


    std::vector<float> vertBuffer; // float buffer containing our mesh vertices data
    std::vector<uint16_t> idxBuffer;
    std::vector<SubMesh_t> subMeshes;

    uint32_t vertexFormat;
    uint16_t strideLength;

	uint32_t vbo;
	uint32_t ibo;

    Mesh():
	strideLength(12), vertexFormat(VF_XYZ), vbo(0), ibo(0)
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

        std::vector<float>& v = m->vertBuffer;
        v.insert(v.end(), verts, verts + 32);

        // the indices is simple
        uint16_t indices[] = {
            0, 1, 2,
            0, 2, 3
        };
        
        std::vector<uint16_t>& idx = m->idxBuffer;
        idx.insert(idx.end(), indices, indices + 6);

        SubMesh s;
        s.idxBegin = 0;
        s.elemCount = idx.size();

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
        
        std::vector<float> &v = m->vertBuffer;
        v.insert(v.end(), verts, verts+(sizeof(verts)/sizeof(float)));

        // index em
        uint16_t indices[36] = {
            0, 1, 2, 0, 2, 3,   // front
            4, 5, 6, 4, 6, 7,   // back
            8, 9, 10, 8, 10, 11,   // right
            12, 13, 14, 12, 14, 15,   // left
            16, 17, 18, 16, 18, 19,   // top            
            20, 21, 22, 20, 22, 23   // bottom
        };

        std::vector<uint16_t> &idx = m->idxBuffer;
        idx.insert(idx.end(), indices, indices+36);

        // append submeshes info
        SubMesh_t s;

        s.idxBegin = 0;
        s.elemCount = 36;

        m->subMeshes.push_back(s);

        return m;
    }
};

#endif