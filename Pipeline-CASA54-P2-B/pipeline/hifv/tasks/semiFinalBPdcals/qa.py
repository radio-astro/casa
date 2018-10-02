from __future__ import absolute_import

import collections

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
import pipeline.qa.scorecalculator as qacalc
from . import semiFinalBPdcals
from . import semiFinalBPdcalsResults


LOG = logging.get_logger(__name__)


class semiFinalBPdcalsQAHandler(pqa.QAPlugin):
    result_cls = semiFinalBPdcalsResults
    child_cls = None
    generating_task = semiFinalBPdcals

    def handle(self, context, result):
        # get a QA score for fraction of failed (flagged) bandpass solutions in the bandpass table
        # < 5%   of data flagged  --> 1
        # 5%-60% of data flagged  --> 1 to 0
        # > 60%  of data flagged  --> 0

        if result.flaggedSolnApplycalbandpass and result.flaggedSolnApplycaldelay:
            self._checkKandBsolution(result.flaggedSolnApplycaldelay)
            self._checkKandBsolution(result.flaggedSolnApplycalbandpass)

            score1 = qacalc.score_total_data_flagged_vla_bandpass(result.bpdgain_touse,
                                                                  result.flaggedSolnApplycalbandpass['antmedian']['fraction'])
            score2 = qacalc.score_total_data_vla_delay(result.ktypecaltable)
            scores = [score1, score2]
        else:
            LOG.error('Error with bandpass and/or delay table.')
            scores = [pqa.QAScore(0.0, longmsg='No flagging stats about the bandpass table or info in delay table.',
                                  shortmsg='Bandpass or delay table problem.')]

        result.qa.pool.extend(scores)

    @staticmethod
    def _checkKandBsolution(table):
        for antenna in table['antspw'].keys():
            for spw in table['antspw'][antenna].keys():
                for pol in table['antspw'][antenna][spw].keys():
                    frac = table['antspw'][antenna][spw][pol]['fraction']
                    if frac == 1.0:
                        LOG.warn('Antenna {!s}, spw {!s}, pol {!s} has a fraction of flagged solutions of: {!s}'.format(antenna, spw, pol, frac))

        return


class semiFinalBPdcalsListQAHandler(pqa.QAPlugin):
    """
    QA handler for a list containing semiFinalBPdcalsResults.
    """
    result_cls = collections.Iterable
    child_cls = semiFinalBPdcalsResults

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result])
        result.qa.pool.extend(collated)
