# still needed
# tests for MingW32Compiler
# don't know how to test gcc_exists() and msvc_exists()...

import os, sys, tempfile

from numpy.testing import *

from scipy.weave import build_tools

def is_writable(val):
    return os.access(val,os.W_OK)

class TestConfigureBuildDir(TestCase):
    def test_default(self):
        " default behavior is to return current directory "
        d = build_tools.configure_build_dir()
        if is_writable('.'):
            assert(d == os.path.abspath('.'))
        assert(is_writable(d))
    def test_curdir(self):
        " make sure it handles relative values. "
        d = build_tools.configure_build_dir('.')
        if is_writable('.'):
            assert(d == os.path.abspath('.'))
        assert(is_writable(d))
    def test_pardir(self):
        " make sure it handles relative values "
        d = build_tools.configure_build_dir('..')
        if is_writable('..'):
            assert(d == os.path.abspath('..'))
        assert(is_writable(d))
    def test_bad_path(self):
        " bad path should return same as default (and warn) "
        d = build_tools.configure_build_dir('_bad_path_')
        d2 = build_tools.configure_build_dir()
        assert(d == d2)
        assert(is_writable(d))

class TestConfigureTempDir(TestConfigureBuildDir):
    def test_default(self):
        " default behavior returns tempdir"
        # this'll fail if the temp directory isn't writable.
        d = build_tools.configure_temp_dir()
        assert(d == tempfile.gettempdir())
        assert(is_writable(d))

class TestConfigureSysArgv(TestCase):
    def test_simple(self):
        build_dir = 'build_dir'
        temp_dir = 'temp_dir'
        compiler = 'compiler'
        pre_argv = sys.argv[:]
        build_tools.configure_sys_argv(compiler,temp_dir,build_dir)
        argv = sys.argv[:]
        bd = argv[argv.index('--build-lib')+1]
        assert(bd == build_dir)
        td = argv[argv.index('--build-temp')+1]
        assert(td == temp_dir)
        argv.index('--compiler='+compiler)
        build_tools.restore_sys_argv()
        assert(pre_argv == sys.argv[:])

if __name__ == "__main__":
    nose.run(argv=['', __file__])
