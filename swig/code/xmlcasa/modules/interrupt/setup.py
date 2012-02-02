from distutils.core import setup, Extension

setup( name="interrupt", version="1.0",
       ext_modules=[Extension("interrupt",
			      [ "interrupt_python.cc" ],
			      library_dirs=['/opt/local/lib'],
			      libraries = [ ])],
       include_dirs = [ ".", "/opt/local/include" ]
)
