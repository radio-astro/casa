from __future__ import absolute_import

import pipeline.infrastructure.renderer.qaadapter as qaadapter
import pipeline.infrastructure.renderer.weblog as weblog
from . import qa
from . import resultobjects
from .makeimages import MakeImages
from ..tclean import renderer as clean_renderer

qaadapter.registry.register_to_imaging_topic(resultobjects.MakeImagesResult)


def _get_imaging_mode(context, result):
    try:
        # check imaging_mode in first tclean result of first makeimages result
        # NB: we only check the first entry in the clean list and assume any that
        # follow will also be VLASS
        if 'VLASS' in result[0].results[0].imaging_mode:
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
