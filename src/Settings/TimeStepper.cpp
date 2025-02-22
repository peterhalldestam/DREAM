/**
 * Construct a time stepper object.
 */

#include "DREAM/EquationSystem.hpp"
#include "DREAM/Settings/SimulationGenerator.hpp"
#include "DREAM/TimeStepper/TimeStepper.hpp"
#include "DREAM/TimeStepper/TimeStepperConstant.hpp"
#include "FVM/UnknownQuantityHandler.hpp"


using namespace DREAM;
using namespace std;


#define MODULENAME "timestep"


/**
 * Define options for the time stepper.
 * 
 * s: Settings object to define settings in.
 */
void SimulationGenerator::DefineOptions_TimeStepper(Settings *s) {
	s->DefineSetting(MODULENAME "/automaticstep", "Step length for the automatic determination of the time step in the ionization time stepper.", (real_t)1e-12);
    s->DefineSetting(MODULENAME "/checkevery", "Check the error every N'th step (0 = check error after _every_ time step)", (int_t)0);
    s->DefineSetting(MODULENAME "/constantstep", "Override the adaptive stepper and force a constant time step (DEBUG OPTION)", (bool)false);
    s->DefineSetting(MODULENAME "/dt", "Length of each time step", (real_t)0.0);
	s->DefineSetting(MODULENAME "/dtmax", "Maximum allowed time step for the adaptive ionization time stepper.", (real_t)0);
    s->DefineSetting(MODULENAME "/nsavesteps", "Number of time steps to save to output (downsampling)", (int_t)0);
    s->DefineSetting(MODULENAME "/nt", "Number of time steps to take", (int_t)0);
	s->DefineSetting(MODULENAME "/safetyfactor", "Safety factor to use when automatically determining the baseline timestep for the adaptive ionization time stepper.", (real_t)50);
    s->DefineSetting(MODULENAME "/tmax", "Maximum simulation time", (real_t)0.0);
    s->DefineSetting(MODULENAME "/type", "Time step generator type", (int_t)OptionConstants::TIMESTEPPER_TYPE_CONSTANT);
    s->DefineSetting(MODULENAME "/verbose", "If true, generates excessive output", (bool)false);

    // Tolerance settings for adaptive time stepper
    DefineToleranceSettings(MODULENAME, s);
}

/**
 * Construct a TimeStepper object according to the settings.
 *
 * eqsys: Equation system object to assign TimeStepper object to.
 * s:     Settings object specifying how to construct the TimeStepper object.
 */
void SimulationGenerator::ConstructTimeStepper(EquationSystem *eqsys, Settings *s) {
    enum OptionConstants::timestepper_type type = (enum OptionConstants::timestepper_type)s->GetInteger(MODULENAME "/type");

    FVM::UnknownQuantityHandler *u = eqsys->GetUnknownHandler();
    vector<len_t> *nontrivials = eqsys->GetNonTrivialUnknowns();
    TimeStepper *ts;
    switch (type) {
        case OptionConstants::TIMESTEPPER_TYPE_CONSTANT:
            ts = ConstructTimeStepper_constant(s, u);
            break;

        case OptionConstants::TIMESTEPPER_TYPE_ADAPTIVE:
            ts = ConstructTimeStepper_adaptive(s, u, nontrivials);
            break;

		case OptionConstants::TIMESTEPPER_TYPE_IONIZATION:
			ts = ConstructTimeStepper_ionization(s, u);
			break;

        default:
            throw SettingsException(
                "Unrecognized time stepper type: %d.", type
            );
    }

    eqsys->SetTimeStepper(ts);
}


/**
 * Construct a TimeStepperConstant object according to the
 * provided settings.
 *
 * s: Settings object specifying how to construct the
 *    TimeStepperConstant object.
 */
TimeStepperConstant *SimulationGenerator::ConstructTimeStepper_constant(Settings *s, FVM::UnknownQuantityHandler *u) {
    real_t tmax = s->GetReal(MODULENAME "/tmax");
    real_t dt   = s->GetReal(MODULENAME "/dt", false);
    int_t nt    = s->GetInteger(MODULENAME "/nt", false);
    int_t nSaveSteps = s->GetInteger(MODULENAME "/nsavesteps");

    bool dtset  = (dt > 0);
    bool ntset  = (nt > 0);

    if (dtset && ntset)
        throw SettingsException(
            "TimeStepper constant: "
            "Ambiguous time step specified. Only one of 'dt' and 'nt' "
            "may be set for the time stepper."
        );
    else if (!dtset && !ntset)
        throw SettingsException(
            "TimeStepper constant: "
            "No time step specified. Exactly one of 'dt' and 'nt' "
            "must be set for the time stepper."
        );

    // Generate object
    if (dtset) {
        s->MarkUsed(MODULENAME "/dt");
        return new TimeStepperConstant(tmax, dt, u, nSaveSteps);
    } else {
        s->MarkUsed(MODULENAME "/nt");
        return new TimeStepperConstant(tmax, (len_t)nt, u, nSaveSteps);
    }
}

/**
 * Construct a TimeStepperAdaptive object according to the
 * provided settings.
 *
 * s: Settings object specifying how to construct the
 *    TimeStepperAdaptive object.
 */
TimeStepperAdaptive *SimulationGenerator::ConstructTimeStepper_adaptive(
    Settings *s, FVM::UnknownQuantityHandler *u,
    vector<len_t> *nontrivials
) {
    int_t checkevery = s->GetInteger(MODULENAME "/checkevery");
    real_t tmax = s->GetReal(MODULENAME "/tmax");
    real_t dt = s->GetReal(MODULENAME "/dt");
    bool verbose = s->GetBool(MODULENAME "/verbose");
    bool conststep = s->GetBool(MODULENAME "/constantstep");

    if (dt == 0)
        dt = 1;

    ConvergenceChecker *cc = LoadToleranceSettings(
        MODULENAME, s, u, *nontrivials
    );

    return new TimeStepperAdaptive(tmax, dt, u, *nontrivials, cc, checkevery, verbose, conststep);
}

/**
 * Construct a TimeStepperIonization object according to the
 * provided settings.
 *
 * s: Settings object specifying how to construct the
 *    TimeStepperIonization object.
 */
TimeStepperIonization *SimulationGenerator::ConstructTimeStepper_ionization(
	Settings *s, FVM::UnknownQuantityHandler *u
) {
	real_t automaticstep = s->GetReal(MODULENAME "/automaticstep");
	real_t dt = s->GetReal(MODULENAME "/dt");
	real_t dtmax = s->GetReal(MODULENAME "/dtmax");
	real_t safetyfactor = s->GetReal(MODULENAME "/safetyfactor");
	real_t tmax = s->GetReal(MODULENAME "/tmax");

	if (dt < 0)
		throw SettingsException("TimeStepper ionization: Initial time step 'dt0' must be non-negative.");

	return new TimeStepperIonization(tmax, dt, dtmax, u, automaticstep, safetyfactor);
}

