import collections
import os

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils

LOG = logging.get_logger(__name__)


class T2_4MDetailsMsSplitRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='mssplit.mako', 
                 description='Split an MS',
                 always_rerender=False):
        super(T2_4MDetailsMsSplitRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, mako_context, pipeline_context, results):
        table_rows = make_split_table(pipeline_context, results)
        mako_context.update({'table_rows': table_rows})


MsSplitTR = collections.namedtuple('MsSplitTR', 'vis outputvis')

def make_split_table(context, results):

    # Will hold all the input and output MS(s)
    rows = []

    # Loop over the results
    for single_result in results:
        vis_cell = os.path.basename(single_result.inputs['vis'])
        outputvis_cell = os.path.basename(single_result.inputs['outputvis'])
        tr = MsSplitTR(vis_cell, outputvis_cell)
        rows.append(tr)

    return utils.merge_td_columns(rows)
