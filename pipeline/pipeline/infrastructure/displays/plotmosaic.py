import pipeline.infrastructure as infrastructure
from pipeline.extern.analysis_scripts.analysisUtils import plotmosaic

LOG = infrastructure.get_logger(__name__)


def plotMosaic(vis='', sourceid='', figfile='', coord='relative', skipsource=-1, doplot=True, help=False):
    """
    Produce a plot of the pointings with the primary beam FWHM and field names.
    """
    LOG.todo('Convert aU.plotMosaic to pipeline')
    return plotmosaic(vis, sourceid, figfile, coord, skipsource, doplot, help, sciencespws=True)
