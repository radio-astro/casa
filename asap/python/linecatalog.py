"""\
A representation of a spectral line catalog.
"""
__revision__ = "$Revision: 1865 $"
from asap._asap import linecatalog as lcbase
from asap.logging import asaplog
import os

class linecatalog(lcbase):
    """\
    This class is a warpper for line catalogs. These can be either ASCII tables
    or the tables saved from this class.

    ASCII tables have the following restrictions:

        * Comments can be present through lines starting with '#'.

        * The first column contains the name of the Molecule. This can't contain
          spaces, if it does it has to be wrapped in double-quotes.

        * The second column contains the frequency of the transition.

        * The third column contains the error in frequency.

        * The fourth column contains a value describing the intensity.

    """

    def __init__(self, name):
        fpath = os.path.abspath(os.path.expandvars(os.path.expanduser(name)))
        if os.path.exists(fpath):
            lcbase.__init__(self, fpath)
        else:
            msg = "File '%s' not found" % fpath
            raise IOError(msg)

    def __repr__(self):
        return lcbase.summary(self, -1)

    def summary(self):
        """
        Print the contents of the table.
        """
        try:
            from IPython.genutils import page as pager
        except ImportError:
            from pydoc import pager
        pager(lcbase.summary(self, -1))

    def set_name(self, name, mode="pattern"):
        """\
        Set a name restriction on the table. This can be a standard unix-style
        pattern or a regular expression.

        Parameters:

            name:       the name patterrn/regex

            mode:       the matching mode, i.e. "pattern" (default) or "regex"

        """
        validmodes = "pattern regex".split()
        if not mode.lower() in validmodes:
            return
        lcbase.set_name(self, name, mode)

    def set_frequency_limits(self, fmin=1.0, fmax=120.0, unit="GHz"):
        """\
        Set frequency limits on the table.

        Parameters:

            fmin:       the lower bound

            fmax:       the upper bound

            unit:       the frequency unit (default "GHz")

        .. note:: The underlying table contains frequency values in MHz
        """
        base = { "GHz": 1000.0, "MHz": 1.0 }
        if not base.has_key(unit):
            raise ValueError("%s is not a valid unit." % unit)
        # the table conatins values in MHz
        lcbase.set_frequency_limits(self, fmin*base[unit], fmax*base[unit])

    def set_strength_limits(self, smin, smax):
        """\
        Set line strength limits on the table (arbitrary units)

        Parameters:

            smin:       the lower bound

            smax:       the upper bound

        """
        lcbase.set_strength_limits(self, smin, smax)

    def save(self, name, overwrite=False):
        """\
        Save the subset of the table to disk. This uses an internal data format
        and can be read in again.
        """
        name = os.path.expanduser(os.path.expandvars(name))
        if os.path.isfile(name) or os.path.isdir(name):
            if not overwrite:
                msg = "File %s exists." % name
                raise IOError(msg)
        lcbase.save(self, name)

    def reset(self):
        """\
        Reset the table to its initial state, i.e. undo all calls to ``set_``.
        """
        lcbase.reset(self)

    def get_row(self, row=0):
        """\
        Get the values in a specified row of the table.

        Parameters:

              row:        the row to retrieve

        """
        if row < 0 or row > len(self)-1:
            raise IndexError("Row index out of bounds.")
        freq = lcbase.get_frequency(self, row)
        name = lcbase.get_name(self, row)
        return { 'name':name, 'value': freq }

    def __len__(self):
        return self.nrow()

    def __getitem__(self, k):
        if k < 0:
            k = self.nrow()-k
        return self.get_row(k)
