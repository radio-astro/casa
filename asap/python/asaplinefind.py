import _asap

class linefinder:
    """
    The class for automated spectral line search in ASAP.

    Example:
       fl=linefinder()
       fl.set_scan(sc)
       fl.set_options(threshold=3)
       nlines=fl.find_lines(edge=(50,0))
       if nlines!=0:
          print "Found ",nlines," spectral lines"
          print fl.get_ranges(False)
       else:
          print "No lines found!"
       sc2=sc.poly_baseline(fl.get_mask(),7)

    The algorithm involves a simple threshold criterion. The line is
    considered to be detected if a specified number of consequtive
    channels (default is 3) is brighter (with respect to the current baseline
    estimate) than the threshold times the noise level. This criterion is
    applied in the iterative procedure updating baseline estimate and trying
    reduced spectral resolutions to detect broad lines as well. The off-line
    noise level is determined at each iteration as an average of 80% of the
    lowest variances across the spectrum (i.e. histogram equalization is
    used to avoid missing weak lines if strong ones are present). For
    bad baseline shapes it is recommended to increase the threshold and
    possibly switch the averaging option off (see set_options) to
    detect strong lines only, fit a high order baseline and repeat the line
    search.

    """

    def __init__(self):
        """
        Create a line finder object.
        """
        self.finder = _asap.linefinder()
        return

    def set_options(self,threshold=1.7320508075688772,min_nchan=3,
        avg_limit=8,box_size=0.2,noise_box='all',noise_stat='mean80'):
        """
        Set the parameters of the algorithm
        Parameters:
             threshold    a single channel S/N ratio above which the
                          channel is considered to be a detection
                          Default is sqrt(3), which together with
                          min_nchan=3 gives a 3-sigma criterion
             min_nchan    a minimal number of consequtive channels,
                          which should satisfy a threshold criterion to
                          be a detection. Default is 3.
             avg_limit    A number of consequtive channels not greater than
                          this parameter can be averaged to search for
                          broad lines. Default is 8.
             box_size     A running mean/median box size specified as a fraction
                          of the total spectrum length. Default is 1/5
             noise_box    Area of the spectrum used to estimate noise stats
                          Both string values and numbers are allowed
                          Allowed string values:
                             'all' use all the spectrum (default)
                             'box' noise box is the same as running mean/median
                                   box
                          Numeric values are defined as a fraction from the
                          spectrum size. Values should be positive.
                          (noise_box == box_size has the same effect as
                           noise_box = 'box')
             noise_stat   Statistics used to estimate noise, allowed values:
                              'mean80' use the 80% of the lowest deviations
                                       in the noise box (default)
                              'median' median of deviations in the noise box

        Note:  For bad baselines threshold should be increased,
               and avg_limit decreased (or even switched off completely by
               setting this parameter to 1) to avoid detecting baseline
               undulations instead of real lines.
        """
        if noise_stat.lower() not in ["mean80",'median']:
           raise RuntimeError, "noise_stat argument in linefinder.set_options can only be mean80 or median"
        nStat = (noise_stat.lower() == "median")
        nBox = -1.
        if isinstance(noise_box,str):
           if noise_box.lower() not in ['all','box']:
              raise RuntimeError, "string-valued noise_box in linefinder.set_options can only be all or box"
           if noise_box.lower() == 'box':
              nBox = box_size
        else:
           nBox = float(noise_box)
        self.finder.setoptions(threshold,min_nchan,avg_limit,box_size,nBox,nStat)
        return

    def set_scan(self, scan):
        """
        Set the 'data' (scantable) to work with.
        Parameters:
             scan:    a scantable
        """
        if not scan:
           raise RuntimeError, 'Please give a correct scan'
        self.finder.setscan(scan)

    def set_data(self, spectrum):
        """
        Set the 'data' (spectrum) to work with
        Parameters: a method to allow linefinder work without setting scantable
        for the purpose of using linefinder inside some method in scantable
        class. (Dec 22, 2010 by W.Kawasaki)
        """
        if isinstance(spectrum, list) or isinstance(spectrum, tuple):
            if not isinstance(spectrum[0], float):
                raise RuntimeError, "Parameter 'spectrum' has to be a list or a tuple of float"
        else:
            raise RuntimeError, "Parameter 'spectrum' has to be a list or a tuple of float"
        self.finder.setdata(spectrum)
        
    def find_lines(self,nRow=0,mask=[],edge=(0,0)):
        """
        Search for spectral lines in the scan assigned in set_scan.
        Parameters:
	     nRow:       a row in the scantable to work with
             mask:       an optional mask (e.g. retreived from scantable)
             edge:       an optional number of channels to drop at
                         the edge of the spectrum. If only one value is
                         specified, the same number will be dropped from
                         both sides of the spectrum. Default is to keep
                         all channels
        A number of lines found will be returned
        """
        if isinstance(edge,int):
           edge=(edge,)

        from asap import _is_sequence_or_number as _is_valid

        if not _is_valid(edge, int):
           raise RuntimeError, "Parameter 'edge' has to be an integer or \
           a pair of integers specified as a tuple"

        if len(edge)>2:
           raise RuntimeError, "The edge parameter should have two \
           or less elements"
        return self.finder.findlines(mask,list(edge),nRow)
    def get_mask(self,invert=False):
        """
        Get the mask to mask out all lines that have been found (default)

        Parameters:
              invert  if True, only channels belong to lines will be unmasked

        Note: all channels originally masked by the input mask or
              dropped out by the edge parameter will still be excluded
              regardless on the invert option
        """
        return self.finder.getmask(invert)
    def get_ranges(self,defunits=True):
        """
        Get ranges (start and end channels or velocities) for all spectral
        lines found.

        Parameters:
              defunits  if True (default), the range will use the same units
                        as set for the scan (e.g. LSR velocity)
                        if False, the range will be expressed in channels
        """
        if (defunits):
            return self.finder.getlineranges()
        else:
            return self.finder.getlinerangesinchannels()
