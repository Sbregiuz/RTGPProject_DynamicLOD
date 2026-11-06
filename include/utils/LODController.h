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
                LODHierarchy lod_h(int(m->vertices.size()));

                MeshSimplification(m, &nfa,&lod_h);
                ~nfa;

                dags.emplace_back(JustToBeSure(m,&lod_h));
            }
        }

        void UpdateLOD(float cameraDistance){
            for(JustToBeSure jtbs: dags){
                //Get contractions/Expansions list
                //Apply to the mesh
                jtbs.m->SyncGPU();
            }
        }



};



