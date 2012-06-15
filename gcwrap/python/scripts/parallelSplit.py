from parallel_go import cluster

class commandBuilder:
    '''
    Simple class to help with the building of the long strings needed to
    execute a command.
    '''
    def __init__(self, commandName):
        # TODO: Ensure that commandName is a string
        self._commandName = commandName
        self._argumentList = {}

    def setArgument(self, argument, value):
        # TODO: Ensure that the argumen is a string
        self._argumentList[argument] = value

    def getCommandString(self):
        firstArgument = True
        output = self._commandName + '('
        for (arg,value) in self._argumentList.items():
            if firstArgument:
                firstArgument = False
            else:
                output += ', '
            if isinstance(value, str):
                output += ("%s = '%s'" % (arg, value))
            else:
                output += ("%s = " % arg) + str(value)
        output += ')'
        return output

class JobData:
    def __init__(self, commandLine, jobInfo = None):
        self.commandLine = commandLine
        self.jobInfo = jobInfo
        self.completed = False
        self.jobId     = None
        self.engineId  = None


class JobQueueManager:
    def __init__(self):
        self.__pendingQueue = []
        self.__executingQueue = []
        self.__completedQueue = []

    def addJob(self, jobData):
        # TODO Check the type of jobData
        self.__pendingQueue.append(jobData)

    def getCompletedJobs(self):
        completedJobs = []
        for job in self.__completedQueue:
            if job.completed:
                completedJobs.append(job)
        return completedJobs

    def getUncompletedJobs(self):
        uncompletedJobs = []
        for job in self.__completedQueue:
            if not job.completed:
                uncompletedJobs.append(job)
        return uncompletedJobs
        
    def getAllJobs(self):
        return self.__completedQueue()

    def executeQueue(self, cluster):
	print "pendingQueue"
	print __pendingQueue
        self.__pendingQueue.reverse()
        engineList = cluster.get_ids()

        while len(self.__pendingQueue) > 0:
            executingJobList = self.__executingQueue
            for jd in executingJobList:
                try:
                    if cluster.check_job(jd.jobId, False):
                        jd.completed = True
                        self.__completedQueue.append(jd)
                        self.__executingQueue.remove(jd)
                        engineList.append(jd.engineId)
                        print "Completed: " + jd.commandLine
                except:
                    # Something went wrong with this job
                    print "Problem Executing: " + jd.commandLine
                    self.__completedQueue.append(jd)
                    self.__executingQueue.remove(jd)
                    # This is crappy but it looks like the engine dies.
                    #engineList.append(jd.engineId)
            
            while len(engineList) > 0 and len(self.__pendingQueue) > 0:
                jd = self.__pendingQueue.pop()
                print "Starting: " + jd.commandLine
                jd.engineId = engineList.pop()
                jd.jobId = cluster.odo(jd.commandLine, jd.engineId)
                self.__executingQueue.append(jd)

            time.sleep(1)

        while len(self.__executingQueue) > 0:
            executingJobList = self.__executingQueue
            for jd in executingJobList:
                try:
                    if cluster.check_job(jd.jobId, False):
                        jd.completed = True
                except:
                    # Something went wrong with this job
                    print "Problem Executing: " + jd.commandLine
                    pass
                finally:
                    self.__completedQueue.append(jd)
                    self.__executingQueue.remove(jd)
                    engineList.append(jd.engineId)
            time.sleep(1)


from collections import namedtuple
import os
class MultiMS:
    MSEntry = namedtuple('MSEntry', 'use')
    BoilerPlate ='''#!MultiMS Specification File: Version 1.0
# This file includes the specification for a multiMS for reduction using CASA
# Please do not modify the first line of this file.
# Any line's beginning with a '#' are treated as comments
# Processing is disabled for MS definition lines beginning with a '!'
# All paths are considered relative to the location of this specification file
''' 
    def __init__(self, mmsName, mmsSpec=None, initialize = True):
        self.__mmsName = mmsName
        self.__entryDict={}
        if initialize:
            self.readSpec(mmsSpec)

    def readSpec(self, mmsSpec = None):
        fd = open(self.__createSpecFilename(mmsSpec), 'r')
        self.__getVersion(fd)
        for line in fd.readlines():
            if line[0] == '#':
                continue
            if line[0] == '!':
                self.addMS(line[1:].rstrip(), False)
            else:
                self.addMS(line.rstrip(), True)
        fd.close()
        print self.__entryDict


    def writeSpec(self, mmsSpec = None):
        fd = open(self.__createSpecFilename(mmsSpec), 'w')
        fd.write(MultiMS.BoilerPlate)
        for path in self.__getSortedEntries():
            if not self.__entryDict[path].use:
                fd.write('!')
            fd.write(path +'\n')
        fd.close()

    def show(self):
        pass

    def addMS(self, msName, use=True):
        '''
        This method adds the specified MS to the MultiMS, the MS Name must
        be unique or an exception in thrown.
        '''
        if msName in self.__entryDict:
            # TODO raise an exception for duplicate entries
            pass
        self.__entryDict[msName] = MultiMS.MSEntry(use)

    def rmMS(self, msName):
        if msName not in self._entryDict:
            # TODO raise an exception for non-existant entry
            pass
        self.__entryDict.pop(msName)

    def getMSPathList(self):
        pathList = []
        for entryKey in self.__getSortedEntries():
            if self.__entryDict[entryKey].use:
                pathList.append(self.__createPath(entryKey))
        return pathList
                       
    def __getSortedEntries(self):
        entryKeys = self.__entryDict.keys()
        entryKeys.sort()
        return entryKeys
        
    def __createSpecFilename(self, mmsSpec):
        if mmsSpec is None:
            mmsSpec = self.__mmsName.replace('.mms', '.mmsSpec')  
        return self.__createPath(mmsSpec)

    def __createPath(self, filename):
        return "%s/%s" % (self.__mmsName, filename)

    def __getVersion(self, fileDescriptor):
        pass



# Actual script code
# c = cluster()
# c.start_engine("sopris", 2, '/Users/jkern/Data/EVLA/TDEM0003')

# option_ms='day2_TDEM0003_10s_norx.ms'
# option_mmsName = 'TDEM0003.mms'
# option_mmsSpec = None


# # Check that the directory exists
# # build the 
# if not os.path.exists(option_mmsName):
#     os.mkdir(option_mmsName)

# msFileRoot = option_mmsName.replace('.mms','')
# visfileroot = "%s/%s" % (option_mmsName, option_mmsName.replace('.mms',''))

# command = commandBuilder("split")
# command.setArgument('vis', option_ms)
                   

 
# jobManager = JobQueueManager()
# for scan in range(5,10):
#     command.setArgument('scan',str(scan))
#     outputVis = msFileRoot + ('.%05d.ms' % scan)
#     command.setArgument('outputvis', "%s/%s" % (option_mmsName,outputVis))
#     jobManager.addJob(JobData(command.getCommandString(), outputVis))

# jobManager.executeQueue(cluster)

# mms = MultiMS(option_mmsName, option_mmsSpec, False)
# for job in jobManager.getCompletedJobs():
#     mms.addMS(job.jobInfo)

# mms.writeFile()

#- Testing read
#mms = MultiMS(option_mmsName, option_mmsSpec)
#print mms.getMSPathList()

