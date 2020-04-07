#ifndef _DREAM_FVM_RADIAL_GRID_HPP
#define _DREAM_FVM_RADIAL_GRID_HPP

namespace DREAM::FVM { class RadialGrid; }

#include "FVM/FVMException.hpp"
#include "FVM/Grid/RadialGridGenerator.hpp"
#include <functional> 

namespace DREAM::FVM {
	class RadialGrid {
	private:
        len_t nr;

        // Radial grid
        // NOTE that 'r' has 'nr' elements, while
        // 'r_f' has (nr+1) elements.
        real_t *r=nullptr, *r_f=nullptr;
        // Radial grid steps
        //   dr[i]   = r_f[i+1] - r_f[i]   (nr elements)
        //   dr_f[i] = r[i+1] - r[i]       (nr-1 elements)
        real_t *dr=nullptr, *dr_f=nullptr;

        // Orbit-phase-space Jacobian factors
        real_t
            **Vp    = nullptr,    // Size NR x (N1*N2)
            **Vp_fr = nullptr,    // Size (NR+1) x (N1*N2)
            **Vp_f1 = nullptr,    // Size NR x ((N1+1)*N2)
            **Vp_f2 = nullptr;    // Size NR x (N1*N2)

        real_t 
            *volVp   = nullptr,   // spatial flux surface averaged jacobian, size nr
            *volVp_f = nullptr;
        
        // Flux-surface averaged quantities
        real_t 
            *effectivePassingFraction = nullptr, // Per's Eq (11.24)
            *magneticFieldMRS         = nullptr, // sqrt(<B^2>)
            *nabla_rSq_avg            = nullptr, // <|nabla r|^2>
            **xiBounceAverage_f1      = nullptr, // {xi} 
            **xiBounceAverage_f2      = nullptr, // {xi}
            **xi21MinusXi2OverB2_f1   = nullptr, // {xi^2(1-xi^2)*Bmin^2/B^2}
            **xi21MinusXi2OverB2_f2   = nullptr; // {xi^2(1-xi^2)*Bmin^2/B^2}
            
        

        // Magnetic field quantities
        len_t ntheta;            // Number of poloidal angle points
        real_t *theta=nullptr;   // Poloidal angle grid (size ntheta)
        real_t
            *B=nullptr,          // Magnetic field strength on r/theta grid (size nr*ntheta)
            *B_f=nullptr,        // Magnetic field strength on r_f/theta grid (size (nr+1)*ntheta)
            *Bmin=nullptr,       // Min B on flux surface (size nr)
            *Bmin_f=nullptr,     // Max B on flux surface (size nr)
            *Jacobian=nullptr,   // Jacobian on r/theta grid (size nr*ntheta)
            *Jacobian_f=nullptr; // Jacobian on r/theta flux grid (size (nr+1)*ntheta)

	protected:
        RadialGridGenerator *generator;

    public:
        RadialGrid(RadialGridGenerator*, const real_t t0=0);
        virtual ~RadialGrid();

        void DeallocateGrid();
        void DeallocateMagneticField();
        void DeallocateVprime();
        void DeallocateFSAvg();

        void Initialize(
            real_t *r, real_t *r_f,
            real_t *dr, real_t *dr_f
        ) {
            DeallocateGrid();

            this->r    = r;
            this->r_f  = r_f;
            this->dr   = dr;
            this->dr_f = dr_f;
        }
        void InitializeMagneticField(
            len_t ntheta, real_t *theta,
            real_t *B, real_t *B_f,
            real_t *Bmin, real_t *Bmin_f,
            real_t *Jacobian, real_t *Jacobian_f
        ) {
            DeallocateMagneticField();
            this->ntheta     = ntheta;
            this->theta      = theta;
            this->B          = B;
            this->B_f        = B_f;
            this->Bmin       = Bmin;
            this->Bmin_f     = Bmin_f;
            this->Jacobian   = Jacobian;
            this->Jacobian_f = Jacobian_f;
        }
        void InitializeVprime(
            real_t **Vp, real_t **Vp_fr,
            real_t **Vp_f1, real_t **Vp_f2
        ) {
            DeallocateVprime();

            this->Vp    = Vp;
            this->Vp_fr = Vp_fr;
            this->Vp_f1 = Vp_f1;
            this->Vp_f2 = Vp_f2;
        }

        void InitializeFSAvg(
            real_t *etf, real_t *sqrtB2avg,
            real_t **xiAvg_f1, real_t **xiAvg_f2,
            real_t **xi2B2Avg_f1, real_t **xi2B2Avg_f2,
            real_t *nabla_r2
            ) {
            DeallocateFSAvg();
            this->effectivePassingFraction = etf;
            this->magneticFieldMRS         = sqrtB2avg;
            this->nabla_rSq_avg            = nabla_r2;
            this->xiBounceAverage_f1       = xiAvg_f1;
            this->xiBounceAverage_f2       = xiAvg_f2;
            this->xi21MinusXi2OverB2_f1    = xi2B2Avg_f1;
            this->xi21MinusXi2OverB2_f2    = xi2B2Avg_f2;
        }

        

        bool Rebuild(const real_t);

        virtual void RebuildJacobians(MomentumGrid **momentumGrids)
        { this->generator->RebuildJacobians(this, momentumGrids); }
        
        virtual void RebuildFSAvgQuantities(MomentumGrid **momentumGrids)
        { this->generator->RebuildFSAvgQuantities(this, momentumGrids); }
        

        
        virtual real_t BounceAverageQuantity(RadialGrid *rGrid, const MomentumGrid* mg, len_t ir, len_t i, len_t j, len_t FluxGrid, std::function<real_t(real_t,real_t)> F)
        { return this->generator->BounceAverageQuantity(rGrid, mg, ir, i, j, FluxGrid,   F); }
        virtual real_t FluxSurfaceAverageQuantity(RadialGrid *rGrid, len_t ir, bool rFluxGrid, std::function<real_t(real_t)> F)
        { return this->generator->FluxSurfaceAverageQuantity(rGrid, ir, rFluxGrid, F); }
        
        

        // Get number of poloidal angle points
        const len_t   GetNTheta() const{return this->ntheta;}
        // Get theta (poloidal angle) grid
        const real_t *GetTheta() const { return this->theta; }
        // Evaluate magnetic field strength at all poloidal angles (on specified flux surface)
        const real_t *BOfTheta() const { return this->B; }
        const real_t *BOfTheta(const len_t ir) const { return this->B+(ir*ntheta); }
        const real_t *BOfTheta_f() const { return this->B_f; }
        const real_t *BOfTheta_f(const len_t ir) const { return this->B_f+(ir*ntheta); }

        const real_t *GetBmin() const {return this->Bmin;}
        const real_t *GetBmin_f() const {return this->Bmin_f;}
        const real_t  GetBmin(const len_t ir) const {return this->Bmin[ir];}
        const real_t  GetBmin_f(const len_t ir) const {return this->Bmin_f[ir];}
        const real_t *GetJacobian() const { return this->Jacobian; }
        const real_t *GetJacobian(const len_t ir) const { return this->Jacobian+(ir*ntheta); }
        const real_t *GetJacobian_f() const { return this->Jacobian_f; }
        const real_t *GetJacobian_f(const len_t ir) const { return this->Jacobian_f+(ir*ntheta); }
        


        // Returns the number of radial grid points in this grid
        len_t GetNr() const { return this->nr; }
        // Returns the vector containing all radial grid points
        const real_t *GetR() const { return this->r; }
        const real_t  GetR(const len_t i) const { return this->r[i]; }
        const real_t *GetR_f() const { return this->r_f; }
        const real_t  GetR_f(const len_t i) const { return this->r_f[i]; }

        // Returns a vector containing all radial steps
        const real_t *GetDr() const { return this->dr; }
        const real_t  GetDr(const len_t i) const { return this->dr[i]; }
        const real_t *GetDr_f() const { return this->dr_f; }
        const real_t  GetDr_f(const len_t i) const { return this->dr_f[i]; }
        
        real_t *const* GetVp() const { return this->Vp; }
        const real_t  *GetVp(const len_t ir) const { return this->Vp[ir]; }
        real_t *const* GetVp_fr() const { return this->Vp_fr; }
        const real_t  *GetVp_fr(const len_t ir) const { return this->Vp_fr[ir]; }
        real_t *const* GetVp_f1() const { return this->Vp_f1; }
        const real_t  *GetVp_f1(const len_t ir) const { return this->Vp_f1[ir]; }
        real_t *const* GetVp_f2() const { return this->Vp_f2; }
        const real_t  *GetVp_f2(const len_t ir) const { return this->Vp_f2[ir]; }

        const real_t *GetVolVp() const {return this->volVp;}
        const real_t GetVolVp(const len_t ir) {return this->volVp[ir];}
        const real_t *GetVolVp_f() const {return this->volVp_f;}
        const real_t GetVolVp_f(const len_t ir) {return this->volVp_f[ir];}
        
//        const bool    IsTrapped(len_t ir,real_t xi0);
        const real_t  *GetEffPassFrac() const { return this->effectivePassingFraction; }
        const real_t   GetEffPassFrac(const len_t ir) const { return this->effectivePassingFraction[ir]; }
        const real_t  *GetBMRS() const { return this->magneticFieldMRS; }
        const real_t   GetBMRS(const len_t ir) const { return this->magneticFieldMRS[ir]; }
        real_t *const* GetXiAvg_f1() const { return this->xiBounceAverage_f1; }
        const real_t  *GetXiAvg_f1(const len_t ir) const { return this->xiBounceAverage_f1[ir]; }
        real_t *const* GetXiAvg_f2() const { return this->xiBounceAverage_f2; }
        const real_t  *GetXiAvg_f2(const len_t ir) const { return this->xiBounceAverage_f2[ir]; }
        real_t *const* GetXi21MinusXi2OverB2Avg_f1() const { return this->xi21MinusXi2OverB2_f1; }
        const real_t  *GetXi21MinusXi2OverB2Avg_f1(const len_t ir) const { return this->xi21MinusXi2OverB2_f1[ir]; }
        real_t *const* GetXi21MinusXi2OverB2Avg_f2() const { return this->xi21MinusXi2OverB2_f2; }
        const real_t  *GetXi21MinusXi2OverB2Avg_f2(const len_t ir) const { return this->xi21MinusXi2OverB2_f2[ir]; }
        
        
        bool NeedsRebuild(const real_t t) const { return this->generator->NeedsRebuild(t); }

        /*len_t GetNCells() const;
        void SetMomentumGrid(const len_t i, MomentumGrid *m, const real_t t0=0);
        void SetAllMomentumGrids(MomentumGrid*, const real_t t0=0);*/
	};

    class RadialGridException : public FVMException {
    public:
        template<typename ... Args>
        RadialGridException(const std::string &msg, Args&& ... args)
            : FVMException(msg, std::forward<Args>(args) ...) {
            AddModule("RadialGrid");
        }
    };
}

#endif/*_DREAM_FVM_RADIAL_GRID_HPP*/
