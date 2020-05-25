/**
 * Implementation of the boundary condition used on the isotropic hot-tail
 * grid to facilitate transfer of particles from/to the cold electron
 * population.
 *
 * NOTE: This boundary condition only works for p/xi grids.
 */

#include "DREAM/Equations/CollisionQuantityHandler.hpp"
#include "DREAM/Equations/Kinetic/BCIsotropicSource.hpp"


using namespace DREAM;



/**
 * Constructor.
 */
BCIsotropicSource::BCIsotropicSource(FVM::Grid *g, CollisionQuantityHandler *cqh)
    : FVM::BC::PInternalBoundaryCondition(g) {
    
    this->slowingDownFreq = cqh->GetNuS();
}


/**
 * Rebuild the flux.
 */
bool BCIsotropicSource::Rebuild(const real_t, FVM::UnknownQuantityHandler*) {
    for (len_t ir = 0; ir < grid->GetNr(); ir++) {
        const len_t nxi = grid->GetMomentumGrid(ir)->GetNp2();
        const real_t p3nuS = this->slowingDownFreq->GetP3NuSAtZero(ir);
        const real_t Vp_p2 = /*TODO*/1;

        for (len_t j = 0; j < nxi; j++)
            this->VpS[ir][j] = p3nuS*Vp_p2;
    }

    return true;
}

