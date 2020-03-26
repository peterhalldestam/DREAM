#ifndef _DREAM_FVM_BOUNDARY_CONDITION_HPP
#define _DREAM_FVM_BOUNDARY_CONDITION_HPP

#include "FVM/config.h"
#include "FVM/Grid/Grid.hpp"
#include "FVM/Matrix.hpp"

namespace DREAM::FVM::BC {
    class BoundaryCondition {
    protected:
        Grid *grid;

    public:
        BoundaryCondition(Grid *g) : grid(g) {};

        virtual bool GridRebuilt() { return false; }

        virtual bool Rebuild(const real_t t) = 0;
        virtual void SetMatrixElements(Matrix*) = 0;
    };
}

#endif/*_DREAM_FVM_BOUNDARY_CONDITION_HPP*/
