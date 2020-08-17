# Base class for scalar quantities (single value evolving in time)
#

import matplotlib.pyplot as plt
import numpy as np

from . OutputException import OutputException
from . UnknownQuantity import UnknownQuantity


class ScalarQuantity(UnknownQuantity):
    

    def __init__(self, name, data, grid, output):
        """
        Constructor.

        :param str name:           Name of quantity.
        :param numpy.ndarray:      Data for quanity.
        :param grid:               Grid on which the quantity is defined.
        :param DREAMOutput output: Parent output object.
        """
        super(ScalarQuantity, self).__init__(name=name, data=data, grid=grid, output=output)


    def __repr__(self):
        """
        Convert this object to an "official" string.
        """
        s = self.__str__() + "\n"
        s += self.dumps()
        return s


    def __str__(self):
        """
        Convert this object to a string.
        """
        return '({}) Scalar quantity of size NT = {}'.format(self.name, self.data.shape[0])


    def __getitem__(self, index):
        """
        Direct access to data.
        """
        return self.data[index]


    def plot(self, ax=None, show=None, t=None):
        """
        Generate a plot of the time evolution of this quantity.

        :param matplotlib.axes.Axes ax: Axes object to create the plot on.
        :param bool show:               If ``True``, calls ``matplotlib.pyplot.show(block=False)`` after creating the plot.
        :param t:                       Selection of time points to plot. Can be a list, numpy array, slice or ``None``.
        """
        genax = ax is None

        if genax:
            ax = plt.axes()

            if show is None:
                show = True
        
        if t is None:
            t = slice(None)

        ax.plot(self.grid.t[t], self.data[t])
        ax.set_xlabel(r'Time $t$ (s)')
        ax.set_ylabel(r'{}'.format(self.getTeXName()))

        if show:
            plt.show(block=False)

        return ax


    def dumps(self, t=None):
        """
        Dumps the data of this quantity as a string.
        
        :param t: Selection of time points to dump.
        """
        return self.data[t].__str__()


    def print(self, t=None):
        """
        Print the data in this quantity.
        """
        print(self.dumps(t))


