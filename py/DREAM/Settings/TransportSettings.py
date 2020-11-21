# General transport settings which can be added to any UnknownQuantity


import numpy as np
from .. DREAMException import DREAMException


TRANSPORT_NONE = 1
TRANSPORT_PRESCRIBED = 2
TRANSPORT_RECHESTER_ROSENBLUTH = 3

BC_CONSERVATIVE = 1
BC_F_0 = 2


class TransportSettings:
    

    def __init__(self, kinetic=False):
        """
        Constructor.

        :param bool kinetic: If ``True``, the coefficient will be assumed kinetic (4D). Otherwise fluid (2D).
        """
        self.kinetic = kinetic
        self.type    = TRANSPORT_NONE

        # Advection
        self.ar        = None
        self.ar_t      = None
        self.ar_r      = None
        self.ar_p      = None
        self.ar_xi     = None
        self.ar_ppar   = None
        self.ar_pperp  = None

        # Diffusion
        self.drr       = None
        self.drr_t     = None
        self.drr_r     = None
        self.drr_p     = None
        self.drr_xi    = None
        self.drr_ppar  = None
        self.drr_pperp = None

        # Rechester-Rosenbluth (diffusive) transport
        self.dBB       = None
        self.dBB_t     = None
        self.dBB_r     = None

        self.boundarycondition = BC_CONSERVATIVE


    def isKinetic(self): return self.kinetic
    

    def prescribeAdvection(self, ar, t=None, r=None, p=None, xi=None, ppar=None, pperp=None):
        """
        Set the advection coefficient to use.
        """
        self._prescribeCoefficient('ar', coeff=ar, t=t, r=r, p=p, xi=xi, ppar=ppar, pperp=pperp)


    def prescribeDiffusion(self, drr, t=None, r=None, p=None, xi=None, ppar=None, pperp=None):
        """
        Set the diffusion coefficient to use.
        """
        self._prescribeCoefficient('drr', coeff=drr, t=t, r=r, p=p, xi=xi, ppar=ppar, pperp=pperp)


    def _prescribeCoefficient(self, name, coeff, t=None, r=None, p=None, xi=None, ppar=None, pperp=None):
        """
        General method for prescribing an advection or diffusion coefficient.
        """
        self.type = TRANSPORT_PRESCRIBED

        if np.isscalar(coeff):
            r = np.array([0])
            t = np.array([0])
            p = np.array([0])
            xi = np.array([0])

            if self.kinetic:
                coeff = coeff * np.ones((1,)*4)
            else:
                coeff = coeff * np.ones((1,)*2)

        r = np.asarray(r)
        t = np.asarray(t)
        
        if r.ndim != 1: r = np.reshape(r, (r.size,))
        if t.ndim != 1: t = np.reshape(t, (t.size,))

        if self.kinetic == False and len(coeff.shape) == 2:
            setattr(self, name, coeff)
            setattr(self, name+'_r', r)
            setattr(self, name+'_t', t)
        elif self.kinetic == True and len(coeff.shape) == 4:
            # Verify that the momentum grid is given
            if p is not None and xi is not None:
                ppar, pperp = None, None
            elif ppar is not None and pperp is not None:
                p, xi = None, None
            else:
                raise TransportException("No momentum grid provided for the 4D transport coefficient.")

            setattr(self, name, coeff)
            setattr(self, name+'_r', r)
            setattr(self, name+'_t', t)

            if p is not None:
                setattr(self, name+'_p', p)
                setattr(self, name+'_xi', xi)
            else:
                setattr(self, name+'_ppar', ppar)
                setattr(self, name+'_pperp', pperp)
        else:
            raise TransportException("Invalid dimensions of prescribed coefficient: {}. Expected {} dimensions.".format(coeff.shape, 4 if self.kinetic else 2))


    def setMagneticPerturbation(self, dBB, t=None, r=None):
        """
        Prescribes the evolution of the magnetic perturbation level (dB/B).

        :param dBB: Magnetic perturbation level.
        :param t:   Time grid on which the perturbation is defined.
        :param r:   Radial grid on which the perturbation is defined.
        """
        self.type = TRANSPORT_RECHESTER_ROSENBLUTH

        if np.isscalar(dBB):
            dBB = dBB * np.ones((1,1))
            r = np.array([0])
            t = np.array([0])

        r = np.asarray(r)
        t = np.asarray(t)

        if r.ndim != 1: r = np.reshape(r, (r.size,))
        if t.ndim != 1: t = np.reshape(t, (t.size,))

        self.dBB_r = r
        self.dBB_t = t
        self.dBB   = dBB


    def setBoundaryCondition(self, bc):
        """
        Set the boundary condition to use for the transport.
        """
        self.boundarycondition = bc


    def fromdict(self, data):
        """
        Set all options from a dictionary.
        """
        self.ar = None
        self.ar_r = None
        self.ar_t = None
        self.ar_p = None
        self.ar_xi = None
        self.ar_ppar = None
        self.ar_pperp = None

        self.drr = None
        self.drr_r = None
        self.drr_t = None
        self.drr_p = None
        self.drr_xi = None
        self.drr_ppar = None
        self.drr_pperp = None

        self.dBB = None
        self.dBB_r = None
        self.dBB_t = None

        if 'type' in data:
            self.type = data['type']

        if 'boundarycondition' in data:
            self.boundarycondition = data['boundarycondition']

        if 'ar' in data:
            self.ar = data['ar']['x']
            self.r  = data['ar']['r']
            self.t  = data['ar']['t']

            if self.kinetic:
                if 'p' in data['ar']: self.ar_p = data['ar']['p']
                if 'xi' in data['ar']: self.ar_xi = data['ar']['xi']
                if 'ppar' in data['ar']: self.ar_ppar = data['ar']['ppar']
                if 'pperp' in data['ar']: self.ar_pperp = data['ar']['pperp']

        if 'drr' in data:
            self.drr = data['drr']['x']
            self.drr_r  = data['drr']['r']
            self.drr_t  = data['drr']['t']

            if self.kinetic:
                if 'p' in data['drr']: self.drr_p = data['drr']['p']
                if 'xi' in data['drr']: self.drr_xi = data['drr']['xi']
                if 'ppar' in data['drr']: self.drr_ppar = data['drr']['ppar']
                if 'pperp' in data['drr']: self.drr_pperp = data['drr']['pperp']

        if 'dBB' in data:
            self.dBB = data['dBB']['x']
            self.dBB_r = data['dBB']['r']
            self.dBB_t = data['dBB']['t']


    def todict(self):
        """
        Returns these settings as a dictionary.
        """
        data = {
            'type': self.type,
            'boundarycondition': self.boundarycondition
        }

        # Advection?
        if self.type == TRANSPORT_PRESCRIBED and self.ar is not None:
            data['ar'] = {
                'x': self.ar,
                'r': self.ar_r,
                't': self.ar_t
            }

            if self.kinetic:
                if self.ar_p is not None:
                    data['ar']['p'] = self.ar_p
                    data['ar']['xi'] = self.ar_xi
                else:
                    data['ar']['ppar'] = self.ar_ppar
                    data['ar']['pperp'] = self.ar_pperp

        # Diffusion?
        if self.type == TRANSPORT_PRESCRIBED and self.drr is not None:
            data['drr'] = {
                'x': self.drr,
                'r': self.drr_r,
                't': self.drr_t
            }

            if self.kinetic:
                if self.drr_p is not None:
                    data['drr']['p'] = self.drr_p
                    data['drr']['xi'] = self.drr_xi
                else:
                    data['drr']['ppar'] = self.drr_ppar
                    data['drr']['pperp'] = self.drr_pperp

        if self.type == TRANSPORT_RECHESTER_ROSENBLUTH and self.dBB is not None:
            data['dBB'] = {
                'x': self.dBB,
                'r': self.dBB_r,
                't': self.dBB_t
            }

        return data


    def verifySettings(self):
        """
        Verify that the settings are consistent.
        """
        if self.type == TRANSPORT_NONE:
            pass
        elif self.type == TRANSPORT_PRESCRIBED:
            self.verifySettingsCoefficient('ar')
            self.verifySettingsCoefficient('drr')
            self.verifyBoundaryCondition()
        elif self.type == TRANSPORT_RECHESTER_ROSENBLUTH:
            self.verifySettingsRechesterRosenbluth()
            self.verifyBoundaryCondition()
        else:
            raise TransportException("Unrecognized transport type: {}".format(self.type))


    def verifyBoundaryCondition(self):
        """
        Verify that the boundary condition has been correctly configured.
        """
        bcs = [BC_CONSERVATIVE, BC_F_0]
        if self.boundarycondition not in bcs:
            raise TransportException("Invalid boundary condition specified for transport: {}".format(self.boundarycondition))


    def verifySettingsCoefficient(self, coeff):
        """
        Verify consistency of the named prescribed transport coefficient.
        """
        g = lambda v : self.__dict__[coeff+v]
        c = g('')

        if c is None: return

        if self.kinetic:
            if c.ndim != 4:
                raise TransportException("{}: Invalid dimensions of transport coefficient: {}".format(coeff, c.shape))
            elif g('_t').ndim != 1 or g('_t').size != c.shape[0]:
                raise TransportException("{}: Invalid dimensions of time vector. Expected {} elements.".format(coeff, c.shape[0]))
            elif g('_r').ndim != 1 or g('_r').size != c.shape[1]:
                raise TransportException("{}: Invalid dimensions of radius vector. Expected {} elements.".format(coeff, c.shape[1]))

            if g('_p') is not None or g('_xi') is not None:
                if g('_xi').ndim != 1 or g('_xi').size != c.shape[2]:
                    raise TransportException("{}: Invalid dimensions of xi vector. Expected {} elements.".format(coeff, c.shape[2]))
                elif g('_p').ndim != 1 or g('_p').size != c.shape[3]:
                    raise TransportException("{}: Invalid dimensions of p vector. Expected {} elements.".format(coeff, c.shape[3]))
            elif g('_ppar') is not None or g('_pperp') is not None:
                if g('_pperp').ndim != 1 or g('_pperp').size != c.shape[2]:
                    raise TransportException("{}: Invalid dimensions of pperp vector. Expected {} elements.".format(coeff, c.shape[2]))
                elif g('_ppar').ndim != 1 or g('_ppar').size != c.shape[3]:
                    raise TransportException("{}: Invalid dimensions of ppar vector. Expected {} elements.".format(coeff, c.shape[3]))
            else:
                raise TransportException("No momentum grid provided for transport coefficient '{}'.".format(coeff))
        else:
            if c.ndim != 2:
                raise TransportException("{}: Invalid dimensions of transport coefficient: {}".format(coeff, c.shape))
            elif g('_t').ndim != 1 or g('_t').size != c.shape[0]:
                raise TransportException("{}: Invalid dimensions of time vector. Expected {} elements.".format(coeff, c.shape[0]))
            elif g('_r').ndim != 1 or g('_r').size != c.shape[1]:
                raise TransportException("{}: Invalid dimensions of radius vector. Expected {} elements.".format(coeff, c.shape[1]))

    def verifySettingsRechesterRosenbluth(self):
        """
        Verify consistency of the Rechester-Rosenbluth transport settings.
        """
        if self.dBB.ndim != 2:
            raise TransportException("Rechester-Rosenbluth: Invalid dimensions of transport coefficient: {}".format(self.dBB.shape))
        elif self.dBB_t.ndim != 1 or self.dBB_t.size != self.dBB.shape[0]:
            raise TransportException("Rechester-Rosenbluth: Invalid dimensions of time vector. Expected {} elements.".format(self.dBB.shape[0]))
        elif self.dBB_r.ndim != 1 or self.dBB_r.size != self.dBB.shape[1]:
            raise TransportException("Rechester-Rosenbluth: Invalid dimensions of radius vector. Expected {} elements.".format(self.dBB.shape[1]))


class TransportException(DREAMException):
    def __init__(self, msg):
        super().__init__(msg)


