class par(str):

        @staticmethod
        def accumtime():
                """ Cumulative table timescale when creating from scratch. This is the sampling
                interval upon which to accumulate *if* an input calibration table (tablein)
                has not been specified. [units=seconds] """

	@staticmethod
	def alg():
		""" 
    		alg -- Algorithm to use:
            		default: 'csclean'; example: alg='clark'; 
            		Options: 'clark','hogbom','csclean'
            		csclean is Cotton-Schwab and cleans entire image
            		hogbom and clark clean inner quarter of image

		"""

	@staticmethod
	def algorithm():
		"""
	        algorithm -- autoflag algorithm name
                default: 'timemed'
                options: 'timemed','freqmed','uvbin','sprej'

                column -- the column on which to operate (DATA, CORRECTED, MODEL)
                expr -- expression to use
                        default: 'ABS I'; example: expr='RE XX'
                        Options: 'ABS','ARG','RE','IM','NORM' + ' ' +  'I','XX','YY','RR','LL'

                thr -- flagging threshold as a multiple of standard-deviation ( n sigma )
                window -- half width for sliding window median filters
		"""

	@staticmethod
	def antenna():
		""" 
                antenna -- Select data by antenna/baseline
                default: 0 for sdimaging, '' (means all) for the other tasks
                Non-negative integers are assumed to be antenna indices, and
                anything else is taken as an antenna name.

                Examples:
                antenna='5&6': baseline between antenna index 5 and index 6.
                antenna='VA05&VA06': baseline between VLA antenna 5 and 6.
                antenna='5&6:7&8': baselines 5-6 and 7-8
                antenna='5': all baselines with antenna 5
                antenna='5,6,10': all baselines including antennas 5, 6, or 10
                antenna='5,6,10&': all baselines with *only* antennas 5, 6, or 10
                                   (cross-correlations only.  Use && to include
                                   autocorrelations, and &&& to get only
                                   autocorrelations.)
                antenna='!ea03,ea12,ea17': all baselines except those that include
                                           EVLA antennas ea03, ea12, or ea17
		"""

	@staticmethod
	def antennalist():
		"""
    		antennalist  -- antenna position ascii file (each row has x y z diam) 
		"""

	@staticmethod
	def antnamescheme():
		"""
      		antnamescheme -- 'old' or 'new' antenna names.
                    default = 'new' gives antnenna names
                       'VA04' for VLA telescope 4 or
                       'EA13' for EVLA telescope 13.
                    'old' gives names '4' and '13'
		"""

	@staticmethod
	def append():
		""" Append calibration solutions to an existing calibration table. 

		Options: True, False

		Fluxscale:
       		append -- Append solutions to the table.
               	default: False; (will overwrite if already existing)
               	example: append=True

		"""

	@staticmethod
	def asdm():
		""" Input ALMA Science Data Model observation. """

	@staticmethod
	def async():
		""" Run task in a separate process (return CASA prompt)
		default: False; example: async=True """

	@staticmethod
	def autocorr():
		"""
        	autocorr -- Flag autocorrelations ?
                default: False
                options: True,False
	
		importvla:
		autocorr --  import autocorrelations to ms
                default = False (no autocorrelations)

		"""

	@staticmethod
	def average():
		"""
		average -- averaging on spectral data
		default: False
		options: True, False
		
		"""

	@staticmethod
	def averageall():
		"""
		averageall -- average multi-resolution spectra
		              spectra are averaged by referring
			      their frequency coverage
	        default: False
		options: True, False
		"""

	@staticmethod
	def avg_limit():
		"""
		avg_limit -- channel averaging for broad lines
		default: 4
		example: a number of consecutive channels not greater than
		         this parameter can be averaged to search for broad lines
	        """
		
	@staticmethod
	def axis():
		"""
        	axis -- The moment axis (0-based)
                default: 3 (spectral axis); example: axis=3
		"""

	@staticmethod
	def bandname():
		""" VLA frequency band. 

		Options: 	'4' = 48 -96 MHz
				'P' = 298 - 345 MHz
				'L' = 1.15 - 1.75 GHz
				'C' = 4.2 - 5.1 GHz
				'X' = 6.8 - 9.6 GHz
				'U' = 13.5 - 16.3 GHz
				'K' = 20.8 - 25.8 GHz
				'Q' = 38 -51 GHz
		
		"""

	@staticmethod
	def bandtype():
		""" Type of bandpass solution ('B', 'BPOLY'). """

	@staticmethod
	def baseline():
		""" 
                baseline -- Baseline index identifiers
                default: [-1] (all); example: baseline=[0,6,11]
		"""

	@staticmethod
	def beamsize():
		"""
		beamsize -- beam size
		default: 0.0
		example: 10.0 (interpreted as '10 arcsec'), '1arcmin'
		"""
		
	@staticmethod
	def bitpix():
		"""
        	bitpix -- Bits per pixel
                default: -32 (floating point)
                <Options: -32 (floating point), 16 (integer)>
		"""

	@staticmethod
	def blmode():
		"""
		blmode -- mode for baseline fitting
		default: 'auto' for sdbaseline, 'none' for sdcal
		options: 'auto', 'list', 'interact', 'none'(for sdcal)
		example: blmode='auto' uses expandable parameters
		in addition to blpoly to run linefinder
		to determine line-free regions
		USE WITH CARE! May need to tweak the parameters,
		thresh, avg_limit, and edge.
		blmode='interact' allows adding and deleting mask.
		regions by drawing rectangles on the plot with mouse.
		Draw a rectangle with LEFT-mouse to ADD the region to
		the mask and with RIGHT-mouse to DELETE the region. 
		"""

	@staticmethod
	def blpoly():
		"""
		blpoly -- order of baseline polynomial
		options: (int) (<0 turns off baseline fitting)
		default: 5 for sdbaseline/sdcal, 1 for sdtpimaging
		example: typically in range 2-9 (higher values
		         seem to be needed for GBT)
		"""
		
	@staticmethod
	def box():
		"""
		box -- 	A box region in the directional portion of an image.
		The directional portion of an image are the axes for right
		ascension and declination, for example.  Boxes are specified
		by there bottom-left corner (blc) and top-right corner (trc)
		as follows: blcx, blcy, trcx, trcy;
		ONLY pixel values acceptable at this time.
		Default: none (all);
		Example: box='0,0,50,50'
		Example: box='[10,20,30,40];[100,100,150,150]'
		"""

	@staticmethod
	def box_size():
		"""
		box_size -- running mean box size
		default: 0.2
		example: a running mean box size specified as a fraction
		of the total spectrum length
		"""

	@staticmethod
	def bptable():
		""" Bandpass calibration solutions table: """

	@staticmethod
	def caldescid():
		""" Calibrator data description index identifier. A unique data description
		identifier describes a single combination of spectral window and polarization: """

	@staticmethod
	def calfield():
		""" List of field names in the incremental calibration table to use.

		calfield specifies the names to select from 'incrtable' when applying to
		'tablein'.

		See syntax in 'help par.selectdata' - same as for field parameter.

		"""

	@staticmethod
	def calmode():
		"""
		(for sdaverage, sdcal, sdtpimaging)
		calmode -- SD calibration mode
		options: 'ps', 'nod', 'fs', 'fsotf', 'quotient', 'none' (for sdaverage/sdcal)
			 'baseline', 'none' (for sdtpimaging)
	        default: 'none'
		example: choose 'none' if you have already calibrated
		
		-----------------------------------------------------------
		
		(for the other tasks)	 
		calmode -- Solutions to solve for when using the gaincal task.
		options: 'p', 'a', and 'ap'.
		"""


	@staticmethod
	def calselect():
		""" Optional subset of calibration data to select (e.g., field name): """

	@staticmethod
	def caltable():
		""" Output calibration table: """

	@staticmethod
	def calwt():
		""" Apply calibration to data weights in addition to the visibilities (boolean): """


	@staticmethod
	def cell():
		"""
		(for sdimaging, sdtpimaging)
		cell -- x and y cell size. default unit arcmin.
		default: ['1.0arcmin', '1.0arcmin']
		example: cell=['0.2arcmin', '0.2arcmin']
		         cell='0.2arcmin' (equivalent to example above)

	        -----------------------------------------------------

		(for simdata)
		cell -- output cell/pixel size
		default: '0.1arcsec'
	        example: 'incell'    #uses incell value for the output cell size
		
		"""

	@staticmethod
	def channelrange():
		"""
		channelrange -- channel range selection
		default: []    #use all channel
		example: [0, 5000]
		         Note that specified values are recognized as
			 'channel' regardless of the value of specunit
		"""
		
	@staticmethod
	def chans():
		""" Range of channels:
		uvcontsub - It takes a simple list of channels or anything that will
		produce such a list of integer channel values:
		examples chans=[0,1,2,3,55,56,57,58,59,60,61]
		        chans=range(0,4)+range(55,62)

		immath, imstat, and imcontsub - It takes a string listing of channel
		numbers, much like the spw paramter such as:
		examples chans="0,3,4,8"
		         chans="3~20;50,51"    #channels 3to20 and 50 and 51
			 chans="<10;>=55"      #chanls 0to9 and 55 and greater (inclusively)
		
		"""

	@staticmethod
	def chanwidth():
		"""
		chanwidth -- channel width
		default: '10MHz'
		"""
		
	@staticmethod
	def circ():
		"""
		circ -- A circle region in the directional portion of an image.
		The directional portion of an image are the axes for right
		ascension and declination, for example. Circles are specified
		by there center and radius as follows: cx, xy, r 
		ONLY pixel values acceptable at this time.
		Default: none (all);
		Example: circ='[10,10,5];[105,110,10]'  # defines 2 cirlces
		"""		

	@staticmethod
	def cleanbox():
		"""
        	cleanbox -- Cleaning region:
                default: [] defaults to inner quarter of image
                Two specification types:
                Explicit pixel ranges
                   example: cleanbox=[110,110,150,145]
                   cleans one regiong defined by the pixel ranges
                Filename with pixel values with ascii format:
                   <fieldindex blc-x blc-y trc-x trc-y> on each line
		"""
	@staticmethod
	def clearpanel():
		"""
		 clearpanel -- Don't clear any of the plots, clear any areas
		 affected by current plot, clear the current plotting area
		 only, or clear the whole plot panel.
		 options: None, Auto, Current, All
		 default: Auto
		 example: clearpanel='Current'
		"""

	@staticmethod
	def clip():
		"""
		clip -- flag data that are outside a specified range
                options: (bool)True,False
                default: False
		"""
		
	@staticmethod
	def clipcolumn():
		"""
        	clipcolumn -- Column to use for clipping.
                default: 'DATA'
                options: 'DATA','CORRECTED','MODEL'
		"""

	@staticmethod
	def clipcorr():
		""" Defines the correlation to clip.
		Options are: 'I','RR','LL','XX','LL'

		"""

	@staticmethod
	def clipexpr():
		"""
        	clipexpr -- Clip using the following: 
                default: 'ABS I'; example: clipexpr='RE XX'
                Options: 'ABS','ARG','RE','IM','NORM' + ' ' +  'I','XX','YY','RR','LL'
		"""
	
	@staticmethod
	def clipfunction():
		""" Defines the function used in evaluating data for clipping.
		Options are: 'ABS','ARG','RE','IM','NORM'

		"""
	
	@staticmethod	
	def clipminmax():
		"""
        	clipminmax -- Range of data (Jy) that will NOT be flagged
                default: [] means do not use clip option
                example: [0.0,1.5]
		
		"""

	@staticmethod
	def clipoutside():
		"""
        	clipoutside -- Clip OUTSIDE the range ?
                default: True
                example: False -> flag data WITHIN the range.
		"""

	@staticmethod
	def colormap():
		"""
		colormap -- the colours to be used for plot lines
		default: None
		example: colormap='green red black cyan magenta'  (HTML standard)
		         colormap='g r k c m'  (abbreviation)
			 colormap='#008000 #00FFFF #FF0090'  (RGB tuple)
		         The plotter will cycle through these colours
			 when lines are overlaid (stacking mode)
		"""
		
	@staticmethod	
	def combinespw():
		"""
        	combinespw -- Combine all spectral windows at one time
                default: True;
                   all spectral windows must have same shape.
                   successful aips transferral must have default=true
		"""

	@staticmethod
	def comment():
		"""
        	comment -- Short description of a versionname (used for mode='save')
                default: ''; example: comment='Clip above 1.85'
                comment = versionname
		"""

	@staticmethod	
	def complist():
		"""
       		complist -- Name of component list
               	default: None; ; example: complist='test.cl'
               	components tool not yet available

		"""

	@staticmethod
	def comptype():
		"""
        	comptype -- component model type
                default: 'P'; example: comptype='G'
                <Options: 'P' (point source), 'G' (gaussian), 'D' (elliptical disk)>
		"""

	@staticmethod
	def concatvis():
		"""
        	concatvis -- Name of visibility file to append to the input'
                default: none; example: concatvis='src2.ms'
		"""

	@staticmethod
	def connect():
		"""
       		connect -- Specifies what points to join with lines, when the
               	plotsymbol specifies line drawing. Valid values
               	include: 'none'(no connections), 'row' (along time and baseline),
                        and 'chan' (joined along channels).
               	default: 'none'; example: connect='chan'
		"""

	@staticmethod
	def constpb():
		""" Used in Sault weighting, the flux scale is constant above this value: """

	@staticmethod
	def constrainflux():
		"""
        	constrainflux -- Constrain image to match target flux;
                otherwise, targetflux is used to initialize model only.
                default: False; example: constrainflux=True
		"""

	@staticmethod
	def contfile():
		""" 
       		contfile -- Name of output continuum image
		Default: none; Example: contfile='ngc5921_cont.im'
		"""

	@staticmethod
	def createimage():
		"""
		createimage -- do imaging?
		default: False
		"""
		
	@staticmethod
	def correlations():
		""" 
       		correlation -- Select correlators:
               	default: 'RR LL' both parallel hands.  Other options are
                  '' (all stokes),'RR','LL','RR LL','XX','YY','XX YY','LR','RL','LR RL','RL LR'
		"""

		
	@staticmethod
	def cyclefactor():	
		"""
        	cyclefactor -- Change the threshold at which the deconvolution cycle will 
                stop and degrid and subtract from the visibilities. For bad PSFs,
                reconcile often (cyclefactor=4 or 5); For good PSFs, use 
                cyclefactor 1.5 to 2.0.
                default: 1.5; example: cyclefactor=4
                <cycle threshold = cyclefactor * max sidelobe * max residual>
		"""

	@staticmethod
	def cyclespeedup():
		"""
        	cyclespeedup -- Cycle threshold doubles in this number of iterations
                default: -1; example: cyclespeedup=500
		"""

	@staticmethod	
	def datacolumn():
		""" Which data column to use (for plotting, splitting, etc):

		plotxy: Visibility file (MS) data column
                default: 'data'; example: datacolumn='model'
                Options: 'data' (raw),'corrected','model','residual'(corrected-model),'weight'

                split: Visibility file (MS) data column
                default: 'corrected'; example: datacolumn='data'
                Options: 'data' (raw), 'corrected', 'model', 'all',
                         'float_data' (single dish), 'lag_data',
                         'float_data,data', and 'lag_data,data'.
                         note: 'all' = whichever of the above that are present.
		"""

	@staticmethod
	def degamp():
		""" Polynomial degree for amplitude bandpass solutions. """

	@staticmethod
	def deglast():
		"""
        	deglast -- Put degenerate axes last in header?
                default: False; example: deglast=True
		"""

	@staticmethod
	def degphase():
		""" Polynomial degree for phase bandpass solutions. """

	@staticmethod
	def direction():
		"""
		(for sdimprocess)
		direction -- scan direction in unit of degree
		default: []
		example: [0.0, 90.0]

		---------------------------------------------

		(for simdata)
		direction -- center of map or "" to center on the model
		* can optionally be a list of pointings, which will override
		pointingspacing. When direction is a list, the centroid of
		direction will be used as the center.
		* otherwise simdata will pack mapsize according to maptype
		default: ""
		example: 'J2000 19h00m00 -40d00m00'
		"""

	@staticmethod
	def dirtol():
		"""
        	dirtol -- Direction shift tolerance for considering data as the same field
                default: ;; means always combine
                example: dirtol='1.arcsec' will not combine data for a field unless
                their phase center is less than 1 arcsec.
		"""

	@staticmethod
	def dochannelmap():
		"""
		dochannelmap -- channel map image or total power image
		options: True (channel map), False (total power)
		default: False (total power)
		"""

	@staticmethod
	def doppler():
		"""
		doppler -- doppler mode
		options: 'RADIO', 'OPTICAL', 'Z', 'BETA', 'GAMMA'
		default: currently set doppler in scantable
		"""
		
	@staticmethod
	def dropdeg():
		"""
        	dropdeg -- Drop degenerate axes?
                default: False; example: dropdeg=True
		"""

	@staticmethod
	def edge():
		"""
		edge -- channels to drop at beginning and end of spectrum
		default: 0
		example: [1000] drops 1000 channels at beginning AND end
		         [1000,500] drops 1000 from beginning and 500 from end
		"""

	@staticmethod
	def ephemesrcname():
		"""
		ephemsrcname -- ephemeris source name for moving source
		default: ''
		if the source name in the data matches one of the known
		solar objects by the system, the tasks sdimaging and sdtpimaging
		automatically set the source name
		"""
		
	@staticmethod
	def estfile():
		"""
        	estfile -- Name of the file containing an image profile fit
		           estimate.
                default:; example: estfile='myimage.estimate'
		"""				

	@staticmethod
	def estimate():
		"""
        	estimate -- Create an estimate of the profile (yes/no)?
                default: False; example: estimate=True
		"""		

	@staticmethod
	def excludepix():
		"""
        	excludepix -- Range of pixel values to exclude
                default: [-1] (don't exclude pixels); example=[100.,200.]
		"""

	@staticmethod
	def excludemask():
		"""
        	excludemask -- Range of masks to exclude
                default: [-1] (don't exclude channels); example=[100,200]
		"""

	@staticmethod
	def expr():
		"""
		expr -- mathematical expression using scantables
		default: ''
		example: expr='("orion_on.asap"-"orion_off.asap")/"orion_off.asap"'
		
		In the expression, input file names should be put inside
		of single or double quotes
		"""

	@staticmethod
	def factor():
		"""
		factor -- scaling factor
		default: 1.0 (no scaling)
		"""
		
	@staticmethod
	def field():
		"""
		field -- Select field using field id(s) or field name(s).
		        [run listobs to obtain the list id's or names]
	        default: 0  (for sdimaging)
		         '' = all fields (for the other ASAP tasks)
			 
	        If field string is a non-negative integer, it is assumed a field index
		otherwise, it is assumed a field name
		         field='0~2'; field ids 0,1,2
			 field='0,4,5~7'; field ids 0,4,5,6,7
			 field='3C286,3C295'; field named 3C286 adn 3C295
			 field = '3,4C*'; field id 3, all names starting with 4C
	        This selection is in addition to scanlist, iflist, and pollist.

		See help par.selectdata for additional syntax.
		See specific task for any additional details.
		"""

	@staticmethod
	def fieldid():
		""" Field index identifier; 0-based. """

	@staticmethod
	def figfile():
		""" File name to store the plotte figure in. File format
		    is determined by the file extension '.png' is
		    recommended.
		"""

	@staticmethod
	def fitfile():
		""" name of output file for fit result
		    default: ''  (no output fit file)
		    example: fitfile="myimage.fit"
		"""

	@staticmethod
	def fitfunc():
		"""
		fitfunc -- function for fitting
		options: 'gauss', 'lorentz'
		default: 'gauss'
		"""

	@staticmethod
	def fitmode():
		"""
		(for sdfit)
		fitmode -- mode for fitting
		options: 'list', 'auto', 'interact'
		default: 'auto'
		example: 'list' will use maskline to define regions to
	                        fit for lines with nfit in each
	                 'auto' will use the linefinder to fir for lines
			        using the following parameters
	                 'interact' allows adding and deleting mask
	                        regions by drawing rectangles on the plot
				with mouse. Draw a rectangle with LEFT-mouse
				to ADD the region to the mask and with RIGHT-mouse
				to DELETE the region

	        -------------------------------------------------------------------
		
		(for uvcontsub)
	        fitmode -- use of the continuum fit model
                options: 'subtract' -- store continuum model and subtract from data
                         'replace'  -- replace vis with continuum model
			 'model'    -- only store continuum model
                default: 'subtract'
		"""

	@staticmethod
	def fitorder():
		""" Polynomial order for fit: """

	@staticmethod
	def fitsfile():
		""" Name of input UV FITS file: """

	@staticmethod
	def fitsimage():
		"""
        	fitsimage -- Name of input image FITS file
                default: none; example='3C273XC1.fits'
		"""

	@staticmethod
	def fixpar():
		"""
        	fixpar -- Control which parameters to let vary in the fit
                default: [] (all vary); example: vary=[False,True,True]
                (this would fix the flux to that set in sourcepar but allow the
                x and y offset positions to be fit).
		"""

	@staticmethod
	def flaglist():
		"""
		flaglist -- list of scan numbers to flag (ranges can be accepted)
		default: [] (use all scans)
		example: [[0,3],80]
		         flag the scan range [0,3] = [0,1,2,3] and scan 80
	        """

	@staticmethod
	def flagmode():
		"""
		flagmode -- flag mode
		options: 'flag', 'unflag', 'restore'
		         in 'restore' mode, a history of flagging is
			 displayed and current flag state is returned
		default: 'flag'
		"""

	@staticmethod
	def flagrow():
		"""
		flagrow -- list of row numbers to apply flag/unflag (row based)
		default: [] (no row selection)
		example: [0,2,3]

		This parameter is effective only when one or more row numbers
		are given explicitly and also clip=False
		"""

	@staticmethod
	def flrange():
		"""
		flrange -- range for flux axis of plot for spectral plotting
		options: (list) [min,max]
		default: [] (full range)
		example: flrange=[-0.1,2.0] if 'K' assumes current fluxunit
		"""
		
	@staticmethod
	def fluxdensity():
		""" 
       		fluxdensity -- Specified flux density [I,Q,U,V] in Jy
               	default=-1, which causes setjy to look up standard I flux densities
               	Otherwise, use the specified flux density.
               	If a model image is also specified, only stokes I will be used.
               	example:  fluxdensity=[2.63,0.21,-0.33,0.02]
		"""

	@staticmethod
	def fluxtable():
		""" 
       		fluxtable -- Name of output, flux-scaled calibration table
               default: none; example: fluxtable='ngc5921.gcal2'
               This gains in this table have been corrected after the
               flux density determination of each source.  The MODEL_DATA
               column has NOT been updated for the flux density of the
               calibrator.  Use setjy to do this.  see task correct for more
               information.
		"""

	@staticmethod
	def fluxunit():
		"""
		fluxunit -- units for line flux
		options: 'K', 'Jy', ''
		default: '' (keep current fluxunit)

		For GBT data, see description for par.telescopeparm
		"""

	@staticmethod
	def fontsize():
		"""
       		fontsize -- Font size for labels
               	default: 10; example: fontsize=2
		"""

	@staticmethod
	def format():
		"""
		format -- format string to print statistic values
		default: '3.3f'
		"""

	@staticmethod
	def frame():
		"""
		frame -- frequency frame for spectral axis
		options: 'LSRK', 'REST', 'TOPO', 'LSRD', 'BARY', 
			 'GEO', 'GALACTO', 'LGROUP', 'CMB'
	        default: currently set frame in scantable
		WARNING: frame='REST' not yet implemented
		"""
		
	@staticmethod
	def freqdep():
		""" Solve for frequency dependent solutions
		Default: False (gain; True=bandpass); example: freqdep=True
		"""

	@staticmethod
	def freqtol():
		""" 
	        freqtol -- Frequency shift tolerance for considering data as the same spwid
                default: ''  means always combine
                example: freqtol='10MHz' will not combine spwid unless they are
                within 10 MHz
		"""

	@staticmethod
	def frequencytol():
		"""
      		frequencytol -- Tolerance in frequency shift in making spectral windows
              	default: 150000 (Hz).  For Doppler shifted data, <10000 Hz may
              	may produce too many unnecessary spectral windows.
              	example: frequencytol = 1500000.0 (units = Hz)
		"""

	@staticmethod
	def ftmachine():
		"""
        	ftmachine -- Gridding method for the image;
                ft (standard interferometric gridding), sd (standard single dish)
                both (ft and sd as appropriate), mosaic (gridding use PB as conv fn)
                default: 'mosaic'; example: ftmachine='ft'
		"""

	@staticmethod
	def gain():
		""" Loop gain for CLEANing: """

	@staticmethod
	def gaincurve():
		""" Apply VLA antenna gain curve correction: """

	@staticmethod
	def gaintype():
		""" Type of gain calibration solution to solve for. 
		This can be either 'G' or 'GSPLINE'.
		"""

	@staticmethod
	def gainfield():
		""" Select a calibration table by field: """
	
	@staticmethod
	def gaintable():
		""" Gain calibration solutions table: """

	@staticmethod
	def gridfunction():
		"""
		gridfunction -- gridding function for imaging
		options: 'BOX' (Box-car), 'SF' (Spheroidal), 'PB' (Primary-beam)
		default: 'BOX'
		"""
		
	@staticmethod
	def hditem():
		"""
        	hditem -- Header item to change
                default: ''; example: hditem='telescope'
                Options: 'object','telescope','observer'
		"""

	@staticmethod
	def hdvalue():
		"""
        	hdvalue -- Value to set Header item to
                default: ''; example: hdvalue='VLA'
                Examples: 
                          hditem='epoch',hdvalue='2007/06/20/00:00:00'
                          hditem='beam',hdvalue=['55arcsec','55arcsec','0deg']
                          hditem='restfrequency',hdvalue='1.421GHz'
                          hditem='restfrequency',hdvalue='1.421GHz 115.272GHz'
                          hditem='projection',hdvalue='TAN'
                          hditem='cdelt4',hdvalue=24415.05
                          hditem='cdelt1',hdvalue=7.75e-5
		"""

	@staticmethod
	def header():
		"""
		header -- print header information on plot
                options: (bool) True, False
                default: True
		"""
	
	@staticmethod
	def headsize():
		"""
		headsize -- header font size
		options: (int)
		default: 9
		"""

	@staticmethod
	def highres():
		""" Name of high resolution (interferometer) image: """

	@staticmethod
	def histogram():
		"""
		histogram -- plot histogram
		options: (bool) True, False
		default: False
		"""

	@staticmethod
	def iflist():
		"""
		iflist -- list of IF id numbers to select
		default: [] (use all IFs)
		example: [15]
		
	        This selection is in addition to scanlist, field, and pollist
		"""

        @staticmethod
        def ignoreables():
                """
                Let time bins ignore boundaries in array, scan, and/or state.
                default = '' (separate time bins by all of the above)
                examples:
                ignorables = 'scan': Can be useful when the scan number
                                     goes up with each integration,
                                     as in many WSRT MSes.
                ignorables = ['array', 'state']: disregard array and state
                                                 IDs when time averaging.
                ignorables = 'state,subarr': Same as above.  ignorables
                                             matches on 'arr', 'scan', and
                                             'state'.
                """
		
	@staticmethod
	def imagename():
		"""
		(for boxit, deconvolve, exportfits, imcontsub, imfit, imhead,
		     immath, immoments, imregrid, imsmooth, imstat, imval,
		     and specfit)
		imagename -- input image name(s)

		------------------------------------------------------------

		(for feather, importfits, makemask, sdimaging, sdimprocess,
		     and sdtpimaging)
		imagename -- output image name

		------------------------------------------------------------
		(for autoclean, clean, mosaic, and widefield)
		imagename -- pre-name of output image(s)
		
		For output image files, imagename is followed by
		'.residual', '.model', '.image', etc.

		------------------------------------------------------------
		"""

	@staticmethod
	def imagermode():
		"""
		imagermode -- Determines advanced imaging/gridding scheme.
		options: '', 'mosaic', 'csclean'
		default: '' means single field clean
		example: imagermode='mosaic'  (image fields as a mosaic)
	                 imagermode='csclean' (use Cotton-schwab cleaning)
		"""

	@staticmethod
	def imsize():
		"""
		imsize -- image pixel size [x,y]
                default: [256,256]
		example: imsize=[500,500]
		         imsize=500  (equivalent to [500,500])
                Need not be a power of 2, but not a prime number
		"""

	@staticmethod
	def inbright():
		"""
		(for simdata)
		inbright -- peak surface brightness to scale input image
		in Jy/pixel.
		default: 'unchanged'
		
		[alert] If you specify 'unchanged' it will take the
		numerical values in your image and assume they are in Jy/pixel,
		even if it says some other unit in the header. This will be made
		more flexible in the future.

		----------------------------------------------------------------
		
		(for simdata)
		inbright -- peak surface brightness to scale input image
		in Jy/square arcsec.
		options: 'default' or surface brightness in Jy/sq.arcsec.
		default: 'default'
		"""

	@staticmethod
	def incell():
		"""
		incell -- pixel size of the model image.
		options: 'header' or pixel size, e.g. '0.1arcsec'
		default: 'header'
		"""
		
	@staticmethod
	def includepix():
		"""
	        includepix -- Range of pixel values to include
                default: [-1] (all pixels); example=[0.02,100.0]
		"""

	@staticmethod
	def includemask():
		"""
                (for msmoments)
	        includemask -- Range of masks to include
                default: [-1] (include all channels); example=[2,100]
		"""

	@staticmethod
	def incremental():
		"""
       		incremental -- Add to the existing MODEL_DATA column?
               	default: False; example: incremental=True
		"""

	@staticmethod
	def incrtable():
		""" Input (incremental) calibration table (any type): """

	@staticmethod
	def integration():
		"""
		integration -- integration (sampling) time
		default: '10s'
		"""
		
	@staticmethod
	def interactive():
		"""
		(for sdstat)
		interactive -- determines interactive masking
		options: True, False
		default: False
		example: interactive=True allows adding and deleting mask
		regions by drawing rectangles on the plot with mouse.
		Draw a rectangle with LEFT-mouse to ADD the region to
		the mask and with RIGHT-mouse to DELETE the region.

		----------------------------------------------------------

		(for widefield)
		interactive -- use interactive clean (with GUI viewer)
		options: True, False
		defalt: False
		"""

	@staticmethod
	def interp():
		""" Setting of the time-dependent interpolation scheme when applying calibration solutions. 

		The options are:
		'nearest' -  calibrate each datum with the calibration value nearest in time.
		'linear' - calibrate each datum with calibration phases and amplitudes linearly interpolated 
		  from neighboring (in time) values. In the case of phase, this mode will assume that phase 
		  jumps greater than 180 degrees between neighboring points indicate a cycle slip, and the 
		  interpolated value will follow this change in cycle accordingly.
		'aipslin' - emulate the interpolation mode used in classic AIPS, i.e., linearly interpolated 
		  amplitudes, with phases derived from interpolation of the complex calibration values. While
  		  this method avoids having to track cycle slips (which is unstable for solutions with very 
		  low SNR), it will yield a phase interpolation which becomes increasingly non-linear as the 
		  spanned phase difference increases. The non-linearity mimics the behavior of 
		  interp='nearest' as the spanned phase difference approaches 180 degrees (the phase of the 
		  interpolated complex calibration value initially changes very slowly, then rapidly jumps 
		  to the second value at the midpoint of the interval). 

		If the uncalibrated phase is changing rapidly, a 'nearest' interpolation is not desirable.
		Usually, interp='linear' is the best choice. 
		"""

	@staticmethod
	def invertmask():
		"""
		invertmask -- invert mask (EXCLUDE masklist instead)
		options: True, False
		default: False
		"""
		
	@staticmethod
	def iteration():
		"""
       		iteration -- Iterate plots:
               	default: ''; no interation
               	Options: 'field', 'antenna', 'baseline'.
               	Use the 'NEXT' button on gui to iterate through values.
               	To abort an iteration, close the gui window.
		"""

	@staticmethod
	def kernel():
		"""
		(for imsmooth)
		kernel -- type of kernel to use when smoothing.
		Currently, only gaussian is supported.
		options: 'gaussian', 'boxcar'
		default: 'gaussian'

		-----------------------------------------------------------
		
		(for sdcal, sdplot, and sdsmooth)
		kernel -- type of spectral smoothing
		options: 'none', 'hanning', 'gaussian', 'boxcar'
		default: 'hanning' for sdsmooth, 'none' for the other tasks
		"""

	@staticmethod
	def kwidth():
		"""
		kwidth -- width of spectral smoothing kernel
		options: (int) in channels
		default: 5
		example: 5 or 10 seem to be popular for boxcar
		         ignored for hanning (fixed at 5 chans)
			 (0 will turn off gaussian or boxcar)
		"""

	@staticmethod
	def layout():
		"""
                layout -- a list of subplots layout in figure coordinate (0-1), 
                          i.e., fraction of the figure width or height.
	        The order of elements should be:
                [left, bottom, right, top, horizontal space btw panels,
                vertical space btw panels]
 		"""

	@staticmethod
	def legendloc():
		"""
		legendloc -- legend location on the axes (0-10)
		options: (integer) 0 -10 
		         Note that 0 ('best') is very slow.
			 The list of legend positions: 
                           0: best
			   1: upper right
			   2: upper left
			   3: lower left
			   4: lower right
			   5: right
			   6: center left
			   7: center right
			   8: lower center
			   9: upper center
			   10: center
	        default: 1 ('upper right')
		"""

	@staticmethod
	def linecat():
		"""
		linecat -- control for line catalog plotting for spectral plotting
		options: (str) 'all', 'none', or by molecule
		default: 'none' (no lines plotted)
		example: linecat='SiO' for SiO lines
		         linescat='*OH' for alcohols
			 uses sprange to limit catalog
		"""

	@staticmethod
	def linedop():
		"""
		linedop -- doppler offset for line catalog plotting (spectral plotting)
		options: (float) doppler velocity (km/s)
		default: 0.0
		example: linedop=-30.0
		"""
		
	@staticmethod
	def linefile():
		""" 
       		linefile -- Name of output line image
		Default: none; Example: outline='ngc5921_line.im'
		"""

	@staticmethod
	def linestyles():
		"""
		linestyles -- the linestyles to be used for plotting lines
		default: None
		example: linestyles='line dashed dotted dashdot dashdotdot dashdashdot'
		         The plotter will cycle through these linestyles
			 when lines are overlaid (stacking mode).

	        warning: linestyles can be specified only one color has been set. 
		"""
		
	@staticmethod
	def linewidth():
		"""
       		linewidth -- Width of plotted lines.
               	default: 1
		example: linewidth=0.75
		"""

	@staticmethod
	def listfile():
		"""
		listfile -- output file name (will not overwrite)
		default: '' (no output file)
		"""
		
	@staticmethod
	def lowres():
		""" Name of low resolution (single dish) image: """

	@staticmethod
	def markersize():	
		"""
       		markersize -- Size of the plotted marks
               	default: 1.0
		"""

	@staticmethod
	def mask():
		""" Name(s) of mask image(s) used for CLEANing:
		    In the image analysys tasks (immath, imfit, and imstat)
		    either a Lattice Expression, which may be mask filename.
		    default:
		    example: mask='mask(myimage.mask)'
	                     mask='"myimage">0.5'
		"""

	@staticmethod
	def maskcenter():
		""" Controls how many channels at the center of each input spectral window are ignored
		on-the-fly. It is usually best to flag these channels directly. """

	@staticmethod
	def maskedge():
		""" Controls how many channels at the edge of each input spectral window are ignored
		on-the-fly. It is usually better to flag these channels directly. """

	@staticmethod
	def maskflag():
		"""
		maskflag -- list of mask regions to apply flag/unflag
		default: []  (entire spectrum)
		example: [[1000,3000],[5000,7000]]
		warning: if one or more rows are given in flagrow, or 
		         clip=True, this parameter is ignored
		"""

	@staticmethod
	def maskline():
		"""
		maskline -- list of mask regions to INCLUDE in LINE fitting
		default: all
		example: maskline=[[3900,4300]] for a single region, or
		         maskline=[[3900,4300],[5000,5400]] for two, etc. 
		"""

	@staticmethod
	def masklist():
		"""
                (for sdbaseline and sdcal)
		masklist -- list of mask regions to INCLUDE in BASELINE fitting
		default: []  (entire spectrum)
		example: [[1000,3000],[5000,7000]]
		         if blmode='auto' then this mask will be applied
			 before fitting

	        ---------------------------------------------------------------

		(for sdstat)
		masklist -- list of mask regions to INCLUDE in stats
		default: []  (entire spectrum)
		example: [4000,4500] for one region
		         [[1000,3000],[5000,7000]] for two regions, etc.
			 
	        ---------------------------------------------------------------

		(for sdtpimaging)
		masklist -- mask in numbers of rows from each edge of each scan
		for baseline fitting
		default: none
		example: [30,30] or [30]
		         uses first 30 rows and last 30 rows of each scan
			 for baseline
		
	        ---------------------------------------------------------------

		(for sdimprocess)
		masklist -- mask width for Basket-Weaving on percentage
		default: 1.0 (1.0% of map size)
		"""
		
        @staticmethod
        def maxpix():
                """
                maxpix -- Maximum pixel value
                default: 0 = autoscale
                """
	@staticmethod	
	def merge():
		"""
        	merge -- Merge operation
                default: 'replace'; will write (or overwrite) a flag file with
                   mode = 'save'
                Options: 'or','and' not recommended for now.
		"""

        @staticmethod
        def minpix():
                """
                minpix -- Minimum pixel value
                default: 0 = autoscale
                """

	@staticmethod
	def minpb(): 
		""" 
        	minpb -- Minimum PB level to use
                default=0.01; example: minpb=0.1
		"""

	@staticmethod
	def minpix():
		"""
        	minpix -- Minimum pixel value
                default: 0 = autoscale
		"""

	@staticmethod
	def min_nchan():
		"""
		min_nchan -- minimum number of consecutive channels for linefinder
		default: 3
		example: minimum number of consecutive channels required to pass threshold
		"""
		
	@staticmethod
	def mode():
		"""
		(for flagdata)
		mode -- mode of operation
		options: 'manualflag', 'autoflag', 'summary', 'quack', 'shadow', 'rfi'
		         'manualflag' = flagging based on specific selection parameter
			                plus clipping and flagging autocorrelations
	                 'autoflag'   = experimental auto-flagging outliers
			 'summary'    = report the amount of flagged data
			 'quack'      = remove/keep specific time range at scan
			                beginning/end
	                 'shadow'     = remove antenna-shadowed data
			 'rfi'        = Redio Frequency Interference auto-flagging
		default: 'manualflag'

		--------------------------------------------------------------------------

                (for imhead)
 	        mode -- Mode, either 'get' or 'put'
                options: 'list', 'get', 'put', 'history', 'summary', 'add', 'del'
		         'list'    = lists the image header keywords and values
			 'get'     = get the specified keyword value(s) from the image
			 'put'     = put the specified keyword value(s) into the image
			 'history' = display the history information in the CASA logger
			 'summary' = information summarizing the CASA image file
			 'add'     = adds a new header key. use with caution
			 'del'     = deletes a header key, hdkey. use with caution
                default: 'summary'

		--------------------------------------------------------------------------

                (for immath)
		mode -- mode for mathematical operation
		options: 'evalexpr' = evaluate a mathematical expression defined in 'expr'
		         'spix'     = spectralindex image
			 'pola'     = polarization position angle image
			 'poli'     = polarization intensity image
		default: 'evalexpr'

		--------------------------------------------------------------------------

                (for makemask)
		mode -- type of data selection
		options: 'mfs', 'channel'
		default: 'mfs'

		--------------------------------------------------------------------------

                (for mosaic, widefield)
		mode -- frequency specification; type of selection
		options: 'mfs'       = produce one image from all specified data
		         'channel'   = use with nchan, start, width to specify
			               output image cube. 
		         'velocity'  = channels are specified in velocity
	                 'frequency' = channels are specified in frequency
	        default: 'mfs'

		--------------------------------------------------------------------------

		(for newflagdata)
		mode -- mode of operation
		options: 'manualflag', 'autoflag', 'summary', 'query', 'extend', 'run'

		--------------------------------------------------------------------------

                (for sdimprocess)
		mode -- processing mode
		options: 'basket', 'press'
		default: 'basket'

		--------------------------------------------------------------------------

		(for specfit)
		mode -- operation mode
		options: 'single' = fits a 1-D model to a single profile
		         'all'    = fits a 1-D model to all profiles
			 'poly'   = fits 1-D polynomials to profiles
		default: 'single'

		--------------------------------------------------------------------------

		(for vishead)
		mode -- operation mode
		options: 'list'    = list all keywords that are recognized, and list the
		                     value(s) for each. Only these keywords can be
				     obtained (get) or changed (put). 
	                 'summary' = equivalent to running taskname='listobs'; verbose=False
			 'get'     = get the specified keyword value(s) from the ms
	                 'put'     = put the specified keyword value(s) into the ms
		default: 'list'
		"""

	@staticmethod
	def model():
		""" Name of input model for pointcal (component list or image).
		ft:
		model -- Name of input model image
               	default: None;
               	example: model='/usr/lib/casapy/data/nrao/VLA/CalModels/3C286_X.im'
		"""

	@staticmethod
	def modimage():
		"""
       		modimage -- Optional model image (I only, please) from which to predict visibilities
              	if specified, setjy will insist that only one field be selected.
              	this model image will be scaled to match the flux implied by
              	the fluxdensity and standard parameters 
		"""
	@staticmethod
	def modelimage():
		"""
		(for mosaic)
		modelimage -- name of output(/input) model image
		default: ''  (none=imagename.model)
		note: this specifies the output model if a single dish image is
		      input or the output model name from the imaging

       		Optional model image from which to predict visibilities
		This can be either a model image from a previous deconvolution
		or an image from a single dish image if single dish uv coverage
		is being introduced in the imaging
		"""

	@staticmethod
	def modifymodel():
		"""
		modifymodel -- modify model image WCS or flux scale
		options: True, False
		default: False
		"""
		
	@staticmethod
	def moments():
		"""
        	moments -- List of moments you would like to compute
                default: 0 (integrated spectrum);example: moments=[0,1]

        	moments=-1  - mean value of the spectrum
        	moments=0   - integrated value of the spectrum
        	moments=1   - intensity weighted coordinate;traditionally used to get 
               		       'velocity fields'
        	moments=2   - intensity weighted dispersion of the coordinate; traditionally
               		       used to get 'velocity dispersion'
        	moments=3   - median of I
        	moments=4   - median coordinate
        	moments=5   - standard deviation about the mean of the spectrum
        	moments=6   - root mean square of the spectrum
        	moments=7   - absolute mean deviation of the spectrum
        	moments=8   - maximum value of the spectrum
        	moments=9   - coordinate of the maximum value of the spectrum
        	moments=10  - minimum value of the spectrum
        	moments=11  - coordinate of the minimum value of the spectrum
		"""

	@staticmethod
	def msselect():
		""" Optional subset of data to select: 

		See: http://aips2.nrao.edu/docs/notes/199/199.htlm

		"""

	@staticmethod
	def msname():
		"""
                (for msmoments)
                Name of input MS data
		default: none; example: msname=&quot;OrionS_rawACSmod&quot;
		"""

	@staticmethod
	def multicolor():
		"""
       		multicolor -- Multi-color plotting of channels and polarizations
                options: 'none','both','chan','corr'
                default: 'none'; example: multicolor='chan'
		"""

	@staticmethod
	def multiplot():
		""" 
       		multiplot -- Automatically Plot calibration for different antennas
               	in separate frames as indicated by subplot (see above)
               	default: False; only one frame is made (with subplots maybe)
               	If true; will cycle antennas through frames
		"""

	@staticmethod
	def multisource():
		"""
        	multisource -- Write in multi-source format
                default: True;
                   false if one source is selected
		"""

	@staticmethod
	def nchan():
		"""
		(for exportuvfits, makemask)
		nchan -- number of channels to select
		default: -1  (all)

		----------------------------------------------------

		(for mosaic, sdimaging, widefield)
		nchan -- number of channels (planes) in output image
		default: 1
		"""
	
	@staticmethod
	def negcomponent():
		"""
        	--- Multi-scale parameters
        	negcomponent -- Stop component search when the largest scale has found this
                number of negative components; -1 means continue component search
                even if the largest component is negative.
                default: 2; example: negcomponent=-1
		"""

	@staticmethod
	def nfit():
		"""
		nfit -- list of number of Gaussian lines to fit in maskline region
		default: 0  (no fitting)
		example: nfit=[1] for single line in single region
		         nfit=[2] for two lines in single region
			 nfit=[1,1] for single lines in each of two regions, etc.
		"""
		
	@staticmethod
	def ngauss():
		"""
        	ngauss -- Number of Gaussian elements to use when fitting profiles                default: 1;
		"""		

	@staticmethod
	def niter():
		""" Number of iterations; set niter=0 for no CLEANing: """

	@staticmethod
	def noise():
		"""
        	--- superuniform/briggs weighting parameter
        	noise   -- noise parameter to use for rmode='abs' in briggs weighting
                   example noise='1.0mJy'
		"""

	@staticmethod
	def noise_thermal():
		"""
		noise_thermal -- add thermal noise
		options: True, False
		default: False

		* [alpha] currently only knows about ALMA (and (E)VLA) receivers
		"""
		
	@staticmethod
	def npercycle():
		"""
        	--  interactive masking
        	npercycle -- when cleanbox is set to 'interactive', this is the number of iterations
           	between each clean to update mask interactively. Set to about niter/5.
		"""

	@staticmethod
	def npixels():
		"""
        	--- superuniform/briggs weighting parameter
        	npixels -- number of pixels to determine uv-cell size for weight calculation
		"""

	@staticmethod
	def npointaver():
		""" Number of points to average together for tuning the
		GSPLINE phase wrapping algorithm. """

	@staticmethod
	def numpoly():
		"""
		numpoly -- order of polynomial fit in Pressed-out
		default: 2
		"""
		
	@staticmethod
	def nxpanel():
		""" Panel number in the x-direction: """

	@staticmethod
	def nypanel():
		""" Panel number in the y-direction: """

	@staticmethod
	def opacity():
		""" Opacity correction to apply (nepers): """

	@staticmethod
	def optical():
		"""
        	optical -- Use the optical (rather than radio) velocity convention
                default: True;
		"""

	@staticmethod
	def outfile():
		"""
		(for immath)
		outfile -- output image file name.
		default: 'immath_results.im'

		Overwriting an existing outfile is not permitted.

		----------------------------------------------------------------

		(for immoments)
        	outfile -- output image file name (or root for multiple moments)
                default: '' (input+auto-determined suffix)
		example: outfile='source_moment'

		----------------------------------------------------------------

		(for msmoments)
        	outfile -- output MS file name (or root for multiple moments)
                default: '' (input+auto-determined suffix)
		example: outfile='source_moment'

		----------------------------------------------------------------

		(for imsmooth)
		outfile -- output image file name.
		default: 'imsmooth_results.im'
		
		----------------------------------------------------------------

		(for ASAP tasks (sd*))
		outfile -- output file name
		default: ''

		Given default value ('') for outfile, some ASAP tasks set output
		file name as sdfile (=input file name) with suffix as follows:
		        <sdfile>_cal             for sdaverage and sdcal,
			<sdfile>_bs              for sdbaseline,
			<sdflag>_f               for sdflag,
			<sdfile>_scaleed<factor> for sdscale, and
			<sdfile>_sm              for sdsmooth.

		----------------------------------------------------------------

		(for uvmodelfit)
	        outfile -- optional output component list table
                default: ''
		example: outfile='componentlist.cl'
		"""

	@staticmethod
	def outform():
		"""
		outform -- output file format
		options: 'ASAP','ASCII','MS2','SDFITS'
		default: 'ASAP'

		the ASAP format is easiest for further sd processing;
		use MS2 for CASA imaging. If ASCII, then will append some
		stuff to the output file.
		"""
		
	@staticmethod
	def outputvis():
		""" Name of output visibility file (MS) """

	@staticmethod
	def overplot():
		"""
       		overplot -- Overplot these values on current plot (if possible)
               	default: False; example: overplot= True
		"""

	@staticmethod
	def overwrite():
		"""
        	overwrite -- overwrite pre-existing imagename or output file
		options: True, False
                default: False
		"""

	@staticmethod
	def panel():
		"""
		panel -- code for splitting into multiple panels for spectral plotting
                options: 'p','b','i','t','s' or
                         'pol','beam','if','time','scan'
                default: 'i'
                example: maximum of 25 panels
                         panel by pol, beam, if, time, scan
		"""
		
	@staticmethod
	def pbcor():
		""" Correct final image for primary beam or not.
		
		default: False; example pbcor=True

		"""


	@staticmethod
	def phasecenter():
		"""
        	phasecenter -- image phase center (for ASAP tasks) or mosaic center:
		               direction measure or fieldid
                default: '' (imply field=0 as center)
		example: phasecenter=6
                         phasecenter='J2000 19h30m00 -40d00m00'
		"""

	@staticmethod
	def phasewrap():
		""" Difference in phase (degrees) between points for
		tuning the GSPLINE phase wrapping algorithm.
		"""

	@staticmethod
	def planes():
		"""
        	planes -- Range of planes to include in the moment
                default: '' (all); 
                example: axis=3; planes='3~9'
                         This will select channels 3-9 from axis 3 (spectral axis)
		"""

	@staticmethod
	def plotcolor():
		"""
       		plotcolor -- pylab color.  Overrides plotsymbol color settings.
                                  See cookbook for details
               	default: 'darkcyan'
               	example: plotcolor='g' (green)
                        plotcolor='slateblue'
                        plotcolor='#7FFF34' (RGB tuple)
		"""

	@staticmethod
	def plotfile():
		"""
		plotfile -- file name for hardcopy output
		options: (str) filename.eps, .ps, .png
		default: ''  (no hardcopy)
		"""

	@staticmethod
	def plotlevel():
		"""
		plotlevel -- control for plotting of results
		options: (int) 0(none), 1(some), 2(more), <0(hardcopy)
		default: 0  (no plotting)

		Given a negative value, hardcopy plot will be named <sdfile>_scans.eps. 
		"""
		
	@staticmethod	
	def plotrange():
		"""
       		plotrange -- Specifies the size of the plot [xmin, xmax, ymin, ymax]
               	default: [0,0,0,0]; example: [-20,100,15,30]
               	Note that if xmin=xmax the values will be ignored and a best
               	guess will be made, simlarly if ymin=ymax.
               	time axes coordinate is in Julian Days, unfortunately.
		"""

	@staticmethod
	def plotstyle():
		"""
		plotstyle -- customise plot settings
                options: (bool) True, False
                default: True
		"""

	@staticmethod
	def plotsymbol():
		"""
	        plotsymbol -- pylab plot symbol.  See cookbook for details
                default: '.': large points
                  ',' = small points
                  '-' = connect points by line
 
		"""

	@staticmethod
	def plottype():
		"""
		plottype -- type of plot
		options: 'spectra','totalpower','pointing','azel'
		default: 'spectra'
		"""

	@staticmethod
	def pointingcolumn():
		"""
		pointingcolumn -- pointing data column to use
                options: 'direction','target','pointing_offset','source_offset','encoder' 
                default: 'direction'
		"""

	@staticmethod
	def pointingspacing():
		"""
		pointingspacing -- spacing in between beams
		default: '1arcmin'
		"""
		
	@staticmethod
	def pointtable():
		""" Name of pointing calibration table: """

	@staticmethod
	def polaverage():
		"""
		polaverage -- average polarizations
		options: True,False
		default: False
		"""

	@staticmethod
	def pollist():
		"""
		pollist -- list of polarization id numbers to select
		default: []  (all)
		example: [1]

		this selection is in addition to scanlist, field, and iflist. 
		"""
		
	@staticmethod
	def poly():
		"""
		poly --	A polygonial region in the directional portion of
		an image. The directional portion of an image are the axes
		for right ascension and declination, for example.  Polygons
		are specified as a list of points, the end points of each
		line segment in the polygon as follows: x1, y1, x2, y2, x3, y3,...
		ONLY pixel values acceptable at this time.
		Default: none (all);
		Example: poly='0,10,25,25,25,0'
		Example: poly='20,20,0,25,25,0;100,110,133,110,112,125,100,110'
		"""
		

	@staticmethod
	def prior():
		"""
        	------parameters useful for mem only
        	prior     -- Prior image to guide mem
		
		mosaic:
	        prior -- Name of MEM prior images
                default: ['']; example: prior='source_mem.image'

		"""

	@staticmethod
	def project():
		"""
      		project -- root for output file names
		default: 'sim'
		"""

	@staticmethod
	def psf():
		"""
        	psf       -- Name of psf image to use e.g  psf='mypsf.image' .
                     But if the psf has 3 parameter, then
                     a gaussian psf is assumed with the values representing
                     the major , minor and position angle  values
                     e.g  psf=['3arcsec', '2.5arcsec', '10deg']
		"""
	@staticmethod
	def psfmode():
		"""
        	psfmode -- Distinguish between Clark and Hogbom style of clean
		default='clark'
		example: psfmode='hogbom'
		"""

	@staticmethod
	def pweight():
		"""
        pweight -- weighting for polarization average
            options: 'var'  = 1/var(spec) weighted
                     'tsys' = 1/Tsys**2 weighted
            default: 'tsys'
		"""

	@staticmethod
	def quackinterval():
		"""
		quackinterval -- Time interval (n timestamps) at scan boundary for VLA quack-flagging
		"""

	@staticmethod
	def quackmode():
		"""
		quackmode -- 'beg','end','both'
		"""

	@staticmethod
	def refant():
		""" Reference antenna: """

	@staticmethod
	def refdate():
		"""
		refdate -- central time of simulated observation
		default: '2012/05/21/22:05:00'
		
                * [alpha] observations are centered at the nearest transit. 
		"""

	@staticmethod
	def refdirection():
		"""
		refdirection -- reference direction of the model image. 
                       options: (str) 'direction', 'header', or reference 
                                direction, e.g., 'J2000 19h00m00 -40d00m00'
                       default: 'direction'
		"""
		
	@staticmethod
	def reference():
		"""
       		reference -- Reference field name(s)
               The names of the fields with a known flux densities or structures.
               The syntax is similar to field.  Hence source index or
               names can be used.
               default: none; example: reference='1328+307'
               The model visibility is determined by using task setjy or task ft
               with a given source structure.
		"""

	@staticmethod
	def refpixel():
		"""
		refpixel -- reference pixel (CRPIX)
		options: '[x,y]' or 'center' or 'header'
		default: 'center'
		example: '[100,100]'
		"""
		
	@staticmethod
	def refspwmap():
		"""
       		refspwmap -- Vector of spectral windows enablings scaling across spectral windows
               default: [-1]==> none.
               Example with 4 spectral windows: if the reference fields were observed only in spw=1 & 3,
               and the transfer fields were observed in all 4 spws (0,1,2,3), specify refspwmap=[1,1,3,3].
               This will ensure that transfer fields observed in spws 0,1,2,3 will be referenced to
               reference field data only in spw 1 or 3.  Pray you don't have to do this.
		"""

	@staticmethod
	def region():
		"""
		region -- File path of a file containing an ImageRegion.
		ImageRegion files can be created with the CASA viewer's
		region manager, and typically have the suffix '.rgn'
		default: None
		example: region="myimage.im.rgn"
		"""

	@staticmethod
	def relmargin():
		"""
		relmargin -- how close pointing centers may approach the edge of the
                output image, as a fraction of pointingspacing.
                * ignored if direction is a list.
		options: (float)
		default: 1.0
		"""
		
	@staticmethod
	def removeoldpanels():
		"""
       		removeoldpanels -- Turn on/off automatic clearing of plot panels
                that lie under the current panel plotting.
               	default: True (removed underlying panels).
		"""

	@staticmethod
	def replacetopplot():
		"""
       		replacetopplot -- When overplotting, replace the last plot only
               	default: False
		"""

	@staticmethod
	def residualfile():
		""" Residual image file. Contains the residual of the
		    original image and its fitted image.
		    default:
		    example: residualfile="myimage.fit.residual"
		"""				

	@staticmethod
	def restfreq():
		"""
        	restfreq -- Specify rest frequency to use for image
		options: (float) or (string with unit) : see example
            	default: ''  (try to use the one specified in input data)
		example: 4.6e10, '46GHz'
	                 Allowed units are 'THz','GHz','MHz','kHz', and 'Hz'
		"""
		
	@staticmethod
	def rmode():
		""" Robustness mode; used for weighting='briggs': """

	@staticmethod
	def robust():
		""" Brigg's robustness parameter.

		Options: -2.0 (close to uniform) to 2.0 (close to natural)

		"""

	@staticmethod
	def rowlist():
		"""
		rowlist --  list of row numbers to process
                default: [] (use all rows)
                example: [0,2,4,6]
                         For expert users only!
                         this selection is applied first, and then followed by
			 the selection with scans, fields, ifs, and polarizations. 
		"""
		
	@staticmethod
	def scales():
		"""
        	--- Multiscale parameter
        	scales  -- in pixel numbers; the size of  component to deconvolve
                default = [0,3,10]
		"""

	@staticmethod
	def scaletsys():
		"""
		scaletsys -- scaling of associated Tsys
		options: True,False
		default: True
		"""
		
	@staticmethod
	def scaletype():
		""" Image plane flux scale type.
		
		Options: 'SAULT', 'PBCOR'

		"""

	@staticmethod
	def scan():
		""" Scan number range
                default: ''=all"""

	@staticmethod
	def scanaverage():
		"""
		scanaverage -- average integrations within scans
                options: (bool) True,False
                default: False
                example: if True, this happens in read-in
                         For GBT, set False!
		"""

	@staticmethod
	def scanlist():
		"""
		scanlist -- list of scan numbers to process
                default: [] (use all scans)
                example: [21,22,23,24]
                         this selection is in addition to field, iflist, and pollist
		"""

	@staticmethod
	def sdfile():
		"""
		sdfile -- name of input SD dataset
		default: ''
		"""

	@staticmethod
	def sdfilelist():
		"""
		sdfilelist -- list of names of input SD dataset
		default: ''
		"""

	@staticmethod
	def sdimages():
		"""
		sdimages -- name of input SD (FITS or CASA) image
		default: ''
		"""
		
	@staticmethod
	def showflags():
		"""
       		showflags -- Show the flagged and unflagged data.
               	default: False; example: showflags=True
		"""
		
	@staticmethod
	def showgui():
		"""
       		showgui -- Turn on/off the displaying of the plotting GUI
               	default: True; example: showgui=False
		"""

	@staticmethod
	def sigma():
		"""
        	------parameters useful for mem only
        	sigma     -- Estimated noise for image
		
		mosaic:
	        sigma -- Target image sigma
                default: '0.001Jy'; example: sigma='0.1Jy'

		"""

	@staticmethod
	def sigmafile():
		"""
        	sigmafile -- File contain weights to be applied to an
		image file when doing profile fitting.
		default:
		example: sigmafile='myimage.weights'
		"""		

	@staticmethod
	def smallscalebias():
		"""
		smallscalebias -- A bias toward smaller scales in multiscale
		clean.  The peak flux found at each scale is weighted by
		a factor = 1 - smallscalebias*scale/max_scale, so that weighted
		peak flux = peak flux*factor.
		Typically the values range from 0.2 to 1.0.
		default: 0.6
		"""
 
	@staticmethod
	def smoothsize():
		"""
		smoothsize -- smoothing beam in Pressed-out
		default: 2.0 (interpreted as 2.0 * beamsize)
		example: '1arcmin' (set smoothsize directly)
		"""

	@staticmethod
	def smoothtime():
		""" The smoothing filter time (sec). """

	@staticmethod
	def smoothtype():
		""" The smoothing filter to be used for calibration solutions.

		Options: 'mean','median','smean'(sliding mean),'smedian'(sliding median),
		'none'(copy table)
		
		"""

	@staticmethod
	def solint():
		""" Solution interval (in seconds): """

	@staticmethod
	def solnorm():
		""" Normalize the resulting solutions after the solve (True/False): """


	@staticmethod
	def sourcepar():
		"""
        	sourcepar -- Starting guess for component parameters (flux,xoffset,yoffset)
                default: [1,0,0]; example: sourcepar=[2.5,0.3,0.1]
                Note: Flux is in Jy, xoffset is in arcsec, yoffset is in arcsec.
		"""

	@staticmethod
	def specunit():
		"""
		specunit -- units for spectral axis
                options: (str) 'channel','km/s','GHz','MHz','kHz','Hz'
                default: '' (=current)
                example: this will be the units for masklist
		"""
		
	@staticmethod
	def splinetime():
		""" Spline timescale (sec); used for gaintype='GSPLINE' """

	@staticmethod
	def splitdata():
		"""
        	splitdata -- Split out continuum and continuum subtracted line data
                default: 'False'; example: splitdata=True
                The continuum data will be in: vis.cont
                The continuum subtracted data will be in: vis.contsub
		"""

	@staticmethod
	def sprange():
		"""
		sprange -- range for spectral axis of plot
                options: (list) [min,max]
                default: [] (full range)
                example: sprange=[42.1,42.5] if 'GHz' assumes current specunit
		"""
		
	@staticmethod
	def spw():
		"""
		(for sdimaging)
		spw -- spectral window id
                default: 0
                example: 1

	        this selection is in addition to scanlist and field

	        -----------------------------------------------------------------

		(for other tasks)
      		spw -- Select spectral window/channels
              	default: ''  (all spectral windows and channels)
		example: spw='0~2,4'        = spectral windows 0,1,2,4 (all channels))
              	         spw='<2'           = spectral windows less than 2 (i.e. 0,1))
			 spw='0:5~61'       = spw 0, channels 5 to 61)
			 spw='0,10,3:3~45'  = spw 0,10 all channels, spw 3, channels 3 to 45.
			 spw='0~2:2~6'      = spw 0,1,2 with channels 2 through 6 in each.
			 spw='0:0~10;15~60' = spectral window 0 with channels 0-10,15-60
			 spw='0:0~10,1:20~30,2:1;2;3' = spw 0, channels 0-10,
			                                spw 1, channels 20-30, and
							spw 2, channels, 1,2 and 3
		"""

	@staticmethod
	def spwmap():
		""" This parameter is used to indicate how solutions derived from different
		spectral windows should be applied to other spectral windows. Nominally data
		in each spectral window will be corrected by solutions derived from the same
		spectral window. This is the default behavior of spwmap, i.e., if spwmap is
		not specified, data will be corrected by solutions from the same spectral window.
		Otherwise, spwmap[j]=i causes soutions derived from the i-th spectral window
		to be used to correct the j-th spectral window. For example, if bandpass solutions
		are available for spectral windows 1 and 3, and it is desired that these be 
		applied to spws 2 and 4 (as well as 1 and 3), use spwmap=[1,1,3,3]. """

	@staticmethod
	def stack():
		"""
		stack -- code for stacking on single plot for spectral plotting
                options: 'p','b','i','t','s' or
                         'pol', 'beam', 'if', 'time', 'scan'
                default: 'p'
                example: maximum of 25 stacked spectra
                         stack by pol, beam, if, time, scan
		"""
		
	@staticmethod
	def standard():
		""" Flux density standard:

		Options: 'Baars', 'Perley 90', 'Perley-Taylor 95',
		'Perley-Taylor 99', 'Perley-Butler 2010', 'Butler-JPL-Horizons 2010'

		"""

	@staticmethod
	def start():
		"""
		start -- start channel
		default: 0
		"""

	@staticmethod
	def startfreq():
		"""
		startfreq -- frequency of first channel
                default: '89GHz'
		"""
		
	@staticmethod
	def starttime():
		"""
      		starttime -- Time after which data will be considered for importing
                    default: '1970/1/31/00:00:00'
                    starttime = '' gives error message
		"""

	@staticmethod
	def statfile():
		"""
		statfile -- name of output file for line statistics
                default: '' (no output statistics file)
                example: 'stat.txt'
		"""
		
	@staticmethod
	def step():
		"""
		step -- increment between channels
		default: 1
		"""

	@staticmethod
	def stokes():
		"""
		stokes -- stokes parameters to select/image
		options: 'I','IV','QU','IQUV',...
                'RR', 'LL', can only be done by flagging one polarization
		"""

	@staticmethod
	def stoptime():
		"""
      		stoptime --  Time before which data will be considered for importing
                    default: '2199/1/31/23:59:59'
                    stoptime = '' gives error message
		"""

	@staticmethod
	def subplot():
		""" 
		subplot -- Panel number on the display screen
                default: 111 (full screen display); example:
                if iteration is non-blank, then
                  subplot=yx1 window will produce y by x plots in the window.
                if iteration = '', then
                  subplot=yxn; means the window will have y rows, and x columns
                  with the present execution placing the plot in location n
                  n = 1,..., xy, in order upper left to right, then down.  An
                  example is the plotting of four fields on one frame
                  field='0'; subplot=221; plotxy()
                  field='1'; subplot=222; plotxy()
                  field='2'; subplot=223; plotxy()
                  field='3'; subplot=224; plotxy()

		"""

	@staticmethod
	def t_atm():
		"""
		t_atm -- atmospheric temperature in K 
		default: 260.0
		"""
		
	@staticmethod
	def t_ground():
		"""
		t_ground -- ambient temperature in K 
		default: 269.0
		"""
		
	@staticmethod
	def tablein():
		""" Input calibration table: 
		"""

	@staticmethod
	def tablename():
		""" 
        	tablename -- Name of table file on disk (MS, calibration table, image)
                	default: none; example: tablename='ngc5921.ms'
		"""

	@staticmethod
	def targetflux():
		"""
        	------parameters useful for mem only
        	targetflux -- Target total flux in image

		mosaic:
	        targetflux -- Target flux for final image 
                default: '1.0Jy'; example: targetflux='200Jy'

		"""

	@staticmethod
	def tau():
		"""
		tau -- atmospheric optical depth
		default: 0.0  (no correction)
		"""

	@staticmethod
	def tau0():
		"""
		tau0 -- zenith opacity at observing frequency
		default: 0.1
		"""

	@staticmethod
	def telescopeparm():
		"""
		telescopeparm -- the telescope name or characteristics
                        options: (str) name or (list) list of gain info
                        default: '' (none set)
                        example: if telescopeparm='', it tries to get the telescope
                                 name from the data.
                                 Full antenna parameters (diameter,ap.eff.) known
                                 to ASAP are
                                 'ATPKSMB', 'ATPKSHOH', 'ATMOPRA', 'DSS-43',
                                 'CEDUNA','HOBART'. For GBT, it fixes default fluxunit
                                 to 'K' first then convert to a new fluxunit.
                                 telescopeparm=[104.9,0.43] diameter(m), ap.eff.
                                 telescopeparm=[0.743] gain in Jy/K
                                 telescopeparm='FIX' to change default fluxunit
		"""

	@staticmethod
	def thresh():
		"""
		thresh -- S/N threshold for linefinder
		default: 5
		example: a single channel S/N ratio above which the channel is
	                 considered to be a detection
		"""
		
	@staticmethod
	def threshold():
		"""
		threshold -- flux level at which to stop CLEANing (units=mJy)
		"""

	@staticmethod
	def timeaverage():
		"""
		timeaverage -- average times for multiple scan cycles
                options: (bool) True,False
                default: False
		"""
	
	@staticmethod
	def timebin():
		"""
                Interval width for time averaging.
                default: '0s' or '-1s' (no averaging)
                example: timebin='30s'
                         '10' means '10s'
                """

	@staticmethod
	def timerange():
		""" Select time range for subset of data.

	              default = '' (all); examples,
       		       timerange = 'YYYY/MM/DD/hh:mm:ss~YYYY/MM/DD/hh:mm:ss'
       		       Note: if YYYY/MM/DD is missing dat defaults to first day in data set
       		       timerange='09:14:0~09:54:0' picks 40 min on first day
       		       timerange= '25:00:00~27:30:00' picks 1 hr to 3 hr 30min on next day
       		       timerange='09:44:00' data within one integration of time
       		       timerange='>10:24:00' data after this time


		Note: currently for flagdata task, this must be in the format:
		dd-mmm-yyyy/hh:mm:ss.s

		help par.selectdata will provide extra syntax.

		"""

	@staticmethod
	def title():
		"""
       		title -- Plot title (above plot)
               	default: ''; example: title='This is my title'
		"""

	@staticmethod
	def tmax():
		"""
		tmax -- maximum value used for process
                default: 0.0 (no threshold in maximum)
                example: 10.0 (mask data larger value than 10.0)
		"""
		
	@staticmethod
	def tmin():
		"""
		tmin -- minimum value used for process
                default: 0.0 (no threshold in minimum)
                example: -10.0 (mask data smaller value than -10.0)
		"""
		
	@staticmethod
	def transfer():
		"""
       		transfer -- Transfer field name(s)
               The names of the fields with a unknown flux densities.  These should
               be point-like calibrator sources.
               The syntax is similar to field.  Hence source index or
               names can be used.
               default: none: all calibrators must be listed.
               example: transfer='1445+099, 3C84'; transfer = '0,4'
               Note: to concatenate two strings
                     str1 = '1445+099'; str2 = '3C84'
               example: transfer = str1 + ', ' + str2

               NOTE: All sources in reference and transfer must have gains in the
               caltable fit.
		"""

	@staticmethod
	def tweight():
		"""
        tweight -- weighting for time average
            options:
                'var'      = 1/var(spec) weighted
                'tsys'     = 1/Tsys**2 weighted
                'tint'     = integration time weighted
                'tintsys'  = Tint/Tsys**2
                'median'   = median averaging
                default: 'tintsys'
		"""
		
	@staticmethod
	def uvtaper():
		"""
        	uvtaper -- Apply additional filtering/uv tapering of the visibilities.
                defalt=False; example: uvtaper=True

        	--- uvtaper parameters
		outertaper -- taper the outer edge of the uv-coverage
		the values can be in terms of uv units or psf units
		default []; example outertaper=['4klambda', '4klambda', '0deg']
		example outertaper=['20arcsec', '10arcsec', '10deg']
		"""


	@staticmethod
	def uvrange():
		"""
		      uvrange -- Select data within uvrange
              		default: '' (all); example:
              		uvrange='0~1000klambda'; uvrange from 0-1000 kilolambda
              		uvrange='>4klambda';uvranges greater than 4 kilolambda
		See help par.selectdata for additional syntax.
		"""

	@staticmethod
	def velocity():
		"""
        	velocity -- Prefer velocity (rather than frequency) as spectral axis
                default: False

		"""

	@staticmethod
	def verbose():
		""" List each observation in addition to the summary (True or False): """

	@staticmethod
	def verify():
		"""
		verify -- verify the results. 
                options: (bool) True,False
                default: False
		"""
		
	@staticmethod
	def verifybl():
		"""
		verifybl -- verify the results of baseline fitting
                options: (bool) True,False
                default: False
		"""

	@staticmethod
	def verifycal():
		"""
		verifycal -- verify the results of calibration
                options: (bool) True,False
                default: False
                WARNING: Currently verifying parameters just asks whether you 
                         accept the displayed calibraion/fit and if not, 
                         continues without doing any calibraion/baseline fit.
		"""
		
	@staticmethod
	def verifysm():
		"""
		verifysm -- verify the results of smoothing
                options: (bool) True,False
                default: False
		"""
		
	@staticmethod
	def versionname():
		"""
        	versionname -- Flag version name
                default: none; example: versionname='original_data'
                No imbedded blanks in the versionname
		"""

	@staticmethod
	def vis():
		""" Input visibility file; CASA MeasurementSet on disk: """

	@staticmethod
	def visnorm():
		""" Normalize the assembled spectral data, in a per baseline manner. If visnorm=True is
		used, this will have the effect of removing any non-frequency dependent closure errors 
		(e.g., as caused by source structure, or introduced by the instrument) from the data,
		and should be used with caution. When visnorm=False is used, closure errors in the data
		(as supplied to the solver) may be visible in the form of offsets between the data and
		solutions. For bandpass calibration, this is usually okay, as the shape of the bandpass
		is the most important aspect of the solution.

		"""

	@staticmethod
	def weighting():
		""" Weighting to apply to visibilities:

		Options: 'natural','uniform','briggs','radial','superuniform'

		"""

	@staticmethod
	def whichhdu():
		"""
        	whichhdu -- If fits file contains multiple images,
                choose this one
                default=0 mean first; example: whichhdu=1
		"""

	@staticmethod
	def whichrep():	
		"""
        	whichrep -- If fits image has multiple coordinate reps,
                choose one.
                default: 0 means first; example: whichrep=1
		"""

	@staticmethod
	def width():
		""" Channel width (value>1 indicates channel averaging: """

	@staticmethod
	def windowsize():
		"""
       		windowsize -- Window size
               	default: 1.0; example: windowsize=0.5
		"""

	@staticmethod
	def writestation():
		"""
        	writestation -- Write station name instead of antenna name
                default: True;
		"""

	@staticmethod
	def writesyscal():
		"""
        	writesyscal -- Write GC and TY tables
                default: False; system temparature and gain tables.
		"""

	@staticmethod
	def xaxis():
		""" 
		xaxis -- Visibility file (MS) data to plot along the x-axis
                default: 'time'
                Options: 
                   'azimuth, 'elevation','baseline','hourangle','parallacticangle',
                   'uvdist','time','u','v','w','x'
                   If plotting versus a data quantity (yaxis='amp'), add options:
                   'channel','real','imag','amp','phase'
                   Note: xaxis='x' is equivalent to running taskname='plotants'
        	yaxis -- Visibility data to plot along the y-axis
                default: 'amp'
                   Data quantity options: 'amp','phase','real','imag'
                   Other options: 'azimuth','elevation','baseline','hourangle','parallacticangle',
                   'u','v','w','uvdist'
		"""

	@staticmethod
	def xlabels():
		"""
       		xlabels -- Label for x axis
               	default: ''; example: xlabels='X Axis'
		"""

	@staticmethod
	def unflag():
		""" Option to unflag data rather than flag it (flagdata task): """

	@staticmethod
	def yaxis():
		""" 
                xaxis -- Visibility file (MS) data to plot along the x-axis
                default: 'time'
                Options:
                   'azimuth, 'elevation','baseline','hourangle','parallacticangle',
                   'uvdist','time','u','v','w','x'
                   If plotting versus a data quantity (yaxis='amp'), add options:
                   'channel','real','imag','amp','phase'
                   Note: xaxis='x' is equivalent to running taskname='plotants'
                yaxis -- Visibility data to plot along the y-axis
                default: 'amp'
                   Data quantity options: 'amp','phase','real','imag'
                   Other options: 'azimuth','elevation','baseline','hourangle','parallacticangle',
                   'u','v','w','uvdist'
		"""

	@staticmethod
	def ylabels():
		"""
       		ylabels -- Label for y axis
               	default: ''; example: ylabels='Y Axis'
		"""

	@staticmethod
	def zeroblanks():
		"""
        	zeroblanks -- Set blanked pixels to zero (not NaN)
                default=True; example: zeroblanks=True
		"""

	@staticmethod
	def selectdata():
               """
               selectdata()
               CASA selectdata syntax (March 13, 2008):

               In CASA, there is a common data selection syntax that is used for the all
               tasks that need a specific subset of the data from visibility data (measurement
               set).  These are:

                       field:  the selection of names associated with a source or mosaic group.
                       spw:    the selection of spectral windows and channels associated with
                               all sources specified by field
                       selectdata: The subset of other parameters which may be needed for the
                               appropriate selection of data.
           
               The field and spw parameters are always visible when displaying the input parameters,
               but the additional parameters are only visible when <selectdata = true>.
           
               The input and help file documentation for each of the tasks should provide an adequate
               description of the data selection syntax needed to process the appopriate data for
               most applications.  However, the data selection capabilities are powerful and still
               evolving, hence a full description of the syntax is needed, and given below.
           
               All of the parameter values must be placed within quotes since they are formally
               strings, even though they often represent integers.  The string property allows a
               richer set of parameter usage.  The field and antenna parameters can be either the
               field_id or antenna_id (the array index used in CASA for this parameter) or the actual
               names of the field or antenna.   For these two parameters, if the string is a
               non-negative integer, it is interpreted as an array index.  Otherwise, it is interpreted
               as the name.  REMEMBER, CASA INDICES START WITH 0.  Illegal strings are ',',';','"','/'.
           
               Execute <inp='plotxy'>, for example, to determine the parameters of the visibility data.
           
               field -- The field names (sources) processed by the task or tool:
           
                       field = ''             data for all field_id's
                       field = '1'            data for field_id = 1
                       field = 'P1151+3435'   data for field P1151+3435
                       field = '2~4'          data for field_id's 2,3,4
                       field = '2,3,4'        data for field_id's 2,3,4
                       field = 'P11*,3,4~6'   data for field_id's 3,4,5,6 and any source name
                                              beginning with P11
                       field = '*11,8'        data for field_id 8 and any source ending with 11
           
               spw -- The spectral window/channel selection for all fields to be processed:
           
                       spw = '0'              spectral window_id=0, all channels
                       spw = '0:0~63'         sp id=0, channels 0 to 63, INCLUSIVE.
                       spw = '0,1,4~7'        sp id's=0,1,4,5,6,7, all channels
                       spw = '*:3~64'         channels 3 through 64 for all sp id's
                                              spw = ' :3~64' will NOT work.
                       spw = '*:0;60~63'      channel 0 and channels 60,61,62,63 for all IF's 
                                              ';' needed to separate different channel ranges in one spw
                       spw = '0:34, 2:10~12,3~4:0~33'
                                              sp id=0, channel 34; sp id=2, channels 10,11,12;
                                              sp id's 3 and 4, channels 0 through 33, inclusive.
           
                      There is also a skipping parameter, denoted by '^'
           
                       spw = '0:0~14^3'       sp id=0, every third channel in 0 to 14
                                                 i.e. channels 0,3,6,9,12
                       spw = '0:0~6^2 ,0:6~38^4'
                                              sp id=1. channels, 0,2,4,6,10,14,18,22,26,30,34,38
                       spw = '*:^2'          all spw, every second channel
           
                       NOTE:  Channel ranges in velocity or frequency have limited functionality.
           
           
               selectdata = true          will open some or all of the following parameters:
           
               antenna -- The antennas and/or baselines for all fields/spw to be processed:
           
                       NOTE:  Some tasks require specific antennas, some tasks require specific
                               baselines.  See help for each task.
                       NOTE:  Antennas can be designated by index or by antenna name depending on
                               how the data were written into CASA.
           
                       antenna-oriented syntax:
                       antenna = ''               all antennas and (of course) baselines
                       antenna = '0,2~18'         baselines that contain antenna indices 0, or 2 thru 18.
                       antenna = '0,VA05'         baselines that contain antenna index 0 and VLA antenna 5
                       antenna = 'VA*,EA13,EA14'  baselines that contain all VLA antennas and
                                                  EVLA antennas 13,14
                       antenna = 'MK,SC,PT,LA'    baselines that contain VLBA antennas at MK, SC
                                                  PT, LA
           
                       baseline-oriented syntax:
                       antenna = '0 & 1'            baseline with antenna index 0 and 1
                       antenna = 'VA05 & VA06'      baseline with VLA antenna 5 and 6
                       antenna = 'VA* & EV*'        baselines between VLA and EVLA antennas
                       antenna = '0~5 & 23~25,27'   all baselines betweeen antennas 0 through 5
                                                    versus 23 through 25, and all baselines
                                                    associated with 27.
           
               timerange -- The time range to be processed:
           
                       The time syntax is   T = 'YYYY/MM/DD/HH:MM:SS.FF'
                       Most applications will use a time range such as:
           
                       timerange = 'T0~T1'                 timer range between T0 and T1
                       timerange = '2007/5/3/5:44:13.33 ~ 2007/5/3/6:44:13.33'
                       timerange = '5:44:13.33 ~ 6:44:13.33'  same as above if the earliest visibility
                                                           data point is on day 2007/5/3
           
                       timerange = '5:44:13.33 + 0:2:0'    Time from 5:44:13.33 to 2 min later
                       timerange = '>6:12:20.0'            Time after 6:12:20
                       timerange = '<8:12:20.0'            Time before 8:12:20 (on data of first data point)
                       timerange = '2007/5/3/5:44:13.33'   Time stamp within one data sampling time interval
           
                       Other short-cuts
                            '2007/06/03/6'      2007June03 at 6h
                            '2007/06/03/5:44'   2007June03 at 5h44m
                            
                       if part of the year, month and day are missing, the date (egs. 2000/06/03) of the
                       earliest visibility data point is assumed:
           
                            03/5:44           2007/06 assumed
                            23:24:23          2007/06/03 assumed
                            25:18:44          2007/06/03 assumed, i.e
                                                  2007/06/04/01:18:44


               correlation --  the correlators to be processed:
           
                       Antenna-based correlations are: 'R', 'L', 'X', 'Y'
                       Baseline-base correlations are: 'RR', 'LL', 'RL', 'LR',
                                          'XX', 'YY', 'XY', 'YX'
                       Various combinations are allowed, and these are task specific
                       egs.  correlation = 'RR LL' in plotxy
           
               uvrange -- The uvrange to be processed:
           
                       uvrange = 'UVMIN ~ UVMAXklambda'        uvrange from UVMIN to UVMAX in klambda
                       uvrange = 'UVMIN ~ UVMAXkm'        uvrange from UVMIN to UMMAX in km
                                                          klambda, km, m, ml, l available
                       uvrange = '<100km'                 uvrange less than 100 km
                       uvrange = '<100km,>1000km'         uvrange less than 100 km, but more than 1000 km
           
                       The default units for uvrange are METERS!
           
               scan -- The scan range to be processed:
           
                       Syntax associated with non-negative integers
           
                       scan = '0~20'                     scan range 0 through 20
                       scan = '2,5,8'                    scans 2, 5 and 8
           
               feed -- The multi-feed number to be processed
           
                        Not yet implemented.  Syntax associated with non-negative integers
           
                       feed = '0~20'                     feed range 0 through 20
                       feed = '2,5,8'                    feeds 2, 5 and 8
           
               array -- The array number associated with the data
           
                        Not yet implemented.
               """

        @staticmethod
        def singledish():
                """
                singledish -- Set True to write data as single-dish format (Scantable)
                              default: False
                              task: importasdm
                """

        @staticmethod
        def varlist():
                """
                varlist -- Dictionary of variables used in expr (mathematical
                           expression) and their values. Keys must be coincide with
                           variables used in expr. Values are substituted in each
                           value in expr.
                           default: {} (empty dictionary)
                           task: sdmath
                """
