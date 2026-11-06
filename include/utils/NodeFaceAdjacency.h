#pragma once 

#include <new>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "utils/mesh.h"

using namespace std;

struct Node;

struct Node{
    int id;
    vector<int> adjacencyFaces;
    vector<int> adjacentVertices;
    bool active = true;

    Node(){
    }
};

class NodeFaceAdjacency{
    public:
        int num_nodes;
        Node *nodes;

        NodeFaceAdjacency(int nodesSize){
            num_nodes = nodesSize;
            nodes = new Node[nodesSize];
        }

        NodeFaceAdjacency(Mesh *m){
            num_nodes = m->vertices.size();
            nodes = new Node[num_nodes];

            for(int i = 0; i < int(m->vertices.size()); i++)
                addNode(i);

            for(Face f : m->faces)
                addFace(&f);
        }
    
        // RULE OF 5
        void operator~(){
            delete[] nodes;
        }

        NodeFaceAdjacency(const NodeFaceAdjacency&) = delete;
        NodeFaceAdjacency& operator=(const NodeFaceAdjacency&) = delete;
        NodeFaceAdjacency(NodeFaceAdjacency &&other) noexcept:num_nodes(other.num_nodes), nodes(other.nodes){
            other.nodes = nullptr;
            other.num_nodes = 0;

        } 

        NodeFaceAdjacency& operator=(NodeFaceAdjacency& other)noexcept{
            if (this != &other){
                delete[] nodes;
                nodes = other.nodes;
                num_nodes = other.num_nodes;
                other.nodes = nullptr;
                other.num_nodes = 0;
            }
            return *this;
        }

        // FIRST: ALl nodes gets added to the nodes array (each node is located in it's ID position)
        void addNode(int n){
            Node nd = Node();
            nd.id = n;
            nodes[n] = nd;
        }

        bool contains(int vID, int vNear){
            if(nodes[vID].adjacentVertices.empty())
                return false;

            for(int i = 0; i< int(nodes[vID].adjacentVertices.size()); i++){
                if(nodes[vID].adjacentVertices[i] == vNear)
                    return true;
            }

            return false;
        }

        //SECOND: Each face is get and processed. This means:
        //          Each vertex adjacent to the face adds it to it's adjacencyFaces
        //          Each vertex adds the other two to it's adjacentVertices
        void addFace(const Face *f){
            int v1 = f->vertices[0];
            int v2 = f->vertices[1];
            int v3 = f->vertices[2]; 

            nodes[v1].adjacencyFaces.push_back(f->index);
            nodes[v2].adjacencyFaces.push_back(f->index);
            nodes[v3].adjacencyFaces.push_back(f->index);

            addAdjacentNode(v1, v2);
            addAdjacentNode(v1, v3);

            addAdjacentNode(v2, v1);
            addAdjacentNode(v2, v3);

            addAdjacentNode(v3, v1);
            addAdjacentNode(v3, v2);
        }
        
        void addAdjacentNode(int vID, int vNear){
            if(!contains(vID, vNear))
                nodes[vID].adjacentVertices.push_back(vNear);
        }
    
        string to_string(){
            std::ostringstream outs;
            for (int i = 0; i < num_nodes; i++){
                outs << "Node ID " << i;
                outs << " ----> [";
                for (int j = 0; j < int(nodes[i].adjacencyFaces.size()); j++){
                    outs << nodes[i].adjacencyFaces[j]<< ", ";
                }

                outs << "]" << endl;
            }
            return outs.str();
        }


};


/*
    Mesh processMesh(aiMesh* mesh, bool pre_process)
    {
        NodeFaceAdjacency g(int(mesh->mNumVertices));
        // data structures for vertices and indices of vertices (for faces)
        vector<Vertex> vertices;
        vector<GLuint> indices;
        vector<Face> faces;

        glm::vec3 vector;
        glm::vec2 vec;

        Vertex vertex;
        Face face_mesh;

        GLuint i,j;

        for(i = 0; i < mesh->mNumVertices; i++)
        {
            
            // the vector data type used by Assimp is different than the GLM vector needed to allocate the OpenGL buffers
            // I need to convert the data structures (from Assimp to GLM, which are fully compatible to the OpenGL)
            
            // vertices coordinates
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;

            g.addNode(i);   //Adding the node the the tmp data structure

            // Normals
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
            // Texture Coordinates
            // if the model has texture coordinates, than we assign them to a GLM data structure, otherwise we set them at 0
            // if texture coordinates are present, than Assimp can calculate tangents and bitangents, otherwise we set them at 0 too
            if(mesh->mTextureCoords[0])
            {
                // in this example we assume the model has only one set of texture coordinates. Actually, a vertex can have up to 8 different texture coordinates. For other models and formats, this code needs to be adapted and modified.
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;

                // Tangents
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;
                // Bitangents
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            }
            else{
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
                cout << "WARNING::ASSIMP:: MODEL WITHOUT UV COORDINATES -> TANGENT AND BITANGENT ARE = 0" << endl;
            }
            // we add the vertex to the list
            vertices.emplace_back(vertex);
        }

        //cout << "This Mesh Has " << mesh -> mNumFaces<< "Faces" << endl;
        // for each face of the mesh, we retrieve the indices of its vertices , and we store them in a vector data structure
        for(i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace* face = &mesh->mFaces[i];

            face_mesh = Face();
            face_mesh.index = i;

            for(j = 0; j < face->mNumIndices; j++){
                int vertex_id = face->mIndices[j];
                face_mesh.vertices[j] = vertex_id;
                indices.emplace_back(vertex_id);
            }
            g.addFace(face_mesh);
            faces.emplace_back(face_mesh);
        }
        //cout << g.to_string() << endl; 
        // we return an instance of the Mesh class created using the vertices and faces data structures we have created above.
        Mesh m = Mesh(vertices, indices, faces, pre_process);
        if(pre_process)
            MeshSimplification(&m, &g);

        m.initialize();
        return m;
    } */
