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
    Single Dish sd* tasks are available after asap_init() is run   
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
    print ' at : Juan Pardo ATM library'
    print ' cb : Calibration utilities'
    print ' cp : Cal solution plotting utilities'
    print ' fg : Flagging/Flag management utilities'
    print ' ia : Image analysis utilities'
    print ' im : Imaging utilities'
    print ' me : Measures utilities'
    print ' ms : MeasurementSet (MS) utilties'
    print ' mp : MS plotting (data (amp/phase) versus other quantities)'
    print ' pm : PlotMS utilities'
    print ' tb : Table utilities (selection, extraction, etc)'
    print ' tp : Table plotting utilities'
    print ' qa : Quanta utilities'
    print ' sl : Spectral line import and search'
    print ' sm : Simulation utilities'
    print ' vp : Voltage pattern/primary beam utilties'
    print ' ---'
    print ' pl : pylab functions (e.g., pl.title, etc)'
    print ' sd : (after running asap_init()) Single dish utilities'
    print ' ---'

def tasklist():
    """ List tasks, organized by catagory """
    print 'Available tasks, organized by category (experimental tasks in parenthesis): \n'
    print 'Import/Export       Information   Data Editing  Display/Plotting'
    print '-------------       -----------   ------------  ----------------'
    print 'importvla           imhead        concat        clearplot'
    print 'importfits          listcal       fixvis        plotants '
    print 'importuvfits        listhistory   flagautocorr  plotcal'    
    print 'exportfits          listobs       flagdata      plotms'
    print 'exportuvfits        listvis       flagdata2     plotxy'
    print '(importasdm)        vishead       flagmanager   imview'
    print '(importgmrt)        visstat       plotms        msview'
    print '                                  plotxy'	 
    print ' '
    print ''
    print 'Data Manipulation   Calibration   Imaging       Modelling   '
    print '-----------------   -----------   -------       ---------   '
    print 'concat              accum         clean         setjy       '
    print 'cvel                applycal      deconvolve    uvcontsub   '
    print 'fixvis              bandpass      feather       uvmodelfit  '
    print 'hanningsmooth       blcal         ft            uvsub'
    print 'split               calstat       csvclean      (uvcontsub2)'
    print 'uvcontsub           clearcal      widefield                '
    print 'uvsub               cvel          (boxit )                 '
    print '(uvcontsub2)        fluxscale     (autoclean)              '
    print '(msmoments)         fixvis                                 '
    print '                    gaincal                                 '
    print '                    gencal                                  '
    print '                    listcal                                 '
    print '                    polcal                                  '
    print '                    setjy                                   '
    print '                    smoothcal                               '
    print '                    (fringecal)                             '
    print '                    (peel)                                  '
    print '                                                            '
    print '                                                            '
    print ''
    print 'Image Analysis  Simulation  Utilities           Single Dish'
    print '--------------  ----------  ---------           (after running asap_init())'
    print 'imcontsub       simdata     browsetable         ---------------------------'
    print 'imhead                      casalogger          sdaverage'
    print 'imfit                       clearplot           sdbaseline'
    print 'immath                      clearstat           sdcal'
    print 'immoments                   csvclean            sdcoadd'
    print 'imregrid                    filecatalog         sdfit'
    print 'imsmooth                    find                sdflag'
    print 'imstat                      help par.parameter  sdflagmanager'
    print 'imval                       help task           sdimaging'
    print 'specfit                     rmtables            sdimprocess'
    print 'imcollapse                  startup             sdlist'
    print 'imstat                      taskhelp            sdmath'
    print 'imtrans                     tasklist            sdplot'
    print '                            toolhelp            sdsave'
    print '                                                sdscale'
    print '                                                sdsmooth '
    print '                                                sdstat '
    print '                                                sdtpimaging '
    print '                                                (msmoments)'
    print ''
    print ''
    print ''
    print 'Spectral Line'
    print '-------------'
    print 'slsearch'
    print 'splattotable'
    if globals().has_key('mytasks') :
        print ''
        print 'User defined tasks'
        print '------------------'
        for key in mytasks.keys() :
            print key

