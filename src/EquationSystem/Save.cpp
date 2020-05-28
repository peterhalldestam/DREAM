/**
 * Routines for saving things from the EquationSystem object.
 */

#include <string>
#include <softlib/SFile.h>
#include "DREAM/EquationSystem.hpp"


using namespace DREAM;
using namespace std;


/**
 * Save charge and species names for ions.
 *
 * sf:   SFile object to write data to.
 * path: Path to group which the data should be written to.
 *       Note that group must exist in the file.
 */
void EquationSystem::SaveIonMetaData(SFile *sf, const string& path) {
    string group;
    if (path.back() == '/')
        group = path;
    else
        group = path + "/";

    // Get list of charges
    const len_t nZ = ionHandler->GetNZ();
    const len_t *Z = ionHandler->GetZs();
    sf->WriteList(group + "Z", Z, nZ);

    // Construct name vector
    const vector<string> namelist = ionHandler->GetNameList();

    string names = "";
    for (auto it = namelist.begin(); it != namelist.end(); it++)
        names += *it + ";";

    sf->WriteString(group + "names", names);
}

/**
 * Save time, radial and momentum grids as raw vectors
 * to the given SFile object.
 *
 * sf:   SFile object to write grids to.
 * path: Path to group which the data should be written to.
 *       Note that the group must exist in the file.
 */
void EquationSystem::SaveGrids(SFile *sf, const string& path) {
    string group;

    if (path.back() == '/')
        group = path;
    else
        group = path + "/";

    // Time grid
    const real_t *t = this->times.data();
    sf->WriteList(group + "t", t, this->times.size());

    // Radial grid
    const real_t *r   = this->fluidGrid->GetRadialGrid()->GetR();
    const real_t *r_f = this->fluidGrid->GetRadialGrid()->GetR_f();
    const real_t *dr  = this->fluidGrid->GetRadialGrid()->GetDr();
    sf->WriteList(group + "r", r, this->fluidGrid->GetNr());
    sf->WriteList(group + "r_f", r_f, this->fluidGrid->GetNr()+1);
    sf->WriteList(group + "dr", dr, this->fluidGrid->GetNr());

    // Volume elements
    const real_t *VpVol = this->fluidGrid->GetVpVol();
    sf->WriteList(group + "Vprime", VpVol, this->fluidGrid->GetNr());

    // Hot-tail grid
    if (this->hottailGrid != nullptr) {
        sf->CreateStruct(group + "hottail");
        SaveMomentumGrid(sf, group + "hottail/", this->hottailGrid, this->hottailGrid_type);
    }

    // Runaway grid
    if (this->runawayGrid != nullptr) {
        sf->CreateStruct(group + "hottail");
        SaveMomentumGrid(sf, group + "hottail/", this->runawayGrid, this->runawayGrid_type);
    }
}

/**
 * XXX Here we assume that all momentum grids are the same
 * Save a momentum grid to the given SFile object.
 *
 * sf:       SFile object to write grids to.
 * gridpath: Full path to grid data in output file.
 * g:        Grid to save.
 */
void EquationSystem::SaveMomentumGrid(
    SFile *sf, const string& gridname, FVM::Grid *g,
    enum OptionConstants::momentumgrid_type tp
) {
    const real_t *p1   = g->GetMomentumGrid(0)->GetP1();
    const real_t *p2   = g->GetMomentumGrid(0)->GetP2();
    const real_t *p1_f = g->GetMomentumGrid(0)->GetP1_f();
    const real_t *p2_f = g->GetMomentumGrid(0)->GetP2_f();
    const real_t *dp1  = g->GetMomentumGrid(0)->GetDp1();
    const real_t *dp2  = g->GetMomentumGrid(0)->GetDp2();
    const len_t np1    = g->GetMomentumGrid(0)->GetNp1();
    const len_t np2    = g->GetMomentumGrid(0)->GetNp2();
    const len_t nr     = g->GetRadialGrid()->GetNr();

    // Write grid type
    sf->WriteInt32List(gridname + "type", (int32_t*)&tp, 1);

    // Grid coordinates
    sf->WriteList(gridname + "p1", p1, np1);
    sf->WriteList(gridname + "p2", p2, np2);

    // Flux grid coordinates
    sf->WriteList(gridname + "p1_f", p1_f, np1+1);
    sf->WriteList(gridname + "p2_f", p2_f, np2+1);

    // Grid cell sizes
    sf->WriteList(gridname + "dp1", dp1, np1);
    sf->WriteList(gridname + "dp2", dp2, np2);

    // Grid volumes
    const real_t *const* Vp = g->GetVp();
    WriteCopyArray(sf, gridname + "Vprime", Vp, nr, np1*np2);
}

/**
 * Save the given array to the specified SFile with the
 * given name. The array is copied before writing so that
 * 'v' need not be stored contiguously in memory.
 *
 * sf:   SFile object to write array to.
 * name: Name of variable in file.
 * v:    2D array data to write.
 * m:    Length of first dimension of 'v'.
 * n:    Length of second dimension of 'v'.
 */
void EquationSystem::WriteCopyArray(
    SFile *sf, const string& name, const real_t *const* v,
    const len_t m, const len_t n
) {
    // First, copy array to make it contiguous
    real_t **t = new real_t*[m];
    t[0] = new real_t[m*n];

    for (len_t i = 1; i < m; i++)
        t[i] = t[i-1] + n;

    for (len_t i = 0; i < m; i++)
        for (len_t j = 0; j < n; j++)
            t[i][j] = v[i][j];

    sf->WriteArray(name, t, m, n);
    
    delete [] t[0];
    delete [] t;
}

