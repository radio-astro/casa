import collections

import pipeline.infrastructure.logging as logging
import pipeline.h.tasks.importdata.renderer as super_renderer
import pipeline.infrastructure.utils as utils

LOG = logging.get_logger(__name__)

DeductionGroupTR = collections.namedtuple('ReductionGroupTR', 'id fmin fmax field msname antenna spw nchan')

class T2_4MDetailsSingleDishImportDataRenderer(super_renderer.T2_4MDetailsImportDataRenderer):
    def __init__(self, template='hsd_importdata.mako',
                 description='Register measurement sets with the pipeline', 
                 always_rerender=False):
        super(T2_4MDetailsSingleDishImportDataRenderer, self).__init__(template, description, always_rerender)
        
    def update_mako_context(self, mako_context, pipeline_context, result):
        super(T2_4MDetailsSingleDishImportDataRenderer, self).update_mako_context(mako_context, pipeline_context, result)

        # collect antennas of each MS and SPW combination
        row_values = []
        for (group_id, group_desc) in pipeline_context.observing_run.ms_reduction_group.items():
            min_freq = '%7.1f'%(group_desc.min_frequency/1.e6)
            max_freq = '%7.1f'%(group_desc.max_frequency/1.e6)

            # ant_collector[msname][spwid] = [ant1_name, ant2_name, ...]
            ant_collector =  collections.defaultdict(lambda: collections.defaultdict(lambda: []))
            for m in group_desc:
                ant_collector[m.ms.basename][m.ms.spectral_windows[m.spw_id].id].append(m.ms.antennas[m.antenna_id].name)

            # construct 
            for msname, ant_spw in ant_collector.items():
                for spwid, antlist in ant_spw.items():
                    ants = str(', ').join(antlist)
                    num_chan = pipeline_context.observing_run.get_ms(name=msname).get_spectral_window(spwid).num_channels
                    tr = DeductionGroupTR(group_id, min_freq, max_freq, group_desc.field_name, msname, ants, spwid, num_chan)
                    row_values.append(tr)

        #print("eduction_group = "+str(reduction_group))
        print("ctx before="+str(mako_context))
        mako_context.update({'reduction_group_rows': utils.merge_td_columns(row_values)})
        print("ctx after="+str(mako_context))
