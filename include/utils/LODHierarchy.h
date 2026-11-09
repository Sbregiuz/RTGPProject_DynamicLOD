/* Directed Acyclic Graph class 
 * This class serves to store the mesh contractions, simplify it and then use it to determine the cut (the resolution of the final mesh)
 */

#pragma once 

#include <glm/glm.hpp>
#include <vector>

const float max_distance = 200.0f;
const float min_distance = 0.0f;

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
    int fadj_left = -1;
    int vadj_right = -1;
    int fadj_right = -1;
    glm::vec3 src_pos;
    vector<FaceRecord> faces_to_rebind;
    int removed_faces;
};

class LODHierarchy
{
    public:
        //Contains all the contraction history
        vector<CollapsedEdge> history;

        //Vector for consulting the parent nodes runime in 0(1)
        vector<int> parent_nodes; 

        vector<bool> active_nodes;

        //how far to contract the mesh (distance based)
        int frontier_cut;

        LODHierarchy(int nodes_size){
            parent_nodes = vector<int>(nodes_size);
            active_nodes = vector<bool>(nodes_size);
            for(int i = 0 ; i < nodes_size; i++){
                active_nodes[i] = true;
            }
        }
    
        void add_contraction(CollapsedEdge ce){
            history.push_back(ce);
            active_nodes[ce.v_src] = false;
            frontier_cut ++;
        }

        void update_parent(int son, int new_parent){
            parent_nodes[son] = new_parent;
        }

        void update_active(int v, bool state){
            active_nodes[v] = state;
        }

        bool isVertexActive(int v){
            return active_nodes[v];
        }

        vector<CollapsedEdge> TransformationsList(float distance_from_camera){
            vector<CollapsedEdge> return_val;
            int max_index = int((history.size()/max_distance)*distance_from_camera);
            for(int i = 0; i < max_index; i++){
                return_val.emplace_back(history[i]);
            }
            cout << "Reconstruct history up to " << max_index << endl;
            return return_val;
        }
};

