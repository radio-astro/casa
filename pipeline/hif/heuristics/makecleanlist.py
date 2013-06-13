
import math
import numpy as np
import os.path
import re

import pipeline.domain.measures as measures
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.utils as utils

LOG = infrastructure.get_logger(__name__)


class MakeCleanListHeuristics(object):

    def __init__(self, context, vislist, spw):
        self.context = context
        self.vislist = vislist
        self.spwidlist = map(int, spw.split(','))

        # calculate beam radius for all spwids, saves repetition later 
        self.beam_radius = {}

        # get the diameter of the smallest antenna used among all vis sets
        diameters = []
        for vis in self.vislist:
            ms = self.context.observing_run.get_ms(name=vis)
            antennas = ms.antennas
            for antenna in antennas:
                diameters.append(antenna.diameter)
        smallest_diameter = np.min(np.array(diameters))

        # get spw info from first vis set, assume spws uniform
        # across datasets
        ms = self.context.observing_run.get_ms(name=vislist[0])
        for spwid in self.spwidlist:
            spw = ms.get_spectral_window(spwid)
            ref_frequency = float(
              spw.ref_frequency.to_units(measures.FrequencyUnits.HERTZ))

            # use the smallest antenna diameter and the reference frequency
            # to estimate the primary beam radius -
            # radius of first null in arcsec = 1.22*lambda/D
            self.beam_radius[spwid] = \
              (1.22 * (3.0e8/ref_frequency) / smallest_diameter) * \
              (180.0 * 3600.0 / math.pi)

    def field_intent_list(self, intent, field):
        intent_list = intent.split(',')
        field_list = utils.safe_split(field)

        field_intent_result = set()

        for vis in self.vislist:
            ms = self.context.observing_run.get_ms(name=vis)
            fields = ms.fields

            if intent.strip() is not '':
                for eachintent in intent_list:
                    re_intent = eachintent.replace('*', '.*')
                    field_intent_result.update(
                      [(fld.name, eachintent) for fld in fields if 
                      re.search(pattern=re_intent, string=str(fld.intents))])

                if field.strip() is not '':
                    # remove items from field_intent_result that are not
                    # consistent with the fields in field_list
                    temp_result = set()
                    re_field = field
                    for char in '()+?.^$[]{}|':
                        re_field = re_field.replace(char, '\%s' % char)
                    re_field = re_field.replace('*', '.*')
                    for eachfield in field_list:
                        temp_result.update([fir for fir in field_intent_result
                          if re.search(pattern=re_field, string=fir[0])])
                    field_intent_result = temp_result

            else:
                if field.strip() is not '':
                    for f in field_list:
                        for char in '()+?.^$[]{}|':
                            f = f.replace(char, '\%s' % char)
                        f = f.replace('*', '.*')
                        fintents_list = [fld.intents for fld in fields if 
                          re.search(pattern=f, string=fld.name)]
                        for fintents in fintents_list:
                            for fintent in fintents:
                                field_intent_result.update((f, fintent))
                            
        return field_intent_result

    def cell(self, field_intent_list, spwid, oversample=3.0):
        # reset state of imager
        casatools.imager.done()

        # put code in try-block to ensure that imager.done gets
        # called at the end
        try:
            cellvs = []
            for field_intent in field_intent_list:
                # select data to be imaged
                for vis in self.vislist:
                    ms = self.context.observing_run.get_ms(name=vis)
                    scanids = [scan.id for scan in ms.scans if
                      field_intent[1] in scan.intents and 
                      field_intent[0] in [fld.name for fld in scan.fields]]
                    scanids = str(scanids)
                    scanids = scanids.replace('[', '')
                    scanids = scanids.replace(']', '')
                    try:
                        casatools.imager.selectvis(vis=vis,
                          field=field_intent[0], spw=spwid, scan=scanids,
                          usescratch=False)
                    except:
                        LOG.warning('%s: failed selection on field %s' %
                          (os.path.basename(vis), field_intent[0]))
                casatools.imager.weight(type='natural')

                # use imager.advise to get the maximum cell size
                aipsfieldofview = '%4.1farcsec' % (2.0 *
                  self.beam_radius[spwid])
                rtn = casatools.imager.advise(takeadvice=False,
                  amplitudeloss=0.5, fieldofview=aipsfieldofview)
                cellv = rtn[2]['value']
                cellu = rtn[2]['unit']
                cellv /= oversample
                cellvs.append(cellv)
                LOG.debug('cell (oversample %s) for %s/%s spw %s: %s' % (
                  oversample, field_intent[0], field_intent[1], spwid, cellv))
                    
            cell = '%.3f%s' % (min(cellvs), cellu)
            LOG.debug('RESULT cell for spw %s: %s' % (spwid, cell))

        finally:
            casatools.imager.done()

        return cell
