"""
Created on 22 Feb 2017

@author: sjw
"""
import collections
import os

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils

__all__ = [
    'T2_4MDetailsRefantRenderer'
]

LOG = logging.get_logger(__name__)


class T2_4MDetailsRefantRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='refant.mako', description='Select reference antennas', always_rerender=False):
        super(T2_4MDetailsRefantRenderer, self).__init__(uri=uri, description=description,
                                                         always_rerender=always_rerender)

    def update_mako_context(self, ctx, context, result):
        refant_table_rows = make_refant_table(result)
        ctx.update({
            'refant_table': refant_table_rows,
        })


RefantTR = collections.namedtuple('RefantTR', 'vis refant')


def make_refant_table(results):
    rows = []

    for result in results:
        vis_cell = os.path.basename(result.inputs['vis'])
        # insert spaces in refant list to allow browser to break string if it wants
        refant_cell = result._refant.replace(',', ', ')
        tr = RefantTR(vis_cell, refant_cell)
        rows.append(tr)

    return utils.merge_td_columns(rows)
