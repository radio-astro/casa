from distutils.core import setup
setup(name="Pipeline",
		description="Pipeline package",
		packages=['pipeline',
			'pipeline.domain',
			'pipeline.domain.tests',
			'pipeline.extern',
			'pipeline.extern.analysis_scripts',
			'pipeline.extern.logutils',
			'pipeline.hco',
			'pipeline.hco.heuristics',
			'pipeline.hco.cli',
			'pipeline.hco.tasks',
			'pipeline.hsd',
			'pipeline.hsd.heuristics',
			'pipeline.hsd.cli',
			'pipeline.hsd.tasks',
			'pipeline.hsd.tasks.calsky',
			'pipeline.hsd.tasks.common',
			'pipeline.hsd.tasks.exportdata',
			'pipeline.hsd.tasks.importdata',
			'pipeline.hsd.tasks.inspectdata',
			'pipeline.hsd.tasks.reduce',
			'pipeline.qa2',
			'pipeline.qa2.utility',
			'pipeline.h',
			'pipeline.h.heuristics',
			'pipeline.h.cli',
			'pipeline.h.tasks',
			'pipeline.h.tasks.common',
			'pipeline.hif',
			'pipeline.hif.heuristics',
			'pipeline.hif.cli',
			'pipeline.hif.tasks',
			'pipeline.hif.tasks.antpos',
			'pipeline.hif.tasks.applycal',
			'pipeline.hif.tasks.bandpass',
			'pipeline.hif.tasks.clean',
			'pipeline.hif.tasks.cleanlist',
			'pipeline.hif.tasks.common',
			'pipeline.hif.tasks.exportdata',
			'pipeline.hif.tasks.flagging',
			'pipeline.hif.tasks.fluxscale',
			'pipeline.hif.tasks.gaincal',
			'pipeline.hif.tasks.importdata',
			'pipeline.hif.tasks.makecleanlist',
			'pipeline.hif.tasks.refant',
			'pipeline.hif.tasks.setmodel',
			'pipeline.hif.tasks.tsyscal',
			'pipeline.hif.tasks.tsysflag',
			'pipeline.hif.tasks.wvrgcal',
			'pipeline.infrastructure',
			'pipeline.infrastructure.displays',
			'pipeline.infrastructure.docutils',
			'pipeline.infrastructure.renderer',
			'pipeline.infrastructure.renderer.templates',
			'pipeline.recipes'],
		)
import os
casaarch=os.getenv('CASAARCH')
pyversion=os.getenv('PYVERSION')
casasrc=os.getenv('CASASRC')
for i in ['h', 'hif', 'hsd']:
	command = '(cd '+casasrc+'/'+i+'/cli; buildmytasks -i='+casaarch+'/pipeline/'+i+'/cli -o='+i+'.py)'
	print command
	os.system( command)
command = 'cp '+casasrc+'/extern/Mako-0.7.0-py2.6.egg '+casaarch+'/pipeline/extern'
print command
os.system( command)
