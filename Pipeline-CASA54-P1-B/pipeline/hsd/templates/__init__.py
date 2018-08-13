"""
Created on 11 Sep 2014

@author: sjw
"""
import pkg_resources

import pipeline.infrastructure.renderer.weblog as weblog

weblog.register_mako_templates(pkg_resources.resource_filename(__name__, ''), prefix='hsd')
