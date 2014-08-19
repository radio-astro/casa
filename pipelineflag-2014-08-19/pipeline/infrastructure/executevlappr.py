# Execute the pipeline processing request
#    Code first as module and convert to class if appropriate
#    Factor and document properly  when details worked out    
#
# Turn some print statement into CASA log statements
#

# imports
import sys
import string
#import inspect
import traceback
import os

import pipeline
import pipeline.extern.XmlObjectifier as XmlObjectifier
from pipeline.infrastructure.casataskdict import CasaTaskDict
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.project as project
import argmapper

# Make sure CASA exceptions are rethrown
try:
    default__rethrow_casa_exceptions = __rethrow_casa_exceptions=True
except Exception, e:
    default__rethrow_casa_exceptions = False
__rethrow_casa_exceptions=True

# Setup path
sys.path.insert (0, os.path.expandvars("$SCIPIPE_HEURISTICS"))

def executeppr (pprXmlFile, importonly=True, dry_run=False, loglevel='info',
    plotlevel='default', interactive=True):

    # Useful mode parameters
    echo_to_screen = interactive

    try:
        # Decode the processing request
        info, structure, relativePath, intentsDict, asdmList, procedureName, commandsList = \
            _getFirstRequest (pprXmlFile)

	# Set the directories
        workingDir = os.path.join (os.path.expandvars("$SCIPIPE_ROOTDIR"),
            relativePath, "working")
        rawDir = os.path.join (os.path.expandvars("$SCIPIPE_ROOTDIR"),
            relativePath, "rawdata")

        # Get the pipeline context 
        context = pipeline.Pipeline(loglevel=loglevel, plotlevel=plotlevel,
	    output_dir=workingDir).context

    except Exception, e:
        casatools.post_to_log ("Beginning pipeline run ...", 
            echo_to_screen=echo_to_screen)
        casatools.post_to_log ("For processing request: " + \
            pprXmlFile, echo_to_screen=echo_to_screen)
	traceback.print_exc(file=sys.stdout)
	errstr=traceback.format_exc()
        casatools.post_to_log (errstr,
	    echo_to_screen=echo_to_screen)
        casatools.post_to_log ("Terminating procedure execution ...", 
            echo_to_screen=echo_to_screen)
	return

    # Request decoded, starting run.
    casatools.post_to_log ("Beginning pipeline run ...", 
        echo_to_screen=echo_to_screen)
    casatools.post_to_log ("For processing request: " + \
        pprXmlFile, echo_to_screen=echo_to_screen)

    # Check for common error conditions.
    if relativePath == "":
        casatools.post_to_log ("    Undefined relative data path", 
            echo_to_screen=echo_to_screen)
        casatools.post_to_log ("Terminating pipeline execution ...", 
            echo_to_screen=echo_to_screen)
	return
    elif len(asdmList) < 1:
        casatools.post_to_log ("    Empty ASDM list", 
            echo_to_screen=echo_to_screen)
        casatools.post_to_log ("Terminating pipeline execution ...", 
            echo_to_screen=echo_to_screen)
	return
    elif len(commandsList) < 1:
        casatools.post_to_log ("    Empty commands list", 
            echo_to_screen=echo_to_screen)
        casatools.post_to_log ("Terminating pipeline execution ...", 
            echo_to_screen=echo_to_screen)
	return

    # List project summary information
    casatools.post_to_log ("Project summary", 
        echo_to_screen=echo_to_screen)
    for item in info:
        casatools.post_to_log ("    " + item[1][0] + item[1][1], 
            echo_to_screen=echo_to_screen)
    ds = dict(info)
    context.project_summary = project.ProjectSummary(
        proposal_code = ds['proposal_code'][1],
	proposal_title = 'unknown',
	piname = 'unknown',
	observatory = ds['observatory'][1],
	telescope = ds['telescope'][1])

    # List project structure information
    casatools.post_to_log ("Project structure", 
        echo_to_screen=echo_to_screen)
    for item in structure:
        casatools.post_to_log ("    " + item[1][0] + item[1][1], 
            echo_to_screen=echo_to_screen)

    # Print the relative path
    casatools.post_to_log ("Directory structure", 
        echo_to_screen=echo_to_screen)
    casatools.post_to_log ("    Working directory: " + workingDir, 
            echo_to_screen=echo_to_screen)
    casatools.post_to_log ("    Raw data directory: " +  rawDir,
        echo_to_screen=echo_to_screen)

    # Construct the ASDM list
    casatools.post_to_log ("Number of ASDMs: " + str(len(asdmList)), 
            echo_to_screen=echo_to_screen)
    files = []
    sessions = []
    defsession = 'session_1'
    for asdm in asdmList:
	session = defsession
	sessions.append(session)
	files.append (os.path.join(rawDir, asdm[1]))
        casatools.post_to_log ("    Session: " + session + \
	    "  ASDM: " + asdm[1], echo_to_screen=echo_to_screen)

    # Paths for all these ASDM should be the same
    #     Add check for this ?

    # Beginning execution
    casatools.post_to_log ("\nStarting procedure execution ...\n", 
        echo_to_screen=echo_to_screen)
    casatools.post_to_log ("Procedure name: " + procedureName + "\n", 
        echo_to_screen=echo_to_screen)

    # Loop over the commands
    for command in commandsList:

	# Get task name and arguments lists.
        taskname = command[0]
        task_args = command[1]
	casatools.set_log_origin(fromwhere=taskname)
        casatools.post_to_log ("Executing command ..." + \
	    taskname, echo_to_screen=echo_to_screen)

	# Search dictionary for CASA to Python task name alias.
	if taskname in CasaTaskDict:
	    taskname = CasaTaskDict[taskname]
            casatools.post_to_log ("    Using python class ..." + \
	        taskname, echo_to_screen=echo_to_screen)

	# List parameters
	for keyword, value in task_args.items():
            casatools.post_to_log ("    Parameter: " + \
	    keyword + " = "+ str(value), echo_to_screen=echo_to_screen)

	# Execute the command
	try:

	    cInputs = pipeline.tasks.__dict__[taskname].Inputs
            if taskname == 'ImportData' or taskname == 'RestoreData' or \
	        taskname == 'ALMAImportData' or taskname == 'VLAImportData':
	        task_args['vis'] = files
	        task_args['session'] = sessions
            elif taskname == 'SDImportData':
                task_args['infiles'] = files
	    
	    remapped_args = argmapper.convert_args(taskname, task_args,
	        convert_nulls=False)
	    inputs = cInputs (context, **remapped_args)
	    #inputs = cInputs (context, **command[1])
	    cTask = pipeline.tasks.__dict__[taskname]
	    task = cTask(inputs)
	    results = task.execute (dry_run=dry_run)
	    casatools.post_to_log('Results ' + str(results),
		echo_to_screen=echo_to_screen)
	    try:
	        results.accept(context)
	    except Exception, e:
		casatools.post_to_log("Error: " \
		    "Failed to update context for " + taskname,
		    echo_to_screen=echo_to_screen)
		raise

            if taskname == 'ImportData' and importonly: 
		casatools.post_to_log(
		    "Terminating execution after running " + taskname,
		    echo_to_screen=echo_to_screen)
	        break

            if taskname == 'ALMAImportData' and importonly: 
		casatools.post_to_log(
		    "Terminating execution after running " + taskname,
		    echo_to_screen=echo_to_screen)
	        break
            
            if taskname == 'VLAImportData' and importonly: 
		casatools.post_to_log(
		    "Terminating execution after running " + taskname,
		    echo_to_screen=echo_to_screen)
	        break

	except Exception, e:
	    #traceback.print_exc(file=sys.stdout)
	    errstr=traceback.format_exc()
            casatools.post_to_log (errstr,
	        echo_to_screen=echo_to_screen)
	    break

    # Save the context
    context.save()

    casatools.post_to_log ("Terminating procedure execution ...", 
        echo_to_screen=echo_to_screen)

    __rethrow_casa_exceptions = default__rethrow_casa_exceptions
    casatools.set_log_origin(fromwhere='')

    return

# Return the intents list, the ASDM list, and the processing commands
# for the first processing request. There should in general be only
# one but the schema permits more. Generalize later if necessary.
#
# TDB: Turn some print statement into CASA log statements
#

def _getFirstRequest (pprXmlFile):

    # Initialize
    info = []
    structure = []
    relativePath = ""
    intentsDict = {}
    commandsList = []
    asdmList = []

    # Turn the XML file into an object
    pprObject = _getPprObject (pprXmlFile=pprXmlFile)

    # Count the processing requests.
    numRequests = _getNumRequests(pprObject=pprObject)
    if (numRequests <= 0):
	print "Terminating execution: No valid processing requests"
        return info, relativePath, intentsDict, asdmList, commandsList
    elif (numRequests > 1):
	print "Warning: More than one processing request"
    print 'Number of processing requests: ', numRequests

    # Get brief project summary
    info = _getProjectSummary(pprObject)

    # Get project structure.
    structure = _getProjectStructure(pprObject)

    # Get the intents dictionary
    numIntents, intentsDict  = _getIntents (pprObject=pprObject,
        requestId=0, numRequests=numRequests) 
    print 'Number of intents: ', numIntents
    print 'Intents dictionary: ', intentsDict

    # Get the commands list
    procedureName, numCommands, commandsList  = _getCommands (pprObject=pprObject,
        requestId=0, numRequests=numRequests) 
    print 'Number of commands: ', numCommands
    print 'Commands list: ', commandsList

    # Count the scheduling block sets. Normally there should be only
    # one although the schema allows multiple sets. Check for this 
    # condition and process only the first.
    numSbSets = _getNumSchedBlockSets(pprObject=pprObject,
        requestId=0, numRequests=numRequests)
    if (numSbSets <= 0):
	print "Terminating execution: No valid scheduling block sets"
        return info, relativePath, intentsDict, asdmList, commandsList
    elif (numSbSets > 1):
	print "Warning: More than one scheduling block set"
    print 'Number of scheduling block sets: ', numSbSets

    # Get the ASDM list
    relativePath, numAsdms, asdmList = _getAsdmList (pprObject=pprObject,
        sbsetId=0, numSbSets=numSbSets, requestId=0,
	numRequests=numRequests)
    print 'Relative path: ', relativePath
    print 'Number of Asdms: ', numAsdms
    print 'ASDM list: ', asdmList

    return info, structure, relativePath, intentsDict, asdmList, procedureName, commandsList

# Give the path to the pipeline processing request XML file return the pipeline
# processing request object.

def _getPprObject(pprXmlFile):
    pprObject = XmlObjectifier.XmlObject (fileName=pprXmlFile)
    return pprObject

# Given the pipeline processing request object print some project summary
# information. Returns a list of tuples to preserve order (key, (prompt, value))

def _getProjectSummary(pprObject):

    ppr_summary = pprObject.SciPipeRequest.ProjectSummary
    summaryList = []
    summaryList.append (('proposal_code', ('Proposal code: ',
        ppr_summary.ProposalCode.getValue())))
    summaryList.append (('observatory', ('Observatory: ',
        ppr_summary.Observatory.getValue())))
    summaryList.append (('telescope', ('Telescope: ',
        ppr_summary.Telescope.getValue())))
    summaryList.append (('processing_site', ('Processsing site: ',
        ppr_summary.ProcessingSite.getValue())))
    summaryList.append (('operator', ('Operator: ',
        ppr_summary.Operator.getValue())))
    summaryList.append (('mode', ('Mode: ',
        ppr_summary.Mode.getValue())))

    return summaryList

# Given the pipeline processing request object print some project structure
# information. Returns a 

def _getProjectStructure(pprObject):

    # backwards compatibility test
    ppr_project = pprObject.SciPipeRequest.ProjectStructure
    structureList = []
    return structureList


# Given the pipeline processing request object return the number of processing
# requests. For EVLA this should always be 1 but check. Assume a single scheduling block
# per processing request.

def _getNumRequests(pprObject):

    ppr_prequests = pprObject.SciPipeRequest.ProcessingRequests

    numRequests = 0

    # Try single element / single scheduling block first. 
    try:
        relative_path = ppr_prequests.ProcessingRequest.DataSet.RelativePath.getValue()
	numRequests = 1
	return numRequests
    except Exception, e:
        pass


    # Next try multiple processing requests  / single scheduling block
    search = 1
    while (search):
        try:
            relative_path = ppr_prequests.ProcessingRequest[numRequests].DataSet.RelativePath.getValue()
	    numRequests = numRequests + 1
        except Exception, e:
	    search = 0
	    if numRequests > 0:
	        return numRequests
	    else:
	        pass


    # Return the number of requests.
    return numRequests



# Given the pipeline processing request object return a list of processing
# intents in the form of a keyword and value dictionary

def _getIntents (pprObject, requestId, numRequests):

    if numRequests == 1:
        ppr_intents = pprObject.SciPipeRequest.ProcessingRequests.ProcessingRequest.ProcessingIntents
    else:
        ppr_intents = pprObject.SciPipeRequest.ProcessingRequests.ProcessingRequest[requestId].ProcessingIntents

    intentsDict = {}
    numIntents = 0
    try:
        intentName = ppr_intents.Intents.Keyword.getValue()
	try:
	    intentValue = ppr_intents.Intents.Value.getValue()
	except Exception, e:
	    intentValue = ""
	numIntents = 1
	intentsDict[intentName] = intentValue
    except Exception, e:
        search = 1
	while (search):
	    try:
                intentName = ppr_intents.Intents[numIntents].Keyword.getValue()
		try:
	            intentValue = ppr_intents.Intents[numIntents].Value.getValue()
		except Exception, e:
		    intentValue = ""
	        numIntents = numIntents + 1
	        intentsDict[intentName] = intentValue
            except Exception, e:
	        search = 0

    return numIntents, intentsDict

# Given the pipeline processing request object return a list of processing
# commands where each element in the list is a tuple consisting of the
# processing command name and the parameter set dictionary.

def _getCommands (pprObject, requestId, numRequests):

    if numRequests == 1:
        ppr_cmds = pprObject.SciPipeRequest.ProcessingRequests.ProcessingRequest.ProcessingProcedure
    else:
        ppr_cmds = pprObject.SciPipeRequest.ProcessingRequests.ProcessingRequest[requestId].ProcessingProcedure

    try:
        procedureName = ppr_cmds.ProcedureTitle 
    except Exception, e:
        procedureName = "Undefined"
    commandsList = []
    numCommands = 0

    try:
        cmdName = ppr_cmds.ProcessingCommand.Command.getValue()
        ppr_params = ppr_cmds.ProcessingCommand.ParameterSet
	numParams, paramsDict = _getParameters (ppr_params)
	numCommands = 1
	commandsList.append((cmdName, paramsDict))
    except Exception, e:
        search = 1
	while (search):
	    try:
                cmdName = ppr_cmds.ProcessingCommand[numCommands].Command.getValue()
                ppr_params = ppr_cmds.ProcessingCommand[numCommands].ParameterSet
	        numParams, paramsDict = _getParameters (ppr_params)
		numCommands = numCommands + 1
	        commandsList.append((cmdName, paramsDict))
	    except Exception, e:
	        search = 0

    return procedureName, numCommands, commandsList

# Given the pipeline processing request object return the number of scheduling
# block sets. For the EVLA there can be only one.

def _getNumSchedBlockSets (pprObject, requestId, numRequests):

    if numRequests == 1:
        ppr_dset = pprObject.SciPipeRequest.ProcessingRequests.ProcessingRequest.DataSet
    else:
        ppr_dset = pprObject.SciPipeRequest.ProcessingRequests.ProcessingRequest[requestId].DataSet

    numSchedBlockSets = 0

    try:
        path = ppr_dset.RelativePath.getValue()
	numSchedBlockSets = 1
    except Exception, e:
        numSchedBlockSets = 0

    return numSchedBlockSets

# Given the pipeline processing request object return a list of ASDMs
# where each element in the list is a tuple consisting of the path
# to the ASDM, the name of the ASDM, and the UID of the ASDM. 

def _getAsdmList (pprObject, sbsetId, numSbSets, requestId, numRequests):

    if numRequests == 1:
        ppr_dset = pprObject.SciPipeRequest.ProcessingRequests.ProcessingRequest.DataSet
	if numSbSets == 1:
	    ppr_dset = ppr_dset
	    relativePath = ppr_dset.RelativePath.getValue()
	else:
	    ppr_dset = ppr_dset
	    relativePath = ""
    else:
        relativePath = ""

    asdmList = []
    try:
	asdmName = ppr_dset.SdmIdentifier.getValue()
	asdmUid = asdmName
	asdmList.append ((relativePath, asdmName, asdmUid))
	numAsdms = 1
    except Exception, e:
        numAsdms = 0

    return relativePath, numAsdms, asdmList

# Given a parameter set object retrieve the parameter set dictionary for
# each command.

def _getParameters (ppsetObject):

    numParams = 0
    paramsDict = {}

    try:
        paramName = ppsetObject.Parameter.Keyword.getValue()
	try:
            paramValue = ppsetObject.Parameter.Value.getValue()
	except Exception, e:
	    paramValue = ""
	numParams = 1
	paramsDict[paramName] = paramValue
    except Exception, e:
        search = 1
	while (search):
	    try:
                paramName = ppsetObject.Parameter[numParams].Keyword.getValue()
	        try:
                    paramValue = ppsetObject.Parameter[numParams].Value.getValue()
	        except Exception, e:
	            paramValue = ""
		numParams = numParams + 1
	        paramsDict[paramName] = paramValue
	    except Exception, e:
	        search = 0

    return numParams, paramsDict
