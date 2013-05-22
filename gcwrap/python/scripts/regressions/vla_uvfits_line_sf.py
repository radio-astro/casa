#
# Driver script for VLA-uvfits-spect, old ngc5921_regression
#
import time
import os
from regressframe import regressionframe

class regressverify :

	##
	## try paths, which are known to include the whole repository
	##
	if os.path.exists('/opt/casa/data'):
		datarepos = '/opt/casa/data'
	elif os.path.exists('/home/casa/data/trunk'):
		datarepos = '/home/casa/data/trunk'
	elif os.path.exists(os.environ['CASAPATH'].split()[0] + "/data"):
		datarepos = os.environ['CASAPATH'].split()[0] + "/data"
	else:
		raise Exception('cannot find data repository')

	print 'datarepos = ', datarepos
	def fill(self) :
		rstat = True
		print "Starting fill verification"
		ms.open('ngc5921.ms')
		if(ms.nrow() != 22653) :
			rstat = False
			print 'ms.nrow failed: got ',ms.nrow, ' expected 22653'
		suminfo = ms.summary()
		if suminfo['nfields'] != 3 :
			rstat = False
			print 'nfields failed got '+suminfo['nfields']+' expected 3'
		if suminfo['field_0']['name'] != '1331+30500002_0' :
			rstat = False
			print 'nfield_0 failed got '+suminfo['field_0']['name']
		if suminfo['field_1']['name'] != '1445+09900002_0' :
			rstat = False
			print 'nfield_1 failed got '+suminfo['field_1']['name']
		if suminfo['field_2']['name'] != 'N5921_2' :
			rstat = False
			print 'nfield_2 failed got '+suminfo['field_2']['name']
		if suminfo['timeref'] != 'TAI' :
			rstat = False
			print 'timeref failed got '+suminfo['timeref']
		ms.close()
		return rstat
	def flag(self) :
		rstat = True
		print "Starting flagging verification"
		flagsummary = tflagdata('ngc5921.ms',mode='summary')
		if(flagsummary['flagged'] != 203994) :
		   rstat = false
		   print 'flagged failed got ', flagsummary['flagged'], ' expected 203994'
		if(flagsummary['total'] != 2854278) :
		   rstat = false
		   print 'flagged failed got ', flagsummary['flagged'], ' expected 2854278'
		return rstat
	def calibrate(self) :
		import listing
		rstat = True
		print "Starting calibrate verification"
		# Before testing listcal output, remove first line of file
		# (First line contains hard-coded path to input files)
		listcalOut = 'ngc5921.listcal.out'
		os.system('mv ' + listcalOut + ' ' + listcalOut + '.tmp')
		os.system('tail -n +2 ' + listcalOut + '.tmp > ' + listcalOut)
		os.system('rm -f ' + listcalOut + '.tmp')

		# Test the listcal output
		print "Comparing listcal output with standard..."
		standardOut = self.datarepos+'/regression/ngc5921/listcal.default.out'

		# Test metadata
		if (not listing.diffMetadata(listcalOut,standardOut,prefix="ngc5921.listcal")):
		    rstat = False
		    print "failed calibrate meta data comparison"

                # Test data (floats)
                precision = '0.003'
                if (not  listing.diffAmpPhsFloat(listcalOut,standardOut,prefix="ngc5921.listcal",
					                                 precision=precision) ):
                    rstat = False
		    print "failed calibrate test data comparison"

		return rstat
	def image(self) :
		rstat = True
		print "Starting image verification"
		cubestats = imstat(imagename='ngc5921.clean.image')
		if((cubestats['max'][0] - 0.052414759993553162)/0.052414759993553162 > 0.05) :
			print "failed clean image max test"
			rstat = False
		if((cubestats['rms'][0] - 0.0020218724384903908)/0.0020218724384903908 > 0.05) :
			print "failed clean image rms test"
			rstat = False
		return rstat
	def analysis(self) :
		rstat = True
		print "Starting analysis verification"
                momzerostats = imstat(imagename='ngc5921.moments.integrated')
		if((momzerostats['max'][0] - 1.48628211)/1.48628211 > 0.05) :
			print "failed momzero max test"
			rstat =False
                momonestats = imstat(imagename='ngc5921.moments.weighted_coord')
		if((momonestats['mean'][0] - 1484.22001478)/1484.22001478 > 0.05) :
			print "failed momone mean test"
			rstat =False
		return rstat


pipeline = regressionframe()
verify = regressverify()

##
## try paths, which are known to include the whole repository
##
if os.path.exists('/opt/casa/data'):
	pipeline.datarepos = '/opt/casa/data'
elif os.path.exists('/home/casa/data/trunk'):
	pipeline.datarepos = '/home/casa/data/trunk'
elif os.path.exists(os.environ['CASAPATH'].split()[0] + "/data"):
	pipeline.datarepos = os.environ['CASAPATH'].split()[0] + "/data"
else:
	raise Exception('cannot find data repository')

pipeline.workdir = './ngc5921_regression'
pipeline.fill['tasks'] = ['importuvfits', 'listobs']
pipeline.fill['importuvfits'] = {}
pipeline.fill['listobs'] = {}
pipeline.fill['importuvfits']['args'] = {'vis':'ngc5921.ms',
		      'fitsfile':pipeline.datarepos+'/regression/ngc5921/ngc5921.fits',
		      'antnamescheme':'new'
		      }
pipeline.fill['listobs']['args'] = {'vis':'ngc5921.ms',
		                         'verbose':True
					 }

pipeline.fill['verify'] = verify.fill

pipeline.flag['tasks'] = ['tflagdata']
pipeline.flag['tflagdata'] = {}
pipeline.flag['tflagdata']['args'] = {'vis':'ngc5921.ms',
									'mode':'manual',
									'autocorr':True}
pipeline.flag['verify'] = verify.flag

pipeline.calibrate['tasks'] = ['setjy', 'bandpass', 'gaincal', 'listcal', 'fluxscale', 'applycal', 'applycal', 'split', 'split', 'exportuvfits']
pipeline.calibrate['applycal'] = {}
pipeline.calibrate['setjy'] = {}
pipeline.calibrate['bandpass'] = {}
pipeline.calibrate['gaincal'] = {}
pipeline.calibrate['listcal'] = {}
pipeline.calibrate['verify'] = verify.calibrate
pipeline.calibrate['fluxscale'] = {}
pipeline.calibrate['setjy']['args'] = {'vis':'ngc5921.ms',
				       'field' : '1331+305*',
				       'modimage' : '',
				       'standard':'Perley-Taylor 99'
				       }
pipeline.calibrate['bandpass']['args'] = {'vis':'ngc5921.ms',
		                    'caltable':'ngc5921.bcal',
				    'gaintable' :'',
				    'gainfield' : '',
				    'interp' : '',
				    'field' : '0',
				    'spw' : '',
				    'selectdata' : False,
				    'bandtype' : 'B',
				    'solint' : 'inf',
				    'combine' : 'scan',
				    'refant' : 'VA15'
		                   }
pipeline.calibrate['gaincal']['args'] = {'vis':'ngc5921.ms',
		                   'caltable':'ngc5921.gcal',
				   'gaintable' : 'ngc5921.bcal',
				   'gainfield' : '',
				   'interp' : 'nearest',
				   'field' : '0,1',
				   'spw' : '0:6~56',
				   'selectdata' : False,
				   'gaintype' : 'G',
				   'solint' : 'inf',
				   'combine' : '',
				   'calmode' : 'ap',
				   'minsnr' : 1.0,
				   'refant' : '15'
                                  }
pipeline.calibrate['listcal']['args'] = {'vis':'ngc5921.ms',
		                   'caltable':'ngc5921.gcal',
				   'listfile':'ngc5921.listcal.out'
				   }
pipeline.calibrate['fluxscale']['args'] = {'vis':'ngc5921.ms',
		                     'fluxtable':'ngc5921.fluxscale',
				     'caltable':'ngc5921.gcal',
				     'reference' : '1331*',
				     'transfer' : '1445*'
                                    }
pipeline.calibrate['applycal'] = {0:{}, 1:{}}
pipeline.calibrate['applycal'][0]['args'] = {'vis':'ngc5921.ms',
		                    'gaintable':['ngc5921.fluxscale','ngc5921.bcal'],
				    'interp' : ['linear', 'nearest'],
				    'spwmap' : [],
				    'spw' : '',
				    'selectdata' : False,
				    'field' : '1,2'
                                   }
pipeline.calibrate['applycal'][1]['args'] = {'vis':'ngc5921.ms',
		                    'gaintable':['ngc5921.fluxscale','ngc5921.bcal'],
				    'interp' : ['linear', 'nearest'],
				    'spwmap' : [],
				    'spw' : '',
				    'selectdata' : False,
				    'field' : '0',
				    'gainfield':['0','*']
		                   }
pipeline.calibrate['split'] = {0:{}, 1:{}}
pipeline.calibrate['split'][0]['args'] = {'vis':'ngc5921.ms',
				    'outputvis' : 'ngc5921.cal.split.ms',
				    'field' : '1445*',
				    'spw':'',
				    'datacolumn':'corrected',
		                   }
pipeline.calibrate['split'][1]['args'] = {'vis':'ngc5921.ms',
				    'outputvis' : 'ngc5921.src.split.ms',
				    'field' : 'N5921*',
				    'spw':'',
				    'datacolumn':'corrected'
		                   }
pipeline.calibrate['exportuvfits'] = {}
pipeline.calibrate['exportuvfits']['args'] = { 'vis' : 'ngc5921.src.split.ms',
		                               'fitsfile' : 'ngc5921.split.uvfits',
				               'datacolumn':'data',
					       'multisource':True,
				               'async':True
		                             }

pipeline.image['verify'] = verify.image
pipeline.image['tasks'] = ['uvcontsub', 'clean', 'exportfits']
pipeline.image['uvcontsub'] = {}
pipeline.image['uvcontsub']['args'] = {'vis':'ngc5921.ms',
		                 'field' : 'N5921*',
				 'fitspw':'0:4~6;50~59',
				 'spw':'0',
				 'solint':'int',
				 'fitorder':0
		                }
pipeline.image['clean'] = {}
pipeline.image['clean']['args'] = {'vis':'ngc5921.ms.contsub',
		                 'imagename':'ngc5921.clean',
				 'mode':'channel',
				 'nchan':46,
				 'start':5,
				 'width':1,
				 'field':'0',
				 'spw':'',
				 'gain':0.1,
				 'psfmode':'clark',
				 'cell':[15.,15.],
				 'niter':6000,
				 'threshold':8.0,
				 'weighting':'briggs',
				 'robust':0.5,
				 'mask':''
		                }
pipeline.image['exportfits'] = {}
pipeline.image['exportfits']['args'] = {'imagename':'ngc5921.clean.image',
		                        'fitsimage':'ngc5921.clean.fits',
		                        'async':True
		                       }

pipeline.analysis['verify'] = verify.analysis
pipeline.analysis['tasks'] = ['immoments']
pipeline.analysis['imstat'] = {0:{},1:{}}
pipeline.analysis['immoments'] = {}
pipeline.analysis['immoments']['args'] = {'imagename':'ngc5921.clean.image',
		                    'moments':[0,1],
				    'excludepix':[-100, 0.009],
				    'chans':'',
				    'outfile':'ngc5921.moments'
		                   }

pipeline.run("NGC5921 Regression")

#fill= rows, sources, spw
#flag          = number flagged
#calibrate = caltable check
#image = inner quadrant & rms
#analysis
