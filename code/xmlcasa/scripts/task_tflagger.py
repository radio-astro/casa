from taskinit import *
import time
import os
import sys

debug = True


def tflagger(vis = None,
             inputtype = None,          # file,table,xml,cmd
             inputfile = None,          # table parameters
             tablerows = None,          
             useapplied = None,
             tbuff = None,              # xml parameters
             ants = None,
             command = None,            # cmd parameter
             reason = None,             # applied to all input types
             ntime = None,              # taken only once per session
             extend = None,             # extend the private flags of any agent
             mode = None,
             selectdata = None,
             spw = None,                # data selection parameters
             field = None,
             antenna = None,
             uvrange = None,
             timerange = None,
             correlation = None,
             scan = None,
             intent = None,
             feed = None, 
             array = None,
             observation = None,
             expression = None,           # mode=clip parameters
             clipminmax = None,
             datacolumn = None, 
             clipoutside = None, 
             channelavg = None,
             quackinterval = None,      # mode=quack parameters
             quackmode = None, 
             quackincrement = None,
             diameter = None,           # mode=shadow parameter
             lowerlimit = None,         # mode=elevation parameters
             upperlimit = None,
             timecutoff = None,         # mode=tfcrop parameters
             freqcutoff = None,
             timefit = None, 
             freqfit = None, 
             maxnpieces = None, 
             flagdimension = None, 
             usewindowstats = None, 
             halfwin = None,
             extendpols = None,         # mode=extendflags parameters
             growtime = None, 
             growfreq = None, 
             growaround = None, 
             flagneartime = None, 
             flagnearfreq = None,
             minrel = None,             # mode=summary parameters
             maxrel = None,
             minabs = None,
             maxabs = None,
             spwchan = None,
             spwcorr = None,
             datadisplay = None,
             writeflags = None,
             flagbackup = None,
             async = None):

    # Things to consider:
    #
    # * Default inputtype is ''
    # * Default mode is manualflag
    # * Reason selecting for all input types
    # * Extendflags can be applied to every mode
    # * Parameter ntime can be applied only once per session
    # * Ability to read flags and edit them......
    # * It is already possible to read flags from the FLAG_CMD of an MS and 
    #   apply them to another MS. Do as follows
    #   flagcmd(vis='target.ms',flagmode='table',flagfile='source.ms',optype='apply')
    # 
    # Schema
    # Read input commands from input file
    # Select based on REASON
    # Configure the TestFlagger tool -> ::configTestFlagger()
    # Create a union of the data selection ranges -> getUnion()
    # Parse the union to the data selection -> ::parseDataSelection()
    # Read the mode and specific parameters.
    # Parse the agent's parameters -> setupAgent() -> ::parseAgentParameters()
    # Initialize the FlagDataHandler and the agents -> ::init()
    # Run the tool
    # Delete the tool
                        
    #
    # Task tflagger
    #    Reads flag commands and applies to MS

    if pCASA.is_mms(vis):
        pCASA.execute("tflagger", locals())
        return

    try:
        from xml.dom import minidom
    except:
        raise Exception, 'Failed to load xml.dom.minidom into python'

    casalog.origin('tflagger')

    tflocal = casac.homefinder.find_home_by_name('testflaggerHome').create()
    mslocal = casac.homefinder.find_home_by_name('msHome').create()

    try:
        if not os.path.exists(vis):
            raise Exception, \
                'Visibility data set not found - please verify the name'

    except Exception, instance:
        print '*** Error ***', instance
                # raise

        # MS HISTORY
        mslocal.open(vis,nomodify=False)
        mslocal.writehistory(message='taskname = flagger', origin='tflagger')
        mslocal.open(vis,nomodify=False)
            
        # TBD
        # Get overall MS time range for later use (if needed)

        # INPUT TYPE
        # TABLE
        if (inputtype == 'table'):
            casalog.post('Reading commands from FLAG_CMD table')
            if inputfile == '':
                input = vis
            else:
                input = inputfile
                
            # readFromTable(input)
        
        # FILE
        elif (inputtype == 'file'):
            casalog.post('Reading commands from file %'+inputfile)
            if inputfile == '':
                input = vis + '/FlagCMD.txt'
            else:
                input = inputfile

            if (type(input) == str) & os.path.exists(input):
                try:
                    ff = open(input, 'r')
                except:
                    raise Exception, 'Error opening file ' + input
            else:
                raise Exception, \
                    'Input command file not found - please verify the name'
                    
            # readFromFile(input)

        # XML
        elif (inputtype == 'xml'):
            casalog.post('Reading commands from the XML file %s'+vis+'/Flag.xml')
            if inputfile == '':
                input = vis
            else:
                input = inputfile
                
            # readFromXML(input)

        # TBD: Does it make any sense to have this if we can already
        # run the task without any input?????
        # COMMAND LIST
        elif (inputtype == 'cmd'):
            casalog.post('Reading commands from a list')
            
        # NO INPUT; run the task with the provided parameters        
        else:
             input = vis   


        # Configure the tool TestFlagger
        tfconfig = {}
        tfconfig['msname'] = vis
        tfconfig['ntime'] = ntime
        
        try:
            tflocal.configTestFlagger(tfconfig)
        except:
            casalog.post('Error in configuring the TestFlagger tool', ERROR)
            raise Exception        


    # write history
    try:
        mslocal.open(vis, nomodify=False)
        mslocal.close()
    except:
        casalog.post('Cannot open MS for history, ignoring', 'WARN')

    return

            
          
            
            
