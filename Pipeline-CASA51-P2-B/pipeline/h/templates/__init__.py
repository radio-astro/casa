import pkg_resources

import pipeline.infrastructure.renderer.weblog as weblog

weblog.register_mako_templates(pkg_resources.resource_filename(__name__, ''), prefix='h')
