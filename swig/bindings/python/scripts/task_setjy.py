from glob import glob
import os
import sys
import shutil
from setjy_helper import * 
from taskinit import *
from parallel.parallel_task_helper import ParallelTaskHelper

def setjy(vis=None, field=None, spw=None,
          selectdata=None, timerange=None, scan=None, observation=None,
          modimage=None, listmodels=None,
          scalebychan=None, fluxdensity=None, spix=None, reffreq=None,
          standard=None, usescratch=None):
  """Fills the model column for flux density calibrators."""
  retval = True
  try:
    casalog.origin('setjy')

    if listmodels:
      casalog.post("Listing model candidates (listmodels == True).")
      if vis:
        casalog.post("%s is NOT being modified." % vis)

      if standard=='Butler-JPL-Horizons 2012':
        ssmoddirs = findCalModels(target='SolarSystemModels',
                  roots=[casa['dirs']['data']],
                  exts=['.im','.ms','tab'])
        if ssmoddirs==set([]):
          casalog.post("No models were found. Missing SolarSystemModels in the CASA data directory","WARN")           
        for d in ssmoddirs:
          lsmodims(d,modpat='*Tb.dat', header='Tb models of solar system objects available for %s' % standard) 
      elif standard=='Butler-JPL-Horizons 2010':
        availmodellist=['Venus', 'Mars', 'Jupiter', 'Uranus', 'Neptune', 'Pluto',
                        'Io', 'Europa', 'Ganymede', 'Callisto', 'Titan','Triton',
                        'Ceres', 'Pallas', 'Vesta', 'Juno', 'Victoria', 'Davida']
        print "Solar system objects recognized by %s:" % standard
        print availmodellist 
      else:
        lsmodims('.', modpat='*.im* *.mod*')
        calmoddirs = findCalModels()
        ssmoddirs=None
        for d in calmoddirs:
          lsmodims(d)
    else:
      if not os.path.isdir(vis):
        casalog.post(vis + " must be a valid MS unless listmodels is True.",
                     "SEVERE")
        return False

      myms = mstool()
      myim = imtool()

      if type(vis) == str and os.path.isdir(vis):
        n_selected_rows = nselrows(vis, field, spw, observation, timerange,
                                   scan)

        if not n_selected_rows:
          casalog.post("No rows were selected.", "SEVERE")
          return False
        elif isinstance(n_selected_rows, dict):
        #if ParallelTaskHelper.isParallelMS(vis):
          # 11/28/2011: task_flagdata makes parellization look really easy, but it
          # doesn't actually work on either of the computers I tried.
          # Take care of the trivial parallelization
          #helper = ParallelTaskHelper('setjy', locals())
          #helper.go()
          #return

          # setjy should only operate on member MSes that have the selection anyway.
          for m in n_selected_rows:
            if n_selected_rows[m] > 0:
              myim.selectvis(vis=m, usescratch=usescractch)
        else:
          myim.open(vis, usescratch=usescratch)
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

      # split the process for solar system objects
      if standard=="Butler-JPL-Horizons 2012":
        casalog.post("Using Butler-JPL-Horizons 2012")
        ssmoddirs = findCalModels(target='SolarSystemModels',
                  roots=[casa['dirs']['data']],
                  exts=['.im','.ms','tab'])
        if ssmoddirs==set([]):
          raise Exception, "Missing Tb models in the data directory"

        setjyutil=ss_setjy_helper(myim,vis,casalog)
        setjyutil.setSolarObjectJy(field=field,spw=spw,scalebychan=scalebychan,
                         timerange=timerange,observation=str(observation), scan=scan)
      else:
        myim.setjy(field=field, spw=spw, modimage=modimage,
                 fluxdensity=fluxdensity, spix=spix, reffreq=reffreq,
                 standard=standard, scalebychan=scalebychan, time=timerange,
                 observation=str(observation), scan=scan)
        myim.close()
  except Exception, instance:
    #print '*** Error ***',instance
    casalog.post('%s' % instance,'SEVERE')
    retval = False
  return retval

def better_glob(pats):
  """
  Unlike ls, glob.glob('pat1 pat2') does not return the union of matches to pat1
  and pat2.  This does.
  """
  retset = set([])
  patlist = pats.split()
  for p in patlist:
    retset.update(glob(p))
  return retset
  
def lsmodims(path, modpat='*', header='Candidate modimages'):
  """
  Does an ls -d of files or directories in path matching modpat.
  
  header describes what is being listed.
  """
  if os.path.isdir(path):
    if better_glob(path + '/' + modpat):
      print "\n%s (%s) in %s:" % (header, modpat, path)
      sys.stdout.flush()
      os.system('cd ' + path + ';ls -d ' + modpat)
    else:
      print "\nNo %s matching '%s' found in %s" % (header.lower(),
                                                   modpat, path)

def findCalModels(target='CalModels',
                  roots=['.', casa['dirs']['data']],
                  permexcludes = ['.svn', 'regression', 'ephemerides',
                                  'geodetic', 'gui'],
                  exts=['.ms', '.im', '.tab']):
  """
  Returns a set of directories ending in target that are in the trees of roots.
  
  Because casa['dirs']['data'] can contain a lot, and CASA tables are
  directories, branches matching permexcludes or exts are excluded for speed.
  """
  retset = set([])
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
        retset.add(path)
  return retset             

def nselrows(vis, field='', spw='', obs='', timerange='', scan=''):
  """
  Returns the number of rows in vis selected by field, spw, obs,
  timerange, and scan.  Empty strings or Nones are treated as '*'.

  If vis is a multims, the return value will be a dict keyed by
  the paths of the member MSes.
  """
  retval = 0
  myms = mstool()
  if ParallelTaskHelper.isParallelMS(vis):
    myms.open(vis)
    mses = myms.getreferencedtables()
    myms.close()
    retval = {}
    for m in mses:
      retval[m] = nselrows(m, field, spw, obs, timerange, scan)
  else:
    msselargs = {'vis': vis}
    if field:
      msselargs['field'] = field
    if spw:
      msselargs['spw'] = spw
    if obs:
      msselargs['observation'] = obs
    if timerange:
      msselargs['time'] = timerange
    if scan:
      msselargs['scan'] = scan
      
    # ms.msseltoindex only goes by the subtables - it does NOT check
    # whether the main table has any rows matching the selection.
    selindices = myms.msseltoindex(**msselargs)

    query = []
    if field:
      query.append("FIELD_ID in " + str(selindices['field'].tolist()))
    if spw:
      query.append("DATA_DESC_ID in " + str(selindices['spw'].tolist()))
    if obs:
      query.append("OBSERVATION_ID in " + str(selindices['obsids'].tolist()))

    # I don't know why ms.msseltoindex takes a time argument - it doesn't seem to
    # appear in the output.
    
    if scan:
      query.append("SCAN_NUMBER in " + str(selindices['scan'].tolist()))

    mytb = tbtool()
    mytb.open(vis)
    st = mytb.query(' and '.join(query),
                    style='python')  # Does style matter here?
    mytb.close()
    retval = st.nrows()
  return retval


