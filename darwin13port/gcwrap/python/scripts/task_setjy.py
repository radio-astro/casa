from glob import glob
import os
import sys
import shutil
from setjy_helper import * 
from taskinit import *
from parallel.parallel_task_helper import ParallelTaskHelper
import pdb

def setjy(vis=None, field=None, spw=None,
          selectdata=None, timerange=None, scan=None, intent=None, observation=None,
          scalebychan=None, standard=None, model=None, modimage=None, 
          listmodels=None, fluxdensity=None, spix=None, reffreq=None, fluxdict=None, 
        #  commented out until polarization fraction/angle handling in place
        #  polindex=None, polangle=None, rm=None,
          useephemdir=None, interpolation=None, usescratch=None):
    """Fills the model column for flux density calibrators."""

    casalog.origin('setjy')
    casalog.post("standard="+standard)
    # Take care of the trivial parallelization
    if ( not listmodels and ParallelTaskHelper.isParallelMS(vis) and usescratch):
        # jagonzal: We actually operate in parallel when usescratch=True because only
        # in this case there is a good trade-off between the parallelization overhead
        # and speed up due to the load involved with MODEL_DATA column creation
        helper = ParallelTaskHelper('setjy', locals())
        retval = helper.go()           
    else:
        retval = setjy_core(vis, field, spw, selectdata, timerange, 
                        scan, intent, observation, scalebychan, standard, model, 
                        modimage, listmodels, fluxdensity, spix, reffreq, fluxdict, 
                        # polindex, polangle, rm, 
                         useephemdir, interpolation, usescratch)

    #pdb.set_trace()
    return retval


def setjy_core(vis=None, field=None, spw=None,
               selectdata=None, timerange=None, scan=None, intent=None, observation=None,
               scalebychan=None, standard=None, model=None, modimage=None, listmodels=None,
               fluxdensity=None, spix=None, reffreq=None, fluxdict=None,
                # polarization handling...
                # polindex=None, polangle=None, rm=None,
               useephemdir=None, interpolation=None, usescratch=None):
    """Fills the model column for flux density calibrators."""

    #retval = True
    clnamelist=[]

    try:
        # Here we only list the models available, but don't perform any operation
        if listmodels:
            retval=True
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
        
        # Actual operation, when either the MODEL_DATA column or visibility model header are set
        else:
            if not os.path.isdir(vis):
              #casalog.post(vis + " must be a valid MS unless listmodels is True.",
              #             "SEVERE")
                raise Exception, "%s is not a valid MS" % vis 
                #return False

            myms = mstool()
            myim = imtool()

            if type(vis) == str and os.path.isdir(vis):
                n_selected_rows = nselrows(vis, field, spw, observation, timerange, scan, intent, usescratch)
                # jagonzal: When  usescratch=True, creating the MODEL column only on a sub-set of
                # Sub-MSs causes problems because ms::open requires all the tables in ConCatTable 
                # to have the same description (MODEL data column must exist in all Sub-MSs)
                #
                # This is a bit of an over-doing but it is necessary for the sake of transparency
                #
                # Besides, this is also the same behavior as when running setjy vs a normal MS
                #
                # Finally, This does not affect the normal MS case because nselrows throws an
                # exception when the user enters an invalid data selection, but it works for the 
                # MMS case because every sub-Ms contains a copy of the entire MMS sub-tables
                if ((not n_selected_rows) and ((not usescratch) or (standard=="Butler-JPL-Horizons 2012"))) :
                    # jagonzal: Turn this SEVERE into WARNING, as explained above
                    casalog.post("No rows were selected.", "WARNING")
                    return True
                else:
                    if (not n_selected_rows):
                        raise Exception, "No rows were selected. Please check your data selection"
                    myim.open(vis, usescratch=usescratch)

            else:
                raise Exception, 'Visibility data set not found - please verify the name'

            if modimage==None:  # defined as 'hidden' with default '' in the xml
      	                        # but the default value does not seem to set so deal
			        # with it here...
                modimage=''
            if model:
                modimage=model
            elif not model and modimage:
                casalog.post("The modimage parameter is deprecated please use model instead", "WARNING")
            # If modimage is not an absolute path, see if we can find exactly 1 match in the likely places.
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
                                 ', '.join(calmoddirs)), 'SEVERE')
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
                #retval &= write_history(myms, vis, 'setjy', param_names,
                retval = write_history(myms, vis, 'setjy', param_names,
                                    param_vals, casalog)
            except Exception, instance:
                casalog.post("*** Error \'%s\' updating HISTORY" % (instance),
                         'WARN')

            # Split the process for solar system objects
            # To maintain the behavior of SetJy such that if a flux density is specified
            # we use it rather than the model, we need to check the state of the 
            # input fluxdensity  JSK 10/25/2012
            # TT comment 07/01/2013: some what redundant as fluxdensity is moved to 
            # a subparameter but leave fluxdensity=-1 for backward compatibility
            userFluxDensity = fluxdensity is not None
            if userFluxDensity and isinstance(fluxdensity, list):
                userFluxDensity = fluxdensity[0] > 0.0
            elif userFluxDensity:
                userFluxDensity = fluxdensity > 0.0

            if standard=="Butler-JPL-Horizons 2012" and not userFluxDensity:
                casalog.post("Using Butler-JPL-Horizons 2012")
                ssmoddirs = findCalModels(target='SolarSystemModels',
                          roots=[casa['dirs']['data']],
                          exts=['.im','.ms','tab'])
                if ssmoddirs==set([]):
                    raise Exception, "Missing Tb models in the data directory"

                setjyutil=ss_setjy_helper(myim,vis,casalog)
                retval=setjyutil.setSolarObjectJy(field=field,spw=spw,scalebychan=scalebychan,
                             timerange=timerange,observation=str(observation), scan=scan, 
                             intent=intent, useephemdir=useephemdir,usescratch=usescratch)
                clnamelist=setjyutil.getclnamelist()
            else:
                # Need to branch out the process for fluxscale since the input dictionary may 
                # contains multiple fields. Since fluxdensity parameter is just a vector contains 
                # IQUV flux densities and so need to run im.setjy per field 
                if standard=="fluxscale": 
                    instandard="Perley-Butler 2010"
                    # function to return selected field, spw, etc
                    fieldidused=parse_fluxdict(fluxdict, vis, field, spw, observation, timerange,
                                           scan, intent, usescratch)

                    if len(fieldidused):
                        retval={}
                        for selfld in fieldidused:
                            selspix=fluxdict[selfld]["spidx"][1]  # setjy only support alpha for now
                            # set all (even if fluxdensity = -1
                            if spw=='':
                                selspw = [] 
                                invalidspw = []
                                for ispw in fluxdict["spwID"].tolist():
                                    # skip spw if fluxd=-1
                                    if fluxdict[selfld][str(ispw)]["fluxd"][0]>-1:
                                        selspw.append(ispw)
                                    else:
                                        invalidspw.append(ispw)
                                if len(invalidspw):
                                    casalog.post("Fluxdict does not contains valid fluxdensity for spw="+
                                                 str(invalidspw)+ 
                                                 ". Model will not set for those spws.", "WARN") 
                            else: # use spw selection
                                selspw = ms.msseltoindex(vis,spw=spw)["spw"].tolist()
                    
                            if fluxdict[selfld]["fitFluxd"]:
                                selfluxd = fluxdict[selfld]["fitFluxd"]
                                selreffreq = fluxdict[selfld]["fitRefFreq"] 
                            else:
                                # use first selected spw's flux density 
                                selfluxd = fluxdict[selfld][str(selspw[0])]['fluxd']
                                # assuming the fluxscale reporting the center freq of a given spw
                                selreffreq=fluxdict["freq"][selspw[0]] 
                            casalog.post("Use fluxdensity=%s, reffreq=%s, spix=%s" %
                                     (selfluxd,selreffreq,selspix)) 
                            curretval=myim.setjy(field=selfld,spw=selspw,modimage=modimage,
                                                 fluxdensity=selfluxd, spix=selspix, reffreq=selreffreq, 
                                                 standard=instandard, scalebychan=scalebychan,
                                                 time=timerange, observation=str(observation), scan=scan, 
                                                 intent=intent, interpolation=interpolation)
                        retval.update(curretval)
                    else:
                        raise Exception, "No field is selected. Check fluxdict and field selection."
                else: 
                    if standard=="manual":
                        instandard="Perley-Butler 2010" # default standard when fluxdensity=-1 or 1
                    else:
                        instandard=standard
                    if spix==[]: # handle the default 
                        spix=0.0
                    # need to modify imager to accept double array for spix
                    retval=myim.setjy(field=field, spw=spw, modimage=modimage, fluxdensity=fluxdensity, 
                                      spix=spix, reffreq=reffreq, standard=instandard, scalebychan=scalebychan, 
                                      time=timerange, observation=str(observation), scan=scan, intent=intent, 
                                      interpolation=interpolation)

            myim.close()

    # This block should catch errors mainly from the actual operation mode 
    except Exception, instance:
        casalog.post('%s' % instance,'SEVERE')
        #retval=False
        raise instance

    finally:
        if standard=='Butler-JPL-Horizons 2012':
            for cln in clnamelist:
                if os.path.exists(cln) and os.path.isdir(cln):
                    shutil.rmtree(cln,True) 

    return retval


def better_glob(pats):
    """
    Unlike ls, glob.glob('pat1 pat2') does not return  
    the union of matches to pat1 and pat2.  This does.
    """
    retset = set([])
    patlist = pats.split()
    for p in patlist:
        retset.update(glob(p))
    return retset
  

def lsmodims(path, modpat='*', header='Candidate modimages'):
    """
    Does an ls -d of files or directories in path matching modpat.
  
    Header describes what is being listed.
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
      # 7/5/2011: glob('/export/data_1/casa/gnuactive/data/*/CalModels/*') doesn't work.
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


def nselrows(vis, field='', spw='', obs='', timerange='', scan='', intent='', usescratch=None):

    # modified to use ms.msselect. If no row is selected ms.msselect will
    # raise an exception  - TT 2013.12.13
    retval = 0
    myms = mstool()

    #msselargs = {'vis': vis}
    msselargs = {}
    if field:
        msselargs['field'] = field
    if spw:
        msselargs['spw'] = spw
    if intent:
       msselargs['scanintent'] = intent

  # only applicable for usescratch=T
    if usescratch:
        if obs:
            if not type(obs)==string:
               sobs = str(obs)
            msselargs['observation'] = sobs
        if timerange:
            msselargs['time'] = timerange
        if scan:
            msselargs['scan'] = scan
    else:
        warnstr='Data selection by '
        datasels=[]
        if timerange: datasels.append('timerange')
        if scan: datasels.append('scan') 
        if obs: datasels.append('observation') 
        if len(datasels):
            warnstr+=str(datasels)+' will be ignored for usescartch=False'
            casalog.post(warnstr,'WARN')
 
    # === skip this use ms.msselect instead ====
    # ms.msseltoindex only goes by the subtables - it does NOT check
    # whether the main table has any rows matching the selection.
#    try:
#        selindices = myms.msseltoindex(**msselargs)
#        print "msselargs=",msselargs," selindices=",selindices
#    except Exception, instance:
#        casalog.post('nselrowscore exception: %s' % instance,'SEVERE')
#        raise instance
#    query = []
#    if field:
#        query.append("FIELD_ID in " + str(selindices['field'].tolist()))
#    if spw:
#        query.append("DATA_DESC_ID in " + str(selindices['spw'].tolist()))
#    if obs and usescratch:
#        query.append("OBSERVATION_ID in " + str(selindices['obsids'].tolist()))

    # I don't know why ms.msseltoindex takes a time argument 
    # - It doesn't seem to appear in the output.
    
#    if scan and usescratch:
#        query.append("SCAN_NUMBER in " + str(selindices['scan'].tolist()))

    #if timerange and usescratch:
    #    query.append("TIME in [select TIME where 

    # for intent (OBS_MODE in STATE subtable), need a subquery part...
#    if intent:
#        query.append("STATE_ID in [select from ::STATE where OBS_MODE==pattern(\""+\
#                  intent+"\") giving [ROWID()]]")
#    print "query=",query
#    mytb = tbtool()
#    mytb.open(vis)
    myms = mstool()
    myms.open(vis)

    if (len(msselargs)==0):
        retval = myms.nrow()
        myms.close()
    else:
        try:
#            st = mytb.query(' and '.join(query),style='python')  # Does style matter here?
#            retval = st.nrows()
#            st.close() # needed to clear tablecache? 
#            mytb.close()
            myms.msselect(msselargs)
            retval = myms.nrow()
            myms.close()
        except Exception, instance:
            casalog.post('nselrowscore exception: %s' % instance,'SEVERE')
            myms.close()
            raise Exception, instance

    return retval

def parse_fluxdict(fluxdict, vis, field='', spw='', observation='', timerange='', scan='', intent='', usescratch=None):
    """
    Parser function for fluxdict (dictionary output from fluxscale) to set
    fluxdensity, spix, and reffreq parameters (as in 'manual' mode)
    """
    # set spix and reffreq if there  

    #(myms,mymsmd) = gentools(['ms','msmd'])
    myms, = gentools(['ms'])

    # dictionary for storing modified (taking AND between the data selection
    # and fluxdict content)
    modmsselargs={}
                    
    msselargs={}
    msselargs['field']=field 
    msselargs['spw']=spw 
    msselargs['scanintent']=intent
    # only applicable for usescratch=T
    if usescratch:
        if obs:
            msselargs['observation'] = obs
        if timerange:
           msselargs['time'] = timerange
        if scan:
           msselargs['scan'] = scan

    try:
        myms.open(vis)
        myms.msselect(msselargs)
        selindices = myms.msselectedindices()
    except Exception, instance:
        casalog.post('parse_fluxdict exception: %s' % instance,'SEVERE')
        raise instance
    finally:
        myms.close()

    # check if fluxdict is valid
    if fluxdict=={}:
        raise Exception, "fluxdict is empty"
    else:
        msg=""
        if not fluxdict.has_key("freq"):
             msg+="freq "
        if not fluxdict.has_key("spwID"):
             msg+="spwID "
        if len(msg):
             raise Exception, "Input fluxdict is missing keywords:"+msg

    # select fields only common to the dictionary and field selection
    fieldids=[]
    for ky in fluxdict.keys():
        try:
            int(ky) 
            fieldids.append(ky) # list in string field ids
        except:
            pass
    if not len(fieldids):
        casalog.post('No field ids was found in the dictionary given for fluxdict. Please check the input.', 'SEVERE')
       

    if len(selindices['field']):
        # also check there is common field id, note field ids in selindices are in numpy.array
        selfieldids = [fd for fd in fieldids if int(fd) in selindices['field'].tolist()]
        if not len(selfieldids):
            raise Exception, "No field was found in fluxdict for the given field selection"
    else:
        selfieldids = fieldids   

    return selfieldids 
