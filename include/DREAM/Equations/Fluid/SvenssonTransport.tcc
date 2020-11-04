/**
 * Implementation of a diffusive transport term which can be applied to both
 * kinetic and fluid grids, and which allows one to prescribe the diffusion
 * coefficient in time and phase space.
 */

#include <type_traits>
#include "DREAM/Equations/Fluid/SvenssonTransport.hpp"

/**
 * Constructor.
 */
template<typename T>
DREAM::SvenssonTransport<T>::SvenssonTransport(
    DREAM::FVM::Grid *grid,
    const len_t nr, const len_t np,
    const real_t pStar,
    const real_t **coeffA, const real_t **coeffD,
    const real_t *r, const real_t *p,
    DREAM::FVM::UnknownQuantityHandler *unknowns,
    DREAM::RunawayFluid *REFluid,
    bool allocCoefficients
) : T(grid, allocCoefficients),
    // nr(nr), np(np),
    pStar(pStar),
    coeffA(coeffA), coeffD(coeffD), r(r), p(p),
    unknowns(unknowns), REFluid(REFluid)
{
    this->EID = this->unknowns->GetUnknownID(OptionConstants::UQTY_E_FIELD); 

    // Interpolation of 

    // YYY Isn't this more in line with what we should be doing?
    // this->nr = this->grid->GetNr();
    // this->np = this->grid->GetNp1(0); // N.B. Assuming uniform p grid

    this->integrand = new real_t[this->grid->GetNp1(0)];
}

/**
 * Destructor.
 */
template<typename T>
DREAM::SvenssonTransport<T>::~SvenssonTransport() {
    // YYY I guess that something should be in here?
    //if (this->integrand != nullptr)
    delete [] this->integrand;
    // if (this->coeffA != nullptr)
    //     delete this->coeffA;
}



/**
 * Rebuild this term by evaluating and setting the diffusion
 * coefficient for the next time step.
 */
template<typename T>
void DREAM::SvenssonTransport<T>::Rebuild(
    const real_t, const real_t, DREAM::FVM::UnknownQuantityHandler*
    ) {
    //const real_t *c = this->prescribedCoeff->Eval(t);
    
    const len_t nr = this->grid->GetNr();
    real_t *dp;
    
    
    // Iterate over the radial flux grid...
    for (len_t ir = 0; ir < nr+1; ir++) {
        // Need interpolation from cell grid to flux grid:
        // pBar_f[0]=pBar[0]
        // pBar_f[ir]  = (pBar[ir-1] + pBar[ir] )*.5
        // pBar_f[nr]= extrapolate
        
        // The varaible to be added to
        dp = this->grid->GetMomentumGrid(ir)->GetDp1();
        real_t pIntCoeff = 0;
        // We don't really need the integrandArray here, maybe replace in future.
        const real_t *integrandArray = this->EvaluateIntegrand(ir);
        for (len_t i = 0; i < this->grid->GetNp1(0); i++) {
            // The actual integration in p
            pIntCoeff += integrandArray[i] * dp[i];
                // Jacobian: * this->grid->GetVp(ir,i,0); 

        }
        // Sets the 
        this->_setcoeff(ir, pIntCoeff);
        
        //offset += this->np;
    }
}



/**
 * Helper function for calculating the inverse of p-bar, with the
 * (optional) additional calculation of the derivative of
 * p-bar-inverse. 
 *
 * `p-bar` is the name given to the factor dividing `-(p - p*)` in the
 * exponential of eqn (4.2) in Svensson et al. 2020
 * [https://arxiv.org/abs/2010.07156v1].
 * 
 * These values are calculated on the flux grid, meaning that
 * interpolation (and extrapolation) from the cell grid is being
 * performed. This is done via inter-/extrapolation of p-bar-inverse,
 * instead of first inter-/extrsapolating the values going into p-bar.
 */
// YYY Should each individual value be interpolated or pBar itsellf?
template<typename T>
real_t DREAM::SvenssonTransport<T>::GetPBarInv_f(len_t ir, real_t *dr_pBarInv_f){
    // Inverse of p-bar on the Flux grid, with additional helper variable.
    real_t pBarInv_f, tmp_pBarInv_f; 

    // Essential values taken on the raidal (cell) grid
    const real_t *E = this->unknowns->GetUnknownData(this->EID);
    const real_t *EcEff = this->REFluid->GetEffectiveCriticalField();
    const real_t *tauRel = this->REFluid->GetElectronCollisionTimeRelativistic();
    const real_t *gamma_r = this->REFluid->GetAvalancheGrowthRate();

    // Grid step size in the radial grid for the derivative.
    const real_t *dr_f = this->grid->GetRadialGrid()->GetDr_f();
    const real_t *dr = this->grid->GetRadialGrid()->GetDr(); 

    
    // Interpolating (extrapolating) the inverse of p bar onto the
    // flux grid.
    // YYY Cells can be of different size!!!
    // Need to redo the interpolation!!!
    if (ir == 0) {
        // Zero flux at r = 0. Therefore choose the value at ir=1/2.
        pBarInv_f = tauRel[0] * gamma_r[0] / (E[0]-EcEff[0]);
        if(dr_pBarInv_f!=nullprt)
            *dr_pBarInv_f = 0.0;
    }
    else if (ir == this->nr) {
        // Linearly extrapolating the value at the end point from the
        // two previous points.
        //
        // N.B.! The extrapolation assume that the grid cell size is
        // uniform, and that the extrapolated value lies half a grid
        // cell away from the last point.

        // pBarInv_f  = 1.5 * (tauRel[ir-1] * gamma_r[ir-1] / (E[ir-1]-EcEff[ir-1]));
        // pBarInv_f -= 0.5 * (tauRel[ir-2] * gamma_r[ir-2] / (E[ir-2]-EcEff[ir-2]));
        pBarInv_f     = tauRel[ir-1] * gamma_r[ir-1] / (E[ir-1]-EcEff[ir-1]);
        tmp_pBarInv_f = tauRel[ir-2] * gamma_r[ir-2] / (E[ir-2]-EcEff[ir-2]);

        // N.B.! This order of operations is important
        // Using the same derivative as for the linear extrapolation.
        if(dr_pBarInv_f!=nullprt)
            *dr_pBarInv_f = (pBarInv_f - tmp_pBarInv_f) / dr[ir-1];
        pBarInv_f *= 1.5;
        pBarInv_f -= 0.5 * tmp_pBarInv_f;
        
    }
    else {
        // In the middle, we simply linearly interpolate
        tmp_pBarInv_f = tauRel[ir-1] * gamma_r[ir-1] / (E[ir-1]-EcEff[ir-1]);
        pBarInv_f  = tauRel[ir] * gamma_r[ir] / (E[ir]-EcEff[ir]);

        // N.B.! This order of operations is important!
        if(dr_pBarInv_f!=nullprt)
            *dr_pBarInv_f = (pBarInv_f - tmp_pBarInv_f) / dr_f[ir-1]; // Derivative

        // Think about this!!
        // tmp_pBarInv_f + 0.5*dr[ir-1] * dr_pBarInv_f
        // (0.5 * dr[ir-1] * tmp_pBarInv_f + 0.5 * dr[ir] * pBarInv_f)/( dr_f[ir] )
        pBarInv_f += tmp_pBarInv_f;
        pBarInv_f *= 0.5;
    }

    return pBarInv_f;
}
