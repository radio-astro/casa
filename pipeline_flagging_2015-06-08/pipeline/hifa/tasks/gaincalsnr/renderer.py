import collections
import os

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.utils as utils

#LOG = logging.get_logger(__name__)


class T2_4MDetailsGaincalSnrRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='gaincalsnr.mako', 
                 description='Estimate gaincal signal to noise ratios',
                 always_rerender=False):
        super(T2_4MDetailsGaincalSnrRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, mako_context, pipeline_context, results):
        table_rows = make_gaincalsnr_table(pipeline_context, results)
        mako_context.update({'table_rows'         : table_rows})

    
GaincalSnrTR = collections.namedtuple('GaincalSnrTR', 'vis spw sensitivities snrs')

def make_gaincalsnr_table(context, results):
    # will hold all the table rows for the results

    qt = casatools.quanta

    rows = []
    for single_result in results:
        ms_for_result = context.observing_run.get_ms(single_result.vis)
        vis_cell = os.path.basename(single_result.vis)

        # measurements will be empty if fluxscale derivation failed
        if len(single_result.spwids) is 0:
            continue
            
        for i in range(len(single_result.spwids)):

	    spwid = '%d' % single_result.spwids[i]

	    # Sensitivities
	    if not single_result.sensitivities[i]:
	        sensitivities = ''
	        snrs = ''
	    else:
                sensitivities = '%0.3f' % single_result.sensitivities[i]
                snrs = '%0.3f' % single_result.snrs[i]
            tr = GaincalSnrTR(vis_cell, spwid, sensitivities, snrs)
            rows.append(tr)

    return utils.merge_td_columns(rows)

