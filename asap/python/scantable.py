"""This module defines the scantable class."""

import os
try:
    from functools import wraps as wraps_dec
except ImportError:
    from asap.compatibility import wraps as wraps_dec

from asap.env import is_casapy
from asap._asap import Scantable
from asap._asap import filler
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
        #if ( l.find('Scantable') != -1 ):
        if ( l.find('Measurement Set') == -1 ):
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
    
class scantable(Scantable):
    """\
        The ASAP container for scans (single-dish data).
    """

    @asaplog_post_dec
    #def __init__(self, filename, average=None, unit=None, getpt=None,
    #             antenna=None, parallactify=None):
    def __init__(self, filename, average=None, unit=None, parallactify=None, **args):
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

            getpt:        for MeasurementSet input data only:
                          If True, all pointing data are filled.
                          The deafult is False, which makes time to load
                          the MS data faster in some cases.

            antenna:      Antenna selection. integer (id) or string (name or id).

            parallactify: Indicate that the data had been parallatified. Default
                          is taken from rc file.

        """
        if average is None:
            average = rcParams['scantable.autoaverage']
        #if getpt is None:
        #    getpt = True
        #if antenna is not None:
        #    asaplog.push("Antenna selection currently unsupported."
        #                 "Using ''")
        #    asaplog.post('WARN')
        #if antenna is None:
        #    antenna = ''
        #elif type(antenna) == int:
        #    antenna = '%s' % antenna
        #elif type(antenna) == list:
        #    tmpstr = ''
        #    for i in range( len(antenna) ):
        #        if type(antenna[i]) == int:
        #            tmpstr = tmpstr + ('%s,'%(antenna[i]))
        #        elif type(antenna[i]) == str:
        #            tmpstr=tmpstr+antenna[i]+','
        #        else:
        #            raise TypeError('Bad antenna selection.')
        #    antenna = tmpstr.rstrip(',')
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
                    # do not reset to the default freqframe
                    #self.set_freqframe(rcParams['scantable.freqframe'])
                #elif os.path.isdir(filename) \
                #         and not os.path.exists(filename+'/table.f1'):
                elif is_ms(filename):
                    # Measurement Set
                    opts={'ms': {}}
                    mskeys=['getpt','antenna']
                    for key in mskeys:
                        if key in args.keys():
                            opts['ms'][key] = args[key]
                    #self._fill([filename], unit, average, getpt, antenna)
                    self._fill([filename], unit, average, opts)
                elif os.path.isfile(filename):
                    #self._fill([filename], unit, average, getpt, antenna)
                    self._fill([filename], unit, average)
                else:
                    msg = "The given file '%s'is not a valid " \
                          "asap table." % (filename)
                    raise IOError(msg)
            elif (isinstance(filename, list) or isinstance(filename, tuple)) \
                  and isinstance(filename[-1], str):
                #self._fill(filename, unit, average, getpt, antenna)
                self._fill(filename, unit, average)
        self.parallactify(parallactify)
        self._add_history("scantable", varlist)

    @asaplog_post_dec
    def save(self, name=None, format=None, overwrite=False):
        """\
        Store the scantable on disk. This can be an asap (aips++) Table,
        SDFITS or MS2 format.

        Parameters:

            name:        the name of the outputfile. For format "ASCII"
                         this is the root file name (data in 'name'.txt
                         and header in 'name'_header.txt)

            format:      an optional file format. Default is ASAP.
                         Allowed are:

                            * 'ASAP' (save as ASAP [aips++] Table),
                            * 'SDFITS' (save as SDFITS file)
                            * 'ASCII' (saves as ascii text file)
                            * 'MS2' (saves as an casacore MeasurementSet V2)
                            * 'FITS' (save as image FITS - not readable by class)
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
            raise RuntimeError( 'Please specify a scanno to drop from the scantable' )
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
        return Scantable._summary(self, True)

    def summary(self, filename=None):
        """\
        Print a summary of the contents of this scantable.

        Parameters:

            filename:    the name of a file to write the putput to
                         Default - no file output

        """
        info = Scantable._summary(self, True)
        if filename is not None:
            if filename is "":
                filename = 'scantable_summary.txt'
            from os.path import expandvars, isdir
            filename = expandvars(filename)
            if not isdir(filename):
                data = open(filename, 'w')
                data.write(info)
                data.close()
            else:
                msg = "Illegal file name '%s'." % (filename)
                raise IOError(msg)
        return page(info)

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
        """Return the spectrum for the current row in the scantable as a list.

        Parameters:

             spec:   the new spectrum

             rowno:  the row number to set the spectrum for

        """
        assert(len(spec) == self.nchan())
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
                          any combination of "pols", "ifs", "beams", "scans",
                          "cycles", "name", "query"

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
                        raise KeyError("Invalid selection key '%s', valid keys are %s" % (k, selector.fields))
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
        if insitu is None: insitu = rcParams['insitu']
        if not insitu:
            workscan = self.copy()
        else:
            workscan = self
        # Select a row
        sel=selector()
        sel.set_scans([workscan.getscan(row)])
        sel.set_cycles([workscan.getcycle(row)])
        sel.set_beams([workscan.getbeam(row)])
        sel.set_ifs([workscan.getif(row)])
        sel.set_polarisations([workscan.getpol(row)])
        sel.set_name(workscan._getsourcename(row))
        workscan.set_selection(sel)
        if not workscan.nrow() == 1:
            msg = "Cloud not identify single row. %d rows selected."%(workscan.nrow())
            raise RuntimeError(msg)
        del sel
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

    def _get_column(self, callback, row=-1):
        """
        """
        if row == -1:
            return [callback(i) for i in range(self.nrow())]
        else:
            if  0 <= row < self.nrow():
                return callback(row)


    def get_time(self, row=-1, asdatetime=False):
        """\
        Get a list of time stamps for the observations.
        Return a datetime object for each integration time stamp in the scantable.

        Parameters:

            row:          row no of integration. Default -1 return all rows

            asdatetime:   return values as datetime objects rather than strings

        """
        from time import strptime
        from datetime import datetime
        times = self._get_column(self._gettime, row)
        if not asdatetime:
            return times
        format = "%Y/%m/%d/%H:%M:%S"
        if isinstance(times, list):
            return [datetime(*strptime(i, format)[:6]) for i in times]
        else:
            return datetime(*strptime(times, format)[:6])


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
    def flag(self, mask=None, unflag=False):
        """\
        Flag the selected data using an optional channel mask.

        Parameters:

            mask:   an optional channel mask, created with create_mask. Default
                    (no mask) is all channels.

            unflag:    if True, unflag the data

        """
        varlist = vars()
        mask = mask or []
        self._flag(mask, unflag)
        self._add_history("flag", varlist)

    @asaplog_post_dec
    def flag_row(self, rows=[], unflag=False):
        """\
        Flag the selected data in row-based manner.

        Parameters:

            rows:   list of row numbers to be flagged. Default is no row
                    (must be explicitly specified to execute row-based flagging).

            unflag: if True, unflag the data.

        """
        varlist = vars()
        self._flag_row(rows, unflag)
        self._add_history("flag_row", varlist)

    @asaplog_post_dec
    def clip(self, uthres=None, dthres=None, clipoutside=True, unflag=False):
        """\
        Flag the selected data outside a specified range (in channel-base)

        Parameters:

            uthres:      upper threshold.

            dthres:      lower threshold

            clipoutside: True for flagging data outside the range [dthres:uthres].
                         False for glagging data inside the range.

            unflag:      if True, unflag the data.

        """
        varlist = vars()
        self._clip(uthres, dthres, clipoutside, unflag)
        self._add_history("clip", varlist)

    @asaplog_post_dec
    def lag_flag(self, start, end, unit="MHz", insitu=None):
        """\
        Flag the data in 'lag' space by providing a frequency to remove.
        Flagged data in the scantable gets interpolated over the region.
        No taper is applied.

        Parameters:

            start:    the start frequency (really a period within the
                      bandwidth)  or period to remove

            end:      the end frequency or period to remove

            unit:     the frequency unit (default "MHz") or "" for
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
        n = self.nchan()
        msk = _n_bools(n, False)
        # test if args is a 'list' or a 'normal *args - UGLY!!!

        ws = (isinstance(args[-1][-1], int) or isinstance(args[-1][-1], float)) \
             and args or args[0]
        for window in ws:
            if len(window) == 1:
                window = [window[0], window[0]]
            if len(window) == 0 or  len(window) > 2:
                raise ValueError("A window needs to be defined as [start(, end)]")
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

    def get_masklist(self, mask=None, row=0):
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
        if len(mask) < 2:
            raise TypeError("The mask elements should be > 1")
        if self.nchan() != len(mask):
            msg = "Number of channels in scantable != number of mask elements"
            raise TypeError(msg)
        data = self._getabcissa(row)
        u = self._getcoordinfo()[0]
        if u == "":
            u = "channel"
        msg = "The current mask window unit is %s" % u
        i = self._check_ifs()
        if not i:
            msg += "\nThis mask is only valid for IF=%d" % (self.getif(i))
        asaplog.push(msg)
        masklist=[]
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
        if len(mask) < 2:
            raise TypeError("The mask elements should be > 1")
        istart=[]
        iend=[]
        if mask[0]: istart.append(0)
        for i in range(len(mask)-1):
            if not mask[i] and mask[i+1]:
                istart.append(i+1)
            elif mask[i] and not mask[i+1]:
                iend.append(i)
        if mask[len(mask)-1]: iend.append(len(mask)-1)
        if len(istart) != len(iend):
            raise RuntimeError("Numbers of mask start != mask end.")
        for i in range(len(istart)):
            if istart[i] > iend[i]:
                raise RuntimeError("Mask start index > mask end index")
                break
        return istart,iend

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
            rfreqs={}
            idlist = self.getmolnos()
            for i in idlist:
                rfreqs[i]=list(self._getrestfreqs(i))
            return rfreqs
        else:
            if type(ids)==list or type(ids)==tuple:
                rfreqs={}
                for i in ids:
                    rfreqs[i]=list(self._getrestfreqs(i))
                return rfreqs
            else:
                return list(self._getrestfreqs(ids))
            #return list(self._getrestfreqs(ids))

    def set_restfreqs(self, freqs=None, unit='Hz'):
        """\
        Set or replace the restfrequency specified and
        If the 'freqs' argument holds a scalar,
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
                selection.set_name("ORION*")
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
                    sel = selector()
                    savesel = self._getselection()
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
                sel = selector()
                savesel = self._getselection()
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
            sel = selector()
            savesel = self._getselection()
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

    def shift_refpix(self, delta):
        """\
        Shift the reference pixel of the Spectra Coordinate by an
        integer amount.

        Parameters:

            delta:   the amount to shift by

        *Note*:

            Be careful using this with broadband data.

        """
        Scantable.shift_refpix(self, delta)

    @asaplog_post_dec
    def history(self, filename=None):
        """\
        Print the history. Optionally to a file.

        Parameters:

            filename:    The name  of the file to save the history to.

        """
        hist = list(self._gethistory())
        out = "-"*80
        for h in hist:
            if h.startswith("---"):
                out = "\n".join([out, h])
            else:
                items = h.split("##")
                date = items[0]
                func = items[1]
                items = items[2:]
                out += "\n"+date+"\n"
                out += "Function: %s\n  Parameters:" % (func)
                for i in items:
                    s = i.split("=")
                    out += "\n   %s = %s" % (s[0], s[1])
                out = "\n".join([out, "-"*80])
        if filename is not None:
            if filename is "":
                filename = 'scantable_history.txt'
            import os
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
    def average_time(self, mask=None, scanav=False, weight='tint', align=False):
        """\
        Return the (time) weighted average of a scan.

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

        Example::

            # time average the scantable without using a mask
            newscan = scan.average_time()

        """
        varlist = vars()
        weight = weight or 'TINT'
        mask = mask or ()
        scanav = (scanav and 'SCAN') or 'NONE'
        scan = (self, )

        if align:
            scan = (self.freq_align(insitu=False), )
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

            d:           the geomtric diameter (metres)

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
                         "ELEVATION" (degrees) and "FACTOR" (multiply data
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
                         Values are  "nearest", "linear" (default), "cubic"
                         and "spline"

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
                         Choose from "nearest", "linear", "cubic" (default)
                         and "spline"

            insitu:      if False a new scantable is returned.
                         Otherwise, the scaling is done in-situ
                         The default is taken from .asaprc (False)

        """
        if insitu is None: insitu = rcParams["insitu"]
        self._math._setinsitu(insitu)
        varlist = vars()
        reftime = reftime or ""
        s = scantable(self._math._freq_align(self, reftime, method))
        s._add_history("freq_align", varlist)
        if insitu: self._assign(s)
        else: return s

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
        if insitu is None: insitu = rcParams['insitu']
        self._math._setinsitu(insitu)
        varlist = vars()
        if not hasattr(tau, "__len__"):
            tau = [tau]
        s = scantable(self._math._opacity(self, tau))
        s._add_history("opacity", varlist)
        if insitu: self._assign(s)
        else: return s

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
        if insitu is None: insitu = rcParams['insitu']
        self._math._setinsitu(insitu)
        varlist = vars()
        s = scantable(self._math._bin(self, width))
        s._add_history("bin", varlist)
        if insitu:
            self._assign(s)
        else:
            return s

    @asaplog_post_dec
    def resample(self, width=5, method='cubic', insitu=None):
        """\
        Return a scan where all spectra have been binned up.

        Parameters:

            width:       The bin width (default=5) in pixels

            method:      Interpolation method when correcting from a table.
                         Values are  "nearest", "linear", "cubic" (default)
                         and "spline"

            insitu:      if False a new scantable is returned.
                         Otherwise, the scaling is done in-situ
                         The default is taken from .asaprc (False)

        """
        if insitu is None: insitu = rcParams['insitu']
        self._math._setinsitu(insitu)
        varlist = vars()
        s = scantable(self._math._resample(self, method, width))
        s._add_history("resample", varlist)
        if insitu: self._assign(s)
        else: return s

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
                        "linear", "circular", "stokes" and "linpol"

        """
        varlist = vars()
        s = scantable(self._math._convertpol(self, poltype))
        s._add_history("convert_pol", varlist)
        return s

    @asaplog_post_dec
    def smooth(self, kernel="hanning", width=5.0, order=2, plot=False, insitu=None):
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

        if plot:
            if rcParams['plotter.gui']:
                from asap.asaplotgui import asaplotgui as asaplot
            else:
                from asap.asaplot import asaplot
            self._p=asaplot()
            self._p.set_panels()
            ylab=s._get_ordinate_label()
            #self._p.palette(0,["#777777","red"])
            for r in xrange(s.nrow()):
                xsm=s._getabcissa(r)
                ysm=s._getspectrum(r)
                xorg=orgscan._getabcissa(r)
                yorg=orgscan._getspectrum(r)
                self._p.clear()
                self._p.hold()
                self._p.set_axes('ylabel',ylab)
                self._p.set_axes('xlabel',s._getabcissalabel(r))
                self._p.set_axes('title',s._getsourcename(r))
                self._p.set_line(label='Original',color="#777777")
                self._p.plot(xorg,yorg)
                self._p.set_line(label='Smoothed',color="red")
                self._p.plot(xsm,ysm)
                ### Ugly part for legend
                for i in [0,1]:
                    self._p.subplots[0]['lines'].append([self._p.subplots[0]['axes'].lines[i]])
                self._p.release()
                ### Ugly part for legend
                self._p.subplots[0]['lines']=[]
                res = raw_input("Accept smoothing ([y]/n): ")
                if res.upper() == 'N':
                    s._setspectrum(yorg, r)
            self._p.unmap()
            self._p = None
            del orgscan

        if insitu: self._assign(s)
        else: return s

    @asaplog_post_dec
    def old_poly_baseline(self, mask=None, order=0, plot=False, uselin=False, insitu=None, rows=None):
        """\
        Return a scan which has been baselined (all rows) by a polynomial.
        
        Parameters:

            mask:       an optional mask

            order:      the order of the polynomial (default is 0)

            plot:       plot the fit and the residual. In this each
                        indivual fit has to be approved, by typing 'y'
                        or 'n'

            uselin:     use linear polynomial fit

            insitu:     if False a new scantable is returned.
                        Otherwise, the scaling is done in-situ
                        The default is taken from .asaprc (False)

            rows:       row numbers of spectra to be processed.
                        (default is None: for all rows)
        
        Example:
            # return a scan baselined by a third order polynomial,
            # not using a mask
            bscan = scan.poly_baseline(order=3)

        """
        if insitu is None: insitu = rcParams['insitu']
        if not insitu:
            workscan = self.copy()
        else:
            workscan = self
        varlist = vars()
        if mask is None:
            mask = [True for i in xrange(self.nchan())]

        try:
            f = fitter()
            if uselin:
                f.set_function(lpoly=order)
            else:
                f.set_function(poly=order)

            if rows == None:
                rows = xrange(workscan.nrow())
            elif isinstance(rows, int):
                rows = [ rows ]
            
            if len(rows) > 0:
                self.blpars = []
                self.masklists = []
                self.actualmask = []
            
            for r in rows:
                f.x = workscan._getabcissa(r)
                f.y = workscan._getspectrum(r)
                f.mask = mask_and(mask, workscan._getmask(r))    # (CAS-1434)
                f.data = None
                f.fit()
                if plot:
                    f.plot(residual=True)
                    x = raw_input("Accept fit ( [y]/n ): ")
                    if x.upper() == 'N':
                        self.blpars.append(None)
                        self.masklists.append(None)
                        self.actualmask.append(None)
                        continue
                workscan._setspectrum(f.fitter.getresidual(), r)
                self.blpars.append(f.get_parameters())
                self.masklists.append(workscan.get_masklist(f.mask, row=r))
                self.actualmask.append(f.mask)

            if plot:
                f._p.unmap()
                f._p = None
            workscan._add_history("poly_baseline", varlist)
            if insitu:
                self._assign(workscan)
            else:
                return workscan
        except RuntimeError:
            msg = "The fit failed, possibly because it didn't converge."
            raise RuntimeError(msg)


    def poly_baseline(self, mask=None, order=0, plot=False, batch=False, insitu=None, rows=None):
        """\
        Return a scan which has been baselined (all rows) by a polynomial.
        Parameters:
            mask:       an optional mask
            order:      the order of the polynomial (default is 0)
            plot:       plot the fit and the residual. In this each
                        indivual fit has to be approved, by typing 'y'
                        or 'n'. Ignored if batch = True. 
            batch:      if True a faster algorithm is used and logs
                        including the fit results are not output
                        (default is False) 
            insitu:     if False a new scantable is returned.
                        Otherwise, the scaling is done in-situ
                        The default is taken from .asaprc (False)
            rows:       row numbers of spectra to be processed.
                        (default is None: for all rows)
        Example:
            # return a scan baselined by a third order polynomial,
            # not using a mask
            bscan = scan.poly_baseline(order=3)
        """
        if insitu is None: insitu = rcParams["insitu"]
        if insitu:
            workscan = self
        else:
            workscan = self.copy()

        varlist = vars()
        nchan = workscan.nchan()
        
        if mask is None:
            mask = [True for i in xrange(nchan)]

        try:
            if rows == None:
                rows = xrange(workscan.nrow())
            elif isinstance(rows, int):
                rows = [ rows ]
            
            if len(rows) > 0:
                self.blpars = []
                self.masklists = []
                self.actualmask = []

            if batch:
                for r in rows:
                    workscan._poly_baseline_batch(mask, order, r)
            elif plot:
                f = fitter()
                f.set_function(lpoly=order)
                for r in rows:
                    f.x = workscan._getabcissa(r)
                    f.y = workscan._getspectrum(r)
                    f.mask = mask_and(mask, workscan._getmask(r))    # (CAS-1434)
                    f.data = None
                    f.fit()
                    
                    f.plot(residual=True)
                    accept_fit = raw_input("Accept fit ( [y]/n ): ")
                    if accept_fit.upper() == "N":
                        self.blpars.append(None)
                        self.masklists.append(None)
                        self.actualmask.append(None)
                        continue
                    workscan._setspectrum(f.fitter.getresidual(), r)
                    self.blpars.append(f.get_parameters())
                    self.masklists.append(workscan.get_masklist(f.mask, row=r))
                    self.actualmask.append(f.mask)
                    
                f._p.unmap()
                f._p = None
            else:
                import array
                for r in rows:
                    pars = array.array("f", [0.0 for i in range(order+1)])
                    pars_adr = pars.buffer_info()[0]
                    pars_len = pars.buffer_info()[1]
                    
                    errs = array.array("f", [0.0 for i in range(order+1)])
                    errs_adr = errs.buffer_info()[0]
                    errs_len = errs.buffer_info()[1]
                    
                    fmsk = array.array("i", [1 for i in range(nchan)])
                    fmsk_adr = fmsk.buffer_info()[0]
                    fmsk_len = fmsk.buffer_info()[1]
                    
                    workscan._poly_baseline(mask, order, r, pars_adr, pars_len, errs_adr, errs_len, fmsk_adr, fmsk_len)
                    
                    params = pars.tolist()
                    fmtd = ""
                    for i in xrange(len(params)): fmtd += "  p%d= %3.6f," % (i, params[i])
                    fmtd = fmtd[:-1]  # remove trailing ","
                    errors = errs.tolist()
                    fmask = fmsk.tolist()
                    for i in xrange(len(fmask)): fmask[i] = (fmask[i] > 0)    # transform (1/0) -> (True/False)
                    
                    self.blpars.append({"params":params, "fixed":[], "formatted":fmtd, "errors":errors})
                    self.masklists.append(workscan.get_masklist(fmask, r))
                    self.actualmask.append(fmask)
                    
                    asaplog.push(str(fmtd))
            
            workscan._add_history("poly_baseline", varlist)
            
            if insitu:
                self._assign(workscan)
            else:
                return workscan
            
        except RuntimeError:
            msg = "The fit failed, possibly because it didn't converge."
            if rcParams["verbose"]:
                asaplog.push(str(msg))
                return
            else:
                raise RuntimeError(msg)


    def auto_poly_baseline(self, mask=None, edge=(0, 0), order=0,
                           threshold=3, chan_avg_limit=1, plot=False,
                           insitu=None, rows=None):
        """\
        Return a scan which has been baselined (all rows) by a polynomial.
        Spectral lines are detected first using linefinder and masked out
        to avoid them affecting the baseline solution.

        Parameters:

            mask:       an optional mask retreived from scantable

            edge:       an optional number of channel to drop at the edge of
                        spectrum. If only one value is
                        specified, the same number will be dropped from
                        both sides of the spectrum. Default is to keep
                        all channels. Nested tuples represent individual
                        edge selection for different IFs (a number of spectral
                        channels can be different)

            order:      the order of the polynomial (default is 0)

            threshold:  the threshold used by line finder. It is better to
                        keep it large as only strong lines affect the
                        baseline solution.

            chan_avg_limit:
                        a maximum number of consequtive spectral channels to
                        average during the search of weak and broad lines.
                        The default is no averaging (and no search for weak
                        lines). If such lines can affect the fitted baseline
                        (e.g. a high order polynomial is fitted), increase this
                        parameter (usually values up to 8 are reasonable). Most
                        users of this method should find the default value
                        sufficient.

            plot:       plot the fit and the residual. In this each
                        indivual fit has to be approved, by typing 'y'
                        or 'n'

            insitu:     if False a new scantable is returned.
                        Otherwise, the scaling is done in-situ
                        The default is taken from .asaprc (False)
            rows:       row numbers of spectra to be processed.
                        (default is None: for all rows)


        Example::

            scan2 = scan.auto_poly_baseline(order=7, insitu=False)

        """
        if insitu is None: insitu = rcParams['insitu']
        varlist = vars()
        from asap.asaplinefind import linefinder
        from asap import _is_sequence_or_number as _is_valid

        # check whether edge is set up for each IF individually
        individualedge = False;
        if len(edge) > 1:
            if isinstance(edge[0], list) or isinstance(edge[0], tuple):
                individualedge = True;

        if not _is_valid(edge, int) and not individualedge:
            raise ValueError, "Parameter 'edge' has to be an integer or a \
            pair of integers specified as a tuple. Nested tuples are allowed \
            to make individual selection for different IFs."

        curedge = (0, 0)
        if individualedge:
            for edgepar in edge:
                if not _is_valid(edgepar, int):
                    raise ValueError, "Each element of the 'edge' tuple has \
                                       to be a pair of integers or an integer."
        else:
            curedge = edge;

        if not insitu:
            workscan = self.copy()
        else:
            workscan = self

        # setup fitter
        f = fitter()
        f.set_function(lpoly=order)

        # setup line finder
        fl = linefinder()
        fl.set_options(threshold=threshold,avg_limit=chan_avg_limit)

        fl.set_scan(workscan)

        if mask is None:
            mask = _n_bools(workscan.nchan(), True)
        
        if rows is None:
            rows = xrange(workscan.nrow())
        elif isinstance(rows, int):
            rows = [ rows ]
        
        # Save parameters of baseline fits & masklists as a class attribute.
        # NOTICE: It does not reflect changes in scantable!
        if len(rows) > 0:
            self.blpars=[]
            self.masklists=[]
            self.actualmask=[]
        asaplog.push("Processing:")
        for r in rows:
            msg = " Scan[%d] Beam[%d] IF[%d] Pol[%d] Cycle[%d]" % \
                (workscan.getscan(r), workscan.getbeam(r), workscan.getif(r), \
                 workscan.getpol(r), workscan.getcycle(r))
            asaplog.push(msg, False)

            # figure out edge parameter
            if individualedge:
                if len(edge) >= workscan.getif(r):
                    raise RuntimeError, "Number of edge elements appear to " \
                                        "be less than the number of IFs"
                    curedge = edge[workscan.getif(r)]

            actualmask = mask_and(mask, workscan._getmask(r))    # (CAS-1434)

            # setup line finder
            fl.find_lines(r, actualmask, curedge)
            
            f.x = workscan._getabcissa(r)
            f.y = workscan._getspectrum(r)
            f.mask = fl.get_mask()
            f.data = None
            f.fit()

            # Show mask list
            masklist=workscan.get_masklist(f.mask, row=r)
            msg = "mask range: "+str(masklist)
            asaplog.push(msg, False)

            if plot:
                f.plot(residual=True)
                x = raw_input("Accept fit ( [y]/n ): ")
                if x.upper() == 'N':
                    self.blpars.append(None)
                    self.masklists.append(None)
                    self.actualmask.append(None)
                    continue

            workscan._setspectrum(f.fitter.getresidual(), r)
            self.blpars.append(f.get_parameters())
            self.masklists.append(masklist)
            self.actualmask.append(f.mask)
        if plot:
            f._p.unmap()
            f._p = None
        workscan._add_history("auto_poly_baseline", varlist)
        if insitu:
            self._assign(workscan)
        else:
            return workscan

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

        Return a scan where all spectra are scaled by the give 'factor'

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
            if isinstance(factor[0], list) or isinstance(factor[0], numpy.ndarray):
                from asapmath import _array2dOp
                s = _array2dOp( self.copy(), factor, "MUL", tsys )
            else:
                s = scantable( self._math._arrayop( self.copy(), factor, "MUL", tsys ) )
        else:
            s = scantable(self._math._unaryop(self.copy(), factor, "MUL", tsys))
        s._add_history("scale", varlist)
        if insitu:
            self._assign(s)
        else:
            return s

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
        basesel = self.get_selection()
        stype = -1
        if sourcetype.lower().startswith("r"):
            stype = 1
        elif sourcetype.lower().startswith("s"):
            stype = 0
        else:
            raise ValueError("Illegal sourcetype use s(ource) or r(eference)")
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
            sel.set_query("SRCNAME == %s('%s')" % (matchtype, match))
        self.set_selection(basesel+sel)
        self._setsourcetype(stype)
        self.set_selection(basesel)
        self._add_history("set_sourcetype", varlist)

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
            sel = self.get_selection()
            sel.set_query("SRCTYPE==psoff")
            self.set_selection(sel)
            offs = self.copy()
            sel.set_query("SRCTYPE==pson")
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
        varlist = vars()
        s = None
        if isinstance(other, scantable):
            s = scantable(self._math._binaryop(self, other, "ADD"))
        elif isinstance(other, float):
            s = scantable(self._math._unaryop(self, other, "ADD", False))
        else:
            raise TypeError("Other input is not a scantable or float value")
        s._add_history("operator +", varlist)
        return s

    @asaplog_post_dec
    def __sub__(self, other):
        """
        implicit on all axes and on Tsys
        """
        varlist = vars()
        s = None
        if isinstance(other, scantable):
            s = scantable(self._math._binaryop(self, other, "SUB"))
        elif isinstance(other, float):
            s = scantable(self._math._unaryop(self, other, "SUB", False))
        else:
            raise TypeError("Other input is not a scantable or float value")
        s._add_history("operator -", varlist)
        return s

    @asaplog_post_dec
    def __mul__(self, other):
        """
        implicit on all axes and on Tsys
        """
        varlist = vars()
        s = None
        if isinstance(other, scantable):
            s = scantable(self._math._binaryop(self, other, "MUL"))
        elif isinstance(other, float):
            s = scantable(self._math._unaryop(self, other, "MUL", False))
        else:
            raise TypeError("Other input is not a scantable or float value")
        s._add_history("operator *", varlist)
        return s


    @asaplog_post_dec
    def __div__(self, other):
        """
        implicit on all axes and on Tsys
        """
        varlist = vars()
        s = None
        if isinstance(other, scantable):
            s = scantable(self._math._binaryop(self, other, "DIV"))
        elif isinstance(other, float):
            if other == 0.0:
                raise ZeroDivisionError("Dividing by zero is not recommended")
            s = scantable(self._math._unaryop(self, other, "DIV", False))
        else:
            raise TypeError("Other input is not a scantable or float value")
        s._add_history("operator /", varlist)
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
        return selector(beams=self.getbeam(rowno),
                        ifs=self.getif(rowno),
                        pols=self.getpol(rowno),
                        scans=self.getscan(rowno),
                        cycles=self.getcycle(rowno))

    def _add_history(self, funcname, parameters):
        if not rcParams['scantable.history']:
            return
        # create date
        sep = "##"
        from datetime import datetime
        dstr = datetime.now().strftime('%Y/%m/%d %H:%M:%S')
        hist = dstr+sep
        hist += funcname+sep#cdate+sep
        if parameters.has_key('self'): del parameters['self']
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
        nchans = [self.nchan(i) for i in range(self.nif(-1))]
        nchans = filter(lambda t: t > 0, nchans)
        return (sum(nchans)/len(nchans) == nchans[0])

    @asaplog_post_dec
    #def _fill(self, names, unit, average, getpt, antenna):
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
            r = filler(tbl)
            rx = rcParams['scantable.reference']
            r.setreferenceexpr(rx)
            msg = "Importing %s..." % (name)
            asaplog.push(msg, False)
            #opts = {'ms': {'antenna' : antenna, 'getpt': getpt} }
            r.open(name, opts)# antenna, -1, -1, getpt)
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
