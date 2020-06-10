#ifndef _DREAM_SOLVER_HPP
#define _DREAM_SOLVER_HPP
/* Definition of the abstract base class 'Solver', which
 * defines the interface for all equation solvers in DREAM.
 */

#include <vector>
#include "DREAM/Equations/CollisionQuantityHandler.hpp"
#include "DREAM/Equations/RunawayFluid.hpp"
#include "DREAM/UnknownQuantityEquation.hpp"
#include "FVM/BlockMatrix.hpp"
#include "FVM/FVMException.hpp"
#include "FVM/UnknownQuantityHandler.hpp"

namespace DREAM {
    class Solver {
    protected:
        FVM::UnknownQuantityHandler *unknowns;
        // List of equations associated with unknowns (owned by the 'EquationSystem')
        std::vector<UnknownQuantityEquation*> *unknown_equations;
        std::vector<len_t> nontrivial_unknowns;

        // Mapping from EquationSystem 'unknown_quantity_id' to index
        // in the block matrix representing the system
        std::map<len_t, len_t> unknownToMatrixMapping;

        // Number of rows in any (jacobian) matrix built by
        // this solver (not counting unknowns which should
        // not appear in the matrix)
        len_t matrix_size;

        CollisionQuantityHandler *cqh_hottail, *cqh_runaway;
        RunawayFluid *REFluid;

        virtual void initialize_internal(const len_t, std::vector<len_t>&) {}

    public:
        Solver(FVM::UnknownQuantityHandler*, std::vector<UnknownQuantityEquation*>*);
        virtual ~Solver() {}

        void BuildJacobian(const real_t, const real_t, FVM::BlockMatrix*);
        void BuildMatrix(const real_t, const real_t, FVM::BlockMatrix*, real_t*);
        void BuildVector(const real_t, const real_t, real_t*, FVM::BlockMatrix*);
        void RebuildTerms(const real_t, const real_t);

        //virtual const real_t *GetSolution() const = 0;
        virtual void Initialize(const len_t, std::vector<len_t>&);

        virtual void SetCollisionHandlers(
            CollisionQuantityHandler *cqh_hottail,
            CollisionQuantityHandler *cqh_runaway,
            RunawayFluid *REFluid
        ) {
            this->cqh_hottail = cqh_hottail;
            this->cqh_runaway = cqh_runaway;
            this->REFluid = REFluid;
        }
        virtual void SetInitialGuess(const real_t*) = 0;
        virtual void Solve(const real_t t, const real_t dt) = 0;
    };

    class SolverException : public DREAM::FVM::FVMException {
    public:
        template<typename ... Args>
        SolverException(const std::string &msg, Args&& ... args)
            : FVMException(msg, std::forward<Args>(args) ...) {
            AddModule("Solver");
        }
    };
}

#endif/*_DREAM_SOLVER_HPP*/
