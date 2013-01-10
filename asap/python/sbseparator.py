import os, shutil
import numpy
import numpy.fft as FFT
import math

from asap.scantable import scantable
from asap.parameters import rcParams
from asap.logging import asaplog, asaplog_post_dec
from asap.selector import selector
from asap.asapgrid import asapgrid2
from asap._asap import SBSeparator

class sbseparator:
    """
    The sbseparator class is defined to separate SIGNAL and IMAGE
    sideband spectra observed by frequency-switching technique.
    It also helps supressing emmission of IMAGE sideband.
    *** WARNING *** THIS MODULE IS EXPERIMENTAL
    Known issues:
    - Frequencies of IMAGE sideband cannot be reconstructed from
      information in scantable in sideband sparation. Frequency
      setting of SIGNAL sideband is stored in output table for now.
    - Flag information (stored in FLAGTRA) is ignored.

    Example:
        # Create sideband separator instance whith 3 input data
        sbsep = sbseparator(['test1.asap', 'test2.asap', 'test3.asap'])
        # Set reference IFNO and tolerance to select data
        sbsep.set_frequency(5, 30, frame='TOPO')
        # Set direction tolerance to select data in unit of radian
        sbsep.set_dirtol(1.e-5)
        # Set rejection limit of solution
        sbsep.set_limit(0.2)
        # Solve image sideband as well
        sbsep.set_both(True)
        # Invoke sideband separation
        sbsep.separate('testout.asap', overwrite = True)
    """
    def __init__(self, infiles):
        self.intables = None
        self.signalShift = []
        self.imageShift = []
        self.dsbmode = True
        self.getboth = False
        self.rejlimit = 0.2
        self.baseif = -1
        self.freqtol = 10.
        self.freqframe = ""
        self.solveother = False
        self.dirtol = [1.e-5, 1.e-5] # direction tolerance in rad (2 arcsec)
        #self.lo1 = 0.

        self.tables = []
        self.nshift = -1
        self.nchan = -1

        self.set_data(infiles)
        
        self._separator = SBSeparator()

    @asaplog_post_dec
    def set_data(self, infiles):
        """
        Set data to be processed.

        infiles  : a list of filenames or scantables
        """
        if not (type(infiles) in (list, tuple, numpy.ndarray)):
            infiles = [infiles]
        if isinstance(infiles[0], scantable):
            # a list of scantable
            for stab in infiles:
                if not isinstance(stab, scantable):
                    asaplog.post()
                    raise TypeError, "Input data is not a list of scantables."

            #self._separator._setdata(infiles)
            self._reset_data()
            self.intables = infiles
        else:
            # a list of filenames
            for name in infiles:
                if not os.path.exists(name):
                    asaplog.post()
                    raise ValueError, "Could not find input file '%s'" % name
            
            #self._separator._setdataname(infiles)
            self._reset_data()
            self.intables = infiles

        asaplog.push("%d files are set to process" % len(self.intables))


    def _reset_data(self):
        del self.intables
        self.intables = None
        self.signalShift = []
        #self.imageShift = []
        self.tables = []
        self.nshift = -1
        self.nchan = -1

    @asaplog_post_dec
    def set_frequency(self, baseif, freqtol, frame=""):
        """
        Set IFNO and frequency tolerance to select data to process.

        Parameters:
          - reference IFNO to process in the first table in the list
          - frequency tolerance from reference IF to select data
          frame  : frequency frame to select IF
        """
        self._reset_if()
        self.baseif = baseif
        if isinstance(freqtol,dict) and freqtol["unit"] == "Hz":
            if freqtol['value'] > 0.:
                self.freqtol = freqtol
            else:
                asaplog.post()
                asaplog.push("Frequency tolerance should be positive value.")
                asaplog.post("ERROR")
                return
        else:
            # torelance in channel unit
            if freqtol > 0:
                self.freqtol = float(freqtol)
            else:
                asaplog.post()
                asaplog.push("Frequency tolerance should be positive value.")
                asaplog.post("ERROR")
                return
        self.freqframe = frame

    def _reset_if(self):
        self.baseif = -1
        self.freqtol = 0
        self.freqframe = ""
        self.signalShift = []
        #self.imageShift = []
        self.tables = []
        self.nshift = 0
        self.nchan = -1

    @asaplog_post_dec
    def set_dirtol(self, dirtol=[1.e-5,1.e-5]):
        """
        Set tolerance of direction to select data
        """
        # direction tolerance in rad
        if not (type(dirtol) in [list, tuple, numpy.ndarray]):
            dirtol = [dirtol, dirtol]
        if len(dirtol) == 1:
            dirtol = [dirtol[0], dirtol[0]]
        if len(dirtol) > 1:
            self.dirtol = dirtol[0:2]
        else:
            asaplog.post()
            asaplog.push("Invalid direction tolerance. Should be a list of float in unit radian")
            asaplog.post("ERROR")
            return
        asaplog.post("Set direction tolerance [%f, %f] (rad)" % \
                     (self.dirtol[0], self.dirtol[1]))

    @asaplog_post_dec
    def set_shift(self, mode="DSB", imageshift=None):
        """
        Set shift mode and channel shift of image band.

        mode       : shift mode ['DSB'|'SSB'(='2SB')]
                     When mode='DSB', imageshift is assumed to be equal
                     to the shift of signal sideband but in opposite direction.
        imageshift : a list of number of channel shift in image band of
                     each scantable. valid only mode='SSB'
        """
        if mode.upper().startswith("D"):
            # DSB mode
            self.dsbmode = True
            self.imageShift = []
        else:
            if not imageshift:
                raise ValueError, "Need to set shift value of image sideband"
            self.dsbmode = False
            self.imageShift = imageshift
            asaplog.push("Image sideband shift is set manually: %s" % str(self.imageShift))

    @asaplog_post_dec
    def set_both(self, flag=False):
        """
        Resolve both image and signal sideband when True.
        """
        self.getboth = flag
        if self.getboth:
            asaplog.push("Both signal and image sidebands are solved and output as separate tables.")
        else:
            asaplog.push("Only signal sideband is solved and output as an table.")

    @asaplog_post_dec
    def set_limit(self, threshold=0.2):
        """
        Set rejection limit of solution.
        """
        #self._separator._setlimit(abs(threshold))
        self.rejlimit = threshold
        asaplog.push("The threshold of rejection is set to %f" % self.rejlimit)


    @asaplog_post_dec
    def set_solve_other(self, flag=False):
        """
        Calculate spectra by subtracting the solution of the other sideband
        when True.
        """
        self.solveother = flag
        if flag:
            asaplog.push("Expert mode: solution are obtained by subtraction of the other sideband.")

    def set_lo1(self,lo1):
        """
        Set LO1 frequency to calculate frequency of image sideband.

        lo1 : LO1 frequency in float
        """
        lo1val = -1.
        if isinstance(lo1, dict) and lo1["unit"] == "Hz":
            lo1val = lo1["value"]
        else:
            lo1val = float(lo1)
        if lo1val <= 0.:
            asaplog.push("Got negative LO1 frequency. It will be ignored.")
            asaplog.post("WARN")
        else:
            self._separator.set_lo1(lo1val)


    def set_lo1root(self, name):
        """
        Set MS name which stores LO1 frequency of signal side band.
        It is used to calculate frequency of image sideband.

        name : MS name which contains 'ASDM_SPECTRALWINDOW' and
               'ASDM_RECEIVER' tables.
        """
        self._separator.set_lo1root(name)


    @asaplog_post_dec
    def separate(self, outname="", overwrite=False):
        """
        Invoke sideband separation.

        outname   : a name of output scantable
        overwrite : overwrite existing table
        """
        # List up valid scantables and IFNOs to convolve.
        #self._separator._separate()
        self._setup_shift()
        #self._preprocess_tables()

        ### TEMPORAL ###
        self._separator._get_asistb_from_scantb(self.tables[0])
        ################

        nshift = len(self.tables)
        signaltab = self._grid_outtable(self.tables[0])
        if self.getboth:
            imagetab = signaltab.copy()

        rejrow = []
        for irow in xrange(signaltab.nrow()):
            currpol = signaltab.getpol(irow)
            currbeam = signaltab.getbeam(irow)
            currdir = signaltab.get_directionval(irow)
            spec_array, tabidx = self._get_specarray(polid=currpol,\
                                                     beamid=currbeam,\
                                                     dir=currdir)
            #if not spec_array:
            if len(tabidx) == 0:
                asaplog.post()
                asaplog.push("skipping row %d" % irow)
                rejrow.append(irow)
                continue
            signal = self._solve_signal(spec_array, tabidx)
            signaltab.set_spectrum(signal, irow)

            # Solve image side side band
            if self.getboth:
                image = self._solve_image(spec_array, tabidx)
                imagetab.set_spectrum(image, irow)

        # TODO: Need to remove rejrow form scantables here
        signaltab.flag_row(rejrow)
        if self.getboth:
            imagetab.flag_row(rejrow)
        
        if outname == "":
            outname = "sbsepareted.asap"
        signalname = outname + ".signalband"
        if os.path.exists(signalname):
            if not overwrite:
                raise Exception, "Output file '%s' exists." % signalname
            else:
                shutil.rmtree(signalname)
        signaltab.save(signalname)

        if self.getboth:
            # Warnings
            asaplog.post()
            asaplog.push("Saving IMAGE sideband.")
            #asaplog.push("Note, frequency information of IMAGE sideband cannot be properly filled so far. (future development)")
            #asaplog.push("Storing frequency setting of SIGNAL sideband in FREQUENCIES table for now.")
            #asaplog.post("WARN")

            imagename = outname + ".imageband"
            if os.path.exists(imagename):
                if not overwrite:
                    raise Exception, "Output file '%s' exists." % imagename
                else:
                    shutil.rmtree(imagename)
            # Update frequency information
            self._separator.set_imgtable(imagetab)
            self._separator.solve_imgfreq()
            imagetab.save(imagename)


    def _solve_signal(self, data, tabidx=None):
        if not tabidx:
            tabidx = range(len(data))

        tempshift = []
        dshift = []
        if self.solveother:
            for idx in tabidx:
                tempshift.append(-self.imageShift[idx])
                dshift.append(self.signalShift[idx] - self.imageShift[idx])
        else:
            for idx in tabidx:
                tempshift.append(-self.signalShift[idx])
                dshift.append(self.imageShift[idx] - self.signalShift[idx])

        shiftdata = numpy.zeros(data.shape, numpy.float)
        for i in range(len(data)):
            shiftdata[i] = self._shiftSpectrum(data[i], tempshift[i])
        ifftdata = self._Deconvolution(shiftdata, dshift, self.rejlimit)
        result_image = self._combineResult(ifftdata)
        if not self.solveother:
            return result_image
        result_signal = self._subtractOtherSide(shiftdata, dshift, result_image)
        return result_signal


    def _solve_image(self, data, tabidx=None):
        if not tabidx:
            tabidx = range(len(data))

        tempshift = []
        dshift = []
        if self.solveother:
            for idx in tabidx:
                tempshift.append(-self.signalShift[idx])
                dshift.append(self.imageShift[idx] - self.signalShift[idx])
        else:
            for idx in tabidx:
                tempshift.append(-self.imageShift[idx])
                dshift.append(self.signalShift[idx] - self.imageShift[idx])

        shiftdata = numpy.zeros(data.shape, numpy.float)
        for i in range(len(data)):
            shiftdata[i] = self._shiftSpectrum(data[i], tempshift[i])
        ifftdata = self._Deconvolution(shiftdata, dshift, self.rejlimit)
        result_image = self._combineResult(ifftdata)
        if not self.solveother:
            return result_image
        result_signal = self._subtractOtherSide(shiftdata, dshift, result_image)
        return result_signal

    @asaplog_post_dec
    def _grid_outtable(self, table):
        # Generate gridded table for output (Just to get rows)
        gridder = asapgrid2(table)
        gridder.setIF(self.baseif)

        cellx = str(self.dirtol[0])+"rad"
        celly = str(self.dirtol[1])+"rad"
        dirarr = numpy.array(table.get_directionval()).transpose()
        mapx = dirarr[0].max() - dirarr[0].min()
        mapy = dirarr[1].max() - dirarr[1].min()
        centy = 0.5 * (dirarr[1].max() + dirarr[1].min())
        nx = max(1, numpy.ceil(mapx*numpy.cos(centy)/self.dirtol[0]))
        ny = max(1, numpy.ceil(mapy/self.dirtol[0]))

        asaplog.push("Regrid output scantable with cell = [%s, %s]" % \
                     (cellx, celly))
        gridder.defineImage(nx=nx, ny=ny, cellx=cellx, celly=celly)
        gridder.setFunc(func='box', convsupport=1)
        gridder.setWeight(weightType='uniform')
        gridder.grid()
        return gridder.getResult()

    @asaplog_post_dec
    def _get_specarray(self, polid=None, beamid=None, dir=None):
        ntable = len(self.tables)
        spec_array = numpy.zeros((ntable, self.nchan), numpy.float)
        nspec = 0
        asaplog.push("Start data selection by POL=%d, BEAM=%d, direction=[%f, %f]" % (polid, beamid, dir[0], dir[1]))
        tabidx = []
        for itab in range(ntable):
            tab = self.tables[itab]
            # Select rows by POLNO and BEAMNO
            try:
                tab.set_selection(pols=[polid], beams=[beamid])
                if tab.nrow() > 0: tabidx.append(itab)
            except: # no selection
                asaplog.post()
                asaplog.push("table %d - No spectrum ....skipping the table" % (itab))
                asaplog.post("WARN")
                continue

            # Select rows by direction
            spec = numpy.zeros(self.nchan, numpy.float)
            selrow = []
            for irow in range(tab.nrow()):
                currdir = tab.get_directionval(irow)
                if (abs(currdir[0] - dir[0]) > self.dirtol[0]) or \
                   (abs(currdir[1] - dir[1]) > self.dirtol[1]):
                    continue
                selrow.append(irow)
            if len(selrow) == 0:
                asaplog.post()
                asaplog.push("table %d - No spectrum ....skipping the table" % (itab))
                asaplog.post("WARN")
                continue
            else:
                seltab = tab.copy()
                seltab.set_selection(selector(rows=selrow))

            if tab.nrow() > 1:
                asaplog.push("table %d - More than a spectrum selected. averaging rows..." % (itab))
                tab = seltab.average_time(scanav=False, weight="tintsys")
            else:
                tab = seltab

            spec_array[nspec] = tab._getspectrum()
            nspec += 1

        if nspec != ntable:
            asaplog.post()
            #asaplog.push("Some tables has no spectrum with POL=%d BEAM=%d. averaging rows..." % (polid, beamid))
            asaplog.push("Could not find corresponding rows in some tables.")
            asaplog.push("Number of spectra selected = %d (table: %d)" % (nspec, ntable))
            if nspec < 2:
                asaplog.push("At least 2 spectra are necessary for convolution")
                asaplog.post("ERROR")
                return False, tabidx

        return spec_array[0:nspec], tabidx


    @asaplog_post_dec
    def _setup_shift(self):
        ### define self.tables, self.signalShift, and self.imageShift
        if not self.intables:
            asaplog.post()
            raise RuntimeError, "Input data is not defined."
        #if self.baseif < 0:
        #    asaplog.post()
        #    raise RuntimeError, "Reference IFNO is not defined."
        
        byname = False
        #if not self.intables:
        if isinstance(self.intables[0], str):
            # A list of file name is given
            if not os.path.exists(self.intables[0]):
                asaplog.post()
                raise RuntimeError, "Could not find '%s'" % self.intables[0]
            
            stab = scantable(self.intables[0],average=False)
            ntab = len(self.intables)
            byname = True
        else:
            stab = self.intables[0]
            ntab = len(self.intables)

        if len(stab.getbeamnos()) > 1:
            asaplog.post()
            asaplog.push("Mult-beam data is not supported by this module.")
            asaplog.post("ERROR")
            return

        valid_ifs = stab.getifnos()
        if self.baseif < 0:
            self.baseif = valid_ifs[0]
            asaplog.post()
            asaplog.push("IFNO is not selected. Using the first IF in the first scantable. Reference IFNO = %d" % (self.baseif))

        if not (self.baseif in valid_ifs):
            asaplog.post()
            errmsg = "IF%d does not exist in the first scantable" %  \
                     self.baseif
            raise RuntimeError, errmsg

        asaplog.push("Start selecting tables and IFNOs to solve.")
        asaplog.push("Checking frequency of the reference IF")
        unit_org = stab.get_unit()
        coord = stab._getcoordinfo()
        frame_org = coord[1]
        stab.set_unit("Hz")
        if len(self.freqframe) > 0:
            stab.set_freqframe(self.freqframe)
        stab.set_selection(ifs=[self.baseif])
        spx = stab._getabcissa()
        stab.set_selection()
        basech0 = spx[0]
        baseinc = spx[1]-spx[0]
        self.nchan = len(spx)
        # frequency tolerance
        if isinstance(self.freqtol, dict) and self.freqtol['unit'] == "Hz":
            vftol = abs(self.freqtol['value'])
        else:
            vftol = abs(baseinc * float(self.freqtol))
            self.freqtol = dict(value=vftol, unit="Hz")
        # tolerance of frequency increment
        inctol = abs(baseinc/float(self.nchan))
        asaplog.push("Reference frequency setup (Table = 0, IFNO = %d):  nchan = %d, chan0 = %f Hz, incr = %f Hz" % (self.baseif, self.nchan, basech0, baseinc))
        asaplog.push("Allowed frequency tolerance = %f Hz ( %f channels)" % (vftol, vftol/baseinc))
        poltype0 = stab.poltype()

        self.tables = []
        self.signalShift = []
        if self.dsbmode:
            self.imageShift = []

        for itab in range(ntab):
            asaplog.push("Table %d:" % itab)
            tab_selected = False
            if itab > 0:
                if byname:
                    stab = scantable(self.intables[itab],average=False)
                else:
                    stab = self.intables[itab]
                unit_org = stab.get_unit()
                coord = stab._getcoordinfo()
                frame_org = coord[1]
                stab.set_unit("Hz")
                if len(self.freqframe) > 0:
                    stab.set_freqframe(self.freqframe)

            # Check POLTYPE should be equal to the first table.
            if stab.poltype() != poltype0:
                asaplog.post()
                raise Exception, "POLTYPE should be equal to the first table."
            # Multiple beam data may not handled properly
            if len(stab.getbeamnos()) > 1:
                asaplog.post()
                asaplog.push("table contains multiple beams. It may not be handled properly.")
                asaplog.push("WARN")

            for ifno in stab.getifnos():
                stab.set_selection(ifs=[ifno])
                spx = stab._getabcissa()
                if (len(spx) != self.nchan) or \
                   (abs(spx[0]-basech0) > vftol) or \
                   (abs(spx[1]-spx[0]-baseinc) > inctol):
                    continue
                tab_selected = True
                seltab = stab.copy()
                seltab.set_unit(unit_org)
                seltab.set_freqframe(frame_org)
                self.tables.append(seltab)
                self.signalShift.append((spx[0]-basech0)/baseinc)
                if self.dsbmode:
                    self.imageShift.append(-self.signalShift[-1])
                asaplog.push("- IF%d selected: sideband shift = %16.12e channels" % (ifno, self.signalShift[-1]))
            stab.set_selection()
            stab.set_unit(unit_org)
            stab.set_freqframe(frame_org)
            if not tab_selected:
                asaplog.post()
                asaplog.push("No data selected in Table %d" % itab)
                asaplog.post("WARN")

        asaplog.push("Total number of IFs selected = %d" % len(self.tables))
        if len(self.tables) < 2:
            asaplog.post()
            raise RuntimeError, "At least 2 IFs are necessary for convolution!"

        if not self.dsbmode and len(self.imageShift) != len(self.signalShift):
            asaplog.post()
            errmsg = "User defined channel shift of image sideband has %d elements, while selected IFNOs are %d" % (len(self.imageShift), len(self.signalShift))
            errmsg += "\nThe frequency tolerance (freqtol) you set may be too small."
            raise RuntimeError, errmsg

        self.signalShift = numpy.array(self.signalShift)
        self.imageShift = numpy.array(self.imageShift)
        self.nshift = len(self.tables)

    @asaplog_post_dec
    def _preprocess_tables(self):
        ### temporary method to preprocess data
        ### Do time averaging for now.
        for itab in range(len(self.tables)):
            self.tables[itab] = self.tables[itab].average_time(scanav=False, weight="tintsys")

#     @asaplog_post_dec
#     def _setup_image_freq(self, table):
#         # User defined coordinate
#         # Get from associated MS
#         # Get from ASDM
#         lo1 = -1.
#         if self.lo1 > 0.:
#             asaplog.push("Using user defined LO1 frequency %e16.12 [Hz]" % self.lo1)
#             lo1 = self.lo1
#         else:
#             print "NOT IMPLEMENTED YET!!!"

#     def save(self, outfile, outform="ASAP", overwrite=False):
#         if not overwrite and os.path.exists(outfile):
#             raise RuntimeError, "Output file '%s' already exists" % outfile
# 
#         #self._separator._save(outfile, outform)

#     def done(self):
#         self.close()

#     def close(self):
#         pass
#         #del self._separator
    


########################################################################
    def _Deconvolution(self, data_array, shift, threshold=0.00000001):
        FObs = []
        Reject = 0
        nshift, nchan = data_array.shape
        nspec = nshift*(nshift-1)/2
        ifftObs  = numpy.zeros((nspec, nchan), numpy.float)
        for i in range(nshift):
           F = FFT.fft(data_array[i])
           FObs.append(F)
        z = 0
        for i in range(nshift):
            for j in range(i+1, nshift):
                Fobs = (FObs[i]+FObs[j])/2.0
                dX = (shift[j]-shift[i])*2.0*math.pi/float(self.nchan)
                #print 'dX,i,j=',dX,i,j
                for k in range(1,self.nchan):
                    if math.fabs(math.sin(dX*k)) > threshold:
                        Fobs[k] += ((FObs[i][k]-FObs[j][k])/2.0/(1.0-math.cos(dX*k))*math.sin(dX*k))*1.0j
                    else: Reject += 1
                ifftObs[z] = FFT.ifft(Fobs)
                z += 1
        print 'Threshold=%s Reject=%d' % (threshold, Reject)
        return ifftObs

    def _combineResult(self, ifftObs):
        nspec = len(ifftObs)
        sum = ifftObs[0]
        for i in range(1,nspec):
            sum += ifftObs[i]
        return(sum/float(nspec))

    def _subtractOtherSide(self, data_array, shift, Data):
        sum = numpy.zeros(len(Data), numpy.float)
        numSP = len(data_array)
        for i in range(numSP):
            SPsub = data_array[i] - Data
            sum += self._shiftSpectrum(SPsub, -shift[i])
        return(sum/float(numSP))

    def _shiftSpectrum(self, data, Shift):
        Out = numpy.zeros(self.nchan, numpy.float)
        w2 = Shift % 1
        w1 = 1.0 - w2
        for i in range(self.nchan):
            c1 = int((Shift + i) % self.nchan)
            c2 = (c1 + 1) % self.nchan
            Out[c1] += data[i] * w1
            Out[c2] += data[i] * w2
        return Out.copy()
