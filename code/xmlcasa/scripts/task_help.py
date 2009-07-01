from tasksinfo import *
def startup():
# startup guide
      """ Start up screen for CASA """
      print """___________________________________________________________
Available tasks:

   accum         flagdata       listcal      sdlist
   applycal      flagmanager    listhistory  sdplot    
   bandpass      fluxscale      listobs      sdsave    
   blcal         ft             listvis      sdscale   
   browsetable   gaincal        makemask     sdsmooth  
   clean         hanningsmooth  mosaic       sdstat    
   clearcal      imcontsub      plotants     setjy     
   clearplot     imhead         plotcal      smoothcal 
   clearstat     immoments      plotms       specfit   
   concat        importfits     plotxy       split     
   deconvolve    importuvfits   sdaverage    tget      
   exportfits    importvla      sdbaseline   uvcontsub 
   exportuvfits  imregrid       sdcal        uvcontsub2
   feather       imsmooth       sdcoadd      uvmodelfit
   filecatalog   imstat         sdfit        viewer    
   find          imval          sdflag       vishead  
   fixvis        invert                      widefield        
                
Additional tasks available for ALMA commissioning use
         (still alpha code as of Beta release):

 simdata        importasdm

Available tools:

   cb (calibrater)       cp (cal plot)    fg (flagger)
   ia (image analysis)   im (imager)      me (measures)
   mp (MS plot)          ms (MS)          qa (quanta)
   sm (simulation)       tb (table)       tp (table plot)
   vp (voltage patterns) pm (plotms)      at (atmosphere)

   pl (pylab functions)
   sd (ASAP functions - run asap_init() to import into CASA)

   casalogger             - Call up the casalogger (if it goes away)
   
___________________________________________________________________
Help:
   help taskname          - Full help for task
   help par.parametername - Full help for parameter name
   apropos string         - List tasks with string in their brief help.
   tasklist               - Task list organized by catagory
   taskhelp               - One line summary of available tasks
   toolhelp               - One line summary of available tools
   startup                - The start up screen
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
      print ' sm : Simulation utilities'
      print ' vp : Voltage pattern/primary beam utilties'
      print ' ---'
      print ' pl : pylab functions (e.g., pl.title, etc)'
      print ' ---'


def tasklist():
      """ List tasks, organized by catagory """
      print 'Available tasks: \n'
      print 'Import/Export    Information  Data Editing  Display/Plot'
      print '-------------    -----------  ------------  ------------'
      print 'importvla        listcal      (fixvis)      clearplot'
      print '(importasdm)     listhistory  flagautocorr  plotants '
      print 'importfits       listobs      flagdata      plotcal'    
      print 'importuvfits     listvis      flagmanager   plotms'
      print 'exportfits       imhead       plotms        plotxy'
      print 'exportuvfits     imstat       plotxy        viewer'
      print '                 vishead'
      print ''
      print 'Calibration      Imaging      Modelling     Utility'
      print '-----------      -------      ---------     -------'
      print 'accum            clean        setjy         help task'
      print 'applycal         deconvolve   uvcontsub     help par.parameter'
      print 'bandpass         feather      (uvcontsub2)  taskhelp'
      print 'blcal            ft           uvmodelfit    tasklist'
      print 'gaincal          invert                     browsetable'
      print 'fluxscale        makemask                   clearplot'
      print '(fringecal)      mosaic                     clearstat'
      print 'clearcal         widefield                  concat'
      print 'listcal                                     filecatalog'
      print 'smoothcal                                   startup'
      print 'polcal                                      split'
      print 'hanningsmooth                               fixvis'
      print '                                                 '
      print ''
      print 'Image Analysis   Simulation   Single Dish'
      print '--------------   ----------   -----------'
      print 'imcontsub        simdata      sdaverage'
      print 'imhead                        sdbaseline'
      print 'immath                        sdcal'
      print 'immoments                     sdcoadd'
      print 'imregrid                      sdfit'
      print 'imsmooth                      sdflag'
      print 'imstat                        sdlist'
      print 'imval                         sdplot'
      print '(specfit)                     sdsave'
      print '                              sdscale'
      print '                              sdsmooth'
      print '                              sdstat'
      if globals().has_key('mytasks') :
         print ''
         print 'User defined tasks'
         print '------------------'
         for key in mytasks.keys() :
             print key
