# Execute the pipeline processing request
#    Code first as module and convert to class if appropriate
#    Factor and document properly  when details worked out    
#
# Turn some print statement into CASA log statements
#

import gc
import os
import string
import sys
import traceback

import argmapper
import pipeline
import pipeline.extern.XmlObjectifier as XmlObjectifier
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.project as project
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.vdp as vdp
from . import task_registry

# Make sure CASA exceptions are rethrown
try:
    default__rethrow_casa_exceptions = __rethrow_casa_exceptions=True
except Exception, e:
    default__rethrow_casa_exceptions = False
__rethrow_casa_exceptions=True


def executeppr (pprXmlFile, importonly=True, breakpoint='breakpoint',
    bpaction='ignore', loglevel='info', plotlevel='default',
    interactive=True):

    # Useful mode parameters
    echo_to_screen = interactive
    workingDir = None
    rawDir = None

    try:

        # Decode the processing request
        casatools.post_to_log ("Analyzing pipeline processing request ...",
            echo_to_screen=echo_to_screen)
        info, structure, relativePath, intentsDict, asdmList, procedureName, commandsList = \
            _getFirstRequest (pprXmlFile)

        # Set the directories
        workingDir = os.path.join (os.path.expandvars("$SCIPIPE_ROOTDIR"),
            relativePath, "working")
        rawDir = os.path.join (os.path.expandvars("$SCIPIPE_ROOTDIR"),
            relativePath, "rawdata")

        # Check for the breakpoint
        bpset = False
        if breakpoint != '':
            for command in commandsList:
                if command[0] == breakpoint:
                    casatools.post_to_log ("    Found break point: " + breakpoint,
                        echo_to_screen=echo_to_screen)
                    casatools.post_to_log ("    Break point action is: " + bpaction,
                        echo_to_screen=echo_to_screen)
                    bpset = True
                    break

        # Get the pipeline context
        #     Resumes from the last context. Consider adding name
        if bpset and bpaction == 'resume':
            context = pipeline.Pipeline(context='last').context
            casatools.post_to_log ("    Resuming from last context",
                echo_to_screen=echo_to_screen)
        else:
            context = pipeline.Pipeline(loglevel=loglevel, plotlevel=plotlevel,
                output_dir=workingDir).context
            casatools.post_to_log ("    Creating new pipeline context",
                echo_to_screen=echo_to_screen)

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
        errorfile = utils.write_errorexit_file(workingDir, 'errorexit', 'txt')
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
        errorfile = utils.write_errorexit_file(workingDir, 'errorexit', 'txt')
        return
    elif len(asdmList) < 1:
        casatools.post_to_log ("    Empty ASDM list",
            echo_to_screen=echo_to_screen)
        casatools.post_to_log ("Terminating pipeline execution ...",
            echo_to_screen=echo_to_screen)
        errorfile = utils.write_errorexit_file(workingDir, 'errorexit', 'txt')
        return
    elif len(commandsList) < 1:
        casatools.post_to_log ("    Empty commands list",
            echo_to_screen=echo_to_screen)
        casatools.post_to_log ("Terminating pipeline execution ...",
            echo_to_screen=echo_to_screen)
        errorfile = utils.write_errorexit_file(workingDir, 'errorexit', 'txt')
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
    ds = dict(structure)
    context.project_structure = project.ProjectStructure(
        ous_entity_id = ds['ous_entity_id'][1],
        ous_part_id = ds['ous_part_id'][1],
        ous_title = ds['ous_title'][1],
        ous_type = ds['ous_type'][1],
        ps_entity_id = ds['ps_entity_id'][1],
        ousstatus_entity_id = ds['ousstatus_entity_id'][1],
        ppr_file=pprXmlFile,
        recipe_name=procedureName)

    # Create performance parameters object
    context.project_performance_parameters = _getPerformanceParameters(intentsDict)

    # Get the session info from the intents dictionary
    if len(intentsDict) > 0:
        sessionsDict = _getSessions (intentsDict)
    else:
        sessionsDict = {}

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
        for key, value in sessionsDict.iteritems():
            if asdm[1] in value:
                session = key.lower()
                break
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
    foundbp = False
    for command in commandsList:

        # Get task name and arguments lists.
        casa_task = command[0]
        task_args = command[1]
        casatools.set_log_origin(fromwhere=casa_task)

        # Handle break point if one is set
        if bpset:
            # Found the break point
            #    Set the found flag
            #    Ignore it  or
            #    Break the loop or
            #    Resume execution
            if casa_task == breakpoint:
                foundbp = True
                if bpaction == 'ignore':
                    casatools.post_to_log(
                    "Ignoring breakpoint " + casa_task,
                        echo_to_screen=echo_to_screen)
                    continue
                elif bpaction == 'break':
                    casatools.post_to_log(
                        "Terminating execution at breakpoint " + casa_task,
                        echo_to_screen=echo_to_screen)
                    break
                elif bpaction == 'resume':
                    casatools.post_to_log(
                        "Resuming execution after breakpoint " + casa_task,
                        echo_to_screen=echo_to_screen)
                    continue
            # Not the break point so check the resume case
            elif not foundbp and bpaction == 'resume':
                casatools.post_to_log("Skipping task " + casa_task, echo_to_screen=echo_to_screen)
                continue

        # Execute
        casatools.post_to_log ("Executing command ..." + casa_task, echo_to_screen=echo_to_screen)

        # Execute the command
        try:
            pipeline_task_class = task_registry.get_pipeline_class_for_task(casa_task)
            pipeline_task_name = pipeline_task_class.__name__
            casatools.post_to_log("    Using python class ..." + pipeline_task_name, echo_to_screen=echo_to_screen)

            # List parameters
            for keyword, value in task_args.items():
                casatools.post_to_log("    Parameter: " + \
                                      keyword + " = " + str(value), echo_to_screen=echo_to_screen)

            if pipeline_task_name == 'ImportData' or pipeline_task_name == 'RestoreData' \
                    or pipeline_task_name == 'ALMAImportData' or pipeline_task_name == 'ALMARestoreData'\
                    or pipeline_task_name == 'VLAImportData' or pipeline_task_name == 'VLARestoreData'\
                    or pipeline_task_name == 'SDImportData' or pipeline_task_name == 'NROImportData':
                task_args['vis'] = files
                task_args['session'] = sessions

            remapped_args = argmapper.convert_args(pipeline_task_class, task_args, convert_nulls=False)
            inputs = vdp.InputsContainer(pipeline_task_class, context, **remapped_args)
            task = pipeline_task_class(inputs)
            results = task.execute(dry_run=False)
            casatools.post_to_log('Results ' + str(results), echo_to_screen=echo_to_screen)

            try:
                results.accept(context)
            except Exception, e:
                casatools.post_to_log("Error: " \
                    "Failed to update context for " + pipeline_task_name,
                    echo_to_screen=echo_to_screen)
                raise
            finally:
                gc.collect()

            # Stop execution if result is a failed task result or a list
            # containing a failed task result.
            tracebacks = utils.get_tracebacks(results)
            if len(tracebacks) > 0:
                casatools.post_to_log("Unexpected error encountered during {}".format(pipeline_task_name),
                                      echo_to_screen=echo_to_screen)
                for tb in tracebacks:
                    casatools.post_to_log("{}".format(tb), echo_to_screen=echo_to_screen)
                errorfile = utils.write_errorexit_file(workingDir, 'errorexit', 'txt')
                break

            if pipeline_task_name == 'ImportData' and importonly:
                casatools.post_to_log(
                    "Terminating execution after running " + pipeline_task_name,
                    echo_to_screen=echo_to_screen)
                break

            if pipeline_task_name == 'ALMAImportData' and importonly:
                casatools.post_to_log(
                    "Terminating execution after running " + pipeline_task_name,
                    echo_to_screen=echo_to_screen)
                break

            if pipeline_task_name == 'VLAImportData' and importonly:
                casatools.post_to_log(
                    "Terminating execution after running " + pipeline_task_name,
                    echo_to_screen=echo_to_screen)
                break

            if pipeline_task_name == 'SDImportData' and importonly:
                casatools.post_to_log("Terminating execution after running " + pipeline_task_name,
                                      echo_to_screen=echo_to_screen)
                break

            if pipeline_task_name == 'NROImportData' and importonly:
                casatools.post_to_log("Terminating execution after running " + pipeline_task_name,
                                      echo_to_screen=echo_to_screen)
                break

        except Exception, e:
            # Log message if an exception occurred that was not handled by
            # standardtask template (not turned into failed task result).
            errstr = traceback.format_exc()
            casatools.post_to_log(errstr, echo_to_screen=echo_to_screen)
            errorfile = utils.write_errorexit_file(workingDir, 'errorexit', 'txt')
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
    #print 'Number of processing requests: ', numRequests

    # Get brief project summary
    info = _getProjectSummary(pprObject)

    # Get project structure.
    structure = _getProjectStructure(pprObject)

    # Get the intents dictionary
    numIntents, intentsDict  = _getIntents (pprObject=pprObject,
        requestId=0, numRequests=numRequests)
    #print 'Number of intents: ', numIntents
    #print 'Intents dictionary: ', intentsDict

    # Get the commands list
    procedureName, numCommands, commandsList  = _getCommands (pprObject=pprObject,
        requestId=0, numRequests=numRequests)
    #print 'Number of commands: ', numCommands
    #print 'Commands list: ', commandsList

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
    #print 'Number of scheduling block sets: ', numSbSets

    # Get the ASDM list
    relativePath, numAsdms, asdmList = _getAsdmList (pprObject=pprObject,
        sbsetId=0, numSbSets=numSbSets, requestId=0,
        numRequests=numRequests)
    #print 'Relative path: ', relativePath
    #print 'Number of Asdms: ', numAsdms
    #print 'ASDM list: ', asdmList

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
    # Note that these are not being filled properly from the project status
    #summaryList.append (('proposal_title', ('Proposal title: ',
    #    ppr_summary.ProposalTitle.getValue())))
    #summaryList.append (('piname', ('Principal investigator: ',
    #    ppr_summary.PIName.getValue())))
    summaryList.append (('observatory', ('Observatory: ',
        ppr_summary.Observatory.getValue())))
    summaryList.append (('telescope', ('Telescope: ',
        ppr_summary.Telescope.getValue())))

    return summaryList

# Given the pipeline processing request object print some project structure
# information. Returns a 

def _getProjectStructure(pprObject):

    # backwards compatibility test
    ppr_project = pprObject.SciPipeRequest.ProjectStructure
    try:
        entityid = ppr_project.OUSStatusRef.getAttribute('entityId')
    except Exception, e:
        ppr_project = ppr_project.AlmaStructure

    structureList = []
    structureList.append (('ous_entity_type', ('ObsUnitSet Entity Type: ',
        ppr_project.ObsUnitSetRef.getAttribute('entityTypeName'))))
    structureList.append (('ous_entity_id', ('ObsUnitSet Entity Id: ',
        ppr_project.ObsUnitSetRef.getAttribute('entityId'))))
    structureList.append (('ous_part_id', ('ObsUnitSet Part Id: ',
        ppr_project.ObsUnitSetRef.getAttribute('partId'))))
    structureList.append (('ous_title', ('ObsUnitSet Title: ',
        ppr_project.ObsUnitSetTitle.getValue())))
    structureList.append (('ous_type', ('ObsUnitSet Type: ',
        ppr_project.ObsUnitSetType.getValue())))
    structureList.append (('ps_entity_type', ('ProjectStatus Entity Type: ',
        ppr_project.ProjectStatusRef.getAttribute('entityTypeName'))))
    structureList.append (('ps_entity_id', ('ProjectStatus Entity Id: ',
        ppr_project.ProjectStatusRef.getAttribute('entityId'))))
    structureList.append (('ousstatus_entity_type', ('OUSStatus Entity Type: ',
        ppr_project.OUSStatusRef.getAttribute('entityTypeName'))))
    structureList.append (('ousstatus_entity_id', ('OUSStatus Entity Id: ',
        ppr_project.OUSStatusRef.getAttribute('entityId'))))

    return structureList


# Given the pipeline processing request object return the number of processing
# requests. This should normally be 1.

def _getNumRequests(pprObject):

    ppr_prequests = pprObject.SciPipeRequest.ProcessingRequests

    numRequests = 0

    # Try single element / single scheduling block first.
    try:
        relative_path = ppr_prequests.ProcessingRequest.DataSet.SchedBlockSet.SchedBlockIdentifier.RelativePath.getValue()
        numRequests = 1
        return numRequests
    except Exception, e:
        pass

    # Try single element / multiple scheduling block next.
    try:
        relative_path = ppr_prequests.ProcessingRequest.DataSet.SchedBlockSet.SchedBlockIdentifier[0].RelativePath.getValue()
        numRequests = 1
        return numRequests
    except Exception, e:
        pass

    # Next try multiple elements  / single scheduling block
    search = 1
    while (search):
        try:
            relative_path = ppr_prequests.ProcessingRequest[numRequests].DataSet.SchedBlockSet.SchedBlockIdentifier.RelativePath.getValue()
            numRequests = numRequests + 1
        except Exception, e:
            search = 0
            if numRequests > 0:
                return numRequests
            else:
                pass

    # Next try multiple elements  / multiple scheduling block
    search = 1
    while (search):
        try:
            relative_path = ppr_prequests.ProcessingRequest[numRequests].DataSet.SchedBlockSet.SchedBlockIdentifier[0].RelativePath.getValue()
            numRequests = numRequests + 1
        except Exception, e:
            search = 0
            if numRequests > 0:
                return numRequests
            else:
                pass

    # Return the number of requests.
    return numRequests


# Return the relative path for the request

#def _getRelativePath (pprObject, requestId, numRequests):
#
#    ppr_prequests = pprObject.SciPipeRequest.ProcessingRequests
#
#    if numRequests == 1:
#        relativePath = ppr_prequests.ProcessingRequest.RelativePath.getValue()
#    else:
#        relativePath = ppr_prequests.ProcessingRequest[requestId].RelativePath.getValue()
#
#    return relativePath


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

def _getPerformanceParameters(intentsDict):

    # Initalize
    performanceParams = project.PerformanceParameters()

    # No performance parameters
    if len(intentsDict) <= 0:
        return performanceParams

    # Supported performance parameters
    #   Don't use. Rely on class __init__ method
    #params = ['desired_angular_resolution',
        #'desired_largest_scale',
        #'desired_spectral_resolution',
        #'desired_sensitivity',
        #'desired_dynamic_range']

    # Set supported attributes
    for key in intentsDict:
        # Parameter not defined in __init__ method
        if not hasattr (performanceParams, key):
            continue
        # Parameter not supported
        #if key not in params:
            #continue
        setattr (performanceParams, key, intentsDict[key])

    return performanceParams

def _getSessions (intentsDict):

    sessionsDict = {}

    searching = True
    ptr = 1
    while searching:
        key = 'SESSION_' + str (ptr)
        if intentsDict.has_key(key):
            #asdmList = intentsDict[key].split(',')
            asdmList = intentsDict[key].split(' | ')
            asdmList = [asdm.translate(string.maketrans(':/', '__')) for asdm in asdmList]
            sessionsDict[key] = asdmList
            ptr = ptr + 1
        else:
            searching = False
            break

    return sessionsDict

# Given the pipeline processing request object return a list of processing
# commands where each element in the list is a tuple consisting of the
# processing command name and the parameter set dictionary.

def _getCommands (pprObject, requestId, numRequests):

    if numRequests == 1:
        ppr_cmds = pprObject.SciPipeRequest.ProcessingRequests.ProcessingRequest.ProcessingProcedure
    else:
        ppr_cmds = pprObject.SciPipeRequest.ProcessingRequests.ProcessingRequest[requestId].ProcessingProcedure

    try:
        procedureName = ppr_cmds.ProcedureTitle.getValue()
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
# block sets.

def _getNumSchedBlockSets (pprObject, requestId, numRequests):

    if numRequests == 1:
        ppr_dset = pprObject.SciPipeRequest.ProcessingRequests.ProcessingRequest.DataSet
    else:
        ppr_dset = pprObject.SciPipeRequest.ProcessingRequests.ProcessingRequest[requestId].DataSet

    numSchedBlockSets = 0

    try:
        path = ppr_dset.SchedBlockSet.SchedBlockIdentifier.RelativePath.getValue()
        numSchedBlockSets = 1
    except Exception, e:
        search = 1
        while (search):
            try:
                path = ppr_dset.SchedBlockSet[numSchedBlocks].SchedBlockIdentifier.RelativePath.getValue()
                numSchedBlockSets = numSchedBlockSets + 1
            except Exception, e:
                search = 0

    return numSchedBlockSets

# Given the pipeline processing request object return a list of ASDMs
# where each element in the list is a tuple consisting of the path
# to the ASDM, the name of the ASDM, and the UID of the ASDM. 

def _getAsdmList (pprObject, sbsetId, numSbSets, requestId, numRequests):

    relativePath = ""
    if numRequests == 1:
        ppr_dset = pprObject.SciPipeRequest.ProcessingRequests.ProcessingRequest.DataSet
        if numSbSets == 1:
            ppr_dset = ppr_dset.SchedBlockSet.SchedBlockIdentifier
        else:
            ppr_dset = ppr_dset.SchedBlockSet[sbsetId].SchedBlockIdentifier
        relativePath = ppr_dset.RelativePath.getValue()
    else:
        ppr_dset = pprObject.SciPipeRequest.ProcessingRequests.ProcessingRequest[requestId].DataSet
        if numSbSets == 1:
            ppr_dset = ppr_dset.SchedBlockSet.SchedBlockIdentifier
        else:
            ppr_dset = ppr_dset.SchedBlockSet[sbsetId].SchedBlockIdentifier
        relativePath = ppr_dset.RelativePath.getValue

    numAsdms = 0
    asdmList = []

    try:
        #asdmPath = ppr_dset.AsdmSet.RelativePath.getValue()
        asdmName = ppr_dset.AsdmIdentifier.AsdmDiskName.getValue()
        asdmUid = ppr_dset.AsdmIdentifier.AsdmRef.ExecBlockId.getValue()
        asdmList.append ((relativePath, asdmName, asdmUid))
        numAsdms = 1
    except Exception, e:
        search = 1
        while (search):
            try:
                #asdmPath = ppr_dset.AsdmSet[numAsdms].RelativePath.getValue()
                #asdmPath = ppr_dset.AsdmSet.RelativePath.getValue()
                #asdmName = ppr_dset.AsdmSet[numAsdms].AsdmIdentifier.AsdmDiskName.getValue()
                asdmName = ppr_dset.AsdmIdentifier[numAsdms].AsdmDiskName.getValue()
                #asdmUid = ppr_dset.AsdmSet[numAsdms].AsdmIdentifier.Asdmref.ExecBlockId.getValue()
                asdmUid = ppr_dset.AsdmIdentifier[numAsdms].AsdmRef.ExecBlockId.getValue()
                numAsdms = numAsdms + 1
                asdmList.append ((relativePath, asdmName, asdmUid))
            except Exception, e:
                search = 0

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
