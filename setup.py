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
			'pipeline.qa2',
			'pipeline.qa2.utility',
			'pipeline.h',
			'pipeline.h.heuristics',
			'pipeline.h.cli',
			'pipeline.h.tasks',
			'pipeline.hif',
			'pipeline.hif.heuristics',
			'pipeline.hif.cli',
			'pipeline.hif.tasks',
			'pipeline.infrastructure',
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
for i in ['recipes'] :
	command = '(cd '+casasrc+'/'+i+'; buildmytasks -i='+casaarch+'/pipeline/'+i+" -o="+i+'.py)'
	print command
	os.system( command)
