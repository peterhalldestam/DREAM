# p/xi output momentum grid object


import numpy as np
from .MomentumGrid import MomentumGrid


class PXiGrid(MomentumGrid):
    

    def __init__(self, name, r, dr, data):
        """
        Constructor.

        name: Grid name.
        r:    Associated radial grid.
        data: Momentum grid data.
        """
        super(PXiGrid, self).__init__(name=name, r=r, dr=dr, data=data)

        self.p1name = 'p'
        self.p2name = 'xi'

        self.p  = data['p1']
        self.xi = data['p2']
        self.dp = data['dp1']
        self.dxi = data['dp2']

