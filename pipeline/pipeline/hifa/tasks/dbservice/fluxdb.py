import os
import sys
import urllib
import csv
from xml.dom import minidom
import datetime

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
from pipeline.infrastructure import casa_tasks

#execfile('analysisUtils.py')
#import analysisUtils as au

LOG = infrastructure.get_logger(__name__)


'''
Write a pipeline task to retrieve source fluxes from the ALMA flux calibrator 
catalog, replacing those derived from the source table.
The task must quit gracefully if it cannot connect to the calibrator catalog.

https://bugs.nrao.edu/browse/CAS-6869
'''

#Sample URL
# http://bender.csrg.cl:2121/bfs-0.2/ssap?NAME=3c279&DATE=04-Apr-2014&FREQUENCY=231.435E9
#Documentation
# http://twiki.csrg.cl/twiki/bin/view/LIRAE/SourceCatalogueVO


class FluxdbInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, serviceurl=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())

    @property
    def serviceurl(self):
        return self._serviceurl

    @serviceurl.setter
    def serviceurl(self, value):
        if value is None:
            #value = 'http://bender.csrg.cl:2121/bfs-0.2/ssap'
            value = 'http://asa-test.alma.cl/bfs/'
        self._serviceurl = value
        
    
class FluxdbResults(basetask.Results):
    def __init__(self, fluxtable=[]):
        super(FluxdbResults, self).__init__()

        self.fluxtable = fluxtable[:]
        print self.fluxtable


        
    def merge_with_context(self, context):
        """Replace flux source table with new results from the database
        """
        LOG.info("FLUX CAL WEB SERVICE MERGING WITH CONTEXT")

    

class Fluxdb(basetask.StandardTaskTemplate):
    Inputs = FluxdbInputs
    
    def prepare(self):
        inputs = self.inputs
        context = inputs.context
        fluxtable = []

        for ms in context.observing_run.measurement_sets:
            for field in ms.get_fields(intent='ATMOSPHERE, BANDPASS, POINTING, SIDEBAND, AMPLITUDE,PHASE'):
                for flux in field.flux_densities:
                    LOG.info(" ")
                    LOG.info(" ")
                    (I, Q, U, V) = flux.casa_flux_density
                    sourcename = field.name
                    LOG.info(str(field.id) + '  ' + field.name + '  ' + str(I))
                    
                    try:
			frequency = str(flux.spw_id.centre_frequency.value)
			LOG.info(frequency)
	            except:
	                LOG.warn("Unable to parse frequency for some fields.")
		
		    try:
			#From domain/observingrun.py
			qt = casatools.quanta
			mt = casatools.measures
			s = qt.time(mt.getvalue(ms.start_time)['m0'], form=['fits'])
			dt = datetime.datetime.strptime(s[0], '%Y-%m-%dT%H:%M:%S')
			
			year = dt.year
			month = dt.strftime("%B")
			day = dt.day
			date = str(day) + '-' + month + '-' + str(year)
		    
			urlparams = self.buildparams(sourcename, date, frequency)
			
			try:
			    try:
                                dom =  minidom.parse(urllib.urlopen(inputs.serviceurl + '?%s' % urlparams))
                                LOG.info(inputs.serviceurl + '?%s' % urlparams)
                            except:
                                LOG.warn("Unable to connect to ALMA flux database.")
			    
			    domtable = dom.getElementsByTagName('TR')

			    
			    for node in domtable:
				row = node.getElementsByTagName('TD')
				rowdict = {}
				rowdict['sourcename']         = row[0].childNodes[0].nodeValue
				rowdict['dbfrequency']        = row[1].childNodes[0].nodeValue
				rowdict['date']               = row[2].childNodes[0].nodeValue
				rowdict['fluxdensity']        = row[3].childNodes[0].nodeValue
				rowdict['fluxdensityerror']   = row[4].childNodes[0].nodeValue
				rowdict['spectralindex']      = row[5].childNodes[0].nodeValue
				rowdict['spectralindexerror'] = row[6].childNodes[0].nodeValue
				rowdict['error2']             = row[7].childNodes[0].nodeValue
				rowdict['error3']             = row[8].childNodes[0].nodeValue
				rowdict['error4']             = row[9].childNodes[0].nodeValue
				rowdict['warning']            = row[10].childNodes[0].nodeValue
				rowdict['notms']              = row[11].childNodes[0].nodeValue
				rowdict['verbose']            = row[12].childNodes[0].nodeValue
				rowdict['url']                = inputs.serviceurl + '?%s' % urlparams
				
				rowdict['fieldid']         = str(field.id)
				rowdict['fieldname']       = field.name
				rowdict['fieldfluxI']      = str(I)
				rowdict['frequency']       = str(frequency)
				rowdict['spw']             = str(flux.spw_id.id)
				
				rowdict['aufluxDensity'] = ' '
				rowdict['aufluxDensityUncertainty'] = ' '
				rowdict['aufrequency'] = ' '
				rowdict['aumeanAge'] = ' ' 
				rowdict['auspectralIndex'] = ' '
				rowdict['auspectralIndexUncertainty'] = ' '
				
				#Compare to Todd's values
				try:
				    audict = au.getALMAFluxForMS(vis=inputs.vis, field=str(field.id), spw=str(flux.spw_id.id))
				    for key in audict.keys():
				        rowdict['aufluxDensity'] = str(audict[key]['fluxDensity'])
				        rowdict['aufluxDensityUncertainty'] = str(audict[key]['fluxDensityUncertainty'])
				        rowdict['aufrequency'] = str(audict[key]['frequency'])
				        rowdict['aumeanAge'] = str(audict[key]['meanAge'])
				        rowdict['auspectralIndex'] = str(audict[key]['spectralIndex'])
				        rowdict['auspectralIndexUncertainty'] = str(audict[key]['spectralIndexUncertainty'])
				        
				except:
				    LOG.info("Unable to use analysisUtils")
				
				fluxtable.append(rowdict)
		            LOG.info("Successful web service call")
		            LOG.info(" ")
		        except:
			     LOG.warn("Unable to parse some XML service field or null results.")

                    except:
                        LOG.warn("Warning received - data might be unavailable for some fields.")

        return FluxdbResults(fluxtable=fluxtable)

    def analyse(self, result):
        return result


    def buildparams(self, sourcename, date, frequency):
        
        params = {'NAME' : sourcename,
                  'DATE' : date,
                  'FREQUENCY' : frequency}
                       
        urlparams = urllib.urlencode(params)
    
        return urlparams