import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.utils as utils
import pipeline.domain.measures as measures

from pipeline.hif.tasks.makeimlist import makeimlist

import math

LOG = infrastructure.get_logger(__name__)


class CheckProductSizeHeuristics(object):
    def __init__(self, inputs):
        self.inputs = inputs
        self.context = inputs.context

    def calculate_sizes(self, imlist):
        cubesizes = []
        productsize = 0.0
        for target in imlist:
            nx, ny = target['imsize']
            nchan = self.context.observing_run.measurement_sets[0].get_spectral_window(target['spw']).num_channels
            if target['nbin'] != -1:
                nbin = target['nbin']
            else:
                nbin = 1
            mfssize = 4. * nx * ny / 1e9 # Should include nterms, though overall size is dominated by cube which is currently always nterms=1
            cubesize = 4. * nx * ny * nchan / nbin / 1e9
            cubesizes.append(cubesize)
            productsize += 2.0 * (mfssize + cubesize)

        return cubesizes, max(cubesizes), productsize

    def mitigate_sizes(self):

        # Initialize mitigation parameter dictionary
        # Possible keys:
        # 'nbins', 'hm_imsize', 'hm_cell', 'field'
        size_mitigation_parameters = {}

        # Create makeimlist inputs
        makeimlist_inputs = makeimlist.MakeImListInputs(self.context)
        makeimlist_inputs.intent = 'TARGET'
        makeimlist_inputs.specmode = 'cube'

        # Create makeimlist task for size calculations
        makeimlist_task = makeimlist.MakeImList(makeimlist_inputs)

        # Get default target setup
        makeimlist_result = makeimlist_task.prepare()
        imlist = makeimlist_result.targets

        # Extract some information for later
        fields = list(set([i['field'] for i in imlist]))
        nfields = len(fields)
        spws = list(set([i['spw'] for i in imlist]))
        # Use first MS in list, assuming that spw definition is the same for all MSs
        ref_ms = self.context.observing_run.measurement_sets[0]
        nchans = dict([(spw, ref_ms.get_spectral_window(spw).num_channels) for spw in spws])
        ch_width_ratios = dict([(spw, \
            float(ref_ms.get_spectral_window(spw).channels[0].effective_bw.convert_to(measures.FrequencyUnits.HERTZ).value) / \
            float(ref_ms.get_spectral_window(spw).channels[0].getWidth().convert_to(measures.FrequencyUnits.HERTZ).value)) \
            for spw in spws])

        if len(fields) == 0:
            LOG.error('Cannot determine any default imaging targets')
            return {}, 0.0, 0.0, True, {'longmsg': 'Cannot determine any default imaging targets', 'shortmsg': 'Cannot determine targets'}

        # Get representative target information
        repr_target, \
        repr_source, \
        repr_spw, \
        reprBW_mode, \
        real_repr_target, \
        minAcceptableAngResolution, \
        maxAcceptableAngResolution = \
            imlist[0]['heuristics'].representative_target()

        # Get original maximum cube and product sizes
        cubesizes, maxcubesize, productsize = self.calculate_sizes(imlist)
        original_maxcubesize = maxcubesize
        original_productsize = productsize
        LOG.info('Default imaging leads to a maximum cube size of %s GB and a product size of %s GB' % (maxcubesize, productsize))
        LOG.info('Allowed maximum cube size: %s GB. Allowed cube size limit: %s GB. Allowed maximum product size: %s GB.' % (self.inputs.maxcubesize, self.inputs.maxcubelimit, self.inputs.maxproductsize))

        # If too large, try to mitigate via channel binning
        if (self.inputs.maxcubesize != -1.0) and (maxcubesize > self.inputs.maxcubesize):
            nbins = []
            for spw, nchan in nchans.iteritems():
                if (nchan == 3840) or (nchan in (1920, 960, 480) and utils.approx_equal(ch_width_ratios[spw], 2.667, 4)):
                    LOG.info('Size mitigation: Setting nbin for SPW %s to 2.' % (spw))
                    nbins.append('%s:2' % (spw))
                else:
                    nbins.append('%s:1' % (spw))
            size_mitigation_parameters['nbins'] = ','.join(nbins)

            # Recalculate sizes
            makeimlist_inputs.nbins = size_mitigation_parameters['nbins']
            makeimlist_result = makeimlist_task.prepare()
            imlist = makeimlist_result.targets
            cubesizes, maxcubesize, productsize = self.calculate_sizes(imlist)
            LOG.info('nbin mitigation leads to a maximum cube size of %s GB' % (maxcubesize))

        # If still too large, try changing the FoV (in makeimlist this is applied to single fields only)
        if (self.inputs.maxcubesize != -1.0) and (maxcubesize > self.inputs.maxcubesize):

            # Calculate PB level at which the largest cube size of all targets
            # is equal to the maximum allowed cube size.
            PB_mitigation = math.exp(math.log(0.2) * self.inputs.maxcubesize / maxcubesize)
            # Account for imsize padding
            PB_mitigation = 1.02 * PB_mitigation
            # Cap at PB=0.7
            PB_mitigation = min(PB_mitigation, 0.7)
            # Round to 2 significant digits
            PB_mitigation = round(PB_mitigation, 2)

            LOG.info('Size mitigation: Setting hm_imsize to %.2gpb' % (PB_mitigation))
            size_mitigation_parameters['hm_imsize'] = '%.2gpb' % (PB_mitigation)

            # Recalculate sizes
            makeimlist_inputs.hm_imsize = size_mitigation_parameters['hm_imsize']
            makeimlist_result = makeimlist_task.prepare()
            imlist = makeimlist_result.targets
            cubesizes, maxcubesize, productsize = self.calculate_sizes(imlist)
            LOG.info('hm_imsize mitigation leads to a maximum cube size of %s GB' % (maxcubesize))

        # If still too large, try changing pixperbeam setting
        if (self.inputs.maxcubesize != -1.0) and (maxcubesize > self.inputs.maxcubesize):
            size_mitigation_parameters['hm_cell'] = '3ppb'
            LOG.info('Size mitigation: Setting hm_cell to 3ppb')

            # Recalculate sizes
            makeimlist_inputs.hm_cell = size_mitigation_parameters['hm_cell']
            makeimlist_result = makeimlist_task.prepare()
            imlist = makeimlist_result.targets
            cubesizes, maxcubesize, productsize = self.calculate_sizes(imlist)
            LOG.info('hm_cell mitigation leads to a maximum cube size of %s GB' % (maxcubesize))

        # Save cube mitigated product size for logs
        cube_mitigated_productsize = productsize

        # If still too large, stop with an error
        if (self.inputs.maxcubesize != -1.0) and (maxcubesize > self.inputs.maxcubesize):
            if maxcubesize > self.inputs.maxcubelimit:
                LOG.error('Maximum cube size cannot be mitigated. Remaining factor: %.4f and cube size larger than limit of %s GB.' % (maxcubesize / self.inputs.maxcubesize, self.inputs.maxcubelimit))
                return size_mitigation_parameters, \
                       original_maxcubesize, original_productsize, \
                       cube_mitigated_productsize, \
                       maxcubesize, productsize, \
                       True, \
                       {'longmsg': 'Maximum cube size cannot be mitigated. Remaining factor: %.4f and cube size larger than limit of %s GB.' % (maxcubesize / self.inputs.maxcubesize, self.inputs.maxcubelimit), \
                        'shortmsg': 'Cube size mitigation error'}
            else:
                LOG.info('Maximum cube size cannot be mitigated. Remaining factor: %.4f. But cube size is smaller than limit of %s GB.' % (maxcubesize / self.inputs.maxcubesize, self.inputs.maxcubelimit))

        # If product size too large, try reducing number of fields / targets
        if (self.inputs.maxproductsize != -1.0) and (productsize > self.inputs.maxproductsize):
            nfields = int(self.inputs.maxproductsize / (productsize / len(fields)))
            if nfields == 0:
                nfields = 1

            # Truncate the field list
            mitigated_fields = fields[:nfields]

            # Make sure the representative source is included in the new list
            for field in fields[nfields:]:
                if utils.dequote(field) == utils.dequote(repr_source):
                    mitigated_fields[0] = field
                    break

            size_mitigation_parameters['field'] = ','.join(mitigated_fields)

            LOG.info('Size mitigation: Setting field to %s' % (size_mitigation_parameters['field']))

            # Recalculate sizes
            makeimlist_inputs.field = size_mitigation_parameters['field']
            makeimlist_result = makeimlist_task.prepare()
            imlist = makeimlist_result.targets
            cubesizes, maxcubesize, productsize = self.calculate_sizes(imlist)
            LOG.info('field / target mitigation leads to product size of %s GB' % (productsize))

        # If product size is still too large, try mitigating further with nbin, FoV, and cell size, otherwise stop with an error
        if (self.inputs.maxproductsize != -1.0) and (productsize > self.inputs.maxproductsize):
            LOG.info('Product size with single target is still too large. Trying nbin mitigation.')

            nbins = []
            for spw, nchan in nchans.iteritems():
                if (nchan == 3840) or (nchan in (1920, 960, 480) and utils.approx_equal(ch_width_ratios[spw], 2.667, 4)):
                    LOG.info('Size mitigation: Setting nbin for SPW %s to 2.' % (spw))
                    nbins.append('%s:2' % (spw))
                else:
                    nbins.append('%s:1' % (spw))
            size_mitigation_parameters['nbins'] = ','.join(nbins)

            # Recalculate sizes
            makeimlist_inputs.nbins = size_mitigation_parameters['nbins']
            makeimlist_result = makeimlist_task.prepare()
            imlist = makeimlist_result.targets
            cubesizes, maxcubesize, productsize = self.calculate_sizes(imlist)
            LOG.info('nbin mitigation leads to a maximum cube size of %s GB' % (maxcubesize))

        if (self.inputs.maxproductsize != -1.0) and (productsize > self.inputs.maxproductsize):
            LOG.info('Product size with single target is still too large. Trying FoV mitigation.')

            # Calculate PB level at which the largest cube size of all targets
            # is equal to the maximum allowed cube size.
            PB_mitigation = math.exp(math.log(0.2) * self.inputs.maxproductsize / maxcubesize)
            # Account for imsize padding
            PB_mitigation = 1.02 * PB_mitigation
            # Cap at PB=0.7
            PB_mitigation = min(PB_mitigation, 0.7)
            # Round to 2 significant digits
            PB_mitigation = round(PB_mitigation, 2)

            LOG.info('Size mitigation: Setting hm_imsize to %.2gpb' % (PB_mitigation))
            size_mitigation_parameters['hm_imsize'] = '%.2gpb' % (PB_mitigation)

            # Recalculate sizes
            makeimlist_inputs.hm_imsize = size_mitigation_parameters['hm_imsize']
            makeimlist_result = makeimlist_task.prepare()
            imlist = makeimlist_result.targets
            cubesizes, maxcubesize, productsize = self.calculate_sizes(imlist)
            LOG.info('hm_imsize mitigation leads to a maximum cube size of %s GB' % (maxcubesize))

        if (self.inputs.maxproductsize != -1.0) and (productsize > self.inputs.maxproductsize):
            LOG.info('Product size with single target is still too large. Trying cell size mitigation.')

            size_mitigation_parameters['hm_cell'] = '3ppb'
            LOG.info('Size mitigation: Setting hm_cell to 3ppb')

            # Recalculate sizes
            makeimlist_inputs.hm_cell = size_mitigation_parameters['hm_cell']
            makeimlist_result = makeimlist_task.prepare()
            imlist = makeimlist_result.targets
            cubesizes, maxcubesize, productsize = self.calculate_sizes(imlist)
            LOG.info('hm_cell mitigation leads to a maximum cube size of %s GB' % (maxcubesize))

        # Check if there is more than one spw leading to cubes larger than
        # 0.5 * maxcubelimit. Remove all but one of these spws and make sure
        # the representative spw is still included.
        spw_oversizes = dict([(i, 0) for i in spws])
        for i, target in enumerate(imlist):
            if cubesizes[i] > 0.5 * self.inputs.maxcubelimit:
                spw_oversizes[target['spw']] += 1

        if [n != 0 for n in spw_oversizes.itervalues()].count(True) > 1:
            oversize_spws = [spw for spw, n in spw_oversizes.iteritems() if n>0]
            if str(repr_spw) in oversize_spws:
                size_mitigation_parameters['spw'] = ','.join([str(repr_spw)] + [spw for spw, n in spw_oversizes.iteritems() if n==0])
            else:
                size_mitigation_parameters['spw'] = ','.join(oversize_spws[0] + [spw for spw, n in spw_oversizes.iteritems() if n==0])
            LOG.info('Size mitigation: Setting (cube) spw to %s' % (size_mitigation_parameters['spw']))

            # Recalculate sizes
            makeimlist_inputs.spw = size_mitigation_parameters['spw']
            makeimlist_result = makeimlist_task.prepare()
            imlist = makeimlist_result.targets
            cubesizes, maxcubesize, productsize = self.calculate_sizes(imlist)
            LOG.info('spw mitigation leads to product size of %s GB' % (productsize))

        # Save cube mitigated product size for logs
        cube_mitigated_productsize = productsize

        if (self.inputs.maxproductsize != -1.0) and (productsize > self.inputs.maxproductsize):
            LOG.error('Product size cannot be mitigated. Remaining factor: %.4f.' % (productsize / self.inputs.maxproductsize / nfields))
            return size_mitigation_parameters, \
                   original_maxcubesize, original_productsize, \
                   cube_mitigated_productsize, \
                   maxcubesize, productsize, \
                   True, \
                   {'longmsg': 'Product size cannot be mitigated. Remaining factor: %.4f.' % (productsize / self.inputs.maxproductsize / nfields), \
                    'shortmsg': 'Product size mitigation error'}

        # Check for case with many targets which will cause long run times in spite
        # of any mitigation.
        max_num_sciencetargets = 30
        if (nfields > max_num_sciencetargets) and (all([nchan > 960 for nchan in nchans.values()])):
            LOG.warn('The number of science targets is > 30 and the total number of spectral channels across all science spws > 960. The imaging pipeline will take substantial time to run on this MOUS.')

        if size_mitigation_parameters != {}:
            return size_mitigation_parameters, \
                   original_maxcubesize, original_productsize, \
                   cube_mitigated_productsize, \
                   maxcubesize, productsize, \
                   False, \
                   {'longmsg': 'Size had to be mitigated (%s)' % (','.join(size_mitigation_parameters.iterkeys())), \
                    'shortmsg': 'Size was mitigated'}
        else:
            return size_mitigation_parameters, \
                   original_maxcubesize, original_productsize, \
                   cube_mitigated_productsize, \
                   maxcubesize, productsize, \
                   False, \
                   {'longmsg': 'No size mitigation needed', \
                    'shortmsg': 'No size mitigation'}
