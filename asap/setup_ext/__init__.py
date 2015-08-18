import os, sys, platform
import subprocess
import glob
import shutil
from distutils.command import build_ext

try:
    from setuptools import setup as _setup
    from setuptools import Extension
except ImportError, ex:
    from distutils.core import setup as _setup
    from distutils.core import Extension

def setup(*args, **kwargs):
    asapso = Extension(name="%s._%s" % (kwargs['name'],kwargs['name']),
                       sources=[])
    d = {'ext_modules': [ asapso ],
         'cmdclass': {'build_ext': scons_ext}
         }
    kwargs.update(d)
    _setup(*args, **kwargs)
    

class scons_ext(build_ext.build_ext):
    """Build extensions using scons instead of distutils.
    """
    _scons_options = []
    user_options = \
            [('casacoreroot=', None, 
              'Prefix for casacore installation location'),
             ('casacorestatic=', None,
              'use static casacore libraries'),
	     ('boostroot=', None, 
              'Prefix for boost_python installation location'),
	     ('boostlib=', None, 'Name of the boost_python library'),
	     ('cfitsioroot=', None, 
              'Prefix for cfitsio installation location'),
	     ('cfitsiolib=', None, 'Name of the cfitsio library'),
	     ('cfitsioincdir=', None, 'The custom cfitsio include dir'),
	     ('wcsroot=', None, 'Prefix for wcslib installation location'),
	     ('wcslib=', None, 'Name of the wcs library'),
	     ('rpfitsroot=', None, 'Prefix for rpfits installation location'),
	     ('rpfitslib=', None, 'Name of the rpfits library'),
	     ('blaslib=', None, 'Name of the blas library'),
	     ('lapacklib=', None, 'Name of the lapack library'),
	     ('f2clib=', None, 'Name of the fortran-to-c library'),
	     ('jobs=','j', 'Number of processes'),
	     ('extraflags=', None, 
              'Extra build flags e.g. static libs, defines etc.'),
             ('extraroot=', None, 
              'Extra root directory where muiltple packages could be found,'
              ' e.g. $HOME, to add $HOME/lib etc to the build.'),
	     ]


    def initialize_options(self):
        """
	Overload to enable custom settings to be picked up
	"""
        build_ext.build_ext.initialize_options(self)
        self._scons_options = []
        # attribute corresponding to directory prefix
        # command line option
        self.jobs = None
        self.extraroot = None
        self.extraflags = None
	self.casacoreroot = None
	self.casacorestatic = None
	self.boostroot = None
	self.boostlib = None
        self.cfitsioroot = None
        self.cfitsiolib = None
        self.cfitsioincdir = None
	self.wcsroot = None
	self.wcslib = None
	self.rpfitsroot = None
	self.rpfitslib = None
	self.blaslib = None
	self.lapacklib = None
	self.f2clib = None

    def finalize_options(self):
        build_ext.build_ext.finalize_options(self)
        for opt in self.user_options:
            attr = opt[0].strip("=")
            v = getattr(self, attr)
            if v is not None:
                if opt[1] is None:
                    self._scons_options.append("=".join([attr, v]))
                else:
                    self._scons_options.append(" ".join(["-"+opt[1], v]))

    def build_extensions(self):
        ext = self.extensions[0]        
        try:
            ext_path = self.get_ext_fullpath(ext.name)
            extdir = os.path.dirname(ext_path)
        except AttributeError:
            # hack for pyhton 2.5
            extdir = os.path.join(self.build_lib, ext.name.split(".")[0])
        if not os.path.exists(extdir):
            os.makedirs(extdir)
        cmd = ['scons'] + self._scons_options
        retcode = subprocess.call(cmd)
        if retcode != 0:
            raise RuntimeError('scons failed')
        # copy extension into distutils build directory
        if os.path.exists("build/_asap.so"):
            shutil.copy("build/_asap.so", extdir)
