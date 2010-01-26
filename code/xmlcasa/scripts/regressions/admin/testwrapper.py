""" Class to wrap a test to use with unittest framework.
    The test needs to contain two functions: data() and run()"""

import os
import commands
import sys
import shutil
import inspect
import re
import string
import traceback
import unittest
#import nose


AIPS_DIR = os.environ["CASAPATH"].split()[0]
DATA_DIR = AIPS_DIR+'/data'

SCRIPT_REPOS=AIPS_DIR+'/code/xmlcasa/scripts/regressions/'
UTILS_DIR = AIPS_DIR+'/code/xmlcasa/scripts/regressions/admin/'
if not os.access(SCRIPT_REPOS, os.F_OK):
    if os.access(AIPS_DIR+'/lib64', os.F_OK):
        SCRIPT_REPOS = AIPS_DIR+'/lib64/python2.5/regressions/'
        UTILS_DIR = AIPS_DIR+'/lib64/casapy/bin/'
    elif os.access(AIPS_DIR+'/lib', os.F_OK):
        SCRIPT_REPOS = AIPS_DIR+'/lib/python2.5/regressions/'
        UTILS_DIR = AIPS_DIR+'/lib/casapy/bin/'        
    else:            #Mac release
        SCRIPT_REPOS = AIPS_DIR+'/Resources/python/regressions/'
        UTILS_DIR = AIPS_DIR+'/MacOS/'        

class UnitTest:
    def __init__(self,testname=''):
        """Take the name of a test file (without .py), wrap it and run"""
#        if (testname == ''):
#            raise Exception, "Null test name"
#            raise Error
        
        self.testname = testname
        self.test = testname+'.py'
        self.workdir = testname+'_work'
        self.scriptdir = SCRIPT_REPOS       
        self.datadir = [DATA_DIR]
    
    def runFuncTest(self):
        print '-------------- Unit Test for %s ---------------'%self.testname
        """Wrap and run a script using unittest and nose"""
        testscript = self.searchscript(self.testname)
        
        # avoid creating a _work directory
        if (testscript == ""):
            return
        
        # create a working directory
        self.cleanup()
        self.createDir()
        
        # copy test to workdir
        self.getTest(testscript, self.testname)
        thisDir = os.getcwd()
        os.chdir(self.workdir)       
                
        # import the test
        mytest = __import__(self.testname)
        reload(mytest)
        os.chdir(thisDir)
        
        #get the data
        dataFiles = mytest.data()
        
        print 'Searching for input data in %s'%(self.datadir)
        for datafile in dataFiles: 
            file = self.locatedata(datafile)
            #if data already exist, remove them
            if(file != ''):
                os.system('rm -rf '+ self.workdir+'/'+datafile)
            if(os.path.isdir(file)):
                shutil.copytree(file, self.workdir+'/'+datafile)
            if(os.path.isfile(file)):
                shutil.copy(file, self.workdir+'/'+datafile)

        # Wrap the test in a FunctionTestCase and return it
        os.chdir(self.workdir)       
        testcase = unittest.FunctionTestCase(mytest.run)
#        unittest.TextTestRunner(verbosity=2).run(testcase)
#        nose.core.TextTestRunner(verbosity=2).run(testcase)
        return testcase
    
#        os.chdir(thisDir)
       
    def runTest(self):    
        print '-------------- Unit Test for %s ---------------'%self.testname
        """Run a unittest script using nose"""    
        testscript = self.searchscript(self.testname)
        
        # create a working directory
        self.cleanup()
        self.createDir()
        
        # copy test to workdir
        self.getTest(testscript, self.testname)
        thisDir = os.getcwd()
        os.chdir(self.workdir)       
                
        # import the test
        mytest = __import__(self.testname)
        reload(mytest)
              
        #Create a suite and run it.     
#        suite = mytest.suite()
#        nose.core.TextTestRunner(verbosity=2).run(suite)

        # Return the tests of this test module
        classes = mytest.suite()
        tests = []
        
#        print classes
        for c in classes:
            for attr, value in c.__dict__.iteritems():
#                print attr, " = ", value
                if len(attr) >= len("test") and \
                   attr[:len("test")] == "test":
                    tests.append(c(attr))

        return tests

        
        
        # clean up
        os.chdir(thisDir)
#        self.cleanup()


    def cleanup(self):
        # for safety, avoid removing the local directory
        if (self.workdir == '.'):
            self.workdir = '/tmp/utests'
        
        if os.path.isdir(self.workdir):
            print 'Cleaning up '+ self.workdir
            shutil.rmtree(self.workdir)


    def createDir(self):
        """Create a working directory"""
        if os.access(self.workdir, os.F_OK) is False:
            print self.workdir+' does not exist, creating it'
            os.makedirs(self.workdir)


    def locatedata(self, datafile):
        
        for repository in self.datadir :

            #Skip hidden directories
            filter_hidden = ' | grep -vE "^\\."  | grep -vE "/\\."'
            
            #See if find understands -L or -follow (depends on find version)
            (err, a) = commands.getstatusoutput('find -L ' + repository+'/ 1>/dev/null 2>&1')
            if not err:
                findstr='find -L '+repository+'/ -name '+datafile+' -print 2>/dev/null' + filter_hidden
            else:
                findstr='find '+repository+'/ -follow -name '+datafile+' -print 2>/dev/null' + filter_hidden
            # A '/' is appended to the directory name; otherwise sometimes find doesn't find.
            #Also, ignore error messages such as missing directory permissions
            
            (find_errorcode, a)=commands.getstatusoutput(findstr)   # stdout and stderr
            #if find_errorcode != 0:
            #    print >> sys.stderr, "%s failed: %s" % (findstr, a)
            retval=''
            b=['']
            if(a!=''):
                b=string.split(a, '\n')
                retval=b[len(b)-1]
                if(len(b) > 1):
                    print 'More than 1 file found with name '+datafile
                print 'Will use', retval
                return retval
        raise Exception, 'Could not find datafile %s in the repository directories %s' \
              % (datafile, self.datadir)
 

    def searchscript(self,  testname):
        """Search for the script"""
        print "Searching for script %s in %s" %(testname,self.scriptdir)                  
        scriptdir=self.scriptdir
        testName=string.lower(testname)

        # search for DIR/tests/<name>.py
        if os.path.isdir(scriptdir+'/tests/'):
            allScripts=os.listdir(scriptdir+'/tests/')
        else:
            allScripts=[]
#        print "allScripts = ", allScripts
        theScript=''
        numOfScript=0
        for scr in allScripts :
            #if(string.find(scr,testname)>=0):
            if(scr == testname+'.py'):
#                print scr, testname
                #if (self.ispythonscript(scr)):
                theScript = 'tests/'+scr
                numOfScript +=1

        # search for DIR/<name>.py
        if os.path.isdir(scriptdir):
            allScripts=os.listdir(scriptdir)
        else:
            allScripts=[]
#        print "allScripts = ", allScripts
        for scr in allScripts:
            #print scriptdir, scr, testname
            if (scr == testname + '.py'):
                theScript = scr
                numOfScript += 1  
                      
        if numOfScript == 0:
            raise Exception, 'Could not find test %s' %testname       
            
        if( numOfScript > 1) :
            print 'More than 1 scripts found for name '+testname
            print 'Using the following one '+ theScript
            
        print "Found", theScript
        return theScript

    def getTest(self, testnamek, testName):
        #Copy the script to the working dir
        if testnamek[0:6] == 'tests/':
            shutil.copy(self.scriptdir+'/'+testnamek,
                        self.workdir+'/'+testName+'.py')
        else:    
            shutil.copy(self.scriptdir+'/'+testnamek, \
                    self.workdir+'/')




        