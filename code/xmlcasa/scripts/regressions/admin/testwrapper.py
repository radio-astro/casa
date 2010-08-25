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

PYVER = str(sys.version_info[0]) + "." + str(sys.version_info[1])

AIPS_DIR = os.environ["CASAPATH"].split()[0]
DATA_DIR = AIPS_DIR+'/data'

SCRIPT_REPOS=AIPS_DIR + "/" + os.environ["CASAPATH"].split()[1] + '/python/' + PYVER + '/tests/'

if not os.access(SCRIPT_REPOS, os.F_OK):
    if os.access(AIPS_DIR+'/lib64', os.F_OK):
        SCRIPT_REPOS = AIPS_DIR+'/lib64/python' + PYVER + '/tests/'
    elif os.access(AIPS_DIR+'/lib', os.F_OK):
        SCRIPT_REPOS = AIPS_DIR+'/lib/python' + PYVER + '/tests/'
    else:            #Mac release
        SCRIPT_REPOS = AIPS_DIR+'/Resources/python/tests/'

class Helper():
    # This class is called when a test is not found. It will
    # raise an exception and make nose fail. This way, the not
    # found test will be counted as an error and won't be ignored.
    def __init__(self, name):
        self.tname = name
    
    def test_dummy(self):
        '''Helper function'''
        raise Exception, "Cannot find test %s"%self.tname

class UnitTest:
    def __init__(self,testname=''):
        """Take the name of a test file (without .py), wrap it and run"""
        self.testname = testname
        self.workdir = testname+'_work'
        self.scriptdir = SCRIPT_REPOS       
        self.datadir = [DATA_DIR]
        self.dataFiles = []
    
    def funcdesc(self):
        '''Name of test for FunctionTestCase'''
        return 'Test '+self.testname
    
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
                                              tearDown=self.funcTeardown,
                                              description=self.funcdesc()))
        
        return testcase


    def getUnitTest(self,list=[]):
        """Set up a unit test script to run wit nose"""    
        print '-------------- Unit Test for %s ---------------'%self.testname
        if list:
            print 'List of specific tests %s'%(list)
            
        # search for script in repository
        testscript = self.searchscript(self.testname, self.scriptdir)

        if (testscript == ""):
            testlist = []
            # Create a dummy list and return it so that nose
            # includes this test in the list of erroneous tests
            # instead of ignoring it.
            t = unittest.FunctionTestCase(Helper(self.testname).test_dummy)
            return [t]

        # copy test to local directory
        self.workdir = os.getcwd()
        self.getTest(testscript, self.testname,self.scriptdir, self.workdir)

        # import the test
        mytest = __import__(self.testname)
        reload(mytest)
        
        # get the classes
        classes = mytest.suite()
        testlist = []

        for c in classes:
            for attr, value in c.__dict__.iteritems():
                if list:
#                    print 'There is a list'
                    for test in list:
                        if test == attr:
                            testlist.append(c(attr))
                else:
#                    print attr, " = ", value
                   if len(attr) >= len("test") and \
                        attr[:len("test")] == "test" : \
#                        attr.rfind('test') != -1 :
                        testlist.append(c(attr))
            
        return testlist
            

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
#            raise Exception, 'Could not find test %s' %TestName 
            print 'ERROR: Could not find test %s' %TestName
            return ""  
            
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
        

