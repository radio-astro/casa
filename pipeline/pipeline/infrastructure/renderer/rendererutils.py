from __future__ import absolute_import
import cgi

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
        spws=mytb.getcol("SPECTRAL_WINDOW_ID")
        
    with casatools.TableReader(tsystable+"/ANTENNA") as mytb:
        antNames = mytb.getcol("NAME")

    with open(htmlreport, 'w') as stream:
        stream.write('<html>')

        with casatools.TableReader(tsystable) as mytb:
            for iant in range(len(antNames)):
                for spw in np.unique(spws):
                    
                    # select rows from table for specified antenna and spw
                    zseltb = mytb.query("SPECTRAL_WINDOW_ID=={0} && ANTENNA1=={1}".format(spw, iant))
                    flags = zseltb.getcol("FLAG")
                    npol = len(flags)
                    nchan = len(flags[0])
                    times = zseltb.getcol("TIME")
                    fields = zseltb.getcol("FIELD_ID")
                    uniqueTimes = np.unique(times)
                    
                    for pol in range(npol):
                        for t in range(len(times)):
                            zflag=np.where(flags[pol,:,t])[0]
                            
                            if (len(zflag) > 0):
                                if (len(zflag) == nchan):
                                    chans = 'all channels'
                                else:
                                    chans = '%d channels: ' % (
                                      len(zflag)) + str(zflag)
                                
                                timeIndex = list(uniqueTimes).index(times[t])
                                myline = antNames[iant] + \
                                  " (#%02d), field %d, time %d, pol %d, spw %d, "%(
                                  iant, fields[t], timeIndex, pol, spw) + \
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
        return ' progress-bar-danger'
    elif score <= SCORE_THRESHOLD_WARNING:
        return ' progress-bar-warning'
    elif score <= SCORE_THRESHOLD_SUBOPTIMAL:
        return ' progress-bar-info'
    else:
        return ' progress-bar-success'

def get_badge_class(pqascore):
    score = pqascore.score
    if score in (None, '', 'N/A'):
        return ''
    elif score <= SCORE_THRESHOLD_ERROR:
        return ' alert-danger'
    elif score <= SCORE_THRESHOLD_WARNING:
        return ' alert-warning'
    elif score <= SCORE_THRESHOLD_SUBOPTIMAL:
        return ' alert-info'
    else:
        return ' alert-success'

def get_bar_width(pqascore):
    if pqascore.score in (None, '', 'N/A'):
        return 0
    else:
        return 5.0 + 95.0 * pqascore.score

def format_score(pqascore):
    if pqascore.score in (None, '', 'N/A'):
        return 'N/A'
    return '%0.2f' % pqascore.score

def get_symbol_badge(result):
    if get_errors_badge(result):
        symbol = '<span class="glyphicon glyphicon-remove-sign alert-danger transparent-bg" aria-hidden="true"></span>' 
    elif get_warnings_badge(result):
        symbol = '<span class="glyphicon glyphicon-exclamation-sign alert-warning transparent-bg" aria-hidden="true"></span>' 
    elif get_suboptimal_badge(result):
        symbol = '<span class="glyphicon glyphicon-question-sign alert-info transparent-bg" aria-hidden="true"></span>' 
    else:
        return '<span class="glyphicon glyphicon-none" aria-hidden="true"></span>'
        
    return symbol

def get_warnings_badge(result):
    warning_logrecords = utils.get_logrecords(result, logging.WARNING) 
    warning_qascores = utils.get_qascores(result, SCORE_THRESHOLD_ERROR, SCORE_THRESHOLD_WARNING)
    l = len(warning_logrecords) + len(warning_qascores)
    if l > 0:
        return '<span class="badge alert-warning pull-right">%s</span>' % l
    else:
        return ''
    
def get_errors_badge(result):
    error_logrecords = utils.get_logrecords(result, logging.ERROR) 
    error_qascores = utils.get_qascores(result, -0.1, SCORE_THRESHOLD_ERROR)
    l = len(error_logrecords) + len(error_qascores)
    if l > 0:
        return '<span class="badge alert-important pull-right">%s</span>' % l
    else:
        return ''

def get_suboptimal_badge(result):
    suboptimal_qascores = utils.get_qascores(result, SCORE_THRESHOLD_WARNING, SCORE_THRESHOLD_SUBOPTIMAL)
    l = len(suboptimal_qascores)
    if l > 0:
        return '<span class="badge alert-info pull-right">%s</span>' % l
    else:
        return ''

def get_plot_command_markup(ctx, command):
    if not command:
        return ''
    stripped = command.replace('%s/' % ctx.report_dir, '')
    stripped = stripped.replace('%s/' % ctx.output_dir, '')
    escaped = cgi.escape(stripped, True).replace('\'', '&#39;')

    btn = '''<a data-toggle="modal" data-target=".plot-command-modal">Plot command</a>

<div class="modal fade plot-command-modal">
  <div class="modal-dialog">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="close" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>
        <h4 class="modal-title">Plot command</h4>
      </div>
      <div class="modal-body">
        <p>%s</p>
      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
      </div>
    </div><!-- /.modal-content -->
  </div><!-- /.modal-dialog -->
</div><!-- /.modal -->''' % escaped

    return btn