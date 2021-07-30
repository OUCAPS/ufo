/*
 * (C) Copyright 2018-2019 UCAR
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 */

#ifndef UFO_FILTERS_QCMANAGER_H_
#define UFO_FILTERS_QCMANAGER_H_

#include <memory>
#include <ostream>

#include "eckit/config/LocalConfiguration.h"
#include "ioda/ObsDataVector.h"
#include "ioda/ObsSpace.h"
#include "oops/base/Variables.h"
#include "oops/interface/ObsFilterBase.h"
#include "oops/util/Printable.h"
#include "ufo/ObsTraits.h"

namespace ioda {
  template <typename DATATYPE> class ObsDataVector;
  class ObsVector;
}

namespace ufo {
class GeoVaLs;
class ObsDiagnostics;

/// \brief Always the first filter to be run.
///
/// The constructor sets the QC flag to `missing` at all locations with missing obs values of QC
/// flags. The postFilter() function sets the QC flag to `Hfailed` if it was previously set to
/// `pass`, but the obs operator failed to produce a valid value.
class QCmanager : public oops::interface::ObsFilterBase<ObsTraits> {
 public:
  QCmanager(ioda::ObsSpace &, const eckit::Configuration &,
            std::shared_ptr<ioda::ObsDataVector<int> >,
            std::shared_ptr<ioda::ObsDataVector<float> >);
  ~QCmanager();

  void preProcess() override {}
  void priorFilter(const GeoVaLs &) override {}
  void postFilter(const ioda::ObsVector &, const ObsDiagnostics &) override;

  oops::Variables requiredVars() const override {return nogeovals_;}
  oops::Variables requiredHdiagnostics() const override {return nodiags_;}

 private:
  void print(std::ostream &) const override;

  ioda::ObsSpace & obsdb_;
  const oops::Variables nogeovals_;
  const oops::Variables nodiags_;
  std::shared_ptr<ioda::ObsDataVector<int>> flags_;
};

}  // namespace ufo

#endif  // UFO_FILTERS_QCMANAGER_H_
