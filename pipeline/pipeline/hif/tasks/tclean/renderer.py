'''
Created on 7 Jan 2015

@author: sjw
'''
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

        qaTool = casatools.quanta

        if results[0]:
            cresults = results[0].results

            for r in cresults:
                if r.empty():
                    continue
                if not r.iterations:
                    continue
                if (r.multiterm):
                    extension = '.tt0'
                else:
                    extension = ''

                maxiter = max(r.iterations.keys())

                with casatools.ImageReader(r.iterations[maxiter]['image']+extension) as image:
                    info = image.miscinfo()
                    spw = pol = field = None
                    if info.has_key('spw'): 
                        spw = info['spw']
                    if info.has_key('field'):
                        field = '%s (%s)' % (info['field'], r.intent)
                    
                    coordsys = image.coordsys()
                    coord_names = numpy.array(coordsys.names())
                    coord_refs = coordsys.referencevalue(format='s')
                    pol = coord_refs['string'][coord_names == 'Stokes'][0]
                    frequency = coord_refs['string'][coord_names == 'Frequency'][0]
                    frequency = qaTool.convert(frequency, 'GHz')
                    info_dict[(field, spw, pol, 'frequency')] = frequency
                    
                    info_dict[(field, spw, pol, 'image name')] = image.name(strippath=True)
                    stats = image.statistics(robust=False)
                    info_dict[(field, spw, pol, 'max')] = stats.get('max')[0]
                    beam = image.restoringbeam()
                    if 'beams' in beam:
                        # 'beams' dict has results for each channel and
                        # each pol product. For now, just use the first beam.
                        beam = beam['beams']['*0']['*0']
                        LOG.warning('%s has per-plane beam shape, displaying '
                                    'only first', 
                                    r.iterations[maxiter]['image'])
                    major = qaTool.convert(beam['major'], 'arcsec')
                    info_dict[(field, spw, pol, 'beam major')] = major
                    minor = qaTool.convert(beam['minor'], 'arcsec')
                    info_dict[(field, spw, pol, 'beam minor')] = minor
                    pa = qaTool.convert(beam['positionangle'], 'deg')
                    info_dict[(field, spw, pol, 'beam pa')] = pa
                    info_dict[(field, spw, pol, 'brightness unit')] = image.brightnessunit()
                    
                    stats = image.statistics(robust=False)
                    info_dict[(field, spw, pol, 'image rms')] = stats.get('rms')[0]
                    
                    summary = image.summary()
                    nchan = summary['shape'][3]
                    width = qaTool.quantity(summary['incr'][3], 
                                        summary['axisunits'][3])
                    width = qaTool.convert(width, 'MHz')
                    width = qaTool.tos(width, 2)
                    
                    info_dict[(field, spw, pol, 'nchan')] = nchan
                    info_dict[(field, spw, pol, 'width')] = width
                    
                with casatools.ImageReader(r.iterations[maxiter]['residual']+extension) as residual:
                    stats = residual.statistics(robust=False)
                    info_dict[(field, spw, pol, 'residual rms')] = stats.get('rms')[0]

                # The RMS value needs to be taken with proper masking.
                # Store the one used for QA scoring.
                info_dict[(field, spw, pol, 'masked rms')] = r.rms

                info_dict[(field, spw, pol, 'score')] = r.qa.representative

        # Make the plots
        plotter = tclean.CleanSummary(context, results[0])
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
                    
        ctx.update({'fields'     : fields,
                    'spws'       : spws,
                    'iterations' : iterations,
                    'plots'      : plots,
                    'plots_dict' : plots_dict,
                    'info_dict'  : info_dict,
                    'dirname'    : weblog_dir})


class CleanPlotsRenderer(basetemplates.CommonRenderer):
    def __init__(self, context, result, plots_dict, field, spw, pol):
        super(CleanPlotsRenderer, self).__init__('cleanplots.mako', context, 
                                                 result)
        
        outfile = 'field%s-spw%s-pol%s-cleanplots.html' % (field, spw, pol)
        self.path = os.path.join(self.dirname, filenamer.sanitize(outfile))
        
        self.extra_data = {'plots_dict' : plots_dict,
                           'field'      : field,
                           'spw'        : spw}
    
    def update_mako_context(self, mako_context):
        mako_context.update(self.extra_data)
