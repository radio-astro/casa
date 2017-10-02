from __future__ import absolute_import

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa


LOG = logging.get_logger(__name__)


class ExportvlassdataQAHandler(pqa.QAResultHandler):

    def handle(self, context, result):

        pass


class ExportvlassdataListQAHandler(pqa.QAResultHandler):
    """
    QA handler for a list containing ExportDataResults.
    """

    def handle(self, context, result):

        pass
