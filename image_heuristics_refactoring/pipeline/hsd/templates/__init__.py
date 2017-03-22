'''
Created on 11 Sep 2014

@author: sjw
'''
import os

import pipeline.infrastructure.renderer.weblog as weblog

weblog.register_mako_templates(os.path.dirname(__file__), prefix='hsd')
