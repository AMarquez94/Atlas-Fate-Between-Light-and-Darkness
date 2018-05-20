#ifndef INC_RENDER_VERTEX_DECLARATIONS_H_
#define INC_RENDER_VERTEX_DECLARATIONS_H_

// -----------------------------------------------------
class CVertexDecl {

public:

    D3D11_INPUT_ELEMENT_DESC * cpu_layout = nullptr;
    UINT numElements = 0;
    UINT bytes_per_vertex = 0;
    std::string name;
    bool instancing = false;
};

// -----------------------------------------------------
class CVertexDeclManager {

    std::map< std::string, CVertexDecl* > decls;

    CVertexDecl* createNew(
        const std::string& name
        , D3D11_INPUT_ELEMENT_DESC* layout
        , UINT numElements
    );

    const CVertexDecl* createInstancedVertexDecl(const std::string& name);

public:

    static CVertexDeclManager& get();

    bool create();
    const CVertexDecl* getByName(const std::string& name);
};


#endif

