"""This module defines the scantable class."""

import os
import re
import tempfile
import numpy
try:
    from functools import wraps as wraps_dec
except ImportError:
    from asap.compatibility import wraps as wraps_dec

from asap.env import is_casapy
from asap._asap import Scantable
from asap._asap import filler, msfiller
from asap.parameters import rcParams
from asap.logging import asaplog, asaplog_post_dec
from asap.selector import selector
from asap.linecatalog import linecatalog
from asap.coordinate import coordinate
from asap.utils import _n_bools, mask_not, mask_and, mask_or, page
from asap.asapfitter import fitter

def preserve_selection(func):
    @wraps_dec(func)
    def wrap(obj, *args, **kw):
        basesel = obj.get_selection()
        try:
            val = func(obj, *args, **kw)
        finally:
            obj.set_selection(basesel)
        return val
    return wrap

def is_scantable(filename):
    """Is the given file a scantable?

    Parameters:

        filename: the name of the file/directory to test

    """
    if ( os.path.isdir(filename)
         and os.path.exists(filename+'/table.info')
         and os.path.exists(filename+'/table.dat') ):
        f=open(filename+'/table.info')
        l=f.readline()
        f.close()
        match_pattern = '^Type = (Scantable)? *$'
        if re.match(match_pattern,l):
            return True
        else:
            return False
    else:
        return False
##     return (os.path.isdir(filename)
##             and not os.path.exists(filename+'/table.f1')
##             and os.path.exists(filename+'/table.info'))

def is_ms(filename):
    """Is the given file a MeasurementSet?

    Parameters:

        filename: the name of the file/directory to test

    """
    if ( os.path.isdir(filename)
         and os.path.exists(filename+'/table.info')
         and os.path.exists(filename+'/table.dat') ):
        f=open(filename+'/table.info')
        l=f.readline()
        f.close()
        if ( l.find('Measurement Set') != -1 ):
            return True
        else:
            return False
    else:
        return False

def normalise_edge_param(edge):
    """\
    Convert a given edge value to a one-dimensional array that can be
    given to baseline-fitting/subtraction functions.
    The length of the output value will be an even because values for
    the both sides of spectra are to be contained for each IF. When
    the length is 2, the values will be applied to all IFs. If the length
    is larger than 2, it will be 2*ifnos(). 
    Accepted format of edge include:
            * an integer - will be used for both sides of spectra of all IFs.
                  e.g. 10 is converted to [10,10]
            * an empty list/tuple [] - converted to [0, 0] and used for all IFs. 
            * a list/tuple containing an integer - same as the above case.
                  e.g. [10] is converted to [10,10]
            * a list/tuple containing two integers - will be used for all IFs. 
                  e.g. [5,10] is output as it is. no need to convert.
            * a list/tuple of lists/tuples containing TWO integers -
                  each element of edge will be used for each IF.
                  e.g. [[5,10],[15,20]] - [5,10] for IF[0] and [15,20] for IF[1].
                  
                  If an element contains the same integer values, the input 'edge'
                  parameter can be given in a simpler shape in the following cases:
                      ** when len(edge)!=2
                          any elements containing the same values can be replaced
                          to single integers.
                          e.g. [[15,15]] can be simplified to [15] (or [15,15] or 15 also). 
                          e.g. [[1,1],[2,2],[3,3]] can be simplified to [1,2,3]. 
                      ** when len(edge)=2
                          care is needed for this case: ONLY ONE of the
                          elements can be a single integer,
                          e.g. [[5,5],[10,10]] can be simplified to [5,[10,10]]
                               or [[5,5],10], but can NOT be simplified to [5,10].
                               when [5,10] given, it is interpreted as
                               [[5,10],[5,10],[5,10],...] instead, as shown before. 
    """
    from asap import _is_sequence_or_number as _is_valid
    if isinstance(edge, list) or isinstance(edge, tuple):
        for edgepar in edge:
            if not _is_valid(edgepar, int):
                raise ValueError, "Each element of the 'edge' tuple has \
                                   to be a pair of integers or an integer."
            if isinstance(edgepar, list) or isinstance(edgepar, tuple):
                if len(edgepar) != 2:
                    raise ValueError, "Each element of the 'edge' tuple has \
                                       to be a pair of integers or an integer."
    else:
        if not _is_valid(edge, int):
            raise ValueError, "Parameter 'edge' has to be an integer or a \
                               pair of integers specified as a tuple. \
                               Nested tuples are allowed \
                               to make individual selection for different IFs."
        

    if isinstance(edge, int):
        edge = [ edge, edge ]                 # e.g. 3   => [3,3]
    elif isinstance(edge, list) or isinstance(edge, tuple):
        if len(edge) == 0:
            edge = [0, 0]                     # e.g. []  => [0,0]
        elif len(edge) == 1:
            if isinstance(edge[0], int):
                edge = [ edge[0], edge[0] ]   # e.g. [1] => [1,1]

    commonedge = True
    if len(edge) > 2: commonedge = False
    else:
        for edgepar in edge:
            if isinstance(edgepar, list) or isinstance(edgepar, tuple):
                commonedge = False
                break

    if commonedge:
        if len(edge) > 1:
            norm_edge = edge
        else:
            norm_edge = edge + edge
    else:
        norm_edge = []
        for edgepar in edge:
            if isinstance(edgepar, int):
                norm_edge += [edgepar, edgepar]
            else:
                norm_edge += edgepar

    return norm_edge

def raise_fitting_failure_exception(e):
    msg = "The fit failed, possibly because it didn't converge."
    if rcParams["verbose"]:
        asaplog.push(str(e))
        asaplog.push(str(msg))
    else:
        raise RuntimeError(str(e)+'\n'+msg)

def pack_progress_params(showprogress, minnrow):
    return str(showprogress).lower() + ',' + str(minnrow)

def pack_blinfo(blinfo, maxirow):
    """\
    convert a dictionary or a list of dictionaries of baseline info
    into a list of comma-separated strings. 
    """
    if isinstance(blinfo, dict):
        res = do_pack_blinfo(blinfo, maxirow)
        return [res] if res != '' else []
    elif isinstance(blinfo, list) or isinstance(blinfo, tuple):
        res = []
        for i in xrange(len(blinfo)):
            resi = do_pack_blinfo(blinfo[i], maxirow)
            if resi != '':
                res.append(resi)
    return res

def do_pack_blinfo(blinfo, maxirow):
    """\
    convert a dictionary of baseline info for a spectrum into
    a comma-separated string. 
    """
    dinfo = {}
    for key in ['row', 'blfunc', 'masklist']:
        if blinfo.has_key(key):
            val = blinfo[key]
            if key == 'row':
                irow = val
            if isinstance(val, list) or isinstance(val, tuple):
                slval = []
                for i in xrange(len(val)):
                    if isinstance(val[i], list) or isinstance(val[i], tuple):
                        for j in xrange(len(val[i])):
                            slval.append(str(val[i][j]))
                    else:
                        slval.append(str(val[i]))
                sval = ",".join(slval)
            else:
                sval = str(val)

            dinfo[key] = sval
        else:
            raise ValueError("'"+key+"' is missing in blinfo.")

    if irow >= maxirow: return ''
    
    for key in ['order', 'npiece', 'nwave']:
        if blinfo.has_key(key):
            val = blinfo[key]
            if isinstance(val, list) or isinstance(val, tuple):
                slval = []
                for i in xrange(len(val)):
                    slval.append(str(val[i]))
                sval = ",".join(slval)
            else:
                sval = str(val)
            dinfo[key] = sval

    blfunc = dinfo['blfunc']
    fspec_keys = {'poly': 'order', 'chebyshev': 'order', 'cspline': 'npiece', 'sinusoid': 'nwave'}

    fspec_key = fspec_keys[blfunc]
    if not blinfo.has_key(fspec_key):
        raise ValueError("'"+fspec_key+"' is missing in blinfo.")

    clip_params_n = 0
    for key in ['clipthresh', 'clipniter']:
        if blinfo.has_key(key):
            clip_params_n += 1
            dinfo[key] = str(blinfo[key])

    if clip_params_n == 0:
        dinfo['clipthresh'] = '0.0'
        dinfo['clipniter']  = '0'
    elif clip_params_n != 2:
        raise ValueError("both 'clipthresh' and 'clipniter' must be given for n-sigma clipping.")

    lf_params_n = 0
    for key in ['thresh', 'edge', 'chan_avg_limit']:
        if blinfo.has_key(key):
            lf_params_n += 1
            val = blinfo[key]
            if isinstance(val, list) or isinstance(val, tuple):
                slval = []
                for i in xrange(len(val)):
                    slval.append(str(val[i]))
                sval = ",".join(slval)
            else:
                sval = str(val)
            dinfo[key] = sval

    if lf_params_n == 3:
        dinfo['use_linefinder'] = 'true'
    elif lf_params_n == 0:
        dinfo['use_linefinder'] = 'false'
        dinfo['thresh']         = ''
        dinfo['edge']           = ''
        dinfo['chan_avg_limit'] = ''
    else:
        raise ValueError("all of 'thresh', 'edge' and 'chan_avg_limit' must be given to use linefinder.")
    
    slblinfo = [dinfo['row'], blfunc, dinfo[fspec_key], dinfo['masklist'], \
                dinfo['clipthresh'], dinfo['clipniter'], \
                dinfo['use_linefinder'], dinfo['thresh'], dinfo['edge'], dinfo['chan_avg_limit']]
    
    return ":".join(slblinfo)

def parse_fitresult(sres):
    """\
    Parse the returned value of apply_bltable() or sub_baseline() and
    extract row number, the best-fit coefficients and rms, then pack
    them into a dictionary.
    The input value is generated by Scantable::packFittingResults() and
    formatted as 'row:coeff[0],coeff[1],..,coeff[n-1]:rms'. 
    """
    res = []
    for i in xrange(len(sres)):
        (srow, scoeff, srms) = sres[i].split(":")
        row = int(srow)
        rms = float(srms)
        lscoeff = scoeff.split(",")
        coeff = []
        for j in xrange(len(lscoeff)):
            coeff.append(float(lscoeff[j]))
        res.append({'row': row, 'coeff': coeff, 'rms': rms})

    return res

class scantable(Scantable):
    """\
        The ASAP container for scans (single-dish data).
    """

    @asaplog_post_dec
    def __init__(self, filename, average=None, unit=None, parallactify=None,
                 **args):
        """\
        Create a scantable from a saved one or make a reference

        Parameters:

            filename:     the name of an asap table on disk
                          or
                          the name of a rpfits/sdfits/ms file
                          (integrations within scans are auto averaged
                          and the whole file is read) or
                          [advanced] a reference to an existing scantable

            average:      average all integrations withinb a scan on read.
                          The default (True) is taken from .asaprc.

            unit:         brightness unit; must be consistent with K or Jy.
                          Over-rides the default selected by the filler
                          (input rpfits/sdfits/ms) or replaces the value
                          in existing scantables

            antenna:      for MeasurementSet input data only:
                          Antenna selection. integer (id) or string 
                          (name or id).

            parallactify: Indicate that the data had been parallactified. 
                          Default (false) is taken from rc file.

            getpt:        Whether to import direction from MS/POINTING
                          table properly or not.
                          This is effective only when filename is MS.
                          The default (True) is to import direction
                          from MS/POINTING.
        """
        if average is None:
            average = rcParams['scantable.autoaverage']
        parallactify = parallactify or rcParams['scantable.parallactify']
        varlist = vars()
        from asap._asap import stmath
        self._math = stmath( rcParams['insitu'] )
        if isinstance(filename, Scantable):
            Scantable.__init__(self, filename)
        else:
            if isinstance(filename, str):
                filename = os.path.expandvars(filename)
                filename = os.path.expanduser(filename)
                if not os.path.exists(filename):
                    s = "File '%s' not found." % (filename)
                    raise IOError(s)
                if is_scantable(filename):
                    ondisk = rcParams['scantable.storage'] == 'disk'
                    Scantable.__init__(self, filename, ondisk)
                    if unit is not None:
                        self.set_fluxunit(unit)
                    if average:
                        self._assign( self.average_time( scanav=True ) )
                    # do not reset to the default freqframe
                    #self.set_freqframe(rcParams['scantable.freqframe'])
                elif is_ms(filename):
                    # Measurement Set
                    opts={'ms': {}}
                    mskeys=['getpt','antenna']
                    for key in mskeys:
                        if key in args.keys():
                            opts['ms'][key] = args[key]
                    self._fill([filename], unit, average, opts)
                elif os.path.isfile(filename):
                    opts={'nro': {}}
                    nrokeys=['freqref']
                    for key in nrokeys:
                        if key in args.keys():
                            opts['nro'][key] = args[key]
                    self._fill([filename], unit, average, opts)
                    # only apply to new data not "copy constructor"
                    self.parallactify(parallactify)
                else:
                    msg = "The given file '%s'is not a valid " \
                          "asap table." % (filename)
                    raise IOError(msg)
            elif (isinstance(filename, list) or isinstance(filename, tuple)) \
                  and isinstance(filename[-1], str):
                self._fill(filename, unit, average)
        self.parallactify(parallactify)
        self._add_history("scantable", varlist)

    @asaplog_post_dec
    def save(self, name=None, format=None, overwrite=False):
        """\
        Store the scantable on disk. This can be an asap (aips++) Table,
        SDFITS or MS2 format.

        Parameters:

            name:        the name of the outputfile. For format 'ASCII'
                         this is the root file name (data in 'name'.txt
                         and header in 'name'_header.txt)

            format:      an optional file format. Default is ASAP.
                         Allowed are:

                            * 'ASAP' (save as ASAP [aips++] Table),
                            * 'SDFITS' (save as SDFITS file)
                            * 'ASCII' (saves as ascii text file)
                            * 'MS2' (saves as an casacore MeasurementSet V2)
                            * 'FITS' (save as image FITS - not readable by 
                                      class)
                            * 'CLASS' (save as FITS readable by CLASS)

            overwrite:   If the file should be overwritten if it exists.
                         The default False is to return with warning
                         without writing the output. USE WITH CARE.

        Example::

            scan.save('myscan.asap')
            scan.save('myscan.sdfits', 'SDFITS')

        """
        from os import path
        format = format or rcParams['scantable.save']
        suffix = '.'+format.lower()
        if name is None or name == "":
            name = 'scantable'+suffix
            msg = "No filename given. Using default name %s..." % name
            asaplog.push(msg)
        name = path.expandvars(name)
        if path.isfile(name) or path.isdir(name):
            if not overwrite:
                msg = "File %s exists." % name
                raise IOError(msg)
        format2 = format.upper()
        if format2 == 'ASAP':
            self._save(name)
        elif format2 == 'MS2':
            msopt = {'ms': {'overwrite': overwrite } }
            from asap._asap import mswriter
            writer = mswriter( self )
            writer.write( name, msopt ) 
        else:
            from asap._asap import stwriter as stw
            writer = stw(format2)
            writer.write(self, name)
        return

    def copy(self):
        """Return a copy of this scantable.

        *Note*:

            This makes a full (deep) copy. scan2 = scan1 makes a reference.

        Example::

            copiedscan = scan.copy()

        """
        sd = scantable(Scantable._copy(self))
        return sd

    def drop_scan(self, scanid=None):
        """\
        Return a new scantable where the specified scan number(s) has(have)
        been dropped.

        Parameters:

            scanid:    a (list of) scan number(s)

        """
        from asap import _is_sequence_or_number as _is_valid
        from asap import _to_list
        from asap import unique
        if not _is_valid(scanid):
            raise RuntimeError( 'Please specify a scanno to drop from the'
                                ' scantable' )
        scanid = _to_list(scanid)
        allscans = unique([ self.getscan(i) for i in range(self.nrow())])
        for sid in scanid: allscans.remove(sid)
        if len(allscans) == 0:
            raise ValueError("Can't remove all scans")
        sel = selector(scans=allscans)
        return self._select_copy(sel)

    def _select_copy(self, selection):
        orig = self.get_selection()
        self.set_selection(orig+selection)
        cp = self.copy()
        self.set_selection(orig)
        return cp

    def get_scan(self, scanid=None):
        """\
        Return a specific scan (by scanno) or collection of scans (by
        source name) in a new scantable.

        *Note*:

            See scantable.drop_scan() for the inverse operation.

        Parameters:

            scanid:    a (list of) scanno or a source name, unix-style
                       patterns are accepted for source name matching, e.g.
                       '*_R' gets all 'ref scans

        Example::

            # get all scans containing the source '323p459'
            newscan = scan.get_scan('323p459')
            # get all 'off' scans
            refscans = scan.get_scan('*_R')
            # get a susbset of scans by scanno (as listed in scan.summary())
            newscan = scan.get_scan([0, 2, 7, 10])

        """
        if scanid is None:
            raise RuntimeError( 'Please specify a scan no or name to '
                                'retrieve from the scantable' )
        try:
            bsel = self.get_selection()
            sel = selector()
            if type(scanid) is str:
                sel.set_name(scanid)
                return self._select_copy(sel)
            elif type(scanid) is int:
                sel.set_scans([scanid])
                return self._select_copy(sel)
            elif type(scanid) is list:
                sel.set_scans(scanid)
                return self._select_copy(sel)
            else:
                msg = "Illegal scanid type, use 'int' or 'list' if ints."
                raise TypeError(msg)
        except RuntimeError:
            raise

    def __str__(self):
        tempFile = tempfile.NamedTemporaryFile()
        Scantable._summary(self, tempFile.name)
        tempFile.seek(0)
        asaplog.clear()
        return tempFile.file.read()

    @asaplog_post_dec
    def summary(self, filename=None):
        """\
        Print a summary of the contents of this scantable.

        Parameters:

            filename:    the name of a file to write the putput to
                         Default - no file output

        """
        if filename is not None:
            if filename is "":
                filename = 'scantable_summary.txt'
            from os.path import expandvars, isdir
            filename = expandvars(filename)
            if isdir(filename):
                msg = "Illegal file name '%s'." % (filename)
                raise IOError(msg)
        else:
            filename = ""
        Scantable._summary(self, filename)

    def get_spectrum(self, rowno):
        """Return the spectrum for the current row in the scantable as a list.

        Parameters:

             rowno:   the row number to retrieve the spectrum from

        """
        return self._getspectrum(rowno)

    def get_mask(self, rowno):
        """Return the mask for the current row in the scantable as a list.

        Parameters:

             rowno:   the row number to retrieve the mask from

        """
        return self._getmask(rowno)

    def set_spectrum(self, spec, rowno):
        """Set the spectrum for the current row in the scantable.

        Parameters:

             spec:   the new spectrum

             rowno:  the row number to set the spectrum for

        """
        assert(len(spec) == self.nchan(self.getif(rowno)))
        return self._setspectrum(spec, rowno)

    def get_coordinate(self, rowno):
        """Return the (spectral) coordinate for a a given 'rowno'.

        *Note*:

            * This coordinate is only valid until a scantable method modifies
              the frequency axis.
            * This coordinate does contain the original frequency set-up
              NOT the new frame. The conversions however are done using the user
              specified frame (e.g. LSRK/TOPO). To get the 'real' coordinate,
              use scantable.freq_align first. Without it there is no closure,
              i.e.::

                  c = myscan.get_coordinate(0)
                  c.to_frequency(c.get_reference_pixel()) != c.get_reference_value()

        Parameters:

             rowno:    the row number for the spectral coordinate

        """
        return coordinate(Scantable.get_coordinate(self, rowno))

    def get_selection(self):
        """\
        Get the selection object currently set on this scantable.

        Example::

            sel = scan.get_selection()
            sel.set_ifs(0)              # select IF 0
            scan.set_selection(sel)     # apply modified selection

        """
        return selector(self._getselection())

    def set_selection(self, selection=None, **kw):
        """\
        Select a subset of the data. All following operations on this scantable
        are only applied to thi selection.

        Parameters:

            selection:    a selector object (default unset the selection), or
                          any combination of 'pols', 'ifs', 'beams', 'scans',
                          'cycles', 'name', 'query'

        Examples::

            sel = selector()         # create a selection object
            self.set_scans([0, 3])    # select SCANNO 0 and 3
            scan.set_selection(sel)  # set the selection
            scan.summary()           # will only print summary of scanno 0 an 3
            scan.set_selection()     # unset the selection
            # or the equivalent
            scan.set_selection(scans=[0,3])
            scan.summary()           # will only print summary of scanno 0 an 3
            scan.set_selection()     # unset the selection

        """
        if selection is None:
            # reset
            if len(kw) == 0:
                selection = selector()
            else:
                # try keywords
                for k in kw:
                    if k not in selector.fields:
                        raise KeyError("Invalid selection key '%s', "
                                       "valid keys are %s" % (k, 
                                                              selector.fields))
                selection = selector(**kw)
        self._setselection(selection)

    def get_row(self, row=0, insitu=None):
        """\
        Select a row in the scantable.
        Return a scantable with single row.

        Parameters:

            row:    row no of integration, default is 0.
            insitu: if False a new scantable is returned. Otherwise, the
                    scaling is done in-situ. The default is taken from .asaprc
                    (False)

        """
        if insitu is None: 
            insitu = rcParams['insitu']
        if not insitu:
            workscan = self.copy()
        else:
            workscan = self
        # Select a row
        sel = selector()
        sel.set_rows([row])
        workscan.set_selection(sel)
        if not workscan.nrow() == 1:
            msg = "Could not identify single row. %d rows selected." \
                % (workscan.nrow())
            raise RuntimeError(msg)
        if insitu:
            self._assign(workscan)
        else:
            return workscan

    @asaplog_post_dec
    def stats(self, stat='stddev', mask=None, form='3.3f', row=None):
        """\
        Determine the specified statistic of the current beam/if/pol
        Takes a 'mask' as an optional parameter to specify which
        channels should be excluded.

        Parameters:

            stat:    'min', 'max', 'min_abc', 'max_abc', 'sumsq', 'sum',
                     'mean', 'var', 'stddev', 'avdev', 'rms', 'median'

            mask:    an optional mask specifying where the statistic
                     should be determined.

            form:    format string to print statistic values

            row:     row number of spectrum to process.
                     (default is None: for all rows)

        Example:
            scan.set_unit('channel')
            msk = scan.create_mask([100, 200], [500, 600])
            scan.stats(stat='mean', mask=m)

        """
        mask = mask or []
        if not self._check_ifs():
            raise ValueError("Cannot apply mask as the IFs have different "
                             "number of channels. Please use setselection() "
                             "to select individual IFs")
        rtnabc = False
        if stat.lower().endswith('_abc'): rtnabc = True
        getchan = False
        if stat.lower().startswith('min') or stat.lower().startswith('max'):
            chan = self._math._minmaxchan(self, mask, stat)
            getchan = True
            statvals = []
        if not rtnabc:
            if row == None:
                statvals = self._math._stats(self, mask, stat)
            else:
                statvals = self._math._statsrow(self, mask, stat, int(row))

        #def cb(i):
        #    return statvals[i]

        #return self._row_callback(cb, stat)

        label=stat
        #callback=cb
        out = ""
        #outvec = []
        sep = '-'*50

        if row == None:
            rows = xrange(self.nrow())
        elif isinstance(row, int):
            rows = [ row ]

        for i in rows:
            refstr = ''
            statunit= ''
            if getchan:
                qx, qy = self.chan2data(rowno=i, chan=chan[i])
                if rtnabc:
                    statvals.append(qx['value'])
                    refstr = ('(value: %'+form) % (qy['value'])+' ['+qy['unit']+'])'
                    statunit= '['+qx['unit']+']'
                else:
                    refstr = ('(@ %'+form) % (qx['value'])+' ['+qx['unit']+'])'

            tm = self._gettime(i)
            src = self._getsourcename(i)
            out += 'Scan[%d] (%s) ' % (self.getscan(i), src)
            out += 'Time[%s]:\n' % (tm)
            if self.nbeam(-1) > 1: out +=  ' Beam[%d] ' % (self.getbeam(i))
            if self.nif(-1) > 1:   out +=  ' IF[%d] ' % (self.getif(i))
            if self.npol(-1) > 1:  out +=  ' Pol[%d] ' % (self.getpol(i))
            #outvec.append(callback(i))
            if len(rows) > 1:
                # out += ('= %'+form) % (outvec[i]) +'   '+refstr+'\n'
                out += ('= %'+form) % (statvals[i]) +'   '+refstr+'\n'
            else:
                # out += ('= %'+form) % (outvec[0]) +'   '+refstr+'\n'
                out += ('= %'+form) % (statvals[0]) +'   '+refstr+'\n'
            out +=  sep+"\n"

        import os
        if os.environ.has_key( 'USER' ):
            usr = os.environ['USER']
        else:
            import commands
            usr = commands.getoutput( 'whoami' )
        tmpfile = '/tmp/tmp_'+usr+'_casapy_asap_scantable_stats'
        f = open(tmpfile,'w')
        print >> f, sep
        print >> f, ' %s %s' % (label, statunit)
        print >> f, sep
        print >> f, out
        f.close()
        f = open(tmpfile,'r')
        x = f.readlines()
        f.close()
        asaplog.push(''.join(x), False)

        return statvals

    def chan2data(self, rowno=0, chan=0):
        """\
        Returns channel/frequency/velocity and spectral value
        at an arbitrary row and channel in the scantable.

        Parameters:

            rowno:   a row number in the scantable. Default is the
                     first row, i.e. rowno=0

            chan:    a channel in the scantable. Default is the first
                     channel, i.e. pos=0

        """
        if isinstance(rowno, int) and isinstance(chan, int):
            qx = {'unit': self.get_unit(),
                  'value': self._getabcissa(rowno)[chan]}
            qy = {'unit': self.get_fluxunit(),
                  'value': self._getspectrum(rowno)[chan]}
            return qx, qy

    def stddev(self, mask=None):
        """\
        Determine the standard deviation of the current beam/if/pol
        Takes a 'mask' as an optional parameter to specify which
        channels should be excluded.

        Parameters:

            mask:    an optional mask specifying where the standard
                     deviation should be determined.

        Example::

            scan.set_unit('channel')
            msk = scan.create_mask([100, 200], [500, 600])
            scan.stddev(mask=m)

        """
        return self.stats(stat='stddev', mask=mask);


    def get_column_names(self):
        """\
        Return a  list of column names, which can be used for selection.
        """
        return list(Scantable.get_column_names(self))

    def get_tsys(self, row=-1):
        """\
        Return the System temperatures.

        Parameters:

            row:    the rowno to get the information for. (default all rows)

        Returns:

            a list of Tsys values for the current selection

        """
        if row > -1:
            return self._get_column(self._gettsys, row)
        return self._row_callback(self._gettsys, "Tsys")

    def get_tsysspectrum(self, row=-1):
        """\
        Return the channel dependent system temperatures.

        Parameters:

            row:    the rowno to get the information for. (default all rows)

        Returns:

            a list of Tsys values for the current selection

        """
        return self._get_column( self._gettsysspectrum, row )

    def set_tsys(self, values, row=-1):
        """\
        Set the Tsys value(s) of the given 'row' or the whole scantable 
        (selection).
        
        Parameters:

            values:    a scalar or list (if Tsys is a vector) of Tsys value(s)
            row:       the row number to apply Tsys values to. 
                       (default all rows)
                       
        """
            
        if not hasattr(values, "__len__"):
            values = [values]
        self._settsys(values, row)

    def get_weather(self, row=-1):
        """\
        Return the weather informations.

        Parameters:

            row:    the rowno to get the information for. (default all rows)

        Returns:

            a dict or list of of dicts of values for the current selection

        """

        values = self._get_column(self._get_weather, row)
        if row > -1:
            return {'temperature': values[0],
                    'pressure': values[1], 'humidity' : values[2],
                    'windspeed' : values[3], 'windaz' : values[4]
                    }
        else:
            out = []
            for r in values:

                out.append({'temperature': r[0],
                            'pressure': r[1], 'humidity' : r[2],
                            'windspeed' : r[3], 'windaz' : r[4]
                    })
            return out

    def _row_callback(self, callback, label):
        out = ""
        outvec = []
        sep = '-'*50
        for i in range(self.nrow()):
            tm = self._gettime(i)
            src = self._getsourcename(i)
            out += 'Scan[%d] (%s) ' % (self.getscan(i), src)
            out += 'Time[%s]:\n' % (tm)
            if self.nbeam(-1) > 1:
                out +=  ' Beam[%d] ' % (self.getbeam(i))
            if self.nif(-1) > 1: out +=  ' IF[%d] ' % (self.getif(i))
            if self.npol(-1) > 1: out +=  ' Pol[%d] ' % (self.getpol(i))
            outvec.append(callback(i))
            out += '= %3.3f\n' % (outvec[i])
            out +=  sep+'\n'

        asaplog.push(sep)
        asaplog.push(" %s" % (label))
        asaplog.push(sep)
        asaplog.push(out)
        asaplog.post()
        return outvec

    def _get_column(self, callback, row=-1, *args):
        """
        """
        if row == -1:
            return [callback(i, *args) for i in range(self.nrow())]
        else:
            if  0 <= row < self.nrow():
                return callback(row, *args)


    def get_time(self, row=-1, asdatetime=False, prec=-1):
        """\
        Get a list of time stamps for the observations.
        Return a datetime object or a string (default) for each
        integration time stamp in the scantable.

        Parameters:

            row:          row no of integration. Default -1 return all rows

            asdatetime:   return values as datetime objects rather than strings

            prec:         number of digits shown. Default -1 to automatic 
                          calculation.
                          Note this number is equals to the digits of MVTime,
                          i.e., 0<prec<3: dates with hh:: only,
                          <5: with hh:mm:, <7 or 0: with hh:mm:ss,
                          and 6> : with hh:mm:ss.tt... (prec-6 t's added)

        """
        from datetime import datetime
        if prec < 0:
            # automagically set necessary precision +1
            prec = 7 - \
                numpy.floor(numpy.log10(numpy.min(self.get_inttime(row))))
            prec = max(6, int(prec))
        else:
            prec = max(0, prec)
        if asdatetime:
            #precision can be 1 millisecond at max
            prec = min(12, prec)

        times = self._get_column(self._gettime, row, prec)
        if not asdatetime:
            return times
        format = "%Y/%m/%d/%H:%M:%S.%f"
        if prec < 7:
            nsub = 1 + (((6-prec)/2) % 3)
            substr = [".%f","%S","%M"]
            for i in range(nsub):
                format = format.replace(substr[i],"")
        if isinstance(times, list):
            return [datetime.strptime(i, format) for i in times]
        else:
            return datetime.strptime(times, format)


    def get_inttime(self, row=-1):
        """\
        Get a list of integration times for the observations.
        Return a time in seconds for each integration in the scantable.

        Parameters:

            row:    row no of integration. Default -1 return all rows.

        """
        return self._get_column(self._getinttime, row)


    def get_sourcename(self, row=-1):
        """\
        Get a list source names for the observations.
        Return a string for each integration in the scantable.
        Parameters:

            row:    row no of integration. Default -1 return all rows.

        """
        return self._get_column(self._getsourcename, row)

    def get_elevation(self, row=-1):
        """\
        Get a list of elevations for the observations.
        Return a float for each integration in the scantable.

        Parameters:

            row:    row no of integration. Default -1 return all rows.

        """
        return self._get_column(self._getelevation, row)

    def get_azimuth(self, row=-1):
        """\
        Get a list of azimuths for the observations.
        Return a float for each integration in the scantable.

        Parameters:
            row:    row no of integration. Default -1 return all rows.

        """
        return self._get_column(self._getazimuth, row)

    def get_parangle(self, row=-1):
        """\
        Get a list of parallactic angles for the observations.
        Return a float for each integration in the scantable.

        Parameters:

            row:    row no of integration. Default -1 return all rows.

        """
        return self._get_column(self._getparangle, row)

    def get_direction(self, row=-1):
        """
        Get a list of Positions on the sky (direction) for the observations.
        Return a string for each integration in the scantable.

        Parameters:

            row:    row no of integration. Default -1 return all rows

        """
        return self._get_column(self._getdirection, row)

    def get_directionval(self, row=-1):
        """\
        Get a list of Positions on the sky (direction) for the observations.
        Return a float for each integration in the scantable.

        Parameters:

            row:    row no of integration. Default -1 return all rows

        """
        return self._get_column(self._getdirectionvec, row)

    @asaplog_post_dec
    def set_unit(self, unit='channel'):
        """\
        Set the unit for all following operations on this scantable

        Parameters:

            unit:    optional unit, default is 'channel'. Use one of '*Hz',
                     'km/s', 'channel' or equivalent ''

        """
        varlist = vars()
        if unit in ['', 'pixel', 'channel']:
            unit = ''
        inf = list(self._getcoordinfo())
        inf[0] = unit
        self._setcoordinfo(inf)
        self._add_history("set_unit", varlist)

    @asaplog_post_dec
    def set_instrument(self, instr):
        """\
        Set the instrument for subsequent processing.

        Parameters:

            instr:    Select from 'ATPKSMB', 'ATPKSHOH', 'ATMOPRA',
                      'DSS-43' (Tid), 'CEDUNA', and 'HOBART'

        """
        self._setInstrument(instr)
        self._add_history("set_instument", vars())

    @asaplog_post_dec
    def set_feedtype(self, feedtype):
        """\
        Overwrite the feed type, which might not be set correctly.

        Parameters:

            feedtype:     'linear' or 'circular'

        """
        self._setfeedtype(feedtype)
        self._add_history("set_feedtype", vars())

    @asaplog_post_dec
    def set_doppler(self, doppler='RADIO'):
        """\
        Set the doppler for all following operations on this scantable.

        Parameters:

            doppler:    One of 'RADIO', 'OPTICAL', 'Z', 'BETA', 'GAMMA'

        """
        varlist = vars()
        inf = list(self._getcoordinfo())
        inf[2] = doppler
        self._setcoordinfo(inf)
        self._add_history("set_doppler", vars())

    @asaplog_post_dec
    def set_freqframe(self, frame=None):
        """\
        Set the frame type of the Spectral Axis.

        Parameters:

            frame:   an optional frame type, default 'LSRK'. Valid frames are:
                     'TOPO', 'LSRD', 'LSRK', 'BARY',
                     'GEO', 'GALACTO', 'LGROUP', 'CMB'

        Example::

            scan.set_freqframe('BARY')

        """
        frame = frame or rcParams['scantable.freqframe']
        varlist = vars()
        # "REST" is not implemented in casacore
        #valid = ['REST', 'TOPO', 'LSRD', 'LSRK', 'BARY', \
        #           'GEO', 'GALACTO', 'LGROUP', 'CMB']
        valid = ['TOPO', 'LSRD', 'LSRK', 'BARY', \
                   'GEO', 'GALACTO', 'LGROUP', 'CMB']

        if frame in valid:
            inf = list(self._getcoordinfo())
            inf[1] = frame
            self._setcoordinfo(inf)
            self._add_history("set_freqframe", varlist)
        else:
            msg  = "Please specify a valid freq type. Valid types are:\n", valid
            raise TypeError(msg)

    @asaplog_post_dec
    def set_dirframe(self, frame=""):
        """\
        Set the frame type of the Direction on the sky.

        Parameters:

            frame:   an optional frame type, default ''. Valid frames are:
                     'J2000', 'B1950', 'GALACTIC'

        Example:

            scan.set_dirframe('GALACTIC')

        """
        varlist = vars()
        Scantable.set_dirframe(self, frame)
        self._add_history("set_dirframe", varlist)

    def get_unit(self):
        """\
        Get the default unit set in this scantable

        Returns:

            A unit string

        """
        inf = self._getcoordinfo()
        unit = inf[0]
        if unit == '': unit = 'channel'
        return unit

    @asaplog_post_dec
    def get_abcissa(self, rowno=0):
        """\
        Get the abcissa in the current coordinate setup for the currently
        selected Beam/IF/Pol

        Parameters:

            rowno:    an optional row number in the scantable. Default is the
                      first row, i.e. rowno=0

        Returns:

            The abcissa values and the format string (as a dictionary)

        """
        abc = self._getabcissa(rowno)
        lbl = self._getabcissalabel(rowno)
        return abc, lbl

    @asaplog_post_dec
    def flag(self, mask=None, unflag=False, row=-1):
        """\
        Flag the selected data using an optional channel mask.

        Parameters:

            mask:   an optional channel mask, created with create_mask. Default
                    (no mask) is all channels.

            unflag:    if True, unflag the data

            row:    an optional row number in the scantable.
                      Default -1 flags all rows
                      
        """
        varlist = vars()
        mask = mask or []
        self._flag(row, mask, unflag)
        self._add_history("flag", varlist)

    @asaplog_post_dec
    def flag_row(self, rows=None, unflag=False):
        """\
        Flag the selected data in row-based manner.

        Parameters:

            rows:   list of row numbers to be flagged. Default is no row
                    (must be explicitly specified to execute row-based 
                    flagging).

            unflag: if True, unflag the data.

        """
        varlist = vars()
        if rows is None:
            rows = []
        self._flag_row(rows, unflag)
        self._add_history("flag_row", varlist)

    @asaplog_post_dec
    def clip(self, uthres=None, dthres=None, clipoutside=True, unflag=False):
        """\
        Flag the selected data outside a specified range (in channel-base)

        Parameters:

            uthres:      upper threshold.

            dthres:      lower threshold

            clipoutside: True for flagging data outside the range 
                         [dthres:uthres].
                         False for flagging data inside the range.

            unflag:      if True, unflag the data.

        """
        varlist = vars()
        self._clip(uthres, dthres, clipoutside, unflag)
        self._add_history("clip", varlist)

    @asaplog_post_dec
    def lag_flag(self, start, end, unit="MHz", insitu=None):
        """\
        Flag the data in 'lag' space by providing a frequency to remove.
        Flagged data in the scantable get interpolated over the region.
        No taper is applied.

        Parameters:

            start:    the start frequency (really a period within the
                      bandwidth)  or period to remove

            end:      the end frequency or period to remove

            unit:     the frequency unit (default 'MHz') or '' for
                      explicit lag channels

        *Notes*:

            It is recommended to flag edges of the band or strong
            signals beforehand.

        """
        if insitu is None: insitu = rcParams['insitu']
        self._math._setinsitu(insitu)
        varlist = vars()
        base = { "GHz": 1000000000., "MHz": 1000000., "kHz": 1000., "Hz": 1.}
        if not (unit == "" or base.has_key(unit)):
            raise ValueError("%s is not a valid unit." % unit)
        if unit == "":
            s = scantable(self._math._lag_flag(self, start, end, "lags"))
        else:
            s = scantable(self._math._lag_flag(self, start*base[unit],
                                               end*base[unit], "frequency"))
        s._add_history("lag_flag", varlist)
        if insitu:
            self._assign(s)
        else:
            return s

    @asaplog_post_dec
    def fft(self, rowno=None, mask=None, getrealimag=False):
        """\
        Apply FFT to the spectra.
        Flagged data in the scantable get interpolated over the region.

        Parameters:

            rowno:          The row number(s) to be processed. int, list 
                            and tuple are accepted. By default (None), FFT 
                            is applied to the whole data.

            mask:           Auxiliary channel mask(s). Given as a boolean
                            list, it is applied to all specified rows.
                            A list of boolean lists can also be used to 
                            apply different masks. In the latter case, the
                            length of 'mask' must be the same as that of
                            'rowno'. The default is None. 
        
            getrealimag:    If True, returns the real and imaginary part 
                            values of the complex results.
                            If False (the default), returns the amplitude
                            (absolute value) normalised with Ndata/2 and
                            phase (argument, in unit of radian).

        Returns:

            A list of dictionaries containing the results for each spectrum.
            Each dictionary contains two values, the real and the imaginary 
            parts when getrealimag = True, or the amplitude(absolute value)
            and the phase(argument) when getrealimag = False. The key for
            these values are 'real' and 'imag', or 'ampl' and 'phase',
            respectively.
        """
        if rowno is None:
            rowno = []
        if isinstance(rowno, int):
            rowno = [rowno]
        elif not (isinstance(rowno, list) or isinstance(rowno, tuple)):
            raise TypeError("The row number(s) must be int, list or tuple.")
        if len(rowno) == 0: rowno = [i for i in xrange(self.nrow())]

        usecommonmask = True
        
        if mask is None:
            mask = []
        if isinstance(mask, list) or isinstance(mask, tuple):
            if len(mask) == 0:
                mask = [[]]
            else:
                if isinstance(mask[0], bool):
                    if len(mask) != self.nchan(self.getif(rowno[0])):
                        raise ValueError("The spectra and the mask have "
                                         "different length.")
                    mask = [mask]
                elif isinstance(mask[0], list) or isinstance(mask[0], tuple):
                    usecommonmask = False
                    if len(mask) != len(rowno):
                        raise ValueError("When specifying masks for each "
                                         "spectrum, the numbers of them "
                                         "must be identical.")
                    for i in xrange(mask):
                        if len(mask[i]) != self.nchan(self.getif(rowno[i])):
                            raise ValueError("The spectra and the mask have "
                                             "different length.")
                else:
                    raise TypeError("The mask must be a boolean list or "
                                    "a list of boolean list.")
        else:
            raise TypeError("The mask must be a boolean list or a list of "
                            "boolean list.")

        res = []

        imask = 0
        for whichrow in rowno:
            fspec = self._fft(whichrow, mask[imask], getrealimag)
            nspec = len(fspec)
            
            i = 0
            v1 = []
            v2 = []
            reselem = {"real":[],"imag":[]} if getrealimag \
                                            else {"ampl":[],"phase":[]}
            
            while (i < nspec):
                v1.append(fspec[i])
                v2.append(fspec[i+1])
                i += 2
            
            if getrealimag:
                reselem["real"]  += v1
                reselem["imag"]  += v2
            else:
                reselem["ampl"]  += v1
                reselem["phase"] += v2
            
            res.append(reselem)
            
            if not usecommonmask: 
                imask += 1
        
        return res

    @asaplog_post_dec
    def create_mask(self, *args, **kwargs):
        """\
        Compute and return a mask based on [min, max] windows.
        The specified windows are to be INCLUDED, when the mask is
        applied.

        Parameters:

            [min, max], [min2, max2], ...
                Pairs of start/end points (inclusive)specifying the regions
                to be masked

            invert:     optional argument. If specified as True,
                        return an inverted mask, i.e. the regions
                        specified are EXCLUDED

            row:        create the mask using the specified row for
                        unit conversions, default is row=0
                        only necessary if frequency varies over rows.

        Examples::

            scan.set_unit('channel')
            # a)
            msk = scan.create_mask([400, 500], [800, 900])
            # masks everything outside 400 and 500
            # and 800 and 900 in the unit 'channel'

            # b)
            msk = scan.create_mask([400, 500], [800, 900], invert=True)
            # masks the regions between 400 and 500
            # and 800 and 900 in the unit 'channel'

            # c)
            #mask only channel 400
            msk =  scan.create_mask([400])

        """
        row = kwargs.get("row", 0)
        data = self._getabcissa(row)
        u = self._getcoordinfo()[0]
        if u == "":
            u = "channel"
        msg = "The current mask window unit is %s" % u
        i = self._check_ifs()
        if not i:
            msg += "\nThis mask is only valid for IF=%d" % (self.getif(i))
        asaplog.push(msg)
        n = len(data)
        msk = _n_bools(n, False)
        # test if args is a 'list' or a 'normal *args - UGLY!!!

        ws = (isinstance(args[-1][-1], int) 
              or isinstance(args[-1][-1], float)) and args or args[0]
        for window in ws:
            if len(window) == 1:
                window = [window[0], window[0]]
            if len(window) == 0 or  len(window) > 2:
                raise ValueError("A window needs to be defined as "
                                 "[start(, end)]")
            if window[0] > window[1]:
                tmp = window[0]
                window[0] = window[1]
                window[1] = tmp
            for i in range(n):
                if data[i] >= window[0] and data[i] <= window[1]:
                    msk[i] = True
        if kwargs.has_key('invert'):
            if kwargs.get('invert'):
                msk = mask_not(msk)
        return msk

    def get_masklist(self, mask=None, row=0, silent=False):
        """\
        Compute and return a list of mask windows, [min, max].

        Parameters:

            mask:       channel mask, created with create_mask.

            row:        calcutate the masklist using the specified row
                        for unit conversions, default is row=0
                        only necessary if frequency varies over rows.

        Returns:

            [min, max], [min2, max2], ...
                Pairs of start/end points (inclusive)specifying
                the masked regions

        """
        if not (isinstance(mask,list) or isinstance(mask, tuple)):
            raise TypeError("The mask should be list or tuple.")
        if len(mask) <= 0:
            raise TypeError("The mask elements should be > 0")
        data = self._getabcissa(row)
        if len(data) != len(mask):
            msg = "Number of channels in scantable != number of mask elements"
            raise TypeError(msg)
        u = self._getcoordinfo()[0]
        if u == "":
            u = "channel"
        msg = "The current mask window unit is %s" % u
        i = self._check_ifs()
        if not i:
            msg += "\nThis mask is only valid for IF=%d" % (self.getif(i))
        if not silent:
            asaplog.push(msg)
        masklist = []
        ist, ien = None, None
        ist, ien=self.get_mask_indices(mask)
        if ist is not None and ien is not None:
            for i in xrange(len(ist)):
                range=[data[ist[i]],data[ien[i]]]
                range.sort()
                masklist.append([range[0],range[1]])
        return masklist

    def get_mask_indices(self, mask=None):
        """\
        Compute and Return lists of mask start indices and mask end indices.

        Parameters:

            mask:       channel mask, created with create_mask.

        Returns:

            List of mask start indices and that of mask end indices,
            i.e., [istart1,istart2,....], [iend1,iend2,....].

        """
        if not (isinstance(mask,list) or isinstance(mask, tuple)):
            raise TypeError("The mask should be list or tuple.")
        if len(mask) <= 0:
            raise TypeError("The mask elements should be > 0")
        istart = []
        iend = []
        if mask[0]: 
            istart.append(0)
        for i in range(len(mask)-1):
            if not mask[i] and mask[i+1]:
                istart.append(i+1)
            elif mask[i] and not mask[i+1]:
                iend.append(i)
        if mask[len(mask)-1]: 
            iend.append(len(mask)-1)
        if len(istart) != len(iend):
            raise RuntimeError("Numbers of mask start != mask end.")
        for i in range(len(istart)):
            if istart[i] > iend[i]:
                raise RuntimeError("Mask start index > mask end index")
                break
        return istart,iend

    @asaplog_post_dec
    def parse_maskexpr(self, maskstring):
        """
        Parse CASA type mask selection syntax (IF dependent).

        Parameters:
            maskstring : A string mask selection expression.
                         A comma separated selections mean different IF -
                         channel combinations. IFs and channel selections
                         are partitioned by a colon, ':'.
                     examples:
                         ''          = all IFs (all channels)
                         '<2,4~6,9'  = IFs 0,1,4,5,6,9 (all channels)
                         '3:3~45;60' = channels 3 to 45 and 60 in IF 3
                         '0~1:2~6,8' = channels 2 to 6 in IFs 0,1, and
                                       all channels in IF8
        Returns:
        A dictionary of selected (valid) IF and masklist pairs,
        e.g. {'0': [[50,250],[350,462]], '2': [[100,400],[550,974]]}
        """
        if not isinstance(maskstring,str):
            asaplog.post()
            asaplog.push("Mask expression should be a string.")
            asaplog.post("ERROR")
        
        valid_ifs = self.getifnos()
        frequnit = self.get_unit()
        seldict = {}
        if maskstring == "":
            maskstring = str(valid_ifs)[1:-1]
        ## split each selection "IF range[:CHAN range]"
        sellist = maskstring.split(',')
        for currselstr in sellist:
            selset = currselstr.split(':')
            # spw and mask string (may include ~, < or >)
            spwmasklist = self._parse_selection(selset[0], typestr='integer',
                                                minval=min(valid_ifs),
                                                maxval=max(valid_ifs))
            for spwlist in spwmasklist:
                selspws = []
                for ispw in range(spwlist[0],spwlist[1]+1):
                    # Put into the list only if ispw exists
                    if valid_ifs.count(ispw):
                        selspws.append(ispw)
            del spwmasklist, spwlist

            # parse frequency mask list
            if len(selset) > 1:
                freqmasklist = self._parse_selection(selset[1], typestr='float',
                                                     offset=0.)
            else:
                # want to select the whole spectrum
                freqmasklist = [None]

            ## define a dictionary of spw - masklist combination
            for ispw in selspws:
                #print "working on", ispw
                spwstr = str(ispw)
                if len(selspws) == 0:
                    # empty spw
                    continue
                else:
                    ## want to get min and max of the spw and
                    ## offset to set for '<' and '>'
                    if frequnit == 'channel':
                        minfreq = 0
                        maxfreq = self.nchan(ifno=ispw)
                        offset = 0.5
                    else:
                        ## This is ugly part. need improvement
                        for ifrow in xrange(self.nrow()):
                            if self.getif(ifrow) == ispw:
                                #print "IF",ispw,"found in row =",ifrow
                                break
                        freqcoord = self.get_coordinate(ifrow)
                        freqs = self._getabcissa(ifrow)
                        minfreq = min(freqs)
                        maxfreq = max(freqs)
                        if len(freqs) == 1:
                            offset = 0.5
                        elif frequnit.find('Hz') > 0:
                            offset = abs(freqcoord.to_frequency(1,
                                                                unit=frequnit)
                                         -freqcoord.to_frequency(0,
                                                                 unit=frequnit)
                                         )*0.5
                        elif frequnit.find('m/s') > 0:
                            offset = abs(freqcoord.to_velocity(1,
                                                               unit=frequnit)
                                         -freqcoord.to_velocity(0,
                                                                unit=frequnit)
                                         )*0.5
                        else:
                            asaplog.post()
                            asaplog.push("Invalid frequency unit")
                            asaplog.post("ERROR")
                        del freqs, freqcoord, ifrow
                    for freq in freqmasklist:
                        selmask = freq or [minfreq, maxfreq]
                        if selmask[0] == None:
                            ## selection was "<freq[1]".
                            if selmask[1] < minfreq:
                                ## avoid adding region selection
                                selmask = None
                            else:
                                selmask = [minfreq,selmask[1]-offset]
                        elif selmask[1] == None:
                            ## selection was ">freq[0]"
                            if selmask[0] > maxfreq:
                                ## avoid adding region selection
                                selmask = None
                            else:
                                selmask = [selmask[0]+offset,maxfreq]
                        if selmask:
                            if not seldict.has_key(spwstr):
                                # new spw selection
                                seldict[spwstr] = []
                            seldict[spwstr] += [selmask]
                    del minfreq,maxfreq,offset,freq,selmask
                del spwstr
            del freqmasklist
        del valid_ifs
        if len(seldict) == 0:
            asaplog.post()
            asaplog.push("No valid selection in the mask expression: "
                         +maskstring)
            asaplog.post("WARN")
            return None
        msg = "Selected masklist:\n"
        for sif, lmask in seldict.iteritems():
            msg += "   IF"+sif+" - "+str(lmask)+"\n"
        asaplog.push(msg)
        return seldict

    @asaplog_post_dec
    def parse_idx_selection(self, mode, selexpr):
        """
        Parse CASA type mask selection syntax of SCANNO, IFNO, POLNO,
        BEAMNO, and row number

        Parameters:
            mode       : which column to select.
                         ['scan',|'if'|'pol'|'beam'|'row']
            selexpr    : A comma separated selection expression.
                     examples:
                         ''          = all (returns [])
                         '<2,4~6,9'  = indices less than 2, 4 to 6 and 9
                                       (returns [0,1,4,5,6,9])
        Returns:
        A List of selected indices
        """
        if selexpr == "":
            return []
        valid_modes = {'s': 'scan', 'i': 'if', 'p': 'pol',
                       'b': 'beam', 'r': 'row'}
        smode =  mode.lower()[0]
        if not (smode in valid_modes.keys()):
            msg = "Invalid mode '%s'. Valid modes are %s" %\
                  (mode, str(valid_modes.values()))
            asaplog.post()
            asaplog.push(msg)
            asaplog.post("ERROR")
        mode = valid_modes[smode]
        minidx = None
        maxidx = None
        if smode == 'r':
            minidx = 0
            maxidx = self.nrow()
        else:
            idx = getattr(self,"get"+mode+"nos")()
            minidx = min(idx)
            maxidx = max(idx)
            del idx
        sellist = selexpr.split(',')
        idxlist = []
        for currselstr in sellist:
            # single range (may include ~, < or >)
            currlist = self._parse_selection(currselstr, typestr='integer',
                                             minval=minidx,maxval=maxidx)
            for thelist in currlist:
                idxlist += range(thelist[0],thelist[1]+1)
        msg = "Selected %s: %s" % (mode.upper()+"NO", str(idxlist))
        asaplog.push(msg)
        return idxlist

    def _parse_selection(self, selstr, typestr='float', offset=0.,
                         minval=None, maxval=None):
        """
        Parameters:
            selstr :    The Selection string, e.g., '<3;5~7;100~103;9'
            typestr :   The type of the values in returned list
                        ('integer' or 'float')
            offset :    The offset value to subtract from or add to
                        the boundary value if the selection string
                        includes '<' or '>' [Valid only for typestr='float']
            minval, maxval :  The minimum/maximum values to set if the
                              selection string includes '<' or '>'.
                              The list element is filled with None by default.
        Returns:
            A list of min/max pair of selections.
        Example:
            _parse_selection('<3;5~7;9',typestr='int',minval=0)
            --> returns [[0,2],[5,7],[9,9]]
            _parse_selection('<3;5~7;9',typestr='float',offset=0.5,minval=0)
            --> returns [[0.,2.5],[5.0,7.0],[9.,9.]]
        """
        selgroups = selstr.split(';')
        sellists = []
        if typestr.lower().startswith('int'):
            formatfunc = int
            offset = 1
        else:
            formatfunc = float
        
        for currsel in  selgroups:
            if currsel.find('~') > 0:
                # val0 <= x <= val1
                minsel = formatfunc(currsel.split('~')[0].strip())
                maxsel = formatfunc(currsel.split('~')[1].strip()) 
            elif currsel.strip().find('<=') > -1:
                bound = currsel.split('<=')
                try: # try "x <= val"
                    minsel = minval
                    maxsel = formatfunc(bound[1].strip())
                except ValueError: # now "val <= x"
                    minsel = formatfunc(bound[0].strip())
                    maxsel = maxval
            elif currsel.strip().find('>=') > -1:
                bound = currsel.split('>=')
                try: # try "x >= val"
                    minsel = formatfunc(bound[1].strip())
                    maxsel = maxval
                except ValueError: # now "val >= x"
                    minsel = minval
                    maxsel = formatfunc(bound[0].strip())
            elif currsel.strip().find('<') > -1:
                bound = currsel.split('<')
                try: # try "x < val"
                    minsel = minval
                    maxsel = formatfunc(bound[1].strip()) \
                             - formatfunc(offset)
                except ValueError: # now "val < x"
                    minsel = formatfunc(bound[0].strip()) \
                         + formatfunc(offset)
                    maxsel = maxval
            elif currsel.strip().find('>') > -1:
                bound = currsel.split('>')
                try: # try "x > val"
                    minsel = formatfunc(bound[1].strip()) \
                             + formatfunc(offset)
                    maxsel = maxval
                except ValueError: # now "val > x"
                    minsel = minval
                    maxsel = formatfunc(bound[0].strip()) \
                             - formatfunc(offset)
            else:
                minsel = formatfunc(currsel)
                maxsel = formatfunc(currsel)
            sellists.append([minsel,maxsel])
        return sellists

#    def get_restfreqs(self):
#        """
#        Get the restfrequency(s) stored in this scantable.
#        The return value(s) are always of unit 'Hz'
#        Parameters:
#            none
#        Returns:
#            a list of doubles
#        """
#        return list(self._getrestfreqs())

    def get_restfreqs(self, ids=None):
        """\
        Get the restfrequency(s) stored in this scantable.
        The return value(s) are always of unit 'Hz'

        Parameters:

            ids: (optional) a list of MOLECULE_ID for that restfrequency(s) to
                 be retrieved

        Returns:

            dictionary containing ids and a list of doubles for each id

        """
        if ids is None:
            rfreqs = {}
            idlist = self.getmolnos()
            for i in idlist:
                rfreqs[i] = list(self._getrestfreqs(i))
            return rfreqs
        else:
            if type(ids) == list or type(ids) == tuple:
                rfreqs = {}
                for i in ids:
                    rfreqs[i] = list(self._getrestfreqs(i))
                return rfreqs
            else:
                return list(self._getrestfreqs(ids))

    @asaplog_post_dec
    def set_restfreqs(self, freqs=None, unit='Hz'):
        """\
        Set or replace the restfrequency specified and
        if the 'freqs' argument holds a scalar,
        then that rest frequency will be applied to all the selected
        data.  If the 'freqs' argument holds
        a vector, then it MUST be of equal or smaller length than
        the number of IFs (and the available restfrequencies will be
        replaced by this vector).  In this case, *all* data have
        the restfrequency set per IF according
        to the corresponding value you give in the 'freqs' vector.
        E.g. 'freqs=[1e9, 2e9]'  would mean IF 0 gets restfreq 1e9 and
        IF 1 gets restfreq 2e9.

        You can also specify the frequencies via a linecatalog.

        Parameters:

            freqs:   list of rest frequency values or string idenitfiers

            unit:    unit for rest frequency (default 'Hz')


        Example::

            # set the given restfrequency for the all currently selected IFs
            scan.set_restfreqs(freqs=1.4e9)
            # set restfrequencies for the n IFs  (n > 1) in the order of the
            # list, i.e
            # IF0 -> 1.4e9, IF1 ->  1.41e9, IF3 -> 1.42e9
            # len(list_of_restfreqs) == nIF
            # for nIF == 1 the following will set multiple restfrequency for
            # that IF
            scan.set_restfreqs(freqs=[1.4e9, 1.41e9, 1.42e9])
            # set multiple restfrequencies per IF. as a list of lists where
            # the outer list has nIF elements, the inner s arbitrary
            scan.set_restfreqs(freqs=[[1.4e9, 1.41e9], [1.67e9]])

       *Note*:

            To do more sophisticate Restfrequency setting, e.g. on a
            source and IF basis, use scantable.set_selection() before using
            this function::

                # provided your scantable is called scan
                selection = selector()
                selection.set_name('ORION*')
                selection.set_ifs([1])
                scan.set_selection(selection)
                scan.set_restfreqs(freqs=86.6e9)

        """
        varlist = vars()
        from asap import linecatalog
        # simple  value
        if isinstance(freqs, int) or isinstance(freqs, float):
            self._setrestfreqs([freqs], [""], unit)
        # list of values
        elif isinstance(freqs, list) or isinstance(freqs, tuple):
            # list values are scalars
            if isinstance(freqs[-1], int) or isinstance(freqs[-1], float):
                if len(freqs) == 1:
                    self._setrestfreqs(freqs, [""], unit)
                else:
                    # allow the 'old' mode of setting mulitple IFs
                    savesel = self._getselection()
                    sel = self.get_selection()
                    iflist = self.getifnos()
                    if len(freqs)>len(iflist):
                        raise ValueError("number of elements in list of list "
                                         "exeeds the current IF selections")
                    iflist = self.getifnos()
                    for i, fval in enumerate(freqs):
                        sel.set_ifs(iflist[i])
                        self._setselection(sel)
                        self._setrestfreqs([fval], [""], unit)
                    self._setselection(savesel)

            # list values are dict, {'value'=, 'name'=)
            elif isinstance(freqs[-1], dict):
                values = []
                names = []
                for d in freqs:
                    values.append(d["value"])
                    names.append(d["name"])
                self._setrestfreqs(values, names, unit)
            elif isinstance(freqs[-1], list) or isinstance(freqs[-1], tuple):
                savesel = self._getselection()
                sel = self.get_selection()
                iflist = self.getifnos()
                if len(freqs)>len(iflist):
                    raise ValueError("number of elements in list of list exeeds"
                                     " the current IF selections")
                for i, fval in enumerate(freqs):
                    sel.set_ifs(iflist[i])
                    self._setselection(sel)
                    self._setrestfreqs(fval, [""], unit)
                self._setselection(savesel)
        # freqs are to be taken from a linecatalog
        elif isinstance(freqs, linecatalog):
            savesel = self._getselection()
            sel = self.get_selection()
            for i in xrange(freqs.nrow()):
                sel.set_ifs(iflist[i])
                self._setselection(sel)
                self._setrestfreqs([freqs.get_frequency(i)],
                                   [freqs.get_name(i)], "MHz")
                # ensure that we are not iterating past nIF
                if i == self.nif()-1: break
            self._setselection(savesel)
        else:
            return
        self._add_history("set_restfreqs", varlist)

    @asaplog_post_dec
    def shift_refpix(self, delta):
        """\
        Shift the reference pixel of the Spectra Coordinate by an
        integer amount.

        Parameters:

            delta:   the amount to shift by

        *Note*:

            Be careful using this with broadband data.

        """
        varlist = vars()
        Scantable.shift_refpix(self, delta)
        s._add_history("shift_refpix", varlist)

    @asaplog_post_dec
    def history(self, filename=None, nrows=-1, start=0):
        """\
        Print the history. Optionally to a file.

        Parameters:

            filename:    The name of the file to save the history to.

        """
        n = self._historylength()
        if nrows == -1:
            nrows = n
        if start+nrows > n:
            nrows = nrows-start
        if n > 1000 and nrows == n:
            nrows = 1000
            start = n-1000
            asaplog.push("Warning: History has {0} entries. Displaying last "
                         "1000".format(n))
        hist = list(self._gethistory(nrows, start))
        out = "-"*80
        for h in hist:
            if not h.strip():
                continue
            if h.find("---") >-1:
                continue
            else:
                items = h.split("##")
                date = items[0]
                func = items[1]
                items = items[2:]
                out += "\n"+date+"\n"
                out += "Function: %s\n  Parameters:" % (func)
                for i in items:
                    if i == '':
                        continue
                    s = i.split("=")
                    out += "\n   %s = %s" % (s[0], s[1])
                out = "\n".join([out, "*"*80])
        if filename is not None:
            if filename is "":
                filename = 'scantable_history.txt'
            filename = os.path.expandvars(os.path.expanduser(filename))
            if not os.path.isdir(filename):
                data = open(filename, 'w')
                data.write(out)
                data.close()
            else:
                msg = "Illegal file name '%s'." % (filename)
                raise IOError(msg)
        return page(out)

    #
    # Maths business
    #
    @asaplog_post_dec
    def average_time(self, mask=None, scanav=False, weight='tint', align=False,
                     avmode="NONE"):
        """\
        Return the (time) weighted average of a scan. Scans will be averaged
        only if the source direction (RA/DEC) is within 1' otherwise

        *Note*:

            in channels only - align if necessary

        Parameters:

            mask:     an optional mask (only used for 'var' and 'tsys'
                      weighting)

            scanav:   True averages each scan separately
                      False (default) averages all scans together,

            weight:   Weighting scheme.
                      'none'     (mean no weight)
                      'var'      (1/var(spec) weighted)
                      'tsys'     (1/Tsys**2 weighted)
                      'tint'     (integration time weighted)
                      'tintsys'  (Tint/Tsys**2)
                      'median'   ( median averaging)
                      The default is 'tint'

            align:    align the spectra in velocity before averaging. It takes
                      the time of the first spectrum as reference time.
            avmode:   'SOURCE' - also select by source name -  or 
                      'NONE' (default). Not applicable for scanav=True or
                      weight=median

        Example::

            # time average the scantable without using a mask
            newscan = scan.average_time()

        """
        varlist = vars()
        weight = weight or 'TINT'
        mask = mask or ()
        scanav = (scanav and 'SCAN') or avmode.upper()
        scan = (self, )

        if align:
            scan = (self.freq_align(insitu=False), )
            asaplog.push("Note: Alignment is don on a source-by-source basis")
            asaplog.push("Note: Averaging (by default) is not")
            # we need to set it to SOURCE averaging here            
        s = None
        if weight.upper() == 'MEDIAN':
            s = scantable(self._math._averagechannel(scan[0], 'MEDIAN',
                                                     scanav))
        else:
            s = scantable(self._math._average(scan, mask, weight.upper(),
                          scanav))
        s._add_history("average_time", varlist)
        return s

    @asaplog_post_dec
    def convert_flux(self, jyperk=None, eta=None, d=None, insitu=None):
        """\
        Return a scan where all spectra are converted to either
        Jansky or Kelvin depending upon the flux units of the scan table.
        By default the function tries to look the values up internally.
        If it can't find them (or if you want to over-ride), you must
        specify EITHER jyperk OR eta (and D which it will try to look up
        also if you don't set it). jyperk takes precedence if you set both.

        Parameters:

            jyperk:      the Jy / K conversion factor

            eta:         the aperture efficiency

            d:           the geometric diameter (metres)

            insitu:      if False a new scantable is returned.
                         Otherwise, the scaling is done in-situ
                         The default is taken from .asaprc (False)

        """
        if insitu is None: insitu = rcParams['insitu']
        self._math._setinsitu(insitu)
        varlist = vars()
        jyperk = jyperk or -1.0
        d = d or -1.0
        eta = eta or -1.0
        s = scantable(self._math._convertflux(self, d, eta, jyperk))
        s._add_history("convert_flux", varlist)
        if insitu: self._assign(s)
        else: return s

    @asaplog_post_dec
    def gain_el(self, poly=None, filename="", method="linear", insitu=None):
        """\
        Return a scan after applying a gain-elevation correction.
        The correction can be made via either a polynomial or a
        table-based interpolation (and extrapolation if necessary).
        You specify polynomial coefficients, an ascii table or neither.
        If you specify neither, then a polynomial correction will be made
        with built in coefficients known for certain telescopes (an error
        will occur if the instrument is not known).
        The data and Tsys are *divided* by the scaling factors.

        Parameters:

            poly:        Polynomial coefficients (default None) to compute a
                         gain-elevation correction as a function of
                         elevation (in degrees).

            filename:    The name of an ascii file holding correction factors.
                         The first row of the ascii file must give the column
                         names and these MUST include columns
                         'ELEVATION' (degrees) and 'FACTOR' (multiply data
                         by this) somewhere.
                         The second row must give the data type of the
                         column. Use 'R' for Real and 'I' for Integer.
                         An example file would be
                         (actual factors are arbitrary) :

                         TIME ELEVATION FACTOR
                         R R R
                         0.1 0 0.8
                         0.2 20 0.85
                         0.3 40 0.9
                         0.4 60 0.85
                         0.5 80 0.8
                         0.6 90 0.75

            method:      Interpolation method when correcting from a table.
                         Values are  'nearest', 'linear' (default), 'cubic'
                         and 'spline'

            insitu:      if False a new scantable is returned.
                         Otherwise, the scaling is done in-situ
                         The default is taken from .asaprc (False)

        """

        if insitu is None: insitu = rcParams['insitu']
        self._math._setinsitu(insitu)
        varlist = vars()
        poly = poly or ()
        from os.path import expandvars
        filename = expandvars(filename)
        s = scantable(self._math._gainel(self, poly, filename, method))
        s._add_history("gain_el", varlist)
        if insitu:
            self._assign(s)
        else:
            return s

    @asaplog_post_dec
    def freq_align(self, reftime=None, method='cubic', insitu=None):
        """\
        Return a scan where all rows have been aligned in frequency/velocity.
        The alignment frequency frame (e.g. LSRK) is that set by function
        set_freqframe.

        Parameters:

            reftime:     reference time to align at. By default, the time of
                         the first row of data is used.

            method:      Interpolation method for regridding the spectra.
                         Choose from 'nearest', 'linear', 'cubic' (default)
                         and 'spline'

            insitu:      if False a new scantable is returned.
                         Otherwise, the scaling is done in-situ
                         The default is taken from .asaprc (False)

        """
        if insitu is None: insitu = rcParams["insitu"]
        oldInsitu = self._math._insitu()
        self._math._setinsitu(insitu)
        varlist = vars()
        reftime = reftime or ""
        s = scantable(self._math._freq_align(self, reftime, method))
        s._add_history("freq_align", varlist)
        self._math._setinsitu(oldInsitu)
        if insitu: 
            self._assign(s)
        else: 
            return s

    @asaplog_post_dec
    def opacity(self, tau=None, insitu=None):
        """\
        Apply an opacity correction. The data
        and Tsys are multiplied by the correction factor.

        Parameters:

            tau:         (list of) opacity from which the correction factor is
                         exp(tau*ZD)
                         where ZD is the zenith-distance.
                         If a list is provided, it has to be of length nIF,
                         nIF*nPol or 1 and in order of IF/POL, e.g.
                         [opif0pol0, opif0pol1, opif1pol0 ...]
                         if tau is `None` the opacities are determined from a
                         model.

            insitu:      if False a new scantable is returned.
                         Otherwise, the scaling is done in-situ
                         The default is taken from .asaprc (False)

        """
        if insitu is None: 
            insitu = rcParams['insitu']
        self._math._setinsitu(insitu)
        varlist = vars()
        if not hasattr(tau, "__len__"):
            tau = [tau]
        s = scantable(self._math._opacity(self, tau))
        s._add_history("opacity", varlist)
        if insitu: 
            self._assign(s)
        else: 
            return s

    @asaplog_post_dec
    def bin(self, width=5, insitu=None):
        """\
        Return a scan where all spectra have been binned up.

        Parameters:

            width:       The bin width (default=5) in pixels

            insitu:      if False a new scantable is returned.
                         Otherwise, the scaling is done in-situ
                         The default is taken from .asaprc (False)

        """
        if insitu is None: 
            insitu = rcParams['insitu']
        self._math._setinsitu(insitu)
        varlist = vars()
        s = scantable(self._math._bin(self, width))
        s._add_history("bin", varlist)
        if insitu:
            self._assign(s)
        else:
            return s

    @asaplog_post_dec
    def reshape(self, first, last, insitu=None):
        """Resize the band by providing first and last channel. 
        This will cut off all channels outside [first, last].
        """
        if insitu is None: 
            insitu = rcParams['insitu']
        varlist = vars()
        if last < 0:
            last = self.nchan()-1 + last
        s = None
        if insitu:
            s = self
        else:
            s = self.copy()
        s._reshape(first,last)
        s._add_history("reshape", varlist)
        if not insitu:
            return s        

    @asaplog_post_dec
    def resample(self, width=5, method='cubic', insitu=None):
        """\
        Return a scan where all spectra have been binned up.

        Parameters:

            width:       The bin width (default=5) in pixels

            method:      Interpolation method when correcting from a table.
                         Values are  'nearest', 'linear', 'cubic' (default)
                         and 'spline'

            insitu:      if False a new scantable is returned.
                         Otherwise, the scaling is done in-situ
                         The default is taken from .asaprc (False)

        """
        if insitu is None: 
            insitu = rcParams['insitu']
        self._math._setinsitu(insitu)
        varlist = vars()
        s = scantable(self._math._resample(self, method, width))
        s._add_history("resample", varlist)
        if insitu: 
            self._assign(s)
        else: 
            return s

    @asaplog_post_dec
    def average_pol(self, mask=None, weight='none'):
        """\
        Average the Polarisations together.

        Parameters:

            mask:        An optional mask defining the region, where the
                         averaging will be applied. The output will have all
                         specified points masked.

            weight:      Weighting scheme. 'none' (default), 'var' (1/var(spec)
                         weighted), or 'tsys' (1/Tsys**2 weighted)

        """
        varlist = vars()
        mask = mask or ()
        s = scantable(self._math._averagepol(self, mask, weight.upper()))
        s._add_history("average_pol", varlist)
        return s

    @asaplog_post_dec
    def average_beam(self, mask=None, weight='none'):
        """\
        Average the Beams together.

        Parameters:
            mask:        An optional mask defining the region, where the
                         averaging will be applied. The output will have all
                         specified points masked.

            weight:      Weighting scheme. 'none' (default), 'var' (1/var(spec)
                         weighted), or 'tsys' (1/Tsys**2 weighted)

        """
        varlist = vars()
        mask = mask or ()
        s = scantable(self._math._averagebeams(self, mask, weight.upper()))
        s._add_history("average_beam", varlist)
        return s

    def parallactify(self, pflag):
        """\
        Set a flag to indicate whether this data should be treated as having
        been 'parallactified' (total phase == 0.0)

        Parameters:

            pflag:  Bool indicating whether to turn this on (True) or
                    off (False)

        """
        varlist = vars()
        self._parallactify(pflag)
        self._add_history("parallactify", varlist)

    @asaplog_post_dec
    def convert_pol(self, poltype=None):
        """\
        Convert the data to a different polarisation type.
        Note that you will need cross-polarisation terms for most conversions.

        Parameters:

            poltype:    The new polarisation type. Valid types are:
                        'linear', 'circular', 'stokes' and 'linpol'

        """
        varlist = vars()
        s = scantable(self._math._convertpol(self, poltype))
        s._add_history("convert_pol", varlist)
        return s

    @asaplog_post_dec
    def smooth(self, kernel="hanning", width=5.0, order=2, plot=False, 
               insitu=None):
        """\
        Smooth the spectrum by the specified kernel (conserving flux).

        Parameters:

            kernel:     The type of smoothing kernel. Select from
                        'hanning' (default), 'gaussian', 'boxcar', 'rmedian'
                        or 'poly'

            width:      The width of the kernel in pixels. For hanning this is
                        ignored otherwise it defauls to 5 pixels.
                        For 'gaussian' it is the Full Width Half
                        Maximum. For 'boxcar' it is the full width.
                        For 'rmedian' and 'poly' it is the half width.

            order:      Optional parameter for 'poly' kernel (default is 2), to
                        specify the order of the polnomial. Ignored by all other
                        kernels.

            plot:       plot the original and the smoothed spectra.
                        In this each indivual fit has to be approved, by
                        typing 'y' or 'n'

            insitu:     if False a new scantable is returned.
                        Otherwise, the scaling is done in-situ
                        The default is taken from .asaprc (False)

        """
        if insitu is None: insitu = rcParams['insitu']
        self._math._setinsitu(insitu)
        varlist = vars()

        if plot: orgscan = self.copy()

        s = scantable(self._math._smooth(self, kernel.lower(), width, order))
        s._add_history("smooth", varlist)

        action = 'H'
        if plot:
            from asap.asapplotter import new_asaplot
            theplot = new_asaplot(rcParams['plotter.gui'])
            from matplotlib import rc as rcp
            rcp('lines', linewidth=1)
            theplot.set_panels()
            ylab=s._get_ordinate_label()
            #theplot.palette(0,["#777777","red"])
            for r in xrange(s.nrow()):
                xsm=s._getabcissa(r)
                ysm=s._getspectrum(r)
                xorg=orgscan._getabcissa(r)
                yorg=orgscan._getspectrum(r)
                if action != "N": #skip plotting if rejecting all
                    theplot.clear()
                    theplot.hold()
                    theplot.set_axes('ylabel',ylab)
                    theplot.set_axes('xlabel',s._getabcissalabel(r))
                    theplot.set_axes('title',s._getsourcename(r))
                    theplot.set_line(label='Original',color="#777777")
                    theplot.plot(xorg,yorg)
                    theplot.set_line(label='Smoothed',color="red")
                    theplot.plot(xsm,ysm)
                    ### Ugly part for legend
                    for i in [0,1]:
                        theplot.subplots[0]['lines'].append(
                            [theplot.subplots[0]['axes'].lines[i]]
                            )
                    theplot.release()
                    ### Ugly part for legend
                    theplot.subplots[0]['lines']=[]
                res = self._get_verify_action("Accept smoothing?",action)
                #print "IF%d, POL%d: got result = %s" %(s.getif(r),s.getpol(r),res)
                if r == 0: action = None
                #res = raw_input("Accept smoothing ([y]/n): ")
                if res.upper() == 'N':
                    # reject for the current rows
                    s._setspectrum(yorg, r)
                elif res.upper() == 'R':
                    # reject all the following rows
                    action = "N"
                    s._setspectrum(yorg, r)
                elif res.upper() == 'A':
                    # accept all the following rows
                    break
            theplot.quit()
            del theplot
            del orgscan

        if insitu: self._assign(s)
        else: return s

    @asaplog_post_dec
    def regrid_channel(self, width=5, plot=False, insitu=None):
        """\
        Regrid the spectra by the specified channel width

        Parameters:

            width:      The channel width (float) of regridded spectra
                        in the current spectral unit.

            plot:       [NOT IMPLEMENTED YET]
                        plot the original and the regridded spectra.
                        In this each indivual fit has to be approved, by
                        typing 'y' or 'n'

            insitu:     if False a new scantable is returned.
                        Otherwise, the scaling is done in-situ
                        The default is taken from .asaprc (False)

        """
        if insitu is None: insitu = rcParams['insitu']
        varlist = vars()

        if plot:
           asaplog.post()
           asaplog.push("Verification plot is not implemtnetd yet.")
           asaplog.post("WARN")

        s = self.copy()
        s._regrid_specchan(width)

        s._add_history("regrid_channel", varlist)

#         if plot:
#             from asap.asapplotter import new_asaplot
#             theplot = new_asaplot(rcParams['plotter.gui'])
#             from matplotlib import rc as rcp
#             rcp('lines', linewidth=1)
#             theplot.set_panels()
#             ylab=s._get_ordinate_label()
#             #theplot.palette(0,["#777777","red"])
#             for r in xrange(s.nrow()):
#                 xsm=s._getabcissa(r)
#                 ysm=s._getspectrum(r)
#                 xorg=orgscan._getabcissa(r)
#                 yorg=orgscan._getspectrum(r)
#                 theplot.clear()
#                 theplot.hold()
#                 theplot.set_axes('ylabel',ylab)
#                 theplot.set_axes('xlabel',s._getabcissalabel(r))
#                 theplot.set_axes('title',s._getsourcename(r))
#                 theplot.set_line(label='Original',color="#777777")
#                 theplot.plot(xorg,yorg)
#                 theplot.set_line(label='Smoothed',color="red")
#                 theplot.plot(xsm,ysm)
#                 ### Ugly part for legend
#                 for i in [0,1]:
#                     theplot.subplots[0]['lines'].append(
#                         [theplot.subplots[0]['axes'].lines[i]]
#                         )
#                 theplot.release()
#                 ### Ugly part for legend
#                 theplot.subplots[0]['lines']=[]
#                 res = raw_input("Accept smoothing ([y]/n): ")
#                 if res.upper() == 'N':
#                     s._setspectrum(yorg, r)
#             theplot.quit()
#             del theplot
#             del orgscan

        if insitu: self._assign(s)
        else: return s

    @asaplog_post_dec
    def _parse_wn(self, wn):
        if isinstance(wn, list) or isinstance(wn, tuple):
            return wn
        elif isinstance(wn, int):
            return [ wn ]
        elif isinstance(wn, str):
            if '-' in wn:                            # case 'a-b' : return [a,a+1,...,b-1,b]
                val = wn.split('-')
                val = [int(val[0]), int(val[1])]
                val.sort()
                res = [i for i in xrange(val[0], val[1]+1)]
            elif wn[:2] == '<=' or wn[:2] == '=<':   # cases '<=a','=<a' : return [0,1,...,a-1,a]
                val = int(wn[2:])+1
                res = [i for i in xrange(val)]
            elif wn[-2:] == '>=' or wn[-2:] == '=>': # cases 'a>=','a=>' : return [0,1,...,a-1,a]
                val = int(wn[:-2])+1
                res = [i for i in xrange(val)]
            elif wn[0] == '<':                       # case '<a' :         return [0,1,...,a-2,a-1]
                val = int(wn[1:])
                res = [i for i in xrange(val)]
            elif wn[-1] == '>':                      # case 'a>' :         return [0,1,...,a-2,a-1]
                val = int(wn[:-1])
                res = [i for i in xrange(val)]
            elif wn[:2] == '>=' or wn[:2] == '=>':   # cases '>=a','=>a' : return [a,-999], which is
                                                     #                     then interpreted in C++
                                                     #                     side as [a,a+1,...,a_nyq]
                                                     #                     (CAS-3759)
                val = int(wn[2:])
                res = [val, -999]
                #res = [i for i in xrange(val, self.nchan()/2+1)]
            elif wn[-2:] == '<=' or wn[-2:] == '=<': # cases 'a<=','a=<' : return [a,-999], which is
                                                     #                     then interpreted in C++
                                                     #                     side as [a,a+1,...,a_nyq]
                                                     #                     (CAS-3759)
                val = int(wn[:-2])
                res = [val, -999]
                #res = [i for i in xrange(val, self.nchan()/2+1)]
            elif wn[0] == '>':                       # case '>a' :         return [a+1,-999], which is
                                                     #                     then interpreted in C++
                                                     #                     side as [a+1,a+2,...,a_nyq]
                                                     #                     (CAS-3759)
                val = int(wn[1:])+1
                res = [val, -999]
                #res = [i for i in xrange(val, self.nchan()/2+1)]
            elif wn[-1] == '<':                      # case 'a<' :         return [a+1,-999], which is
                                                     #                     then interpreted in C++
                                                     #                     side as [a+1,a+2,...,a_nyq]
                                                     #                     (CAS-3759)
                val = int(wn[:-1])+1
                res = [val, -999]
                #res = [i for i in xrange(val, self.nchan()/2+1)]

            return res
        else:
            msg = 'wrong value given for addwn/rejwn'
            raise RuntimeError(msg)

    @asaplog_post_dec
    def apply_bltable(self, insitu=None, retfitres=None, inbltable=None, outbltable=None, overwrite=None):
        """\
        Subtract baseline based on parameters written in Baseline Table.

        Parameters:
            insitu:        if True, baseline fitting/subtraction is done
                           in-situ. If False, a new scantable with
                           baseline subtracted is returned. Actually,
                           format of the returned value depends on both 
                           insitu and retfitres (see below). 
                           The default is taken from .asaprc (False)
            retfitres:     if True, the results of baseline fitting (i.e.,
                           coefficients and rms) are returned.
                           default is False.
                           The format of the returned value of this
                           function varies as follows:
                           (1) in case insitu=True and retfitres=True:
                               fitting result.
                           (2) in case insitu=True and retfitres=False:
                               None.
                           (3) in case insitu=False and retfitres=True:
                               a dictionary containing a new scantable
                               (with baseline subtracted) and the fitting
                               results.
                           (4) in case insitu=False and retfitres=False:
                               a new scantable (with baseline subtracted).
            inbltable:     name of input baseline table. The row number of
                           scantable and that of inbltable must be
                           identical.
            outbltable:    name of output baseline table where baseline 
                           parameters and fitting results recorded.
                           default is ''(no output).
            overwrite:     if True when an existing baseline table is
                           specified for outbltable, overwrites it. 
                           Otherwise there is no harm.
                           default is False.
        """

        try:
            varlist = vars()
            if retfitres      is None: retfitres      = False
            if inbltable      is None: raise ValueError("bltable missing.")
            if outbltable     is None: outbltable     = ''
            if overwrite      is None: overwrite      = False

            if insitu is None: insitu = rcParams['insitu']
            if insitu:
                workscan = self
            else:
                workscan = self.copy()

            sres = workscan._apply_bltable(inbltable,
                                           retfitres,
                                           outbltable,
                                           os.path.exists(outbltable),
                                           overwrite)
            if retfitres: res = parse_fitresult(sres)

            workscan._add_history('apply_bltable', varlist)

            if insitu:
                self._assign(workscan)
                if retfitres:
                    return res
                else:
                    return None
            else:
                if retfitres:
                    return {'scantable': workscan, 'fitresults': res}
                else:
                    return workscan
        
        except RuntimeError, e:
            raise_fitting_failure_exception(e)

    @asaplog_post_dec
    def sub_baseline(self, insitu=None, retfitres=None, blinfo=None, bltable=None, overwrite=None):
        """\
        Subtract baseline based on parameters written in the input list. 

        Parameters:
            insitu:        if True, baseline fitting/subtraction is done
                           in-situ. If False, a new scantable with
                           baseline subtracted is returned. Actually,
                           format of the returned value depends on both 
                           insitu and retfitres (see below). 
                           The default is taken from .asaprc (False)
            retfitres:     if True, the results of baseline fitting (i.e.,
                           coefficients and rms) are returned.
                           default is False.
                           The format of the returned value of this
                           function varies as follows:
                           (1) in case insitu=True and retfitres=True:
                               fitting result.
                           (2) in case insitu=True and retfitres=False:
                               None.
                           (3) in case insitu=False and retfitres=True:
                               a dictionary containing a new scantable
                               (with baseline subtracted) and the fitting
                               results.
                           (4) in case insitu=False and retfitres=False:
                               a new scantable (with baseline subtracted).
            blinfo:        baseline parameter set stored in a dictionary
                           or a list of dictionary. Each dictionary 
                           corresponds to each spectrum and must contain
                           the following keys and values:
                             'row': row number, 
                             'blfunc': function name. available ones include
                                       'poly', 'chebyshev', 'cspline' and
                                       'sinusoid', 
                             'order': maximum order of polynomial. needed
                                      if blfunc='poly' or 'chebyshev', 
                             'npiece': number or piecewise polynomial.
                                       needed if blfunc='cspline', 
                             'nwave': a list of sinusoidal wave numbers.
                                      needed if blfunc='sinusoid', and 
                             'masklist': min-max windows for channel mask.
                                         the specified ranges will be used
                                         for fitting. 
            bltable:       name of output baseline table where baseline
                           parameters and fitting results recorded.
                           default is ''(no output).
            overwrite:     if True when an existing baseline table is
                           specified for bltable, overwrites it. 
                           Otherwise there is no harm.
                           default is False.
                           
        Example:
            sub_baseline(blinfo=[{'row':0, 'blfunc':'poly', 'order':5,
                                  'masklist':[[10,350],[352,510]]},
                                 {'row':1, 'blfunc':'cspline', 'npiece':3,
                                  'masklist':[[3,16],[19,404],[407,511]]}
                                  ])

                the first spectrum (row=0) will be fitted with polynomial
                of order=5 and the next one (row=1) will be fitted with cubic
                spline consisting of 3 pieces. 
        """

        try:
            varlist = vars()
            if retfitres      is None: retfitres      = False
            if blinfo         is None: blinfo         = []
            if bltable        is None: bltable        = ''
            if overwrite      is None: overwrite      = False

            if insitu is None: insitu = rcParams['insitu']
            if insitu:
                workscan = self
            else:
                workscan = self.copy()

            nrow = workscan.nrow()

            in_blinfo = pack_blinfo(blinfo=blinfo, maxirow=nrow)

            print "in_blinfo=< "+ str(in_blinfo)+" >"

            sres = workscan._sub_baseline(in_blinfo,
                                          retfitres,
                                          bltable,
                                          os.path.exists(bltable),
                                          overwrite)
            if retfitres: res = parse_fitresult(sres)
            
            workscan._add_history('sub_baseline', varlist)

            if insitu:
                self._assign(workscan)
                if retfitres:
                    return res
                else:
                    return None
            else:
                if retfitres:
                    return {'scantable': workscan, 'fitresults': res}
                else:
                    return workscan

        except RuntimeError, e:
            raise_fitting_failure_exception(e)

    @asaplog_post_dec
    def calc_aic(self, value=None, blfunc=None, order=None, mask=None,
                 whichrow=None, uselinefinder=None, edge=None,
                 threshold=None, chan_avg_limit=None):
        """\
        Calculates and returns model selection criteria for a specified
        baseline model and a given spectrum data. 
        Available values include Akaike Information Criterion (AIC), the
        corrected Akaike Information Criterion (AICc) by Sugiura(1978),
        Bayesian Information Criterion (BIC) and the Generalised Cross
        Validation (GCV). 

        Parameters:
            value:         name of model selection criteria to calculate.
                           available ones include 'aic', 'aicc', 'bic' and
                           'gcv'. default is 'aicc'.
            blfunc:        baseline function name. available ones include 
                           'chebyshev', 'cspline' and 'sinusoid'.
                           default is 'chebyshev'.
            order:         parameter for basline function. actually stands for
                           order of polynomial (order) for 'chebyshev',
                           number of spline pieces (npiece) for 'cspline' and
                           maximum wave number for 'sinusoid', respectively.
                           default is 5 (which is also the default order value
                           for [auto_]chebyshev_baseline()).
            mask:          an optional mask. default is [].
            whichrow:      row number. default is 0 (the first row)
            uselinefinder: use sd.linefinder() to flag out line regions
                           default is True.
            edge:           an optional number of channel to drop at
                            the edge of spectrum. If only one value is
                            specified, the same number will be dropped
                            from both sides of the spectrum. Default
                            is to keep all channels. Nested tuples
                            represent individual edge selection for
                            different IFs (a number of spectral channels
                            can be different)
                            default is (0, 0).
            threshold:      the threshold used by line finder. It is
                            better to keep it large as only strong lines
                            affect the baseline solution.
                            default is 3.
            chan_avg_limit: a maximum number of consequtive spectral
                            channels to average during the search of
                            weak and broad lines. The default is no
                            averaging (and no search for weak lines).
                            If such lines can affect the fitted baseline
                            (e.g. a high order polynomial is fitted),
                            increase this parameter (usually values up
                            to 8 are reasonable). Most users of this
                            method should find the default value sufficient.
                            default is 1.

        Example:
            aic = scan.calc_aic(blfunc='chebyshev', order=5, whichrow=0)
        """

        try:
            varlist = vars()

            if value          is None: value          = 'aicc'
            if blfunc         is None: blfunc         = 'chebyshev'
            if order          is None: order          = 5
            if mask           is None: mask           = []
            if whichrow       is None: whichrow       = 0
            if uselinefinder  is None: uselinefinder  = True
            if edge           is None: edge           = (0, 0)
            if threshold      is None: threshold      = 3
            if chan_avg_limit is None: chan_avg_limit = 1

            return self._calc_aic(value, blfunc, order, mask,
                                  whichrow, uselinefinder, edge,
                                  threshold, chan_avg_limit)
            
        except RuntimeError, e:
            raise_fitting_failure_exception(e)

    @asaplog_post_dec
    def sinusoid_baseline(self, mask=None, applyfft=None, 
                          fftmethod=None, fftthresh=None,
                          addwn=None, rejwn=None,
                          insitu=None,
                          clipthresh=None, clipniter=None,
                          plot=None,
                          getresidual=None,
                          showprogress=None, minnrow=None,
                          outlog=None,
                          blfile=None, csvformat=None,
                          bltable=None):
        """\
        Return a scan which has been baselined (all rows) with sinusoidal 
        functions.

        Parameters:
            mask:          an optional mask
            applyfft:      if True use some method, such as FFT, to find
                           strongest sinusoidal components in the wavenumber
                           domain to be used for baseline fitting.
                           default is True.
            fftmethod:     method to find the strong sinusoidal components.
                           now only 'fft' is available and it is the default.
            fftthresh:     the threshold to select wave numbers to be used for
                           fitting from the distribution of amplitudes in the
                           wavenumber domain. 
                           both float and string values accepted. 
                           given a float value, the unit is set to sigma.
                           for string values, allowed formats include:
                               'xsigma' or 'x' (= x-sigma level. e.g., 
                               '3sigma'), or
                               'topx' (= the x strongest ones, e.g. 'top5'). 
                           default is 3.0 (unit: sigma). 
            addwn:         the additional wave numbers to be used for fitting.
                           list or integer value is accepted to specify every
                           wave numbers. also string value can be used in case
                           you need to specify wave numbers in a certain range, 
                           e.g., 'a-b' (= a, a+1, a+2, ..., b-1, b),
                                 '<a'  (= 0,1,...,a-2,a-1),
                                 '>=a' (= a, a+1, ... up to the maximum wave
                                        number corresponding to the Nyquist
                                        frequency for the case of FFT).
                           default is [0]. 
            rejwn:         the wave numbers NOT to be used for fitting. 
                           can be set just as addwn but has higher priority: 
                           wave numbers which are specified both in addwn
                           and rejwn will NOT be used. default is []. 
            insitu:        if False a new scantable is returned.
                           Otherwise, the scaling is done in-situ
                           The default is taken from .asaprc (False)
            clipthresh:    Clipping threshold. (default is 3.0, unit: sigma)
            clipniter:     maximum number of iteration of 'clipthresh'-sigma 
                           clipping (default is 0)
            plot:      *** CURRENTLY UNAVAILABLE, ALWAYS FALSE ***
                           plot the fit and the residual. In this each
                           indivual fit has to be approved, by typing 'y'
                           or 'n'
            getresidual:   if False, returns best-fit values instead of
                           residual. (default is True)
            showprogress:  show progress status for large data.
                           default is True.
            minnrow:       minimum number of input spectra to show.
                           default is 1000.
            outlog:        Output the coefficients of the best-fit
                           function to logger (default is False)
            blfile:        Name of a text file in which the best-fit
                           parameter values to be written
                           (default is '': no file/logger output)
            csvformat:     if True blfile is csv-formatted, default is False.
            bltable:       name of a baseline table where fitting results
                           (coefficients, rms, etc.) are to be written.
                           if given, fitting results will NOT be output to
                           scantable (insitu=True) or None will be 
                           returned (insitu=False). 
                           (default is "": no table output)

        Example:
            # return a scan baselined by a combination of sinusoidal curves 
            # having wave numbers in spectral window up to 10, 
            # also with 3-sigma clipping, iteration up to 4 times
            bscan = scan.sinusoid_baseline(addwn='<=10',clipthresh=3.0,clipniter=4)

        Note:
            The best-fit parameter values output in logger and/or blfile are now
            based on specunit of 'channel'. 
        """
        
        try:
            varlist = vars()
        
            if insitu is None: insitu = rcParams['insitu']
            if insitu:
                workscan = self
            else:
                workscan = self.copy()
            
            if mask          is None: mask          = []
            if applyfft      is None: applyfft      = True
            if fftmethod     is None: fftmethod     = 'fft'
            if fftthresh     is None: fftthresh     = 3.0
            if addwn         is None: addwn         = [0]
            if rejwn         is None: rejwn         = []
            if clipthresh    is None: clipthresh    = 3.0
            if clipniter     is None: clipniter     = 0
            if plot          is None: plot          = False
            if getresidual   is None: getresidual   = True
            if showprogress  is None: showprogress  = True
            if minnrow       is None: minnrow       = 1000
            if outlog        is None: outlog        = False
            if blfile        is None: blfile        = ''
            if csvformat     is None: csvformat     = False
            if bltable       is None: bltable       = ''

            sapplyfft = 'true' if applyfft else 'false'
            fftinfo = ','.join([sapplyfft, fftmethod.lower(), str(fftthresh).lower()])

            scsvformat = 'T' if csvformat else 'F'

            #CURRENTLY, PLOT=true is UNAVAILABLE UNTIL sinusoidal fitting is implemented as a fitter method. 
            workscan._sinusoid_baseline(mask,
                                        fftinfo, 
                                        #applyfft, fftmethod.lower(), 
                                        #str(fftthresh).lower(), 
                                        workscan._parse_wn(addwn), 
                                        workscan._parse_wn(rejwn),
                                        clipthresh, clipniter,
                                        getresidual, 
                                        pack_progress_params(showprogress, 
                                                             minnrow),
                                        outlog, scsvformat+blfile,
                                        bltable)
            workscan._add_history('sinusoid_baseline', varlist)

            if bltable == '':
                if insitu:
                    self._assign(workscan)
                else:
                    return workscan
            else:
                if not insitu:
                    return None
            
        except RuntimeError, e:
            raise_fitting_failure_exception(e)


    @asaplog_post_dec
    def auto_sinusoid_baseline(self, mask=None, applyfft=None, 
                               fftmethod=None, fftthresh=None,
                               addwn=None, rejwn=None,
                               insitu=None,
                               clipthresh=None, clipniter=None,
                               edge=None, threshold=None, chan_avg_limit=None,
                               plot=None,
                               getresidual=None,
                               showprogress=None, minnrow=None,
                               outlog=None,
                               blfile=None, csvformat=None,
                               bltable=None):
        """\
        Return a scan which has been baselined (all rows) with sinusoidal 
        functions.
        Spectral lines are detected first using linefinder and masked out
        to avoid them affecting the baseline solution.

        Parameters:
            mask:           an optional mask retreived from scantable
            applyfft:       if True use some method, such as FFT, to find
                            strongest sinusoidal components in the wavenumber
                            domain to be used for baseline fitting.
                            default is True.
            fftmethod:      method to find the strong sinusoidal components.
                            now only 'fft' is available and it is the default.
            fftthresh:      the threshold to select wave numbers to be used for
                            fitting from the distribution of amplitudes in the
                            wavenumber domain. 
                            both float and string values accepted. 
                            given a float value, the unit is set to sigma.
                            for string values, allowed formats include:
                                'xsigma' or 'x' (= x-sigma level. e.g., 
                                '3sigma'), or
                                'topx' (= the x strongest ones, e.g. 'top5'). 
                            default is 3.0 (unit: sigma). 
            addwn:          the additional wave numbers to be used for fitting.
                            list or integer value is accepted to specify every
                            wave numbers. also string value can be used in case
                            you need to specify wave numbers in a certain range, 
                            e.g., 'a-b' (= a, a+1, a+2, ..., b-1, b),
                                  '<a'  (= 0,1,...,a-2,a-1),
                                  '>=a' (= a, a+1, ... up to the maximum wave
                                         number corresponding to the Nyquist
                                         frequency for the case of FFT).
                            default is [0]. 
            rejwn:          the wave numbers NOT to be used for fitting. 
                            can be set just as addwn but has higher priority: 
                            wave numbers which are specified both in addwn
                            and rejwn will NOT be used. default is []. 
            insitu:         if False a new scantable is returned.
                            Otherwise, the scaling is done in-situ
                            The default is taken from .asaprc (False)
            clipthresh:     Clipping threshold. (default is 3.0, unit: sigma)
            clipniter:      maximum number of iteration of 'clipthresh'-sigma 
                            clipping (default is 0)
            edge:           an optional number of channel to drop at
                            the edge of spectrum. If only one value is
                            specified, the same number will be dropped
                            from both sides of the spectrum. Default
                            is to keep all channels. Nested tuples
                            represent individual edge selection for
                            different IFs (a number of spectral channels
                            can be different)
            threshold:      the threshold used by line finder. It is
                            better to keep it large as only strong lines
                            affect the baseline solution.
            chan_avg_limit: a maximum number of consequtive spectral
                            channels to average during the search of
                            weak and broad lines. The default is no
                            averaging (and no search for weak lines).
                            If such lines can affect the fitted baseline
                            (e.g. a high order polynomial is fitted),
                            increase this parameter (usually values up
                            to 8 are reasonable). Most users of this
                            method should find the default value sufficient.
            plot:       *** CURRENTLY UNAVAILABLE, ALWAYS FALSE ***
                            plot the fit and the residual. In this each
                            indivual fit has to be approved, by typing 'y'
                            or 'n'
            getresidual:    if False, returns best-fit values instead of
                            residual. (default is True)
            showprogress:   show progress status for large data.
                            default is True.
            minnrow:        minimum number of input spectra to show.
                            default is 1000.
            outlog:         Output the coefficients of the best-fit
                            function to logger (default is False)
            blfile:         Name of a text file in which the best-fit
                            parameter values to be written
                            (default is "": no file/logger output)
            csvformat:      if True blfile is csv-formatted, default is False.
            bltable:        name of a baseline table where fitting results
                            (coefficients, rms, etc.) are to be written.
                            if given, fitting results will NOT be output to
                            scantable (insitu=True) or None will be 
                            returned (insitu=False). 
                            (default is "": no table output)

        Example:
            bscan = scan.auto_sinusoid_baseline(addwn='<=10', insitu=False)
        
        Note:
            The best-fit parameter values output in logger and/or blfile are now
            based on specunit of 'channel'. 
        """

        try:
            varlist = vars()

            if insitu is None: insitu = rcParams['insitu']
            if insitu:
                workscan = self
            else:
                workscan = self.copy()
            
            if mask           is None: mask           = []
            if applyfft       is None: applyfft       = True
            if fftmethod      is None: fftmethod      = 'fft'
            if fftthresh      is None: fftthresh      = 3.0
            if addwn          is None: addwn          = [0]
            if rejwn          is None: rejwn          = []
            if clipthresh     is None: clipthresh     = 3.0
            if clipniter      is None: clipniter      = 0
            if edge           is None: edge           = (0,0)
            if threshold      is None: threshold      = 3
            if chan_avg_limit is None: chan_avg_limit = 1
            if plot           is None: plot           = False
            if getresidual    is None: getresidual    = True
            if showprogress   is None: showprogress   = True
            if minnrow        is None: minnrow        = 1000
            if outlog         is None: outlog         = False
            if blfile         is None: blfile         = ''
            if csvformat      is None: csvformat      = False
            if bltable        is None: bltable        = ''

            sapplyfft = 'true' if applyfft else 'false'
            fftinfo = ','.join([sapplyfft, fftmethod.lower(), str(fftthresh).lower()])

            scsvformat = 'T' if csvformat else 'F'

            #CURRENTLY, PLOT=true is UNAVAILABLE UNTIL sinusoidal fitting is implemented as a fitter method. 
            workscan._auto_sinusoid_baseline(mask,
                                             fftinfo, 
                                             workscan._parse_wn(addwn), 
                                             workscan._parse_wn(rejwn), 
                                             clipthresh, clipniter, 
                                             normalise_edge_param(edge), 
                                             threshold, chan_avg_limit, 
                                             getresidual, 
                                             pack_progress_params(showprogress,
                                                                  minnrow), 
                                             outlog, scsvformat+blfile, bltable)
            workscan._add_history("auto_sinusoid_baseline", varlist)

            if bltable == '':
                if insitu:
                    self._assign(workscan)
                else:
                    return workscan
            else:
                if not insitu:
                    return None
            
        except RuntimeError, e:
            raise_fitting_failure_exception(e)

    @asaplog_post_dec
    def cspline_baseline(self, mask=None, npiece=None, insitu=None, 
                         clipthresh=None, clipniter=None, plot=None, 
                         getresidual=None, showprogress=None, minnrow=None, 
                         outlog=None, blfile=None, csvformat=None,
                         bltable=None):
        """\
        Return a scan which has been baselined (all rows) by cubic spline 
        function (piecewise cubic polynomial).

        Parameters:
            mask:         An optional mask
            npiece:       Number of pieces. (default is 2)
            insitu:       If False a new scantable is returned.
                          Otherwise, the scaling is done in-situ
                          The default is taken from .asaprc (False)
            clipthresh:   Clipping threshold. (default is 3.0, unit: sigma)
            clipniter:    maximum number of iteration of 'clipthresh'-sigma 
                          clipping (default is 0)
            plot:     *** CURRENTLY UNAVAILABLE, ALWAYS FALSE ***
                          plot the fit and the residual. In this each
                          indivual fit has to be approved, by typing 'y'
                          or 'n'
            getresidual:  if False, returns best-fit values instead of
                          residual. (default is True)
            showprogress: show progress status for large data.
                          default is True.
            minnrow:      minimum number of input spectra to show.
                          default is 1000.
            outlog:       Output the coefficients of the best-fit
                          function to logger (default is False)
            blfile:       Name of a text file in which the best-fit
                          parameter values to be written
                          (default is "": no file/logger output)
            csvformat:    if True blfile is csv-formatted, default is False.
            bltable:      name of a baseline table where fitting results
                          (coefficients, rms, etc.) are to be written.
                          if given, fitting results will NOT be output to
                          scantable (insitu=True) or None will be 
                          returned (insitu=False). 
                          (default is "": no table output)

        Example:
            # return a scan baselined by a cubic spline consisting of 2 pieces 
            # (i.e., 1 internal knot),
            # also with 3-sigma clipping, iteration up to 4 times
            bscan = scan.cspline_baseline(npiece=2,clipthresh=3.0,clipniter=4)
        
        Note:
            The best-fit parameter values output in logger and/or blfile are now
            based on specunit of 'channel'. 
        """
        
        try:
            varlist = vars()
            
            if insitu is None: insitu = rcParams['insitu']
            if insitu:
                workscan = self
            else:
                workscan = self.copy()

            if mask         is None: mask         = []
            if npiece       is None: npiece       = 2
            if clipthresh   is None: clipthresh   = 3.0
            if clipniter    is None: clipniter    = 0
            if plot         is None: plot         = False
            if getresidual  is None: getresidual  = True
            if showprogress is None: showprogress = True
            if minnrow      is None: minnrow      = 1000
            if outlog       is None: outlog       = False
            if blfile       is None: blfile       = ''
            if csvformat    is None: csvformat    = False
            if bltable      is None: bltable      = ''

            scsvformat = 'T' if csvformat else 'F'

            #CURRENTLY, PLOT=true UNAVAILABLE UNTIL cubic spline fitting is implemented as a fitter method. 
            workscan._cspline_baseline(mask, npiece,
                                       clipthresh, clipniter, 
                                       getresidual, 
                                       pack_progress_params(showprogress, 
                                                            minnrow),
                                       outlog, scsvformat+blfile,
                                       bltable)
            workscan._add_history("cspline_baseline", varlist)

            if bltable == '':
                if insitu:
                    self._assign(workscan)
                else:
                    return workscan
            else:
                if not insitu:
                    return None
            
        except RuntimeError, e:
            raise_fitting_failure_exception(e)

    @asaplog_post_dec
    def auto_cspline_baseline(self, mask=None, npiece=None, insitu=None, 
                              clipthresh=None, clipniter=None,
                              edge=None, threshold=None, chan_avg_limit=None, 
                              getresidual=None, plot=None,
                              showprogress=None, minnrow=None, outlog=None,
                              blfile=None, csvformat=None, bltable=None):
        """\
        Return a scan which has been baselined (all rows) by cubic spline
        function (piecewise cubic polynomial).
        Spectral lines are detected first using linefinder and masked out
        to avoid them affecting the baseline solution.

        Parameters:
            mask:           an optional mask retreived from scantable
            npiece:         Number of pieces. (default is 2)
            insitu:         if False a new scantable is returned.
                            Otherwise, the scaling is done in-situ
                            The default is taken from .asaprc (False)
            clipthresh:     Clipping threshold. (default is 3.0, unit: sigma)
            clipniter:      maximum number of iteration of 'clipthresh'-sigma 
                            clipping (default is 0)
            edge:           an optional number of channel to drop at
                            the edge of spectrum. If only one value is
                            specified, the same number will be dropped
                            from both sides of the spectrum. Default
                            is to keep all channels. Nested tuples
                            represent individual edge selection for
                            different IFs (a number of spectral channels
                            can be different)
            threshold:      the threshold used by line finder. It is
                            better to keep it large as only strong lines
                            affect the baseline solution.
            chan_avg_limit: a maximum number of consequtive spectral
                            channels to average during the search of
                            weak and broad lines. The default is no
                            averaging (and no search for weak lines).
                            If such lines can affect the fitted baseline
                            (e.g. a high order polynomial is fitted),
                            increase this parameter (usually values up
                            to 8 are reasonable). Most users of this
                            method should find the default value sufficient.
            plot:       *** CURRENTLY UNAVAILABLE, ALWAYS FALSE ***
                            plot the fit and the residual. In this each
                            indivual fit has to be approved, by typing 'y'
                            or 'n'
            getresidual:    if False, returns best-fit values instead of
                            residual. (default is True)
            showprogress:   show progress status for large data.
                            default is True.
            minnrow:        minimum number of input spectra to show.
                            default is 1000.
            outlog:         Output the coefficients of the best-fit
                            function to logger (default is False)
            blfile:         Name of a text file in which the best-fit
                            parameter values to be written
                            (default is "": no file/logger output)
            csvformat:      if True blfile is csv-formatted, default is False.
            bltable:        name of a baseline table where fitting results
                            (coefficients, rms, etc.) are to be written.
                            if given, fitting results will NOT be output to
                            scantable (insitu=True) or None will be 
                            returned (insitu=False). 
                            (default is "": no table output)

        Example:
            bscan = scan.auto_cspline_baseline(npiece=3, insitu=False)
        
        Note:
            The best-fit parameter values output in logger and/or blfile are now
            based on specunit of 'channel'. 
        """

        try:
            varlist = vars()

            if insitu is None: insitu = rcParams['insitu']
            if insitu:
                workscan = self
            else:
                workscan = self.copy()
            
            #if mask           is None: mask           = [True for i in xrange(workscan.nchan())]
            if mask           is None: mask           = []
            if npiece         is None: npiece         = 2
            if clipthresh     is None: clipthresh     = 3.0
            if clipniter      is None: clipniter      = 0
            if edge           is None: edge           = (0, 0)
            if threshold      is None: threshold      = 3
            if chan_avg_limit is None: chan_avg_limit = 1
            if plot           is None: plot           = False
            if getresidual    is None: getresidual    = True
            if showprogress   is None: showprogress   = True
            if minnrow        is None: minnrow        = 1000
            if outlog         is None: outlog         = False
            if blfile         is None: blfile         = ''
            if csvformat      is None: csvformat      = False
            if bltable        is None: bltable        = ''

            scsvformat = 'T' if csvformat else 'F'

            #CURRENTLY, PLOT=true UNAVAILABLE UNTIL cubic spline fitting is implemented as a fitter method.
            workscan._auto_cspline_baseline(mask, npiece,
                                            clipthresh, clipniter, 
                                            normalise_edge_param(edge), 
                                            threshold, 
                                            chan_avg_limit, getresidual, 
                                            pack_progress_params(showprogress, 
                                                                 minnrow), 
                                            outlog,
                                            scsvformat+blfile,
                                            bltable)
            workscan._add_history("auto_cspline_baseline", varlist)

            if bltable == '':
                if insitu:
                    self._assign(workscan)
                else:
                    return workscan
            else:
                if not insitu:
                    return None
            
        except RuntimeError, e:
            raise_fitting_failure_exception(e)

    @asaplog_post_dec
    def chebyshev_baseline(self, mask=None, order=None, insitu=None, 
                           clipthresh=None, clipniter=None, plot=None, 
                           getresidual=None, showprogress=None, minnrow=None, 
                           outlog=None, blfile=None, csvformat=None,
                           bltable=None):
        """\
        Return a scan which has been baselined (all rows) by Chebyshev polynomials.

        Parameters:
            mask:         An optional mask
            order:        the maximum order of Chebyshev polynomial (default is 5)
            insitu:       If False a new scantable is returned.
                          Otherwise, the scaling is done in-situ
                          The default is taken from .asaprc (False)
            clipthresh:   Clipping threshold. (default is 3.0, unit: sigma)
            clipniter:    maximum number of iteration of 'clipthresh'-sigma 
                          clipping (default is 0)
            plot:     *** CURRENTLY UNAVAILABLE, ALWAYS FALSE ***
                          plot the fit and the residual. In this each
                          indivual fit has to be approved, by typing 'y'
                          or 'n'
            getresidual:  if False, returns best-fit values instead of
                          residual. (default is True)
            showprogress: show progress status for large data.
                          default is True.
            minnrow:      minimum number of input spectra to show.
                          default is 1000.
            outlog:       Output the coefficients of the best-fit
                          function to logger (default is False)
            blfile:       Name of a text file in which the best-fit
                          parameter values to be written
                          (default is "": no file/logger output)
            csvformat:    if True blfile is csv-formatted, default is False.
            bltable:      name of a baseline table where fitting results
                          (coefficients, rms, etc.) are to be written.
                          if given, fitting results will NOT be output to
                          scantable (insitu=True) or None will be 
                          returned (insitu=False). 
                          (default is "": no table output)

        Example:
            # return a scan baselined by a cubic spline consisting of 2 pieces 
            # (i.e., 1 internal knot),
            # also with 3-sigma clipping, iteration up to 4 times
            bscan = scan.cspline_baseline(npiece=2,clipthresh=3.0,clipniter=4)
        
        Note:
            The best-fit parameter values output in logger and/or blfile are now
            based on specunit of 'channel'. 
        """
        
        try:
            varlist = vars()
            
            if insitu is None: insitu = rcParams['insitu']
            if insitu:
                workscan = self
            else:
                workscan = self.copy()

            if mask         is None: mask         = []
            if order        is None: order        = 5
            if clipthresh   is None: clipthresh   = 3.0
            if clipniter    is None: clipniter    = 0
            if plot         is None: plot         = False
            if getresidual  is None: getresidual  = True
            if showprogress is None: showprogress = True
            if minnrow      is None: minnrow      = 1000
            if outlog       is None: outlog       = False
            if blfile       is None: blfile       = ''
            if csvformat    is None: csvformat    = False
            if bltable      is None: bltable      = ''

            scsvformat = 'T' if csvformat else 'F'

            #CURRENTLY, PLOT=true UNAVAILABLE UNTIL cubic spline fitting is implemented as a fitter method. 
            workscan._chebyshev_baseline(mask, order,
                                         clipthresh, clipniter, 
                                         getresidual, 
                                         pack_progress_params(showprogress, 
                                                              minnrow),
                                         outlog, scsvformat+blfile, 
                                         bltable)
            workscan._add_history("chebyshev_baseline", varlist)

            if bltable == '':
                if insitu:
                    self._assign(workscan)
                else:
                    return workscan
            else:
                if not insitu:
                    return None
            
        except RuntimeError, e:
            raise_fitting_failure_exception(e)

    @asaplog_post_dec
    def auto_chebyshev_baseline(self, mask=None, order=None, insitu=None, 
                              clipthresh=None, clipniter=None,
                              edge=None, threshold=None, chan_avg_limit=None, 
                              getresidual=None, plot=None,
                              showprogress=None, minnrow=None, outlog=None,
                              blfile=None, csvformat=None, bltable=None):
        """\
        Return a scan which has been baselined (all rows) by Chebyshev polynomials. 
        Spectral lines are detected first using linefinder and masked out
        to avoid them affecting the baseline solution.

        Parameters:
            mask:           an optional mask retreived from scantable
            order:          the maximum order of Chebyshev polynomial (default is 5)
            insitu:         if False a new scantable is returned.
                            Otherwise, the scaling is done in-situ
                            The default is taken from .asaprc (False)
            clipthresh:     Clipping threshold. (default is 3.0, unit: sigma)
            clipniter:      maximum number of iteration of 'clipthresh'-sigma 
                            clipping (default is 0)
            edge:           an optional number of channel to drop at
                            the edge of spectrum. If only one value is
                            specified, the same number will be dropped
                            from both sides of the spectrum. Default
                            is to keep all channels. Nested tuples
                            represent individual edge selection for
                            different IFs (a number of spectral channels
                            can be different)
            threshold:      the threshold used by line finder. It is
                            better to keep it large as only strong lines
                            affect the baseline solution.
            chan_avg_limit: a maximum number of consequtive spectral
                            channels to average during the search of
                            weak and broad lines. The default is no
                            averaging (and no search for weak lines).
                            If such lines can affect the fitted baseline
                            (e.g. a high order polynomial is fitted),
                            increase this parameter (usually values up
                            to 8 are reasonable). Most users of this
                            method should find the default value sufficient.
            plot:       *** CURRENTLY UNAVAILABLE, ALWAYS FALSE ***
                            plot the fit and the residual. In this each
                            indivual fit has to be approved, by typing 'y'
                            or 'n'
            getresidual:    if False, returns best-fit values instead of
                            residual. (default is True)
            showprogress:   show progress status for large data.
                            default is True.
            minnrow:        minimum number of input spectra to show.
                            default is 1000.
            outlog:         Output the coefficients of the best-fit
                            function to logger (default is False)
            blfile:         Name of a text file in which the best-fit
                            parameter values to be written
                            (default is "": no file/logger output)
            csvformat:      if True blfile is csv-formatted, default is False.
            bltable:        name of a baseline table where fitting results
                            (coefficients, rms, etc.) are to be written.
                            if given, fitting results will NOT be output to
                            scantable (insitu=True) or None will be 
                            returned (insitu=False). 
                            (default is "": no table output)

        Example:
            bscan = scan.auto_cspline_baseline(npiece=3, insitu=False)
        
        Note:
            The best-fit parameter values output in logger and/or blfile are now
            based on specunit of 'channel'. 
        """

        try:
            varlist = vars()

            if insitu is None: insitu = rcParams['insitu']
            if insitu:
                workscan = self
            else:
                workscan = self.copy()
            
            if mask           is None: mask           = []
            if order          is None: order          = 5
            if clipthresh     is None: clipthresh     = 3.0
            if clipniter      is None: clipniter      = 0
            if edge           is None: edge           = (0, 0)
            if threshold      is None: threshold      = 3
            if chan_avg_limit is None: chan_avg_limit = 1
            if plot           is None: plot           = False
            if getresidual    is None: getresidual    = True
            if showprogress   is None: showprogress   = True
            if minnrow        is None: minnrow        = 1000
            if outlog         is None: outlog         = False
            if blfile         is None: blfile         = ''
            if csvformat      is None: csvformat      = False
            if bltable        is None: bltable        = ''

            scsvformat = 'T' if csvformat else 'F'

            #CURRENTLY, PLOT=true UNAVAILABLE UNTIL cubic spline fitting is implemented as a fitter method.
            workscan._auto_chebyshev_baseline(mask, order,
                                              clipthresh, clipniter, 
                                              normalise_edge_param(edge), 
                                              threshold, 
                                              chan_avg_limit, getresidual, 
                                              pack_progress_params(showprogress, 
                                                                   minnrow), 
                                              outlog, scsvformat+blfile,
                                              bltable)
            workscan._add_history("auto_chebyshev_baseline", varlist)

            if bltable == '':
                if insitu:
                    self._assign(workscan)
                else:
                    return workscan
            else:
                if not insitu:
                    return None
            
        except RuntimeError, e:
            raise_fitting_failure_exception(e)

    @asaplog_post_dec
    def poly_baseline(self, mask=None, order=None, insitu=None, 
                      clipthresh=None, clipniter=None, plot=None, 
                      getresidual=None, showprogress=None, minnrow=None, 
                      outlog=None, blfile=None, csvformat=None,
                      bltable=None):
        """\
        Return a scan which has been baselined (all rows) by a polynomial.
        Parameters:
            mask:         an optional mask
            order:        the order of the polynomial (default is 0)
            insitu:       if False a new scantable is returned.
                          Otherwise, the scaling is done in-situ
                          The default is taken from .asaprc (False)
            clipthresh:   Clipping threshold. (default is 3.0, unit: sigma)
            clipniter:    maximum number of iteration of 'clipthresh'-sigma 
                          clipping (default is 0)
            plot:         plot the fit and the residual. In this each
                          indivual fit has to be approved, by typing 'y'
                          or 'n'
            getresidual:  if False, returns best-fit values instead of
                          residual. (default is True)
            showprogress: show progress status for large data.
                          default is True.
            minnrow:      minimum number of input spectra to show.
                          default is 1000.
            outlog:       Output the coefficients of the best-fit
                          function to logger (default is False)
            blfile:       Name of a text file in which the best-fit
                          parameter values to be written
                          (default is "": no file/logger output)
            csvformat:    if True blfile is csv-formatted, default is False.
            bltable:      name of a baseline table where fitting results
                          (coefficients, rms, etc.) are to be written.
                          if given, fitting results will NOT be output to
                          scantable (insitu=True) or None will be 
                          returned (insitu=False). 
                          (default is "": no table output)

        Example:
            # return a scan baselined by a third order polynomial,
            # not using a mask
            bscan = scan.poly_baseline(order=3)
        """
        
        try:
            varlist = vars()
        
            if insitu is None: 
                insitu = rcParams["insitu"]
            if insitu:
                workscan = self
            else:
                workscan = self.copy()

            if mask         is None: mask         = []
            if order        is None: order        = 0
            if clipthresh   is None: clipthresh   = 3.0
            if clipniter    is None: clipniter    = 0
            if plot         is None: plot         = False
            if getresidual  is None: getresidual  = True
            if showprogress is None: showprogress = True
            if minnrow      is None: minnrow      = 1000
            if outlog       is None: outlog       = False
            if blfile       is None: blfile       = ''
            if csvformat    is None: csvformat    = False
            if bltable      is None: bltable      = ''

            scsvformat = 'T' if csvformat else 'F'

            if plot:
                outblfile = (blfile != "") and \
                    os.path.exists(os.path.expanduser(
                                    os.path.expandvars(blfile))
                                   )
                if outblfile: 
                    blf = open(blfile, "a")
                
                f = fitter()
                f.set_function(lpoly=order)
                
                rows = xrange(workscan.nrow())
                #if len(rows) > 0: workscan._init_blinfo()

                action = "H"
                for r in rows:
                    f.x = workscan._getabcissa(r)
                    f.y = workscan._getspectrum(r)
                    if mask:
                        f.mask = mask_and(mask, workscan._getmask(r))    # (CAS-1434)
                    else: # mask=None
                        f.mask = workscan._getmask(r)
                    
                    f.data = None
                    f.fit()

                    if action != "Y": # skip plotting when accepting all
                        f.plot(residual=True)
                    #accept_fit = raw_input("Accept fit ( [y]/n ): ")
                    #if accept_fit.upper() == "N":
                    #    #workscan._append_blinfo(None, None, None)
                    #    continue
                    accept_fit = self._get_verify_action("Accept fit?",action)
                    if r == 0: action = None
                    if accept_fit.upper() == "N":
                        continue
                    elif accept_fit.upper() == "R":
                        break
                    elif accept_fit.upper() == "A":
                        action = "Y"
                    
                    blpars = f.get_parameters()
                    masklist = workscan.get_masklist(f.mask, row=r, silent=True)
                    #workscan._append_blinfo(blpars, masklist, f.mask)
                    workscan._setspectrum((f.fitter.getresidual() 
                                           if getresidual else f.fitter.getfit()), r)
                    
                    if outblfile:
                        rms = workscan.get_rms(f.mask, r)
                        dataout = \
                            workscan.format_blparams_row(blpars["params"], 
                                                         blpars["fixed"], 
                                                         rms, str(masklist), 
                                                         r, True, csvformat)
                        blf.write(dataout)

                f._p.unmap()
                f._p = None

                if outblfile: 
                    blf.close()
            else:
                workscan._poly_baseline(mask, order,
                                        clipthresh, clipniter, #
                                        getresidual, 
                                        pack_progress_params(showprogress, 
                                                             minnrow), 
                                        outlog, scsvformat+blfile,
                                        bltable)  #
            
            workscan._add_history("poly_baseline", varlist)
            
            if insitu:
                self._assign(workscan)
            else:
                return workscan
            
        except RuntimeError, e:
            raise_fitting_failure_exception(e)

    @asaplog_post_dec
    def auto_poly_baseline(self, mask=None, order=None, insitu=None, 
                           clipthresh=None, clipniter=None, 
                           edge=None, threshold=None, chan_avg_limit=None,
                           getresidual=None, plot=None, 
                           showprogress=None, minnrow=None, outlog=None,
                           blfile=None, csvformat=None, bltable=None):
        """\
        Return a scan which has been baselined (all rows) by a polynomial.
        Spectral lines are detected first using linefinder and masked out
        to avoid them affecting the baseline solution.

        Parameters:
            mask:           an optional mask retreived from scantable
            order:          the order of the polynomial (default is 0)
            insitu:         if False a new scantable is returned.
                            Otherwise, the scaling is done in-situ
                            The default is taken from .asaprc (False)
            clipthresh:     Clipping threshold. (default is 3.0, unit: sigma)
            clipniter:      maximum number of iteration of 'clipthresh'-sigma 
                            clipping (default is 0)
            edge:           an optional number of channel to drop at
                            the edge of spectrum. If only one value is
                            specified, the same number will be dropped
                            from both sides of the spectrum. Default
                            is to keep all channels. Nested tuples
                            represent individual edge selection for
                            different IFs (a number of spectral channels
                            can be different)
            threshold:      the threshold used by line finder. It is
                            better to keep it large as only strong lines
                            affect the baseline solution.
            chan_avg_limit: a maximum number of consequtive spectral
                            channels to average during the search of
                            weak and broad lines. The default is no
                            averaging (and no search for weak lines).
                            If such lines can affect the fitted baseline
                            (e.g. a high order polynomial is fitted),
                            increase this parameter (usually values up
                            to 8 are reasonable). Most users of this
                            method should find the default value sufficient.
            plot:           plot the fit and the residual. In this each
                            indivual fit has to be approved, by typing 'y'
                            or 'n'
            getresidual:    if False, returns best-fit values instead of
                            residual. (default is True)
            showprogress:   show progress status for large data.
                            default is True.
            minnrow:        minimum number of input spectra to show.
                            default is 1000.
            outlog:         Output the coefficients of the best-fit
                            function to logger (default is False)
            blfile:         Name of a text file in which the best-fit
                            parameter values to be written
                            (default is "": no file/logger output)
            csvformat:      if True blfile is csv-formatted, default is False.
            bltable:        name of a baseline table where fitting results
                            (coefficients, rms, etc.) are to be written.
                            if given, fitting results will NOT be output to
                            scantable (insitu=True) or None will be 
                            returned (insitu=False). 
                            (default is "": no table output)

        Example:
            bscan = scan.auto_poly_baseline(order=7, insitu=False)
        """

        try:
            varlist = vars()

            if insitu is None: 
                insitu = rcParams['insitu']
            if insitu:
                workscan = self
            else:
                workscan = self.copy()

            if mask           is None: mask           = []
            if order          is None: order          = 0
            if clipthresh     is None: clipthresh     = 3.0
            if clipniter      is None: clipniter      = 0
            if edge           is None: edge           = (0, 0)
            if threshold      is None: threshold      = 3
            if chan_avg_limit is None: chan_avg_limit = 1
            if plot           is None: plot           = False
            if getresidual    is None: getresidual    = True
            if showprogress   is None: showprogress   = True
            if minnrow        is None: minnrow        = 1000
            if outlog         is None: outlog         = False
            if blfile         is None: blfile         = ''
            if csvformat      is None: csvformat      = False
            if bltable        is None: bltable        = ''

            scsvformat = 'T' if csvformat else 'F'

            edge = normalise_edge_param(edge)

            if plot:
                outblfile = (blfile != "") and \
                    os.path.exists(os.path.expanduser(os.path.expandvars(blfile)))
                if outblfile: blf = open(blfile, "a")
                
                from asap.asaplinefind import linefinder
                fl = linefinder()
                fl.set_options(threshold=threshold, avg_limit=chan_avg_limit)
                fl.set_scan(workscan)
                
                f = fitter()
                f.set_function(lpoly=order)

                rows = xrange(workscan.nrow())
                #if len(rows) > 0: workscan._init_blinfo()

                action = "H"
                for r in rows:
                    idx = 2*workscan.getif(r)
                    if mask:
                        msk = mask_and(mask, workscan._getmask(r)) # (CAS-1434)
                    else: # mask=None
                        msk = workscan._getmask(r)
                    fl.find_lines(r, msk, edge[idx:idx+2])  

                    f.x = workscan._getabcissa(r)
                    f.y = workscan._getspectrum(r)
                    f.mask = fl.get_mask()
                    f.data = None
                    f.fit()

                    if action != "Y": # skip plotting when accepting all
                        f.plot(residual=True)
                    #accept_fit = raw_input("Accept fit ( [y]/n ): ")
                    accept_fit = self._get_verify_action("Accept fit?",action)
                    if r == 0: action = None
                    if accept_fit.upper() == "N":
                        #workscan._append_blinfo(None, None, None)
                        continue
                    elif accept_fit.upper() == "R":
                        break
                    elif accept_fit.upper() == "A":
                        action = "Y"

                    blpars = f.get_parameters()
                    masklist = workscan.get_masklist(f.mask, row=r, silent=True)
                    #workscan._append_blinfo(blpars, masklist, f.mask)
                    workscan._setspectrum(
                        (f.fitter.getresidual() if getresidual 
                                                else f.fitter.getfit()), r
                        )

                    if outblfile:
                        rms = workscan.get_rms(f.mask, r)
                        dataout = \
                            workscan.format_blparams_row(blpars["params"], 
                                                         blpars["fixed"], 
                                                         rms, str(masklist), 
                                                         r, True, csvformat)
                        blf.write(dataout)
                    
                f._p.unmap()
                f._p = None

                if outblfile: blf.close()
            else:
                workscan._auto_poly_baseline(mask, order,
                                             clipthresh, clipniter, 
                                             edge, threshold, 
                                             chan_avg_limit, getresidual, 
                                             pack_progress_params(showprogress,
                                                                  minnrow), 
                                             outlog, scsvformat+blfile,
                                             bltable)
            workscan._add_history("auto_poly_baseline", varlist)

            if bltable == '':
                if insitu:
                    self._assign(workscan)
                else:
                    return workscan
            else:
                if not insitu:
                    return None
            
        except RuntimeError, e:
            raise_fitting_failure_exception(e)

    def _init_blinfo(self):
        """\
        Initialise the following three auxiliary members:
           blpars : parameters of the best-fit baseline, 
           masklists : mask data (edge positions of masked channels) and 
           actualmask : mask data (in boolean list), 
        to keep for use later (including output to logger/text files). 
        Used by poly_baseline() and auto_poly_baseline() in case of
        'plot=True'. 
        """
        self.blpars = []
        self.masklists = []
        self.actualmask = []
        return

    def _append_blinfo(self, data_blpars, data_masklists, data_actualmask):
        """\
        Append baseline-fitting related info to blpars, masklist and
        actualmask. 
        """
        self.blpars.append(data_blpars)
        self.masklists.append(data_masklists)
        self.actualmask.append(data_actualmask)
        return
        
    @asaplog_post_dec
    def rotate_linpolphase(self, angle):
        """\
        Rotate the phase of the complex polarization O=Q+iU correlation.
        This is always done in situ in the raw data.  So if you call this
        function more than once then each call rotates the phase further.

        Parameters:

            angle:   The angle (degrees) to rotate (add) by.

        Example::

            scan.rotate_linpolphase(2.3)

        """
        varlist = vars()
        self._math._rotate_linpolphase(self, angle)
        self._add_history("rotate_linpolphase", varlist)
        return

    @asaplog_post_dec
    def rotate_xyphase(self, angle):
        """\
        Rotate the phase of the XY correlation.  This is always done in situ
        in the data.  So if you call this function more than once
        then each call rotates the phase further.

        Parameters:

            angle:   The angle (degrees) to rotate (add) by.

        Example::

            scan.rotate_xyphase(2.3)

        """
        varlist = vars()
        self._math._rotate_xyphase(self, angle)
        self._add_history("rotate_xyphase", varlist)
        return

    @asaplog_post_dec
    def swap_linears(self):
        """\
        Swap the linear polarisations XX and YY, or better the first two
        polarisations as this also works for ciculars.
        """
        varlist = vars()
        self._math._swap_linears(self)
        self._add_history("swap_linears", varlist)
        return

    @asaplog_post_dec
    def invert_phase(self):
        """\
        Invert the phase of the complex polarisation
        """
        varlist = vars()
        self._math._invert_phase(self)
        self._add_history("invert_phase", varlist)
        return

    @asaplog_post_dec
    def add(self, offset, insitu=None):
        """\
        Return a scan where all spectra have the offset added

        Parameters:

            offset:      the offset

            insitu:      if False a new scantable is returned.
                         Otherwise, the scaling is done in-situ
                         The default is taken from .asaprc (False)

        """
        if insitu is None: insitu = rcParams['insitu']
        self._math._setinsitu(insitu)
        varlist = vars()
        s = scantable(self._math._unaryop(self, offset, "ADD", False))
        s._add_history("add", varlist)
        if insitu:
            self._assign(s)
        else:
            return s

    @asaplog_post_dec
    def scale(self, factor, tsys=True, insitu=None):
        """\

        Return a scan where all spectra are scaled by the given 'factor'

        Parameters:

            factor:      the scaling factor (float or 1D float list)

            insitu:      if False a new scantable is returned.
                         Otherwise, the scaling is done in-situ
                         The default is taken from .asaprc (False)

            tsys:        if True (default) then apply the operation to Tsys
                         as well as the data

        """
        if insitu is None: insitu = rcParams['insitu']
        self._math._setinsitu(insitu)
        varlist = vars()
        s = None
        import numpy
        if isinstance(factor, list) or isinstance(factor, numpy.ndarray):
            if isinstance(factor[0], list) or isinstance(factor[0], 
                                                         numpy.ndarray):
                from asapmath import _array2dOp
                s = _array2dOp( self, factor, "MUL", tsys, insitu )
            else:
                s = scantable( self._math._arrayop( self, factor, 
                                                    "MUL", tsys ) )
        else:
            s = scantable(self._math._unaryop(self, factor, "MUL", tsys))
        s._add_history("scale", varlist)
        if insitu:
            self._assign(s)
        else:
            return s

    @preserve_selection
    def set_sourcetype(self, match, matchtype="pattern", 
                       sourcetype="reference"):
        """\
        Set the type of the source to be an source or reference scan
        using the provided pattern.

        Parameters:

            match:          a Unix style pattern, regular expression or selector

            matchtype:      'pattern' (default) UNIX style pattern or
                            'regex' regular expression

            sourcetype:     the type of the source to use (source/reference)

        """
        varlist = vars()
        stype = -1
        if sourcetype.lower().startswith("r") or sourcetype.lower() == "off":
            stype = 1
        elif sourcetype.lower().startswith("s") or sourcetype.lower() == "on":
            stype = 0
        else:
            raise ValueError("Illegal sourcetype use s(ource)/on or r(eference)/off")
        if matchtype.lower().startswith("p"):
            matchtype = "pattern"
        elif matchtype.lower().startswith("r"):
            matchtype = "regex"
        else:
            raise ValueError("Illegal matchtype, use p(attern) or r(egex)")
        sel = selector()
        if isinstance(match, selector):
            sel = match
        else:
            sel.set_query("SRCNAME=%s('%s')" % (matchtype, match))
        self.set_selection(sel)
        self._setsourcetype(stype)
        self._add_history("set_sourcetype", varlist)


    def set_sourcename(self, name):
        varlist = vars()
        self._setsourcename(name)
        self._add_history("set_sourcename", varlist)

    @asaplog_post_dec
    @preserve_selection
    def auto_quotient(self, preserve=True, mode='paired', verify=False):
        """\
        This function allows to build quotients automatically.
        It assumes the observation to have the same number of
        "ons" and "offs"

        Parameters:

            preserve:       you can preserve (default) the continuum or
                            remove it.  The equations used are

                            preserve: Output = Toff * (on/off) - Toff

                            remove:   Output = Toff * (on/off) - Ton

            mode:           the on/off detection mode
                            'paired' (default)
                            identifies 'off' scans by the
                            trailing '_R' (Mopra/Parkes) or
                            '_e'/'_w' (Tid) and matches
                            on/off pairs from the observing pattern
                            'time'
                            finds the closest off in time

        .. todo:: verify argument is not implemented

        """
        varlist = vars()
        modes = ["time", "paired"]
        if not mode in modes:
            msg = "please provide valid mode. Valid modes are %s" % (modes)
            raise ValueError(msg)
        s = None
        if mode.lower() == "paired":
            from asap._asap import srctype
            sel = self.get_selection()
            #sel.set_query("SRCTYPE==psoff")
            sel.set_types(srctype.psoff)
            self.set_selection(sel)
            offs = self.copy()
            #sel.set_query("SRCTYPE==pson")
            sel.set_types(srctype.pson)
            self.set_selection(sel)
            ons = self.copy()
            s = scantable(self._math._quotient(ons, offs, preserve))
        elif mode.lower() == "time":
            s = scantable(self._math._auto_quotient(self, mode, preserve))
        s._add_history("auto_quotient", varlist)
        return s

    @asaplog_post_dec
    def mx_quotient(self, mask = None, weight='median', preserve=True):
        """\
        Form a quotient using "off" beams when observing in "MX" mode.

        Parameters:

            mask:           an optional mask to be used when weight == 'stddev'

            weight:         How to average the off beams.  Default is 'median'.

            preserve:       you can preserve (default) the continuum or
                            remove it.  The equations used are:

                                preserve: Output = Toff * (on/off) - Toff

                                remove:   Output = Toff * (on/off) - Ton

        """
        mask = mask or ()
        varlist = vars()
        on = scantable(self._math._mx_extract(self, 'on'))
        preoff = scantable(self._math._mx_extract(self, 'off'))
        off = preoff.average_time(mask=mask, weight=weight, scanav=False)
        from asapmath  import quotient
        q = quotient(on, off, preserve)
        q._add_history("mx_quotient", varlist)
        return q

    @asaplog_post_dec
    def freq_switch(self, insitu=None):
        """\
        Apply frequency switching to the data.

        Parameters:

            insitu:      if False a new scantable is returned.
                         Otherwise, the swictching is done in-situ
                         The default is taken from .asaprc (False)

        """
        if insitu is None: insitu = rcParams['insitu']
        self._math._setinsitu(insitu)
        varlist = vars()
        s = scantable(self._math._freqswitch(self))
        s._add_history("freq_switch", varlist)
        if insitu:
            self._assign(s)
        else:
            return s

    @asaplog_post_dec
    def recalc_azel(self):
        """Recalculate the azimuth and elevation for each position."""
        varlist = vars()
        self._recalcazel()
        self._add_history("recalc_azel", varlist)
        return

    @asaplog_post_dec
    def __add__(self, other):
        """
        implicit on all axes and on Tsys
        """
        varlist = vars()
        s = self.__op( other, "ADD" ) 
        s._add_history("operator +", varlist)
        return s

    @asaplog_post_dec
    def __sub__(self, other):
        """
        implicit on all axes and on Tsys
        """
        varlist = vars()
        s = self.__op( other, "SUB" ) 
        s._add_history("operator -", varlist)
        return s

    @asaplog_post_dec
    def __mul__(self, other):
        """
        implicit on all axes and on Tsys
        """
        varlist = vars()
        s = self.__op( other, "MUL" ) ;
        s._add_history("operator *", varlist)
        return s


    @asaplog_post_dec
    def __div__(self, other):
        """
        implicit on all axes and on Tsys
        """
        varlist = vars()
        s = self.__op( other, "DIV" ) 
        s._add_history("operator /", varlist)
        return s

    @asaplog_post_dec
    def __op( self, other, mode ):
        s = None
        if isinstance(other, scantable):
            s = scantable(self._math._binaryop(self, other, mode))
        elif isinstance(other, float):
            if other == 0.0:
                raise ZeroDivisionError("Dividing by zero is not recommended")
            s = scantable(self._math._unaryop(self, other, mode, False))
        elif isinstance(other, list) or isinstance(other, numpy.ndarray):
            if isinstance(other[0], list) \
                    or isinstance(other[0], numpy.ndarray):
                from asapmath import _array2dOp
                s = _array2dOp( self, other, mode, False )
            else:
                s = scantable( self._math._arrayop( self, other, 
                                                    mode, False ) )
        else:
            raise TypeError("Other input is not a scantable or float value")
        return s

    @asaplog_post_dec
    def get_fit(self, row=0):
        """\
        Print or return the stored fits for a row in the scantable

        Parameters:

            row:    the row which the fit has been applied to.

        """
        if row > self.nrow():
            return
        from asap.asapfit import asapfit
        fit = asapfit(self._getfit(row))
        asaplog.push( '%s' %(fit) )
        return fit.as_dict()

    @preserve_selection
    def flag_nans(self):
        """\
        Utility function to flag NaN values in the scantable.
        """
        import numpy
        basesel = self.get_selection()
        for i in range(self.nrow()):
            sel = self.get_row_selector(i)
            self.set_selection(basesel+sel)
            nans = numpy.isnan(self._getspectrum(0))
            if numpy.any(nans):
                bnans = [ bool(v) for v in nans]
                self.flag(bnans)
        
        self.set_selection(basesel)

    def get_row_selector(self, rowno):
        return selector(rows=[rowno])

    def _add_history(self, funcname, parameters):
        if not rcParams['scantable.history']:
            return
        # create date
        sep = "##"
        from datetime import datetime
        dstr = datetime.now().strftime('%Y/%m/%d %H:%M:%S')
        hist = dstr+sep
        hist += funcname+sep#cdate+sep
        if parameters.has_key('self'): 
            del parameters['self']
        for k, v in parameters.iteritems():
            if type(v) is dict:
                for k2, v2 in v.iteritems():
                    hist += k2
                    hist += "="
                    if isinstance(v2, scantable):
                        hist += 'scantable'
                    elif k2 == 'mask':
                        if isinstance(v2, list) or isinstance(v2, tuple):
                            hist += str(self._zip_mask(v2))
                        else:
                            hist += str(v2)
                    else:
                        hist += str(v2)
            else:
                hist += k
                hist += "="
                if isinstance(v, scantable):
                    hist += 'scantable'
                elif k == 'mask':
                    if isinstance(v, list) or isinstance(v, tuple):
                        hist += str(self._zip_mask(v))
                    else:
                        hist += str(v)
                else:
                    hist += str(v)
            hist += sep
        hist = hist[:-2] # remove trailing '##'
        self._addhistory(hist)


    def _zip_mask(self, mask):
        mask = list(mask)
        i = 0
        segments = []
        while mask[i:].count(1):
            i += mask[i:].index(1)
            if mask[i:].count(0):
                j = i + mask[i:].index(0)
            else:
                j = len(mask)
            segments.append([i, j])
            i = j
        return segments

    def _get_ordinate_label(self):
        fu = "("+self.get_fluxunit()+")"
        import re
        lbl = "Intensity"
        if re.match(".K.", fu):
            lbl = "Brightness Temperature "+ fu
        elif re.match(".Jy.", fu):
            lbl = "Flux density "+ fu
        return lbl

    def _check_ifs(self):
#        return len(set([self.nchan(i) for i in self.getifnos()])) == 1
        nchans = [self.nchan(i) for i in self.getifnos()]
        nchans = filter(lambda t: t > 0, nchans)
        return (sum(nchans)/len(nchans) == nchans[0])

    @asaplog_post_dec
    def _fill(self, names, unit, average, opts={}):
        first = True
        fullnames = []
        for name in names:
            name = os.path.expandvars(name)
            name = os.path.expanduser(name)
            if not os.path.exists(name):
                msg = "File '%s' does not exists" % (name)
                raise IOError(msg)
            fullnames.append(name)
        if average:
            asaplog.push('Auto averaging integrations')
        stype = int(rcParams['scantable.storage'].lower() == 'disk')
        for name in fullnames:
            tbl = Scantable(stype)
            if is_ms( name ):
                r = msfiller( tbl )
            else:
                r = filler( tbl )
            msg = "Importing %s..." % (name)
            asaplog.push(msg, False)
            r.open(name, opts)
            rx = rcParams['scantable.reference']
            r.setreferenceexpr(rx)
            r.fill()
            if average:
                tbl = self._math._average((tbl, ), (), 'NONE', 'SCAN')
            if not first:
                tbl = self._math._merge([self, tbl])
            Scantable.__init__(self, tbl)
            r.close()
            del r, tbl
            first = False
            #flush log
        asaplog.post()
        if unit is not None:
            self.set_fluxunit(unit)
        if not is_casapy():
            self.set_freqframe(rcParams['scantable.freqframe'])

    def _get_verify_action( self, msg, action=None ):
        valid_act = ['Y', 'N', 'A', 'R']
        if not action or not isinstance(action, str):
            action = raw_input("%s [Y/n/a/r] (h for help): " % msg)
        if action == '':
            return "Y"
        elif (action.upper()[0] in valid_act):
            return action.upper()[0]
        elif (action.upper()[0] in ['H','?']):
            print "Available actions of verification [Y|n|a|r]"
            print " Y : Yes for current data (default)"
            print " N : No for current data"
            print " A : Accept all in the following and exit from verification"
            print " R : Reject all in the following and exit from verification"
            print " H or ?: help (show this message)"
            return self._get_verify_action(msg)
        else:
            return 'Y'

    def __getitem__(self, key):
        if key < 0:
            key += self.nrow()
        if key >= self.nrow():
            raise IndexError("Row index out of range.")
        return self._getspectrum(key)

    def __setitem__(self, key, value):
        if key < 0:
            key += self.nrow()
        if key >= self.nrow():
            raise IndexError("Row index out of range.")
        if not hasattr(value, "__len__") or \
                len(value) > self.nchan(self.getif(key)):
            raise ValueError("Spectrum length doesn't match.")
        return self._setspectrum(value, key)

    def __len__(self):
        return self.nrow()

    def __iter__(self):
        for i in range(len(self)):
            yield self[i]
