/*
 * (C) Copyright 2019 UCAR
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 */

#ifndef UFO_FILTERS_THINNING_H_
#define UFO_FILTERS_THINNING_H_

#include <ostream>
#include <string>

#include "boost/shared_ptr.hpp"

#include "eckit/config/LocalConfiguration.h"
#include "ioda/ObsDataVector.h"
#include "ioda/ObsSpace.h"
#include "oops/base/Variables.h"
#include "oops/util/ObjectCounter.h"
#include "oops/util/Printable.h"

namespace ioda {
  template <typename DATATYPE> class ObsDataVector;
  class ObsVector;
}

namespace ufo {
  class GeoVaLs;
  class ObsDiagnostics;

/// Thinning: randonly thin a given percentage of observations

class Thinning : public util::Printable,
                  private util::ObjectCounter<Thinning> {
 public:
  static const std::string classname() {return "ufo::Thinning";}

  Thinning(const ioda::ObsSpace &, const eckit::Configuration &,
           boost::shared_ptr<ioda::ObsDataVector<int> >,
           boost::shared_ptr<ioda::ObsDataVector<float> >);
  ~Thinning();

  void preProcess() const;
  void priorFilter(const GeoVaLs &) const {}
  void postFilter(const ioda::ObsVector &, const ObsDiagnostics &) const {}

  const oops::Variables & requiredGeoVaLs() const {return geovars_;}
  const oops::Variables & requiredHdiagnostics() const {return diagvars_;}

 private:
  void print(std::ostream &) const;

  const ioda::ObsSpace & obsdb_;
  const eckit::LocalConfiguration config_;
  const oops::Variables geovars_;
  const oops::Variables diagvars_;
  ioda::ObsDataVector<int> & flags_;
};

}  // namespace ufo

#endif  // UFO_FILTERS_THINNING_H_
