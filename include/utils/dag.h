/* Directed Acyclic Graph class 
 * This class serves to store the mesh details, simplify it and then use it to determine the cut (the resolution of the final mesh)
 */

#pragma once 
#include <vector>
class Dag
{
    public: 




    private:
    class DagNode{
        int vertexIdx;
        std::pmr::vector<DagNode> neighbors;
    };


};

