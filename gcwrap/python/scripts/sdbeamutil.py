import numpy as np
from numpy import sqrt, cos, sin
from taskinit import casalog, gentools, qatool

import scipy.special as spspec
import scipy.signal
import scipy.interpolate
from scipy import optimize

##################################################
### Prediction of theoretical beam size
##################################################
class TheoreticalBeam:
    """
    The class to derive the theoretical beam size of an image.

    Example:
    bu = sdbeamutil.TheoreticalBeam()
    # set imaging, antenna, and pointing informations
    bu.set_antenna('12m',blockage='0.75m',taper=10)
    bu.set_sampling(['12arcsec','12arcsec'])
    bu.set_image_param('5arcsec', '115GHz','SF', 6, -1, -1, -1)
    # print summary of setup to logger
    bu.summary()
    # get theoretical beam size of an image.
    beam = bu.get_beamsize_image()
    """
    def __init__(self):
        self.is_antenna_set = False
        self.is_kernel_set = False
        self.is_sampling_set = False
        self.antenna_diam_m = -1.
        self.antenna_block_m = 0.0
        self.taper = 10
        self.ref_freq = -1.
        self.kernel_type=""
        self.kernel_param={}
        self.pa = "0.0deg"
        self.sampling_arcsec = []
        self.cell_arcsec = []

    def __to_arcsec_list(self, angle):
        """return a list of angles in arcsec (value only without unit)"""
        if type(angle) not in [list, tuple, np.ndarray]:
            angle = [angle]
        return [self.__to_arcsec(val) for val in angle]

    def __to_arcsec(self, angle):
        """convert angle to arcsec and return the value without unit."""
        my_qa = qatool()
        if my_qa.isangle(angle):
            return my_qa.getvalue(my_qa.convert(angle, "arcsec"))[0]
        elif my_qa.getunit(angle)=='': return float(angle)
        else: raise ValueError, "Invalid angle: %s" % (str(angle))

    def __parse_width(self, val, cell_size_arcsec):
        """
        Convert value in unit of arcsec
        if val is angle, returns a float value in unit of arcsec.
        else the unit is assumed to be pixel and multiplied by cell_size_arcsec
        """
        my_qa = qatool()
        if my_qa.isangle(val): return self.__to_arcsec(val)
        elif my_qa.getunit(val) in ('', 'pixel'):
            return my_qa.getvalue(val)*cell_size_arcsec
        else: raise ValueError, "Invalid width %s" % str(val)

    
    def set_antenna(self, diam, blockage="0.0m", taper=10):
        """
        set parameters to construct antenna beam
        antenna diameter and blockage
        taper: the illumination taper in dB
        """
        # try quantity
        my_qa = qatool()
        self.antenna_diam_m  = my_qa.getvalue(my_qa.convert(diam, "m"))[0]
        self.antenna_block_m = my_qa.getvalue(my_qa.convert(blockage, "m"))[0]
        self.taper = taper
        self.is_antenna_set = True

    def set_sampling(self, intervals, pa="0deg"):
        """
        Aet sampling interval of observation
        intervals: pointing inteval of observation (['10arcsec','10arcsec'])
        pa: position angle (NOT USED)
        """
        self.pa = pa
        self.sampling_arcsec = [ abs(a) for a in
                                 self.__to_arcsec_list(intervals) ]
        self.is_sampling_set = True

    def set_image_param(self, cell, ref_freq, gridfunction,
                        convsupport, truncate, gwidth, jwidth,is_alma=False):
        """
        Set imaging parameters
        cell: image pixel size
        ref_freq: reference frequency of image to calculate beam size
        gridfunction, convsupport, truncate, gwidth, jwidth:
            parameters passed to imager
        is_alma: valid only for PB kernel to use 10.7m
        """
        self.ref_freq = ref_freq
        self.cell_arcsec = [ abs(a) for a in
                             self.__to_arcsec_list(cell) ]
        if gridfunction.upper() == "SF":
            self.__set_sf_kernel(convsupport)
        elif gridfunction.upper() == "GJINC":
            self.__set_gjinc_kernel(truncate,gwidth,jwidth)
        elif gridfunction.upper() == "GAUSS":
            self.__set_gauss_kernel(truncate, gwidth)
        elif gridfunction.upper() == "BOX":
            self.__set_box_kernel(self.cell_arcsec[0])
        elif gridfunction.upper() == "PB":
            self.__set_pb_kernel(is_alma)
        self.is_kernel_set = True

    def __set_sf_kernel(self,convsupport):
        """Set SF kernel parameter to the class"""
        self.kernel_type="SF"
        self.kernel_param = dict(convsupport=convsupport)

    def __set_gjinc_kernel(self,truncate,gwidth,jwidth):
        """Set GJINC kernel parameter to the class"""
        self.kernel_type="GJINC"
        self.kernel_param = dict(truncate=truncate,gwidth=gwidth,jwidth=jwidth)

    def __set_gauss_kernel(self,truncate,gwidth):
        """Set GAUSS kernel parameter to the class"""
        self.kernel_type="GAUSS"
        self.kernel_param = dict(truncate=truncate,gwidth=gwidth)

    def __set_box_kernel(self,width):
        """Set BOX kernel parameter to the class"""
        self.kernel_type="BOX"
        self.kernel_param=dict(width=width)

    def __set_pb_kernel(self,alma=False):
        """Set PB kernel parameter to the class"""
        self.kernel_type = "PB"
        self.kernel_param=dict(alma=alma)

    def get_beamsize_image(self):
        """
        Returns FWHM of theoretical beam size in image.
        The FWHM is derived by fitting gridded beam with Gaussian function.
        """
        # assert necessary information is set
        self.__assert_antenna()
        self.__assert_kernel()
        self.__assert_sampling()
        casalog.post("Calculating theoretical beam size of the image")
        # construct theoretic beam for image
        axis, beam = self.get_antenna_beam()
        casalog.post("Length of convolution array=%d, total width=%f arcsec, separation=%f arcsec" % (len(axis), axis[-1]-axis[0], axis[1]-axis[0]),
                     priority="DEBUG1")
        kernel = self.get_kernel(axis)
        sampling = self.get_box_kernel(axis,self.sampling_arcsec[0])
        # convolution
        gridded = scipy.signal.convolve(beam,kernel,mode='same')
        gridded /= max(gridded)
        result = scipy.signal.convolve(gridded,sampling,mode='same')
        result /= max(result)
        #fwhm_arcsec = findFWHM(axis,result)
        fwhm_arcsec, dummy = self.gaussfit(axis, result, minlevel=0.0,truncate=False)
        ### DEBUG MESSAGE
        casalog.post("- initial FWHM of beam = %f arcsec" %
                     findFWHM(axis,beam))
        casalog.post("- FWHM of gridding kernel = %f arcsec" %
                     findFWHM(axis,kernel))
        casalog.post("- FWHM of theoretical beam = %f arcsec" %
                     findFWHM(axis,result))
        casalog.post("- FWHM of theoretical beam (gauss fit) = %f arcsec" %
                     fwhm_arcsec)
        ###
        del result
        if len(self.sampling_arcsec)==1 and \
               (len(self.cell_arcsec)==1 or self.kernel_type != "BOX"):
            fwhm_geo_arcsec = fwhm_arcsec
        else:
            pa = None
            if len(self.sampling_arcsec) > 1:
                sampling = self.get_box_kernel(axis, self.sampling_arcsec[1])
            elif self.kernel_type == "BOX" and len(self.cell_arcsec) > 1:
                kernel = self.get_box_kernel(axis,self.cell_arcsec[1])
                gridded = scipy.signal.convolve(beam,kernel,mode='same')
                gridded /= max(gridded)
            result = scipy.signal.convolve(gridded,sampling,mode='same')
            result /= max(result)
            #fwhm1 = findFWHM(axis,result)
            fwhm1, dummy = self.gaussfit(axis, result, minlevel=0.0,truncate=False)
            fwhm_geo_arcsec = np.sqrt(fwhm_arcsec*fwhm1)
            ### DEBUG MESSAGE
            casalog.post("The second axis")
            casalog.post("- FWHM of gridding kernel = %f arcsec" %
                         findFWHM(axis,kernel))
            casalog.post("- FWHM of theoretical beam = %f arcsec" %
                         findFWHM(axis,result))
            casalog.post("- FWHM of theoretical beam (gauss fit) = %f arcsec" %
                         fwhm1)
            del result
        # clear-up axes
        del axis, beam, kernel, sampling, gridded

        return dict(major="%farcsec" % (fwhm_geo_arcsec),
                    minor="%farcsec" % (fwhm_geo_arcsec),
                    pa=self.pa)

    def __assert_antenna(self):
        """Raise an error if antenna information is not set"""
        if not self.is_antenna_set: raise RuntimeError, "Antenna is not set"

    def __assert_kernel(self):
        """Raise an error if imaging parameters are not set"""
        if not self.is_kernel_set: raise RuntimeError, "Kernel is not set."

    def __assert_sampling(self):
        """Raise an error if sampling information is not set"""
        if not self.is_sampling_set:
            raise RuntimeError, "Sampling information is not set"

    def get_antenna_beam(self):
        """
        Returns arrays of antenna beam response and it's horizontal axis
        Picked from AnalysisUtils (revision 1.2204, 2015/02/18)
        """
        self.__assert_antenna()
        self.__assert_kernel()

        fwhm_arcsec = primaryBeamArcsec(frequency=self.ref_freq,
                                        diameter=self.antenna_diam_m,
                                        taper=self.taper, showEquation=True,
                                        obscuration=self.antenna_block_m,
                                        fwhmfactor=None)
        truncate = False
        convolutionPixelSize = 0.02 #arcsec
        # avoid too coarse convolution array w.r.t. sampling
        if self.is_sampling_set and \
               min(self.sampling_arcsec) < 5*convolutionPixelSize:
            convolutionPixelSize = min(self.sampling_arcsec) / 5.0
        # avoid too fine convolution arrays.
        sizes = list(self.cell_arcsec)+[fwhm_arcsec]
        if self.is_sampling_set: sizes += list(self.sampling_arcsec)
        minsize = min(sizes)
        support_min = 1000.
        support_fwhm = 5000.
        if minsize > support_min*convolutionPixelSize:
            convolutionPixelSize = minsize/support_min
        if fwhm_arcsec > support_fwhm*convolutionPixelSize:
            convolutionPixelSize = min(fwhm_arcsec/support_fwhm,minsize/10.)
        
        if (self.taper < 0.1):
            # Airly disk
            return self.buildAiryDisk(fwhm_arcsec, 3., convolutionPixelSize,
                                      truncate=truncate,
                                      obscuration=self.antenna_block_m,
                                      diameter=self.antenna_diam_m)
        else:
            # Gaussian beam
            myxaxis = np.arange(-3*fwhm_arcsec,
                                3*fwhm_arcsec+0.5*convolutionPixelSize,
                                convolutionPixelSize)
            myfunction = self.gauss(myxaxis,[fwhm_arcsec,truncate])
            return myxaxis, myfunction

    def get_kernel(self, axis):
        """Returns imaging kernel array"""
        self.__assert_kernel()
        if self.kernel_type == "SF":
            ### TODO: what to do for cell[0]!=cell[1]???
            return self.get_sf_kernel(axis,self.kernel_param['convsupport'],
                                      self.cell_arcsec[0])
        elif self.kernel_type == "GJINC":
            return self.get_gjinc_kernel(axis,self.kernel_param['truncate'],
                                         self.kernel_param['gwidth'],
                                         self.kernel_param['jwidth'],
                                         self.cell_arcsec[0])
        elif self.kernel_type == "GAUSS":
            return self.get_gauss_kernel(axis,self.kernel_param['truncate'],
                                         self.kernel_param['gwidth'],
                                         self.cell_arcsec[0])
        elif self.kernel_type == "BOX":
            return self.get_box_kernel(axis,self.kernel_param['width'])
        elif self.kernel_type == "PB":
            diam = self.antenna_diam_m
            if self.kernel_param['alma']:
                diam = 10.7
                casalog.post("Using effective antenna diameter %fm for %s kernel of ALMA antennas" % (diam,self.kernel_type))
            epsilon = self.antenna_block_m/diam
            return self.get_pb_kernel(axis,diam,self.ref_freq, epsilon=epsilon)
            #return (self.rootAiryIntensity(axis, epsilon))**2
        else:
            raise RuntimeError, "Invalid kernel: %s" % self.kernel_type

    def summary(self):
        """Print summary of parameters set to the class"""
        casalog.post("="*40)
        casalog.post("Summary of Image Beam Parameters")
        casalog.post("="*40)
        casalog.post("[Antenna]")
        self.__antenna_summary()

        casalog.post("\n[Imaging Parameters]")
        self.__kernel_summary()

        casalog.post("\n[Sampling]")
        self.__sampling_summary()

    def __notset_message(self):
        casalog.post("Not set.")

    def __antenna_summary(self):
        """Print summary of antenna setup"""
        if not self.is_antenna_set:
            self.__notset_message()
            return
        casalog.post("diameter: %f m" % (self.antenna_diam_m))
        casalog.post("blockage: %f m" % (self.antenna_block_m))

    def __kernel_summary(self):
        """Print summary of imaging parameter setup"""
        if not self.is_kernel_set:
            self.__notset_message()
            return
        casalog.post("reference frequency: %s" % str(self.ref_freq))
        casalog.post("cell size: %s arcsec" % str(self.cell_arcsec))
        casalog.post("kernel type: %s" % self.kernel_type)
        for key, val in self.kernel_param.items():
            casalog.post("%s: %s" % (key, str(val)))

    def __sampling_summary(self):
        """Print summary of sampling setup"""
        if not self.is_sampling_set:
            self.__notset_message()
            return
        casalog.post("sampling interval: %s arcsec" % str(self.sampling_arcsec))
        casalog.post("position angle: %s" % (self.pa))


    ##############################
    ### Construct Kernel arrays
    ##############################
    #### BOX ###
    def get_box_kernel(self, axis, width):
        """
        Returns a box kernel array with specified width.
        axis: an array of xaxis values
        width: kernel width
        output array
            out[i] = 1.0 (-width/2.0 <= x[i] <= width/2.0)
                   = 0.0 (else)
        """
        data = np.zeros(len(axis))
        indices = np.where(abs(axis) <= width/2.0)
        data[indices] = 1.0
        return data

    ### SF ###
    def get_sf_kernel(self, axis, convsupport, cell_size):
        """
        Returns spheroidal kernel array
        axis: an array of xaxis values
        convsupport: the truncation radius of SF kernel in pixel unit.
        cell_size: image pixel size

        Modified version of one in AnalysisUtils.sfBeamPredict (revision 1.2204, 2015/02/18)
        """
        convsupport = 3 if convsupport == -1 else convsupport
        supportwidth = (convsupport*1.0 + 0.0)
        c = 5.356*np.pi/2.0 # value obtained by matching Fred's grdsf.f output with scipy(m=0,n=0)
        sfaxis = axis/float(supportwidth*cell_size*1.0)
        indices = np.where(abs(sfaxis)<1)[0]
        centralRegion = sfaxis[indices]
        centralRegionY = self.spheroidalWaveFunction(centralRegion, 0, 0, c, 1)
        mysf = np.zeros(len(axis))
        mysf[indices] += centralRegionY/max(centralRegionY)
        return mysf

    def spheroidalWaveFunction(self, x, m=0, n=0, c=0, alpha=0):
        """
        Returns spheroidal wave function
        Migrated from AnalysisUtils (revision 1.2204, 2015/02/18)
        """
        if (type(x) != list and type(x) != np.ndarray):
            returnScalar = True
            x = [x]
        else:
            returnScalar = False
        cv = scipy.special.pro_cv(m,n,c)  # get the eigenvalue
        result = scipy.special.pro_ang1_cv(m,n,c,cv,x)[0]
        for i in range(len(x)):
            nu = x[i] # (i-0.5*len(x))/(0.5*len(x))  # only true if x is symmetric about zero
            result[i] *= (1-nu**2)**alpha
        # The peak of this function is about 10000 for m=0,n=0,c=6
        if (returnScalar):
            return result[0]
        else:
            return result
    
    ### GAUSS ###
    def get_gauss_kernel(self, axis, truncate, gwidth, cell_arcsec):
        """
        Returns a gaussian kernel array
        axis : an array of xaxis values
        truncate : truncation radius
            NOTE definition is different from truncate in gauss()!
        gwidth : kernel gwidth
        cell_arcsec : image pixel size in unit of arcsec
        """
        if gwidth == -1:
            gwidth = np.sqrt(np.log(2.0))
        gwidth_arcsec = self.__parse_width(gwidth, cell_arcsec)
        # get gauss for full axis
        result = self.gauss(axis,[gwidth_arcsec])
        # truncate kernel outside the truncation radius
        if truncate == -1:
            trunc_arcsec = gwidth_arcsec*1.5
        elif truncate is not None:
            trunc_arcsec = self.__parse_width(truncate, cell_arcsec)
        idx = np.where(abs(axis)>trunc_arcsec)
        result[idx] = 0.
        return result

    def gauss(self, x, parameters):
        """
        Computes the value of the Gaussian function at the specified
        location(s) with respect to the peak (which is assumed to be at x=0).
        truncate: if not None, then set result to zero if below this value.
        -Todd Hunter
        Migrated from AnalysisUtils (revision 1.2204, 2015/02/18)
        """
        if (type(parameters) != np.ndarray and type(parameters) != list):
            parameters = np.array([parameters])
        if (len(parameters) < 2):
            parameters = np.array([parameters[0],0])
        fwhm = parameters[0]
        x = np.asarray(x, dtype=np.float64)
        sigma = fwhm/2.3548201
        result = np.exp(-(x**2/(2.0*sigma**2)))
        idx = np.where(result < parameters[1])[0]
        result[idx] = 0
        return result

    ### GJINC ###
    def get_gjinc_kernel(self, axis, truncate, gwidth, jwidth, cell_arcsec):
        """
        Returns a GJinc kernel array
        axis : an array of xaxis values
        truncate : truncation radius (NOT SUPPORTED YET)
        gwidth jwidth : kernel gwidth
        cell_arcsec : image pixel size in unit of arcsec
        """
        if gwidth == -1:
            gwidth = 2.52*np.sqrt(np.log(2.0))
        if jwidth == -1:
            jwidth = 1.55
        gwidth_arcsec = self.__parse_width(gwidth, cell_arcsec)
        jwidth_arcsec = self.__parse_width(jwidth, cell_arcsec)
        mygjinc = self.trunc(self.gjinc(axis, gwidth=gwidth_arcsec,
                                        jwidth=jwidth_arcsec,
                                        useCasaJinc=True, normalize=False))
        return mygjinc

    def gjinc(self, x, gwidth, jwidth, useCasaJinc=False, normalize=False):
        """
        Migrated from AnalysisUtils (revision 1.2204, 2015/02/18)
        """
        if (useCasaJinc):
            result = self.grdjinc1(x,jwidth,normalize) * self.gjincGauss(x, gwidth)
        else:
            result = self.jinc(x,jwidth) * self.gjincGauss(x, gwidth)
        return result

    def grdjinc1(self, val, c, normalize=True):
        """
        Migrated from AnalysisUtils (revision 1.2204, 2015/02/18)
        """
        # Casa's function
        #// Calculate J_1(x) using approximate formula
        xs = np.pi * val / c
        result = []
        for x in xs:
          x = abs(x)  # I added this to make it symmetric
          ax = abs(x)
          if (ax < 8.0 ):
            y = x * x
            ans1 = x * (72362614232.0 + y * (-7895059235.0 \
                       + y * (242396853.1 + y * (-2972611.439 \
                       + y * (15704.48260 + y * (-30.16036606))))))
            ans2 = 144725228442.0 + y * (2300535178.0 \
                       + y * (18583304.74 + y * (99447.43394 \
                       + y * (376.9991397 + y * 1.0))))
            ans = ans1 / ans2
          else:
            z = 8.0 / ax
            y = z * z
            xx = ax - 2.356194491
            ans1 = 1.0 + y * (0.183105e-2 + y * (-0.3516396496e-4 \
                      + y * (0.2457520174e-5 + y * (-0.240337019e-6))))
            ans2 = 0.04687499995 + y * (-0.2002690873e-3 \
                      + y * (0.8449199096e-5 + y * (-0.88228987e-6  \
                      + y * (0.105787412e-6))))
            ans = sqrt(0.636619772 / ax) * (cos(xx) * ans1 - z * sin(xx) * ans2)
          if (x < 0.0):
            ans = -ans
          if (x == 0.0):
            out = 0.5
          else:
            out = ans / x
          if (normalize):
            out = out / 0.5
          result.append(out)
        return(result)

    def jinc(self, x, jwidth):
        """
        The peak of this function is 0.5.
        Migrated from AnalysisUtils (revision 1.2204, 2015/02/18)
        """
        argument = np.pi*np.abs(x)/jwidth
        np.seterr(invalid='ignore') # prevent warning for central point
        result = scipy.special.j1(argument) / argument 
        np.seterr(invalid='warn')
        for i in range(len(x)):
            if (abs(x[i]) < 1e-8):
                result[i] = 0.5
        return result

    def gjincGauss(self, x, gwidth):
        return (np.exp(-np.log(2)*(x/float(gwidth))**2))  

    ### Airly disk ###
    def get_pb_kernel(self, axis,diam,ref_freq, epsilon=0.0):
        """
        Return Airy Disk array defined by the axis, diameter, reference frequency
        and ratio of central hole and antenna diameter
        
        axis: x-axis values
        diameter: antenna diameter in unit of m
        reference frequency: the reference frequency of the image
        epsilon: ratio of central hole diameter to antenna diameter
        """
        a = (self.rootAiryIntensity(axis, epsilon))**2
        airyfwhm = findFWHM(axis,a)
        fwhm = primaryBeamArcsec(frequency=ref_freq, diameter=diam,
                                 taper=self.taper, showEquation=False,
                                 obscuration=diam*epsilon,
                                 fwhmfactor=None)
        ratio = fwhm/airyfwhm
        tempaxis = axis/ratio
        a = self.rootAiryIntensity(tempaxis, epsilon)
        return a**2        

    def buildAiryDisk(self, fwhm, xaxisLimitInUnitsOfFwhm, convolutionPixelSize,
                      truncate=False, obscuration=0.75,diameter=12.0):
        """
        This function computes the Airy disk (with peak of 1.0) across a grid of points
        specified in units of the FWHM of the disk.
        fwhm: a value in arcsec
        xaxisLimitInUnitsOfFwhm: an integer or floating point unitless value
        truncate: if True, truncate the function at the first null (on both sides)
        obscuration: central hole diameter (meters)
        diameter: dish diameter (meters)
          obscuration and diameter are used to compute the blockage ratio (epsilon)
          and its effect on the pattern
        Migrated from AnalysisUtils (revision 1.2204, 2015/02/18)
        """
        epsilon = obscuration/diameter
#        print "Using epsilon = %f" % (epsilon)
        myxaxis = np.arange(-xaxisLimitInUnitsOfFwhm*fwhm,
                            xaxisLimitInUnitsOfFwhm*fwhm+0.5*convolutionPixelSize,
                            convolutionPixelSize)
        a = (self.rootAiryIntensity(myxaxis, epsilon))**2
        # Scale the Airy disk to the desired FWHM, and recompute on finer grid
        airyfwhm = findFWHM(myxaxis,a)
        ratio = fwhm/airyfwhm
        myxaxis = np.arange(-xaxisLimitInUnitsOfFwhm*fwhm/ratio,
                            (xaxisLimitInUnitsOfFwhm*fwhm+0.5*convolutionPixelSize)/ratio,
                            convolutionPixelSize/ratio)
        a = self.rootAiryIntensity(myxaxis, epsilon)
        if (truncate):
            a = self.trunc(a)
        a = a**2
        myxaxis *= ratio
        return(myxaxis, a)

    def rootAiryIntensity(self, myxaxis, epsilon=0.0, showplot=False):
        """
        This function computes 2*J1(x)/x, which can be squared to get an Airy disk.
        myxaxis: the x-axis values to use
        epsilon: radius of central hole in units of the dish diameter
        Migrated from AnalysisUtils.py (revision 1.2204, 2015/02/18)
        """
        if (epsilon > 0):
            a = (2*spspec.j1(myxaxis)/myxaxis - \
                 epsilon**2*2*spspec.j1(myxaxis*epsilon)/(epsilon*myxaxis)) / (1-epsilon**2)
        else:
            a = 2*spspec.j1(myxaxis)/myxaxis  # simpler formula for epsilon=0
        return(a)
    
    def trunc(self, result):
        """
        Truncates a list at the first null on both sides of the center,
        starting at the center and moving outward in each direction.
        Assumes the list is positive in the center, e.g. a Gaussian beam.
        -Todd Hunter
        Migrated from AnalysisUtils (revision 1.2204, 2015/02/18)
        """
        # casa default truncate=-1, which means truncate at radius of first null
        mask = np.zeros(len(result))
        truncateBefore = 0
        truncateBeyond = len(mask)
        for r in range(len(result)/2,len(result)):
            if (result[r]<0):
                truncateBeyond = r
                break
        for r in range(len(result)/2,0,-1):
            if (result[r]<0):
                truncateBefore = r
                break
        mask[truncateBefore:truncateBeyond] = 1
#        print "Truncating outside of pixels %d-%d (len=%d)" % (truncateBefore,truncateBeyond-1,len(mask))
        result *= mask
        return result

    def gaussfit_errfunc(self,parameters,x,y):
        """Migrated from AnalysisUtils (revision 1.2204, 2015/02/18)"""
        return (y - self.gauss(x,parameters))

    def gaussfit(self, x, y, showplot=False, minlevel=0, verbose=False, 
                 title=None, truncate=False):
        """
        Fits a 1D Gaussian assumed to be centered at x=0 with amp=1 to the 
        specified data, with an option to truncate it at some level.   
        Returns the FWHM and truncation point.
        Migrated from AnalysisUtils (revision 1.2204, 2015/02/18)
        """
        fwhm_guess = findFWHM(x,y)
        if (truncate == False):
            parameters = np.asarray([fwhm_guess], dtype=np.float64)
        else:
            parameters = np.asarray([fwhm_guess,truncate], dtype=np.float64)
        if (verbose): print "Fitting for %d parameters: guesses = %s" % (len(parameters), parameters)
        xx = np.asarray(x, dtype=np.float64)
        yy = np.asarray(y, dtype=np.float64)
        lenx = len(x)
        if (minlevel > 0):
            xwidth = findFWHM(x,y,minlevel)
            xx = x[np.where(np.abs(x) < xwidth*0.5)[0]]
            yy = y[np.where(np.abs(x) < xwidth*0.5)[0]]
            if (verbose):
                print "Keeping %d/%d points, guess = %f arcsec" % (len(x),lenx,fwhm_guess)
        result = optimize.leastsq(self.gaussfit_errfunc, parameters, args=(xx,yy),
                                  full_output=1)
        bestParameters = result[0]
        infodict = result[2]
        numberFunctionCalls = infodict['nfev']
        mesg = result[3]
        ier = result[4]
        if (verbose):
            print "optimize.leastsq: ier=%d, #calls=%d, message = %s" % (ier,numberFunctionCalls,mesg)
        if (type(bestParameters) == list or type(bestParameters) == np.ndarray):
            fwhm = bestParameters[0]
            if verbose: print "fitted FWHM = %f" % (fwhm)
            if (truncate != False):
                truncate = bestParameters[1]
                print "optimized truncation = %f" % (truncate)
        else:
            fwhm = bestParameters
        return(fwhm,truncate)
    
def findFWHM(x,y,level=0.5, s=0):
    """
    Measures the FWHM of the specified profile.  This works
    well in a noise-free environment.  The data are assumed to
    be sorted by the x variable.
    x: the position variable
    y: the intensity variable
    level: the signal level for which to find the full width
    s: see help scipy.interpolate.UnivariateSpline
    -Todd Hunter
    Migrated from AnalysisUtils (revision 1.2204, 2015/02/18)
    """
    halfmax = np.max(y)*level
    spline = scipy.interpolate.UnivariateSpline(x, y-halfmax, s=s)
    result = spline.roots()
    if (len(result) == 2):
        x0,x1 = result
        return(abs(x1-x0))
    elif (len(result) == 1):
        return(2*result[0])
    else:
        ### modified (KS 2015/02/19)
        #print "More than two crossings (%d), fitting slope to points near that power level." % (len(result))
        #result = 2*findZeroCrossingBySlope(x, y-halfmax)
        #return(result)
        errmsg = "Unsupported FWHM search in CASA. More than two corssings (%d) at level %f (%f \% of peak)." % (len(result), halfmax, level)
        raise Exception, errmsg

def primaryBeamArcsec(frequency, diameter, obscuration, taper, 
                      showEquation=True, use2007formula=True, fwhmfactor=None):
    """
    Implements the Baars formula: b*lambda / D.
      if use2007formula==False, use the formula from ALMA Memo 456        
      if use2007formula==True, use the formula from Baars 2007 book
        (see au.baarsTaperFactor)     
      In either case, the taper value is expected to be entered as positive.
        Note: if a negative value is entered, it is converted to positive.
    The effect of the central obstruction on the pattern is also accounted for
    by using a spline fit to Table 10.1 of Schroeder's Astronomical Optics.
    fwhmfactor: if given, then ignore the taper
    For further help and examples, see https://safe.nrao.edu/wiki/bin/view/ALMA/PrimaryBeamArcsec
    -- Todd Hunter
    Simplified version of the one in AnalysisUtils (revision 1.2204, 2015/02/18)
    """
    if (fwhmfactor != None):
        taper = effectiveTaper(fwhmfactor,diameter,obscuration,use2007formula)
        if (taper == None): return
    if (taper < 0):
        taper = abs(taper)
    if (obscuration>0.4*diameter):
        print "This central obscuration is too large for the method of calculation employed here."
        return
    if (type(frequency) == str):
        my_qa = qatool()
        frequency = my_qa.getvalue(my_qa.convert(frequency, "Hz"))[0]
    lambdaMeters = 2.99792458e8/frequency
    b = baarsTaperFactor(taper,use2007formula) * centralObstructionFactor(diameter, obscuration)
    if (showEquation):
        if (use2007formula):
            formula = "Baars (2007) Eq 4.13"
        else:
            formula = "ALMA memo 456 Eq. 18"
        casalog.post("Coefficient from %s for a -%.1fdB edge taper and obscuration ratio=%g/%g = %.3f*lambda/D" % (formula, taper, obscuration, diameter, b))
    return(b*lambdaMeters*3600*180/(diameter*np.pi))

def effectiveTaper(fwhmFactor=1.16, diameter=12, obscuration=0.75, 
                   use2007formula=True):
    """
    The inverse of (Baars formula multiplied by the central
    obstruction factor).  Converts an observed value of the constant X in
    the formula FWHM=X*lambda/D into a taper in dB (positive value).
    if use2007formula == False, use Equation 18 from ALMA Memo 456     
    if use2007formula == True, use Equation 4.13 from Baars 2007 book
    -- Todd Hunter
    Migrated from AnalysisUtils (revision 1.2204, 2015/02/18)
    """
    cOF = centralObstructionFactor(diameter, obscuration)
    if (fwhmFactor < 1.02 or fwhmFactor > 1.22):
        print "Invalid fwhmFactor (1.02<fwhmFactor<1.22)"
        return
    if (baarsTaperFactor(10,use2007formula)*cOF<fwhmFactor):
        increment = 0.01
        for taper_dB in np.arange(10,10+increment*1000,increment):
            if (baarsTaperFactor(taper_dB,use2007formula)*cOF-fwhmFactor>0): break
    else:
        increment = -0.01
        for taper_dB in np.arange(10,10+increment*1000,increment):
            if (baarsTaperFactor(taper_dB,use2007formula)*cOF-fwhmFactor<0): break
    return(taper_dB)

def baarsTaperFactor(taper_dB, use2007formula=True):
    """
    Converts a taper in dB to the constant X
    in the formula FWHM=X*lambda/D for the parabolic illumination pattern.
    We assume that taper_dB comes in as a positive value.
    use2007formula:  False --> use Equation 18 from ALMA Memo 456.
                     True --> use Equation 4.13 from Baars 2007 book
    - Todd Hunter
    Migrated from AnalysisUtils (revision 1.2204, 2015/02/18)
    """
    tau = 10**(-0.05*taper_dB)
    if (use2007formula):
        return(1.269 - 0.566*tau + 0.534*(tau**2) - 0.208*(tau**3))
    else:
        return(1.243 - 0.343*tau + 0.12*(tau**2))

def centralObstructionFactor(diameter=12.0, obscuration=0.75):
    """
    Computes the scale factor of an Airy pattern as a function of the
    central obscuration, using Table 10.1 of Schroeder's 'Astronomical Optics'.
    -- Todd Hunter
    Migrated from AnalysisUtils (revision 1.2204, 2015/02/18)
    """
    epsilon = obscuration/diameter
    myspline = scipy.interpolate.UnivariateSpline([0,0.1,0.2,0.33,0.4], [1.22,1.205,1.167,1.098,1.058], s=0)
    factor = myspline(epsilon)/1.22
    if (type(factor) == np.float64):
        # casapy 4.2
        return(factor)
    else:
        # casapy 4.1 and earlier
        return(factor[0])

