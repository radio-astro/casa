import os
import glob

def generate(env):
    def SGlob(pattern):
        path = env.GetBuildPath('SConscript').replace('SConscript', '')
        return [ i.replace(path, '') for i in glob.glob(path + pattern) ]
    env.SGlob = SGlob

    def AddCustomPath(path=""):
        if not len(path) or not os.path.exists(path):
            return
        env.PrependUnique(CPPPATH = [os.path.join(path, "include")])
        env.PrependUnique(LIBPATH = [os.path.join(path, "lib")])
    env.AddCustomPath = AddCustomPath

    def AddCustomPackage(pkgname=None):
        if pkgname is None:
            return
        pkgroot = env.get("%sroot" % pkgname, None)
        pkgincd = env.get("%sincdir" % pkgname, None)
        pkglibd = env.get("%slibdir" % pkgname, None)
        incd = None
        libd = None
        if pkgroot is not None:
            incd = os.path.join(pkgroot, "include")
            libd = os.path.join(pkgroot, "lib")
        else:       
            if pkgincd is not None:
                incd = pkgincd
            if pkglibd is not None:
                libd = pkglibd
        if incd is not None:
            if not os.path.exists(incd):
                print "Custom %s include dir '%s' not found" % (pkgname, incd)
                env.Exit(1)
            env.PrependUnique(CPPPATH = [incd])
        if libd is not None:
            if not os.path.exists(libd):
                print "Custom %s lib dir '%s' not found" % (pkgname, libd)
                env.Exit(1)
            env.PrependUnique(LIBPATH = [libd])

    env.AddCustomPackage = AddCustomPackage

    def CheckFortran(conf):
            
        if not conf.env.has_key("FORTRAN"):
            # auto-detect fortran
            detect_fortran = conf.env.Detect(['gfortran', 'g77', 'f77'])
            if not detect_fortran:
                print "No fortran compiler found. Specify FORTRAN and f2clib."
                conf.env.Exit(1)
            conf.env["FORTRAN"] = detect_fortran
            fdict = {'gfortran': 'gfortran', 'g77': 'g2c', 'f77': 'f2c'}
            f2clib = conf.env.get("f2clib", fdict[detect_fortran])
            if not conf.CheckLib(f2clib):
                env.Exit(1)
        else:
            if not conf.env.has_key("f2clib"):
                print "A custom fortran compiler also needs f2clib defined"
                env.Exit(1)
            else:
                if not conf.CheckLib(env["f2clib"]):
                    env.Exit(1)
        if conf.env["FORTRAN"].startswith("g77"):
            fflags = ["-Wno-globals", "-fno-second-underscore"]
            conf.env.Append(SHFORTRANFLAGS=fflags)
            conf.env.Append(FORTRANFLAGS=fflags)
    env.CheckFortran = CheckFortran

    def WalkDirTree(targetroot, sourceroot, sources):
        ifiles = []
        ofiles = []
        for s in sources:
            if os.path.isdir(os.path.join(sourceroot ,s)):
                for d,ld,f in os.walk(os.path.join(sourceroot ,s)):
                    for fl in f:
                        ifile = os.path.join(d, fl)
                        ifiles.append(ifile)
                        ofile = ifile.replace(sourceroot, targetroot)
                        ofiles.append(ofile)
        return ofiles, ifiles
    env.WalkDirTree = WalkDirTree

    def null_action(target, source, env): return 0

    def message(target, source, env):
        return "%s" % target[0]
    env.MessageAction = env.Action(null_action, message)

def exists(env):
    try:
        import os
        import glob
    except ImportError:
        return False
    else:
        return True
