/**
 * Definition of equations relating to j_hot (the radial profile 
 * of parallel current density j_|| / (B/B_min) of hot electrons).
 */

#include "DREAM/EquationSystem.hpp"
#include "DREAM/Settings/SimulationGenerator.hpp"
#include "DREAM/Equations/Fluid/CurrentDensityFromDistributionFunction.hpp"
#include "FVM/Equation/ConstantParameter.hpp"
#include "FVM/Equation/IdentityTerm.hpp"
#include "FVM/Grid/Grid.hpp"


using namespace DREAM;

#define MODULENAME "eqsys/j_hot"


/**
 * Construct the equation for the hot parallel current, 'j_hot'.
 * If the hot-tail grid is enabled, j_hot will be an integral of
 * the hot electron distribution. If it does not exist, it is set
 * to 0.
 *
 * eqsys:  Equation system to put the equation in.
 * s:      Settings object describing how to construct the equation.
 */
void SimulationGenerator::ConstructEquation_j_hot(
    EquationSystem *eqsys, Settings*
) {
//    const real_t t0 = 0;

    FVM::Grid *fluidGrid   = eqsys->GetFluidGrid();
    FVM::Grid *hottailGrid = eqsys->GetHotTailGrid();
    len_t id_j_hot = eqsys->GetUnknownID(OptionConstants::UQTY_J_HOT);
    len_t id_f_hot = eqsys->GetUnknownID(OptionConstants::UQTY_F_HOT);

    // If the hot-tail grid is enabled, we calculate j_hot as a
    // moment of the hot electron distribution function...
    if (hottailGrid) {
        FVM::Equation *eqn = new FVM::Equation(fluidGrid);

        CurrentDensityFromDistributionFunction *mq  = new CurrentDensityFromDistributionFunction(
            fluidGrid, hottailGrid, id_j_hot, id_f_hot
        );
        eqn->AddTerm(mq);
        eqsys->SetEquation(id_j_hot, id_f_hot, eqn, "Moment of f_hot");

        // Identity part
        FVM::Equation *eqnIdent = new FVM::Equation(fluidGrid);
        eqnIdent->AddTerm(new FVM::IdentityTerm(fluidGrid, -1.0));
        eqsys->SetEquation(id_j_hot, id_j_hot, eqnIdent);

        // Initialize to zero
        //eqsys->SetInitialValue(OptionConstants::UQTY_N_HOT, nullptr, t0);
    // Otherwise, we set it to zero...
    } else {
        FVM::Equation *eqn = new FVM::Equation(fluidGrid);

        eqn->AddTerm(new FVM::ConstantParameter(fluidGrid, 0));
        eqn->AddTerm(new FVM::IdentityTerm(fluidGrid));

        eqsys->SetEquation(id_j_hot, id_j_hot, eqn, "zero");
    }
}

