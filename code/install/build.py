#!/usr/bin/python
#
# Usage: build.py URL REVISION TYPE [dry]
#
# This script builds CASA and creates documentation of the build procedure.
#
# The documentation is written to $HOME/documentation/
#
# Possible values for TYPE are full/incremental/test.
#
# Examples:
#    Build from scratch and generate documentation:
#
#       $ build.py https://svn.cv.nrao.edu/svn/casa/active 10809 full
#
#    Dry run, just generate the documentation:
#
#       $ build.py https://svn.cv.nrao.edu/svn/casa/active 10809 full dry
#
#
# Supported platforms: RH5.3 32bit, RH5.3 64bit, OSX 10.5, 10.6
#

import subprocess
import time
import commands
import sys
import os
import string

# string that depends on OS
class platform:
    def __init__(self, n, val):
        self.value = val
        self.name = n

# string that depends on concrete machine
class machine:
    def __init__(self, n, val):
        self.value = val
        self.name = n

# Base class for build methods (documentation, execute)
class builder:

    def __init__(self):
        self.temp = False
        
    def chdir(self, *texts):
        self.do("", "cd ", *texts)

    def set_env(self, env, *texts):
        self.do("", "export ", env, '="', self._transform(texts), '"')

    def svn_exe(self, repository, revision, local_dir):
        if self.type == "full":
            if not self.temp:
                self.comment("svn might prompt you to accept a fingerprint; in that case answer t(emporarily). Or you can send a 't' to the standard input of svn, in order to do so in a script")
                self.temp = True
            self.do("", "echo t | svn checkout -r %s %s %s" % (revision, repository, local_dir))
        else:
            self.do("", "echo t | svn update -r %s %s" % (revision, local_dir))


# Write sh script
class sh_builder(builder):
    def __init__(self, file, type):
        builder.__init__(self);
        self.type = type
        self.file = open(file, 'w')
        # Do not write to the script immediately. 
        # First accumulate all commands and variable definitions,
        # then finally (in __del__) write the variable definitons
        # before the commands.
        self.commands = []
        self.definitions = {}
        self.file.write("#!/bin/bash\n")
        self.file.write("#\n")
        self.file.write("# This Bourne Shell script does a " + type + " build of CASA. It was generated from the command\n")
        self.file.write("#\n")
        self.file.write("#         " + string.join(sys.argv) + "\n")
        self.file.write("#\n")
        self.file.write("\n")

    def __del__(self):
        
        # Time to flush
        
        # First define variables
        for (n,v) in self.definitions.items():
            self.file.write('%s=%s\n' % (n, v))
        self.file.write('\n')

        # Then commands
        if len(self.commands) > 0:
            for c in self.commands[:-1]:
                self.file.write(c)
            self.file.write('exec ' + self.commands[-1])

        self.file.close()

    def _transform(self, texts):
        result = ""
        for t in texts:
            if type(t) is str:
                s = t
            elif t.__class__ == platform or \
                 t.__class__ == machine:

                 if not t.name in self.definitions.keys():
                     self.definitions[t.name] = t.value
                 s = "$" + t.name

            else:
                raise Exception("Illegal type in " + str(texts) + ": " + str(type(t)) + str(t))
            result += s

        return result

    def do(self, comment, *texts):
        self.comment(comment)
        cmd = self._transform(texts)
        self.commands.append(cmd + '\n\n')

    def comment(self, *comment):
        c = self._transform(comment)
        if c != "":
            self.commands.append('# ' + c + '\n')


# Write HTML documentation
class html_builder(builder):
    def __init__(self, file, oss, arch, type):
        builder.__init__(self);
        self.type = type
        self.html = open(file, 'w')
        self.html.write('<html><head></head><body>')
        self.html.write('These are instructions for doing a ' + type + ' build of CASA on ' + oss + ', ' + arch + '. ')
        self.html.write('Text written in ' + self._transform([platform('', 'blue')]) + ' is platform dependent, ')
        self.html.write('and you can leave that unchanged if you are really on ' + oss + ', ' + arch + '. ')
        self.html.write('Text written in ' + self._transform([machine('', 'red')]) + ' depend on the specific machine ')
        self.html.write('used (such as directory paths), and you must substitute this as necessary according to your local setup.')
        self.outfile = None

    def __del__(self):
        self.html.write('</body></html>')
        self.html.close()

    # Output of the next command
    def set_outfile(self, outfile):
        self.outfile = outfile

    def _transform(self, texts):
        result = ""
        for t in texts:
            if type(t) is str:
                color = '000000'
                s = t
            elif t.__class__ == platform:
                color = '0000ff'
                s = t.value
            elif t.__class__ == machine:
                color = 'ff0000'
                s = t.value
            else:
                raise Exception("Illegal type" + str(type(t)) + str(t))
            result += '<font color="%s">%s</font>' % (color, s)

        return result

    def do(self, comment, *texts):
        self.comment(comment)
        cmd = self._transform(texts)
        self.html.write("<p><table border=0 cellpadding=10><tr><td><pre>" + os.getcwd() + "> " + cmd + "</pre></td>")
        if self.outfile:
            self.html.write("<td><a href=\"" + self.outfile + "\">output</a></td>")
        self.html.write('</tr></table><p>\n')
        self.html.flush()

    def comment(self, *comment):
        self.html.write('<p>' + self._transform(comment))


# Execute build steps
# echo each command
class exe_builder(builder):

    def __init__(self, dry, type):
        builder.__init__(self);
        self.type = type
        self.dry = dry         # dry run (true/false)
        self.outfile = None
    
    # Output of next command goes here
    def set_outfile(self, outfile):
        self.outfile = outfile

    def _transform(self, texts):
        result = ""
        for t in texts:
            if type(t) is str:
                s = t
            elif t.__class__ == platform or \
                 t.__class__ == machine:

                 s = t.value

            else:
                raise Exception("Illegal type in " + str(texts) + ": " + str(type(t)) + str(t))
            result += s

        return result

    def do(self, comment, *texts):
        cmd = self._transform(texts)
        if not self.dry:
            print cmd
        sys.stdout.flush()
        if not self.dry:

            if self.outfile:
                # Echo the command to the output file,
                # then the stdout+stderr of that command to the file

                f = open(self.outfile, "w")
                f.write(os.getcwd() + "> " + cmd + "\n")
                f.close()
                p1 = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=True)
                p2 = subprocess.Popen(["tee", "-a", self.outfile], stdin=p1.stdout)
            else:
                p1 = subprocess.Popen(cmd, shell=True)
            status = os.waitpid(p1.pid, 0)[1]
            if self.outfile:
                os.waitpid(p2.pid, 0)
            if status != 0:
                raise Exception("Error: " + str(cmd))
        else:
            f = open(self.outfile, "w")
            f.write('[Dry run] - Output of ' + cmd + '\n')
            f.close()
        
    def comment(self, *comment):
        pass

    # os.system("cd <path>") won't work...
    def chdir(self, *texts):
        # Substitute any environment variables by calling the shell
        dir = commands.getoutput("echo " + self._transform(texts))
        if not self.dry:
            print "cd", dir
        sys.stdout.flush()
        if not self.dry:
            os.chdir(dir)

    def set_env(self, env, *texts):
        val = commands.getoutput("echo " + self._transform(texts))
        if not self.dry:
            print env, "=", val
        sys.stdout.flush()
        if not self.dry:
            os.environ[env] = val

# This class executes and creates HTML
# for the given commands
# The output of the execution is inserted
# in the HTML
class exe_and_doc_builder(builder):
    def __init__(self, dir, oss, arch, type, dry):
        builder.__init__(self);
        self.type = type
        self.dir = dir
        if not os.path.isdir(dir):
            os.system("mkdir -p " + dir)
        else:
            os.system("rm -rf " + dir + "/*")
        
        self.html = html_builder(dir + '/build.html', oss, arch, type)
        self.sh   =   sh_builder(dir + '/build.sh', type)
        self.exe  = exe_builder(dry = dry, type = type)
        self.outfile = 0

    def do(self, comment, *texts):
        out = self.dir + "/" + str(self.outfile) + ".txt"
        self.html.set_outfile(str(self.outfile) + ".txt")
        self.exe.set_outfile(out)

        self.html.do(comment, *texts)
        self.exe.do(comment, *texts)

        self.html.set_outfile(None)
        self.exe.set_outfile(None)
        self.outfile += 1

        self.sh.do(comment, *texts)

    def comment(self, comment, *texts):
        self.html.comment(comment, *texts)
        self.exe.comment(comment, *texts)
        self.sh.comment(comment, *texts)
    
    def chdir(self, *texts):
        self.html.chdir(*texts)
        self.exe.chdir(*texts)
        self.sh.chdir(*texts)

    def set_env(self, env, *texts):
        self.html.set_env(env, *texts)
        self.exe.set_env(env, *texts)
        self.sh.set_env(env, *texts)


def build_casa(b, url, revision, type, ops, architecture):

    builddirs = {'Linux' : {'i386' : 'linux_gnu', 'x86_64' : 'linux_64b'}, 'Darwin' : {'10.5' : 'darwin', '10.6': 'darwin'}}
    if not ops in builddirs.keys() or \
            not architecture in builddirs[ops].keys():
        raise Exception, ("Unknown OS and architecture " + str(ops) + ", " + str(architecture) + ", must be one of " + str(builddirs))

    # Setup platform- and machine dependent variables
    oss = platform('oss', ops)
    arch = platform('arch', architecture)
    builddir = platform('builddir', builddirs[ops][architecture])

    if ops == "Darwin":
        datadir = machine('datadir', "/opt/casa_data/trunk")
        prefix = machine('prefix', "/opt/casa/active")    # Prefix is hardcoded in 3rd-party executables
        if architecture == '10.5':
            coredir = platform('coredir', "core2-apple-darwin8")
            third = platform('third', "core2-apple-darwin8/3rd-party")
        else:
            coredir = platform('coredir', "core2-apple-darwin10")
            third = platform('third', "core2-apple-darwin10")
    else:
        datadir = machine('datadir', "/mnt/hgfs/trunk")
        prefix = machine('prefix', "/opt/casa")

    if ops == "Linux":
        threads = machine("threads", "2")
    else:
        threads = machine("threads", "4")


    b.comment("The following documentation was verified as of ", url, ", revision ", revision, " on " + time.asctime() + ". Here we go...")

    #
    # Install external packages
    #

    if type == "full":
        b.comment("Buidling CASA from source consists of two major steps: 1) Build and install CASACore, 2) Build \"code\" which uses the libraries from CASACore")

        b.comment("You will need to install quite a number of packages which are required for CASA development.")
        if ops == "Darwin":

           b.do("Create the directory, where you want to install CASA. On Mac, you should use a subdirectory of /opt/casa/. You do not need to (and should not) install CASA as root, just make the install directory writeable for non-root users.",
                "ls -l ", prefix)
           b.chdir(prefix, "/..")
           if architecture == '10.5':
               third_party = "3rd-party.tbz"
               d = "https://svn.cv.nrao.edu/casa/osx_distro/developers/10.5/"
           else:
               third_party = "core2.10.6.tar.bz2"
               d = "https://svn.cv.nrao.edu/casa/osx_distro/developers/10.6/"
               
           if False:
               b.do("", "curl " + d + third_party + " -o " + third_party)
           else:
               b.do("Now you should download " + d + third_party + " to the local directory. This could take a very long time, therefore I cheat and link to a local copy of the 3rd-party package", "ln -s /opt/casa_build/" + third_party + " .")


           if architecture == '10.5':
               qt = "qt-sdk-mac-opensource-2009.02.dmg"
               qwt = "qwt5.2.tbz"
               b.do("Then get the Qt SDK from " + d + " and put it in the current directory", "ln -s /opt/casa_build/" + qt + " .")

               b.do("Get qwt", "curl %s/%s -o %s" % (d, qwt, qwt))

               b.do("You may want to double check that you have the right packages,", "cksum %s %s %s" % (third_party, qt, qwt))

           else:
               gfortran = "gfortran-42-5646.pkg"
               b.do("Get the Fortran compiler,", "curl %s%s -o %s" % (d, gfortran, gfortran))

               b.do("You may want to double check that you have the right packages,", "cksum %s %s" % (third_party, gfortran))


           b.comment("The paths in the binary contents of the 3rd party package requires that it is extracted in this directory,")
           
           if architecture == '10.5':
               b.do("", "mkdir ", coredir)
               b.chdir(coredir)
               b.do("", "tar jxvf ../" + third_party)
               
               b.do("Workaround for CAS-2071, do not use the Qt in 3rd-party:",
                    "rm -rf ", prefix, "/../", third, "/include/Qt*")

               b.chdir("3rd-party")
               b.do("", "tar jxvf ../../qwt5.2.tbz")
               b.chdir("../..")
           else:
               b.do("", "tar jxvf " + third_party)

               if architecture == "10.6":
                   b.comment("In the, not so likely, case that you are running Snow Leopard on older hardware without 64bit support, you will need to do the following, because the distributed python and perl executables do not run on 32bit):")
                   
                   b.do("", "rm ", prefix, "/../", third, "/bin/python")
                   b.do("", "rm ", prefix, "/../", third, "/bin/pythonw")
                   b.do("", "ln -s /usr/bin/python ", prefix, "/../", third, "/bin/python")
                   b.do("", "ln -s /usr/bin/pythonw ", prefix, "/../", third, "/bin/pythonw")
                   
                   b.comment("The python version must be 2.6.4. If it is not, go and install python 2.6.4")
                   b.do("",  prefix, "/../", third, "/bin/python -V")
                   b.do("",  prefix, "/../", third, "/bin/pythonw -V")
                   
                   b.do("Also, if your hardware is 32 bit, you need to put a perl in the path which works (version is less critical)",
                        "rm ", prefix, "/../", third, "/bin/perl")
                   b.do("", "which perl")

           
           if architecture == "10.5":
               b.do("", "hdiutil attach ./qt-sdk-mac-opensource-2009.02.dmg -mountroot .")
           
               b.comment("If you did not install Qt already, do 'sudo installer -package Qt\ SDK/QtSDK.mpkg/ -target .'")
               b.comment("Password:")
               b.comment("installer: Package name is Qt SDK")
               b.comment("installer: Installing at base path /")
               b.comment("installer: The install was successful.")
           
           else:
               b.comment("If you did not already, install gfortran")
               b.comment("'sudo installer -pkg " + gfortran + " -target /'")
               b.comment("2010-02-10 19:46:59.675 installer[463:4007] PackageKit: *** Missing bundle identifier: /Library/Receipts/Citrix ICA Client.pkg")
               b.comment("2010-02-10 19:46:59.850 installer[463:4007] PackageKit: *** Missing bundle identifier: /Library/Receipts/vpnclient-api.pkg")
               b.comment("2010-02-10 19:46:59.854 installer[463:4007] PackageKit: *** Missing bundle identifier: /Library/Receipts/vpnclient-bin.pkg")
               b.comment("2010-02-10 19:46:59.864 installer[463:4007] PackageKit: *** Missing bundle identifier: /Library/Receipts/vpnclient-gui.pkg")
               b.comment("2010-02-10 19:46:59.880 installer[463:4007] PackageKit: *** Missing bundle identifier: /Library/Receipts/vpnclient-kext.pkg")
               b.comment("2010-02-10 19:46:59.889 installer[463:4007] PackageKit: *** Missing bundle identifier: /Library/Receipts/vpnclient-profiles.pkg")
               b.comment("2010-02-10 19:46:59.906 installer[463:4007] PackageKit: *** Missing bundle identifier: /Library/Receipts/vpnclient-startup.pkg")
               b.comment("installer: Package name is GNU Fortran 4.2.4 for Xcode 3.2 (build 5646)")
               b.comment("installer: Installing at base path /")
               b.comment("installer: The install was successful.")



           b.comment("If you did not already install CMake, do it now.")
           b.chdir(prefix, "/..")
           b.do("", "curl http://www.cmake.org/files/v2.8/cmake-2.8.0-Darwin-universal.dmg -o cmake-2.8.0-Darwin-universal.dmg")
           b.do("", "hdiutil attach cmake-2.8.0-Darwin-universal.dmg  -mountroot .")
           b.comment("sudo installer -package cmake-2.8.0-Darwin-universal/cmake-2.8.0-Darwin-universal.pkg -target /")
           b.comment("Password:")
           b.comment("installer: Package name is CMake")
           b.comment("installer: Installing at base path /")
           b.comment("installer: The install was successful.")

        else:
            # Linux
            b.comment("On Linux, we use yum to install the required RPM development packages. You will need root access to do so. In the following the sudo command is used.")
            b.do("First, setup the yum configuration to point to CASA's repositories. A way to do that is to create a /etc/yum.repos.d/casa.repo containing the following four lines (execute these commands)",
                 "echo [casa] > casa.repo")
            b.do("", "echo \"name=CASA RPMs for RedHat Enterprise Linux 5 (", arch, ")\" >> casa.repo")
            b.do("", "echo \"baseurl=https://svn.cv.nrao.edu/casa/repo/el5/", arch, "\" >> casa.repo")
            b.do("", "echo \"gpgkey=https://svn.cv.nrao.edu/casa/RPM-GPG-KEY-casa http://ww.jpackage.org/jpackage.asc http://svn.cv.nrao.edu/casa/repo/el5/RPM-GPG-KEY-EPEL\" >> casa.repo")
            b.do("Move the file to /etc/yum.repos.d/ as superuser", "sudo mv casa.repo /etc/yum.repos.d/")
            b.do("Your yum configuration should now look like this,", "cat /etc/yum.repos.d/casa.repo")
                        
            b.do("Remove this package if you have it installed (which is to avoid conflicts with qt434-devel-4.3.4),",
                 "sudo yum -y erase qt4-devel")
            
            b.comment("Install all required development packages. Either answer yes when prompted, or pass the -y flag to yum.")
            if False:
                # Brute force
                b.do("", "sudo yum -y install casapy-test-devel")
            else:
                b.do("", "sudo yum -y install antlr-c++-devel antlr-c++-shared casapy-boost casapy-boost-devel casapy-ccmtools casapy-ccmtools-python casapy-ccmtools-shared casapy-python casapy-python-devel cfitsio-devel dbus-c++ fftw3 fftw3-devel qt434-devel qt434-qwt-devel rpfits tix tix-devel wcslib xerces-c xerces-c-devel aatm aatm-devel dbus-c++-devel blas-devel lapack lapack-devel pgplot pgplot-devel cmake")

            # It should cause packages to be installed only from the casa RPM
            # repository. If there were conflicts with other repositories, you 
            # might need to b.do("", "sudo rm -f /etc/yum.repos.d/*") and start over

            b.comment("Create the directory where you want to build CASA. Here we choose to build everything in /opt/casa. You could also create a subdirectory in your home directory, but be aware that if have an NFS mounted home directory, all the file I/O that the build system is going to do, will be somewhat slower.")

            b.do("", "sudo chmod 777 /opt")
            b.do("", "mkdir ", prefix)

            b.comment("For building CASACore you will need to install SCons")
            b.chdir("/tmp")
            b.do("", "wget http://prdownloads.sourceforge.net/scons/scons-1.2.0.tar.gz")

            # Darwin: b.do("", "curl -o ./scons-1.2.0.tar.gz http://heanet.dl.sourceforge.net/project/scons/scons/1.2.0/scons-1.2.0.tar.gz")
            # But don't need to install scons on mac

            b.do("", "tar zxvf scons-1.2.0.tar.gz")
            b.chdir("scons-1.2.0")
            b.do("You can install the scons executable whereever you like, just change the --prefix", "python setup.py install --prefix=", prefix, "/scons")

    # endif full build

    if type == "incremental" and ops == "Linux":
      if False:
        b.do("", "sudo yum -y upgrade antlr-c++-devel antlr-c++-shared casapy-boost casapy-boost-devel casapy-ccmtools casapy-ccmtools-python casapy-ccmtools-shared casapy-python casapy-python-devel cfitsio-devel dbus-c++ fftw3 fftw3-devel qt434-devel qt434-qwt-devel rpfits tix tix-devel wcslib xerces-c xerces-c-devel aatm aatm-devel dbus-c++-devel blas-devel cmake")

    if ops == "Darwin":
        b.comment("It is essential that 3rd-party/bin comes before other stuff in your $PATH.")
        b.set_env('PATH', prefix, "/../", third, "/bin:$PATH")
        b.comment("This is a workaround for QWT and CASACore libraries not containing the right rpaths (Note: Do not define the environment variable named DYLD_LIBRARY_PATH or something else might break.)")
        if architecture == "10.5":
            qwtdir = "qwt-5.2.0"
        else:
            qwtdir = "qwt-5.2.1-svn"
        b.set_env('DYLD_FALLBACK_LIBRARY_PATH', prefix, "/", builddir, "/lib:", prefix, "/../", third, "/" + qwtdir + "/lib:", prefix, "/../", third, "/lib")
        if architecture == "10.6":
            b.comment("Also, you need to define DYLD_FRAMEWORK_PATH in order to work around a linking issue with the ccmtools libraries,")
            b.set_env('DYLD_FRAMEWORK_PATH', prefix, "/../", third, "/Library/Frameworks")
    else:
        b.set_env('PATH', "$PATH:", prefix, "/scons/bin:/usr/lib/casapy/bin")

    if type == "full":
        b.comment("You have now installed all prequisite packages. It is time to compile.")

    if type == "full" and ops == "Darwin":
        b.comment("Install aatm from sources.")
        b.chdir(prefix)
        b.svn_exe(url + "/aatm", revision, "aatm")
        b.chdir("aatm")
        if ops == "Linux":
            b.do("", "./configure --prefix=", prefix, "/", builddir, " --without-boost")
        elif architecture == "10.5":
            b.do("", "./configure --prefix=", prefix, "/", builddir, " --with-boost=", prefix, "/../", third, " --with-boost-program-options=\"boost_program_options-mt-1_35\" LDFLAGS=\"-march=i586 -mtune=nocona\" CFLAGS=\"-march=i586 -mtune=nocona\" CXXFLAGS=\"-march=i586 -mtune=nocona\"")
        else:
            b.do("", "./configure --prefix=", prefix, "/", builddir, " --with-boost=", prefix, "/../", third, " LDFLAGS=\"-arch i386\" CXXFLAGS=\"-arch i386\"")
        b.do("", "make -j ", threads)
        b.do("", "make install")

    if type == "full" and ops == "Darwin" and architecture == "10.5":
        b.comment("Build WCSLIB from sources")
        b.chdir(prefix)
        b.svn_exe(url + "/wcslib", revision, "wcslib")
        b.chdir("./wcslib")

        b.do("This directory must already exist, or the WCSLIB build will fail",
             "mkdir -p ../", builddir, "/bin")
        
        b.comment("Compile WCSLIB, make sure that . is in the PATH, or the build will fail")
        b.set_env('PATH', "$PATH:.")
        b.do("", "./configure --prefix=", prefix, "/", builddir, " F77=gfortran FFLAGS=-fPIC CFLAGS=-fPIC")
        b.do("Warning: Do not try parallel building (failures have been seen which do not happen with a non-parallel build)", "make -j 1")
        b.do("", "make install")
        b.chdir("..")

    b.chdir(prefix)
        
    if type != "test":
        b.comment("Check out CASACore's sources.")
        b.svn_exe(url + "/casacore", revision, "casacore")
    
    if type == "full":
        b.comment("We need parts of CASA's data repository, ", prefix, "/data/geodetic and ", prefix, "/data/ephemerides, in order to build CASACore.")
        if ops == "Darwin":
            b.comment("The location of the data repository at /opt/casa/data is fixed on Mac.")
        b.chdir("..")
        b.do("", "rm -rf ./data")
        b.do("", "mkdir -p ./data")
        b.chdir("./data")
        b.do("", "svn checkout https://svn.cv.nrao.edu/svn/casa-data/trunk/ephemerides ephemerides")
        b.do("", "svn checkout https://svn.cv.nrao.edu/svn/casa-data/trunk/geodetic geodetic")
        b.chdir(prefix)
        b.do("", "ln -s /opt/casa/data .")

    b.comment("Compile CASACore.")

    b.chdir("casacore")
    
    if architecture == "x86_64":
        extra_cpp_flags = platform('extra_cpp_flags', ' -DAIPS_64B')
    else:
        extra_cpp_flags = platform('extra_cpp_flags', '')
     
    if type == "test":
      if False:
        b.do("", "scons -j ", threads, " test")
    else:
        b.comment("You need to give quite many command line options to scons for it to find the packages it needs. You are advised to put the scons command in a one-liner script (or save the command somewhere else), so that you can easily repeat it later.")
        b.comment("SCons' -j option is used to enable parallel builds. See scons --help for more.")
        b.comment("The --extra-cppflags option specifies the compile flags (for C++ code), you can add -g in order to compile with debugging info enabled.")
        if ops == "Linux":
            b.comment("Also, you should use -fno-omit-frame-pointer for better debugging support on 64-bit Linux.")
            b.do("", "scons -j ", threads, " --prefix=", prefix, "/", builddir, " --extra-cppflags=\"-g -fno-omit-frame-pointer -DCASA_USECASAPATH -DCASACORE_NEEDS_RETHROW", extra_cpp_flags, "\" --libdir=", prefix, "/", builddir, "/lib --data-dir=", prefix, "/data --cfitsio-libdir=/usr/lib --cfitsio-incdir=/usr/include/cfitsio --extra-cflags=\"-g -fno-omit-frame-pointer\" --extra-fflags=\"-g  -fno-omit-frame-pointer\" --extra-libs=\"-lfftw3f_threads -lfftw3_threads -lfftw3f -lfftw3 -lgomp\" --enable-shared --disable-static")
        elif ops == "Darwin":
            if architecture == "10.6":
                archflags = platform('archflags', '-arch i386')
            else:
                archflags = platform('archflags', '')

            b.do("", "scons -j ", threads, " --extra-root=", prefix, "/../", third, " --prefix=", prefix, "/", builddir, " --extra-cppflags=\"", archflags, " -g -DCASA_USECASAPATH -DCASACORE_NEEDS_RETHROW -DCASACORE_NOEXIT -DCASA_NOTAPE -fopenmp\" --data-dir=", prefix, "/../data --wcs-root=", prefix, "/", builddir, " --cfitsio-libdir=", prefix, "/../", third, "/lib --cfitsio-incdir=", prefix, "/../", third, "/include --fftw3-root=", prefix, "/../", third, " --extra-libs=\"-lfftw3f_threads -lfftw3_threads -lfftw3f -lfftw3 -lgomp\" --extra-linkflags=\"", archflags, "\" --extra-fflags=\"", archflags, "\" --enable-shared --disable-static")
        else:
            assert False
        b.do("If scons did not end with an error message, the build was successful and you can install the libraries and headers", "scons install")
        if ops == "Darwin":
            b.comment("If the compilation of CASACore stops with the error message \"flex: fatal internal error, exec failed\", it might help to download and install flex-2.5.35 from sources, put it in your path and try again.")
    b.chdir("..")


    b.comment("Next up is CASA non-core,")

    if type != "test":
        b.svn_exe(url + "/code", revision, "code")

    b.chdir("code")

    if type == "full":
        b.comment("You need to create a build directory, which holds cmake internals and object files (libraries etc.), before they are installed into ", prefix, "/", builddir)
        b.comment("You may have more than one build directory, e.g. for debug and release, or cross compiling. We'll just use one named build")
        b.do("", "mkdir ./build")
        b.chdir("build")
        b.do("Invoke CMake. The argument to cmake is the path to your source directory, in this case the parent directory", "cmake ..")
        b.chdir("..")
   
    if type != "test":
        b.chdir("build")
        b.do("", "make -j ", threads, " VERBOSE=on")
        b.do("You should now have the casapy executable (and other executables) at this location:",
             "ls -l ", prefix, "/", builddir, "/bin")

    b.comment("Then setup your CASAPATH and PATH environment variable in order to be able to run casapy (and compile ASAP). Just source the script that was created during the build:")
    b.do("", "source ", prefix, "/casainit.sh")
    b.comment("or you can set the variables manually (note: it is a space, not a '/', in CASAPATH)")
    b.set_env('CASAPATH', "", prefix, " ", builddir)
    b.set_env('PATH', "$PATH:", prefix, "/", builddir, "/bin")

    if type != "test":
        b.comment("Build ASAP (optionally, if you want to reduce single dish data). This must be done after building the rest of CASA.")
        b.chdir(prefix)
        b.svn_exe(url + "/asap", revision, "asap")
        b.chdir("asap")
        b.do("", "make")
        b.do("", "make install")

    if type == "test" or type == "full":
        b.comment("In order to verify that your build is correct, you may want to run the python unit test suite.")

        b.comment("First you must check out CASA's data repository from https://svn.cv.nrao.edu/svn/casa-data/trunk and create a link to it from where you built CASA.")
        b.comment("The data repository is big. You can save 50% of disk space if you retrieve the repository with 'svn export' instead of 'svn checkout'.")
        b.do("Remove the link first (in case it already exists)", "rm -f ", prefix, "/data")
        b.do("", "ln -s ", datadir, " ", prefix, "/data")

        b.do("Then launch the python unit test suite,", "casapy --nogui --log2term -c ", prefix, "/code/xmlcasa/scripts/regressions/admin/runUnitTest.py --mem")
        b.comment("which will summarize near the end if your build is okay or if there were problems.")
        #
        # Does not seem to return here... 
        #
        #b.do("", "rm -f $HOME/XUnit.tar.gz")
        #b.chdir("nosexml")
        #b.do("", "tar zcf $HOME/XUnit.tar.gz *.xml")

        if False:
            b.comment("Run regression suite...")
            b.do("Link to the local data repository checkout", "rm -rf ", prefix, "/data")
            b.do("", "ln -s ", datadir, " ", prefix, "/data")
            b.do("Get old results out of the way and launch the suite,", "rm -rf /tmp/results")
            b.do("", "mkdir -p /tmp/results")
            b.do("", prefix, "/code/xmlcasa/scripts/regressions/admin/scheduler.pl -all -noloop -noclean -res_dir=/tmp/results -work=/tmp/work /tmp/ ", prefix, "/data 9999 14400 </dev/null")

def main(argv):
    if len(argv) < 4:
        raise Exception("Usage: %s svn_url svn_revision build_type [dry]" % argv[0])
    url = argv[1]
    revision = argv[2]
    type = argv[3]

    assert type in ["full", "incremental", "test"]

    # Determine architecture
    oss = os.uname()[0]
    if oss == "Linux":
        arch = os.uname()[4]
        if arch == 'i686':
            arch = 'i386'
    elif oss == "Darwin":
        sw_vers = commands.getoutput("/usr/bin/sw_vers -productVersion")
        arch = sw_vers[:4]
        if not arch in ["10.5", "10.6"]:
            raise Exception("Unsupported software version: %s" % sw_vers)

    if False:
        # Create HTML and shell script
        b = html_builder('/tmp/build.html', oss, arch, type)
        build_casa(b, url, revision, type, oss, arch)

        s = sh_builder('/tmp/build.sh', type)
        build_casa(s, url, revision, type, oss, arch)
        s = 123  # force destructor call
        sys.exit(0)

    dry = (len(argv) > 4)
    doc_dir = os.getenv('HOME') + "/documentation/" + type + "-" + oss + "-" + arch
    if dry:
        print "Writing documentation for", oss, arch, "to", doc_dir
    else:
        print "Going to build on", oss, arch
    sys.stdout.flush()
    b = exe_and_doc_builder(doc_dir,
                            oss, arch, type = type, dry = dry)
    build_casa(b, url, revision, type, oss, arch)
    return 0          

if __name__ == "__main__":
    sys.exit(main(sys.argv))
