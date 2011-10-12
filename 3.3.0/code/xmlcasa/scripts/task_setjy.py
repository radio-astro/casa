import os
import sys
from taskinit import *

def setjy(vis=None, field=None, spw=None,
          selectdata=None, timerange=None, scan=None, observation=None,
          modimage=None, listmodimages=None,
          scalebychan=None, fluxdensity=None, spix=None, reffreq=None,
          standard=None):
       """Fills the model column for flux density calibrators."""
       retval = True
       try:
         casalog.origin('setjy')

         def findCalModels(target='CalModels',
                           roots=['.', casa['dirs']['data']],
                           permexcludes = ['.svn', 'regression', 'ephemerides',
                                           'geodetic', 'gui'],
                           exts=['.ms', '.im', '.tab']):
             """
             Returns a list of directories ending in target that are in the
             trees of roots.

             Because casa['dirs']['data'] can contain a lot, and CASA tables
             are directories,and CASA tables are directories, branches matching
             permexcludes or exts are excluded for speed.
             """
             retlist = []
             for root in roots:
                 # Do a walk to find target directories in root.
                 # 7/5/2011:
                 # glob('/export/data_1/casa/gnuactive/data/*/CalModels/*') doesn't
                 # work.
                 for path, dirs, fnames in os.walk(root, followlinks=True):
                     excludes = permexcludes[:]
                     for ext in exts:
                         excludes += [d for d in dirs if ext in d]
                     for d in excludes:
                         if d in dirs:
                             dirs.remove(d)
                     if path.split('/')[-1] == target:
                         retlist.append(path)
             return retlist             

         if listmodimages:
             casalog.post("Listing modimage candidates (listmodimages == True).")
             casalog.post("%s is NOT being modified." % vis)

             def lsmodims(path, modpat='*', header='Candidate modimages'):
                 """
                 Does an ls -d of files or directories in path matching modpat.

                 header describes what is being listed.
                 """
                 if os.path.isdir(path):
                     print "\n%s (%s) in %s:" % (header, modpat, path)
                     sys.stdout.flush()
                     os.system('cd ' + path + ';ls -d ' + modpat)

             lsmodims('.', modpat='*.im* *.mod*')
             calmoddirs = findCalModels()
             for d in calmoddirs:
                 lsmodims(d)
         else:
             myim = imtool.create()
             myms = mstool.create()

             if type(vis) == str and os.path.isdir(vis):
                 myim.open(vis, usescratch=True)
             else:
                 raise Exception, 'Visibility data set not found - please verify the name'

             # If modimage is not an absolute path, see if we can find exactly
             # 1 match in the likely places.
             if modimage and modimage[0] != '/':
                 cwd = os.path.abspath('.')
                 calmoddirs = [cwd]
                 calmoddirs += findCalModels(roots=[cwd,
                                                    casa['dirs']['data']])
                 candidates = []
                 for calmoddir in calmoddirs:
                     cand = calmoddir + '/' + modimage
                     if os.path.isdir(cand):
                         candidates.append(cand)
                 if not candidates:
                     casalog.post("%s was not found for modimage in %s." %(modimage,
                                                                           ', '.join(calmoddirs)),
                                  'SEVERE')
                     return False
                 elif len(candidates) > 1:
                     casalog.post("More than 1 candidate for modimage was found:",
                                  'SEVERE')
                     for c in candidates:
                         casalog.post("\t" + c, 'SEVERE')
                     casalog.post("Please pick 1 and use the absolute path (starting with /).",
                                  'SEVERE')
                     return False
                 else:
                     modimage = candidates[0]
                     casalog.post("Using %s for modimage." % modimage, 'INFO')

             # Write the parameters to HISTORY before the tool writes anything.
             try:
                    param_names = setjy.func_code.co_varnames[:setjy.func_code.co_argcount]
                    param_vals = [eval(p) for p in param_names]   
                    retval &= write_history(myms, vis, 'setjy', param_names,
                                            param_vals, casalog)
             except Exception, instance:
                    casalog.post("*** Error \'%s\' updating HISTORY" % (instance),
                                 'WARN')

             #if observation or scan or timerange:
             #       myim.selectvis(vis='', observation=observation, scan=scan, time=timerange)

             myim.setjy(field=field, spw=spw, modimage=modimage,
                        fluxdensity=fluxdensity, spix=spix, reffreq=reffreq,
                        standard=standard, scalebychan=scalebychan, time=timerange,
                        observation=str(observation), scan=scan)
             myim.close()
       except Exception, instance:
              print '*** Error ***',instance
       return retval

