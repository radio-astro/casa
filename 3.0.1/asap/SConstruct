import os
import sys
import distutils.sysconfig
import platform
import SCons

moduledir = distutils.sysconfig.get_python_lib()
if  platform.architecture()[0] == '64bit':
    # hack to install into /usr/lib64 if scons is in the 32bit /usr/lib/
    if moduledir.startswith("/usr/lib/"):
        moduledir = moduledir.replace("lib", "lib64")

opts = Options("options.cfg")
opts.AddOptions(
                ("FORTRAN", "The fortran compiler", None),
                ("f2clib", "The fortran to c library", None),
                PathOption("prefix",
	        "The root installation path",
                           distutils.sysconfig.PREFIX),
                PathOption("moduledir",
                            "The python module path (site-packages))",
                            moduledir),
		PathOption("casacoreroot", "The location of casacore",
				    "/usr/local"),
		("boostroot", "The root dir where boost is installed", None),
		("boostlib", "The name of the boost python library", 
		 "boost_python"),
		("boostlibdir", "The boost library location", None),
		("boostincdir", "The boost header file location", None),
		("lapackroot", 
		 "The root directory where lapack is installed", None),
		("lapacklibdir", "The lapack library location", None),
		("lapacklib",
		 "The lapack library name (e.g. for specialized AMD libraries",
		 "lapack"),
		("blasroot", 
		 "The root directory where blas is installed", None),
		("blaslibdir", "The blas library location", None),
		("blaslib",
		 "The blas library name (e.g. for specialized AMD libraries",
		 "blas"),
		("cfitsioroot", 
		 "The root directory where cfistio is installed", None),
		("cfitsiolibdir", "The cfitsio library location", None),
		("cfitsiolib", "The cfitsio library name", "cfitsio"),
		("cfitsioincdir", "The cfitsio include location", None),
		("wcsroot", 
		 "The root directory where wcs is installed", None),
		("wcslibdir", "The wcs library location", None),
		("rpfitslib", "The rpfits library name", "rpfits"),
		("rpfitsroot", 
		 "The root directory where rpfits is installed", None),
		("rpfitslibdir", "The rpfits library location", None),
#		("rpfitsincdir", "The rpfits include location", None),
                EnumOption("mode", "The type of build.", "debug",
                           ["release","debug"], ignorecase=1),
                ("makedist",
                 "Make a binary archive giving a suffix, e.g. sarge or fc5",
                 ""),
                EnumOption("makedoc", "Build the userguide in specified format",
                           "none",
                           ["none", "pdf", "html"], ignorecase=1)
                )

env = Environment( toolpath = ['./scons'],
                   tools = ["default", "archiver", "utils",
                            "quietinstall"],
                   ENV = { 'PATH' : os.environ[ 'PATH' ],
                          'HOME' : os.environ[ 'HOME' ] },
                   options = opts)

Help(opts.GenerateHelpText(env))
env.SConsignFile()

if env["PLATFORM"] == "darwin":
    env.EnsureSConsVersion(0,96,95)

casacoretooldir = os.path.join(env["casacoreroot"],"share",
				   "casacore")
if not os.path.exists(casacoretooldir):
    print "Could not find casacore scons tools"
    Exit(1)

# load casacore specific build flags
env.Tool('casa', [casacoretooldir])

if not env.GetOption('clean'):
    conf = Configure(env)

    conf.env.AppendUnique(LIBPATH=os.path.join(conf.env["casacoreroot"], 
					       "lib"))
    conf.env.AppendUnique(CPPPATH=os.path.join(conf.env["casacoreroot"], 
					       "include", "casacore"))
    if not conf.CheckLib("casa_casa", language='c++'): Exit(1)
    conf.env.PrependUnique(LIBS=["casa_ms", "casa_components", 
                                 "casa_coordinates", "casa_lattices", 
                                 "casa_fits", "casa_measures", "casa_scimath",
                                 "casa_scimath_f", "casa_tables", 
                                 "casa_mirlib"])
    conf.env.Append(CPPPATH=[distutils.sysconfig.get_python_inc()])
    if not conf.CheckHeader("Python.h", language='c'):
        Exit(1)
    pylib = 'python'+distutils.sysconfig.get_python_version()
    if env['PLATFORM'] == "darwin":
        conf.env.Append(FRAMEWORKS=["Python"])
    else:
        if not conf.CheckLib(library=pylib, language='c'): Exit(1)

    conf.env.AddCustomPackage('boost')
    if not conf.CheckLibWithHeader(env["boostlib"], 
                                   'boost/python.hpp', language='c++'): 
        Exit(1)
    # test for cfitsio
    if not conf.CheckLib("m"): Exit(1)
    conf.env.AddCustomPackage('cfitsio')
    if not conf.CheckLibWithHeader(conf.env["cfitsiolib"], 
				   'fitsio.h', language='c'):
        Exit(1)
    conf.env.AddCustomPackage('wcs')
    if not conf.CheckLibWithHeader('wcs', 'wcslib/wcs.h', language='c'):
        Exit(1)
    conf.env.AddCustomPackage('rpfits')
    if not conf.CheckLib(conf.env["rpfitslib"], language="c"):
	Exit(1)

    # test for blas/lapack
    conf.env.AddCustomPackage("lapack")
    if not conf.CheckLib(conf.env["lapacklib"]): Exit(1)
    blasname = conf.env.get("blaslib", "blas")
    conf.env.AddCustomPackage("blas")
    if not conf.CheckLib(conf.env["blaslib"]): Exit(1)
    conf.env.CheckFortran(conf)
    if not conf.CheckLib('stdc++', language='c++'): Exit(1)
    env = conf.Finish()

env["version"] = "2.2.x"

if env['mode'] == 'release':
    if env["PLATFORM"] != "darwin":
	env.Append(LINKFLAGS=['-Wl,-O1', '-s'])
    env.Append(CCFLAGS=["-O2"])
else:
    env.Append(CCFLAGS=["-g"])

# Export for SConscript files
Export("env")

# build externals
env.SConscript("external/SConscript")
# build library
so = env.SConscript("src/SConscript", build_dir="build", duplicate=0)
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

# install targets
somod = env.Install("$moduledir/asap", so )
pymods = env.Install("$moduledir/asap", env.SGlob("python/*.py"))
bins = env.Install("$prefix/bin", ["bin/asap", "bin/asap_update_data"])
shares = env.Install("$moduledir/asap/data", "share/ipythonrc-asap")
env.Alias('install', [somod, pymods, bins, shares])

# install aips++ data repos
rootdir=None
outdir =  os.path.join(env["moduledir"],'asap','data')
sources = ['ephemerides','geodetic']
if os.path.exists("/nfs/aips++/data"):
    rootdir = "/nfs/aips++/data"
elif os.path.exists("data"):
    rootdir = "./data"
if rootdir is not None:
    ofiles, ifiles = env.WalkDirTree(outdir, rootdir, sources)
    data =  env.InstallAs(ofiles, ifiles)
    env.Alias('install', data)

# make binary distribution
if len(env["makedist"]):
    env["stagedir"] = "asap-%s-%s" % (env["version"], env["makedist"])
    env.Command('Staging distribution for archive in %s' % env["stagedir"],
                '', env.MessageAction)
    st0 = env.QInstall("$stagedir/asap", [so,  env.SGlob("python/*.py")] )
    env.QInstall("$stagedir/bin", ["bin/asap", "bin/asap_update_data"])
    env.QInstall("$stagedir", ["bin/install"])
    env.QInstall("$stagedir/asap/data", "share/ipythonrc-asap")
    if rootdir is not None:
        # This creates a directory Using data table... - disabled
        #env.Command("Using data tables in %s" % rootdir,
        #           '', env.MessageAction)
        outdir =  os.path.join(env["stagedir"],'asap','data')
        ofiles, ifiles = env.WalkDirTree(outdir, rootdir, sources)
        env.QInstallAs(ofiles, ifiles)
    else:
        env.Command("No data tables available. Use 'asap_update_data' after install",
                    '', env.MessageAction)
    arch = env.Archiver(os.path.join("dist",env["stagedir"]),
                        env["stagedir"])
    env.AddPostAction(arch, Delete("$stagedir"))
if env["makedoc"].lower() != "none":
    env.SConscript("doc/SConscript")

if env.GetOption("clean"):
    Execute(Delete(".sconf_temp"))
