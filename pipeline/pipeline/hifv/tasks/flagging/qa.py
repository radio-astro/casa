from __future__ import absolute_import

import collections
import os

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa
import pipeline.infrastructure.utils as utils
import pipeline.qa.scorecalculator as qacalc
from . import checkflag
from . import flagbaddeformatters
from . import targetflag

LOG = logging.get_logger(__name__)

class FlagBadDeformattersQAHandler(pqa.QAPlugin):
    result_cls = flagbaddeformatters.FlagBadDeformattersResults
    child_cls = None
    generating_task = flagbaddeformatters.FlagBadDeformatters

    def handle(self, context, result):

        # Check for existence of the the target MS.
        score1 = self._ms_exists(os.path.dirname(result.inputs['vis']), os.path.basename(result.inputs['vis']))
        scores = [score1]

        result.qa.pool.extend(scores)

    def _ms_exists(self, output_dir, ms):
        '''
        Check for the existence of the target MS
        '''
        return qacalc.score_path_exists(output_dir, ms, 'Flag Bad Deformatters')

class FlagBadDeformattersListQAHandler(pqa.QAPlugin):
    """
    QA handler for a list containing FlagBadDeformattersResults.
    """
    result_cls = collections.Iterable
    child_cls = flagbaddeformatters.FlagBadDeformattersResults
    generating_task = flagbaddeformatters.FlagBadDeformatters

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result])
        result.qa.pool[:] = collated
        mses = [r.inputs['vis'] for r in result]
        longmsg = 'No missing target MS(s) for %s' % utils.commafy(mses, quotes=False, conjunction='or')
        result.qa.all_unity_longmsg = longmsg





class CheckflagQAHandler(pqa.QAPlugin):
    result_cls = checkflag.CheckflagResults
    child_cls = None
    generating_task = checkflag.Checkflag

    def handle(self, context, result):

        # Check for existence of the the target MS.
        score1 = self._ms_exists(os.path.dirname(result.inputs['vis']), os.path.basename(result.inputs['vis']))
        scores = [score1]

        result.qa.pool.extend(scores)

    def _ms_exists(self, output_dir, ms):
        '''
        Check for the existence of the target MS
        '''
        return qacalc.score_path_exists(output_dir, ms, 'Checkflag')

class CheckflagListQAHandler(pqa.QAPlugin):
    """
    QA handler for a list containing CheckflagResults.
    """
    result_cls = collections.Iterable
    child_cls = checkflag.CheckflagResults
    generating_task = checkflag.Checkflag

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result])
        result.qa.pool[:] = collated
        mses = [r.inputs['vis'] for r in result]
        longmsg = 'No missing target MS(s) for %s' % utils.commafy(mses, quotes=False, conjunction='or')
        result.qa.all_unity_longmsg = longmsg




class TargetflagQAHandler(pqa.QAPlugin):
    result_cls = targetflag.TargetflagResults
    child_cls = None
    generating_task = targetflag.Targetflag

    def handle(self, context, result):

        # Check for existence of the the target MS.
        score1 = self._ms_exists(os.path.dirname(result.inputs['vis']), os.path.basename(result.inputs['vis']))
        scores = [score1]

        result.qa.pool.extend(scores)

    def _ms_exists(self, output_dir, ms):
        '''
        Check for the existence of the target MS
        '''
        return qacalc.score_path_exists(output_dir, ms, 'Targetflag')

class TargetflagListQAHandler(pqa.QAPlugin):
    """
    QA handler for a list containing TargetflagResults.
    """
    result_cls = collections.Iterable
    child_cls = targetflag.TargetflagResults
    generating_task = targetflag.Targetflag

    def handle(self, context, result):
        # collate the QAScores from each child result, pulling them into our
        # own QAscore list
        collated = utils.flatten([r.qa.pool for r in result])
        result.qa.pool[:] = collated
        mses = [r.inputs['vis'] for r in result]
        longmsg = 'No missing target MS(s) for %s' % utils.commafy(mses, quotes=False, conjunction='or')
        result.qa.all_unity_longmsg = longmsg