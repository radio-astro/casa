"""
Created on 7 Jan 2015

@author: sjw
"""
import collections
import itertools
import os
import string

import numpy

import pipeline.infrastructure.casatools as casatools
from . import display
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.rendererutils as rendererutils
import pipeline.infrastructure.utils as utils

LOG = logging.get_logger(__name__)


ImageRow = collections.namedtuple('ImageInfo', (
    'field spw spwnames pol frequency_label frequency beam beam_pa sensitivity '
    'cleaning_threshold residual_ratio non_pbcor_label non_pbcor pbcor score '
    'fractional_bw_label fractional_bw aggregate_bw_label aggregate_bw '
    'image_file nchan plot qa_url iterdone stopcode stopreason'))


class T2_4MDetailsTcleanRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='tclean.mako', 
                 description='Produce a cleaned image', 
                 always_rerender=False):
        super(T2_4MDetailsTcleanRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, ctx, context, results):
        # There is only ever one CleanListResult in the ResultsList as it
        # operates over multiple measurement sets, so we can set the result to
        # the first item in the list
        if not results[0]:
            return

        makeimages_result = results[0]
        clean_results = makeimages_result.results

        weblog_dir = os.path.join(context.report_dir, 'stage%s' % results.stage_number)
        qaTool = casatools.quanta

        # Get results info
        image_rows = []

        # Holds a mapping of image name to image stats. This information is used to scale the MOM8 images.
        image_stats = {}

        for r in clean_results:
            if r.empty():
                continue
            if not r.iterations:
                continue
            if r.multiterm:
                extension = '.tt0'
            else:
                extension = ''

            maxiter = max(r.iterations.keys())

            field = None

            image_path = r.iterations[maxiter]['image'].replace('.image', '.image%s' % extension)

            LOG.info('Getting properties of %s for the weblog' % image_path)
            with casatools.ImageReader(image_path) as image:
                image_name = str(image.name(strippath=True))
                info = image.miscinfo()
                coordsys = image.coordsys()
                brightness_unit = image.brightnessunit()
                summary = image.summary()
                beam = image.restoringbeam()

                # While the image tool is open, read and cache the image
                # stats for use in the plot generation classes.
                stats = image.statistics(robust=False)

            # cache image statistics while we have them in scope.
            image_rms = stats.get('rms')[0]
            image_max = stats.get('max')[0]
            image_stats[image_path] = display.ImageStats(rms=image_rms, max=image_max)

            spw = info.get('spw', None)
            if spw is not None:
                nspwnam = info.get('nspwnam', None)
                spwnames = ','.join([info.get('spwnam%02d' % (i+1)) for i in xrange(nspwnam)])
            else:
                spwnames = None
            if 'field' in info:
                field = '%s (%s)' % (info['field'], r.intent)

            coord_names = numpy.array(coordsys.names())
            coord_refs = coordsys.referencevalue(format='s')
            pol = coord_refs['string'][coord_names == 'Stokes'][0]

            #
            # beam calculation
            #
            if 'beams' in beam:
                # 'beams' dict has results for each channel and
                # each pol product. For now, just use the first beam.
                beam = beam['beams']['*0']['*0']
                LOG.warning('%s has per-plane beam shape, displaying only first',
                            r.iterations[maxiter]['image'].replace('.image', '.image%s' % extension))

            #
            # beam value
            #
            try:
                beam_major = qaTool.convert(beam['major'], 'arcsec')
                beam_minor = qaTool.convert(beam['minor'], 'arcsec')
                row_beam = '%#.3g x %#.3g %s' % (beam_major['value'], beam_minor['value'], beam_major['unit'])
            except:
                row_beam = '-'

            #
            # beam position angle
            #
            try:
                beam_pa = qaTool.convert(beam['positionangle'], 'deg')
                row_beam_pa = casatools.quanta.tos(beam_pa, 1)
            except:
                row_beam_pa = '-'

            nchan = summary['shape'][3]
            width = qaTool.quantity(summary['incr'][3], summary['axisunits'][3])
            width = qaTool.convert(width, 'MHz')
            width = qaTool.tos(width, 4)

            # eff_ch_bw_MHz = qaTool.convert(r.eff_ch_bw, 'MHz')['value']
            # eff_ch_bw_text = '%.5g MHz (TOPO)' % (eff_ch_bw_MHz)
            # effective_channel_bandwidth = eff_ch_bw_text

            #
            # centre frequency heading
            #
            if nchan > 1:
                row_frequency_label = 'centre frequency of cube'
            elif nchan == 1:
                row_frequency_label = 'centre frequency of image'
            else:
                row_frequency_label = 'centre frequency'

            #
            # centre frequency value
            #
            try:
                frequency_axis = list(summary['axisnames']).index('Frequency')
                center_frequency = summary['refval'][frequency_axis] + \
                    (summary['shape'][frequency_axis] / 2.0 - 0.5 - summary['refpix'][frequency_axis]) \
                    * summary['incr'][frequency_axis]
                centre_ghz = qaTool.convert('%s %s' % (center_frequency, summary['axisunits'][frequency_axis]),
                                            'GHz')
                row_frequency = '%s (LSRK)' % casatools.quanta.tos(centre_ghz, 4)
            except:
                row_frequency = '-'

            #
            # residual peak / scaled MAD
            #
            with casatools.ImageReader(r.iterations[maxiter]['residual'] + extension) as residual:
                residual_stats = residual.statistics(robust=True)
                residual_abs = abs(residual.getchunk())

            residual_robust_rms = residual_stats.get('medabsdevmed')[0] * 1.4826  # see CAS-9631
            if 'VLA' in r.imaging_mode:
                residual_max = residual_abs.max()  # see CAS-10731
            else:
                residual_max = residual_stats.get('max')[0]
            row_residual_ratio = '%.2f' % (residual_max / residual_robust_rms)

            #
            # theoretical sensitivity
            #
            if 'VLA' in r.imaging_mode:
                row_sensitivity = '-'
            else:
                row_sensitivity = '%.2g %s' % (r.sensitivity, brightness_unit)

            #
            # clean iterations, for VLASS
            #
            if 'VLASS' in r.imaging_mode:
                row_iterdone = r.tclean_iterdone
                row_stopcode = r.tclean_stopcode
                row_stopreason = r.tclean_stopreason
            else:
                row_iterdone = None
                row_stopcode = None
                row_stopreason = None

            #
            # cleaning threshold cell
            #
            if 'VLASS' in r.imaging_mode:
                row_cleaning_threshold = '%.2g' % r.threshold
            elif 'VLA' in r.imaging_mode:
                row_cleaning_threshold = '-'
            else:
                if r.threshold:
                    row_cleaning_threshold = '%.2g %s' % (casatools.quanta.convert(r.threshold, brightness_unit)['value'], brightness_unit)
                    if r.dirty_dynamic_range:
                        row_cleaning_threshold += '<br>Dirty DR: %.2g' % r.dirty_dynamic_range
                        row_cleaning_threshold += '<br>DR correction: %.2g' % r.DR_correction_factor
                    else:
                        row_cleaning_threshold += '<br>No DR information'
                else:
                    row_cleaning_threshold = '-'

            #
            # heading for non-pbcor RMS cell
            #
            if nchan is None:
                non_pbcor_label = 'No RMS information'
            elif nchan == 1:
                non_pbcor_label = 'non-pbcor image RMS'
            else:
                non_pbcor_label = 'non-pbcor image RMS / RMS<sub>min</sub> / RMS<sub>max</sub>'

            #
            # value for non-pbcor RMS cell
            #
            if nchan is None or r.image_rms is None:
                row_non_pbcor = '-'
            elif nchan == 1:
                row_non_pbcor = '%#.2g %s' % (r.image_rms, brightness_unit)
            else:
                row_non_pbcor = '%#.2g / %#.2g / %#.2g %s' % (r.image_rms, r.image_rms_min, r.image_rms_max, brightness_unit)

            #
            # pbcor image max / min cell
            #
            if r.image_max is not None and r.image_min is not None:
                row_pbcor = '%#.3g / %#.3g %s' % (r.image_max, r.image_min, brightness_unit)
            else:
                row_pbcor = '-'

            #
            # fractional bandwidth calculation
            #
            try:
                frequency1 = summary['refval'][frequency_axis] + (-0.5 - summary['refpix'][frequency_axis]) * summary['incr'][frequency_axis]
                frequency2 = summary['refval'][frequency_axis] + (summary['shape'][frequency_axis] - 0.5 - summary['refpix'][frequency_axis]) * summary['incr'][frequency_axis]
                # full_bw_GHz = qaTool.convert(abs(frequency2 - frequency1), 'GHz')['value']
                fractional_bw = (frequency2 - frequency1) / (0.5 * (frequency1 + frequency2))
                fractional_bandwidth = '%.2g%%' % (fractional_bw * 100.)
            except:
                fractional_bandwidth = 'N/A'

            #
            # fractional bandwidth heading and value
            #
            nterms = r.multiterm if r.multiterm else 1
            if nchan is None:
                row_fractional_bw_label = 'No channel / width information'
                row_fractional_bw = '-'
            elif nchan > 1:
                row_fractional_bw_label = 'channels'
                if r.orig_specmode == 'repBW':
                    row_fractional_bw = '%d x %s (repBW, LSRK)' % (nchan, width)
                else:
                    row_fractional_bw = '%d x %s (LSRK)' % (nchan, width)
            else:
                row_fractional_bw_label = 'fractional bandwidth / nterms'
                row_fractional_bw = '%s / %s' % (fractional_bandwidth, nterms)

            #
            # aggregate bandwidth heading
            #
            if nchan == 1:
                row_bandwidth_label = 'aggregate bandwidth'
            else:
                row_bandwidth_label = None

            #
            # aggregate bandwidth value
            #
            aggregate_bw_GHz = qaTool.convert(r.aggregate_bw, 'GHz')['value']
            row_aggregate_bw = '%.3g GHz (LSRK)' % aggregate_bw_GHz

            #
            #  score value
            #
            if r.qa.representative is not None:
                badge_class = rendererutils.get_badge_class(r.qa.representative)
                row_score = '<span class="badge %s">%0.2f</span>' % (badge_class, r.qa.representative.score)
            else:
                row_score = '-'

            # create our table row for this image.
            # Plot is set to None as we have a circular dependency: the row
            # needs the plot, but the plot generator needs the image_stats
            # cache. We will later set plot to the correct value.
            row = ImageRow(
                field=field,
                spw=spw,
                spwnames=spwnames,
                pol=pol,
                frequency_label=row_frequency_label,
                frequency=row_frequency,
                beam=row_beam,
                beam_pa=row_beam_pa,
                sensitivity=row_sensitivity,
                cleaning_threshold=row_cleaning_threshold,
                residual_ratio=row_residual_ratio,
                non_pbcor_label=non_pbcor_label,
                non_pbcor=row_non_pbcor,
                pbcor=row_pbcor,
                score=row_score,
                fractional_bw_label=row_fractional_bw_label,
                fractional_bw=row_fractional_bw,
                aggregate_bw_label=row_bandwidth_label,
                aggregate_bw=row_aggregate_bw,
                image_file=image_name.replace('.pbcor', ''),
                nchan=nchan,
                plot=None,
                qa_url=None,
                iterdone=row_iterdone,
                stopcode=row_stopcode,
                stopreason=row_stopreason,
            )
            image_rows.append(row)

        plotter = display.CleanSummary(context, makeimages_result, image_stats)
        plots = plotter.plot()

        plots_dict = make_plot_dict(plots)

        # construct the renderers so we know what the back/forward links will be
        # sort the rows so the links will be in the same order as the rows
        image_rows.sort(key=lambda row: (row.field, utils.natural_sort(row.spw), row.pol))
        temp_urls = (None, None, None)
        qa_renderers = [TCleanPlotsRenderer(context, results, plots_dict, row.field, str(row.spw), row.pol, temp_urls)
                        for row in image_rows]
        qa_links = triadwise([renderer.path for renderer in qa_renderers])

        final_rows = []
        for row, renderer, qa_urls in zip(image_rows, qa_renderers, qa_links):
            try:
                final_iter = sorted(plots_dict[row.field][str(row.spw)].keys())[-1]
                plot = get_plot(plots_dict, row.field, str(row.spw), final_iter, 'image')

                renderer = TCleanPlotsRenderer(context, results,
                                               plots_dict, row.field, str(row.spw), row.pol,
                                               qa_urls)
                with renderer.get_file() as fileobj:
                    fileobj.write(renderer.render())

                values = dict(row.__dict__)
                values['plot'] = plot
                values['qa_url'] = renderer.path
                new_row = ImageRow(**values)
                final_rows.append(new_row)
            except:
                final_rows.append(row)
        # primary sort images by field, secondary sort on spw, then by pol
        final_rows.sort(key=lambda row: (row.field, utils.natural_sort(row.spw), row.pol))

        ctx.update({
            'plots': plots,
            'plots_dict': plots_dict,
            'image_info': final_rows,
            'dirname': weblog_dir
        })


class TCleanPlotsRenderer(basetemplates.CommonRenderer):
    def __init__(self, context, result, plots_dict, field, spw, pol, urls):
        super(TCleanPlotsRenderer, self).__init__('tcleanplots.mako', context, result)

        outfile = 'field%s-spw%s-pol%s-cleanplots.html' % (field, spw, pol)
        # HTML encoded filenames, so can't have plus sign
        valid_chars = "_.-%s%s" % (string.ascii_letters, string.digits)
        self.path = os.path.join(self.dirname, filenamer.sanitize(outfile, valid_chars))

        # Determine whether any of targets were run with specmode = 'cube',
        # in which case the weblog will need to show the MOM0_FC and
        # MOM8_FC columns.
        show_mom0_8_fc = any([item.specmode == 'cube' for item in result[0].results])
        
        if show_mom0_8_fc:
            colorder = ['pbcorimage', 'residual', 'cleanmask', 'mom0_fc', 'mom8_fc']
        else:
            colorder = ['pbcorimage', 'residual', 'cleanmask']

        self.extra_data = {
            'plots_dict': plots_dict,
            'field': field,
            'spw': spw,
            'colorder': colorder,
            'qa_previous': urls[0],
            'qa_next': urls[2],
            'base_url': os.path.join(self.dirname, 't2-4m_details.html')
        }
    
    def update_mako_context(self, mako_context):
        mako_context.update(self.extra_data)


def get_plot(plots, field, spw, i, colname):
    try:
        return plots[field][spw][i][colname]
    except KeyError:
        return None


def make_plot_dict(plots):
    # Make the plots
    fields = sorted({p.parameters['field'] for p in plots})
    spws = sorted({p.parameters['spw'] for p in plots})
    iterations = sorted({p.parameters['iter'] for p in plots})
    types = sorted({p.parameters['type'] for p in plots})

    iteration_dim = lambda: collections.defaultdict(dict)
    spw_dim = lambda: collections.defaultdict(iteration_dim)
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

    return plots_dict


def triadwise(iterable):
    with_nones = [None] + list(iterable) + [None]
    "s -> (s0,s1,s2), (s1,s2,s3), (s2,s3,s4), ..."
    a, b, c = itertools.tee(with_nones, 3)
    next(b, None)
    next(c, None)
    next(c, None)
    return itertools.izip(a, b, c)
