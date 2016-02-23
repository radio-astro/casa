'''
Created on 11 Sep 2014

@author: sjw
'''

import os
import types
import numpy as np

import pipeline.hif.tasks.bandpass.renderer as baserenderer
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.casatools as casatools

LOG = logging.get_logger(__name__)


class T2_4MDetailsBandpassRenderer(baserenderer.T2_4MDetailsBandpassRenderer):
    """
    T2_4MDetailsBandpassRenderer generates the detailed T2_4M-level plots and
    output specific to the bandpass calibration task.
    """
    def __init__(self, uri='bandpass.mako', 
                 description='Phase-up bandpass calibration',
                 always_rerender=False):
        super(T2_4MDetailsBandpassRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def get_phaseup_applications(self, context, result, ms):
        hm_phaseup = result.inputs.get('hm_phaseup', 'N/A')
        if not hm_phaseup:
            return []
        
        calmode_map = {'p':'Phase only',
                       'a':'Amplitude only',
                       'ap':'Phase and amplitude'}
        
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

            LOG.todo('Make all CalAppOrigins a list?')
            if type(calapp.origin) is not types.ListType:
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
                    solint = self._format_bandpass_solint (ms, solint, spws[0])

                # TODO get this from the calapp rather than the top-level
                # inputs?
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
            afreqsolint = timesolint + ',' + freqsolint  + '(%0.1fch)' % (nchan) 

        return afreqsolint

