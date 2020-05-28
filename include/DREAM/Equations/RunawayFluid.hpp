#ifndef _DREAM_EQUATIONS_RUNAWAY_FLUID_HPP
#define _DREAM_EQUATIONS_RUNAWAY_FLUID_HPP

namespace DREAM { class RunawayFluid; }

#include "DREAM/Equations/SlowingDownFrequency.hpp"
#include "DREAM/Equations/PitchScatterFrequency.hpp"
#include <algorithm>
#include <gsl/gsl_integration.h>
#include <gsl/gsl_roots.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_min.h>

namespace DREAM {
    class RunawayFluid{
    private:
        const real_t constPreFactor = 4*M_PI
                                *Constants::r0*Constants::r0
                                *Constants::c;

        FVM::RadialGrid *rGrid;
        FVM::UnknownQuantityHandler *unknowns;
        SlowingDownFrequency *nuS;
        PitchScatterFrequency *nuD;
        CoulombLogarithm *lnLambdaEE;
        len_t nr;
        CollisionQuantity::collqty_settings *collQtySettings;

        gsl_integration_workspace *gsl_ad_w;
        const gsl_root_fsolver_type *GSL_rootsolver_type;
        gsl_root_fsolver *fsolve;
        const gsl_min_fminimizer_type *fmin_type;
        gsl_min_fminimizer *fmin;


        len_t id_ncold;
        len_t id_ntot;
        len_t id_Tcold;
        len_t id_Eterm;

        real_t *ncold;
        real_t *ntot;
        real_t *Tcold;
        real_t *Eterm;

        real_t *Ec_free=nullptr;                // Connor-Hastie field with only bound
        real_t *Ec_tot=nullptr;                 // Connor-Hastie field with free+bound
        real_t *EDreic=nullptr;                 // Dreicer field
        real_t *criticalREMomentum=nullptr;     // Critical momentum for runaway p_star 
        real_t *pc_COMPLETESCREENING = nullptr;
        real_t *pc_NOSCREENING = nullptr;
        real_t *avalancheGrowthRate=nullptr;    // (dnRE/dt)_ava = nRE*Gamma_ava
        real_t *tritiumRate=nullptr;            // (dnRE/dt)_Tritium
        real_t *comptonRate=nullptr;            // (dnRE/dt)_Compton
        real_t *effectiveCriticalField=nullptr; // Eceff: Gamma_ava(Eceff) = 0

        //real_t *exactPitchDistNormalization;
        //real_t *approximatePitchDistNormalization;

       


        bool gridRebuilt;
        void AllocateQuantities();
        void DeallocateQuantities();
        //void AllocateGSLWorkspaces();
        //void FreeGSLWorkspaces();

        void CalculateDerivedQuantities();
        void CalculateEffectiveCriticalField(bool useApproximateMethod);
        void CalculateCriticalMomentum();
        void CalculateGrowthRates();


        static void FindECritInterval(len_t ir, real_t *E_lower, real_t *E_upper, void *par);
        static void FindPExInterval(real_t *p_ex_guess, real_t *p_ex_lower, real_t *p_ex_upper, void *par, real_t p_upper_threshold);
        static void FindRoot(real_t x_lower, real_t x_upper, real_t *root, gsl_function gsl_func, gsl_root_fsolver *s);
        static void FindInterval(real_t *x_lower, real_t *x_upper, gsl_function gsl_func );
//        void CalculateDistributionNormalizationFactors();

        real_t BounceAverageFunc(len_t ir, std::function<real_t(real_t,real_t)> Func);

        static real_t FindUExtremumAtE(real_t Eterm, void *par);
        static real_t evaluateNegUAtP(real_t p, void *par);
        static real_t evaluateApproximateUAtP(real_t p, void *par);
        static real_t UAtPFunc(real_t p, void *par);
        
        
        static real_t pStarFunction(real_t, void *);
        real_t evaluateBarNuSNuDAtP(len_t ir, real_t p){real_t p2=p*p; 
                return nuS->evaluateAtP(ir,p,collQtySettings->collfreq_type, OptionConstants::COLLQTY_COLLISION_FREQUENCY_MODE_SUPERTHERMAL)*nuD->evaluateAtP(ir,p,collQtySettings->collfreq_type, OptionConstants::COLLQTY_COLLISION_FREQUENCY_MODE_SUPERTHERMAL)*p2*p2*p2/(sqrt(1+p2)*(1+p2));}
        
    protected:
    public:
        RunawayFluid(FVM::Grid *g, FVM::UnknownQuantityHandler *u, SlowingDownFrequency *nuS, 
        PitchScatterFrequency *nuD, CoulombLogarithm *lnLEE, CollisionQuantity::collqty_settings *cqs);
        ~RunawayFluid();

        real_t testEvalU(len_t ir, real_t p, real_t Eterm, bool useApproximateMethod);

        real_t evaluateAnalyticPitchDistribution(len_t ir, real_t xi0, real_t p, real_t Eterm, gsl_integration_workspace *gsl_ad_w);
        real_t evaluateApproximatePitchDistribution(len_t ir, real_t xi0, real_t p, real_t Eterm);
        static real_t evaluateTritiumRate(real_t gamma_c);
        static real_t evaluateComptonRate(real_t pc,gsl_integration_workspace *gsl_ad_w);
        static real_t evaluateComptonPhotonFluxSpectrum(real_t Eg);
        static real_t evaluateComptonTotalCrossSectionAtP(real_t Eg, real_t pc);


        void Rebuild(bool useApproximateMethod);
        void GridRebuilt(){gridRebuilt = true;}
        const real_t GetEffectiveCriticalField(len_t ir) const
            {return effectiveCriticalField[ir];}
        const real_t* GetEffectiveCriticalField() const
            {return effectiveCriticalField;}
        
        const real_t GetDreicerElectricField(len_t ir) const
            {return EDreic[ir];}
        const real_t* GetDreicerElectricField() const
            {return EDreic;}
        
        const real_t GetConnorHastieField_COMPLETESCREENING(len_t ir) const
            {return Ec_free[ir];}
        const real_t* GetConnorHastieField_COMPLETESCREENING() const
            {return Ec_free;}
        
        const real_t GetConnorHastieField_NOSCREENING(len_t ir) const
            {return Ec_tot[ir];}
        const real_t* GetConnorHastieField_NOSCREENING() const
            {return Ec_tot;}
        
        

        const real_t GetAvalancheGrowthRate(len_t ir) const
            {return avalancheGrowthRate[ir];}
        const real_t* GetAvalancheGrowthRate() const
            {return avalancheGrowthRate;}
        
        const real_t GetTritiumRunawayRate(len_t ir) const
            {return tritiumRate[ir];}
        const real_t* GetTritiumRunawayRate() const
            {return tritiumRate;}
        
        const real_t GetComptonRunawayRate(len_t ir) const
            {return comptonRate[ir];}
        const real_t* GetComptonRunawayRate() const
            {return comptonRate;}

        const real_t GetEffectiveCriticalRunawayMomentum(len_t ir) const
            {return criticalREMomentum[ir];}
        const real_t* GetEffectiveCriticalRunawayMomentum() const
            {return criticalREMomentum;}
        

        const CollisionQuantity::collqty_settings *GetSettings() const{return collQtySettings;}
    };

}


#endif/*_DREAM_EQUATIONS_RUNAWAY_FLUID_HPP*/

    


