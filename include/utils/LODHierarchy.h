/* Directed Acyclic Graph class 
 * This class serves to store the mesh contractions, simplify it and then use it to determine the cut (the resolution of the final mesh)
 */

#pragma once 

#include "glm/common.hpp"
#include <algorithm>
#include <glm/glm.hpp>
#include <iterator>
#include <vector>
#include "utils/Env_Variables.h"

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
    float cumulative_error = 0.0f;
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

        const float bounding_sphere_radius;

        int frontier_cut;

        LODHierarchy(int nodes_size, float sphere_radius): bounding_sphere_radius(sphere_radius){
            parent_nodes = vector<int>(nodes_size);
            active_nodes = vector<bool>(nodes_size);
            for(int i = 0 ; i < nodes_size; i++){
                active_nodes[i] = true;
            }
            frontier_cut = -1;
        }
    
        void add_contraction(CollapsedEdge ce) {

            float prev_error = history.empty() ? 0.0f : history.back().cumulative_error;
            ce.cumulative_error = std::max(prev_error, ce.cumulative_error);
            if (ce.cumulative_error < 1e-6f) {
                ce.cumulative_error = 0.0f;
            }

            history.push_back(ce);
            active_nodes[ce.v_src] = false;
            frontier_cut++;
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


        bool VerifyMonotonicity(){
            float last_error = -1000000;
            for (CollapsedEdge ce : history) {
                if (last_error > ce.cumulative_error)
                    return false;
                last_error = ce.cumulative_error;
            }
            return true;
        }

        TransformationInfo TransformationsList(float distance_from_camera){
            distance_from_camera = distance_from_camera - bounding_sphere_radius;
            //Considera togliere dalla distanza il bounding radius mesh
            distance_from_camera = max(0.0f, distance_from_camera);
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

        void normalize_errors() {
            std::vector<CollapsedEdge>& history = this->history;
            if (history.empty() || bounding_sphere_radius <= 0.0f) return;

            float max_err = history.back().cumulative_error;
            if (max_err <= 0.0f) return; // Flat sub-mesh (all zero errors), keep as-is

            float max_world_deviation = 0.15f * bounding_sphere_radius;
            float scale = max_world_deviation / max_err;
            // cout << "THIS IS THE SCALE TO APPLY " << scale << endl;

            for (auto& ce : history) {
                ce.cumulative_error *= scale;
            }
        }

    private:

        int new_cut(float distance_from_camera){
            int idx = cut_function_selector;
            switch (idx) {
                case 0 : 
                    return linear_interpolation(distance_from_camera);

                case 1 :
                    return linear_interpolation(distance_from_camera);

                case 2 :
                    return screen_space_coverage(distance_from_camera);

                default : 
                    return screen_space_error(distance_from_camera);
            }
        }

        int linear_interpolation(float distance_from_camera){
            float t = (distance_from_camera - min_range)/(max_range - min_range);

            float f = glm::clamp(t, 0.0f, 1.0f);

            return int(f * history.size());  //Range (0,n)       
        }

        int screen_space_error(float distance_from_camera){
            float max_error = distance_from_camera * sse_error_partial;

            auto it = upper_bound(history.begin(), history.end(), max_error, 
                        [](float target_error, CollapsedEdge &ce){
                            return target_error < ce.cumulative_error;
                        }
                    );

            return int(distance(history.begin(), it));
        }

        int screen_space_coverage(float distance_from_camera){
            float pixel_diameter = ssc_error_partial * (bounding_sphere_radius / distance_from_camera);

            float f = 1 - glm::clamp( (pixel_diameter - min_pixels)/(max_pixels - min_pixels) , 0.0f, 1.0f);
            return int(f * history.size());
        }

};

