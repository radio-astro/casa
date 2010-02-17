""" Class to wrap a test to use with unittest framework."""

import os
import commands
import sys
import shutil
import inspect
import re
import string
import traceback
import casac
import unittest

AIPS_DIR = os.environ["CASAPATH"].split()[0]
DATA_DIR = AIPS_DIR+'/data'

SCRIPT_REPOS=AIPS_DIR+'/code/xmlcasa/scripts/tests/'
UTILS_DIR = AIPS_DIR+'/code/xmlcasa/scripts/regressions/admin/'
if not os.access(SCRIPT_REPOS, os.F_OK):
    if os.access(AIPS_DIR+'/lib64', os.F_OK):
        SCRIPT_REPOS = AIPS_DIR+'/lib64/python2.5/tests/'
        UTILS_DIR = SCRIPT_REPOS+'admin'
    elif os.access(AIPS_DIR+'/lib', os.F_OK):
        SCRIPT_REPOS = AIPS_DIR+'/lib/python2.5/tests/'
        UTILS_DIR = SCRIPT_REPOS+'admin'
    else:            #Mac release
        SCRIPT_REPOS = AIPS_DIR+'/Resources/python/tests/'
        UTILS_DIR = SCRIPT_REPOS+'admin'        

sys.path.append(UTILS_DIR)

class UnitTest:
    def __init__(self,testname=''):
        """Take the name of a test file (without .py), wrap it and run"""
        self.testname = testname
        self.workdir = testname+'_work'
        self.scriptdir = SCRIPT_REPOS       
        self.datadir = [DATA_DIR]
        self.dataFiles = []

        
    def runFuncTest(self):
        # DEPRECATED, replaced by getFuncTest
        print '-------------- Unit Test for %s ---------------'%self.testname
        """Wrap a script using unittest"""
        testscript = self.searchscript(self.testname, self.scriptdir)
        
        # avoid creating a _work directory
        if (testscript == ""):
            return
        
        # create a working directory
        self.cleanup(self.workdir)
        self.createDir(self.workdir)
        
        # copy test to workdir
        self.getTest(testscript, self.testname, self.scriptdir, self.workdir)
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
            file = self.locatedata(datafile, self.datadir)
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

        return testcase
    
    def funcSetup(self):        
        """Copy data files to local working directory"""
        
        dataFiles = self.dataFiles
        print 'Searching for input data in %s'%(self.datadir)
        for datafile in dataFiles: 
            file = self.locatedata(datafile, self.datadir)
            #if data already exist, remove them
            if(file != ''):
                os.system('rm -rf '+ self.workdir+'/'+datafile)
            if(os.path.isdir(file)):
                shutil.copytree(file, self.workdir+'/'+datafile)
            if(os.path.isfile(file)):
                shutil.copy(file, self.workdir+'/'+datafile)
        
    def funcTeardown(self):
        """Remove data files from working directory"""
        
        dataFiles = self.dataFiles
        for datafile in dataFiles:
            file = self.workdir+'/'+datafile
            os.system('rm -rf ' + file)
        
        self.dataFiles = []

    def getFuncTest(self):
        print '-------------- Unit Test for %s ---------------'%self.testname
        """Wrap a script using unittest"""
        testscript = self.searchscript(self.testname, self.scriptdir)
        
        # avoid creating a _work directory
        if (testscript == ""):
            return

        # copy test to local directory
        self.workdir = os.getcwd()
        self.getTest(testscript, self.testname,self.scriptdir, self.workdir)

        # import the test
        mytest = __import__(self.testname)
        reload(mytest)

        #get the data
        try:
            self.dataFiles = mytest.data()
        except:
            print 'No data needed or found'
              
        # Wrap the test, funcSetup and funcTeardown in a FunctionTestCase and return it
        testcase = (unittest.FunctionTestCase(mytest.run,setUp=self.funcSetup,
                                              tearDown=self.funcTeardown))
        
        return testcase

    def runTest(self):    
                # DEPRECATED, replaced by getUnitTest
        print '-------------- Unit Test for %s ---------------'%self.testname
        """Set up a unit test script to run wit nose"""    
        testscript = self.searchscript(self.testname, self.scriptdir)

        # avoid creating a _work directory
        if (testscript == ""):
            return
        
        # create a working directory
        self.cleanup(self.workdir)
        self.createDir(self.workdir)
        
        # copy test to workdir
        self.getTest(testscript, self.testname,self.scriptdir, self.workdir)
        thisDir = os.getcwd()
        os.chdir(self.workdir)       
                
        # import the test
        mytest = __import__(self.testname)
        reload(mytest)
              
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

    def getUnitTest(self):
        print '-------------- Unit Test for %s ---------------'%self.testname
        """Set up a unit test script to run wit nose"""    
        testscript = self.searchscript(self.testname, self.scriptdir)

        # avoid creating a _work directory
        if (testscript == ""):
            return

        # copy test to local directory
        self.workdir = os.getcwd()
        self.getTest(testscript, self.testname,self.scriptdir, self.workdir)

        # import the test
        mytest = __import__(self.testname)
        reload(mytest)
              
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
        

    def cleanup(self,workdir):
        # for safety, avoid removing the local directory
        if (workdir == '.'):
            workdir = '/tmp/utests'
        
        if os.path.isdir(workdir):
            print 'Cleaning up '+ workdir
            shutil.rmtree(workdir)


    def createDir(self, workdir):
        """Create a working directory"""
        if os.access(workdir, os.F_OK) is False:
            print workdir+' does not exist, creating it'
            os.makedirs(workdir)


    def locatedata(self, datafile, datadir):
        
        for repository in datadir :

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
              % (datafile, datadir)
 

    def searchscript(self, testname, scriptdir):
        """Search for the script"""
        print "Searching for script %s in %s" %(testname,scriptdir)                  
#        scriptdir=self.scriptdir
        TestName=string.lower(testname)
#        print 'testname='+testname

#        # search for DIR/tests/<name>.py
#        if os.path.isdir(scriptdir+'/tests/'):
#            allScripts=os.listdir(scriptdir+'/tests/')
#        else:
#            allScripts=[]
##        print "allScripts = ", allScripts
        theScript=''
        numOfScript=0
#        for scr in allScripts :
##            print 'scr='+scr
#            #if(string.find(scr,testname)>=0):
#            if(scr == TestName+'.py'):
##                print scr, testname
#                #if (self.ispythonscript(scr)):
#                theScript = 'tests/'+scr
#                numOfScript +=1

        # search for DIR/<name>.py
        if os.path.isdir(scriptdir):
            allScripts=os.listdir(scriptdir)
        else:
            allScripts=[]
#        print "allScripts = ", allScripts
        for scr in allScripts:
#            print scriptdir, scr, testname
            if (scr == TestName + '.py'):
                theScript = scr
                numOfScript += 1  
                      
        if numOfScript == 0:
            raise Exception, 'Could not find test %s' %TestName       
            
        if( numOfScript > 1) :
            print 'More than 1 scripts found for name '+TestName
            print 'Using the following one '+ theScript
            
        print "Found", theScript
        return theScript

    def getTest(self, testnamek, testName, scriptdir, workdir):
        print 'Copy the script to the working dir'
        if testnamek[0:6] == 'tests/':
            shutil.copy(scriptdir+'/'+testnamek,
                        workdir+'/'+testName+'.py')
        else:    
            shutil.copy(scriptdir+'/'+testnamek, \
                    workdir+'/')


class ExecTest(unittest.TestCase,UnitTest):
    """Wraps scripts to run with execfile"""
    
    def setup(self):
        self.workdir = self.testname+'_work'
        self.scriptdir = SCRIPT_REPOS       
        
        self.testscript = self.searchscript(self.testname,self.scriptdir)
        # avoid creating a _work directory
        if (self.testscript == ""):
            return
        
        # create a working directory
        self.cleanup(self.workdir)
        self.createDir(self.workdir)
        
        # copy test to workdir
        self.getTest(self.testscript, self.testname, self.scriptdir, self.workdir)
        thisDir = os.getcwd()
        os.chdir(self.workdir)    
           
    def testrun(self):
        #self.testname is defined in the calling function
        # run self.setup before calling this function
        """Run test with execfile"""
        
        # run the test
        a=inspect.stack()
        stacklevel=0
        for k in range(len(a)):
            if (string.find(a[k][1], 'ipython console') > 0):
                stacklevel=k
                break
        gl=sys._getframe(stacklevel).f_globals
        
        execfile(self.testscript, gl)   
        

        