from asap._asap import Scantable
from asap import rcParams
from asap import print_log
from asap import asaplog
from asap import selector
from asap import linecatalog
from asap import _n_bools, mask_not, mask_and, mask_or

class scantable(Scantable):
    """
        The ASAP container for scans
    """

    def __init__(self, filename, average=None, unit=None, getpt=None):
        """
        Create a scantable from a saved one or make a reference
        Parameters:
            filename:    the name of an asap table on disk
                         or
                         the name of a rpfits/sdfits/ms file
                         (integrations within scans are auto averaged
                         and the whole file is read)
                         or
                         [advanced] a reference to an existing
                         scantable
            average:     average all integrations withinb a scan on read.
                         The default (True) is taken from .asaprc.
            unit:         brightness unit; must be consistent with K or Jy.
                         Over-rides the default selected by the reader
                         (input rpfits/sdfits/ms) or replaces the value
                         in existing scantables
        """
        if average is None:
            average = rcParams['scantable.autoaverage']
        if getpt is None:
            getpt = False
        varlist = vars()
        from asap._asap import stmath
        self._math = stmath()
        if isinstance(filename, Scantable):
            Scantable.__init__(self, filename)
        else:
            if isinstance(filename, str):
                import os.path
                filename = os.path.expandvars(filename)
                filename = os.path.expanduser(filename)
                if not os.path.exists(filename):
                    s = "File '%s' not found." % (filename)
                    if rcParams['verbose']:
                        asaplog.push(s)
                        print asaplog.pop().strip()
                        return
                    raise IOError(s)
                if os.path.isdir(filename) \
                    and not os.path.exists(filename+'/table.f1'):
                    # crude check if asap table
                    if os.path.exists(filename+'/table.info'):
                        ondisk = rcParams['scantable.storage'] == 'disk'
                        Scantable.__init__(self, filename, ondisk)
                        if unit is not None:
                            self.set_fluxunit(unit)
                        # do not reset to the default freqframe
                        #self.set_freqframe(rcParams['scantable.freqframe'])
                    else:
                        msg = "The given file '%s'is not a valid " \
                              "asap table." % (filename)
                        if rcParams['verbose']:
                            print msg
                            return
                        else:
                            raise IOError(msg)
                else:
                    self._fill([filename], unit, average, getpt)
            elif (isinstance(filename, list) or isinstance(filename, tuple)) \
                  and isinstance(filename[-1], str):
                self._fill(filename, unit, average, getpt)
        self._add_history("scantable", varlist)
        print_log()

    def save(self, name=None, format=None, overwrite=False):
        """
        Store the scantable on disk. This can be an asap (aips++) Table,
        SDFITS or MS2 format.
        Parameters:
            name:        the name of the outputfile. For format "ASCII"
                         this is the root file name (data in 'name'.txt
                         and header in 'name'_header.txt)
            format:      an optional file format. Default is ASAP.
                         Allowed are - 'ASAP' (save as ASAP [aips++] Table),
                                       'SDFITS' (save as SDFITS file)
                                       'ASCII' (saves as ascii text file)
                                       'MS2' (saves as an aips++
                                              MeasurementSet V2)
            overwrite:   If the file should be overwritten if it exists.
                         The default False is to return with warning
                         without writing the output. USE WITH CARE.
        Example:
            scan.save('myscan.asap')
            scan.save('myscan.sdfits', 'SDFITS')
        """
        from os import path
        if format is None: format = rcParams['scantable.save']
        suffix = '.'+format.lower()
        if name is None or name == "":
            name = 'scantable'+suffix
            msg = "No filename given. Using default name %s..." % name
            asaplog.push(msg)
        name = path.expandvars(name)
        if path.isfile(name) or path.isdir(name):
            if not overwrite:
                msg = "File %s exists." % name
                if rcParams['verbose']:
                    print msg
                    return
                else:
                    raise IOError(msg)
        format2 = format.upper()
        if format2 == 'ASAP':
            self._save(name)
        else:
            from asap._asap import stwriter as stw
            writer = stw(format2)
            writer.write(self, name)
        print_log()
        return

    def copy(self):
        """
        Return a copy of this scantable.
        Note:
            This makes a full (deep) copy. scan2 = scan1 makes a reference.
        Parameters:
            none
        Example:
            copiedscan = scan.copy()
        """
        sd = scantable(Scantable._copy(self))
        return sd

    def drop_scan(self, scanid=None):
        """
        Return a new scantable where the specified scan number(s) has(have)
        been dropped.
        Parameters:
            scanid:    a (list of) scan number(s)
        """
        from asap import _is_sequence_or_number as _is_valid
        from asap import _to_list
        from asap import unique
        if not _is_valid(scanid):
            if rcParams['verbose']:
                print "Please specify a scanno to drop from the scantable"
                return
            else:
                raise RuntimeError("No scan given")
        try:
            scanid = _to_list(scanid)
            allscans = unique([ self.getscan(i) for i in range(self.nrow())])
            for sid in scanid: allscans.remove(sid)
            if len(allscans) == 0:
                raise ValueError("Can't remove all scans")
        except ValueError:
            if rcParams['verbose']:
                print "Couldn't find any match."
                return
            else: raise
        try:
            bsel = self.get_selection()
            sel = selector()
            sel.set_scans(allscans)
            self.set_selection(bsel+sel)
            scopy = self._copy()
            self.set_selection(bsel)
            return scantable(scopy)
        except RuntimeError:
            if rcParams['verbose']:
                print "Couldn't find any match."
            else:
                raise


    def get_scan(self, scanid=None):
        """
        Return a specific scan (by scanno) or collection of scans (by
        source name) in a new scantable.
        Note:
            See scantable.drop_scan() for the inverse operation.
        Parameters:
            scanid:    a (list of) scanno or a source name, unix-style
                       patterns are accepted for source name matching, e.g.
                       '*_R' gets all 'ref scans
        Example:
            # get all scans containing the source '323p459'
            newscan = scan.get_scan('323p459')
            # get all 'off' scans
            refscans = scan.get_scan('*_R')
            # get a susbset of scans by scanno (as listed in scan.summary())
            newscan = scan.get_scan([0, 2, 7, 10])
        """
        if scanid is None:
            if rcParams['verbose']:
                print "Please specify a scan no or name to " \
                      "retrieve from the scantable"
                return
            else:
                raise RuntimeError("No scan given")

        try:
            bsel = self.get_selection()
            sel = selector()
            if type(scanid) is str:
                sel.set_name(scanid)
                self.set_selection(bsel+sel)
                scopy = self._copy()
                self.set_selection(bsel)
                return scantable(scopy)
            elif type(scanid) is int:
                sel.set_scans([scanid])
                self.set_selection(bsel+sel)
                scopy = self._copy()
                self.set_selection(bsel)
                return scantable(scopy)
            elif type(scanid) is list:
                sel.set_scans(scanid)
                self.set_selection(sel)
                scopy = self._copy()
                self.set_selection(bsel)
                return scantable(scopy)
            else:
                msg = "Illegal scanid type, use 'int' or 'list' if ints."
                if rcParams['verbose']:
                    print msg
                else:
                    raise TypeError(msg)
        except RuntimeError:
            if rcParams['verbose']: print "Couldn't find any match."
            else: raise

    def __str__(self):
        return Scantable._summary(self, True)

    def summary(self, filename=None):
        """
        Print a summary of the contents of this scantable.
        Parameters:
            filename:    the name of a file to write the putput to
                         Default - no file output
            verbose:     print extra info such as the frequency table
                         The default (False) is taken from .asaprc
        """
        info = Scantable._summary(self, True)
        #if verbose is None: verbose = rcParams['scantable.verbosesummary']
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
                if rcParams['verbose']:
                    print msg
                else:
                    raise IOError(msg)
        if rcParams['verbose']:
            try:
                from IPython.genutils import page as pager
            except ImportError:
                from pydoc import pager
            pager(info)
        else:
            return info


    def get_selection(self):
        """
        Get the selection object currently set on this scantable.
        Parameters:
            none
        Example:
            sel = scan.get_selection()
            sel.set_ifs(0)              # select IF 0
            scan.set_selection(sel)     # apply modified selection
        """
        return selector(self._getselection())

    def set_selection(self, selection=selector()):
        """
        Select a subset of the data. All following operations on this scantable
        are only applied to thi selection.
        Parameters:
            selection:    a selector object (default unset the selection)
        Examples:
            sel = selector()         # create a selection object
            self.set_scans([0, 3])    # select SCANNO 0 and 3
            scan.set_selection(sel)  # set the selection
            scan.summary()           # will only print summary of scanno 0 an 3
            scan.set_selection()     # unset the selection
        """
        self._setselection(selection)

    def get_row(self, row=0, insitu=None):
        """
        Select a row in the scantable.
        Return a scantable with single row.
        Parameters:
            row: row no of integration, default is 0.
            insitu: if False a new scantable is returned.
                    Otherwise, the scaling is done in-situ
                    The default is taken from .asaprc (False)
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

    def stats(self, stat='stddev', mask=None):
        """
        Determine the specified statistic of the current beam/if/pol
        Takes a 'mask' as an optional parameter to specify which
        channels should be excluded.
        You can get min/max values with their
        channels/frequencies/velocities by selecting stat='min_abc'
        or 'max_abc'.
        Parameters:
            stat:    'min', 'max', 'min_abc', 'max_abc', 'sumsq', 'sum',
                     'mean', 'var', 'stddev', 'avdev', 'rms', 'median'
            mask:    an optional mask specifying where the statistic
                     should be determined.
        Example:
            scan.set_unit('channel')
            msk = scan.create_mask([100, 200], [500, 600])
            scan.stats(stat='mean', mask=m)
        """
        if mask == None:
            mask = []
        axes = ['Beam', 'IF', 'Pol', 'Time']
        if not self._check_ifs():
            raise ValueError("Cannot apply mask as the IFs have different "
                             "number of channels. Please use setselection() "
                             "to select individual IFs")

        if stat.lower().find('_abc') == -1:
            statvals = self._math._stats(self, mask, stat)
            getchan = False
            sstat = str(stat)
        else:
            chan = self._math._minmaxchan(self, mask, stat)
            getchan = True
            statvals = []
            sstat = stat.lower().strip('_abc')
        out = ''
        axes = []
        for i in range(self.nrow()):
            axis = []
            axis.append(self.getscan(i))
            axis.append(self.getbeam(i))
            axis.append(self.getif(i))
            axis.append(self.getpol(i))
            axis.append(self.getcycle(i))
            axes.append(axis)
            tm = self._gettime(i)
            src = self._getsourcename(i)
            xpos = ''
            if getchan:
                qx, qy = self.chan2data(rowno=i, chan=chan[i])
                statvals.append(qy['value'])
                xpos = '(x = %3.3f' % (qx['value'])+' ['+qx['unit']+'])'
            out += 'Scan[%d] (%s) ' % (axis[0], src)
            out += 'Time[%s]:\n' % (tm)
            if self.nbeam(-1) > 1: out +=  ' Beam[%d] ' % (axis[1])
            if self.nif(-1) > 1: out +=  ' IF[%d] ' % (axis[2])
            if self.npol(-1) > 1: out +=  ' Pol[%d] ' % (axis[3])
            out += '= %3.3f   ' % (statvals[i]) +xpos+'\n' 
            out +=  "--------------------------------------------------\n"

        if rcParams['verbose']:
            print "--------------------------------------------------"
            print " ", sstat
            print "--------------------------------------------------"
            print out
        #else:
            #retval = { 'axesnames': ['scanno', 'beamno', 'ifno', 'polno', 'cycleno'],
            #           'axes' : axes,
            #           'data': statvals}
        return statvals

    def chan2data(self, rowno=0, chan=0):
        """
        Returns channel/frequency/velocity and spectral value
        at an arbitrary row and channel in the scantable.
        Parameters:
            rowno:   a row number in the scantable. Default is the
                     first row, i.e. rowno=0
            chan:    a channel in the scantable. Default is the first
                     channel, i.e. pos=0
        """
        if isinstance(rowno, int) and isinstance(chan, int):
            x, xlbl = self.get_abcissa(rowno)
            qx = {'unit': xlbl, 'value': x[chan]}
            qy = {'unit': self.get_fluxunit(),
                  'value': self._getspectrum(rowno)[chan]}
            return qx, qy

    def stddev(self, mask=None):
        """
        Determine the standard deviation of the current beam/if/pol
        Takes a 'mask' as an optional parameter to specify which
        channels should be excluded.
        Parameters:
            mask:    an optional mask specifying where the standard
                     deviation should be determined.

        Example:
            scan.set_unit('channel')
            msk = scan.create_mask([100, 200], [500, 600])
            scan.stddev(mask=m)
        """
        return self.stats(stat='stddev', mask=mask);


    def get_column_names(self):
        """
        Return a  list of column names, which can be used for selection.
        """
        return list(Scantable.get_column_names(self))

    def get_tsys(self):
        """
        Return the System temperatures.
        Returns:
            a list of Tsys values for the current selection
        """

        return self._row_callback(self._gettsys, "Tsys")

    def _row_callback(self, callback, label):
        axes = []
        axesnames = ['scanno', 'beamno', 'ifno', 'polno', 'cycleno']
        out = ""
        outvec = []
        for i in range(self.nrow()):
            axis = []
            axis.append(self.getscan(i))
            axis.append(self.getbeam(i))
            axis.append(self.getif(i))
            axis.append(self.getpol(i))
            axis.append(self.getcycle(i))
            axes.append(axis)
            tm = self._gettime(i)
            src = self._getsourcename(i)
            out += 'Scan[%d] (%s) ' % (axis[0], src)
            out += 'Time[%s]:\n' % (tm)
            if self.nbeam(-1) > 1: out +=  ' Beam[%d] ' % (axis[1])
            if self.nif(-1) > 1: out +=  ' IF[%d] ' % (axis[2])
            if self.npol(-1) > 1: out +=  ' Pol[%d] ' % (axis[3])
            outvec.append(callback(i))
            out += '= %3.3f\n' % (outvec[i])
            out +=  "--------------------------------------------------\n"
        if rcParams['verbose']:
            print "--------------------------------------------------"
            print " %s" % (label)
            print "--------------------------------------------------"
            print out
        # disabled because the vector seems more useful
        #retval = {'axesnames': axesnames, 'axes': axes, 'data': outvec}
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
        """
        Get a list of time stamps for the observations.
        Return a datetime object for each integration time stamp in the scantable.
        Parameters:
            row:          row no of integration. Default -1 return all rows
            asdatetime:   return values as datetime objects rather than strings
        Example:
            none
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
        """
        Get a list of integration times for the observations.
        Return a time in seconds for each integration in the scantable.
        Parameters:
            row:    row no of integration. Default -1 return all rows.
        Example:
            none
        """
        return self._get_column(self._getinttime, row)        
        

    def get_sourcename(self, row=-1):
        """
        Get a list source names for the observations.
        Return a string for each integration in the scantable.
        Parameters:
            row:    row no of integration. Default -1 return all rows.
        Example:
            none
        """
        return self._get_column(self._getsourcename, row)

    def get_elevation(self, row=-1):
        """
        Get a list of elevations for the observations.
        Return a float for each integration in the scantable.
        Parameters:
            row:    row no of integration. Default -1 return all rows.
        Example:
            none
        """
        return self._get_column(self._getelevation, row)

    def get_azimuth(self, row=-1):
        """
        Get a list of azimuths for the observations.
        Return a float for each integration in the scantable.
        Parameters:
            row:    row no of integration. Default -1 return all rows.
        Example:
            none
        """
        return self._get_column(self._getazimuth, row)

    def get_parangle(self, row=-1):
        """
        Get a list of parallactic angles for the observations.
        Return a float for each integration in the scantable.
        Parameters:
            row:    row no of integration. Default -1 return all rows.
        Example:
            none
        """
        return self._get_column(self._getparangle, row)

    def get_direction(self, row=-1):
        """
        Get a list of Positions on the sky (direction) for the observations.
        Return a float for each integration in the scantable.
        Parameters:
            row:    row no of integration. Default -1 return all rows
        Example:
            none
        """
        return self._get_column(self._getdirection, row)

    def get_directionval(self, row=-1):
        """
        Get a list of Positions on the sky (direction) for the observations.
        Return a float for each integration in the scantable.
        Parameters:
            row:    row no of integration. Default -1 return all rows
        Example:
            none
        """
        return self._get_column(self._getdirectionvec, row)

    def set_unit(self, unit='channel'):
        """
        Set the unit for all following operations on this scantable
        Parameters:
            unit:    optional unit, default is 'channel'
                     one of '*Hz', 'km/s', 'channel', ''
        """
        varlist = vars()
        if unit in ['', 'pixel', 'channel']:
            unit = ''
        inf = list(self._getcoordinfo())
        inf[0] = unit
        self._setcoordinfo(inf)
        self._add_history("set_unit", varlist)

    def set_instrument(self, instr):
        """
        Set the instrument for subsequent processing.
        Parameters:
            instr:    Select from 'ATPKSMB', 'ATPKSHOH', 'ATMOPRA',
                      'DSS-43' (Tid), 'CEDUNA', and 'HOBART'
        """
        self._setInstrument(instr)
        self._add_history("set_instument", vars())
        print_log()

    def set_feedtype(self, feedtype):
        """
        Overwrite the feed type, which might not be set correctly.
        Parameters:
            feedtype:     'linear' or 'circular'
        """
        self._setfeedtype(feedtype)
        self._add_history("set_feedtype", vars())
        print_log()

    def set_doppler(self, doppler='RADIO'):
        """
        Set the doppler for all following operations on this scantable.
        Parameters:
            doppler:    One of 'RADIO', 'OPTICAL', 'Z', 'BETA', 'GAMMA'
        """
        varlist = vars()
        inf = list(self._getcoordinfo())
        inf[2] = doppler
        self._setcoordinfo(inf)
        self._add_history("set_doppler", vars())
        print_log()

    def set_freqframe(self, frame=None):
        """
        Set the frame type of the Spectral Axis.
        Parameters:
            frame:   an optional frame type, default 'LSRK'. Valid frames are:
                     'REST', 'TOPO', 'LSRD', 'LSRK', 'BARY',
                     'GEO', 'GALACTO', 'LGROUP', 'CMB'
        Examples:
            scan.set_freqframe('BARY')
        """
        if frame is None: frame = rcParams['scantable.freqframe']
        varlist = vars()
        valid = ['REST', 'TOPO', 'LSRD', 'LSRK', 'BARY', \
                   'GEO', 'GALACTO', 'LGROUP', 'CMB']

        if frame in valid:
            inf = list(self._getcoordinfo())
            inf[1] = frame
            self._setcoordinfo(inf)
            self._add_history("set_freqframe", varlist)
        else:
            msg  = "Please specify a valid freq type. Valid types are:\n", valid
            if rcParams['verbose']:
                print msg
            else:
                raise TypeError(msg)
        print_log()

    def set_dirframe(self, frame=""):
        """
        Set the frame type of the Direction on the sky.
        Parameters:
            frame:   an optional frame type, default ''. Valid frames are:
                     'J2000', 'B1950', 'GALACTIC'
        Examples:
            scan.set_dirframe('GALACTIC')
        """
        varlist = vars()
        try:
            Scantable.set_dirframe(self, frame)
        except RuntimeError, msg:
            if rcParams['verbose']:
                print msg
            else:
                raise
        self._add_history("set_dirframe", varlist)

    def get_unit(self):
        """
        Get the default unit set in this scantable
        Returns:
            A unit string
        """
        inf = self._getcoordinfo()
        unit = inf[0]
        if unit == '': unit = 'channel'
        return unit

    def get_abcissa(self, rowno=0):
        """
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
        print_log()
        return abc, lbl

    def flag(self, mask=None, unflag=False):
        """
        Flag the selected data using an optional channel mask.
        Parameters:
            mask:   an optional channel mask, created with create_mask. Default
                    (no mask) is all channels.
            unflag:    if True, unflag the data
        """
        varlist = vars()
        if mask is None:
            mask = []
        try:
            self._flag(mask, unflag)
        except RuntimeError, msg:
            if rcParams['verbose']:
                print msg
                return
            else: raise
        self._add_history("flag", varlist)

    def lag_flag(self, frequency, width=0.0, unit="GHz", insitu=None):
        """
        Flag the data in 'lag' space by providing a frequency to remove.
        Flagged data in the scantable gets set to 0.0 before the fft.
        No taper is applied.
        Parameters:
            frequency:    the frequency (really a period within the bandwidth) 
                          to remove
            width:        the width of the frequency to remove, to remove a 
                          range of frequencies aroung the centre.
            unit:         the frequency unit (default "GHz")
        Notes:
            It is recommended to flag edges of the band or strong 
            signals beforehand.
        """
        if insitu is None: insitu = rcParams['insitu']
        self._math._setinsitu(insitu)
        varlist = vars()
        base = { "GHz": 1000000000., "MHz": 1000000., "kHz": 1000., "Hz": 1. }
        if not base.has_key(unit):
            raise ValueError("%s is not a valid unit." % unit)
        try:
            s = scantable(self._math._lag_flag(self, frequency*base[unit],
                                               width*base[unit]))
        except RuntimeError, msg:
            if rcParams['verbose']:
                print msg
                return
            else: raise
        s._add_history("lag_flag", varlist)
        print_log()
        if insitu:
            self._assign(s)
        else:
            return s


    def create_mask(self, *args, **kwargs):
        """
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
        Example:
            scan.set_unit('channel')
            a)
            msk = scan.create_mask([400, 500], [800, 900])
            # masks everything outside 400 and 500
            # and 800 and 900 in the unit 'channel'

            b)
            msk = scan.create_mask([400, 500], [800, 900], invert=True)
            # masks the regions between 400 and 500
            # and 800 and 900 in the unit 'channel'
            c)
            mask only channel 400
            msk =  scan.create_mask([400, 400])
        """
        row = 0
        if kwargs.has_key("row"):
            row = kwargs.get("row")
        data = self._getabcissa(row)
        u = self._getcoordinfo()[0]
        if rcParams['verbose']:
            if u == "": u = "channel"
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
            if (len(window) != 2 or window[0] > window[1] ):
                raise TypeError("A window needs to be defined as [min, max]")
            for i in range(n):
                if data[i] >= window[0] and data[i] <= window[1]:
                    msk[i] = True
        if kwargs.has_key('invert'):
            if kwargs.get('invert'):
                msk = mask_not(msk)
        print_log()
        return msk

    def get_masklist(self, mask=None, row=0):
        """
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
        if rcParams['verbose']:
            if u == "": u = "channel"
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
        """
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
        """
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
        """
        ********NEED TO BE UPDATED begin************
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
        ********NEED TO BE UPDATED end************
        You can also specify the frequencies via a linecatalog/

        Parameters:
            freqs:   list of rest frequency values or string idenitfiers
            unit:    unit for rest frequency (default 'Hz')

        Example:
            # set the given restfrequency for the all currently selected IFs
            scan.set_restfreqs(freqs=1.4e9)
            # set multiple restfrequencies to all the selected data
            scan.set_restfreqs(freqs=[1.4e9, 1.41e9, 1.42e9])
            # If the number of IFs in the data is >= 2 the IF0 gets the first
            # value IF1 the second... NOTE that freqs needs to be
            # specified in list of list (e.g. [[],[],...] ).
            scan.set_restfreqs(freqs=[[1.4e9],[1.67e9]])
            #set the given restfrequency for the whole table (by name)
            scan.set_restfreqs(freqs="OH1667")

        Note:
            To do more sophisticate Restfrequency setting, e.g. on a
            source and IF basis, use scantable.set_selection() before using
            this function.
            # provide your scantable is call scan
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
            # TT mod
            #self._setrestfreqs(freqs, "",unit)
            self._setrestfreqs([freqs], [""],unit)
        # list of values
        elif isinstance(freqs, list) or isinstance(freqs, tuple):
            # list values are scalars
            if isinstance(freqs[-1], int) or isinstance(freqs[-1], float):
                self._setrestfreqs(freqs, [""],unit)
            # list values are tuples, (value, name)
            elif isinstance(freqs[-1], dict):
                #sel = selector()
                #savesel = self._getselection()
                #iflist = self.getifnos()
                #for i in xrange(len(freqs)):
                #    sel.set_ifs(iflist[i])
                #    self._setselection(sel)
                #    self._setrestfreqs(freqs[i], "",unit)
                #self._setselection(savesel)
                self._setrestfreqs(freqs["value"],
                                   freqs["name"], "MHz")
            elif isinstance(freqs[-1], list) or isinstance(freqs[-1], tuple):
                sel = selector()
                savesel = self._getselection()
                iflist = self.getifnos()
                if len(freqs)>len(iflist):
                    raise ValueError("number of elements in list of list exeeds the current IF selections")
                for i in xrange(len(freqs)):
                    sel.set_ifs(iflist[i])
                    self._setselection(sel)
                    self._setrestfreqs(freqs[i]["value"],
                                       freqs[i]["name"], "MHz")
                self._setselection(savesel)
        # freqs are to be taken from a linecatalog
        elif isinstance(freqs, linecatalog):
            sel = selector()
            savesel = self._getselection()
            for i in xrange(freqs.nrow()):
                sel.set_ifs(iflist[i])
                self._setselection(sel)
                self._setrestfreqs(freqs.get_frequency(i),
                                   freqs.get_name(i), "MHz")
                # ensure that we are not iterating past nIF
                if i == self.nif()-1: break
            self._setselection(savesel)
        else:
            return
        self._add_history("set_restfreqs", varlist)

    def shift_refpix(self, delta):
	"""
	Shift the reference pixel of the Spectra Coordinate by an 
	integer amount.
	Parameters:
	    delta:   the amount to shift by
        Note:
	    Be careful using this with broadband data.
        """
	Scantable.shift(self, delta)

    def history(self, filename=None):
        """
        Print the history. Optionally to a file.
        Parameters:
            filename:    The name  of the file to save the history to.
        """
        hist = list(self._gethistory())
        out = "-"*80
        for h in hist:
            if h.startswith("---"):
                out += "\n"+h
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
                out += "\n"+"-"*80
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
                if rcParams['verbose']:
                    print msg
                else:
                    raise IOError(msg)
        if rcParams['verbose']:
            try:
                from IPython.genutils import page as pager
            except ImportError:
                from pydoc import pager
            pager(out)
        else:
            return out
        return
    #
    # Maths business
    #

    def average_time(self, mask=None, scanav=False, weight='tint', align=False):
        """
        Return the (time) weighted average of a scan.
        Note:
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
        Example:
            # time average the scantable without using a mask
            newscan = scan.average_time()
        """
        varlist = vars()
        if weight is None: weight = 'TINT'
        if mask is None: mask = ()
        if scanav: scanav = "SCAN"
        else: scanav = "NONE"
        scan = (self, )
        try:
            if align:
                scan = (self.freq_align(insitu=False), )
            s = None
            if weight.upper() == 'MEDIAN':
                s = scantable(self._math._averagechannel(scan[0], 'MEDIAN',
                                                         scanav))
            else:
                s = scantable(self._math._average(scan, mask, weight.upper(),
                              scanav))
        except RuntimeError, msg:
            if rcParams['verbose']:
                print msg
                return
            else: raise
        s._add_history("average_time", varlist)
        print_log()
        return s

    def convert_flux(self, jyperk=None, eta=None, d=None, insitu=None):
        """
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
        if jyperk is None: jyperk = -1.0
        if d is None: d = -1.0
        if eta is None: eta = -1.0
        s = scantable(self._math._convertflux(self, d, eta, jyperk))
        s._add_history("convert_flux", varlist)
        print_log()
        if insitu: self._assign(s)
        else: return s

    def gain_el(self, poly=None, filename="", method="linear", insitu=None):
        """
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
        if poly is None:
            poly = ()
        from os.path import expandvars
        filename = expandvars(filename)
        s = scantable(self._math._gainel(self, poly, filename, method))
        s._add_history("gain_el", varlist)
        print_log()
        if insitu: self._assign(s)
        else: return s

    def freq_align(self, reftime=None, method='cubic', insitu=None):
        """
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
        if reftime is None: reftime = ""
        s = scantable(self._math._freq_align(self, reftime, method))
        s._add_history("freq_align", varlist)
        print_log()
        if insitu: self._assign(s)
        else: return s

    def opacity(self, tau, insitu=None):
        """
        Apply an opacity correction. The data
        and Tsys are multiplied by the correction factor.
        Parameters:
            tau:         Opacity from which the correction factor is
                         exp(tau*ZD)
                         where ZD is the zenith-distance
            insitu:      if False a new scantable is returned.
                         Otherwise, the scaling is done in-situ
                         The default is taken from .asaprc (False)
        """
        if insitu is None: insitu = rcParams['insitu']
        self._math._setinsitu(insitu)
        varlist = vars()
        s = scantable(self._math._opacity(self, tau))
        s._add_history("opacity", varlist)
        print_log()
        if insitu: self._assign(s)
        else: return s

    def bin(self, width=5, insitu=None):
        """
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
        print_log()
        if insitu: self._assign(s)
        else: return s


    def resample(self, width=5, method='cubic', insitu=None):
        """
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
        print_log()
        if insitu: self._assign(s)
        else: return s


    def average_pol(self, mask=None, weight='none'):
        """
        Average the Polarisations together.
        Parameters:
            mask:        An optional mask defining the region, where the
                         averaging will be applied. The output will have all
                         specified points masked.
            weight:      Weighting scheme. 'none' (default), 'var' (1/var(spec)
                         weighted), or 'tsys' (1/Tsys**2 weighted)
        """
        varlist = vars()
        if mask is None:
            mask = ()
        s = scantable(self._math._averagepol(self, mask, weight.upper()))
        s._add_history("average_pol", varlist)
        print_log()
        return s

    def average_beam(self, mask=None, weight='none'):
        """
        Average the Beams together.
        Parameters:
            mask:        An optional mask defining the region, where the
                         averaging will be applied. The output will have all
                         specified points masked.
            weight:      Weighting scheme. 'none' (default), 'var' (1/var(spec)
                         weighted), or 'tsys' (1/Tsys**2 weighted)
        """
        varlist = vars()
        if mask is None:
            mask = ()
        s = scantable(self._math._averagebeams(self, mask, weight.upper()))
        s._add_history("average_beam", varlist)
        print_log()
        return s

    def convert_pol(self, poltype=None):
        """
        Convert the data to a different polarisation type.
        Parameters:
            poltype:    The new polarisation type. Valid types are:
                        "linear", "stokes" and "circular"
        """
        varlist = vars()
        try:
            s = scantable(self._math._convertpol(self, poltype))
        except RuntimeError, msg:
            if rcParams['verbose']:
                print msg
                return
            else:
                raise
        s._add_history("convert_pol", varlist)
        print_log()
        return s

    def smooth(self, kernel="hanning", width=5.0, insitu=None):
        """
        Smooth the spectrum by the specified kernel (conserving flux).
        Parameters:
            kernel:     The type of smoothing kernel. Select from
                        'hanning' (default), 'gaussian', 'boxcar' and
                        'rmedian'
            width:      The width of the kernel in pixels. For hanning this is
                        ignored otherwise it defauls to 5 pixels.
                        For 'gaussian' it is the Full Width Half
                        Maximum. For 'boxcar' it is the full width.
                        For 'rmedian' it is the half width.
            insitu:     if False a new scantable is returned.
                        Otherwise, the scaling is done in-situ
                        The default is taken from .asaprc (False)
        Example:
             none
        """
        if insitu is None: insitu = rcParams['insitu']
        self._math._setinsitu(insitu)
        varlist = vars()
        s = scantable(self._math._smooth(self, kernel.lower(), width))
        s._add_history("smooth", varlist)
        print_log()
        if insitu: self._assign(s)
        else: return s


    def poly_baseline(self, mask=None, order=0, plot=False, uselin=False, insitu=None):
        """
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
        Example:
            # return a scan baselined by a third order polynomial,
            # not using a mask
            bscan = scan.poly_baseline(order=3)
        """
        if insitu is None: insitu = rcParams['insitu']
        varlist = vars()
        if mask is None:
            mask = [True for i in xrange(self.nchan(-1))]
        from asap.asapfitter import fitter
        try:
            f = fitter()
            f.set_scan(self, mask)
            if uselin:
                f.set_function(lpoly=order)
            else:
                f.set_function(poly=order)
            s = f.auto_fit(insitu, plot=plot)
            # Save parameters of baseline fits as a class attribute.
            # NOTICE: It does not reflect changes in scantable!
            self.blpars = f.blpars
            s._add_history("poly_baseline", varlist)
            print_log()
            if insitu: self._assign(s)
            else: return s
        except RuntimeError:
            msg = "The fit failed, possibly because it didn't converge."
            if rcParams['verbose']:
                print msg
                return
            else:
                raise RuntimeError(msg)


    def auto_poly_baseline(self, mask=[], edge=(0, 0), order=0,
                           threshold=3, chan_avg_limit=1, plot=False,
                           insitu=None):
        """
        Return a scan which has been baselined (all rows) by a polynomial.
        Spectral lines are detected first using linefinder and masked out
        to avoid them affecting the baseline solution.

        Parameters:
            mask:       an optional mask retreived from scantable
            edge:       an optional number of channel to drop at
                        the edge of spectrum. If only one value is
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

        Example:
            scan2=scan.auto_poly_baseline(order=7)
        """
        if insitu is None: insitu = rcParams['insitu']
        varlist = vars()
        from asap.asapfitter import fitter
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

        # setup fitter
        f = fitter()
        f.set_function(poly=order)

        # setup line finder
        fl = linefinder()
        fl.set_options(threshold=threshold,avg_limit=chan_avg_limit)

        if not insitu:
            workscan = self.copy()
        else:
            workscan = self

        fl.set_scan(workscan)

        rows = range(workscan.nrow())
        # Save parameters of baseline fits & masklists as a class attribute.
        # NOTICE: It does not reflect changes in scantable!
        if len(rows) > 0:
            self.blpars=[]
            self.masklists=[]
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

            # setup line finder
            fl.find_lines(r, mask, curedge)
            outmask=fl.get_mask()
            f.set_scan(workscan, fl.get_mask())
            f.x = workscan._getabcissa(r)
            f.y = workscan._getspectrum(r)
            f.data = None
            f.fit()
            
            # Show mask list
            masklist=workscan.get_masklist(fl.get_mask(),row=r)
            msg = "mask range: "+str(masklist)
            asaplog.push(msg, False)

            fpar = f.get_parameters()
            if plot:
                f.plot(residual=True)
                x = raw_input("Accept fit ( [y]/n ): ")
                if x.upper() == 'N':
                    self.blpars.append(None)
                    self.masklists.append(None)
                    continue
            workscan._setspectrum(f.fitter.getresidual(), r)
            self.blpars.append(fpar)
            self.masklists.append(masklist)
        if plot:
            f._p.unmap()
            f._p = None
        workscan._add_history("auto_poly_baseline", varlist)
        if insitu:
            self._assign(workscan)
        else:
            return workscan

    def rotate_linpolphase(self, angle):
        """
        Rotate the phase of the complex polarization O=Q+iU correlation.
        This is always done in situ in the raw data.  So if you call this
        function more than once then each call rotates the phase further.
        Parameters:
            angle:   The angle (degrees) to rotate (add) by.
        Examples:
            scan.rotate_linpolphase(2.3)
        """
        varlist = vars()
        self._math._rotate_linpolphase(self, angle)
        self._add_history("rotate_linpolphase", varlist)
        print_log()
        return


    def rotate_xyphase(self, angle):
        """
        Rotate the phase of the XY correlation.  This is always done in situ
        in the data.  So if you call this function more than once
        then each call rotates the phase further.
        Parameters:
            angle:   The angle (degrees) to rotate (add) by.
        Examples:
            scan.rotate_xyphase(2.3)
        """
        varlist = vars()
        self._math._rotate_xyphase(self, angle)
        self._add_history("rotate_xyphase", varlist)
        print_log()
        return

    def swap_linears(self):
        """
        Swap the linear polarisations XX and YY, or better the first two 
        polarisations as this also works for ciculars.
        """
        varlist = vars()
        self._math._swap_linears(self)
        self._add_history("swap_linears", varlist)
        print_log()
        return

    def invert_phase(self):
        """
        Invert the phase of the complex polarisation
        """
        varlist = vars()
        self._math._invert_phase(self)
        self._add_history("invert_phase", varlist)
        print_log()
        return

    def add(self, offset, insitu=None):
        """
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
        print_log()
        if insitu:
            self._assign(s)
        else:
            return s

    def scale(self, factor, tsys=True, insitu=None):
        """
        Return a scan where all spectra are scaled by the give 'factor'
        Parameters:
            factor:      the scaling factor
            insitu:      if False a new scantable is returned.
                         Otherwise, the scaling is done in-situ
                         The default is taken from .asaprc (False)
            tsys:        if True (default) then apply the operation to Tsys
                         as well as the data
        """
        if insitu is None: insitu = rcParams['insitu']
        self._math._setinsitu(insitu)
        varlist = vars()
        s = scantable(self._math._unaryop(self, factor, "MUL", tsys))
        s._add_history("scale", varlist)
        print_log()
        if insitu:
            self._assign(s)
        else:
            return s

    def auto_quotient(self, preserve=True, mode='paired'):
        """
        This function allows to build quotients automatically.
        It assumes the observation to have the same numer of
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

        """
        modes = ["time", "paired"]
        if not mode in modes:
            msg = "please provide valid mode. Valid modes are %s" % (modes)
            raise ValueError(msg)
        varlist = vars()
        s = None
        if mode.lower() == "paired":
            basesel = self.get_selection()
            sel = selector()+basesel
            sel.set_query("SRCTYPE==1")
            self.set_selection(sel)
            offs = self.copy()
            sel.set_query("SRCTYPE==0")
            self.set_selection(sel)
            ons = self.copy()
            s = scantable(self._math._quotient(ons, offs, preserve))
            self.set_selection(basesel)
        elif mode.lower() == "time":
            s = scantable(self._math._auto_quotient(self, mode, preserve))
        s._add_history("auto_quotient", varlist)
        print_log()
        return s

    def mx_quotient(self, mask = None, weight='median', preserve=True):
        """
        Form a quotient using "off" beams when observing in "MX" mode.
        Parameters:
            mask:           an optional mask to be used when weight == 'stddev'
            weight:         How to average the off beams.  Default is 'median'.
            preserve:       you can preserve (default) the continuum or
                            remove it.  The equations used are
                            preserve: Output = Toff * (on/off) - Toff
                            remove:   Output = Toff * (on/off) - Ton
        """
        if mask is None: mask = ()
        varlist = vars()
        on = scantable(self._math._mx_extract(self, 'on'))
        preoff = scantable(self._math._mx_extract(self, 'off'))
        off = preoff.average_time(mask=mask, weight=weight, scanav=False)
        from asapmath  import quotient
        q = quotient(on, off, preserve)
        q._add_history("mx_quotient", varlist)
        print_log()
        return q

    def freq_switch(self, insitu=None):
        """
        Apply frequency switching to the data.
        Parameters:
            insitu:      if False a new scantable is returned.
                         Otherwise, the swictching is done in-situ
                         The default is taken from .asaprc (False)
        Example:
            none
        """
        if insitu is None: insitu = rcParams['insitu']
        self._math._setinsitu(insitu)
        varlist = vars()
        s = scantable(self._math._freqswitch(self))
        s._add_history("freq_switch", varlist)
        print_log()
        if insitu: self._assign(s)
        else: return s

    def recalc_azel(self):
        """
        Recalculate the azimuth and elevation for each position.
        Parameters:
            none
        Example:
        """
        varlist = vars()
        self._recalcazel()
        self._add_history("recalc_azel", varlist)
        print_log()
        return

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
        print_log()
        return s

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
        print_log()
        return s

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
        print_log()
        return s


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
        print_log()
        return s

    def get_fit(self, row=0):
        """
        Print or return the stored fits for a row in the scantable
        Parameters:
            row:    the row which the fit has been applied to.
        """
        if row > self.nrow():
            return
        from asap.asapfit import asapfit
        fit = asapfit(self._getfit(row))
        if rcParams['verbose']:
            print fit
            return
        else:
            return fit.as_dict()

    def _add_history(self, funcname, parameters):
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

    def _fill(self, names, unit, average, getpt):
        import os
        from asap._asap import stfiller
        first = True
        fullnames = []
        for name in names:
            name = os.path.expandvars(name)
            name = os.path.expanduser(name)
            if not os.path.exists(name):
                msg = "File '%s' does not exists" % (name)
                if rcParams['verbose']:
                    asaplog.push(msg)
                    print asaplog.pop().strip()
                    return
                raise IOError(msg)
            fullnames.append(name)
        if average:
            asaplog.push('Auto averaging integrations')
        stype = int(rcParams['scantable.storage'].lower() == 'disk')
        for name in fullnames:
            tbl = Scantable(stype)
            r = stfiller(tbl)
            msg = "Importing %s..." % (name)
            asaplog.push(msg, False)
            print_log()
            r._open(name, -1, -1, getpt)
            r._read()
            #tbl = r._getdata()
            if average:
                tbl = self._math._average((tbl, ), (), 'NONE', 'SCAN')
                #tbl = tbl2
            if not first:
                tbl = self._math._merge([self, tbl])
                #tbl = tbl2
            Scantable.__init__(self, tbl)
            r._close()
            del r, tbl
            first = False
        if unit is not None:
            self.set_fluxunit(unit)
        #self.set_freqframe(rcParams['scantable.freqframe'])

