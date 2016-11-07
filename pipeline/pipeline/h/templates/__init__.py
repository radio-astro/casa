import os

import pipeline.infrastructure.renderer.weblog as weblog

weblog.register_mako_templates(os.path.dirname(__file__), prefix='h')
