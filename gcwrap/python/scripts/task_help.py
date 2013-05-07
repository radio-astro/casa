from tasksinfo import *
def startup():
    # startup guide
    """ Start up screen for CASA """
    print """___________________________________________________________________
    For help use the following commands:
    tasklist               - Task list organized by category
    taskhelp               - One line summary of available tasks
    help taskname          - Full help for task
    toolhelp               - One line summary of available tools
    help par.parametername - Full help for parameter name
___________________________________________________________________"""

def taskhelp(scrap=None):
    """ Briefly describe all tasks with scrap in their name or one-line description. """
    if scrap:
        scrap = str(scrap)
        foundtasks = [ft for ft in tasksum.keys() if ft.find(scrap) > -1 or tasksum[ft].find(scrap) > -1]
        if not foundtasks:
            print "No tasks were found with '%s' in their name or description." % scrap
        return
    else:
        print 'Available tasks: \n'
        foundtasks = tasksum.keys()
        foundtasks.sort()   # Already sorted?!
        widestftlen = max([len(ft) for ft in foundtasks])
        fmt = "%%-%ds : %%s" % widestftlen
        for ft in foundtasks:
            print fmt % (ft, tasksum[ft])            
def toolhelp():
    """ List all tools with one-line description: """
    print ' '
    print 'Available tools: \n'
    print ' af : Agent flagger utilities'
    print ' at : Juan Pardo ATM library'
    print ' ca : Calibration analysis utilities'
    print ' cb : Calibration utilities'
    print ' cl : Component list utilities'
    print ' cp : Cal solution plotting utilities'
    print ' cs : Coordinate system utilities'
    print ' cu : Class utilities'
    print ' dc : Deconvolver utilities'
    print ' fi : Fitting utilities'
    print ' fn : Functional utilities'
    print ' ia : Image analysis utilities'
    print ' im : Imaging utilities'
    print ' me : Measures utilities'
    print ' ms : MeasurementSet (MS) utilities'
    print ' msmd : MS metadata accessors'
    print ' mp : MS plotting (data (amp/phase) versus other quantities)'
    print ' mt : MS transformer utilities'
    print ' qa : Quanta utilities'
    print ' pm : PlotMS utilities'
    print ' po : Imagepol utilities'
    print ' rg : Region manipulation utilities'
    print ' sl : Spectral line import and search'
    print ' sm : Simulation utilities'
    print ' tb : Table utilities (selection, extraction, etc)'
    print ' tp : Table plotting utilities'
    print ' vp : Voltage pattern/primary beam utilities'
    print ' ---'
    print ' pl : pylab functions (e.g., pl.title, etc)'
    print ' sd : Single dish utilities'
    print ' ---'

def tasklist():
    """ List tasks, organized by category """
    print 'Available tasks, organized by category (experimental tasks in parenthesis ()'
    print '  deprecated tasks in curly brackets {}).'
    print ''
    for i in range(0,3):
        col1 = thecats[i*4]
        col2 = thecats[i*4+1]
        col3 = thecats[i*4+2]
        col4 = thecats[i*4+3]
        count1 = len(allcat[col1])
        count2 = len(allcat[col2])
        count3 = len(allcat[col3])
        count4 = len(allcat[col4])
        maxcount = max([count1, count2, count3, count4])
        taskrow = ''
	print
        print '%-18.18s  %-18.18s  %-18.18s  %-18.18s'% (col1.capitalize(), col2.capitalize(), col3.capitalize(), col4.capitalize())
	print '------------------  ------------------  ------------------  ------------------'
        for i in range(0, maxcount) :
            if(i<count1) :
                task1 = allcat[col1][i]
            else :
                task1 = ' '
            if(i<count2) :
                task2 = allcat[col2][i] 
            else :
                task2 = ' '
            if(i<count3) :
                task3 = allcat[col3][i]
            else :
                task3 = ' '
            if(i<count4) :
                task4 = allcat[col4][i]
            else :
                task4 = ' '
            print '%-18.18s  %-18.18s  %-18.18s  %-18.18s'% (task1, task2, task3, task4)

    if globals().has_key('mytasks') :
        print ''
        print 'User defined tasks'
        print '------------------'
        for key in mytasks.keys() :
            print key

