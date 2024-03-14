#include "mesh.h"

MeshData MeshData::genMeshFromTiles(std::vector<Tile> tiles)
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

    return MeshData { 
        vertices,
        {}
    };
}

// static std::optional<std::vector<MeshData>> importMeshesFromFile(const std::string &path)
//     {
//         // Check if file exists
//         std::ifstream fin(path);
// 
// 
//         if (fin.fail()) 
//         {
//             Log::error("Failed to open file %s while importing mesh... Does it exist?", path.c_str());
//             return std::nullopt;
//         }
// 
//         fin.close();
// 
//         Assimp::Importer importer;
// 
//         const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);
// 
//         if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
//             Log::error("ERROR::ASSIMP::%s", importer.GetErrorString());
//             return std::nullopt;
//         }
// 
//         std::vector<MeshData> meshes;
//         for (u32 i = 0; i < scene->mNumMeshes; i++)
//         {
//             std::vector<Vertex> vertices;
//             std::vector<u32> indices;
//             aiMesh* mesh = scene->mMeshes[i];
// 
//             for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
//                 Vertex vertex;
//                 // process vertex positions, normals and texture coordinates
//                 vertex.pos = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
//                 vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
// 
//                 if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
//                     vertex.texcoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
//                 else
//                     vertex.texcoord = glm::vec2(0.0f, 0.0f);
// 
//                 vertices.push_back(vertex);
//             }
// 
//             // process indices
//             for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
//                 aiFace face = mesh->mFaces[i];
//                 for (unsigned int j = 0; j < face.mNumIndices; j++)
//                     indices.push_back(face.mIndices[j]);
//             }
// 
//             meshes.push_back(Mesh::fromVertexData(vertices, indices));
//         }
// 
//         return meshes;
//     }
// };
