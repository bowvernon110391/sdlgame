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
    static Mesh* createBox(float w) {
        if (w <= 0.f)
            w = 1.f;

        // now we create
        float half = .5f * w;

        // create cube with colors
        // 8 vertices
        Mesh *m = new Mesh();

        m->vertexFormat = VF_XYZ | VF_COLOR;
        m->strideLength = 24;

        // create 8 vertices, front set first, then back
        // 
        float verts[48] = {
            // xyz rgb
            -half, -half, half, 0, 0, 1,
             half, -half, half, 1, 0, 1,
             half,  half, half, 1, 1, 1,
            -half,  half, half, 0, 1, 1,

             half, -half,-half, 1, 0, 0,
            -half, -half,-half, 0, 0, 0,
            -half,  half,-half, 0, 1, 0,
             half,  half,-half, 1, 1, 0
        };
        
        std::vector<float> &v = m->vertBuffer;
        v.insert(v.end(), verts, verts+48);

        // index em
        uint16_t indices[36] = {
            0, 1, 2, 0, 2, 3,   // front
            1, 4, 7, 1, 7, 2,   // right
            4, 5, 6, 4, 6, 7,   // back
            5, 0, 3, 5, 3, 6,   // left
            3, 2, 6, 6, 2, 7,   // top
            5, 4, 0, 0, 4, 1    // bottom
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