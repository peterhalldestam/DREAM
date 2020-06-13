#ifndef _DREAM_FVM_EQUATION_SCALAR_LINEAR_TERM_HPP
#define _DREAM_FVM_EQUATION_SCALAR_LINEAR_TERM_HPP

#include "FVM/Equation/EvaluableEquationTerm.hpp"
#include "FVM/UnknownQuantityHandler.hpp"
#include <algorithm>

namespace DREAM::FVM {
    class ScalarLinearTerm : public EvaluableEquationTerm {
    private:
        virtual void DeallocateWeights();
        virtual void AllocateWeights();
        

    protected:
        len_t nWeights;
        UnknownQuantityHandler *unknowns;
        Grid *targetGrid;
        len_t uqtyId;
        real_t *weights = nullptr;
        virtual void SetWeights() = 0;

        
    public:
        ScalarLinearTerm(Grid*,Grid*, UnknownQuantityHandler*, const len_t);
        
        /**
         * This term shows up together with 'PredeterminedParameter' and
         * such, and so we never actually want to assign anything to the
         * vector when evaluating this term (this term indicates that we
         * want to evaluate EVERYTHING ELSE in the equation). */
        virtual real_t* Evaluate(real_t*, const real_t*, const len_t, const len_t) override;

        virtual void SetMatrixElements(Matrix*, real_t*) override;
        virtual void SetVectorElements(real_t*, const real_t*) override;

        /**
         * The following methods are to be inherited from DiagonalTerm
         */
        virtual len_t GetNumberOfNonZerosPerRow() const override 
            {return nWeights;}
        virtual len_t GetNumberOfNonZerosPerRow_jac() const override
            {return GetNumberOfNonZerosPerRow(); }
        virtual void Rebuild(const real_t, const real_t, UnknownQuantityHandler*) override;
        virtual void SetJacobianBlock(const len_t uqtyId, const len_t derivId, Matrix *jac, const real_t* x) override;
        virtual bool GridRebuilt() override;

    };
}

#endif/*_DREAM_FVM_EQUATION_SCALAR_LINEAR_TERM_HPP*/
