import os
import sys
import distutils.sysconfig
import platform
import SCons

# try to autodetect numpy
def get_numpy_incdir():
    try:
        # try to find an egg
        from pkg_resources import require
        tmp = require("numpy")
        import numpy
        return numpy.__path__[0]+"/core/include"
    except Exception:
        # now try standard package
        try:
            import numpy
            return numpy.__path__[0]+"/core/include"
        except ImportError:
            pass
    return ""

def get_libdir():
    return os.path.basename(distutils.sysconfig.get_config_var('LIBDIR'))

LIBDIR = 'lib' #get_libdir()

EnsureSConsVersion(1,0,0)

opts = Variables("options.cache")
opts.AddVariables(
		("extraroot", "Addition tree to look for packages", None),
		("extraflags", "Additional build flags", None),
		PathVariable("casacoreroot", "The location of casacore",
                             "/usr/local"),
		BoolVariable("casacorestatic", 
                             "Link statically against casacore",
                             False),
		("boostroot", "The root dir where boost is installed", None),
		("boostlib", "The name of the boost python library", 
		 "boost_python"),
		("boostlibdir", "The boost library location", None),
		("boostincdir", "The boost header file location", None),
		("cfitsioroot", 
		 "The root directory where cfistio is installed", None),
		("cfitsiolibdir", "The cfitsio library location", None),
		("cfitsiolib", "The cfitsio library name", "cfitsio"),
		("cfitsioincdir", "The cfitsio include location", None),
		("wcslib", "The wcs library name", "wcs"),
		("wcsroot", 
		 "The root directory where wcs is installed", None),
		("wcslibdir", "The wcs library location", None),
		("rpfitslib", "The rpfits library name", "rpfits"),
		("rpfitsroot", 
		 "The root directory where rpfits is installed", None),
		("rpfitslibdir", "The rpfits library location", None),

                ("pyraproot", "The root directory where libpyrap is installed",
                 None),
                ("numpyincdir", "numpy header file directory",
                 get_numpy_incdir()),
                BoolVariable("enable_pyrap", 
                             "Use pyrap conversion library from system", 
                             False),
                ("pyraplib", "The name of the pyrap library", "pyrap"),
                ("pyraplibdir", "The directory where libpyrap is installed",
                 None),
                ("pyrapincdir", "The pyrap include location",
                 None),
                EnumVariable("mode", "The type of build.", "release",
                           ["release","debug"], ignorecase=1),
                EnumVariable("makedoc", 
                             "Build the userguide in specified format",
                             "none",
                             ["none", "pdf", "html"], ignorecase=1),
                BoolVariable("apps", "Build cpp apps", True),
                BoolVariable("alma", "Enable alma specific functionality", 
                             False),
                )

env = Environment( toolpath = ['./scons'],
                   tools = ["default", "utils", "casa"],
                   ENV = { 'PATH' : os.environ[ 'PATH' ],
                          'HOME' : os.environ[ 'HOME' ] },
                   options = opts)

env.Help(opts.GenerateHelpText(env))
env.SConsignFile()
if not ( env.GetOption('clean') or env.GetOption('help') ):

    conf = Configure(env)
    if conf.env.get("extraroot", None):
        conf.env.AddCustomPackage('extra')
    conf.env.Append(CPPPATH=[distutils.sysconfig.get_python_inc()])
    if not conf.CheckHeader("Python.h", language='c'):
        Exit(1)
    pylib = 'python'+distutils.sysconfig.get_python_version()
    if env['PLATFORM'] == "darwin":
        conf.env.Append(FRAMEWORKS=["Python"])
    else:
        if not conf.CheckLib(library=pylib, language='c'): Exit(1)

    conf.env.AddCustomPackage('boost')
    libname = conf.env["boostlib"]
    if libname.find(".") > -1 and os.path.exists(libname):
        conf.env.AppendUnique(LIBS=[env.File(libname)])
    else:
        if not conf.CheckLibWithHeader(libname,
                                       'boost/python.hpp', language='c++'): 
            Exit(1)

    if env["enable_pyrap"]:
        conf.env.AddCustomPackage('pyrap')
        if conf.CheckLib(conf.env["pyraplib"], language='c++', autoadd=0):
            conf.env.PrependUnique(LIBS=env['pyraplib'])
        else:
            Exit(1)
    else:
        conf.env.AppendUnique(CPPPATH=[conf.env["numpyincdir"]])
        if conf.CheckHeader("numpy/numpyconfig.h"):
            conf.env.Append(CPPDEFINES=["-DAIPS_USENUMPY"])
        else:
            conf.env.Exit(1)
        # compile in pyrap from here...
        conf.env["pyrapint"] = "#/external/libpyrap/pyrap-0.3.2"
    conf.env.Append(CPPFLAGS=['-DHAVE_LIBPYRAP'])

    if not conf.CheckLib("m"): 
        Exit(1)
    # test for cfitsio
    conf.env.AddCustomPackage('cfitsio')
    libname = conf.env["cfitsiolib"]
    if not conf.CheckHeader("fitsio.h"):
        #SuSE is being special
        conf.env.AppendUnique(CPPPATH=['/usr/include/libcfitsio0'])
        if not conf.CheckHeader("fitsio.h"):
            Exit(1)
    if libname.find(".") > -1 and os.path.exists(libname):
        conf.env.AppendUnique(LIBS=[env.File(libname)])
    else:
        if not conf.CheckLib(libname, language='c'):
            Exit(1)
    conf.env.AddCustomPackage('wcs')
    libname = conf.env["wcslib"]
    if libname.find(".") > -1 and os.path.exists(libname):
        conf.env.AppendUnique(LIBS=[env.File(libname)])
    else:
        if not conf.CheckLibWithHeader(libname, 
                                       'wcslib/wcs.h', language='c'):
            Exit(1)

    conf.env.AddCustomPackage('rpfits')
    if not conf.CheckLibWithHeader(conf.env["rpfitslib"], "RPFITS.h",
                                   language="c"):
	Exit(1)
    
    libpath = ""
    for p in [conf.env["casacoreroot"], conf.env.get("extraroot", "")]:
        pth = os.path.join(p, "include", "casacore")        
	if os.path.exists(pth):
            libpath = os.path.join(p, LIBDIR)
            conf.env.AppendUnique(CPPPATH=[pth])
	    break
    cclibs = ["casa_images", "casa_ms", "casa_components", 
              "casa_coordinates", "casa_lattices", 
              "casa_fits", "casa_measures", "casa_scimath",
              "casa_scimath_f", "casa_tables", "casa_casa"]
    if conf.env["casacorestatic"]:
        libs = [ env.File(os.path.join(libpath, "lib"+lib+".a")) \
                 for lib in cclibs ]
    else:
        conf.env.AppendUnique(LIBPATH=libpath)
        if not conf.CheckLibWithHeader("casa_casa", "casa/aips.h",
                                       language='c++', autoadd=0): 
            Exit(1)
        libs = cclibs
    conf.env.PrependUnique(LIBS=libs)

    if not conf.CheckLib('stdc++', language='c++'): Exit(1)
    if conf.env["alma"]:
        conf.env.Append(CPPFLAGS=['-DUSE_CASAPY'])
    if conf.env.get("extraflags"):
        flags = conf.env.ParseFlags(conf.env["extraflags"])
        conf.env.MergeFlags(flags)
    env = conf.Finish()

opts.Save('options.cache', env)

env["version"] = "4.3.x"

if env['mode'] == 'release':
    if env["PLATFORM"] != "darwin":
	env.Append(LINKFLAGS=['-Wl,-O1', '-s'])
    env.Append(CCFLAGS=["-O2"])
else:
    env.Append(CCFLAGS=["-g", "-W", "-Wall"])

# Export for SConscript files
Export("env")

# build externals
ext = env.SConscript("external-alma/SConscript")

# build library
so = env.SConscript("src/SConscript", variant_dir="build", duplicate=0)

apps = env.SConscript("apps/SConscript")

# test module import, to see if there are unresolved symbols
def test_module(target,source,env):
    pth = str(target[0])
    mod = os.path.splitext(pth)[0]
    sys.path.insert(2, os.path.split(mod)[0])
    __import__(os.path.split(mod)[1])
    print "ok"
    return 0

def test_str(target, source, env):
    return "Testing module..."

taction = Action(test_module, test_str)
env.AddPostAction(so, taction)

if env.GetOption("clean"):
    Execute(Delete(".sconf_temp"))
    Execute(Delete("options.cache"))
    Execute(Delete(".sconsign.dblite"))
if env["makedoc"].lower() != "none":
    env.SConscript("doc/SConscript")
