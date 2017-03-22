'''
Created on 25 Nov 2014

@author: sjw
'''
import collections
import pipeline.infrastructure.utils as utils

FlagTotal = collections.namedtuple('FlagSummary', 'flagged total')


def flags_for_result(result, context, non_science_agents=[]):
    ms = context.observing_run.get_ms(result.inputs['vis'])
    summaries = result.summaries

    by_intent = flags_by_intent(ms, summaries)
    by_spw = flags_by_science_spws(ms, summaries)
    merged = utils.dict_merge(by_intent, by_spw)
    
    adjusted = adjust_non_science_totals(merged, non_science_agents)

    return {ms.basename : adjusted}

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

def adjust_non_science_totals(flagtotals, non_science_agents=[]):
    """
    Return a copy of the FlagSummary dictionaries, with totals reduced to
    account for flagging performed by non-science flagging agents.
    
    The incoming flagtotals report how much data was flagged per agent per
    data selection. These flagtotals are divided into two groups: those whose 
    agent should be considered 'non-science' (and are indicated as such in the 
    non_science_agents argument) and the remainder. The total number of rows
    flagged due to non-science agents is calculated and subtracted from the 
    total for each of the remainder agents.     
    """
    agents_to_copy = set(non_science_agents)
    agents_to_adjust = set(flagtotals.keys()) - agents_to_copy
    data_selections = set()
    for result in flagtotals.values():
        data_selections.update(set(result.keys()))

    # copy agents that use the total number of visibilities across to new 
    # results
    adjusted_results = dict((agent, flagtotals[agent]) 
                            for agent in agents_to_copy
                            if agent in flagtotals)
    
    # tot up how much data was flagged by each agent per data selection
    flagged_non_science = {}
    for data_selection in data_selections:
        flagged_non_science[data_selection] = sum([v[data_selection].flagged 
                                                   for v in adjusted_results.values()])
        
    # subtract this 'number of rows flagged per data selection' from the total
    # for the remaining agents
    for agent in agents_to_adjust:
        for data_selection in flagtotals[agent]:
            unadjusted = flagtotals[agent][data_selection]
            adjusted = FlagTotal(unadjusted.flagged, 
                                 unadjusted.total - flagged_non_science[data_selection])
            if agent not in adjusted_results:
                adjusted_results[agent] = {}
            adjusted_results[agent][data_selection] = adjusted

    return adjusted_results
