#pragma once

#include "utils/NodeFaceAdjacency.h"
#include "utils/mesh_simplifier.h"
#include <utils/model.h>
#include <utils/LODHierarchy.h>

struct JustToBeSure{
    Mesh *m;
    LODHierarchy *lod_h;

    JustToBeSure(Mesh *mesh, LODHierarchy *lod) : m(mesh),lod_h(lod){}
};

class LODController{
    
    vector<JustToBeSure> dags;
    
    public:
        LODController(Model *model){
            for (int i = 0; i < int(model->meshes.size()); i++){
                Mesh *m = &model->meshes[i];

                NodeFaceAdjacency nfa(m);
                LODHierarchy* lod_h = new LODHierarchy(int(m->vertices.size()));

                MeshSimplification(m, &nfa,lod_h);
                ~nfa;

                // cout << "ORIGNAL FACES VS ACTIVE FACES: " << m->faces.size()<< " vs "<<CountActiveFaces(m) << endl;
                dags.emplace_back(JustToBeSure(m,lod_h));
            }
        }

        void UpdateLOD(float cameraDistance){
            for(JustToBeSure &jtbs: dags){
                
                TransformationInfo tinfo = jtbs.lod_h->TransformationsList(cameraDistance);
                vector<CollapsedEdge> transformations = tinfo.transformations;

                // Operation Selector
                auto f = tinfo.split 
                    ? &LODController::Apply_VertexSplit 
                    : &LODController::Apply_EdgeCollapse;

                for( CollapsedEdge ce : tinfo.transformations){
                    (this->*f)(jtbs.m,ce);
                    jtbs.lod_h->update_active(ce.v_src, tinfo.split);


                    // if (can_be_split(&ce, jtbs.lod_h)){
                    //     Apply_VertexSplit(jtbs.m, transformations[i]);
                    //     jtbs.lod_h->update_active(ce.v_src, true);
                    // }
                }
                jtbs.m->SyncGPU();

                // cout << "ORIGNAL FACES VS ACTIVE FACES: " << jtbs.m->faces.size()<< " vs "<<CountActiveFaces(jtbs.m) << endl;
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



    int CountActiveFaces(Mesh *m){
        int count =0 ; 
        for (Face f : m->faces){
            if(f.active == true) 
                count++;
        }
        return count;
    }
};



