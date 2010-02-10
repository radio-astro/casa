from tasksinfo import *
def startup():
# startup guide
      """ Start up screen for CASA """
      print """___________________________________________________________
Available tasks:

   accum	  flagmanager	    listobs	      sdscale
   applycal	  fluxscale	    listvis	      sdsim
   bandpass	  ft		    makemask	      sdsmooth
   blcal	  gaincal	    plotants	      sdstat
   browsetable	  gencal	    plotcal	      sdtpimaging
   calstat	  hanningsmooth	    plotmsl	      setjyt
   clean	  imcontsub	    plotxy	      smoothcal
   clearcal	  imfit		    sdaverage	      specfit
   clearplot	  imhead	    sdbaseline	      split
   clearstat	  immoments	    sdcal	      tget
   concat	  importfits	    sdcoadd	      tput
   deconvolve	  importuvfits	    sdfit	      uvcontsub
   exportfits	  importvla	    sdflag	      uvcontsub2
   exportuvfits	  imregrid	    sdimaging	      uvmodelfit
   feather	  imsmooth	    sdimprocess	      viewer
   filecatalog	  imstat	    sdlist	      vishead
   find		  imval		    sdmath	      visstat
   fixvis	  listcal	    sdplot
   flagdata	  listhistory	    sdsave


                
sd* tasks are available after asap_init() is run

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
      print ' sd : (after running asap_init()) Single dish utilities'
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
      print '                 visstat'
      print ''
      print 'Calibration      Imaging      Modelling     Utility'
      print '-----------      -------      ---------     -------'
      print 'accum            clean        setjy         help task'
      print 'applycal         deconvolve   uvcontsub     help par.parameter'
      print 'bandpass         feather      (uvcontsub2)  taskhelp'
      print 'blcal            ft           uvmodelfit    tasklist'
      print 'calstat          makemask                   browsetable'
      print 'gaincal          mosaic                     clearplot'
      print 'fluxscale        widefield                  clearstat'
      print '(fringecal)                                 concat'
      print 'clearcal                                    filecatalog'
      print 'listcal                                     startup'
      print 'smoothcal                                   split'
      print 'polcal                                      fixvis'
      print 'hanningsmooth'
      print '                                                 '
      print ''
      print 'Image Analysis   Simulation   Single Dish (after running asap_init())'
      print '--------------   ----------   -----------'
      print 'imcontsub        simdata      sdaverage'
      print 'imhead                        sdbaseline'
      print 'immath                        sdcal'
      print 'immoments                     sdcoadd'
      print 'imregrid                      sdfit'
      print 'imsmooth                      sdflag'
      print 'imstat                        sdimaging'
      print 'imval                         sdimprocess'
      print '(specfit)                     sdlist'
      print '                              sdmath'
      print '                              sdplot'
      print '                              sdsave'
      print '                              sdscale'
      print '                              (sdsim)'
      print '                              sdsmooth'
      print '                              sdstat'
      print '                              sdtpimaging'
      if globals().has_key('mytasks') :
         print ''
         print 'User defined tasks'
         print '------------------'
         for key in mytasks.keys() :
             print key
