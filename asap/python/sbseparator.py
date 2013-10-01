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
        self._separator = SBSeparator(infiles)


    def set_frequency(self, baseif, freqtol, frame=""):
        """
        Set IFNO and frequency tolerance to select data to process.

        Parameters:
          - reference IFNO to process in the first table in the list
          - frequency tolerance from reference IF to select data (string)
          frame  : frequency frame to select IF
        """
        if type(freqtol) in (float, int):
            freqtol = str(freqtol)
        elif isinstance(freqtol, dict):
            try:
                freqtol = str(freqtol['value']) + freqtol['unit']
            except:
                raise ValueError, "Invalid frequency tolerance."
        self._separator.set_freq(baseif, freqtol, frame)


    def set_dirtol(self, dirtol=["2arcsec", "2arcsec"]):
        """
        Set tolerance of direction to select data
        """
        if isinstance(dirtol, str):
            dirtol = [dirtol]

        self._separator.set_dirtol(dirtol)
    
            
    def set_shift(self, imageshift=[]):
        """
        Set shift mode and channel shift of image band.

        imageshift : a list of number of channels shifted in image
                     side band of each scantable.
                     If the shifts are not set, they are assumed to be
                     equal to those of signal side band, but in opposite
                     direction as usual by LO1 offsetting of DSB receivers.
        """
        if not imageshift:
            imageshift = []
        self._separator.set_shift(imageshift)


    @asaplog_post_dec
    def set_both(self, flag=False):
        """
        Resolve both image and signal sideband when True.
        """
        self._separator.solve_both(flag)
        if flag:
            asaplog.push("Both signal and image sidebands will be solved and stored in separate tables.")
        else:
            asaplog.push("Only signal sideband will be solved and stored in an table.")

    @asaplog_post_dec
    def set_limit(self, threshold=0.2):
        """
        Set rejection limit of solution.
        """
        self._separator.set_limit(threshold)


    @asaplog_post_dec
    def set_solve_other(self, flag=False):
        """
        Calculate spectra by subtracting the solution of the other sideband
        when True.
        """
        self._separator.subtract_other(flag)
        if flag:
            asaplog.push("Expert mode: solution are obtained by subtraction of the other sideband.")


    def set_lo1(self, lo1, frame="TOPO", reftime=-1, refdir=""):
        """
        Set LO1 frequency to calculate frequency of image sideband.

        lo1     : LO1 frequency
        frame   : the frequency frame of LO1
        reftime : the reference time used in frequency frame conversion.
        refdir  : the reference direction used in frequency frame conversion.
        """
        self._separator.set_lo1(lo1, frame, reftime, refdir)


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
        out_default = "sbseparated.asap"
        if len(outname) == 0:
            outname = out_default
            asaplog.post()
            asaplog.push("The output file name is not specified.")
            asaplog.push("Using default name '%s'" % outname)
            asaplog.post("WARN")

        if os.path.exists(outname):
            if overwrite:
                asaplog.push("removing the old file '%s'" % outname)
                shutil.rmtree(outname)
            else:
                asaplog.post()
                asaplog.push("Output file '%s' already exists." % outname)
                asaplog.post("ERROR")
                return False

        self._separator.separate(outname)

