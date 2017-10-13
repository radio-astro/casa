from __future__ import absolute_import
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog

from .makeimages import MakeImages
from . import resultobjects
from ..tclean import renderer as clean_renderer

from . import qa

pipelineqa.registry.add_handler(qa.MakeImagesQAHandler())
pipelineqa.registry.add_handler(qa.MakeImagesListQAHandler())
qaadapter.registry.register_to_imaging_topic(resultobjects.MakeImagesResult)


def _get_imaging_mode(context):
    # NB: we only check the first entry in the clean list and assume any that
    # follow will also be VLASS
    try:
        if 'VLASS' in context.clean_list_pending[0]['heuristics'].imaging_mode:
            # this key should match the key against which the renderer is
            # registered
            return 'VLASS'
    except IndexError:
        pass
    except KeyError:
        pass

    return None


# no key/key_fn is specified, so this will be used for all non-VLASS data
weblog.add_renderer(MakeImages,
                    clean_renderer.T2_4MDetailsTcleanRenderer(description='Calculate clean products'),
                    group_by=weblog.UNGROUPED)


# if this is VLASS data, use a different weblog template
weblog.add_renderer(MakeImages,
                    clean_renderer.T2_4MDetailsTcleanRenderer(description='Calculate clean products',
                                                              uri='vlass_tclean.mako'),
                    group_by=weblog.UNGROUPED,
                    key='VLASS',
                    key_fn=_get_imaging_mode)
