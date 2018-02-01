from __future__ import absolute_import

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.pipelineqa as pqa

LOG = logging.get_logger(__name__)


class ExportvlassdataQAHandler(pqa.QAPlugin):

    def handle(self, context, result):

        pass


class ExportvlassdataListQAHandler(pqa.QAPlugin):
    """
    QA handler for a list containing ExportDataResults.
    """

    def handle(self, context, result):

        pass
