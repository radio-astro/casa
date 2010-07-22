import casac
from taskinit import casalog
from imageTest import * 
from testbase import *
from tableMaker import *
import time
import os
import shutil
import commands
import sys
import signal
import pdb
import traceback
import re
import cProfile

PYVER = str(sys.version_info[0]) + "." + str(sys.version_info[1])

imager = casac.homefinder.find_home_by_name('imagerHome')
image = casac.homefinder.find_home_by_name('imageHome')
quantity=casac.Quantity
# DATA_REPOS=[ '/home/onager4/Regression/Data/', '/aips++/data/', '/home/rohir2/jmcmulli/']
# SCRIPT_REPOS='/home/ballista3/Regression/Scripts/'
# WORKING_DIR='/home/onager4/Regression/WorkingDir/'
# RESULT_DIR='/home/ballista3/Regression/Result/'
# TEMPLATE_RESULT_DIR='/home/onager4/Regression/TemplateResults/'
# RESULT_TEMPLATE_FOLDER='/home/ballista3/Regression/COMPARISON_VALUES_latest.txt'
#DATA_REPOS=[ '/scratch/ALMA/casa/data/']
#DATA_REPOS=['/tmp/hest']
#INFRASTRUC_DIR='/scratch/ALMA/casa/test/admin/'
#WORKING_DIR='/scratch/tmp/Regression/WorkingDir/'
#RESULT_DIR='/scratch/tmp/Regression/Result/'
#TEMPLATE_RESULT_DIR='/scratch/ALMA/casa/test/tests/third/reference/'

AIPS_DIR = os.environ["CASAPATH"].split()[0]

SCRIPT_REPOS=AIPS_DIR+'/code/xmlcasa/scripts/regressions/'
UTILS_DIR = AIPS_DIR+'/code/xmlcasa/scripts/regressions/admin/'
if not os.access(SCRIPT_REPOS, os.F_OK):
    if os.access(AIPS_DIR+'/lib64', os.F_OK):
        SCRIPT_REPOS = AIPS_DIR+'/lib64/python'+PYVER+'/regressions/'
        UTILS_DIR = AIPS_DIR+'/lib64/casapy/bin/'
    elif os.access(AIPS_DIR+'/lib', os.F_OK):
        SCRIPT_REPOS = AIPS_DIR+'/lib/python'+PYVER+'/regressions/'
        UTILS_DIR = AIPS_DIR+'/lib/casapy/bin/'        
    else:            #Mac release
        SCRIPT_REPOS = AIPS_DIR+'/Resources/python/regressions/'
        UTILS_DIR = AIPS_DIR+'/MacOS/'        
# because casapy releases have a different directory structure


# set to True to skip the test execution and reuse product files
#dry=True
dry = False

class runTest:
    def __init__(self, test, \
                 DATA_REPOS=[AIPS_DIR+'/data'], \
                 WORKING_DIR='/tmp/casa_regression_work/', \
                 RESULT_DIR='/tmp/casa_regression_result/', \
                 retemplate=False,
                 cleanup=True,
                 profile=False):
        """cleanup: set to False to keep data around.
        profile: set to True to enable C++ profiling.  This requires that the command 'sudo opcontrol' must work.  You also need the 'dot' tool distributed as part of graphviz.  Run 'dot -Txxx' to verify that your dot installation supports PNG images.
        Note, a profile is created only for the casapy process. If you want to include profiles for async / child processes, refer to the documentation for opreport."""
        casalog.showconsole(onconsole=True)
        
        TEMPLATE_RESULT_DIR=AIPS_DIR+'/data/regression/'
        tests = [test]
        if type(tests) != type([]):
            raise TypeError
        self.resultdir=RESULT_DIR
        self.imdir=WORKING_DIR+'/IMAGES/'
        self.tester=testbase(WORKING_DIR)
        self.imagertests=[]
        self.result=[]
        self.numTests=0
        ####Get the directories right
        self.tester.setDataBaseDir(DATA_REPOS)
        self.tester.setScriptsDir(SCRIPT_REPOS)
        self.tester.setResultDir(RESULT_DIR)
        self.tester.setWorkingDir(WORKING_DIR)
        self.resultsubdir = ''

        print SCRIPT_REPOS
        
        if((len(tests)==1) and (tests[0]=='all')):
            self.numTests=self.tester.locateTests()
        else:
            self.numTests=self.tester.locateTests(tests)
        testName=''
             
        #pdb.set_trace()
        for k in range(self.numTests) :
            ### cleanup before each test
            if not dry and cleanup:
                self.tester.cleanup()

            self.tester.createDirs()

            uname1 = os.uname()[1]

            if self.tester.testname(k)[0:6] == 'tests/':
                testName=string.split(self.tester.testname(k)[6:], ".py")[0]
            else:
                testName=string.split(self.tester.testname(k), ".py")[0]
            self.resultsubdir = \
                              self.resultdir + "/result-" + \
                              testName       + "-" + \
                              uname1         + "-" + \
                              time.strftime('%Y_%m_%d_%H_%M')
            if not os.path.isdir(self.resultsubdir):
                os.mkdir(self.resultsubdir)

            logfilename = testName+'.log'
            if (os.path.isfile(self.resultsubdir+'/'+logfilename)):
                os.remove(self.resultsubdir+'/'+logfilename)

            # redirect stdout and stderr and casalog
            print 'Run test '+testName
            print "Redirect stdout/stderr to", self.resultsubdir+'/'+logfilename
            save_stdout = sys.stdout
            save_stderr = sys.stderr
            fsock = open(self.resultsubdir+'/'+logfilename, 'w')
            sys.stdout = logger("STDOUT", [save_stdout, fsock])
            sys.stderr = logger("STDERR", [save_stderr, fsock])

            testlog = self.tester.workingDirectory+"/test.log"
            open(testlog, "w").close()  # create empty file
            casalog.setlogfile(testlog) # seems to append to an existing file

            try:
                self.tester.getTest(self.tester.testname(k), testName)

                profilepage = RESULT_DIR+'/'+time.strftime('%Y_%m_%d/')+testName+'_profile.html'
                process_data = "%s/profile.txt"  % self.tester.workingDirectory

                os.system("echo -n > " + process_data)
                pp = SCRIPT_REPOS + '/profileplot.py'  # for release
                if not os.path.isfile(pp):
                    pp = SCRIPT_REPOS + '/admin/profileplot.py' # for devel
                pyt = UTILS_DIR + '/python'  # for release
                if not os.path.isfile(pyt):
                    pyt = '/usr/lib64/casapy/bin/python'    # for devel
                if not os.path.isfile(pyt):
                    pyt = '/usr/lib/casapy/bin/python'    # for devel
                profileplot_pid=os.spawnlp( \
                    os.P_NOWAIT,
                    pyt,
                    pyt,
                    pp,
                    testName, RESULT_DIR, profilepage, process_data)
                presentDir=os.getcwd()
                os.chdir(self.tester.workingDirectory)

                short_description = self.tester.getDescription(testName, k)
                if short_description != None and short_description.find("'") >= 0:
                    print >> sys.stderr, \
                          "Warning: Short description contains ': '%s'" % \
                          short_description
                    short_description = short_description.replace("'", "")

                prof = cProfile.Profile()
                
                try:
                    self.op_init(profile)
                    time1=time.time()

                    #prof.runctx("(leResult, leImages)=self.tester.runtests(testName, k, dry)", globals(), locals())
                    #prof.runctx("(leResult, leImages)=self.tester.runtests(testName, k, dry)", gl, lo)
                    #prof.run("(leResult, leImages) = self.tester.runtests(testName, k, dry)")
                    (leResult, leImages) = prof.runcall(self.tester.runtests, testName, k, dry)

                    # returns absolute_paths, relative_paths
                    exec_success = True
                except:
                    leResult=[]
                    exec_success = False
                    print >> sys.stderr, "%s failed, dumping traceback:" % testName
                    traceback.print_exc() # print and swallow exception

                time2=time.time()
                time2=(time2-time1)/60.0
                
                try:
                    print "now in ", os.getcwd()
                    prof.dump_stats(self.resultsubdir+'/cProfile.profile')
                except:
                    print >> sys.stderr, "Failed to write profiling data!"
               
                self.op_done(profile)

                # Dump contents of any *.log file produced
                # by the regression script
                #
                # !! Does not handle out of diskspace
                #
                files = os.listdir('.')
                for f in files:
                    if f != 'casapy.log' and \
                           re.compile('.log$').search(f) != None:

                        for line in open(f, 'r'):
                            #print f + '    ddd'+line
                            fsock.write(f + ': ' + line)

                #
                # Report and deal with out of diskspace
                #
                space_left = commands.getoutput( \
                    "df -k " + self.tester.workingDirectory + \
                    " | awk '{print $4}' | tail -1")
                space_left_h = commands.getoutput( \
                    "df -h " + self.tester.workingDirectory + \
                    " | awk '{print $4}' | tail -1")
                space_used = commands.getoutput( \
                    "du -kc " + self.tester.workingDirectory + \
                    " | tail -1 | awk '{print $1}'")
                space_used_h = commands.getoutput( \
                    "du -hc " + self.tester.workingDirectory + \
                    " | tail -1 | awk '{print $1}'")

                if int(space_left) < 1000*1000:
                    print >> sys.stderr, "Warning: Only " + \
                          space_left_h + ' disk space left, ' + \
                          space_used_h + ' used'
                    # Clean up early, so that this infrastructure can continue
                    if not exec_success and cleanup:
                        self.tester.cleanup()
                        
                # Copy C++ profiling info
                if profile:
                    os.system('cp cpp_profile.* ' + self.resultsubdir)

                os.chdir(presentDir)

                # Terminate profiling process
                os.kill(profileplot_pid,signal.SIGHUP)
                status = os.waitpid(profileplot_pid, 0)[1]
                #print str(profileplot_pid) + ' exit: ' + str(status)

                pagename=time.strftime('%Y_%m_%d/')+testName+'_profile.html'

                # entries common for all tests based on this run
                self.result_common = {}
                self.result_common['CASA'] = "'" + self.get_casa_version() + "'", "CASA version"
                self.result_common['host'] = uname1, "os.uname[1]"
                self.result_common['platform'] = "'" + self.get_platform()[0] + " " + self.get_platform()[1] + "'", "OS"

                self.result_common['date'] = time.strftime('%Y_%m_%d_%H_%M'), ""
                self.result_common['testid'] = testName, "test name"
                if short_description != None:
                    self.result_common['description'] = "'" + short_description + "'", "test short description"

                # Figure out data repository version
                if os.system("which svnversion") == 0:
                    (errorcode, datasvnr) = commands.getstatusoutput('cd '+DATA_REPOS[0]+' && svnversion 2>&1 | grep -vi warning')
                else:
                    errorcode = 1
                if errorcode != 0 or datasvnr == "exported":
                    # If that didn't work, look at ./version in the toplevel dir
                    (errorcode, datasvnr) = commands.getstatusoutput( \
                        'cd '+DATA_REPOS[0]+" && grep -E 'Rev:' version" \
                        )
                    if errorcode != 0:
                        datasvnr = "Unknown version"
                        
                self.result_common['data_version'] = "'"+datasvnr+"'", "Data repository version"

                # execution test
                exec_result = self.result_common.copy()
                exec_result['version'] = 2, "version of this file"
                exec_result['type'] = "exec", "test type"
                exec_result['time'] = time2*60, "execution time in seconds"
                exec_result['disk'] = space_used, "disk space (KB) in use after test"
                exec_result['runlog'] = logfilename, "execution logfile"

                # read time/memory data
                mem = ""
                try:
                    process_file = open(process_data, "r")
                except:
                    print "Warning: Failed to open file:", process_data
                    process_file = None

                if process_file != None:
                    lineno = 0
                    for line in process_file:
                        lineno += 1
                        if len(line) > 0 and line[0] != '#':
                            try:
                                (t, m_virtual, m_resident, nfiledesc,
                                 cpu_us, cpu_sy, cpu_id, cpu_wa) = line.split()
                                mem = mem + \
                                      str(t)          + ',' + \
                                      str(m_virtual)  + ',' + \
                                      str(m_resident) + ',' + \
                                      str(nfiledesc)  + ',' + \
                                      str(cpu_us)     + ',' + \
                                      str(cpu_sy)     + ',' + \
                                      str(cpu_id)     + ',' + \
                                      str(cpu_wa)     + ';'
                            except:
                                print >> sys.stderr, "Error parsing %s:%d: '%s'" % \
                                      (process_data, lineno, line)
                    process_file.close()
                exec_result['resource'] = mem, "time(s),virtual(Mbytes),resident(Mbytes),nfiledesc,cpu_us,cpu_sy,cpu_id,cpu_wa"

                whatToTest=self.tester.whatQualityTest()
                keys=[]
                #if len(whatToTest) != 0:
                #    keys=whatToTest.keys()
                #    print 'THE KEYS ARE ', keys
                for j in range(len(leResult)) :   
                    templateImage=TEMPLATE_RESULT_DIR+"/"+testName+"/reference/"+leImages[j]
                    if retemplate:
                        if os.access(templateImage, os.F_OK):
                            shutil.rmtree(templateImage)
                    print 'TemplateImage '+templateImage
                    print 'theImage '+leResult[j]
                    print 'theImage '+leImages[j]

                    product_exists  = os.access(leResult[j], os.F_OK)
                    template_exists = os.access(templateImage, os.F_OK)

                    if product_exists and retemplate:
                        print 'Create template from', leResult[j]
                        if not os.path.isdir(TEMPLATE_RESULT_DIR+"/"+testName):
                            os.mkdir(TEMPLATE_RESULT_DIR+"/"+testName)
                        shutil.copytree(leResult[j], templateImage)

                    if not product_exists:
                        print >> sys.stderr, leResult[j], 'missing!'
                        exec_success = False
                        whatToTest[leResult[j]] = []
                        
                    if not template_exists:
                        print >> sys.stderr, templateImage, 'missing!'

                    for leQualityTest in whatToTest[leResult[j]] :
                        print leResult[j]+' WHAT : ', whatToTest[leResult[j]]
                        self.result=self.result_common.copy()
                        self.result['version'] = 1, "version of this file"
                        self.result['type'] = leQualityTest, "test type"
                        self.result['image'] = leImages[j], "test image"

                        if not product_exists:
                            self.result['status'] = 'fail', "result of regression test"
                            self.result['reason'] = "'Product image missing'", "reason of failure"
                        elif not template_exists:
                            self.result['status'] = 'fail', "result of regression test"
                            self.result['reason'] = "'Reference image missing'", "reason of failure"
                        else:
                            if os.access(self.imdir, os.F_OK):
                                shutil.rmtree(self.imdir)
                                
                            if(leQualityTest=='simple'):
                                self.simpleStats(leResult[j], templateImage, testName, WORKING_DIR, RESULT_DIR)
                            elif(leQualityTest=='pol2'):
                                self.polImageTest(leResult[j], templateImage, testName, WORKING_DIR, RESULT_DIR, 2)
                            elif(leQualityTest=='pol4'):
                                self.polImageTest(leResult[j], templateImage, testName, WORKING_DIR, RESULT_DIR, 4)
                            elif(leQualityTest=='cube'):
                                self.cubeImageTest(leResult[j], templateImage, testName, WORKING_DIR, RESULT_DIR)
                            else:
                                self.polImageTest(leResult[j], templateImage, testName, WORKING_DIR, RESULT_DIR, 1)

                            # Pick up any images produced in test
                            if os.path.isdir(self.imdir):
                                i = 0
                                for image in os.listdir(self.imdir):
                                    i = i + 1
                                    shutil.copy(self.imdir + '/' + image, \
                                                self.resultsubdir+'/'+os.path.basename(image))
                                    self.result['imagefile_'+str(i)] = "'"+os.path.basename(image)+"'", 'regression image '+str(i)

                        self.create_log(leImages[j].replace('/', '-'))

                # Create exec log now that we now if
                # required images were produced
                exec_result['status'] = ("fail", "pass") [exec_success], "execution status"
                self.result = exec_result
                self.create_log("")

                # Restore stdout/stderr
                sys.stderr = save_stderr
                sys.stdout = save_stdout
                fsock.close()
                casalog.setlogfile("casapy.log")
                os.system("sed 's/^/casapy.log: /' "+testlog+" >> "+self.resultsubdir+'/'+logfilename)

                if not dry and cleanup:
                    self.tester.cleanup()
            except:                
                sys.stderr = save_stderr
                sys.stdout = save_stdout
                fsock.close()
                casalog.setlogfile("casapy.log")
                os.system("sed 's/^/casapy.log: /' "+testlog+" >> "+self.resultsubdir+'/'+logfilename)

                print "Unexpected error:", sys.exc_info()[0]
                raise
            
        # end for k...
                
        print "Created ", self.resultsubdir


    def op_init(self, oprofile):
        if oprofile:
            os.system("sudo opcontrol --deinit && sudo opcontrol --init && sudo opcontrol --reset && sudo opcontrol --start --callgraph=999 --no-vmlinux --separate=lib --event=\"default\"")

    def op_done(self, oprofile):
        if oprofile:
            casapy = os.environ["CASAPATH"].split()[0] + '/' + \
                     os.environ["CASAPATH"].split()[1] + '/bin/casapy'

            gprof2dot = os.environ["CASAPATH"].split()[0] + \
                        '/code/xmlcasa/scripts/regressions/admin/gprof2dot.py'
            
            os.system("sudo opcontrol --stop && sudo opcontrol --dump")
            os.system("opreport -clf image-exclude:/no-vmlinux " + casapy + " > cpp_profile.txt")
            os.system("cat cpp_profile.txt | " + gprof2dot + " -e0.1 -n1 -f oprofile > cpp_profile.dot")
            os.system("cat cpp_profile.dot | dot -Tpng -o cpp_profile.png")
            os.system("opannotate --source > cpp_profile.cc")


    def polImageTest(self, imageName, templateImage, testName, WORKING_DIR, RESULT_DIR, numPol=2):
        a = ImageTest(imageName, write=True,
                      resultDir = self.resultdir,
                      imDir=self.imdir)
        b = ImageTest(templateImage, write=False,
                      resultDir = self.resultdir,
                      imDir=self.imdir)
        status = 1
        pol=['I', 'V']
        if(numPol==4):
            pol=['I','Q','U','V']
        quickresult='<pre>'
        ##do only the 'I' subtraction
        for k in range(1):
            print 'POL TEST ', k, 'numpol ', numPol
            out1, rms1 = a.bmodel(plane=k)
            out2, rms2 = b.bmodel(plane=k)
 #           rms1=a.subtract(plane=k)
 #           rms2=b.subtract(plane=k)
        
#            quickresult+=('Pol #%s\n  Image    coord: [%.3f,%.3f]\n  FWHM in x: %.6f\n  FWHM in y: %.6f\n'%(pol[k],out1[0][0],out1[0][1],out1[0][2],out1[0][3]))
#            quickresult+=('Pol #%s\n  Template coord: [%.3f,%.3f]\n  FWHM in x: %.6f\n  FWHM in y: %.6f\n'%(pol[k],out2[0][0],out2[0][1],out2[0][2],out2[0][3]))
            if(out1[0] != False and out2[0] != False):
                quickresult+=('Pol %s : Component Found in Image  :\nra %s  dec %s \nbmax %s bmin %s \nbpa %s flux %s \n'% \
                              (pol[k],\
                               out1[0][0],out1[0][1],\
                               out1[0][2],out1[0][3],\
                               out1[0][4],out1[0][5]))
                quickresult+=('Pol %s : Component Found in Template:\nra %s dec %s \nbmax %s bmin %s \nbpa %s flux %s \n'% \
                              (pol[k],out2[0][0],out2[0][1],out2[0][2],out2[0][3],out2[0][4],out2[0][5]))

                self.result['image_'+pol[k]+'_ra']   = "'"+out1[0][0]+"'", pol[k]+" component RA"
                self.result['image_'+pol[k]+'_dec']  = "'"+out1[0][1]+"'", pol[k]+" component DEC"
                self.result['image_'+pol[k]+'_bmax'] = "'"+out1[0][2]+"'", "major axis"
                self.result['image_'+pol[k]+'_bmin'] = "'"+out1[0][3]+"'", "minor axis"
                self.result['image_'+pol[k]+'_bpa']  = "'"+out1[0][4]+"'", "position angle"
                self.result['image_'+pol[k]+'_flux'] = "'"+out1[0][5]+"'", "flux"

                # Duplicate of above
                self.result['ref_'+pol[k]+'_ra']   = "'"+out2[0][0]+"'", pol[k]+" component RA"
                self.result['ref_'+pol[k]+'_dec']  = "'"+out2[0][1]+"'", pol[k]+" component DEC"
                self.result['ref_'+pol[k]+'_bmax'] = "'"+out2[0][2]+"'", "major axis"
                self.result['ref_'+pol[k]+'_bmin'] = "'"+out2[0][3]+"'", "minor axis"
                self.result['ref_'+pol[k]+'_bpa']  = "'"+out2[0][4]+"'", "position angle"
                self.result['ref_'+pol[k]+'_flux'] = "'"+out2[0][5]+"'", "flux"


                if(abs(rms2-rms1) > rms2/2.0):
                    status=0
            else:
                quickresult+=('Image fitting did not converge \n')
                print >> sys.stderr, 'Image fitting did not converge'
                status=2
        ### Do a simple stats for pol Q,U,V
        for k in range(1, numPol):
            rms1,max1,min1,returnFlag1=b.simple_stats(plane=k)
            rms2,max2,min2,returnFlag=a.simple_stats(plane=k, sigma=rms1)
            quickresult+='Image Pol %s   min: %f\nmax: %f\nrms: %f \n' %\
                          (pol[k],min2,max2,rms2)
            quickresult+='Template Pol %s min: %f\nmax: %f\nrms: %f \n' %\
                          (pol[k],min1,max1,rms1)

            self.result['image_'+pol[k]+'_min'] = min2, pol[k]+" min"
            self.result['image_'+pol[k]+'_max'] = max2, pol[k]+" max"
            self.result['image_'+pol[k]+'_rms'] = rms2, pol[k]+" rms"

            self.result['ref_'+pol[k]+'_min'] = min1, pol[k]+" min"
            self.result['ref_'+pol[k]+'_max'] = max1, pol[k]+" max"
            self.result['ref_'+pol[k]+'_rms'] = rms1, pol[k]+" rms"
            
            if(not returnFlag):
                status=status*0
            if(abs(max2-max1) > rms2/2.0):
                status=status*0
            if(abs(min2-min1) > rms2/2.0):
                status=status*0   
        quickresult+='</pre>'
        page=a.done()
        b.done()
        
        self.result['status'] = ['fail', 'pass'][status==1], "result of regression test"
       
    def simpleStats(self, imageName, templateImage, testname, WORKING_DIR, RESULT_DIR):
        a=ImageTest(imageName,write=True,resultDir=self.resultdir,imDir=self.imdir)
        b=ImageTest(templateImage,write=False,resultDir=self.resultdir,imDir=self.imdir)
        rms1,max1,min1,returnFlag1=b.simple_stats()
        b.done()
        status=1   # 1     : pass
                   # 2     : unknown
                   # other : fail
        
        rms2,max2,min2,returnFlag=a.simple_stats(sigma=rms1)
        a.changeImage(templateImage)
        rms1,max1,min1,returnFlag1=a.simple_stats()
        if(not returnFlag):
            status=0
        quickresult='<pre>'
        quickresult+='Image    min: %f\nmax: %f\nrms: %f \n' %(min2,max2,rms2)
        quickresult+='Template min: %f\nmax: %f\nrms: %f \n' %(min1,max1,rms1)
        quickresult+='</pre>'
        page=a.done()
        #b.done()

        self.result['status'] = ['fail', 'pass'][status==1], "result of regression test"
        self.result['image_min'] = min2, "image min"
        self.result['image_max'] = max2, "image max"
        self.result['image_rms'] = rms2, "image rms"
        self.result['ref_min'] = min1, "reference min"
        self.result['ref_max'] = max1, "reference max"
        self.result['ref_rms'] = rms1, "reference rms"
        
    def cubeImageTest(self, imageName, templateImage, testname, WORKING_DIR, RESULT_DIR):
        a=ImageTest(imageName,write=True,resultDir=self.resultdir,imDir=self.imdir)
#        b=ImageTest(templateImage,write=False,resultDir=self.resultdir,imDir=self.imdir)
        status=1
#        XY1,fwhm1=a.auto_fitCube(a.b,verbose=0)
        XY1,fwhm1=a.auto_fitCube2()
        
        a.changeImage(templateImage)
        
#        XY2,fwhm2=a.auto_fitCube(a.b,verbose=0)

        XY2,fwhm2=a.auto_fitCube2()
        
        if(abs((XY1[0][0]-XY2[0][0])/XY2[0][0]) > 0.1):
            status=0
        if(abs((fwhm1[0]-fwhm2[0])/fwhm2[0]) > 0.1):
            status=0
        quickresult='<pre>'
        quickresult+='On image    \n  optimized coord: [%.3f,%.3f]\n  FWHM: %.6f\n\nfit #1\n  optimized coord: [%.3f,%.3f]\n  FWHM: %.6f \n'%(XY1[0][0],XY1[0][1],fwhm1[0],XY1[1][0],XY1[1][1],fwhm1[1])
        quickresult+='On Template \n  optimized coord: [%.3f,%.3f]\n  FWHM: %.6f\n\nfit #1\n  optimized coord: [%.3f,%.3f]\n  FWHM: %.6f \n'%(XY2[0][0],XY2[0][1],fwhm2[0],XY2[1][0],XY2[1][1],fwhm2[1])
        quickresult+='</pre>'
        page=a.done()
#        b.done()

        self.result['status'] = ['fail', 'pass'][status==1], "result of regression test"

        self.result['image_x'] = XY1[0][0], "image optimized coord"
        self.result['image_y'] = XY1[0][1], "image optimized coord"
        self.result['image_fwhm'] = fwhm1[0], "image FWHM"
        self.result['image_fit1_x'] = XY1[1][0], "image fit1 optimized coord"
        self.result['image_fit1_y'] = XY1[1][1], "image fit1 optimized coord"
        self.result['image_fit1_fwhm'] = fwhm1[1], "image fit1 FWHM"

        self.result['ref_x'] = XY2[0][0], "reference optimized coord"
        self.result['ref_y'] = XY2[0][1], "reference optimized coord"
        self.result['ref_fwhm'] = fwhm2[0], "reference FWHM"
        self.result['ref_fit1_x'] = XY2[1][0], "reference fit1 optimized coord"
        self.result['ref_fit1_y'] = XY2[1][1], "reference fit1 optimized coord"
        self.result['ref_fit1_fwhm'] = fwhm2[1], "reference fit1 FWHM"

    def get_casa_version(self):
        a=inspect.stack()
        stacklevel=0
        for k in range(len(a)):
            if (string.find(a[k][1], 'ipython console') > 0):
                stacklevel=k     
        myf=sys._getframe(stacklevel).f_globals

        return "CASA Version " + myf['casa']['build']['version'] + " (r"+myf['casa']['source']['revision'] + ")"

    def create_log(self, product_file):
        filename = "%s/result-%s-%s.txt" % \
                   (self.resultsubdir, product_file, \
                    self.result['type'][0])
        filename = filename.replace("--", "-")
        
        print "Writing file", filename
        try:
            self.logfd=open(filename, "w")
        except:
            print "Could not open file", filename
            raise
        for k, v in self.result.iteritems():
            # too much output for time/mem stats:
            print k, "=", str(v[0])[0:100], "#", v[1]
            self.logfd.write("%-20s = %-40s # %-20s\n" % (k, v[0], v[1]))
        self.logfd.close()

    def get_platform(self):
        OS = os.uname()[0]
        REV = os.uname()[2]
        MACH = os.uname()[4]
        if OS == 'SunOS':
            OS='Solaris'
            ARCH=commands.getoutput("uname -p")
            #return "%s %s (%s %s)" % (OS,REV,ARCH, commands.getoutput("uname -v"))
            return "%s %s" % (OS, MACH), \
                   "%s (%s %s)" % (REV,ARCH, commands.getoutput("uname -v"))
        elif OS == "AIX":
            return "%s %s" % (OS, MACH), \
                   "%s (%s)" % (commands.getoutput("oslevel"),
                                commands.getoutput("oslevel -r"))
        elif OS == "Linux" or OS == "Darwin":
            KERNEL=REV
            if os.path.isfile("/etc/redhat-release"):
                DIST = commands.getoutput("head -1 /etc/redhat-release")
            elif os.path.isfile("/etc/SUSE-release"):
                DIST = commands.getoutput("head -1 /etc/SUSE-release")
            elif os.path.isfile("/etc/SuSE-release"):
                DIST = commands.getoutput("head -1 /etc/SuSE-release")
            elif os.path.isfile("/etc/mandrake-release"):
                DIST = commands.getoutput("head -1 /etc/mandrake-release")
            elif os.path.isfile("/etc/lsb-release"):
                DIST = \
                     commands.getoutput("grep DISTRIB_ID /etc/lsb-release | sed 's/.*=\s*//'") + \
                     ' release ' + commands.getoutput("grep DISTRIB_RELEASE /etc/lsb-release | sed 's/.*=\s*//'") + \
                     ' (' + commands.getoutput("grep DISTRIB_CODENAME /etc/lsb-release | sed 's/.*=\s*//'") + \
                     ')'
            elif os.path.isfile("/etc/debian_version"):
                DIST = commands.getoutput("head -1 /etc/debian_version")
            else:
                DIST = "???"

            if MACH == "i686" or MACH == "i386":
                WORDSIZE = "32"
            elif MACH == "x86_64" or MACH == "ia64":
                WORDSIZE = "64"
            else:
                WORDSIZE = "??"

            if OS == "Darwin":
                if commands.getoutput("sysctl  -n hw.optional.x86_64").find("1") >= 0:
                    WORDSIZE = "64"
                else:
                    WORDSIZE = "32"

                vers = commands.getoutput("/usr/bin/sw_vers -productVersion")
                if vers.find("10.4") >= 0:
                    name = "Tiger"
                elif vers.find("10.5") >= 0:
                    name = "Leopard"
                elif vers.find("10.6") >= 0:
                    name = "Snow Leopard"

                DIST = commands.getoutput("/usr/bin/sw_vers -productName") + " " + \
                       vers + " (" + name + " " + \
                       commands.getoutput("/usr/bin/sw_vers -buildVersion") + ")"

            return "%s %s %s-bit" % (OS, MACH, WORDSIZE), DIST
            #return "%s %s %s (%s %s %s)" % (OS, DIST, REV, PSEUDONAME, KERNEL, MACH)
            #return "%s %s %s (%s)" % (OS, DIST, REV, PSEUDONAME)
        else:
            return "??? ??? ??-bit", "???"


class logger:
    def __init__(self, name, ss):
        self.streams = ss          # list of file objects where txt is sent
        self.name = name           # for printing only
        self.bol = True            # are we at beginning of line?
    def wwrite(self, txt, fback):  # this method is no throw
                                   # (because it is used to
                                   #   print error messages)
                                   #
                                   # fback: frame of caller
        try:
            #sys.__stderr__.write(txt)
            if self.bol:
                fb = fback
                if fb != None:
                    pos = " %s:%d" % \
                          (os.path.basename(fb.f_code.co_filename), \
                           fb.f_lineno)
                else:
                    pos = ""
                for s in self.streams:
                    s.write("%s%s: " % \
                                      (self.name, pos))
            for s in self.streams:
                #s.write("'"+txt+"'")
                s.write(txt)
                s.flush()
            self.bol = (len(txt) > 0 and txt[-1] == '\n');
            # note: doesn't handle \n in the middle of a string

        except Exception, e:
            # should not happen
            sys.__stderr__.write(str(e) + " -- " + txt + "\n")

    def write(self, txt):
        self.wwrite(txt, inspect.currentframe().f_back)

    def writelines(self, lines):
        fb = inspect.currentframe().f_back
        for l in lines:
            self.wwrite(l, fb)

    def flush(self):
        for s in self.streams:
            s.flush()

    # fake remaining methods to make this class behave like a file
    #def __getattr__(self, name):
#        return getattr(self.streams[0], name)
    
#    def __setattr__(self, name, value):
#        if name in dir(self):
#            self.__dict__[name] = value
#        else:
#            setattr(self.streams[0], name, value)
       
    def __del__(self):
        pass
        #print >> sys.__stdout__,  "Die", self.name
