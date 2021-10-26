/*
 * (C) Copyright 2021 UK Met Office
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#include "ufo/identity/ObsIdentity.h"

#include <ostream>
#include <vector>

#include "ioda/ObsVector.h"

#include "oops/util/Logger.h"

#include "ufo/GeoVaLs.h"
#include "ufo/ObsDiagnostics.h"
#include "ufo/utils/OperatorUtils.h"  // for getOperatorVariables

namespace ufo {

// -----------------------------------------------------------------------------
static ObsOperatorMaker<ObsIdentity> obsIdentityMaker_("Identity");
// -----------------------------------------------------------------------------

ObsIdentity::ObsIdentity(const ioda::ObsSpace & odb,
                         const Parameters_ & parameters)
  : ObsOperatorBase(odb)
{
  oops::Log::trace() << "ObsIdentity constructor starting" << std::endl;

  getOperatorVariables(parameters.variables.value(), odb.obsvariables(),
                       operatorVars_, operatorVarIndices_);
  requiredVars_ += operatorVars_;

  // Check whether level index 0 is closest to the Earth's surface.
  levelIndexZeroAtSurface_  = parameters.levelIndex0IsClosestToSurface.value();

  oops::Log::trace() << "ObsIdentity constructor finished" << std::endl;
}

// -----------------------------------------------------------------------------

ObsIdentity::~ObsIdentity() {
  oops::Log::trace() << "ObsIdentity destructed" << std::endl;
}

// -----------------------------------------------------------------------------

void ObsIdentity::simulateObs(const GeoVaLs & gv, ioda::ObsVector & ovec,
                              ObsDiagnostics &) const {
  oops::Log::trace() << "ObsIdentity: simulateObs starting" << std::endl;

  std::vector<double> vec(ovec.nlocs());
  for (int jvar : operatorVarIndices_) {
    const std::string& varname = ovec.varnames().variables()[jvar];
    // Get GeoVaL at the level closest to the Earth's surface.
    if (levelIndexZeroAtSurface_)
      gv.getAtLevel(vec, varname, 0);
    else
      gv.getAtLevel(vec, varname, gv.nlevs(varname) - 1);
    for (size_t jloc = 0; jloc < ovec.nlocs(); ++jloc) {
      const size_t idx = jloc * ovec.nvars() + jvar;
      ovec[idx] = vec[jloc];
    }
  }

  oops::Log::trace() << "ObsIdentity: simulateObs finished" << std::endl;
}

// -----------------------------------------------------------------------------

void ObsIdentity::print(std::ostream & os) const {
  os << "ObsIdentity operator" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace ufo
