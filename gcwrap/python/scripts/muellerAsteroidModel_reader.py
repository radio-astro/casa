# return flux density of thermo-physical models for asteroids
# -- read a tabulated model flux density by Mueller for asteroids and
#  flux for a frequency scaled from the nearest value from the table
import os
import numpy as np

class asteroid_TPM:
    # objectname: capitalized name
    # frequency in GHz 
    def __init__(self, objectname, frequency, mjd, path='',casalog=None):
        if not casalog:
           casalog = casac.logsink()
        self._casalog = casalog
        self.knownsrcdict={'Ceres':{'interval':'1hour'},
                           'Lutetia':{'interval':'15min'},
                           'Pallas':{'interval':'15min'},
                           'Vesta':{'interval':'15min'}}
        self.src=objectname
        self.setfreq(frequency)
        self.setepoch(mjd)
        if path =="":
            self.setdatapath(os.environ['CASAPATH'].split()[0]+'/data/alma/SolarSystemModels/TPM')
        else:
            self.setdatapath(path)
        self.openedFile=""
        self.data=None

    def setfreq(self,frequency):
        if type(frequency)!=float and type(frequency)!=np.float32 and type(frequency)!=np.float64:
            raise IOError("frequency must be float (in GHz)")
        self.freq=frequency

    def setepoch(self,mjd):
        if type(mjd)!=float:
            raise IOError("mjd must be float (in MJD)")
        self.mjd=mjd

    def setdatapath(self,path):
        if os.path.exists(path):
            if path[-1]!='/':
                path+='/'
            self.datapath=path
        else:
            raise IOError("Model data path:"+path+" does not exists")
        self._casalog.post("Model data path: "+path)


    def getFluxFromMuellerModel(self):
    
        (reffreq, refflux) =  self._muellerModelDataReader()
        #print "scaling from flux density =",refflux,' Jy @ ',reffreq,'GHz'
        return (refflux*(self.freq/reffreq)**2)


    def _muellerModelDataReader(self):
        """
        read flux density from the asteroid models by
        Mueller
        """
        from casac import casac 
        qa = casac.quanta()

        reffreq=None
        refflux=None

        date=qa.time(str(self.mjd)+'d',form='ymd')[0]
        yr=date.split('/')[0] 
       
        # ------------------------------ 
        # Following section should only to read the data initially and keep the data and only scan the new model data file 
        # when the mjd is outside of the current data...
        # 
        #if int(yr) < 2015: yr = '2015'
        if self.src in self.knownsrcdict.keys():
            filename = self.datapath+self.src+"_ALMA_TPMprediction_"+yr+"_"+self.knownsrcdict[self.src]['interval']+".txt"
            if (self.openedFile != filename):
                # read a new file
                self.openedFile = filename
                import glob
                flist = glob.glob(filename)
                if len(flist)==1:
                    self._casalog.post("Reading model data: "+flist[0])
                    with open(flist[0],"r") as f:
                        self.data = f.readlines()
                elif len(flist)==0:
                    raise IOError("Cannot find the model file for "+self.src+" for date="+date+".")
           # else:
           #     self._casalog.post("use the file already read..self.openedFile="+self.openedFile)

            freqs = self._checkheader(self.data)
            if len(freqs)==0:
                raise IOError("Problem in reading model data file!")

            i = np.abs(freqs-self.freq).argmin()
            reffreq = freqs[i]
            #self._casalog.post("self.freq= %s i=%s reffreq=%s" % (self.freq, i, reffreq))
            #print "self.freq= %s i=%s reffreq=%s" % (self.freq, i, reffreq)
            
            maindata = []
            mjds = []
            fluxes = []
            for line in self.data[8:]:
                if line.find('----')<0:
                    maindata.append(line)
                    mjds.append(float(line.split()[5]))
                    # i th flux (0 based) 
                    fluxes.append(float(line.split()[(i+1)+5]))

            mjdarr = np.array(mjds)
            flxarr = np.array(fluxes)
            # ---------------------

            # find flux density of nearest in time
            t = np.abs(mjdarr-self.mjd).argmin()
            refflux = flxarr[t] 
            #print "flxarr=",flxarr
            #print "freq, mjd=",self.freq,self.mjd
            #print "refflux@reffreq=%s@%s at %s" % (refflux,reffreq,mjdarr[t])
            #self._casalog.post("refflux@reffreq=%s@%s at %s" % (refflux,reffreq,mjdarr[t]),"DEBUG1")
       
        return (reffreq, refflux)


    def _checkheader(self,data):
        """
        check the header information for the model data file
    
        header/data example:
        Ceres_ALMA_TPMprediction_2015_1hour.out             ../DELIVERY20141028/Ceres_horizons_2015_1hour.txt
        --------------------------------------------------------------------------------------------------------------------
        predictions at [micron]           9993.1    3747.4    2606.9    1998.6    1499.0    1303.4    1153.0     999.3  ...  
        corresponds to [GHz]                30.0      80.0     115.0     150.0     200.0     230.0     260.0     300.0  ... 
        --------------------------------------------------------------------------------------------------------------------
        YYYY MM DD HH MI  MJD         [Jy] flx01     flx02     flx03     flx04     flx05     flx06     flx07     flx08  ... 
        --------------------------------------------------------------------------------------------------------------------

        2015  1  1  0  0  57023.00000     0.0106    0.0748    0.1538    0.2605    0.4608    0.6080    0.7741    1.0261  ... 
        2015  1  1  1  0  57023.04167     0.0106    0.0748    0.1538    0.2605    0.4608    0.6080    0.7741    1.0260  ...

        """
        import numpy as np
        # first seven lines are expected be headers follow by a blank line
	# file info
	header1 = data[0]
	# wavelength (micron)
	header2 = data[2]
	# frequency (GHz)
	header3 = data[3] 
	# flux column labels
	header4 = data[5] 

	# freq list
	if header3.find("corresponds to [GHz]")>=0:
	    freqv = []
	    for freq in header3.split(): 
		try:
		    freqv.append(float(freq))
		except:
		    pass
	    if len(freqv) == 0:
		freqs = np.array([0.0])
		raise IOError("Frequency data was not detected")
	    else:
		freqs = np.array(freqv)
	if header4.find("YYYY")<0:
	    if header4.split()[5] != 'MJD':
		print "The header info (column names) seem to be different from expected format (i.e. YYYY MM DD HH MI MJD flx01 ...)" 
	     
	return freqs       
