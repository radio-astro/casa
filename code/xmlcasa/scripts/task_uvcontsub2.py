import os
import re
import shutil
import tempfile

# shutil.copytree is useless with directories created by tempfile
# (or any directories that already exist).
from distutils.dir_util import copy_tree

from taskinit import *
from update_spw import *

mycb, myms, mytb = gentools(['cb', 'ms', 'tb'])

def uvcontsub2(vis, field, fitspw, combine, solint, fitorder, spw, want_cont):
    try:
        casalog.origin('uvcontsub2')

        # Get these checks done and out of the way.
        # This one is redundant - it is already checked at the XML level.
        if not os.path.isdir(vis):
            raise Exception, 'Visibility data set not found - please verify the name'
        if 'spw' not in combine:
            spwmfitspw = subtract_spws(spw, fitspw)
            if spwmfitspw == 'UNKNOWN':
                # Rats.  We need to make '' explicit.
                mytb.open(vis + '/SPECTRAL_WINDOW')
                spwmfitspw = subtract_spws('0~' + str(mytb.nrows() - 1), fitspw)
                mytb.close()
            if spwmfitspw:
                raise Exception, "combine must include 'spw' when the fit is being applied to spws outside fitspw."
        
        # cb will put the continuum-subtracted data in CORRECTED_DATA, so
        # protect vis by splitting out its relevant parts to a working copy.
        csvis = vis.rstrip('/') + '.contsub'
        
        # The working copy will need all of the channels in fitspw + spw, so we
        # may or may not need to filter it down to spw at the end.
        myfitspw = fitspw
        myspw = spw

        tempspw = join_spws(fitspw, spw)
#        if 'spw' not in combine:
#            # We only need the spws (but potentially all the channels) listed
#            # in fitspw.
#            tempspw = re.sub(r':[^,]+', '', fitspw)
        if tempspw:
            # The split will reindex the spws.  Update spw and fitspw.
            # Do fitspw first because the spws in spw are supposed to be
            # a subset of the ones in fitspw.
            casalog.post('split is being run internally, and the selected spws')
            casalog.post('will be renumbered to start from 0 in the output!')

            # Initialize spwmap.
            spwmap = update_spw(tempspw, None)[1]

            # Now get myfitspw.
            myfitspw = update_spw(fitspw, spwmap)[0]
            myspw = update_spw(spw, spwmap)[0]

        final_csvis = csvis
        workingdir = os.path.abspath(os.path.dirname(vis.rstrip('/')))
        csvis = tempfile.mkdtemp(prefix=csvis.split('/')[-1], dir=workingdir)

        # ms does not have a colnames method, so open vis with tb even though
        # it is already open with myms.  Note that both use nomodify=True,
        # however, and no problem was revealed in testing.
        mytb.open(vis, nomodify=True)
        if 'CORRECTED_DATA' in mytb.colnames():
            whichcol = 'CORRECTED_DATA'
        else:
            # DON'T remind the user that split before uvcontsub2 wastes time -
            # scratch columns will eventually go away.
            whichcol = 'DATA'
        mytb.close()

        casalog.post('Preparing to add scratch columns.')
        if whichcol != 'DATA' or tempspw != '':
            casalog.post('splitting to ' + csvis + ' with spw="'
                         + tempspw + '"')
            myms.open(vis, nomodify=True)
            myms.split(csvis, spw=tempspw, whichcol=whichcol)
            myms.close()
        else:
            # This takes almost 30s/GB.  (lustre, 8/2011)
            casalog.post('Copying ' + vis + ' to ' + csvis + ' with cp.')
            copy_tree(vis, csvis)
        
        if (type(csvis) == str) and os.path.isdir(csvis):
            mycb.open(csvis)
        else:
            raise Exception, 'Visibility data set not found - please verify the name'

        ## for now, forbid requests for fitorder>0 
        #if (fitorder>0):
        #    raise Exception, "Sorry, uvcontsub2 currently only supports fitorder=0."
        
        # select the data for continuum subtraction
        mycb.reset()
        mycb.selectvis(spw=myfitspw, field=field)

        # Arrange apply of existing other calibrations
        # First do the existing cal tables...
        #ngaintab = 0;
        #if (gaintable!=['']):
        #    ngaintab=len(gaintable)
        #ngainfld = len(gainfield)
        #nspwmap = len(spwmap)
        #ninterp = len(interp)

        # handle list of list issues with spwmap
        #if (nspwmap>0):
        #       if (type(spwmap[0])!=list):
        #           # first element not a list, only one spwmap specified
        #           # make it a list of list
        #           spwmap=[spwmap];
        #           nspwmap=1;

        #for igt in range(ngaintab):
        #    if (gaintable[igt]!=''):

        #        # field selection is null unless specified
        #        thisgainfield=''
        #        if (igt<ngainfld):
        #            thisgainfield=gainfield[igt]

        #           # spwmap is null unless specifed
        #           thisspwmap=[-1]
        #           if (igt<nspwmap):
        #               thisspwmap=spwmap[igt];

        #           # interp is 'linear' unless specified
        #           thisinterp='linear'
        #           if (igt<ninterp):
        #               if (interp[igt]==''):
        #                   interp[igt]=thisinterp
        #               thisinterp=interp[igt]
                    
        #           mycb.setapply(t=0.0,table=gaintable[igt],field=thisgainfield,
        #               calwt=True,spwmap=thisspwmap,interp=thisinterp)

        # ...and now the specialized terms
        # (BTW, interp irrelevant for these, since they are evaluated)
        #if (opacity>0.0): mycb.setapply(type='TOPAC',t=-1,opacity=opacity,calwt=True)
        #if gaincurve: mycb.setapply(type='GAINCURVE',t=-1,calwt=True)

        # Apply parallactic angle, if requested
        #if parang: mycb.setapply(type='P')

        # Set up the solve
        amuellertab = tempfile.mkdtemp(prefix='Temp_contsub.tab',
                                       dir=workingdir)

        mycb.setsolve(type='A', t=solint, table=amuellertab, combine=combine,
                    fitorder=fitorder)

        # solve for the continuum
        mycb.solve()

        # subtract the continuum
        mycb.selectvis(field=field, spw=myspw)
        aspwmap=-1
        # if we combined on spw in solve, fanout the result with spwmap=-999;
        if 'spw' in combine:
            aspwmap = -999
        mycb.setapply(table=amuellertab, spwmap=aspwmap)

        # Generate CORRECTED_DATA without continuum
        mycb.correct()

        if want_cont:
            # Generate MODEL_DATA with only the continuum model
            mycb.corrupt()

        mycb.close()

        # Delete the temporary caltable
        shutil.rmtree(amuellertab)

        # Move the line data from CORRECTED_DATA to DATA, and do any
        # final filtering by spw.
        myms.open(csvis)
        # Using ^ in spw is untested here!
        myms.split(final_csvis, spw=myspw, whichcol='corrected')
        myms.close()

        # Not a dict, because we want to maintain the order.
        param_names = uvcontsub2.func_code.co_varnames[:uvcontsub2.func_code.co_argcount]
        param_vals = [eval(p) for p in param_names]
            
        write_history(myms, final_csvis, 'uvcontsub2', param_names, param_vals,
                      casalog)

        #casalog.post("\"want_cont\" = \"%s\"" % want_cont, 'WARN')
        #casalog.post("\"csvis\" = \"%s\"" % csvis, 'WARN')
        if want_cont:
            myms.open(csvis)
            # No channel averaging (== skipping for fitorder == 0) is done
            # here, but it would be a reasonable option.  The user can always
            # do it by running split again.
            myms.split(final_csvis[:-3],             # .contsub -> .cont
                       whichcol='MODEL_DATA',
                       spw=myspw)
            myms.close()
            write_history(myms, final_csvis[:-3], 'uvcontsub2', param_names,
                          param_vals, casalog)

        #casalog.post("rming \"%s\"" % csvis, 'WARN')
        shutil.rmtree(csvis)

    except Exception, instance:
        casalog.post('Error in uvcontsub2: ' + str(instance), 'SEVERE')
        mycb.close()                        # Harmless if cb is closed.
        raise Exception
