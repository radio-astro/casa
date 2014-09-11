'''
Created on 8 Sep 2014

@author: sjw
'''
import contextlib
import os

import pipeline.infrastructure.logging as logging
from . import weblog

LOG = logging.get_logger(__name__)


class T2_4MDetailsDefaultRenderer(object):
    def __init__(self, uri='t2-4m_details-generic.html',
                 description='No description set for this task',
                 always_rerender=False):
        self.uri = uri
        # TODO remove template access after conversion
        self.description = description
        self.always_rerender = always_rerender

    def get_display_context(self, context, result):
        mako_context = {'pcontext' : context,
                        'result'   : result,
                        'stagelog' : self._get_stagelog(context, result),
                        'dirname'  : 'stage%s' % result.stage_number}
        self.update_mako_context(mako_context, context, result)
        return mako_context

    def update_mako_context(self, mako_context, pipeline_context, result):
        LOG.trace('No-op update_mako_context for %s', self.__class__.__name__)
        
    def render(self, context, result):
        display_context = self.get_display_context(context, result)
        uri = self.uri
        template = weblog.TEMPLATE_LOOKUP.get_template(uri)
        return template.render(**display_context)

    def _get_stagelog(self, context, result):
        """
        Read in the CASA log extracts from the file in the stage directory.
        """
        stagelog_path = os.path.join(context.report_dir,
                                     'stage%s' % result.stage_number,
                                     'casapy.log')

        if not os.path.exists(stagelog_path):
            return None
        
        with open(stagelog_path, 'r') as f:
            return ''.join([l.expandtabs() for l in f.readlines()])


class CommonRenderer(object):
    def __init__(self, uri, context, result):
        self.uri = uri
        self.context = context
        self.result = result
        # this should be overwritten by the superclass constructor, otherwise
        # an exception will occur when the template is rendered
        self.path = None
        
        stage = 'stage%s' % result.stage_number
        self.dirname = os.path.join(context.report_dir, stage)

    def render(self):
        mako_context = {'pcontext' : self.context,
                        'result'   : self.result,
                        'dirname'  : self.dirname}
        self.update_mako_context(mako_context)
        t = weblog.TEMPLATE_LOOKUP.get_template(self.uri)
        return t.render(**mako_context)

    def update_mako_context(self, mako_context):
        pass

    def get_file(self):
        if self.path is None:
            LOG.error('No path specified for renderer')
            raise IOError()
        
        if not os.path.exists(self.dirname):
            os.makedirs(self.dirname)
            
        file_obj = open(self.path, 'w')
        return contextlib.closing(file_obj)
