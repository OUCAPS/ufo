/*
 * (C) Copyright 2020 Met Office
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

/* 1-D var qc and retrieval of atmospheric state
 *   J(x) = (x-xb)T B-1 (x-xb) + (y-H(x))T R-1 (y-H(x))
 *   Code adapted from Met Office OPS System
 */

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <vector>

#include "ufo/filters/rttovonedvarcheck/RTTOVOneDVarCheck.h"
#include "ufo/filters/rttovonedvarcheck/RTTOVOneDVarCheck.interface.h"
#include "ufo/GeoVaLs.h"

#include "eckit/exception/Exceptions.h"

namespace ufo {

// -----------------------------------------------------------------------------

RTTOVOneDVarCheck::RTTOVOneDVarCheck(ioda::ObsSpace & obsdb, const Parameters_ & parameters,
                                 std::shared_ptr<ioda::ObsDataVector<int> > flags,
                                 std::shared_ptr<ioda::ObsDataVector<float> > obserr)
  : FilterBase(obsdb, parameters, flags, obserr), channels_(), retrieved_vars_(),
    hoxdiags_retrieved_vars_(), parameters_(parameters)
{
  oops::Log::trace() << "RTTOVOneDVarCheck contructor starting" << std::endl;

  // Check only one variable has been defined - BT
  // Get channels from filter variables
  if (filtervars_.size() != 1) {
     throw eckit::UserError("RTTOVOneDVarCheck contructor:"
                            " only one variable allowed, aborting.");
  }
  channels_ = filtervars_[0].channels();

  // Check at least one channel has been defined
  if (channels_.empty()) {
     throw eckit::UserError("RTTOVOneDVarCheck contructor: no channels defined, aborting.");
  }

  // Setup Fortran object
  ufo_rttovonedvarcheck_create_f90(keyRTTOVOneDVarCheck_, obsdb, parameters_.toConfiguration(),
              channels_.size(), channels_[0], retrieved_vars_, QCflags::onedvar, QCflags::pass);

  // Create hofxdiags
  for (size_t jvar = 0; jvar < retrieved_vars_.size(); ++jvar) {
    for (size_t jch = 0; jch < channels_.size(); ++jch) {
      hoxdiags_retrieved_vars_.push_back("brightness_temperature_jacobian_"+
                   retrieved_vars_[jvar]+"_"+std::to_string(channels_[jch]));
    }
  }

  // Populate variables list - which makes sure this is run as a post filter
  // because the obs bias value is needed.
  Variables model_vars(retrieved_vars_);
  allvars_ += Variables(model_vars, "GeoVaLs");
  allvars_ += Variables(filtervars_, "ObsBiasData");

  // Check that SatRad compatibility is false.  Any profile manipulations are done in
  // this filter rather than the RTTOV interface because the update GeoVaLs are needed
  // for the minimization.
  if (parameters_.ModOptions.value().obsOptions.value().satRadCompatibility.value()) {
    std::stringstream msg;
    msg << "SatRad compatibility for RTTOV should always be false with RTTOVOneDVarCheck "
        << "=> update yaml and try again";
    throw eckit::UserError(msg.str(), Here());
  }
  oops::Log::trace() << "RTTOVOneDVarCheck contructor complete. " << std::endl;
}

// -----------------------------------------------------------------------------

RTTOVOneDVarCheck::~RTTOVOneDVarCheck() {
  ufo_rttovonedvarcheck_delete_f90(keyRTTOVOneDVarCheck_);
  oops::Log::trace() << "RTTOVOneDVarCheck destructed" << std::endl;
}

// -----------------------------------------------------------------------------

void RTTOVOneDVarCheck::applyFilter(const std::vector<bool> & apply,
                               const Variables & filtervars,
                               std::vector<std::vector<bool>> &) const {
  oops::Log::trace() << "RTTOVOneDVarCheck Filter starting" << std::endl;

// Get GeoVaLs - the copy is needed to check the z-direction orientation.
// this is a temporary measure until the filter is updated to deal with toptobottom.
  const ufo::GeoVaLs * gvals = data_.getGeoVaLs();
  ufo::GeoVaLs geoVaLsCopy = *gvals;
  if (geoVaLsCopy.has("air_pressure")) {
    geoVaLsCopy.reorderzdir("air_pressure", "bottom2top");
  } else {
    throw eckit::BadValue("GeoVaLs must contain air_pressure");
  }

// Create oops variable with the list of channels
  oops::Variables variables = filtervars.toOopsVariables();

// Convert apply to char for passing to fortran
// needed for channel selection
  std::vector<char> apply_char(apply.size(), 'F');
  for (size_t i = 0; i < apply_char.size(); i++) {
    if (apply[i]) {apply_char[i]='T';}
  }

// Save qc flags to database for retrieval in fortran - needed for channel selection
  flags_->save("FortranQC");    // temporary measure as per ROobserror qc

// Read in ObsBias for all channels in the database and save to db for access in Fortran
// there is currently no mechanism for passing ioda::ObsDataVectors to Fortran.
// This is saved to ObsBias rather than ObsBiasData to avoid replication of datasets because
// the oops::Observer saves to ObsBias.
  Variable obsbiasvar("brightness_temperature@ObsBiasData",
                      obsdb_.obsvariables().channels());
  ioda::ObsDataVector<float> obsbias(obsdb_, obsbiasvar.toOopsVariables());
  data_.get(obsbiasvar, obsbias);
  obsbias.save("ObsBias");

// Pass it all to fortran
  ufo_rttovonedvarcheck_apply_f90(keyRTTOVOneDVarCheck_,
                                  parameters_.ModOptions.value().toConfiguration(),
                                  variables, hoxdiags_retrieved_vars_,
                                  geoVaLsCopy.toFortran(),
                                  apply_char.size(), apply_char[0]);

// Read qc flags from database
  flags_->read("FortranQC");    // temporary measure as per ROobserror qc

  oops::Log::trace() << "RTTOVOneDVarCheck Filter complete" << std::endl;
}

// -----------------------------------------------------------------------------

void RTTOVOneDVarCheck::print(std::ostream & os) const {
  os << "RTTOVOneDVarCheck: config =  " << parameters_ << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace ufo
