#include "mesh.h"

Mesh gen_mesh_from_tiles(std::vector<Tile> tiles)
{
    std::vector<Vertex> vertices;
 
    for (auto tile : tiles)
    {
        // TODO(jklmn): Do this using elements instead to reduce vertex count
        vertices.push_back(tile.vertices[0]);
        vertices.push_back(tile.vertices[1]);
        vertices.push_back(tile.vertices[2]);

        vertices.push_back(tile.vertices[0]);
        vertices.push_back(tile.vertices[2]);
        vertices.push_back(tile.vertices[3]);
    }

    return Mesh::fromVertexData(vertices);
}

static Mesh fromVertexData(const std::vector<Vertex> &vertices, const std::vector<u32> indices = {} )
    {
        Mesh mesh;

        mesh.numElements = !indices.empty() ? indices.size() : vertices.size();

        glGenVertexArrays(1, &mesh.VAO);
        glGenBuffers(1, &mesh.VBO);
        if (!indices.empty())
        {
            GLuint EBO;
            glGenBuffers(1, &EBO);
            mesh.EBO = EBO;
        }

        glBindVertexArray(mesh.VAO);

        glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

        if (!indices.empty())
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO.value());
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(u32), indices.data(), GL_STATIC_DRAW);
        }

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal)));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, texcoord)));

        glBindVertexArray(0);

        return mesh;
};

static std::optional<std::vector<Mesh>> importMeshesFromFile(const std::string &path)
    {
        // Check if file exists
        std::ifstream fin(path);


        if (fin.fail()) 
        {
            Log::error("Failed to open file %s while importing mesh... Does it exist?", path.c_str());
            return std::nullopt;
        }

        fin.close();

        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            Log::error("ERROR::ASSIMP::%s", importer.GetErrorString());
            return std::nullopt;
        }

        std::vector<Mesh> meshes;
        for (u32 i = 0; i < scene->mNumMeshes; i++)
        {
            std::vector<Vertex> vertices;
            std::vector<u32> indices;
            aiMesh* mesh = scene->mMeshes[i];

            for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                Vertex vertex;
                // process vertex positions, normals and texture coordinates
                vertex.pos = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
                vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

                if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
                    vertex.texcoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
                else
                    vertex.texcoord = glm::vec2(0.0f, 0.0f);

                vertices.push_back(vertex);
            }

            // process indices
            for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
                aiFace face = mesh->mFaces[i];
                for (unsigned int j = 0; j < face.mNumIndices; j++)
                    indices.push_back(face.mIndices[j]);
            }

            meshes.push_back(Mesh::fromVertexData(vertices, indices));
        }

        return meshes;
    }
};
