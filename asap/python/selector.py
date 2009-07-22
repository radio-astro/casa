from asap._asap import selector as _selector
from asap import unique, _to_list

class selector(_selector):
    """
    A selection object to be applied to scantables to restrict the
    scantables to specific rows.
    """
    def __init(self):
        _selector.__init__(self)

    def reset(self):
        """
        Unset all selections.
        """
        self._reset()

    def is_empty(self):
        """
        Has anything been set?
        """
        return self._empty()

    def set_polarisations(self, pols=[]):
        """
        Set the polarisations to be selected in the scantable.
        Parameters:
             pols:     a list of integers of 0-3, or strings, e.g ["I","Q"].
                       Default [] is no selection
        Example:
             sel = selector()
             # These are equivalent if data is 'linear'
             sel.set_polarisations(["XX","Re(XY)"])
             sel.set_polarisations([0,2])
             # reset the polarisation selection
             sel.set_polarisations()

        """
        vec = _to_list(pols, str) or _to_list(pols, int)
        if isinstance(vec, list): # is an empty and/or valid vector
            if len(vec) and isinstance(vec[-1],str):
                self._setpolstrings(vec)
                return
            self._setpols(vec)
        else:
            raise TypeError('Unknown pol type. Please use [0,1...] or ["XX","YY"...]')
    
    # for the americans
    set_polarizations = set_polarisations
    # for the lazy
    set_pols = set_polarisations

    def set_ifs(self, ifs=[]):
        """
        Set a sequence of IF numbers (0-based).
        Parameters:
            ifs:    a list of integers. Default [] is to unset the selection.
        """
        vec = _to_list(ifs, int)
        if isinstance(vec,list):
            self._setifs(vec)
        else:
            raise TypeError('Unknown IFno type. Use lists of integers.')

    def set_scans(self, scans=[]):
        """
        Set a sequence of Scan numbers (0-based).
        Parameters:
            scans:    a list of integers. Default [] is to unset the selection.
        """
        vec = _to_list(scans, int)
        if isinstance(vec,list):
            self._setscans(vec)
        else:
            raise TypeError('Unknown Scan number type. Use lists of integers.')

    def set_beams(self, beams=[]):
        """
        Set a sequence of Beam numbers (0-based).
        Parameters:
            beams:    a list of integers. Default [] is to unset the selection.
        """
        vec = _to_list(beams, int)
        if isinstance(vec,list):
            self._setbeams(vec)
        else:
            raise TypeError('Unknown Beam number type. Use lists of integers.')

    def set_cycles(self, cycles=[]):
        """
        Set a sequence of IF numbers (0-based).
        Parameters:
            cycless:    a list of integers. Default [] is to unset the selection.
        """
        vec = _to_list(cycles, int)
        if isinstance(vec,list):
            self._setcycles(vec)
        else:
            raise TypeError('Unknown Cycle number type. Use lists of integers.')


    def set_name(self, name):
        """
        Set a selection based on a name. This can be a unix pattern , e.g. "*_R"
        Parameters:
            name:    a string containing a source name or pattern
        Examples:
            # select all reference scans which start with "Orion"
            selection.set_name("Orion*_R")
        """
        if isinstance(name, str):
            self._setname(name)
        else:
            raise TypeError('name must be a string')

    def set_tsys(self, tsysmin=0.0, tsysmax=None):
        """
        Select by Tsys range.
        Parameters:
            tsysmin:     the lower threshold. Default 0.0
            tsysmax:     the upper threshold. Default None.
        Examples:
            # select all spectra with Tsys <= 500.0
            selection.set_tsys(tsysmax=500.0)

        """
        taql =  "SELECT FROM $1 WHERE TSYS[0] >= %f" % (tsysmin)
        if isinstance(tsysmax, float):
            taql = taql + " AND TSYS[0] <= %f" % ( tsysmax)
        self._settaql(taql)

    def set_query(self, query):
        """
        Select by Column query. Power users only!
        Example:
            # select all off scans with integration times over 60 seconds.
            selection.set_query("SRCTYPE == 1 AND INTERVAL > 60.0")
        """
        taql = "SELECT FROM $1 WHERE " + query
        self._settaql(taql)

    def set_order(self, order):
        """
        Set the order the scantable should be sorted by.
        Parameters:
            order:    The list of column names to sort by in order
        """
        self._setorder(order)

    def get_scans(self):
        return list(self._getscans())
    def get_cycles(self):
        return list(self._getcycles())
    def get_beams(self):
        return list(self._getbeams())
    def get_ifs(self):
        return list(self._getifs())
    def get_pols(self):
        return list(self._getpols())
    def get_poltypes(self):
        return list(self._getpoltypes())
    def get_order(self):
        return list(self._getorder())
    def get_query(self):
	prefix = "SELECT FROM $1 WHERE "
        return self._gettaql().replace(prefix, "")

    def get_name(self):
        print "NYI"
        s = self._gettaql()
        return
    def __str__(self):
	out = ""
	d = {"SCANNO": self.get_scans(),
	     "CYCLENO": self.get_cycles(),
	     "BEAMNO": self.get_beams(),
	     "IFNO": self.get_ifs(),
	     "Pol Type": self.get_poltypes(),
	     "POLNO": self.get_pols(),
	     "QUERY": self.get_query(),
	     "Sort Order": self.get_order()
	     }
	for k,v in d.iteritems():
	    if v:
		out += "%s: %s\n" % (k, v)
	if len(out):
	    return out[:-1]
	else:
	    return out
    def __add__(self, other):
        """
        Merge two selections.
        """
        union = selector()
        gets = [[self._getscans(), other._getscans(), union._setscans],
                [self._getcycles(), other._getcycles(),union._setcycles],
                [self._getbeams(), other._getbeams(), union._setbeams],
                [self._getifs(), other._getifs(), union._setifs],
                [self._getpols(), other._getpols(), union._setpols]]
        for v in gets:
            vec = list(v[0]+v[1])
            vec.sort()
            v[2](unique(vec))
        q = other.get_query()
        qs = self.get_query()
        if len(q) and len(qs):
            union.set_query(qs +" AND " + q)
        else:
            if len(q):
                union.set_query(q)
            elif len(qs):
                union.set_query(qs)
        return union
