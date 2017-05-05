"""
Created on 28 Apr 2017

@author: Vincent Geers (UKATC)
"""

import os
import numpy as np

import pipeline.hif.tasks.bandpass.renderer as baserenderer
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.utils as utils

LOG = logging.get_logger(__name__)


class T2_4MDetailsBandpassflagRenderer(baserenderer.T2_4MDetailsBandpassRenderer):
    """
    Renders detailed HTML output for the Bandpassflag task.
    """
    # FIXME: set rerender to false before final commit
    def __init__(self, uri='bandpassflag.mako',
                 description='Phase-up bandpass calibration + flagging',
                 always_rerender=True):
        super(T2_4MDetailsBandpassflagRenderer, self).__init__(
            uri=uri, description=description, always_rerender=always_rerender)

    def update_mako_context(self, mako_context, pipeline_context, results):

        # Run the standard hifa bandpass mako context update.
        super(T2_4MDetailsBandpassflagRenderer, self).update_mako_context(
            mako_context, pipeline_context, results)

        # Create html reports for flagging.
        htmlreports = self.get_htmlreports(pipeline_context, results)

        mako_context.update({
            'htmlreports': htmlreports,
        })

    def get_phaseup_applications(self, context, result, ms):
        hm_phaseup = result.inputs.get('hm_phaseup', 'N/A')
        if not hm_phaseup:
            return []

        calmode_map = {'p': 'Phase only',
                       'a': 'Amplitude only',
                       'ap': 'Phase and amplitude'}

        # identify phaseup from 'preceding' list attached to result
        phaseup_calapps = []
        for previous_result in result.preceding:
            for calapp in previous_result:
                l = [cf for cf in calapp.calfrom if cf.caltype == 'gaincal']
                if l and calapp not in phaseup_calapps:
                    phaseup_calapps.append(calapp)

        applications = []
        for calapp in phaseup_calapps:
            solint = calapp.origin.inputs['solint']

            if solint == 'inf':
                solint = 'Infinite'

            # Convert solint=int to a real integration time.
            # solint is spw dependent; science windows usually have the same
            # integration time, though that's not guaranteed by the MS.
            if solint == 'int':
                in_secs = ['%0.2fs' % (dt.seconds + dt.microseconds * 1e-6)
                           for dt in utils.get_intervals(context, calapp)]
                solint = 'Per integration (%s)' % utils.commafy(in_secs,
                                                                quotes=False,
                                                                conjunction='or')

            calmode = calapp.origin.inputs.get('calmode', 'N/A')
            calmode = calmode_map.get(calmode, calmode)
            minblperant = calapp.origin.inputs.get('minblperant', 'N/A')
            minsnr = calapp.origin.inputs.get('minsnr', 'N/A')
            flagged = 'TODO'
            phaseupbw = result.inputs.get('phaseupbw', 'N/A')

            a = baserenderer.PhaseupApplication(ms.basename, calmode, solint,
                                                minblperant, minsnr, flagged,
                                                phaseupbw)
            applications.append(a)

        return applications

    def get_bandpass_table(self, context, result, ms):
        applications = []

        bandtype_map = {'B': 'Channel',
                        'BPOLY': 'Polynomial'}

        for calapp in result.final:
            gaintable = os.path.basename(calapp.gaintable)
            to_intent = ', '.join(calapp.intent.split(','))
            if to_intent == '':
                to_intent = 'ALL'

            if not isinstance(calapp.origin, list):
                calapp_origins = [calapp.origin]
            else:
                calapp_origins = calapp.origin

            for calapp_origin in calapp_origins:
                spws = calapp_origin.inputs['spw'].split(',')

                solint = calapp_origin.inputs['solint']

                if solint == 'inf':
                    solint = 'Infinite'

                # Convert solint=int to a real integration time.
                # solint is spw dependent; science windows usually have the same
                # integration time, though that's not guaranteed by the MS.
                if solint == 'int':
                    in_secs = ['%0.2fs' % (dt.seconds + dt.microseconds * 1e-6)
                               for dt in utils.get_intervals(context, calapp, set(spws))]

                if len(spws) == 1:
                    solint = self._format_bandpass_solint(ms, solint, spws[0])

                bandtype = calapp_origin.inputs['bandtype']
                bandtype = bandtype_map.get(bandtype, bandtype)
                a = baserenderer.BandpassApplication(ms.basename, bandtype, solint,
                                                     to_intent, ', '.join(spws), gaintable)
                applications.append(a)

        return applications

    def _format_bandpass_solint(self, ms, bpsolint, spwid):

        # Split the bandpass solint into time and frequency components
        try:
            timesolint, freqsolint = bpsolint.split(',')
        except:
            return bpsolint

        # Get the spectral window
        try:
            spw = ms.get_spectral_window(spwid)
        except:
            return bpsolint

        # make sure freqsolint is a valid quanta
        qa = casatools.quanta
        try:
            qfreqsolint = qa.quantity(freqsolint)
        except:
            return bpsolint

        # Compute median channel width in Hz
        channels = spw.channels
        chanwidths = np.zeros(spw.num_channels)
        for i in range(spw.num_channels):
            chanwidths[i] = (channels[i].high - channels[i].low).value
        median_chanwidth = np.median(chanwidths)

        # If freqsolint in channels compute equivalent
        # frequency interval in MHz and vice versa
        if qa.getunit(qfreqsolint) == 'ch':
            nchan = qa.getvalue(qfreqsolint)[0]
            afreqsolint = timesolint + ',' + freqsolint + '(%fMHz)' % (1.0e-6 * nchan * median_chanwidth)
        else:
            nchan = qa.getvalue(qa.convert(qfreqsolint, 'Hz'))[0] / median_chanwidth
            afreqsolint = timesolint + ',' + freqsolint + '(%0.1fch)' % (nchan)

        return afreqsolint

    def get_htmlreports(self, context, results):
        report_dir = context.report_dir
        weblog_dir = os.path.join(report_dir,
                                  'stage%s' % results.stage_number)

        htmlreports = {}
        for result in results:
            flagcmd_abspath = self._write_flagcmd_to_disk(weblog_dir, result)
            flagcmd_relpath = os.path.relpath(flagcmd_abspath, report_dir)
            table_basename = os.path.basename(result.vis)
            htmlreports[table_basename] = flagcmd_relpath

        return htmlreports

    def _write_flagcmd_to_disk(self, weblog_dir, result):
        tablename = os.path.basename(result.vis)
        filename = os.path.join(weblog_dir, '%s-flag_commands.txt' % tablename)
        flagcmds = [l.flagcmd for l in result.flagcmds()]
        with open(filename, 'w') as flagfile:
            flagfile.writelines(['# Flag commands for %s\n#\n' % tablename])
            flagfile.writelines(['%s\n' % cmd for cmd in flagcmds])
            if not flagcmds:
                flagfile.writelines(['# No flag commands generated\n'])
        return filename

