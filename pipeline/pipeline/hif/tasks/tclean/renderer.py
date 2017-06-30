"""
Created on 7 Jan 2015

@author: sjw
"""
import collections
import os

import numpy

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.displays.tclean as tclean
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates

LOG = logging.get_logger(__name__)


class T2_4MDetailsTcleanRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='tclean.mako', 
                 description='Produce a cleaned image', 
                 always_rerender=False):
        super(T2_4MDetailsTcleanRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, ctx, context, results):
        weblog_dir = os.path.join(context.report_dir,
                                  'stage%s' % results.stage_number)

        # There is only ever one CleanListResult in the ResultsList as it
        # operates over multiple measurement sets, so we can set the result to
        # the first item in the list

        # Get results info
        info_dict = {}

        # Holds a mapping of image name to image stats. This information is used to scale the MOM8 images.
        image_stats = {}

        qaTool = casatools.quanta

        if results[0]:
            cresults = results[0].results

            for r in cresults:
                if r.empty():
                    continue
                if not r.iterations:
                    continue
                if r.multiterm:
                    extension = '.tt0'
                else:
                    extension = ''

                maxiter = max(r.iterations.keys())

                field = spw = pol = None

                image_path = r.iterations[maxiter]['image'].replace('.image', '.image%s' % (extension))

                LOG.info('Getting properties of %s for the weblog.' % (image_path))

                with casatools.ImageReader(image_path) as image:
                    info = image.miscinfo()
                    spw = info.get('spw', None)
                    if 'field' in info:
                        field = '%s (%s)' % (info['field'], r.intent)

                    coordsys = image.coordsys()
                    coord_names = numpy.array(coordsys.names())
                    coord_refs = coordsys.referencevalue(format='s')
                    pol = coord_refs['string'][coord_names == 'Stokes'][0]
                    info_dict[(field, spw, pol, 'image name')] = image.name(strippath=True)

                    stats = image.statistics(robust=False)
                    beam = image.restoringbeam()
                    if 'beams' in beam:
                        # 'beams' dict has results for each channel and
                        # each pol product. For now, just use the first beam.
                        beam = beam['beams']['*0']['*0']
                        LOG.warning('%s has per-plane beam shape, displaying '
                                    'only first', 
                                    r.iterations[maxiter]['image'].replace('.image', '.image%s' % (extension)))
                    try:
                        major = qaTool.convert(beam['major'], 'arcsec')
                        info_dict[(field, spw, pol, 'beam major')] = major
                        minor = qaTool.convert(beam['minor'], 'arcsec')
                        info_dict[(field, spw, pol, 'beam minor')] = minor
                        pa = qaTool.convert(beam['positionangle'], 'deg')
                        info_dict[(field, spw, pol, 'beam pa')] = pa
                    except:
                        info_dict[(field, spw, pol, 'beam major')] = None
                        info_dict[(field, spw, pol, 'beam minor')] = None
                        info_dict[(field, spw, pol, 'beam pa')] = None

                    info_dict[(field, spw, pol, 'brightness unit')] = image.brightnessunit()

                    # These should be obsolete since we need to measure these quantities
                    # with proper masking.
                    image_rms = stats.get('rms')[0]
                    image_max = stats.get('max')[0]
                    info_dict[(field, spw, pol, 'image rms')] = image_rms
                    info_dict[(field, spw, pol, 'max')] = image_max
                    image_stats[image_path] = tclean.ImageStats(rms=image_rms, max=image_max)

                    summary = image.summary()
                    nchan = summary['shape'][3]
                    width = qaTool.quantity(summary['incr'][3], 
                                        summary['axisunits'][3])
                    width = qaTool.convert(width, 'MHz')
                    width = qaTool.tos(width, 4)

                    info_dict[(field, spw, pol, 'nchan')] = nchan
                    info_dict[(field, spw, pol, 'width')] = width

                    aggregate_bw_GHz = qaTool.convert(r.aggregate_bw, 'GHz')['value']
                    aggregate_bw_text = '%.3g GHz (LSRK)' % (aggregate_bw_GHz)
                    info_dict[(field, spw, pol, 'aggregate bandwidth')] = aggregate_bw_text

                    eff_ch_bw_MHz = qaTool.convert(r.eff_ch_bw, 'MHz')['value']
                    eff_ch_bw_text = '%.5g MHz (TOPO)' % (eff_ch_bw_MHz)
                    info_dict[(field, spw, pol, 'effective channel bandwidth')] = eff_ch_bw_text

                    try:
                        frequency_axis = list(summary['axisnames']).index('Frequency')

                        center_frequency = summary['refval'][frequency_axis] + (summary['shape'][frequency_axis] / 2.0 - 0.5 - summary['refpix'][frequency_axis]) * summary['incr'][frequency_axis]
                        center_frequency = qaTool.convert('%s %s' % (center_frequency, summary['axisunits'][frequency_axis]), 'GHz')
                        info_dict[(field, spw, pol, 'frequency')] = center_frequency

                        frequency1 = summary['refval'][frequency_axis] + (-0.5 - summary['refpix'][frequency_axis]) * summary['incr'][frequency_axis]
                        frequency2 = summary['refval'][frequency_axis] + (summary['shape'][frequency_axis] - 0.5 - summary['refpix'][frequency_axis]) * summary['incr'][frequency_axis]
                        full_bw_GHz = qaTool.convert(abs(frequency2 - frequency1), 'GHz')['value']
                        fractional_bw = (frequency2 - frequency1) / (0.5 * (frequency1 + frequency2))
                        info_dict[(field, spw, pol, 'fractional bandwidth')] = '%.2g%%' % (fractional_bw * 100.)
                    except:
                        info_dict[(field, spw, pol, 'fractional bandwidth')] = 'N/A'

                    info_dict[(field, spw, pol, 'nterms')] = r.multiterm if r.multiterm else 1

                with casatools.ImageReader(r.iterations[maxiter]['residual']+extension) as residual:
                    stats = residual.statistics(robust=True)
                    info_dict[(field, spw, pol, 'residual rms')] = stats.get('rms')[0]
                    residual_robust_rms = stats.get('medabsdevmed')[0] * 1.4826  # see CAS-9631
                    residual_max = stats.get('max')[0]
                    info_dict[(field, spw, pol, 'residual peak/rms')] = residual_max / residual_robust_rms

                # The min, max and RMS values need to be taken with proper masking.
                # Store the one used for QA scoring.
                info_dict[(field, spw, pol, 'non-masked min')] = r.image_min
                info_dict[(field, spw, pol, 'non-masked max')] = r.image_max
                info_dict[(field, spw, pol, 'masked rms')] = r.image_rms
                info_dict[(field, spw, pol, 'sensitivity')] = r.sensitivity
                info_dict[(field, spw, pol, 'min sensitivity')] = r.min_sensitivity
                info_dict[(field, spw, pol, 'max sensitivity')] = r.max_sensitivity
                info_dict[(field, spw, pol, 'min field id')] = r.min_field_id
                info_dict[(field, spw, pol, 'max field id')] = r.max_field_id
                info_dict[(field, spw, pol, 'threshold')] = r.threshold
                info_dict[(field, spw, pol, 'dirty DR')] = r.dirty_dynamic_range
                info_dict[(field, spw, pol, 'DR correction factor')] = r.DR_correction_factor
                info_dict[(field, spw, pol, 'maxEDR used')] = r.maxEDR_used
                info_dict[(field, spw, pol, 'score')] = r.qa.representative

        # Make the plots
        plotter = tclean.CleanSummary(context, results[0], image_stats)
        plots = plotter.plot()        

        fields = sorted(set([p.parameters['field'] for p in plots]))
        spws = sorted(set([p.parameters['spw'] for p in plots]))
        iterations = sorted(set([p.parameters['iter'] for p in plots]))
        types = sorted(set([p.parameters['type'] for p in plots]))
        
        iteration_dim = lambda : collections.defaultdict(dict)
        spw_dim = lambda : collections.defaultdict(iteration_dim)
        plots_dict = collections.defaultdict(spw_dim)
        for field in fields:
            for spw in spws:
                for iteration in iterations:
                    for t in types:
                        matching = [p for p in plots
                                    if p.parameters['field'] == field
                                    and p.parameters['spw'] == spw
                                    and p.parameters['iter'] == iteration
                                    and p.parameters['type'] == t]
                        if matching:
                            plots_dict[field][spw][iteration][t] = matching[0]

        ctx.update({
            'fields': fields,
            'spws': spws,
            'iterations': iterations,
            'plots': plots,
            'plots_dict': plots_dict,
            'info_dict': info_dict,
            'dirname': weblog_dir
        })


class TCleanPlotsRenderer(basetemplates.CommonRenderer):
    def __init__(self, context, result, plots_dict, field, spw, pol):
        super(TCleanPlotsRenderer, self).__init__('tcleanplots.mako', context, 
                                                 result)
        
        outfile = 'field%s-spw%s-pol%s-cleanplots.html' % (field, spw, pol)
        self.path = os.path.join(self.dirname, filenamer.sanitize(outfile))
                
        # Determine whether any of targets were run with specmode = 'cube',
        # in which case the weblog will need to show the MOM0_FC and
        # MOM8_FC columns.
        show_mom0_8_fc = any([item.specmode == 'cube' for item in result[0].results])
        
        if show_mom0_8_fc:
            colorder = ['pbcorimage', 'residual', 'cleanmask', 'mom0_fc', 'mom8_fc']
        else:
            colorder = ['pbcorimage', 'residual', 'cleanmask']
        
        self.extra_data = {'plots_dict' : plots_dict,
                           'field'      : field,
                           'spw'        : spw,
                           'colorder'   : colorder}
    
    def update_mako_context(self, mako_context):
        mako_context.update(self.extra_data)
