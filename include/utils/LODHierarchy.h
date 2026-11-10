/* Directed Acyclic Graph class 
 * This class serves to store the mesh contractions, simplify it and then use it to determine the cut (the resolution of the final mesh)
 */

#pragma once 

#include "glm/common.hpp"
#include <algorithm>
#include <glm/glm.hpp>
#include <vector>

const float max_distance = 180.0f;
const float min_distance = 10.0f;

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

struct TransformationInfo{
    vector<CollapsedEdge> transformations;
    bool split;
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
            frontier_cut = -1;
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

        int new_cut(float distance_from_camera){
            float t = (distance_from_camera - min_distance)/(max_distance - min_distance);

            float f = glm::clamp(t, 0.0f, 1.0f);

            return int(f * history.size());  //Range (0,n)       
        }

        TransformationInfo TransformationsList(float distance_from_camera){
            TransformationInfo return_val;

            //New cut
            int cut = new_cut(distance_from_camera);

            //If nothing changed, return
            if(cut == frontier_cut)
                return return_val;

            int lower = min(cut,frontier_cut);
            int upper = max(cut,frontier_cut);

            //Prepare transformation list
            for(int i = lower; i < upper; i++){ //
                return_val.transformations.emplace_back(history[i]);
            }

            //If the new cut is lower than the previous cut --> We must INCREASE Resolution (Split)
            //Otherwise we must collapse further
            if (cut < frontier_cut){
                reverse(return_val.transformations.begin(), return_val.transformations.end());
                return_val.split = true;
            }
            else{
                return_val.split = false;
            }

            // cout << "Reconstruct history FROM "<< frontier_cut << " up to " << cut << endl;
            frontier_cut = cut;
            return return_val;
        }
};

