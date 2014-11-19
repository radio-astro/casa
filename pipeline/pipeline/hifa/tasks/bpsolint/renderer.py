import collections
import os

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.utils as utils

LOG = logging.get_logger(__name__)


class T2_4MDetailsBpSolintRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='bpsolint.mako', 
                 description='Compute bandpass solution intervals',
                 always_rerender=False):
        super(T2_4MDetailsBpSolintRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, mako_context, pipeline_context, results):
        table_rows = make_bpsolint_table(pipeline_context, results)
        mako_context.update({'table_rows'         : table_rows})

    
BpsolintTR = collections.namedtuple('BpsolintTR', 'vis spw phintervals phnpoints bpintervals, bpnpoints')

def make_bpsolint_table(context, results):
    # will hold all the flux stat table rows for the results

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

	    # Phaseup solution intervalue
	    if not single_result.phsolints[i]:
	        phintervals = ''
	        phnpoints = ''
	    else:
		if single_result.phsolints[i] == 'int':
	            phintervals = 'int (%d)' % (single_result.phintsolints[i])
		else:
	            phintervals = '%7.3f (%d)' % (qt.getvalue(single_result.phsolints[i])[0],
		        single_result.phintsolints[i])
	        phnpoints = '%d' % single_result.nphsolutions[i]

	    # Bandpass solutions intervals
	    if not single_result.bpsolints[i]:
	        bpintervals = ''
	        bpnpoints = ''
	    else:
	        bpintervals = '%f (%d)' % (qt.getvalue(single_result.bpsolints[i])[0],
		    single_result.bpchansolints[i])
	        bpnpoints = '%d' % single_result.nbpsolutions[i]

            tr = BpsolintTR(vis_cell, spwid,  phintervals, phnpoints,
                bpintervals, bpnpoints)
            rows.append(tr)

    return utils.merge_td_columns(rows)

