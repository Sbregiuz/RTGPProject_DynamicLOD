#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/geometric.hpp"
#include "glm/matrix.hpp"
#include <queue>
#include <utils/mesh.h>
#include <utils/NodeFaceAdjacency.h>
#include <vector>
#include <glm/glm.hpp>
#include <glfw/glfw3.h>
#include <glad/glad.h>

struct PriorityRecord;
struct FaceRecord;
struct CollapsedEdge;

bool compare_prio(PriorityRecord p1, PriorityRecord p2);
float compute_elision_cost(int vsrc,int vdst);
bool is_adj (Face f, int src, int dst);
void GetAdjFaces(Mesh *m, int src, vector<int> *adjFaces);
void GetAdjVertex (Mesh *m, int f, int src, int dst, CollapsedEdge *edge);
glm::vec4 faceNormal(Mesh *m, int face_id);
glm::mat4x4 fundamental_plane_q_mat(glm::vec4 plane);
void provaStampaListaDIMatrici(vector<glm::mat4x4> *Q);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> STRUCTS
struct PriorityRecord{
    int v_src;
    int v_dst;
    int elision_cost;
};

struct FaceRecord{
    int fIdx;
    int cornerIdx;

    FaceRecord(int f, int c){
        fIdx = f;
        cornerIdx = c;
    }
};

struct CollapsedEdge{
    int v_src;
    int v_dst;
    int vadj_left,vadj_right;
    glm::vec3 src_pos;
    vector<FaceRecord> faces_to_rebind;
};

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> FUNCTIONS
//
void MeshSimplification(Mesh *m, NodeFaceAdjacency *nfa){
    vector<glm::mat4> Q(m->vertices.size(), glm::mat4(0.0f));  
    
    const vector<Face> &faces = m->faces;
    // Extracting the face normal based plane equation for each of the faces
    for(int i = 0; i < int(faces.size()); i++){
        glm::vec4 face_plane_eq = faceNormal(m, i);
        glm::mat4 Kp = fundamental_plane_q_mat(face_plane_eq);    

        Q[faces[i].vertices[0]] += Kp;
        Q[faces[i].vertices[1]] += Kp;
        Q[faces[i].vertices[2]] += Kp;
    }


    priority_queue<PriorityRecord,vector<PriorityRecord> ,decltype(&compare_prio)> QEM_prio_queue(&compare_prio);

    for(int vsrc = 0; vsrc < int(m->vertices.size()); vsrc++){
        for (int vdst : nfa->nodes[vsrc].adjacentVertices){
            //SANITY CHECK
           PriorityRecord pr = PriorityRecord();
           pr.v_src = vsrc;
           pr.v_dst = vdst;
           pr.elision_cost = compute_elision_cost(vsrc, vdst);
        }
    }
    

    //Quadratic error computation
    //Choose the vertices to collapse 
    //Collapse the edge
    //Iterate
}

CollapsedEdge HalfEdgeCollapse(Mesh *m,NodeFaceAdjacency *nfa, int src, int dst){
    CollapsedEdge c = CollapsedEdge();
    c.v_src = src;
    c.v_dst = dst;
    vector<int> adjfaces;
    c.src_pos = m->vertices[src].Position;
    // GetAdjFaces(m, src, &adjfaces);
    // GetAdjVertex(m,facesIdx[0], src, dst, &c);
    // Set the src vertex as deactivated
    return c;
}

std::vector<CollapsedEdge> VertexSplit(){
    


    return std::vector<CollapsedEdge>();
}


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>> HELPER FUNCTIONS <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

bool compare_prio(PriorityRecord p1, PriorityRecord p2){
    return p1.elision_cost < p2.elision_cost;
}

bool is_adj (Face f, int src, int dst){ 
    bool v1 = f.vertices[0] == src || f.vertices[1] == src || f.vertices[2] == src;
    bool v2 = f.vertices[0] == dst || f.vertices[1] == dst || f.vertices[2] == dst;
    return v1 && v2;
}

void GetAdjFaces(Mesh *m, NodeFaceAdjacency *nfa, int src, vector<int> *adjFaces){
    for (int& fId : nfa->nodes[src].adjacencyFaces){
        adjFaces->emplace_back(fId);
    } 
}

void GetAdjVertex (Mesh *m, int f, int src, int dst, CollapsedEdge *edge){
    int idx1 = -1; 
    int idx2 = -1;
    for(int i = 0 ; i < 3; i++){ 
        if( m->faces[f].vertices[i] == src)
            idx1 = i;
        else if (m->faces[f].vertices[i] == dst) {
            idx2 = i;
        }
    }
    if(idx1 == (idx2+1)%3){ //Right Vertex
        FaceRecord fr = FaceRecord(f,(idx1+1)%3);
        edge->faces_to_rebind.emplace_back(fr);
        edge->vadj_right = m->faces[f].vertices[fr.cornerIdx];
    }else{                  //Left Vertex
        FaceRecord fr = FaceRecord(f,(idx2+1)%3);
        edge->faces_to_rebind.emplace_back(fr);
        edge->vadj_left = m->faces[f].vertices[fr.cornerIdx];
    }
}

glm::vec4 faceNormal(Mesh *m, int face_id){
    Face f = m->faces[face_id]; 
    glm::vec3 pos1 = m->vertices[f.vertices[0]].Position;
    glm::vec3 pos2 = m->vertices[f.vertices[1]].Position;
    glm::vec3 pos3 = m->vertices[f.vertices[2]].Position;
    glm::vec3 vect_1 = pos3 - pos1;
    glm::vec3 vect_2 = pos2 - pos1;
    glm::vec3 normal = glm::normalize(vect_1*vect_2);
    float d_value = -(glm::dot(normal,pos1)); 
    return glm::vec4(normal,d_value);
}

glm::mat4x4 fundamental_plane_q_mat(glm::vec4 plane){
    return glm::outerProduct(plane, plane);
}

void provaStampaListaDIMatrici(vector<glm::mat4x4> *Q){
    for (int i=0; i< int(Q->size()); i++){
        cout << endl << "{";
        for (int r = 0; r < 4; r++){
            cout << "[ ";
            for(int c = 0 ; c < 4 ; c++){
                cout << Q[i][r][c] << ", ";
            }
            cout << "]"<<endl;
        }
        cout << "}" << endl;
    }
}
