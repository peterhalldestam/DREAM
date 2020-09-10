#
# TimeStepper settings object
# #################################

import numpy as np

from .. DREAMException import DREAMException


TYPE_CONSTANT = 1
TYPE_ADAPTIVE = 2


class TimeStepper:
    
    def __init__(self, ttype=1, checkevery=0, tmax=None, dt=None, nt=None, reltol=1e-6, verbose=False, constantstep=False):
        """
        Constructor.
        """
        self.set(ttype=ttype, checkevery=checkevery, tmax=tmax, dt=dt, nt=nt, reltol=reltol, verbose=verbose, constantstep=constantstep)
        

    def set(self, ttype=1, checkevery=0, tmax=None, dt=None, nt=None, reltol=1e-6, verbose=False, constantstep=False):
        """
        Set properties of the time stepper.
        """
        self.type = int(ttype)

        self.setCheckInterval(checkevery)
        self.setTmax(tmax)
        self.setDt(dt)
        self.setNt(nt)
        self.setRelativeTolerance(reltol)
        self.setVerbose(verbose)
        self.setConstantStep(constantstep)


    ######################
    # SETTERS
    ######################
    def setCheckInterval(self, checkevery):
        if checkevery < 0:
            raise DREAMException("TimeStepper: Invalid value assigned to 'checkevery': {}".format(checkevery))
        
        self.checkevery = int(checkevery)


    def setConstantStep(self, constantstep):
        self.constantstep = bool(constantstep)


    def setDt(self, dt):
        if dt is None:
            self.dt = None
            return

        if dt <= 0:
            raise DREAMException("TimeStepper: Invalid value assigned to 'dt': {}".format(tmax))
        if self.nt is not None and dt > 0:
            raise DREAMException("TimeStepper: 'dt' may not be set alongside 'nt'.")
            
        self.dt = float(dt)


    def setNt(self, nt):
        if nt is None:
            self.nt = None
            return

        if nt <= 0:
            raise DREAMException("TimeStepper: Invalid value assigned to 'dt': {}".format(tmax))
        if self.dt is not None and self.dt > 0:
            raise DREAMException("TimeStepper: 'nt' may not be set alongside 'dt'.")
            
        self.nt = int(nt)


    def setRelTol(self, reltol): self.setRelativeTolerance(reltol=reltol)


    def setRelativeTolerance(self, reltol):
        if reltol <= 0:
            raise DREAMException("TimeStepper: Invalid value assigned to 'reltol': {}".format(reltol))

        self.reltol = float(reltol)


    def setTmax(self, tmax):
        if tmax is None:
            self.tmax = None
            return

        if tmax <= 0:
            raise DREAMException("TimeStepper: Invalid value assigned to 'tmax': {}".format(tmax))

        self.tmax = float(tmax)


    def setType(self, ttype):
        if ttype != TYPE_CONSTANT and ttype != TYPE_ADAPTIVE:
            raise DREAMException("TimeStepper: Unrecognized time stepper type specified: {}".format(ttype))

        if ttype == TYPE_ADAPTIVE:
            self.nt = None

        self.type = int(ttype)


    def setVerbose(self, verbose=True):
        self.verbose = bool(verbose)


    def fromdict(self, data):
        """
        Load settings from the given dictionary.
        """
        def scal(v):
            if type(v) == np.ndarray: return v[0]
            else: return v

        self.type = data['type']
        self.tmax = data['tmax']

        if 'checkevery' in data: self.checkevery = int(scal(data['checkevery']))
        if 'dt' in data: self.dt = float(scal(data['dt']))
        if 'nt' in data: self.nt = int(scal(data['nt']))
        if 'reltol' in data: self.reltol = float(scal(data['reltol']))
        if 'verbose' in data: self.verbose = bool(scal(data['verbose']))
        if 'constantstep' in data: self.constantstep = bool(scal(data['constantstep']))

        self.verifySettings()


    def todict(self, verify=True):
        """
        Returns a Python dictionary containing all settings of
        this TimeStepper object.
        """
        if verify:
            self.verifySettings()

        data = {
            'type': self.type,
            'tmax': self.tmax
        }

        if self.dt is not None: data['dt'] = self.dt

        if self.type == TYPE_CONSTANT:
            if self.nt is not None: data['nt'] = self.nt
        elif self.type == TYPE_ADAPTIVE:
            data['checkevery'] = self.checkevery
            data['reltol'] = self.reltol
            data['verbose'] = self.verbose
            data['constantstep'] = self.constantstep

        return data


    def verifySettings(self):
        """
        Verify that the TimeStepper settings are consistent.
        """
        if self.type == TYPE_CONSTANT:
            if self.tmax is None or self.tmax <= 0:
                raise DREAMException("TimeStepper constant: 'tmax' must be set to a value > 0.")
            
            # Verify that _exactly_ one of 'dt' and 'nt' is
            # set to a valid value
            dtSet = (self.dt is not None and self.dt > 0)
            ntSet = (self.nt is not None and self.nt > 0)

            if dtSet and ntSet:
                raise DREAMException("TimeStepper constant: Exactly one of 'dt' and 'nt' must be > 0.")
        elif self.type == TYPE_ADAPTIVE:
            if self.tmax is None or self.tmax <= 0:
                raise DREAMException("TimeStepper adaptive: 'tmax' must be set to a value > 0.")
            elif self.nt is not None:
                raise DREAMException("TimeStepper adaptive: 'nt' cannot be used with the adaptive time stepper.")

            if type(self.checkevery) != int or self.checkevery < 0:
                raise DREAMException("TimeStepper adaptive: 'checkevery' must be a non-negative integer.")
            elif type(self.reltol) != float or self.reltol < 0:
                raise DREAMException("TimeStepper adaptive: 'reltol' must be a positive real number.")
            elif type(self.verbose) != bool:
                raise DREAMException("TimeStepper adaptive: 'verbose' must be a boolean.")
            elif type(self.constantstep) != bool:
                raise DREAMException("TimeStepper adaptive: 'constantstep' must be a boolean.")
        else:
            raise DREAMException("Unrecognized time stepper type selected: {}.".format(self.type))


