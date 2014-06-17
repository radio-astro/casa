from __future__ import absolute_import

import numpy as np

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.utils as utils

LOG = infrastructure.get_logger(__name__)

SCORE_THRESHOLD_ERROR = 0.33
SCORE_THRESHOLD_WARNING = 0.66
SCORE_THRESHOLD_SUBOPTIMAL = 0.9


def printTsysFlags(tsystable, htmlreport):
    """Method that implements a version of printTsysFlags by Todd Hunter.
    """
    with casatools.TableReader(tsystable) as mytb:
        flags = mytb.getcol("FLAG")
        npol = len(flags)
        antIndices=mytb.getcol("ANTENNA1")
        spws=mytb.getcol("SPECTRAL_WINDOW_ID")
        times=mytb.getcol("TIME")
        fields=mytb.getcol("FIELD_ID")
        uniqueTimes = np.unique(times)

    with casatools.TableReader(tsystable+"/ANTENNA") as mytb:
        antNames = mytb.getcol("NAME")

    with open(htmlreport, 'w') as stream:
        stream.write('<html>')

        for iant in range(len(antNames)):
            for spw in np.unique(spws):
                # zsel will be an array of (a few) row numbers that
                # match the ant and spw
                zsel = np.where((antIndices==iant)*(spws==spw))[0]

                for pol in range(npol):
                    for t in range(len(zsel)):
                        zflag=np.where(flags[pol,:,zsel[t]])

                        if (len(zflag[0]) > 0):
                            if (len(zflag[0]) == len(flags[0,:,zsel[t]])):
                                chans = 'all channels'
                            else:
                                chans = '%d channels: ' % (
                                  len(zflag[0])) + str(zflag[0])

                            timeIndex = list(uniqueTimes).index(times[zsel[t]])
                            myline = antNames[iant] + \
                              " (#%02d), field %d, time %d, pol %d, spw %d, "%(
                              iant, fields[zsel[t]], timeIndex, pol, spw) + \
                              chans + "<br>\n"

                            stream.write(myline)

            # format break between antennas
            stream.write('<br>\n')

def renderflagcmds(flagcmds, htmlflagcmds):
    """Method to render a list of flagcmds into html format.
    """
    lines = []
    for flagcmd in flagcmds:
        lines.append(flagcmd.flagcmd)

    with open(htmlflagcmds, 'w') as stream:
        stream.write('<html>')
        stream.write('<head/>')
        stream.write('<body>')
        stream.write('''This is the list of flagcmds created by this stage.
          <br>''')

        for line in lines:
            stream.write('%s<br>' % line)
        stream.write('</body>')
        stream.write('</html>')



def get_bar_class(pqascore):
    score = pqascore.score
    if score in (None, '', 'N/A'):
        return ''
    elif score <= SCORE_THRESHOLD_ERROR:
        return ' bar-danger'
    elif score <= SCORE_THRESHOLD_WARNING:
        return ' bar-warning'
    elif score <= SCORE_THRESHOLD_SUBOPTIMAL:
        return ' bar-info'
    else:
        return ' bar-success'

def get_badge_class(pqascore):
    score = pqascore.score
    if score in (None, '', 'N/A'):
        return ''
    elif score <= SCORE_THRESHOLD_ERROR:
        return ' badge-important'
    elif score <= SCORE_THRESHOLD_WARNING:
        return ' badge-warning'
    elif score <= SCORE_THRESHOLD_SUBOPTIMAL:
        return ' badge-info'
    else:
        return ' badge-success'

def get_bar_width(pqascore):
    if pqascore.score in (None, '', 'N/A'):
        return 0
    else:
        return 100.0 * pqascore.score

def format_score(pqascore):
    if pqascore.score in (None, '', 'N/A'):
        return 'N/A'
    return '%0.2f' % pqascore.score

def get_symbol_badge(result):
    if get_errors_badge(result):
        symbol = '<i class="fa fa-minus-circle text-error"></i>' 
    elif get_warnings_badge(result):
        symbol = '<i class="fa fa-exclamation-triangle text-warning"></i>' 
    elif get_suboptimal_badge(result):
        symbol = '<i class="fa fa-info-circle text-info"></i>' 
    else:
        return ''
        
    return symbol

def get_warnings_badge(result):
    warning_logrecords = utils.get_logrecords(result, logging.WARNING) 
    warning_qascores = utils.get_qascores(result, SCORE_THRESHOLD_ERROR, SCORE_THRESHOLD_WARNING)
    l = len(warning_logrecords) + len(warning_qascores)
    if l > 0:
        return '<span class="badge badge-warning pull-right">%s</span>' % l
    else:
        return ''
    
def get_errors_badge(result):
    error_logrecords = utils.get_logrecords(result, logging.ERROR) 
    error_qascores = utils.get_qascores(result, -0.1, SCORE_THRESHOLD_ERROR)
    l = len(error_logrecords) + len(error_qascores)
    if l > 0:
        return '<span class="badge badge-important pull-right">%s</span>' % l
    else:
        return ''

def get_suboptimal_badge(result):
    suboptimal_qascores = utils.get_qascores(result, SCORE_THRESHOLD_WARNING, SCORE_THRESHOLD_SUBOPTIMAL)
    l = len(suboptimal_qascores)
    if l > 0:
        return '<span class="badge badge-info pull-right">%s</span>' % l
    else:
        return ''
