import os

import numpy

import display as pbcorimages
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates

LOG = logging.get_logger(__name__)


class T2_4MDetailsMakepbcorimagesRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='pbcor.mako',
                 description='Produce primary beam corrected tt0 images',
                 always_rerender=False):
        super(T2_4MDetailsMakepbcorimagesRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, ctx, context, results):
        weblog_dir = os.path.join(context.report_dir,
                                  'stage%s' % results.stage_number)

        # There is only ever one MakepbcorimagesResults in the ResultsList as it
        # operates over multiple measurement sets, so we can set the result to
        # the first item in the list

        # Get results info
        info_dict = {}

        # Holds a mapping of image name to image stats. This information is used to scale the MOM8 images.
        image_stats = {}

        qaTool = casatools.quanta

        pbcorplots = {}

        for r in results:
            pbcorimagenames = r.pbcorimagenames

            for pbcorimagename in pbcorimagenames:
                image_path = pbcorimagename
                LOG.info('Getting properties of %s for the weblog.' % (image_path))

                with casatools.ImageReader(image_path) as image:
                    info = image.miscinfo()
                    spw = info.get('spw', None)
                    field = ''
                    # if 'field' in info:
                    #     field = '%s (%s)' % (info['field'], r.intent)

                    coordsys = image.coordsys()
                    coord_names = numpy.array(coordsys.names())
                    coord_refs = coordsys.referencevalue(format='s')
                    pol = coord_refs['string'][coord_names == 'Stokes'][0]
                    info_dict[(field, spw, pol, 'image name')] = image.name(strippath=True)

                    stats = image.statistics(robust=False)
                    beam = image.restoringbeam()

            # Make the plots of the pbcor images
            plotter = pbcorimages.PbcorimagesSummary(context, r)
            plots = plotter.plot()
            ms = os.path.basename(r.inputs['vis'])
            pbcorplots[ms] = plots

        ctx.update({'pbcorplots': pbcorplots,
                    'info_dict': info_dict,
                    'dirname': weblog_dir,
                    'plotter': plotter})