###Base class for casa test scripts
###More complicated tests may inherit from this
import os
import string
import commands
import shutil
import sys
from  casac import *
import inspect
import re
#from tasks import *  # execfile
#from taskinit import casalog

ia=casac.image()
ms=casac.ms()
tb=casac.table()

class testbase :
    def __init__(self, workdir=None):
        if workdir == None:
            workdir=os.getcwd()
        self.dataBaseDirectory=[workdir+'/Data']
        self.resultDirectory=workdir+'/Results'
        self.workingDirectory=workdir
        self.scriptRepository=workdir+'/Scripts'
        self.testsToRun=[]
        self.testList={}
        self.notest=False

    def createDirs(self):
        if os.access(self.resultDirectory, os.F_OK) is False:
            print self.resultDirectory+' does not exist, creating it'
            os.makedirs(self.resultDirectory)
        if os.access(self.workingDirectory, os.F_OK) is False:
            print self.workingDirectory+' does not exist, creating it'
            os.makedirs(self.workingDirectory)
            
    def setDataBaseDir(self, dir=['./Data']):
        if type(dir) != type(["directory", "list"]):
            raise TypeError, type(dir)
        self.dataBaseDirectory=dir

    def setResultDir(self, dir='./Results'):
        self.resultDirectory=dir

    def setWorkingDir(self, dir='./Temporaire'):
        self.workingDirectory=dir
        
    def setScriptsDir(self, dir='./Scripts'):
        self.scriptRepository=dir

    def cleanup(self):
        if os.path.isdir(self.workingDirectory):
            #print 'Removing '+ self.workingDirectory
            shutil.rmtree(self.workingDirectory)

    def locateTests(self, tests=None):
        theF=[]
        theFiles=[]
        if tests is None :
            # This does not work, need to search also for DIR/name.py
            # Also, searching in .../tests/<name>/regression.py is
            # obsolete
            raise Exception, "Unsupported!"
        
            theFiles=os.listdir(self.scriptRepository)
            tempy=[]
            ###locate only the *.py files
            for leF in theFiles:
                if os.path.isdir(self.scriptRepository + '/tests/' + leF) and \
                   os.path.isfile(self.scriptRepository + '/tests/' + leF + '/regression.py'):
                    tempy.append(leF + '/regression.py')
            theFiles=tempy
        else:
            for leF in tests :
                leScript=self.searchscript(leF)
                if(leScript != ''):
                   theFiles.append(leScript)

        self.testsToRun=[]
        for k  in range(len(theFiles)):
            #self.testsToRun.append(string.split(theFiles[k],'.py')[0])
            self.testsToRun.append(theFiles[k])          
        print "tests to run = ", self.testsToRun
        return len(self.testsToRun)

    def getTest(self, testnamek, testName):
        #Copy the script to the working dir
        if testnamek[0:6] == 'tests/':
            shutil.copy(self.scriptRepository+'/'+testnamek,
                        self.workingDirectory+'/'+testName+'.py')
        shutil.copy(self.scriptRepository+'/'+testnamek, \
                    self.workingDirectory+'/')
    
    def testname(self, ind=0):
        if(len(self.testsToRun)==0):
            print 'No tests defined yet'
            return ''
        return self.testsToRun[ind]

    def getDescription(self, testName, testId):
        leFile=self.testsToRun[testId]

        if leFile[0:6] == 'tests/':
            leTest = __import__(testName)
            reload(leTest)
            try:
                # Fails if module does not
                # define the description() function
                desc = leTest.description()
                desc = re.sub('\s+', ' ', desc)

            except:
                desc = None
        else:
            desc = None
        return desc
            
    def runtests(self, testName, testId=0, dry=False):
        try:
            leFile=self.testsToRun[testId]

            if leFile[0:6] == 'tests/':
                print "Import", leFile
                leTest = __import__(testName)
                #print "  imported"
                allData=leTest.data()
                # check if there is a doCopy function and use it if possible
                try:
                    doCopy=leTest.doCopy()
                    print doCopy
                    if (len(doCopy) != len(allData)):
                        doCopy=[-1]
                        print 'Error: doCopy function supplied list of incorrect length.'
                    else:
                        print 'Using doCopy function to determine whether to copy or link input data.'
                except:
                    doCopy=[-1]
                    
                print "Required data =", allData
                for leIndex, leData in enumerate(allData) : 
                    theData=self.locatedata(leData)
                    if(theData != ''):
                        os.system('rm -rf '+ self.workingDirectory+'/'+leData)
                        if(os.path.isdir(theData)):
                            if(doCopy[0] == -1 or doCopy[leIndex] != 0):
                                shutil.copytree(theData, self.workingDirectory+'/'+leData)
                            else:
                                os.system('ln -sf '+theData+' '+self.workingDirectory+'/'+leData)
                        if(os.path.isfile(theData)):
                            if(doCopy[0] == -1 or doCopy[leIndex] != 0):
                                shutil.copy(theData, self.workingDirectory+'/'+leData)
                            else:
                                os.system('ln -sf '+theData+' '+self.workingDirectory+'/'+leData)
                if not dry:
                    theImages=leTest.run()
                else:
                    # ngc5921redux
                    theImages = ['ngc5921_regression/ngc5921.clean.image']

                    # h121
                    theImages =  ['nrao150.3mm.image', 'h121.co10.image',  '0224b.3mm.image', 'h121b.co10.image', 'h121all.3mm.image', 'h121c.co10.image' ]


                del leTest

                if type(theImages) == None:
                    raise Exception, "Illegal return value from run()"                   
                    
                leResult=[]
                for leImage in theImages :
                    leResult.append(self.workingDirectory+'/'+leImage)
                self.defineQualityTestList(leResult)
                return leResult, theImages
            else:
                print "execfile", leFile
                if dry:
                    return [], []
                a=inspect.stack()
                stacklevel=0
                for k in range(len(a)):
                    if (string.find(a[k][1], 'ipython console') > 0):
                        stacklevel=k
                        break
                gl=sys._getframe(stacklevel).f_globals

                # Execute the test from a generated exec-<test>.py
                # which does the following
                #    regstate=True
                #    execfile(leFile)
                #    if not regstate:
                #        raise Exception ...
                #
                # This is a workaround: Changes in regstate
                # do not propagate to here if the regression
                # script is run directly with execfile() from
                # here. But an exeption propagates
                
                fd=open('exec-'+leFile, 'w')
                print >> fd, "regstate=True"   # used in regression scripts to signal error
                print >> fd, "execfile('"+leFile+"')"
                print >> fd, "print 'regstate =', regstate"
                print >> fd, "if not regstate:"
                print >> fd, "    raise Exception, 'regstate = False'"
                fd.close()

                # What we really want, but regstate doesn't propagate:
                # execfile(leFile, gl)
                execfile('./exec-'+leFile, gl)
                
                return [], []   # no product images known
        except:
            self.notest=True
            #print >> sys.stderr, "Error running test:", sys.exc_info()[0]
            raise
            
    def defineQualityTestList(self,theResult):
        self.testList.clear()
        for k in range(len(theResult)) :
            if(os.path.isdir(theResult[k])):
                # previous developer said:
                # Good chance its an image for now till i know whether its a caltable or ms
                # actually check for ms, if not assume image
                tb.open(theResult[k])
                tabkwords=tb.keywordnames()
                tb.done()
                if('MS_VERSION' in tabkwords):
                    ms.open(theResult[k])
                    self.testList[theResult[k]]=[]
                    self.testList[theResult[k]].append('ms')
                    ms.done()
                elif('coords' in tabkwords):
                    ## table is an image                    
                    self.testList[theResult[k]]=[]
                    self.testList[theResult[k]].append('simple')
                    ia.open(theResult[k])
                    shp=ia.shape()
                    ## Moment images do not have a spectral axis...
                    # much assumes the "canonical" CASA axis order.
                    # imageTest will adddegaxes to any im it opens (2011/12/15)
                    mycs=ia.coordsys()
                    findstok=mycs.findcoordinate("stokes")
                    findspec=mycs.findcoordinate("spectral")
                    if findspec['return']:                        
                        spix=findspec['pixel']
                        if(shp[spix]>5):
                            print 'spectral shape= ', shp[spix],' ', theResult[k], k 
                            self.testList[theResult[k]].append('cube')
                    if findstok['return']:
                        kpix=findstok['pixel']
                        if(shp[kpix]==1):
                            self.testList[theResult[k]].append('pol1')
                        elif( shp[kpix]==2):
                            self.testList[theResult[k]].append('pol2')
                        elif(shp[kpix]==4):
                            self.testList[theResult[k]].append('pol4')
                    ia.close()                
            
    def whatQualityTest(self):
        if(self.notest):
            return []
        return self.testList
    
    def searchscript(self,  testname):
        scriptdir=self.scriptRepository
        print "searching for script", testname, "in ",scriptdir
        testName=string.lower(testname)

        # search for DIR/tests/<name>.py
        if os.path.isdir(scriptdir+'/tests/'):
            allScripts=os.listdir(scriptdir+'/tests/')
        else:
            allScripts=[]
        print "allScripts = ", allScripts
        theScript=''
        numOfScript=0
        for scr in allScripts :
            #if(string.find(scr,testname)>=0):
            if(scr == testname+'.py'):
                print scr, testname
                #if (self.ispythonscript(scr)):
                theScript = 'tests/'+scr
                numOfScript +=1

        # search for DIR/<name>.py
        if os.path.isdir(scriptdir):
            allScripts=os.listdir(scriptdir)
        else:
            allScripts=[]
        print "allScripts = ", allScripts
        for scr in allScripts:
            #print scriptdir, scr, testname
            if (scr == testname + '.py'):
                theScript = scr
                numOfScript += 1             
        if numOfScript == 0:
            raise Exception("Could not find test %s" % testname)
        if( numOfScript > 1) :
            print 'More than 1 scripts found for name '+testname
            print 'Using the following one '+ theScript
        print "Found", theScript
        return theScript

    def ispythonscript(self, thescript):
        if(string.find(thescript,'.py', len(thescript)-3) > 0):
            return True
        else:
            return False

    def locatedata(self, datafile):
        for repository in self.dataBaseDirectory :

            # Skip hidden directories
            filter_hidden = ' | grep -vE "^\\."  | grep -vE "/\\."'
            
            # See if find understands -L or -follow (depends on find version)
            (err, a) = commands.getstatusoutput('find -L ' + repository+'/ 1>/dev/null 2>&1')
            if not err:
                findstr='find -L '+repository+'/ -name '+datafile+' -print 2>/dev/null' + filter_hidden
            else:
                findstr='find '+repository+'/ -follow -name '+datafile+' -print 2>/dev/null' + filter_hidden
            # A '/' is appended to the directory name; otherwise sometimes find doesn't find.
            # Also, ignore error messages such as missing directory permissions
            
            (find_errorcode, a)=commands.getstatusoutput(findstr)   # stdout and stderr
            if find_errorcode > 0 and not a:
                a = self.python_find(datafile, repository)
                print "*** python found",a
            #if find_errorcode != 0:
            #    print >> sys.stderr, "%s failed: %s" % (findstr, a)
            retval=''
            b=['']
            if(a!=''):
                b=string.split(a, '\n')
                retval=b[len(b)-1]
                if(len(b) > 1):
                    print 'more than 1 file found with name '+datafile
                print 'will use', retval
                return retval
        raise Exception, 'Could not find datafile %s in the repository directories %s' \
              % (datafile, self.dataBaseDirectory)

    def python_find(self, name, path):
        # http://stackoverflow.com/questions/1724693/find-a-file-in-python
        ret = ""
        count = 0
        for root, dirs, files in os.walk(path):
            if name in files:
                if count > 0:
                    ret += "\n"
                ret += os.path.join(root, name)
        return ret
