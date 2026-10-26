#pragma once

#include <utils/mesh.h>
#include <vector>
#include <glm/glm.hpp>
#include <glfw/glfw3.h>
#include <glad/glad.h>

class CollapsedEdge{
    private:
        int v_src;
        int v_dst;
        int vadj_left,vadj_right;
        glm::vec3 src_pos;
        int faces_to_rebind[];

    public:
        CollapsedEdge(int src, int dst, int adj_right, int adj_left){
            v_src = src;
            v_dst = dst;
            vadj_left = adj_left;
            vadj_right = adj_right;
        };
};

CollapsedEdge HalfEdgeCollapse(Mesh *m, int src, int dst){
    return CollapsedEdge(src,dst,0,0);
}

std::vector<CollapsedEdge> VertexSplit(){
    


    return std::vector<CollapsedEdge>();
}
