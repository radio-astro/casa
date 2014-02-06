from __future__ import absolute_import
import os
import shutil

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
import pipeline.qa2.bpcal as bpcal

from . import common

LOG = logging.get_logger(__name__)


class BandpassQAPool(pqa.QAScorePool):
    score_types = {'AMPLITUDE_SCORE_DD'  : 'amplitude derivative',
                   'AMPLITUDE_SCORE_FN'  : 'amplitude Wiener function',
                   'AMPLITUDE_SCORE_SNR' : 'amplitude SNR',
                   'PHASE_SCORE_DD'      : 'phase derivative',
                   'PHASE_SCORE_FN'      : 'phase Wiener function',
                   'PHASE_SCORE_RMS'     : 'phase RMS'}

    short_msg = {'AMPLITUDE_SCORE_DD'  : 'Amp derivative',
                 'AMPLITUDE_SCORE_FN'  : 'Amp flatness',
                 'AMPLITUDE_SCORE_SNR' : 'Amp SNR',
                 'PHASE_SCORE_DD'      : 'Phase derivative',
                 'PHASE_SCORE_FN'      : 'Phase flatness',
                 'PHASE_SCORE_RMS'     : 'Phase RMS'}

    def __init__(self, rawdata, caltable):
        super(BandpassQAPool, self).__init__()
        # rawdata will hold the dictionary output by the CA QA task
        self.rawdata = rawdata
        self._num_pols = utils.get_num_caltable_polarizations(caltable)
        
    def update_scores(self, ms):
        """
        MeasurementSet is needed to convert from integer identifier stored in
        QA dictionary to the antenna, spw and pol it represents.
        """
#        "Lowest score for %s analysis is %s (ant %s spw %s pol %s)"
        self.pool[:] = [self._get_qascore(ms, t) for t in self.score_types] 

    def _get_qascore(self, ms, score_type):
        (min_score, spw_str, qa_id) = self._get_min(score_type)
        identifier = self._get_identifier_from_qa_id(ms, spw_str, qa_id)
        longmsg = 'Lowest score for %s is %0.2f (%s %s)' % (self.score_types[score_type],
                                                            min_score,
                                                            ms.basename,
                                                            identifier)
        shortmsg = self.short_msg[score_type]
        return pqa.QAScore(min_score, longmsg=longmsg, shortmsg=shortmsg)

    def _get_min(self, score_type):
        rawscores = self.rawdata['QA2SCORES'][score_type]
        
        # attrs to hold score and QA identifier
        min_score = 1.0
        min_id = None
        min_spw = None

        for spw_str, spwdict in rawscores.items():
            for id_str, score in spwdict.items():
                if score < min_score:
                    min_score = score
                    min_spw = spw_str
                    min_id = id_str

        return (min_score, min_spw, min_id)

    def _get_identifier_from_qa_id(self, ms, spw_str, qa_id):
        spw = ms.get_spectral_window(spw_str)
        dd = ms.get_data_description(spw=spw)
        if dd is None:
            return 'unknown origin'

        ant_id = int(qa_id) / self._num_pols
        feed_id = int(qa_id) % self._num_pols
                
        polarization = dd.get_polarization_label(feed_id)
        antenna = ms.get_antenna(ant_id)[0]

        return '%s spw %s %s' % (antenna.name, spw_str, polarization)


class BandpassQAHandler(pqa.QAResultHandler):    
    """
    QA handler for an uncontained BandpassResult.
    """
    result_cls = common.BandpassResults
    child_cls = None

    def handle(self, context, result):
        vis = result.inputs['vis']
        ms = context.observing_run.get_ms(vis)
    
        qa2_dir = os.path.join(context.report_dir,
                               'stage%s' % result.stage_number,
                               'qa2')

        if not os.path.exists(qa2_dir):
            os.makedirs(qa2_dir)

        for calapp in result.final:
            (root, _) = os.path.splitext(os.path.basename(calapp.gaintable))
            qa2_file = os.path.join(qa2_dir, root + '.bpcal.stats')
            if os.path.exists(qa2_file):
                LOG.info('Removing existing QA2 statistics table from %s'
                         % qa2_file)
                shutil.rmtree(qa2_file)

            try:
                qa2_results = bpcal.bpcal(calapp.gaintable, qa2_dir)
                result.qa = BandpassQAPool(qa2_results, calapp.gaintable)
                result.qa.update_scores(ms)
            except Exception as e:
                LOG.error('Problem occurred running QA2 analysis. QA2 '
                          'results will not be available for this task')
                LOG.exception(e)


class BandpassListQAHandler(pqa.QAResultHandler):
    """
    QA handler for a list containing BandpassResults.
    """
    result_cls = list
    child_cls = common.BandpassResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QA2score list
        collated = utils.flatten([r.qa.pool for r in result]) 
        result.qa.pool[:] = collated
