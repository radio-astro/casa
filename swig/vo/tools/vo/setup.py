
"""
setup.py file for SWIG vo
"""

from distutils.core import setup, Extension

vo_module = Extension("_vo", sources=["vo_wrap.cxx", "vo_cmpt.cc"],
                       include_dirs=["/opt/casa/active/darwinllvm/../darwinllvm/include/casacore",
                                     "/opt/casa/active/darwinllvm/../darwinllvm/include/casacode",
                                     "/opt/casa/active/darwinllvm/../bindings",
                                     "../..",
                                     ".",
                                     "/opt/casa/darwin11/Library/Frameworks/Python.framework/Headers",
                                     ".", "/iraf/iraf/vendor/voclient/include"],
                       library_dirs=["/iraf/iraf/vendor/voclient/lib", "/opt/casa/active/darwinllvm/../darwinllvm/lib"],
                       libraries=["VOCLient", "tools"]
                                  )

setup (name = "vo",
       version = "0.1",
       author      = "Wes Young",
       description = """Simple swig vo example""",
       ext_modules = [vo_module],
       py_modules = ["vo"],
       )
