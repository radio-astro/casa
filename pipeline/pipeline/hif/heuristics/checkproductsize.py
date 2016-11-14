import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.utils as utils

from pipeline.hif.tasks.makeimlist import makeimlist

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
            mfssize = 4. * nx * ny / 1e9 # Should include nterms
            cubesize = 4. * nx * ny * nchan / 1e9
            cubesizes.append(cubesize)
            productsize += 2.0 * (mfssize + cubesize)

        return max(cubesizes), productsize

    def mitigate_sizes(self):

        # Initialize mitigation parameter dictionary
        # Possible keys:
        # 'nbins', 'sfpblimit', 'pixperbeam', 'field'
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
        spws = list(set([i['spw'] for i in imlist]))
        nchans = dict([(spw, self.context.observing_run.measurement_sets[0].get_spectral_window(spw).num_channels) for spw in spws])

        if len(fields) == 0:
            return {}, 0.0, 0.0, True, {'longmsg': 'Cannot determine any default imaging targets', 'shortmsg': 'Cannot determine targets'}

        # Get product sizes
        maxcubesize, productsize = self.calculate_sizes(imlist)

        # If too large, check for nchan >= 3840
        if maxcubesize > self.inputs.maxcubesize:
            nbins = []
            for spw, nchan in nchans.iteritems():
                if nchan >= 3840:
                    nbins.append('%s:2' % (spw))
                else:
                    nbins.append('%s:1' % (spw))
            size_mitigation_parameters['nbins'] = ','.join(nbins)

            # Recalculate sizes
            makeimlist_inputs.nbins = size_mitigation_parameters['nbins']
            makeimlist_result = makeimlist_task.prepare()
            imlist = makeimlist_result.targets
            maxcubesize, productsize = self.calculate_sizes(imlist)

        # If still too large, check for nchan >= 1920
        if maxcubesize > self.inputs.maxcubesize:
            nbins = []
            for spw, nchan in nchans.iteritems():
                if nchan >= 1920:
                    nbins.append('%s:2' % (spw))
                else:
                    nbins.append('%s:1' % (spw))
            size_mitigation_parameters['nbins'] = ','.join(nbins)

            # Recalculate sizes
            makeimlist_inputs.nbins = size_mitigation_parameters['nbins']
            makeimlist_result = makeimlist_task.prepare()
            imlist = makeimlist_result.targets
            maxcubesize, productsize = self.calculate_sizes(imlist)

        # If still too large, try changing FOV
        if maxcubesize > self.inputs.maxcubesize:
            if maxcubesize < 1.4 * self.inputs.maxcubesize:
                size_mitigation_parameters['sfpblimit'] = 0.3
            else:
                size_mitigation_parameters['sfpblimit'] = 0.5

            # Recalculate sizes
            makeimlist_inputs.sfpblimit = size_mitigation_parameters['sfpblimit']
            makeimlist_result = makeimlist_task.prepare()
            imlist = makeimlist_result.targets
            maxcubesize, productsize = self.calculate_sizes(imlist)

        # If still too large, try changing pixperbeam setting
        if maxcubesize > self.inputs.maxcubesize:
            size_mitigation_parameters['pixperbeam'] = 3

            # Recalculate sizes
            makeimlist_inputs.sfpblimit = size_mitigation_parameters['sfpblimit']
            makeimlist_result = makeimlist_task.prepare()
            imlist = makeimlist_result.targets
            maxcubesize, productsize = self.calculate_sizes(imlist)

        # If still too large, stop with an error
        if maxcubesize > self.inputs.maxcubesize:
            return size_mitigation_parameters, \
                   maxcubesize, \
                   productsize, \
                   True, \
                   {'longmsg': 'Maximum cube size cannot be mitigated. Remaining factor: %.4f' % (maxcubesize / self.inputs.maxcubesize), \
                    'shortmsg': 'Cube size mitigation error'}

        # If product size too large, try reducing number of fields
        if productsize > self.inputs.maxproductsize:
            # If product size is exceeded with single field, stop with an error
            if productsize / self.inputs.maxproductsize / len(fields) > 1:
                return size_mitigation_parameters, \
                       maxcubesize, productsize, \
                       True, \
                       {'longmsg': 'Product size cannot be mitigated. Remaining factor: %.4f.' % (productsize / self.inputs.maxproductsize / len(fields)), \
                        'shortmsg': 'Product size mitigation error'}
            else:
                nfields = int(self.inputs.maxproductsize / (productsize / len(fields)))
                size_mitigation_parameters['fields'] = fields[:nfields]

                # Recalculate sizes
                makeimlist_inputs.sfpblimit = size_mitigation_parameters['sfpblimit']
                makeimlist_result = makeimlist_task.prepare()
                imlist = makeimlist_result.targets
                maxcubesize, productsize = self.calculate_sizes(imlist)

        if size_mitigation_parameters != {}:
            return size_mitigation_parameters, \
                   maxcubesize, productsize, \
                   False, \
                   {'longmsg': 'Size mitigation succeeded', \
                    'shortmsg': 'Size mitigation succeeded'}
        else:
            return size_mitigation_parameters, \
                   maxcubesize, productsize, \
                   False, \
                   {'longmsg': 'No size mitigation needed', \
                    'shortmsg': 'No size mitigation'}
