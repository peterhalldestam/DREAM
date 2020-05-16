#include "DREAM/Equations/CollisionFrequency.hpp"
#include "DREAM/Constants.hpp"
#include "DREAM/Settings/OptionConstants.hpp"
#include "FVM/UnknownQuantityHandler.hpp"
#include "DREAM/NotImplementedException.hpp"
#include "FVM/FVMException.hpp"
#include <string>

using namespace DREAM;

CollisionFrequency::CollisionFrequency(FVM::Grid *g, FVM::UnknownQuantityHandler *u, IonHandler *ih,  
                CoulombLogarithm *lnLee, CoulombLogarithm *lnLei,
                enum OptionConstants::momentumgrid_type mgtype,  struct CollisionQuantityHandler::collqtyhand_settings *cqset)
                : CollisionQuantity(g,u,ih,mgtype,cqset) {
    lnLambdaEE = lnLee;
    lnLambdaEI = lnLei;
    
}

void CollisionFrequency::RebuildPlasmaDependentTerms(){
    nbound = ionHandler->evaluateBoundElectronDensityFromQuasiNeutrality(nbound);
    len_t indZ;
    for(len_t iz = 0; iz<nZ; iz++)
        for(len_t Z0=0; Z0<=Zs[iz]; Z0++)
            for(len_t ir=0; ir<nr; ir++){
                indZ = ionIndex[iz][Z0];            
                ionDensities[ir][indZ] = ionHandler->GetIonDensity(ir,iz,Z0);
            }
            

    if(collQtySettings->collfreq_mode==OptionConstants::COLLQTY_COLLISION_FREQUENCY_MODE_FULL)
        InitializeGSLWorkspace();
    if (!buildOnlyF1F2){
        setNColdTerm(nColdTerm,mg->GetP(),nr,np1,np2_store);
        setNColdTerm(nColdTerm_fr,mg->GetP(),nr+1,np1,np2_store);
    }
    setNColdTerm(nColdTerm_f1,mg->GetP_f1(),nr,np1+1,np2_store);
    setNColdTerm(nColdTerm_f2,mg->GetP_f2(),nr,np1,np2_store+1);
}


/**
 * Rebuilds partial contributions that only depend on the grid. If P-Xi grid, only store momentum
 * dependent quantities on size np1 array. 
 */
void CollisionFrequency::RebuildConstantTerms(){
    const len_t *ZAtomicCharge = ionHandler->GetZs();
    len_t indZ;
    for(len_t iz = 0; iz<nZ; iz++){
        Zs[iz] = ZAtomicCharge[iz];
        for(len_t Z0=0; Z0<=Zs[iz]; Z0++){
            indZ = ionHandler->GetIndex(iz,Z0);
            ionIndex[iz][Z0] = indZ; 
        }
    }
    len_t ind;
    for(len_t iZ = 0; iZ<nZ; iZ++){
        for(len_t Z0=0; Z0<=Zs[iZ]-1; Z0++){
            ind = ionIndex[iZ][Z0];
            atomicParameter[ind] = GetAtomicParameter(iZ,Z0);
        }
    }

    if (!buildOnlyF1F2){
        setPreFactor(preFactor,mg->GetP(),np1,np2_store);
        setPreFactor(preFactor_fr,mg->GetP(),np1,np2_store);
        setIonTerm(ionTerm,mg->GetP(),np1,np2_store);
        setIonTerm(ionTerm_fr,mg->GetP(),np1,np2_store);
        if(isPartiallyScreened){
            setScreenedTerm(screenedTerm,mg->GetP(),np1,np2_store);
            setScreenedTerm(screenedTerm_fr,mg->GetP(),np1,np2_store);
        }
    }
    setPreFactor(preFactor_f1,mg->GetP_f1(),np1+1,np2_store);
    setPreFactor(preFactor_f2,mg->GetP_f2(),np1,np2_store+1);
    setIonTerm(ionTerm_f1,mg->GetP_f1(),np1+1,np2_store);
    setIonTerm(ionTerm_f2,mg->GetP_f2(),np1,np2_store+1);
    if(isPartiallyScreened){
        setScreenedTerm(screenedTerm_f1,mg->GetP_f1(),np1+1,np2_store);
        setScreenedTerm(screenedTerm_f2,mg->GetP_f2(),np1,np2_store+1);
    }
    if(isNonlinear)
        calculateIsotropicNonlinearOperatorMatrix();

}

/**
 * Calculates and stores the momentum-dependent prefactor to the collision frequency.
 */
void CollisionFrequency::setPreFactor(real_t *&preFactor, const real_t *pIn, len_t np1, len_t np2){
    real_t p, PF;
    len_t ind;
    for(len_t i = 0; i<np1; i++){
        for (len_t j = 0; j<np2; j++){
            ind = np1*j+i;
            p = pIn[ind];
            if(p==0)
                PF = ReallyLargeNumber; 
            else
                PF = evaluatePreFactorAtP(p);
            preFactor[ind] = PF;
        }
    }
}

/**
 * Assembles collision frequency on one of the grids.
 */
void CollisionFrequency::AssembleQuantity(real_t **&collisionQuantity, len_t nr, len_t np1, len_t np2, len_t fluxGridType){
    real_t *nColdContribution = new real_t[nr*np1*np2];
    real_t *niContribution = new real_t[nzs*nr*np1*np2];
    real_t collQty;
    real_t *ncold = unknowns->GetUnknownData(id_ncold);
    const len_t *Zs = ionHandler->GetZs();
    nColdContribution = GetUnknownPartialContribution(id_ncold,fluxGridType,nColdContribution);
    niContribution    = GetUnknownPartialContribution(id_ni,   fluxGridType,niContribution);

    len_t indZ;
    for(len_t ir=0; ir<nr; ir++){
        for(len_t j=0; j<np2; j++){
            for(len_t i=0; i<np1; i++){
                collQty = ncold[ir]*nColdContribution[np1*np2*ir + np1*j + i];
                for(len_t iz = 0; iz<nZ; iz++){
                    for(len_t Z0=0; Z0<=Zs[iz]; Z0++){
                        indZ = ionIndex[iz][Z0];
                        collQty += ionDensities[ir][indZ]*niContribution[indZ*nr*np1*np2 + np1*np2*ir + np1*j + i];
                    }
                }
                collisionQuantity[ir][j*np1+i] = collQty; 
            }
        }
    }
    delete [] nColdContribution;
    delete [] niContribution;
}


real_t *CollisionFrequency::GetUnknownPartialContribution(len_t id_unknown, len_t fluxGridMode, real_t *&partQty){
    if(id_unknown == id_ncold)
        GetNColdPartialContribution(fluxGridMode,partQty);
    else if(id_unknown == id_ni)
        GetNiPartialContribution(fluxGridMode,partQty);
    else if(id_unknown == id_fhot){
        if(!( (fluxGridMode==2)&&(np2=1)&&(isPXiGrid) ) )
            throw FVM::FVMException("Nonlinear contribution to collision frequencies is only implemented for hot-tails, with p-xi grid and np2=1 and evaluated on the p flux grid.");
        GetNonlinearPartialContribution(partQty);
    } else
        throw FVM::FVMException("Invalid id_unknown: %s does not contribute to the collision frequencies",unknowns->GetUnknown(id_unknown)->GetName());
    

    return partQty;
}



void CollisionFrequency::AddNonlinearContribution(){
    real_t *fHot = unknowns->GetUnknownData(id_fhot);
    real_t *fHotContribution = new real_t[nr*np1*(np1+1)];
    GetNonlinearPartialContribution(fHotContribution);
    for (len_t ir=0;ir<nr;ir++)
        for(len_t i=0; i<np1+1; i++)
            for(len_t ip=0; ip<np1; ip++)
                collisionQuantity_f1[ir][i] += fHotContribution[ip*(np1+1)*nr + ir*(np1+1) + i] * fHot[np1*ir+ip];
}



void CollisionFrequency::setIonTerm(real_t *&ionTerm, const real_t *pIn, len_t np1, len_t np2){
    
    real_t p;
    len_t ind, pind;
    for(len_t i = 0; i<np1; i++){
        for (len_t j = 0; j<np2; j++){
            pind = np1*j+i;
            p = pIn[pind];
            for(len_t iz = 0; iz<nZ; iz++){
                for(len_t Z0=0; Z0<=Zs[iz]; Z0++){
                    ind = ionIndex[iz][Z0];
                    ionTerm[ind*np1*np2 + pind] = evaluateIonTermAtP(iz,Z0,p);
                }
            }
        }
    }
}

void CollisionFrequency::setScreenedTerm(real_t *&screenedTerm, const real_t *pIn, len_t np1, len_t np2){

    real_t p;
    len_t ind, pind;
    for(len_t i = 0; i<np1; i++){
        for (len_t j = 0; j<np2; j++){
            pind = np1*j+i;
            p = pIn[pind];
            for(len_t iz = 0; iz<nZ; iz++){
                for(len_t Z0=0; Z0<=Zs[iz]; Z0++){
                    ind = ionIndex[iz][Z0];
                    screenedTerm[ind*np1*np2 + pind] = evaluateScreenedTermAtP(iz,Z0,p);
                }
            }
        }
    }
}

void CollisionFrequency::setNColdTerm(real_t **&nColdTerm, const real_t *pIn, len_t nr, len_t np1, len_t np2){
    real_t p;
    len_t pind;
    // Depending on setting, set nu_s to superthermal or full formula (with maxwellian)
    if (collQtySettings->collfreq_mode==OptionConstants::COLLQTY_COLLISION_FREQUENCY_MODE_SUPERTHERMAL) {
        for(len_t i=0;i<np1;i++){
            for(len_t j=0;j<np2;j++){
                pind = np1*j+i;
                p = pIn[pind];
                for(len_t ir=0; ir<nr; ir++)
                    nColdTerm[ir][pind] = evaluateElectronTermAtP(ir,p);
            }
        }
    } 
}






real_t CollisionFrequency::psi0Integrand(real_t x, void *params){
    real_t gamma = *(real_t *) params;
    return 1/sqrt( (x+gamma)*(x+gamma)-1 );
} 
real_t CollisionFrequency::psi1Integrand(real_t x, void *params){
    real_t gamma = *(real_t *) params;
    return (x+gamma)/sqrt((x+gamma)*(x+gamma)-1); // integrated with weight w(x) = exp(-(x-gamma)/Theta) 
} 
/** 
 * Evaluates integral appearing in relativistic test-particle operator
 * Psi0 = int_0^p exp( -(sqrt(1+s^2)-1)/Theta) / sqrt(1+s^2) ds;
 */
real_t CollisionFrequency::evaluatePsi0(len_t ir, real_t p) {
    real_t gamma = sqrt(1+p*p);
    real_t *T_cold = unknowns->GetUnknownData(id_Tcold);
    
    gsl_function F;
    F.function = &(CollisionFrequency::psi0Integrand); 
    F.params = &gamma;
    real_t psi0int; 
    gsl_integration_fixed(&F, &psi0int, gsl_w[ir]);

    
    real_t Theta = T_cold[ir] / Constants::mc2inEV;
    return evaluateExp1OverThetaK(Theta,0) - exp( -(gamma-1)/Theta ) * psi0int;

}
real_t CollisionFrequency::evaluatePsi1(len_t ir, real_t p) {
    
    real_t *T_cold = unknowns->GetUnknownData(id_Tcold);
    real_t gamma = sqrt(1+p*p);
    gsl_function F;
    F.function = &(CollisionFrequency::psi1Integrand); 
    F.params = &gamma;
    real_t psi1int; 
    gsl_integration_fixed(&F, &psi1int, gsl_w[ir]);

    real_t Theta = T_cold[ir] / Constants::mc2inEV;
    return evaluateExp1OverThetaK(Theta,1) - exp( -(gamma-1)/Theta ) * psi1int;
    

}


real_t CollisionFrequency::evaluateExp1OverThetaK(real_t Theta, real_t n) {
    real_t ThetaThreshold = 0.002;
    /**
     * Since cyl_bessel_k ~ exp(-1/Theta), for small Theta you get precision issues.
     * Instead using asymptotic expansion for bessel_k for small Theta.
     */
    if (Theta > ThetaThreshold)
        return exp(1/Theta)*std::cyl_bessel_k(n,1/Theta);
    else {
//        return sqrt(M_PI*Theta/2)*(1 + 15*Theta/8 + 105*Theta*Theta/128 - 945*Theta*Theta*Theta/3072);
        real_t n2 = n*n;
        return sqrt(M_PI*Theta/2)*(1 + (4*n2-1)/8 * Theta + (4*n2-1)*(4*n2-9)*Theta*Theta/128 + (4*n2-1)*(4*n2-9)*(4*n2-25)*Theta*Theta*Theta/3072);
    }
}



/**
 * Allocates quantities which will be used in the calculation of the collision frequency.
 */
void CollisionFrequency::AllocatePartialQuantities(){
    
    DeallocatePartialQuantities();
    InitializeGSLWorkspace();
    nbound = new real_t[nr];
    Zs = new real_t[nZ];
    ionIndex = new real_t*[nZ];
    ionDensities = new real_t*[nr];
    for(len_t iz=0;iz<nZ;iz++)
        ionIndex[iz] = new real_t[ionHandler->GetZ(iz)+1];
    for(len_t ir=0; ir<nr;ir++)
        ionDensities[ir] = new real_t[nzs];

    if(!buildOnlyF1F2){
        preFactor    = new real_t[np1*np2_store];
        preFactor_fr = new real_t[np1*np2_store];
        ionTerm = new real_t[nzs*np1*np2_store];
        ionTerm_fr = new real_t[nzs*np1*np2_store];
    }
    preFactor_f1 = new real_t[(np1+1)*np2_store];
    preFactor_f2 = new real_t[np1*(np2_store+1)];
    ionTerm_f1 = new real_t[nzs*(np1+1)*np2_store];
    ionTerm_f2 = new real_t[nzs*np1*(np2_store+1)];
        
    if(!buildOnlyF1F2){
        if(isPartiallyScreened){
            screenedTerm    = new real_t[nzs*np1*np2_store];
            screenedTerm_fr = new real_t[nzs*np1*np2_store];
        }
        nColdTerm = new real_t*[nr];
        nColdTerm_fr = new real_t*[nr+1];
        for(len_t ir=0;ir<nr;ir++)
            nColdTerm[ir] = new real_t[np1*np2_store];
        for(len_t ir=0;ir<nr+1;ir++)
            nColdTerm_fr[ir] = new real_t[np1*np2_store];
    }
        
        
    if(isPartiallyScreened){
        screenedTerm_f1 = new real_t[nzs*(np1+1)*np2_store];
        screenedTerm_f2 = new real_t[nzs*np1*(np2_store+1)];
    }

    nColdTerm_f1 = new real_t*[nr];
    nColdTerm_f2 = new real_t*[nr];
    for(len_t ir=0;ir<nr;ir++){
        nColdTerm_f1[ir] = new real_t[(np1+1)*np2_store];
        nColdTerm_f2[ir] = new real_t[np1*(np2_store+1)];
    }
    atomicParameter = new real_t[nzs];


    if (isNonlinear){
        nonlinearMat = new real_t*[np1+1]; // multiply matrix by f lnLc to get p*nu_s on p flux grid
        for (len_t i = 0; i<np1+1; i++){
            nonlinearMat[i] = new real_t[np1];
        }
        const real_t *p = mg->GetP1();
        trapzWeights = new real_t[np1];
        for (len_t i = 1; i<np1-1; i++){
            trapzWeights[i] = (p[i+1]-p[i-1])/2;
        }
        nonlinearWeights = new real_t[np1];
    }
    
}


void CollisionFrequency::DeallocatePartialQuantities(){
    DeallocateGSL();
    if (nbound != nullptr){
        delete [] nbound;
        delete [] Zs;
        for(len_t iz=0; iz<nZ; iz++)
            delete [] ionIndex[iz];
        for(len_t ir=0; ir<nr;ir++)
            delete [] ionDensities[ir];
        
        delete [] ionIndex;
        delete [] ionDensities; 
    }
    if(preFactor!=nullptr){
        delete [] preFactor;
        delete [] preFactor_fr;
        delete [] ionTerm;
        delete [] ionTerm_fr;
    }   
    if(preFactor_f1 != nullptr){
        delete [] preFactor_f1;
        delete [] preFactor_f2;
        delete [] ionTerm_f1;
        delete [] ionTerm_f2;
    }

    if(!buildOnlyF1F2){
        if(nColdTerm != nullptr){
            for(len_t ir=0;ir<nr;ir++)
                delete [] nColdTerm[ir];
            for(len_t ir=0;ir<nr+1;ir++)
                delete [] nColdTerm_fr[ir];
            delete [] nColdTerm;
            delete [] nColdTerm_fr;
        }
        if (screenedTerm != nullptr){
            delete [] screenedTerm;
            delete [] screenedTerm_fr;            
        }
    }
    if (screenedTerm_f1 != nullptr){
        delete [] screenedTerm_f1;
        delete [] screenedTerm_f2;
    }

    if(nColdTerm_f1 != nullptr){
        for(len_t ir=0;ir<nr;ir++){
            delete [] nColdTerm_f1[ir];
            delete [] nColdTerm_f2[ir];
        }
        delete [] nColdTerm_f1;
        delete [] nColdTerm_f2;
    }
    if(atomicParameter != nullptr)
        delete [] atomicParameter;

    if(nonlinearMat != nullptr){
        for(len_t i = 0; i<np1+1;i++){
            delete [] nonlinearMat[i];
        }
        delete [] nonlinearMat;
        delete [] trapzWeights;
        delete [] nonlinearWeights;
    }
}


/**
 * Initializes a GSL workspace for each radius (used for relativistic test particle operator evaluation),
 * using a T_cold-dependent fixed quadrature. 
 */
void CollisionFrequency::InitializeGSLWorkspace(){
 /** 
  * (consider using a single regular dynamic quadrature instead as the integral is somewhat tricky, 
  * since in the limit p/mc -> 0 the integral is sharply peaked at p_min -- goes as int 1/sqrt(x) dx,0,inf --
  * and may be challenging to resolve using a fixed point quadrature)
  */
    DeallocateGSL();
    real_t *T_cold = unknowns->GetUnknownData(id_Tcold);
    
    gsl_w = new gsl_integration_fixed_workspace*[nr];
    const real_t lowerLim = 0; // integrate from 0 to inf
    const gsl_integration_fixed_type *T = gsl_integration_fixed_laguerre;
    const len_t Npoints = 20; // play around with this number -- may require larger, or even sufficient with lower
    const real_t alpha = 0.0;
    real_t b;
    real_t Theta;
    for (len_t ir = 0; ir<nr; ir++){
        Theta = T_cold[ir]/Constants::mc2inEV;
        b = 1/Theta;
        gsl_w[ir] = gsl_integration_fixed_alloc(T, Npoints, lowerLim, b, alpha, 0.0);
    }
}


void CollisionFrequency::DeallocateGSL(){
    if (this->gsl_w == nullptr)
        return;

    for (len_t ir=0; ir<this->nr; ir++)
        gsl_integration_fixed_free(gsl_w[ir]);
}


