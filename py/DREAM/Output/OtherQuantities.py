
import numpy as np

from . OtherQuantity import OtherQuantity
from . OtherKineticQuantity import OtherKineticQuantity


class OtherQuantities:
    

    SPECIAL_TREATMENT = {
        # List of other quantities with their own classes
        'nu_s_f1': OtherKineticQuantity,
        'nu_s_f2': OtherKineticQuantity,
    }


    def __init__(self, other=None, grid=None, output=None, momentumgrid=None):
        """
        Constructor.
        """
        self.grid = grid
        self.quantities = {}
        self.output = output
        self.momentumgrid = momentumgrid
        
        if other is not None:
            self.setQuantities(other)


    def __getitem__(self, index):
        """
        Direct access by name to the list of quantities.
        """
        return self.quantities[index]


    def keys(self): return self.quantities.keys()


    def getQuantityNames(self):
        """
        Get a list with the names of all other quantities
        stored in the output file.
        """
        return list(self.quantities.keys())


    def setGrid(self, grid):
        """
        Sets the grid that was used for the DREAM simulation.
        """
        self.grid = grid


    def setQuantity(self, name, data):
        """
        Add the given quantity to the list of other quantities.

        name: Name of the quantity.
        data: Data of the quantity (raw, as a dict from the output file).
        """
        if name in self.SPECIAL_TREATMENT:
            o = self.SPECIAL_TREATMENT[name](name=name, data=data, grid=self.grid, output=self.output, momentumgrid=self.momentumgrid)
        else:
            o = OtherQuantity(name=name, data=data, grid=self.grid, output=self.output, momentumgrid=self.momentumgrid)

        setattr(self, name, o)
        self.quantities[name] = o


    def setQuantities(self, quantities):
        """
        Add a list of other quantities to this handler.
        """
        for oqn in quantities:
            self.setQuantity(name=oqn, data=quantities[oqn])


