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
        void addFace(Face f){
            int v1 = f.vertices[0];
            int v2 = f.vertices[1];
            int v3 = f.vertices[2]; 

            nodes[v1].adjacencyFaces.push_back(f.index);
            nodes[v2].adjacencyFaces.push_back(f.index);
            nodes[v3].adjacencyFaces.push_back(f.index);

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
