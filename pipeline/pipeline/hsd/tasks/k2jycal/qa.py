from __future__ import absolute_import
import pipeline.qa.scorecalculator as qacalc
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils

from . import k2jycal

LOG = logging.get_logger(__name__)

class K2JyCalQAHandler(pqa.QAResultHandler):
    result_cls = k2jycal.SDK2JyCalResults
    child_cls = None

    def handle(self, context, result):
        is_missing_factor = False
        for ms in context.observing_run.measurement_sets:
            vis = ms.basename
            for spw in ms.get_spectral_windows(science_windows_only=True):
                spwid = spw.id
                ddid = ms.get_data_description(spw=spwid)
                for ant in ms.get_antenna():
                    ant_name = ant.name
                    corrs = map(ddid.get_polarization_label, range(ddid.num_polarizations))
                    for corr in corrs:
                        if not self.__check_factor(result.factors, 
                                                   vis, spwid, ant_name, corr):
                            is_missing_factor = True
                            print("Missing: %s, %s, %s, %s" % (vis, spwid, ant_name, corr))
                            break
        
        shortmsg = "Missing Jy/K factors for some data" if is_missing_factor else "Jy/K factor is applied to all data"
        longmsg = "Missing Jy/K factors for some data. Images and their units may be wrong." if is_missing_factor else shortmsg
        score = 0.0 if is_missing_factor else 1.0
        scores = [ pqa.QAScore(score, longmsg=longmsg, shortmsg=shortmsg) ]
        result.qa.pool.extend(scores)
        #result.qa.pool[:] = scores

    def __check_factor(self, factors, vis, spw, ant, corr):
        if (not factors.has_key(vis) or factors[vis] is None): return False
        if (not factors[vis].has_key(spw) or \
                factors[vis][spw] is None): return False
        if (not factors[vis][spw].has_key(ant) or \
                factors[vis][spw][ant] is None): return False
        # pol needs mapping
        ant_factor = factors[vis][spw][ant]
        if ant_factor.has_key(corr): pol=corr
        elif ant_factor.has_key('I') and corr in ("XX", "YY", "RR", "LL"): pol = "I"
        if (not factors[vis][spw][ant].has_key(pol) or \
                factors[vis][spw][ant][pol] is None): return False
        return True
