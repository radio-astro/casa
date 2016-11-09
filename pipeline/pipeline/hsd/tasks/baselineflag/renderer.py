'''
Created on Nov 9, 2016

@author: kana
'''
import collections
import os

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils

LOG = logging.get_logger(__name__)


class T2_4MDetailsBLFlagRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    '''
    The renderer class for baselineflag.
    '''
    def __init__(self, uri='hsd_blflag.mako',
                 description='Flag data by Tsys, weather, and statistics of spectra',
                 always_rerender=False):
        '''
        Constructor
        '''
        super(T2_4MDetailsBLFlagRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, ctx, context, result):
        accum_flag = accumulate_flag_per_source_spw(result)
        table_rows = make_summary_table(accum_flag)

        ctx.update({'sumary_table_rows': table_rows})

FlagSummaryTR = collections.namedtuple('FlagSummaryTR', 'field spw before additional total')

    
def accumulate_flag_per_source_spw(results):
    # Accumulate flag per field, spw from the output of flagdata to a dictionary
    # accum_flag[field][spw] = {'additional': # of flagged in task,
    #                           'total': # of total samples,
    #                           'before': # of flagged before task,
    #                           'after': total # of flagged}
    accum_flag = {}
    for r in results:
        before, after = r.outcome['flagdata_summary']
        if not before['name']=='before' or not after['name']=='after':
            raise RuntimeError, "Got unexpected flag summary"
        for field, fieldflag in after.items():
            if not isinstance(fieldflag, dict) or not fieldflag.has_key('spw'):
                continue
            if not accum_flag.has_key(field):
                accum_flag[field] = {}
            spwflag = fieldflag['spw']
            for spw, flagval in spwflag.items():
                if not accum_flag[field].has_key(spw):
                    accum_flag[field][spw] = dict(before=0, additional=0, after=0, total=0)
                # sum up incremental flags
                accum_flag[field][spw]['before'] += before[field]['spw'][spw]['flagged']
                accum_flag[field][spw]['after'] += flagval['flagged']
                accum_flag[field][spw]['total'] += flagval['total']
                accum_flag[field][spw]['additional'] += (flagval['flagged']-before[field]['spw'][spw]['flagged'])
    return accum_flag
                                                   
def make_summary_table(flagdict):
    # will hold all the flag summary table rows for the results
    rows = []
    for field, flagperspw in flagdict.items():
        for spw, flagval in flagperspw.items():
            frac_before = flagval['before']/flagval['total']
            frac_total = flagval['after']/flagval['total']
            frac_additional = (flagval['after']-flagval['before'])/flagval['total']

            tr = FlagSummaryTR(field, spw, '%0.1f%%'%(frac_before*100), '%0.1f%%'%(frac_additional*100), '%0.1f%%'%(frac_total*100))
            rows.append(tr)
    
    return utils.merge_td_columns(rows, num_to_merge=2)
        