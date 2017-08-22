from __future__ import absolute_import

import os
import shutil

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.utils as utils
from .basecleansequence import BaseCleanSequence
from .resultobjects import BoxResult

LOG = infrastructure.get_logger(__name__)


class AutoMaskThresholdSequence(BaseCleanSequence):

    def __init__(self, gridder, threshold='0.0mJy', sensitivity=0.0, niter=5000):
        """Constructor.
        """
        BaseCleanSequence.__init__(self)
        self.gridder = gridder
        self.threshold = threshold
        self.sensitivity = sensitivity
        self.dr_corrected_sensitivity = sensitivity
        self.niter = niter
        self.iter = None
        self.result = BoxResult()
        self.sidelobe_ratio = -1

    def iteration(self, new_cleanmask, pblimit_image=-1, pblimit_cleanmask=-1, spw=None, frequency_selection=None, keep_iterating=False):

        if (self.multiterm):
            extension = '.tt0'
        else:
            extension = ''

        if self.iter is None:
            raise Exception, 'no data for iteration'

        elif self.iter == 0:
            self.result.cleanmask = new_cleanmask
            self.result.threshold = self.threshold
            self.result.sensitivity = self.sensitivity
            self.result.niter =  self.niter
            self.result.iterating = True
        elif self.iter == 1 and keep_iterating:
            if self.flux not in (None, ''):
                # Make a circular one
                cm = casatools.image.newimagefromimage(infile=self.flux+extension,
                  outfile=new_cleanmask, overwrite=True)
                # verbose = False to suppress warning message
                cm.calcmask('T')
                cm.calc('1', verbose=False)
                cm.calc('replace("%s"["%s" > %f], 0)' % (os.path.basename(new_cleanmask), self.flux+extension, pblimit_cleanmask), verbose=False)
                cm.calcmask('"%s" > %s' % (self.flux+extension, str(pblimit_image)))
                cm.done()

                if frequency_selection is not None:
                    channel_ranges = []
                    for spwid in spw.split(','):
                        spwkey = 'spw%s' % (spwid)
                        if frequency_selection.has_key(spwkey):
                            if (frequency_selection[spwkey] not in (None, 'NONE', '')):
                                channel_ranges.extend(utils.freq_selection_to_channels(new_cleanmask, frequency_selection[spwkey].split()[0]))
                    if channel_ranges != []:
                        with casatools.ImageReader(new_cleanmask) as iaTool:
                            shape = iaTool.shape()
                            rgTool = casatools.regionmanager
                            for channel_range in channel_ranges:
                                LOG.info('Unmasking channels %d to %d' % (channel_range[0], channel_range[1]))
                                region = rgTool.box([0,0,0,channel_range[0]], [shape[0]-1, shape[1]-1, 0, channel_range[1]])
                                iaTool.set(region=region, pixels=0.0, pixelmask=False)
                            rgTool.done()

            self.result.cleanmask = new_cleanmask
            # CAS-10489: old centralregion option needs higher threshold
            cqa = casatools.quanta
            self.result.threshold = '%sJy' % (cqa.getvalue(cqa.mul(self.threshold, 2.0))[0])
            self.result.sensitivity = self.sensitivity
            self.result.niter = self.niter
            self.result.iterating = keep_iterating

        else:
            self.result.cleanmask = ''
            self.result.threshold = '0.0mJy'
            self.result.sensitivity = 0.0
            self.result.niter =  0
            self.result.iterating = False

        return self.result
