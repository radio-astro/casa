import sys
sys.path.insert(0, '.')
from setuptools import setup, find_packages
setup(name="Pipeline",
		description="Pipeline package",
		packages=find_packages()
		)
import os
casaarch=os.getenv('CASAARCH')
pyversion=os.getenv('PYVERSION')
casasrc=os.getenv('CASASRC')
for i in ['h', 'hif', 'hsd', 'hifa', 'hifv']:
	srcdir = casasrc + '/' + i + '/cli'
	destdir = casaarch + '/pipeline/' + i + '/cli'
	if(srcdir == destdir) :
	   command = '(cd '+casasrc+'/'+i+'/cli; buildmytasks -o='+i+'.py)'
	else :
	   command = '(cd '+casasrc+'/'+i+'/cli; buildmytasks -i='+casaarch+'/pipeline/'+i+'/cli -o='+i+'.py)'
	print command
	os.system( command)
if ((casasrc+'/extern') != (casaarch+'/pipeline/extern')) :
   command = 'cp '+casasrc+'/extern/Mako-0.7.0-py2.6.egg '+casaarch+'/pipeline/extern'
   print command
   os.system( command)
