/*
 * (C) British Crown Copyright 2021 Met Office
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#ifndef UFO_GNSSRO_REFMETOFFICE_OBSGNSSROREFMETOFFICETLAD_H_
#define UFO_GNSSRO_REFMETOFFICE_OBSGNSSROREFMETOFFICETLAD_H_

#include <memory>
#include <ostream>
#include <string>

#include "oops/base/Variables.h"
#include "oops/util/ObjectCounter.h"
#include "ufo/gnssro/RefMetOffice/ObsGnssroRefMetOfficeTLAD.interface.h"
#include "ufo/LinearObsOperatorBase.h"
#include "ObsGnssroRefMetOfficeParameters.h"

// Forward declarations
namespace eckit {
  class Configuration;
}

namespace ioda {
  class ObsSpace;
  class ObsVector;
}

namespace ufo {
  class GeoVaLs;
  class ObsDiagnostics;

// -----------------------------------------------------------------------------
/// GnssroRefMetOffice observation operator - TL/AD
// -----------------------------------------------------------------------------
class ObsGnssroRefMetOfficeTLAD : public LinearObsOperatorBase,
                          private util::ObjectCounter<ObsGnssroRefMetOfficeTLAD> {
 public:
  static const std::string classname() {return "ufo::ObsGnssroRefMetOfficeTLAD";}

  ObsGnssroRefMetOfficeTLAD(const ioda::ObsSpace &, const eckit::Configuration &);
  virtual ~ObsGnssroRefMetOfficeTLAD();

  // Obs Operators
  void setTrajectory(const GeoVaLs &, ObsDiagnostics &) override;
  void simulateObsTL(const GeoVaLs &, ioda::ObsVector &) const override;
  void simulateObsAD(GeoVaLs &, const ioda::ObsVector &) const override;

  // Other
  const oops::Variables & requiredVars() const override {return *varin_;}

  int & toFortran() {return keyOperGnssroRefMetOffice_;}
  const int & toFortran() const {return keyOperGnssroRefMetOffice_;}

 private:
  void print(std::ostream &) const override;
  F90hop keyOperGnssroRefMetOffice_;
  std::unique_ptr<const oops::Variables> varin_;
  ObsGnssroRefMetOfficeParameters parameters_;
};

// -----------------------------------------------------------------------------

}  // namespace ufo
#endif  // UFO_GNSSRO_REFMETOFFICE_OBSGNSSROREFMETOFFICETLAD_H_
