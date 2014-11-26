'''
Created on 25 Nov 2014

@author: sjw
'''
import collections
import pipeline.infrastructure.utils as utils

FlagTotal = collections.namedtuple('FlagSummary', 'flagged total')


def flags_for_result(result, context):
    ms = context.observing_run.get_ms(result.inputs['vis'])
    summaries = result.summaries

    by_intent = flags_by_intent(ms, summaries)
    by_spw = flags_by_science_spws(ms, summaries)

    return {ms.basename : utils.dict_merge(by_intent, by_spw)}

def flags_by_intent(ms, summaries):
    # create a dictionary of scans per observing intent, eg. 'PHASE':[1,2,7]
    intent_scans = {}
    for intent in ('BANDPASS', 'PHASE', 'AMPLITUDE', 'TARGET'):
        # convert IDs to strings as they're used as summary dictionary keys
        intent_scans[intent] = [str(s.id) for s in ms.scans
                                if intent in s.intents]

    # while we're looping, get the total flagged by looking in all scans 
    intent_scans['TOTAL'] = [str(s.id) for s in ms.scans]

    total = collections.defaultdict(dict)

    previous_summary = None
    for summary in summaries:

        for intent, scan_ids in intent_scans.items():
            flagcount = 0
            totalcount = 0

            for i in scan_ids:
                flagcount += int(summary['scan'][i]['flagged'])
                totalcount += int(summary['scan'][i]['total'])
    
                if previous_summary:
                    flagcount -= int(previous_summary['scan'][i]['flagged'])

            ft = FlagTotal(flagcount, totalcount)
            total[summary['name']][intent] = ft
            
        previous_summary = summary
            
    return total 

def flags_by_science_spws(ms, summaries):
    science_spws = ms.get_spectral_windows(science_windows_only=True)

    total = collections.defaultdict(dict)

    previous_summary = None
    for summary in summaries:

        flagcount = 0
        totalcount = 0

        for spw in science_spws:
            spw_id = str(spw.id)
            flagcount += int(summary['spw'][spw_id]['flagged'])
            totalcount += int(summary['spw'][spw_id]['total'])
    
            if previous_summary:
                flagcount -= int(previous_summary['spw'][spw_id]['flagged'])

        ft = FlagTotal(flagcount, totalcount)
        total[summary['name']]['SCIENCE SPWS'] = ft
            
        previous_summary = summary
            
    return total
