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
#include <utils/ContinuousLod.h>

struct PriorityRecord;

bool compare_prio(PriorityRecord p1, PriorityRecord p2);
bool sanity_check(int vsrc, int vdst, Mesh *m, Node *n);
float compute_elision_cost(glm::vec4,glm::mat4x4 Qcombo);
bool is_adj (Face f, int src, int dst);
void GetAdjFaces(Mesh *m, int src, vector<int> *adjFaces);
void GetLRVertices (Mesh *m, int f, int src, int dst, CollapsedEdge *edge);
glm::vec4 faceNormal(Mesh *m, int face_id);
glm::mat4x4 fundamental_plane_q_mat(glm::vec4 plane);
void provaStampaListaDIMatrici(vector<glm::mat4x4> *Q);


CollapsedEdge HalfEdgeCollapse(Mesh *m,NodeFaceAdjacency *nfa, int src, int dst);


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> STRUCTS
struct PriorityRecord{
    int v_src;
    int v_dst;
    float elision_cost;
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

           PriorityRecord pr = PriorityRecord();
           pr.v_src = vsrc;
           pr.v_dst = vdst;
           glm::vec4 v = glm::vec4(m->vertices[vdst].Position, 1.0f);
           pr.elision_cost = compute_elision_cost(v, Q[vsrc]+Q[vdst]);
           QEM_prio_queue.push(pr);
        }
    }
   
    float residual_percentage = 0.05;
    int target_faces = m->faces.size()*residual_percentage;
    int actual_faces = m->faces.size();
    while (actual_faces > target_faces && !QEM_prio_queue.empty()){

        //Extracting the cheapest cost contraction
        PriorityRecord extracted = QEM_prio_queue.top();
        QEM_prio_queue.pop();

        //If the record has deactivated node/s OR this compression would cause foldovers skip it
        if(nfa->nodes[extracted.v_src].active == false || nfa->nodes[extracted.v_dst].active == false)
            continue;

        if (!sanity_check(extracted.v_src, extracted.v_dst,m, &nfa->nodes[extracted.v_src]))
            continue;

        //Collapse the half edge and update the cost
        CollapsedEdge ce = HalfEdgeCollapse(m, nfa, extracted.v_src, extracted.v_dst);

        // ---> Aggiungere ce nel DAG 
        m->dag.add_contraction(ce);
        
        // ---> Aggiornare parentela Gerarchica (DAG -> Parents[src] = dst)
        m->dag.update_parent(extracted.v_src, extracted.v_dst);

        Q[extracted.v_dst] += Q[extracted.v_src]; 

        //Update the cost adding new record in the QEM_prio_queue
        for ( int v_id : nfa->nodes[extracted.v_dst].adjacentVertices){
            //If the possible destination node is deactivated -> skip
            if (!nfa->nodes[v_id].active)
                continue;

            //Otherwise create a new PriorityRecord
            PriorityRecord pr = PriorityRecord();
            pr.v_src = extracted.v_dst;
            pr.v_dst = v_id;
            pr.elision_cost = compute_elision_cost(glm::vec4(m->vertices[v_id].Position,1.0f), Q[pr.v_src]+Q[pr.v_dst]);
            QEM_prio_queue.push(pr);
        }
        actual_faces -= ce.removed_faces;
    }
}

CollapsedEdge HalfEdgeCollapse(Mesh *m,NodeFaceAdjacency *nfa, int src, int dst){
    CollapsedEdge c = CollapsedEdge();
    c.v_src = src;
    c.v_dst = dst;
    c.src_pos = m->vertices[src].Position;
    c.removed_faces = 0;

    vector<int> &adjFaces = nfa->nodes[src].adjacencyFaces;
    for (int f_id : adjFaces){
        Face &f = m->faces[f_id];
        
        if (!f.active)
            continue;

        if (is_adj(f,src,dst)){
            f.active = false; 
            c.removed_faces+=1;
            
            GetLRVertices(m, f_id, src, dst, &c); 
        }else{
            int src_corner = -1;
            
            for (int k = 0; k < 3; ++k) {
                if (f.vertices[k] == src) {
                    src_corner = k;
                    f.vertices[k] = dst; // FONDAMENTALE: rimappa il vertice nella mesh!
                    break;
                }
            }
            
            FaceRecord fr = FaceRecord(f_id, src_corner);
            c.faces_to_rebind.push_back(fr);

            nfa->nodes[dst].adjacencyFaces.push_back(f_id);
        }
    }
    
    nfa->nodes[src].active = false;
    return c;
}

std::vector<CollapsedEdge> VertexSplit(){
    


    return std::vector<CollapsedEdge>();
}


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>> HELPER FUNCTIONS <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

bool compare_prio(PriorityRecord p1, PriorityRecord p2){
    return p1.elision_cost > p2.elision_cost;
}

bool sanity_check(int vsrc, int vdst, Mesh *m ,Node *n_src){
    glm::vec3 dst_position = m->vertices[vdst].Position;

    for(int f_id : n_src->adjacencyFaces){
        Face f = m->faces[f_id];

        if (!f.active)
            continue;

        if (f.vertices[0] == vdst || f.vertices[1] == vdst || f.vertices[2] == vdst) 
            continue;

        glm::vec3 p0 = m->vertices[f.vertices[0]].Position;
        glm::vec3 p1 = m->vertices[f.vertices[1]].Position;
        glm::vec3 p2 = m->vertices[f.vertices[2]].Position;

        glm::vec3 crossp = glm::cross(p1-p0, p2-p0);
        glm::vec3 n_old = glm::normalize(crossp);
        
        glm::vec3 p0_new = (f.vertices[0] == vsrc) ? dst_position : p0;
        glm::vec3 p1_new = (f.vertices[1] == vsrc) ? dst_position : p1;
        glm::vec3 p2_new = (f.vertices[2] == vsrc) ? dst_position : p2;

        glm::vec3 crossp_new = glm::cross(p1_new - p0_new, p2_new - p0_new);
        glm::vec3 n_new = glm::normalize(crossp_new);

        if(glm::dot(n_old, n_new) <= 0.0f)
            return false;
    }

    return true;
}

float compute_elision_cost(glm::vec4 v,glm::mat4x4 Qcombo){
    glm::vec4 v_t = glm::vec4(0.0f); 
    glm::mat4x4 m_t = glm::transpose(Qcombo);

    for(int i = 0; i< 4 ; i++){
        v_t[i] = glm::dot(v,m_t[i]);
    }

    return glm::dot(v_t,v);
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

void GetLRVertices (Mesh *m, int f, int src, int dst, CollapsedEdge *edge){
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
        edge->vadj_right = m->faces[f].vertices[(idx1+1)%3];
    }else{                  //Left Vertex
        edge->vadj_left = m->faces[f].vertices[(idx2+1)%3];
    }
}

glm::vec4 faceNormal(Mesh *m, int face_id){
    Face f = m->faces[face_id]; 
    glm::vec3 pos1 = m->vertices[f.vertices[0]].Position;
    glm::vec3 pos2 = m->vertices[f.vertices[1]].Position;
    glm::vec3 pos3 = m->vertices[f.vertices[2]].Position;
    glm::vec3 vect_1 = pos3 - pos1;
    glm::vec3 vect_2 = pos2 - pos1;
    glm::vec3 normal = glm::normalize(glm::cross(vect_1,vect_2));
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
