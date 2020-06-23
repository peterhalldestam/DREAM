#include "FVM/Equation/DiagonalComplexTerm.hpp"
#include "DREAM/Equations/RunawayFluid.hpp"
#include "DREAM/IonHandler.hpp"
/**
 * Implementation of a class which represents the sigma*E contribution to the ohmic current equation.
 * Uses the Sauter formula for the conductivity which is valid across all collisionality regimes 
 * (i.e. goes beyond the collisionless banana limit which the kinetic DREAM equation considers)
 */
namespace DREAM {
    class CurrentFromConductivityTerm : public FVM::DiagonalComplexTerm {
    private:
        RunawayFluid *REFluid;
        IonHandler *ionHandler;
    protected:
        // Set weights for the Jacobian block. Uses differentiated conductivity provided by REFluid. 
        virtual void SetDiffWeights(len_t derivId, len_t nMultiples) override {
            real_t *dSigma = REFluid->evaluatePartialContributionSauterConductivity(ionHandler->evaluateZeff(),derivId);

            len_t offset = 0;
            for(len_t n = 0; n<nMultiples; n++){
                for (len_t ir = 0; ir < nr; ir++){
                    real_t w0 = 1.0/sqrt(grid->GetRadialGrid()->GetFSA_B2(ir));
                    for(len_t i = 0; i < n1[ir]*n2[ir]; i++)
                            diffWeights[offset + i] = w0*dSigma[offset + i];
                    offset += n1[ir]*n2[ir];
                }
            }
        }

        // Set weights as the conductivity with a geometric factor 
        virtual void SetWeights() override {
            len_t offset = 0;
            for (len_t ir = 0; ir < nr; ir++){
                real_t w = REFluid->evaluateSauterElectricConductivity(ir, ionHandler->evaluateZeff(ir))
                            / sqrt(grid->GetRadialGrid()->GetFSA_B2(ir));
                for(len_t i = 0; i < n1[ir]*n2[ir]; i++)
                    weights[offset + i] = w;
                offset += n1[ir]*n2[ir];
            }
        }
    public:
        CurrentFromConductivityTerm(FVM::Grid* g, FVM::UnknownQuantityHandler *u, RunawayFluid *ref, IonHandler *ih) 
            : FVM::DiagonalComplexTerm(g,u), REFluid(ref), ionHandler(ih)
        {
            /**
             * So far, we only account for the temperature dependence in the conductivity 
             * Jacobian and not, for example, ion densities which would enter through Zeff
             * and n_cold via the collisionality in the neoclassical corrections (and lnLambda). 
             */
            AddUnknownForJacobian(unknowns,unknowns->GetUnknownID(OptionConstants::UQTY_T_COLD));
        }

    };
}