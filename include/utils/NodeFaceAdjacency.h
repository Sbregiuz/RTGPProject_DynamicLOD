#pragma once 

#include "utils/mesh.h"
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <glm/glm.hpp>

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
        Node *nodes;
        int num_nodes;

        NodeFaceAdjacency(int nodesSize){
            num_nodes = nodesSize;
            nodes = new Node[nodesSize];
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
