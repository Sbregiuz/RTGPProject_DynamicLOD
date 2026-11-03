/* Directed Acyclic Graph class 
 * This class serves to store the mesh contractions, simplify it and then use it to determine the cut (the resolution of the final mesh)
 */

#pragma once 

#include <glm/glm.hpp>
#include <vector>

using namespace std;

struct FaceRecord;
struct CollapsedEdge;

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
    int vadj_left = -1;
    int vadj_right = -1;
    glm::vec3 src_pos;
    vector<FaceRecord> faces_to_rebind;
    int removed_faces;
};

class ResolutionFrontier
{
    public:
        //Contains all the contraction history
        vector<CollapsedEdge> history;

        //Vector for consulting the parent nodes runime in 0(1)
        vector<int> parent_nodes; 

        //how far to contract the mesh (distance based)
        int frontier_cut;

        ResolutionFrontier(int nodes_size){
            parent_nodes = vector<int>(nodes_size);
        }
    
        void add_contraction(CollapsedEdge ce){
            history.push_back(ce);
            frontier_cut ++;
        }

        void update_parent(int son, int new_parent){
            parent_nodes[son] = new_parent;
        }

};

