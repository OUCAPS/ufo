/*
 * (C) Copyright 2021 UCAR
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#ifndef UFO_AEROSOLS_AOP_OBSAODEXTTLAD_H_
#define UFO_AEROSOLS_AOP_OBSAODEXTTLAD_H_

#include <ostream>
#include <string>

#include "oops/base/Variables.h"
#include "oops/util/ObjectCounter.h"

#include "ufo/aerosols/AOP/ObsAodExtParameters.h"
#include "ufo/aerosols/AOP/ObsAodExtTLAD.interface.h"
#include "ufo/LinearObsOperatorBase.h"

// Forward declarations
namespace ioda {
  class ObsSpace;
  class ObsVector;
}

namespace ufo {
  class GeoVaLs;

// -----------------------------------------------------------------------------
/// AodExt TL/AD observation operator class
class ObsAodExtTLAD : public LinearObsOperatorBase,
                       private util::ObjectCounter<ObsAodExtTLAD> {
 public:
  static const std::string classname() {return "ufo::ObsAodExtTLAD";}

  typedef ObsAodExtParameters Parameters_;

  ObsAodExtTLAD(const ioda::ObsSpace &, const Parameters_ &);
  virtual ~ObsAodExtTLAD();

  // Obs Operators
  void setTrajectory(const GeoVaLs &, ObsDiagnostics &) override;
  void simulateObsTL(const GeoVaLs &, ioda::ObsVector &) const override;
  void simulateObsAD(GeoVaLs &, const ioda::ObsVector &) const override;

  // Other
  const oops::Variables & requiredVars() const override {return varin_;}

  int & toFortran() {return keyOper_;}
  const int & toFortran() const {return keyOper_;}

 private:
  void print(std::ostream &) const override;
  F90hop keyOper_;
  oops::Variables varin_;
};

// -----------------------------------------------------------------------------

}  // namespace ufo
#endif  // UFO_AEROSOLS_AOP_OBSAODEXTTLAD_H_
