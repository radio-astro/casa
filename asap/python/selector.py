import re
import math
import string
from asap._asap import selector as _selector, srctype
from asap.utils import unique, _to_list

class selector(_selector):
    """
    A selection object to be applied to scantables to restrict the
    scantables to specific rows.
    """
    fields = ["pols", "ifs", "beams", "scans", "cycles", "name", "query", "types", "rows"]

    def __init__(self, *args, **kw):
        if len(args) == 1:
            if isinstance(args[0], self.__class__) \
               or isinstance(args[0], _selector):
                _selector.__init__(self, args[0])
            else:
                raise TypeError("Argument can only be a selector object")
        else:
            _selector.__init__(self)
            for k,v  in kw.items():
                if k in self.fields:
                    func = getattr(self, "set_%s" % k)
                    func(v)

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
            selection.set_query("SRCTYPE == PSOFF AND INTERVAL > 60.0")
        """
        rx = re.compile("((SRCTYPE *[!=][=] *)([a-zA-Z.]+))", re.I)
        for r in rx.findall(query):
            sval = None
            stype = r[-1].lower()
            if stype.find('srctype.') == -1:
                stype = ".".join(["srctype", stype])
            try:
                sval = eval(stype)
                sval = "%s%d" % (r[1], sval)
            except:
                continue
            query = query.replace(r[0], sval)
        taql = "SELECT FROM $1 WHERE " + query
        self._settaql(taql)

    def set_order(self, order):
        """
        Set the order the scantable should be sorted by.
        Parameters:
            order:    The list of column names to sort by in order
        """
        self._setorder(order)

    def set_rows(self, rows=[]):
        """
        Set a sequence of row numbers (0-based). Power users Only!
        NOTICE row numbers can be changed easily by sorting,
        prior selection, etc.
        Parameters:
            rows:    a list of integers. Default [] is to unset the selection.
        """
        vec = _to_list(rows, int)
        if isinstance(vec,list):
            self._setrows(vec)
        else:
            raise TypeError('Unknown row number type. Use lists of integers.')

    def set_types(self, types=[]):
        """
        Set a sequence of source types.
        Parameters:
            types:    a list of integers. Default [] is to unset the selection.
        """
        vec = _to_list(types, int)
        if isinstance(vec,list):
            self._settypes(vec)
        else:
            raise TypeError('Unknown row number type. Use lists of integers.')

    def set_msselection_field(self, selection):
        """
        Set a field selection in msselection syntax. The msselection
        suppports the following syntax:

        pattern match:
            - UNIX style pattern match for source name using '*'
              (compatible with set_name)

        field id selection:
            - simple number in string ('0', '1', etc.)
            - range specification using '~' ('0~1', etc.)
            - range specification using '>' or '<' in combination
              with '=' ('>=1', '<3', etc.)

        comma separated multiple selection:
            - selections can be combined by using ',' ('0,>1',
              'mysource*,2~4', etc.)
        """
        selection_list =  map(string.strip, selection.split(','))
        query_list = list(self.generate_query(selection_list))
        if len(query_list) > 0:
            query = 'SELECT FROM $1 WHERE ' + ' || '.join(query_list)
            self._settaql(query)

    def generate_query(self, selection_list):
        for s in selection_list:
            if s.isdigit() or re.match('^[<>]=?[0-9]*$', s) or \
                    re.match('^[0-9]+~[0-9]+$', s):
                #print '"%s" is ID selection using < or <='%(s)
                a = FieldIdRegexGenerator(s)
                yield '(%s)'%(a.get_regex())
            elif len(s) > 0:
                #print '"%s" is UNIX style pattern match'%(s)
                yield '(SRCNAME == pattern(\'%s\'))'%(s)
        
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
    def get_types(self):
        return list(self._gettypes())
    def get_rows(self):
        return list(self._getrows())
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
             "SRCTYPE": self.get_types(),
             "ROWS": self.get_rows(),
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
        if self.is_empty():
            return selector(other)
        elif other.is_empty():
            return selector(self)
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

class FieldIdRegexGenerator(object):
    def __init__(self, pattern):
        if pattern.isdigit():
            self.regex = 'FIELDNAME == regex(\'.+__%s$\')'%(pattern)
        else:
            self.regex = None
            ineq = None
            if pattern.find('<') >= 0:
                ineq = '<'
                s = pattern.strip().lstrip(ineq).lstrip('=')
                if not s.isdigit():
                    raise RuntimeError('Invalid syntax: %s'%(pattern))
                self.id = int(s) + (-1 if pattern.find('=') < 0 else 0)
                self.template = string.Template('FIELDNAME == regex(\'.+__${reg}$\')')
            elif pattern.find('>') >= 0:
                ineq = '>'
                s = pattern.strip().lstrip(ineq).lstrip('=')
                if not s.isdigit():
                    raise RuntimeError('Invalid syntax: %s'%(pattern))
                self.id = int(s) + (-1 if pattern.find('=') >= 0 else 0)
                self.template = string.Template('FIELDNAME == regex(\'.+__[0-9]+$\') && FIELDNAME != regex(\'.+__${reg}$\')')
            elif pattern.find('~') >= 0:
                s = map(string.strip, pattern.split('~'))
                if len(s) == 2 and s[0].isdigit() and s[1].isdigit():
                    self.id = [int(s[0])-1,int(s[1])]
                else:
                    raise RuntimeError('Invalid syntax: %s'%(pattern))
                self.template = string.Template('FIELDNAME == regex(\'.+__${reg}$\') && FIELDNAME != regex(\'.+__${optreg}$\')')
            else:
                raise RuntimeError('Invalid syntax: %s'%(pattern))
            #print 'self.id=',self.id

    def get_regex(self):
        if self.regex is not None:
            # 'X'
            return self.regex
        elif isinstance(self.id, list):
            # 'X~Y'
            return self.template.safe_substitute(reg=self.__compile(self.id[1]),
                                                 optreg=self.__compile(self.id[0]))
        else:
            # '<(=)X' or '>(=)X'
            return self.template.safe_substitute(reg=self.__compile(self.id))

    def __compile(self, idx):
        pattern = ''
        if idx >= 0:
            numerics = map(int,list(str(idx)))
            #numerics.reverse()
            num_digits = len(numerics)
            #print 'numerics=',numerics
            if num_digits == 1:
                if numerics[0] == 0:
                    pattern = '0'
                else:
                    pattern = '[0-%s]'%(numerics[0])
            elif num_digits == 2:
                pattern = '(%s)'%('|'.join(
                        list(self.__gen_two_digit_pattern(numerics))))
            elif num_digits == 3:
                pattern = '(%s)'%('|'.join(
                        list(self.__gen_three_digit_pattern(numerics))))
            else:
                raise RuntimeError('ID > 999 is not supported')
        else:
            raise RuntimeError('ID must be >= 0')
        return pattern

    def __gen_two_digit_pattern(self, numerics):
        assert len(numerics) == 2
        yield '[0-9]'
        if numerics[0] == 2:
            yield '1[0-9]'
        elif numerics[0] > 2:
            yield '[1-%s][0-9]'%(numerics[0]-1)
        if numerics[1] == 0:
            yield '%s%s'%(numerics[0],numerics[1])
        else:
            yield '%s[0-%s]'%(numerics[0],numerics[1])

    def __gen_three_digit_pattern(self, numerics):
        assert len(numerics) == 3
        yield '[0-9]'
        yield '[1-9][0-9]'
        if numerics[0] == 2:
            yield '1[0-9][0-9]'
        elif numerics[0] > 2:
            yield '[1-%s][0-9][0-9]'%(numerics[0]-1)
        if numerics[1] == 0:
            if numerics[2] == 0:
                yield '%s00'%(numerics[0])
            else:
                yield '%s0[0-%s]'%(numerics[0],numerics[2])
        else:
            if numerics[1] > 1:
                yield '%s[0-%s][0-9]'%(numerics[0],numerics[1]-1)
            elif numerics[1] == 1:
                yield '%s0[0-9]'%(numerics[0])
            if numerics[0] == 0:
                yield '%s%s%s'%(numerics[0],numerics[1],numerics[2])
            else:
                yield '%s%s[0-%s]'%(numerics[0],numerics[1],numerics[2])
