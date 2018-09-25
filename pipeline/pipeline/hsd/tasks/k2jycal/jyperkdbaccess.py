from __future__ import absolute_import

import urllib
import urllib2
import json
import numpy
import os
import re
import string

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools

LOG = infrastructure.get_logger(__name__)

class JyPerKDatabaseAccessProto(object):
    BASE_URL = 'https://asa.alma.cl/science/jy-kelvins'
    HIGH_URL = BASE_URL + '/high-freq/'
    LOW_URL = BASE_URL + '/low-freq/'
    HIGH_BAND = set(['ALMA Band 8'])
    LOW_BAND = set(['ALMA Band {0}'.format(i) for i in xrange(3,8)])
    
    def __init__(self, context=None):
        self.context = context
    
    def _get_observing_band(self, ms):
        if self.context is None:
            return 'Unknown'

        spws = ms.get_spectral_windows(science_windows_only=True)
        bands = [spw.band for spw in spws]
        return numpy.unique(bands)
    
    def getJyPerK(self, vis):
        # get Jy/K value from DB
        jyperk = self.get(vis)
        
        # convert to pipeline-friendly format
        basename = os.path.basename(re.sub('\.ms$', '', vis.rstrip('/')))        
        formatted = self.format_jyperk(basename, jyperk)
        
        return formatted
    
    def get(self, vis):
        # receiver band is necessary to switch URL to be accessed
        ms = self.context.observing_run.get_ms(vis)
        observing_band = self._get_observing_band(ms)
        LOG.info('observing band is {0}'.format(observing_band))
        high_freq = not self.HIGH_BAND.isdisjoint(observing_band)
        low_freq = not self.LOW_BAND.isdisjoint(observing_band)
        
        if low_freq == True:
            LOG.info('Low frequency DB')
            url = self.LOW_URL
        if high_freq == True:
            LOG.info('High frequency DB')
            url = self.HIGH_URL
        # TODO: do different bands coexist in one MS?
        
        obs_start = ms.start_time['m0']
        obs_end = ms.end_time['m0']
        qa = casatools.quanta
        start_date = qa.sub(obs_start, qa.quantity(1.0, 'd'))
        end_date = qa.add(obs_end, qa.quantity(1.0, 'd'))
        str_start_date = qa.time(start_date, form='fits')[0]
        str_end_date = qa.time(end_date, form='fits')[0]
        testparams = dict(max_elevation=80.0, min_elevation=70.0,
                          band=3, 
                          start_date=str_start_date, end_date=str_end_date)
        encoded = urllib.urlencode(testparams)
        
        try:
            # try opening url
            response = urllib2.urlopen('?'.join([url, encoded]))
        except urllib2.HTTPError as e:
            msg = 'Failed to load URL: {0}\n'.format(url) \
                + 'Error Message: HTTPError(code={0}, Reason="{1}")\n'.format(e.code, e.reason)
            LOG.error(msg)
            raise e
        except urllib2.URLError as e:
            msg = 'Failed to load URL: {0}\n'.format(url) \
                + 'Error Message: URLError(Reason="{0}")\n'.format(e.reason)
            LOG.error(msg)
            raise e
            
        LOG.info('URL="{0}"'.format(response.geturl()))
        
        retval = json.load(response)
        # retval should be a dict that consists of
        # 'query': query data
        # 'total': number of data
        # 'data': data
        return retval
    
    def format_jyperk(self, basename, jyperk):
        template = string.Template('$vis $antenna $spw I $value')
        data = jyperk['data']
        factors = [map(str, template.safe_substitute(vis=basename, **d).split()) for d in data]
        return factors
