#ifndef _DREAM_FVM_GRID_HPP
#define _DREAM_FVM_GRID_HPP

namespace DREAM::FVM {
    class Grid {
    private:
    protected:
        RadialGrid *rgrid;
		MomentumGrid **momentumGrids;
    public:
        Grid(RadialGridGenerator*, MomentumGridGenerator*, const real_t t0=0);
        ~Grid();

        // Returns pointer to the momentum grid with the specified index
        MomentumGrid *GetMomentumGrid(const len_t i) { return this->momentumGrids[i]; }
        RadialGrid *GetRadialGrid() { return this->rgrid; }

        const len_t GetNCells() const;
        const len_t GetNr() const { return this->rgrid->GetNr(); }

        real_t *const* GetVp() const { return this->rgrid->Vp; }
        const real_t *GetVp(const len_t ir) const { return this->rgrid->Vp[ir]; }
        real_t *const* GetVp_fr() const { return this->rgrid->Vp_fr; }
        const real_t *GetVp_fr(const len_t ir) const { return this->rgrid->Vp_fr[ir]; }
        real_t *const* GetVp_f1() const { return this->rgrid->Vp_f1; }
        const real_t *GetVp_f1(const len_t ir) const { return this->rgrid->Vp_f1[ir]; }
        real_t *const* GetVp_f2() const { return this->rgrid->Vp_f2; }
        const real_t *GetVp_f2(const len_t ir) const { return this->rgrid->Vp_f2[ir]; }

        bool Rebuild(const real_t);
        void RebuildJacobians() { this->rgrid->RebuildJacobians(this); }
    };
}

#endif/*_DREAM_FVM_GRID_HPP*/
