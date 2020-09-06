#ifndef _DREAM_TRANSPORT_PRESCRIBED_HPP
#define _DREAM_TRANSPORT_PRESCRIBED_HPP

#include "FVM/Equation/AdvectionTerm.hpp"
#include "FVM/Equation/DiffusionTerm.hpp"
#include "FVM/Interpolator1D.hpp"
#include "FVM/Interpolator3D.hpp"

namespace DREAM {
    template<typename T>
    class TransportPrescribed : public T {
    private:
        // Diffusion coefficient data, prescribed in time and
        // on the given computational grid.
        FVM::Interpolator1D *prescribedCoeff=nullptr;

        len_t nt, nr, np1, np2;
        const real_t **coeff, *t, *r, *p1, *p2;
        enum FVM::Interpolator3D::momentumgrid_type
            momtype,    // Type of momentum grid used for 'coeff'
            gridtype;   // Type of momentum grid used for 'grid'
        enum FVM::Interpolator3D::interp_method interpmethod = FVM::Interpolator3D::INTERP_LINEAR;

    public:
        TransportPrescribed<T>(
            FVM::Grid*,
            const len_t, const len_t, const len_t, const len_t,
            const real_t**, const real_t*, const real_t*,
            const real_t*, const real_t*, enum FVM::Interpolator3D::momentumgrid_type,
            enum FVM::Interpolator3D::momentumgrid_type,
            enum FVM::Interpolator3D::interp_method interpmethod=FVM::Interpolator3D::INTERP_LINEAR,
            bool allocCoefficients=false
        );
        virtual ~TransportPrescribed<T>();

        void InterpolateCoefficient();

        virtual bool GridRebuilt() override;
        virtual void Rebuild(const real_t, const real_t, FVM::UnknownQuantityHandler*) override;
    };

    // Typedefs
    typedef TransportPrescribed<FVM::AdvectionTerm> TransportPrescribedAdvective;
    typedef TransportPrescribed<FVM::DiffusionTerm> TransportPrescribedDiffusive;
}

#include "TransportPrescribed.tcc"

#endif/*_DREAM_TRANSPORT_PRESCRIBED_HPP*/
