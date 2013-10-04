import time
import os
from regressframe import regressionframe

class regressverify :
	datarepos = '/home/casa/data/trunk'
	def fill(self) :
		rstat = True 
		return rstat
	def flag(self) :
		rstat = True 
		return rstat
	def calibrate(self) :
		rstat = True 
		return rstat
	def image(self) :
		rstat = True 
		return rstat
	def analysis(self) :
		rstat = True 
		return rstat


pipeline = regressionframe()
verify = regressverify()

pipeline.datarepos ='/home/casa/data/trunk'
pipeline.workdir ='./g192'
datapath = pipeline.datarepos+'/regression/ATST1/G192/'

pipeline.fill['tasks'] = ['importvla']
pipeline.fill['importvla'] = {}
pipeline.fill['importvla']['args'] = {'archivefiles':[datapath + 'AS758_C030425.xp1',datapath+'AS758_C030425.xp2',datapath+'AS758_C030425.xp3',datapath+'AS758_C030426.xp4',datapath+'AS758_C030426.xp5'],
	                              'vis':'g192_a.ms',
				      'bandname':'K',
				      'frequencytol':10000000.0}
pipeline.fill['verify'] = verify.fill

pipeline.flag['tasks'] = ['tflagdata']
pipeline.flag['tflagdata'] = {}
pipeline.flag['tflagdata']['args'] = {'vis':'g192_a.ms',
									'mode':'manual',
									'autocorr':True}
pipeline.flag['verify'] = verify.flag

pipeline.calibrate['tasks'] = ['setjy', 'gencal', 'gaincal', 'bandpass', 'fluxscale', 'applycal']
pipeline.calibrate['setjy'] = {}
pipeline.calibrate['gencal'] = {}
pipeline.calibrate['gaincal'] = {}
pipeline.calibrate['bandpass'] = {}
pipeline.calibrate['fluxscale'] = {}
pipeline.calibrate['applycal'] = {}
pipeline.calibrate['setjy']['args'] = {'vis':'g192_a.ms',
				       'field':'4',
				       'standard':'Perley-Taylor 99'
				       }

pipeline.calibrate['gencal']['args'] = {'vis':'g192_a.ms',
					'caltable':'g192_a.opac',
					'caltype':'opac',
					'parameter':[0.062]
					}

pipeline.calibrate['gaincal']['args'] = {'vis':'g192_a.ms',
					 'caltable':'g192_a.gcal',
					 'field':'0,2,3,4',
					 'spw':'0:3~117',
					 'gaintype':'G',
					 'solint':'inf',
					 'combine':'',
					 'refant':'VA05',
					 'gaintable':'g192_a.opac'
					 }
pipeline.calibrate['bandpass']['args'] = {'vis':'g192_a.ms',
	                               'caltable':'g192_a.bcal',
	                               'field':'3',
	                               'gaintable':['g192_a.opac','g192_a.gcal'],
	                               'gainfield':['','3'],
	                               'interp':['','nearest'],
				       'solint':'inf',
				       'combine':'scan',
                                       'refant':'VA05' 
	                              }
pipeline.calibrate['fluxscale']['args'] = {'vis':'g192_a.ms',
	                              'caltable':'g192_a.gcal',
				      'fluxtable':'g192_a.fluxcal',
				      'reference':['1331+305'],
				      'transfer':['0530+135','05309+13319']
	                              }
pipeline.calibrate['applycal']['args'] = {'vis':'g192_a.ms',
	                               'field':'0,1,2',
				       'gaintable':['g192_a.opac','g192_a.fluxcal','g192_a.bcal'],
				       'gainfield':['','0,2']
	                              }
pipeline.calibrate['verify'] = verify.calibrate

pipeline.image['tasks'] = ['split', 'split', 'clean']
pipeline.image['split'] = {0:{}, 1:{}}
pipeline.image['split'][0]['args'] = {'vis':'g192_a.ms',
		                      'outputvis':'g192_cal.split.ms',
				      'field':'4',
				      'spw':'0:9~108',
				      'datacolumn':'corrected'
				      }
pipeline.image['split'][1]['args'] = {'vis':'g192_a.ms',
                                      'outputvis':'g192_src.split.ms',
		                      'field':'1',
		                      'spw':'0:9~108',
		                      'datacolumn':'corrected'
		                      }
pipeline.image['clean'] = {}
pipeline.image['clean']['args'] = {'vis':'g192_src.split.ms',
		                   'imagename':'g192_a2',
		                   'mode':'channel',
		                   'psfmode':'hogbom',
		                   'niter':5000,
		                   'gain':0.1,
		                   'threshold':15.0,
		                   'mask':'',
		                   'nchan':40,
		                   'start':34,
		                   'width':1,
		                   'field':'0',
		                   'spw':'0',
		                   'imsize':[512,512],
		                   'cell':[0.5,0.5],
		                   'weighting':'natural'
				   }
pipeline.image['verify'] = verify.image

pipeline.analysis['tasks'] = ['exportfits']
pipeline.analysis['exportfits'] = {}
pipeline.analysis['exportfits']['args'] = {'imagename':'g192_a2.image',
	                                 'fitsimage':'g192_a2.fits'}
pipeline.analysis['verify'] = verify.analysis
pipeline.run("G192 regression")
