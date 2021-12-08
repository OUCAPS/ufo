! (C) copyright 2021 Met Office
!
! this software is licensed under the terms of the apache licence version 2.0
! which can be obtained at http://www.apache.org/licenses/license-2.0.

!> Fortran module containing main type, setup and utilities for the
!! main rttovonedvarcheck object

module ufo_rttovonedvarcheck_setup_mod

use fckit_configuration_module, only: fckit_configuration
use iso_c_binding
use kinds
use ufo_rttovonedvarcheck_constants_mod

implicit none
private

public ufo_rttovonedvarcheck_setup

!===============================================================================
! type definitions
!===============================================================================

!---------------
! 1. 1DVar type
!---------------

type, public :: ufo_rttovonedvarcheck
  character(len=max_string)        :: qcname !< name of the filter
  character(len=max_string)        :: b_matrix_path !< path to the b-matrix file
  character(len=max_string)        :: r_matrix_path !< path to the r-matrix file
  character(len=max_string)        :: forward_mod_name !< forward model name (only RTTOV at the moment)
  character(len=max_string), allocatable :: retrieval_variables(:) !< list of variables which form the 1D-var retrieval vector
  character(len=max_string)        :: EmissivityType !< method used to initialize the surface emissivity
  type(c_ptr)                      :: obsdb !< pointer to the observation space
  integer(c_int)                   :: onedvarflag !< flag uased by the qc manager for a 1D-var check
  integer(c_int)                   :: passflag !< flag uased by the qc manager to flag good data
  integer                          :: nlevels ! number 1D-Var model levels
  integer                          :: nmvars !< number of variables being used in the retrieval
  integer                          :: nchans !< maximum number of channels (channels can be removed by previous qc checks)
  integer(c_int), allocatable      :: channels(:) !< integer list of channels
  integer, allocatable             :: EmissToChannelMap(:) !< integer list to map emissivity elements to channels
  integer, allocatable             :: ChannelToEmissMap(:) !< integer list to map channels to emissivity elements
  integer                          :: StartOb !< starting ob number for testing
  integer                          :: FinishOb !< finishing ob number for testing
  logical                          :: qtotal !< flag to enable total humidity retrievals
  logical                          :: UseQtsplitRain !< flag to choose whether to split rain in qsplit routine
  logical                          :: RTTOV_mwscattSwitch !< flag to switch on RTTOV-scatt
  logical                          :: RTTOV_usetotalice !< flag for use of total ice in RTTOV MW scatt
  logical                          :: UseMLMinimization !< flag to turn on marquardt-levenberg minimizer
  logical                          :: UseJforConvergence !< flag to Use J for convergence
  logical                          :: UseRHwaterForQC !< flag to use water in relative humidity check
  logical                          :: Store1DVarLWP !< Output the LWP if the profile converges
  logical                          :: Store1DVarCLW !< Output the CLW profile if 1dvar converrges for later use
  logical                          :: UseColdSurfaceCheck !< flag to use cold water check to adjust starting surface parameters
  logical                          :: FullDiagnostics !< flag to turn on full diagnostics
  logical                          :: pcemiss !< flag gets turned off in emissivity eigen vector file is present
  logical                          :: mwEmissRetrieval !< if true do emissivity retrival using the mwemiss method
  integer                          :: Max1DVarIterations !< maximum number of iterations
  integer                          :: JConvergenceOption !< integer to select convergence option
  integer                          :: IterNumForLWPCheck !< choose which iteration to start checking LWP
  integer                          :: MaxMLIterations !< maximum number of iterations for internal Marquardt-Levenberg loop
  integer                          :: NumEmissElements !< the number of surface emissivity elements to be retrieved
  real(kind_real)                  :: RetrievedErrorFactor !< check retrieved BTs all within factor * stdev of obs
  real(kind_real)                  :: ConvergenceFactor !< 1d-var convergence if using change in profile
  real(kind_real)                  :: Cost_ConvergenceFactor !< 1d-var convergence if using % change in cost
  real(kind_real)                  :: EmissSeaDefault !< default emissivity value to use over sea
  real(kind_real)                  :: EmissLandDefault !< default emissivity value to use over land
  real(kind_real)                  :: EmissSeaIceDefault !< default emissivity value to use over sea ice
  character(len=max_string)        :: EmissGroupInObsSpace !< emissivity location in the ObsSpace
  real(kind_real)                  :: SkinTempErrorLand !< value to scale the skin temperature error over land
  character(len=max_string)        :: EmisEigVecPath !< path to eigen vector file for IR PC emissivity
  character(len=max_string)        :: EmisAtlas !< path to the emissivity atlas for IR PC emissivity
end type ufo_rttovonedvarcheck

contains

!------------------------------------------------------------------------------
!> Setup the defaults for the main rttovonedvarcheck object and read in the 
!! contents of the yaml file.
!!
!! \author Met Office
!!
!! \date 09/06/2020: Created
!!
subroutine ufo_rttovonedvarcheck_setup(self, f_conf, channels)

implicit none

! subroutine arguments
type(ufo_rttovonedvarcheck), intent(inout) :: self
type(fckit_configuration), intent(in)      :: f_conf       !< yaml file contents
integer(c_int), intent(in)                 :: channels(:)

! local variables
character(len=max_string)     :: tmp
character(len=:), allocatable :: str
character(len=:), allocatable :: str_array(:)
type(fckit_configuration)     :: surface_emissivity_conf
integer                       :: size_geovals, size_extravars

! Creat surface emissivity conf from main configuration
call f_conf % get_or_die("surface emissivity", surface_emissivity_conf)

! Setup core paths and names
self % qcname = "rttovonedvarcheck"
call f_conf % get_or_die("BMatrix",str)
self % b_matrix_path = str
call f_conf % get_or_die("RMatrix",str)
self % r_matrix_path = str
call f_conf % get_or_die("ModName",str)
self % forward_mod_name = str
call f_conf % get_or_die("nlevels",self % nlevels)

! Variables for profile (x,xb)
size_geovals = f_conf % get_size("retrieval variables from geovals")
size_extravars = f_conf % get_size("retrieval variables not from geovals")
self % nmvars = size_geovals + size_extravars
allocate(self % retrieval_variables(self % nmvars))
call f_conf % get_or_die("retrieval variables from geovals", str_array)
self % retrieval_variables(1:size_geovals) = str_array
call f_conf % get_or_die("retrieval variables not from geovals", str_array)
self % retrieval_variables(size_geovals+1 : size_geovals+size_extravars) = str_array

! Satellite channels
self % nchans = size(channels)
allocate(self % channels(self % nchans))
self % channels(:) = channels(:)

! Flag for total humidity
call f_conf % get_or_die("qtotal", self % qtotal)

! Flag to choose whether to split rain in qsplit routine
call f_conf % get_or_die("UseQtSplitRain", self % UseQtsplitRain)

! Flag for RTTOV MW scatt
call f_conf % get_or_die("RTTOVMWScattSwitch", self % RTTOV_mwscattSwitch)

! Flag for use of total ice in RTTOV MW scatt
call f_conf % get_or_die("RTTOVUseTotalIce", self % RTTOV_usetotalice)

! Flag to turn on marquardt-levenberg minimiser
call f_conf % get_or_die("UseMLMinimization", self % UseMLMinimization)

! Flag to Use J for convergence
call f_conf % get_or_die("UseJforConvergence", self % UseJforConvergence)

! Flag to use water in relative humidity check
call f_conf % get_or_die("UseRHwaterForQC", self % UseRHwaterForQC)

! Flag to use cold water check to adjust starting surface parameters
call f_conf % get_or_die("UseColdSurfaceCheck", self % UseColdSurfaceCheck)

! Flag to output the LWP if the profile converges
call f_conf % get_or_die("Store1DVarLWP", self % Store1DVarLWP)

! Flag to output the CLW if the profile converges
call f_conf % get_or_die("Store1DVarCLW", self % Store1DVarCLW)

! Flag to turn on full diagnostics
call f_conf % get_or_die("FullDiagnostics", self % FullDiagnostics)

! maximum number of iterations allowed
call f_conf % get_or_die("Max1DVarIterations", self % Max1DVarIterations)

! integer to select convergence option
! 1= percentage change in cost tested between iterations
! otherwise = absolute change in cost tested between iterations
call f_conf % get_or_die("JConvergenceOption", self % JConvergenceOption)

! Choose which iteration to start checking the liquid water path
call f_conf % get_or_die("IterNumForLWPCheck", self % IterNumForLWPCheck)

! Check the retrieved brightness temperatures are within a factor * error of the
! observed and bias corrected BTs.  If this value is less than 0.0 this check is
! not performed
call f_conf % get_or_die("RetrievedErrorFactor", self % RetrievedErrorFactor)

! Convergence factor used when the absolute difference in the profile is used
! to determine convergence.
call f_conf % get_or_die("ConvergenceFactor", self % ConvergenceFactor)

! Cost threshold for convergence check when cost function value is used for convergence
call f_conf % get_or_die("CostConvergenceFactor", self % Cost_ConvergenceFactor)

! Maximum number of iterations for internal Marquardt-Levenberg loop
call f_conf % get_or_die("MaxMLIterations", self % MaxMLIterations)

! Starting observation number for loop - used for testing
call f_conf % get_or_die("StartOb", self % StartOb)

! Finishing observation number for loop - used for testing
call f_conf % get_or_die("FinishOb", self % FinishOb)

! Value to scale the skin temperature error over land. -1.0 is default so no scaling
! is done because the value has to be positive.
call f_conf % get_or_die("SkinTempErrorLand", self % SkinTempErrorLand)

!!! --------------------------------------------
!!! Variables from the Emissivity configuration
!!! --------------------------------------------

! How to initialise the surface emissivity
! rttovtocalculate - rttov will calculate over all surfaces
! fixed - values of EmissSeaDefault, EmissLandDefault, EmissSeaIceDefault
! readfromdb - read values from the db that have previously been read in
!              and set calc_emiss to true where zero.
! readfromdbwitherror - read values from the db that have previously
!              been read in and set calc_emiss to true where zero.
!              Read in the emissivity error as well.
! princialcomponent - net ready yet
call surface_emissivity_conf % get_or_die("type", str)
self % EmissivityType = str

! Default emissivity value to use over sea
! used with type = fixed
call surface_emissivity_conf % get_or_die("EmissSeaDefault", self % EmissSeaDefault)

! Default emissivity value to use over land
! used with type = fixed
call surface_emissivity_conf % get_or_die("EmissLandDefault", self % EmissLandDefault)

! Default emissivity value to use over seaice
! used with type = fixed
call surface_emissivity_conf % get_or_die("EmissSeaIceDefault", self % EmissSeaIceDefault)

! Location of the surface emissivity in the ObsSpace.  Used when either readfromdb or
! readfromdbwitherror is used
call surface_emissivity_conf % get_or_die("group in obs space", str)
self % EmissGroupInObsSpace = str

! Flag to decide if mwemiss retrieval needed
call surface_emissivity_conf % get_or_die("retrieve mw emissivity", &
                                          self % mwEmissRetrieval)

if (self % mwEmissRetrieval) then

  ! Number of surface emissivity elements to be retrieved.  This will be checked
  ! against the number in the b-matrix.
  ! used when surface_emissivity is retrieved unless pcemiss is specified
  call surface_emissivity_conf % get_or_die("number of surface emissivity retrieval elements", &
                                            self % NumEmissElements)

  ! Maps the correct emissivity element to the correct instrument channel.
  ! Must be of size self % NumEmissElements
  ! used when surface_emissivity is retrieved unless pcemiss is specified
  allocate(self % EmissToChannelMap(self % NumEmissElements))
  call surface_emissivity_conf % get_or_die("emissivity to channel mapping", &
                                            self % EmissToChannelMap)

  ! Maps the instrument channels to the correct emissivity element used in the retrieval.
  ! used when surface_emissivity is retrieved unless pcemiss is specified
  allocate(self % ChannelToEmissMap(self % nchans))
  call surface_emissivity_conf % get_or_die("channel to emissivity mapping", &
                                          self % ChannelToEmissMap)

end if

! Default eigen value path is blank but needs to be present if using PC emiss
! used with type = principalComponentEmiss
call surface_emissivity_conf % get_or_die("EmisEigVecPath",str)
self % EmisEigVecPath = str
self % pcemiss = .false. 
if (len(trim(self % EmisEigVecPath)) > 0) then
  self % pcemiss = .true.
end if

! Default emis atlas path is blank
! used with type = principalComponentEmiss
call surface_emissivity_conf % get_or_die("EmisAtlas",str)
self % EmisAtlas = str

!!! --------------------------------------------

! Print self
if (self % FullDiagnostics) then
  call ufo_rttovonedvarcheck_print(self)
end if

end subroutine ufo_rttovonedvarcheck_setup

!------------------------------------------------------------------------------
!> Print contents of rttovonedvarcheck object
!!
!! \author Met Office
!!
!! \date 03/08/2020: Created
!!
subroutine ufo_rttovonedvarcheck_print(self)

implicit none

type(ufo_rttovonedvarcheck), intent(in) :: self

integer :: ii

write(*,*) "qcname = ", trim(self % qcname)
write(*,*) "b_matrix_path = ", trim(self % b_matrix_path)
write(*,*) "r_matrix_path = ", trim(self % r_matrix_path)
write(*,*) "forward_mod_name = ", trim(self % forward_mod_name)
write(*,*) "retrieval_variables = "
do ii = 1, self % nmvars
  write(*,*) trim(self % retrieval_variables(ii))," "
end do
write(*,*) "nlevels = ",self %  nlevels
write(*,*) "nmvars = ",self % nmvars
write(*,*) "nchans = ",self % nchans
write(*,*) "channels(:) = ",self % channels(:)
write(*,*) "qtotal = ",self % qtotal
write(*,*) "RTTOV_mwscattSwitch = ",self % RTTOV_mwscattSwitch
write(*,*) "RTTOV_usetotalice = ",self % RTTOV_usetotalice
write(*,*) "UseMLMinimization = ",self % UseMLMinimization
write(*,*) "UseJforConvergence = ",self % UseJforConvergence
write(*,*) "UseRHwaterForQC = ", self % UseRHwaterForQC
write(*,*) "UseColdSurfaceCheck = ", self % UseColdSurfaceCheck
write(*,*) "UseQtsplitRain = ",self % UseQtsplitRain
write(*,*) "FullDiagnostics = ",self % FullDiagnostics
write(*,*) "Max1DVarIterations = ",self % Max1DVarIterations
write(*,*) "JConvergenceOption = ",self % JConvergenceOption
write(*,*) "IterNumForLWPCheck = ",self % IterNumForLWPCheck
write(*,*) "ConvergenceFactor = ",self % ConvergenceFactor
write(*,*) "CostConvergenceFactor = ",self % Cost_ConvergenceFactor
write(*,*) "MaxMLIterations = ",self % MaxMLIterations
write(*,*) "Store1DVarLWP = ",self % Store1DVarLWP
write(*,*) "Store1DVarCLW = ",self % Store1DVarCLW
write(*,*) "Emissivity variables:"
write(*,*) "emissivity type = ",self % EmissivityType
write(*,*) "EmissSeaDefault = ",self % EmissSeaDefault
write(*,*) "EmissLandDefault = ",self % EmissLandDefault
write(*,*) "EmissSeaIceDefault = ",self % EmissSeaIceDefault
write(*,*) "mwEmissRetrieval = ",self % mwEmissRetrieval
write(*,*) "NumEmissElements = ",self % NumEmissElements
write(*,*) "EmissToChannelMap = ",self % EmissToChannelMap
write(*,*) "ChannelToEmissMap = ",self % ChannelToEmissMap
write(*,*) "Use PC for Emissivity = ", self % pcemiss
write(*,*) "EmisEigVecPath = ",self % EmisEigVecPath
write(*,*) "EmisAtlas = ",self % EmisAtlas

end subroutine ufo_rttovonedvarcheck_print

! ------------------------------------------------------------------------------

end module ufo_rttovonedvarcheck_setup_mod
