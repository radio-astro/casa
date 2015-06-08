from __future__ import absolute_import

import pipeline.infrastructure.api as api
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure as infrastructure
from pipeline.hsd.heuristics import DataTypeHeuristics
LOG = infrastructure.get_logger(__name__)

class TsysSpwMapHeuristics(api.Heuristic):
    """
    Heuristics for Tsys spw mapping
    Examine frequency coverage and choose one Tsys spw for each science spw. 
    Score for frequency coverage is calculated by the following formula:
    
        score = (min(fmax_tsys,fmax_science) - max(fmin_tsys,fmin_science)) 
                    / (fmax_science - fmin_science)
                   
    The score 1.0 is the best (whole science frequency range is covered by Tsys 
    spw) while score <= 0.0 is the worst (no overlap between Tsys spw and science 
    spw).
    """
    def calculate(self, ms, spwmap_pairs):
        scores = {}
        spws = ms.spectral_windows
        freq_range = lambda spw: (float(spw.min_frequency.value), float(spw.max_frequency.value)) 
        for (spwfrom, spwto) in spwmap_pairs:
            from_range = freq_range(spws[spwfrom])
            to_range = freq_range(spws[spwto])
            score = (min(from_range[1], to_range[1]) - max(from_range[0], to_range[0])) / (to_range[1] - to_range[0])
            if not scores.has_key(spwto):
                scores[spwto] = {}
            scores[spwto][spwfrom] = score
            
        LOG.debug('scores=%s'%(scores))

        tsysspwmap = list(best_spwmap(scores))
        
        return tsysspwmap
    
def best_spwmap(scores):
    for (spw,score_list) in scores.items():
        best_score = 0.0
        best_spw = -1
        for (atmspw,score) in score_list.items():
            if score > best_score:
                best_spw = atmspw
                best_score = score
        LOG.debug('science spw %s: best_spw %s (score %s)'%(spw,best_spw,best_score))
        
        if best_score < 1.0:
            LOG.warn('spw %s: Tsys spw %s doesn\'t cover whole frequnecy range (only %s%% overlap)'%(spw,best_spw,100.0*best_score))
        yield [best_spw, spw]