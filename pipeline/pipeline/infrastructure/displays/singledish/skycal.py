import os
import re

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.displays.bandpass as bandpass
import pipeline.infrastructure.casatools as casatools

LOG = logging.get_logger(__name__)

class SingleDishSkyCalDisplayBase(object):
    def init_with_field(self, context, result, field):
        vis = self._vis 
        ms = context.observing_run.get_ms(vis)
        num_fields = len(ms.fields)
        if field.isdigit() and int(field) < num_fields:
            self.field_id = int(field)
            self.field_name = ms.get_fields(self.field_id)[0].clean_name
        else:
            self.field_name = field
            fields = ms.get_fields(name=field)
            assert len(fields) == 1
            self.field_id = fields[0].id
        LOG.debug('field: ID %s Name \'%s\''%(self.field_id, self.field_name))
        old_prefix = self._figroot.replace('.png', '')
        self._figroot = self._figroot.replace('.png', '-%s.png'%(self.field_name))
        new_prefix = self._figroot.replace('.png', '')
        
        self._update_figfile(old_prefix, new_prefix)
            
        if not self._kwargs.has_key('field'):
            self._kwargs['field'] = self.field_id
            
    def add_field_identifier(self, plots):
        for plot in plots:
            if not plot.parameters.has_key('field'):
                plot.parameters['field'] = self.field_name
                
    def _update_figfile(self):
        raise NotImplementedError()


class SingleDishSkyCalSummaryChart(bandpass.BandpassAmpVsFreqSummaryChart, SingleDishSkyCalDisplayBase):
    def __init__(self, context, result, field):
        super(SingleDishSkyCalSummaryChart, self).__init__(context, result)
        
        self.init_with_field(context, result, field)
         
    def plot(self):
        self._kwargs['timeranges'] = '0'
        
        wrappers = super(SingleDishSkyCalSummaryChart, self).plot()
         
        self.add_field_identifier(wrappers)
        
        return wrappers
    
    def _update_figfile(self, old_prefix, new_prefix):
        for (ant_id, figfiles) in self._figfile.items():
            self._figfile[ant_id] = map(lambda x: x.replace(old_prefix, new_prefix), figfiles)
        
    
class SingleDishSkyCalDetailChart(bandpass.BandpassDetailChart, SingleDishSkyCalDisplayBase):
    def __init__(self, context, result, field):
        super(SingleDishSkyCalDetailChart, self).__init__(context, result,
                                                          xaxis='freq', yaxis='amp', showatm=True,
                                                          overlay='time')
        
        self.init_with_field(context, result, field)
    
    def plot(self):
        wrappers = super(SingleDishSkyCalDetailChart, self).plot()
         
        self.add_field_identifier(wrappers)
        
        return wrappers
    
    def _update_figfile(self, old_prefix, new_prefix):
        for spw_id in self._figfile.keys():
            for antenna_id, figfile in self._figfile[spw_id].items():
                new_figfile = figfile.replace(old_prefix, new_prefix)
                self._figfile[spw_id][antenna_id] = new_figfile 

        