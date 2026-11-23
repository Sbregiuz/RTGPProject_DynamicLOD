#pragma once

#include "utils/NodeFaceAdjacency.h"
#include "utils/mesh_simplifier.h"
#include <utils/model.h>
#include <utils/LODHierarchy.h>

struct SubMeshController{
    // const float bounding_sphere_radius;
    Mesh *m;
    LODHierarchy *lod_h;

    SubMeshController(Mesh *mesh, LODHierarchy *lod) : m(mesh),lod_h(lod){}
};

class LODController{
    
    vector<SubMeshController> dags;
    
    public:
        LODController(Model *model){
            for (int i = 0; i < int(model->meshes.size()); i++){
                Mesh *m = &model->meshes[i];

                NodeFaceAdjacency nfa(m);
                float b_sphere_radius = ComputeBoundingSphere(m);
                LODHierarchy* lod_h = new LODHierarchy(int(m->vertices.size()),b_sphere_radius);
                // cout << "--------------------------------------------------------" << endl;
                // cout << "Radius is "<< b_sphere_radius << endl;
                MeshSimplification(m, &nfa,lod_h);
                // ~nfa;

                // cout << "Verify Monotonicity " << ((lod_h->VerifyMonotonicity()) ? "true" : "false") <<endl;
                // cout << "Error Range ( "<< lod_h->history.front().cumulative_error<< ", " << lod_h->history.back().cumulative_error<< ")"<<endl;
                lod_h->normalize_errors();
                // cout << "NORMALIZED >> Error Range ( "<< lod_h->history.front().cumulative_error<< ", " << lod_h->history.back().cumulative_error<< ")"<<endl;
                
                dags.emplace_back(SubMeshController(m,lod_h));
            }
        }

        void UpdateLOD(float cameraDistance){
            for(SubMeshController &controller: dags){
                
                TransformationInfo tinfo = controller.lod_h->TransformationsList(cameraDistance);
                vector<CollapsedEdge> transformations = tinfo.transformations;

                // Operation Selector
                auto f = tinfo.split 
                    ? &LODController::Apply_VertexSplit 
                    : &LODController::Apply_EdgeCollapse;

                for( CollapsedEdge ce : tinfo.transformations){
                    (this->*f)(controller.m,ce);
                    controller.lod_h->update_active(ce.v_src, tinfo.split);


                    // if (can_be_split(&ce, controller.lod_h)){
                    //     Apply_VertexSplit(controller.m, transformations[i]);
                    //     controller.lod_h->update_active(ce.v_src, true);
                    // }
                }
                controller.m->SyncGPU();
            }
        }

    private:
        void Apply_EdgeCollapse(Mesh *m,CollapsedEdge ce){
            Apply(m, ce, false, ce.v_dst);
        }

        void Apply_VertexSplit(Mesh *m,CollapsedEdge ce){
            Apply(m, ce, true, ce.v_src);
        }

        void Apply(Mesh *m, CollapsedEdge ce, bool activate, int v){
            if (ce.fadj_left != -1)
                m->faces[ce.fadj_left].active = activate;
            if (ce.fadj_right != -1)
                m->faces[ce.fadj_right].active = activate;

            //Rebound faces -> Substitute the v_dst in cornerIdx position with v_src
            for(FaceRecord &fr : ce.faces_to_rebind){
                Face *f = &m->faces[fr.fIdx];
                f->vertices[fr.cornerIdx] = v;
            }
        }

        bool can_be_split(CollapsedEdge *ce, LODHierarchy *lod_h){

            if (ce->vadj_left != -1 && !lod_h->isVertexActive(ce->vadj_left)) return false;
            if (ce->vadj_right != -1 && !lod_h->isVertexActive(ce->vadj_right)) return false;

            return true;
        }
            

        float ComputeBoundingSphere(const Mesh* m){
            if (m->vertices.empty()) {
                return 0.0f;
            }

            // Computing AABB sizes
            glm::vec3 minBounds(std::numeric_limits<float>::max());
            glm::vec3 maxBounds(std::numeric_limits<float>::lowest());

            for (const auto& v : m->vertices) {
                minBounds = glm::min(minBounds, v.Position);
                maxBounds = glm::max(maxBounds, v.Position);
            }

            // Finding the center of the sphere (the center of the AABB)
            glm::vec3 localCenter = 0.5f * (minBounds + maxBounds);

            // sphere radius is the distance from the fartest vertex and the sphere center
            float maxDistSq = 0.0f;
            for (const auto& v : m->vertices) {
                glm::vec3 diff = v.Position - localCenter;
                float distSq = glm::dot(diff, diff);
                if (distSq > maxDistSq) {
                    maxDistSq = distSq;
                }
            }

            float localRadius = std::sqrt(maxDistSq);
            return localRadius;
        }



    int CountActiveFaces(Mesh *m){
        int count =0 ; 
        for (Face f : m->faces){
            if(f.active == true) 
                count++;
        }
        return count;
    }
};
