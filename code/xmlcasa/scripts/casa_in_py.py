#
import os
import re
import sys
import string
import commands

from get_user import get_user

## Avoid a clash with the command for printing the working directory.
#import pwd as pwd_module 

try:
    import casac
except ImportError, e:
    print "failed to load casa:\n", e
    exit(1)

#import matplotlib


##
## first set up CASAPATH
##
#if os.environ.has_key('CASAPATH') :
#    __casapath__ = os.environ['CASAPATH'].split(' ')[0]
#    if not os.path.exists(__casapath__ + "/data") :
#        raise RuntimeError, "CASAPATH environment variable is improperly set"
#else :
#    __casapath__ = casac.__file__
#    while __casapath__ and __casapath__ != "/" :
#        if os.path.exists( __casapath__ + "/data") :
#            break
#        __casapath__ = os.path.dirname(__casapath__)
#    if __casapath__ and __casapath__ != "/" :
#        os.environ['CASAPATH']=__casapath__ + " linux local host"
#    else :
#        raise RuntimeError, "CASAPATH environment variable must be set"
#
###
### next adjust the PYTHONPATH
###
#if re.match( r'.*/\d+\.\d+\.\d+\w*-\d+$', __casapath__ ) :
#    for root, dirs, files in os.walk(os.path.dirname(__casapath__)):
#        if root.endswith("/numpy"):
#            sys.path.append(os.path.dirname(root))
#            break
#else:
#    for root, dirs, files in os.walk(__casapath__):
#        if root.endswith("/numpy"):
#            sys.path.append(os.path.dirname(root))
#            break
#
###
### next adjust PATH and LD_LIBRARY_PATH
###
#for root, dirs, files in os.walk(__casapath__):
#    if root.endswith("/bin") and "casapyinfo" in files :
#        __ipcontroller__ = (lambda fd: fd.readline().strip('\n'))(os.popen(root + "/casapyinfo --exec 'which ipcontroller'"))
#        if os.path.exists(__ipcontroller__) :
#            os.environ['PATH'] = os.path.dirname(__ipcontroller__) + ":" + os.environ['PATH']
#        else :
#            raise RuntimeError, "cannot configure CASA tasking system"
#        __ld_library_path__ = (lambda fd: fd.readline().strip('\n').split(':'))(os.popen(root + "/casapyinfo --exec 'echo $LD_LIBRARY_PATH'"))
#        map(lambda x: sys.path.append(x),__ld_library_path__)
#        break

#
#from taskinit import *
# cannot do this because taskinit contain
# other functions the engines do not need
# we load tools and tasks explicitly

##
## finally load tools
####

__quantahome__ = casac.homefinder.find_home_by_name('quantaHome')
__measureshome__ = casac.homefinder.find_home_by_name('measuresHome')
__imagerhome__ = casac.homefinder.find_home_by_name('imagerHome')
__calibraterhome__ = casac.homefinder.find_home_by_name('calibraterHome')
__mshome__ = casac.homefinder.find_home_by_name('msHome')
__tableplothome__ = casac.homefinder.find_home_by_name('tableplotHome')
__msplothome__ = casac.homefinder.find_home_by_name('msplotHome')
__calplothome__ = casac.homefinder.find_home_by_name('calplotHome')
__tablehome__ = casac.homefinder.find_home_by_name('tableHome')
__flaggerhome__ = casac.homefinder.find_home_by_name('flaggerHome')
__autoflaghome__ = casac.homefinder.find_home_by_name('autoflagHome')
__imagehome__ = casac.homefinder.find_home_by_name('imageHome')
__imagepolhome__ = casac.homefinder.find_home_by_name('imagepolHome')
__simulatorhome__ = casac.homefinder.find_home_by_name('simulatorHome')
__componentlisthome__ = casac.homefinder.find_home_by_name('componentlistHome')
__coordsyshome__ = casac.homefinder.find_home_by_name('coordsysHome')
__regionmanagerhome__ = casac.homefinder.find_home_by_name('regionmanagerHome')
__utilshome__ = casac.homefinder.find_home_by_name('utilsHome')
__deconvolverhome__ = casac.homefinder.find_home_by_name('deconvolverHome')
__vpmanagerhome__ = casac.homefinder.find_home_by_name('vpmanagerHome')
__vlafillertaskhome__ = casac.homefinder.find_home_by_name('vlafillertaskHome')
__atmospherehome__ = casac.homefinder.find_home_by_name('atmosphereHome')
__loghome__ =  casac.homefinder.find_home_by_name('logsinkHome')
__utilstool__ = casac.homefinder.find_home_by_name('utilsHome')
#__plotmshome__ = casac.homefinder.find_home_by_name('plotmsHome')



casalog = __loghome__.create()
casalog.setglobal(True)
quanta = __quantahome__.create( )
measures = __measureshome__.create( )
imager = __imagerhome__.create( )
calibrater = __calibraterhome__.create( )
ms = __mshome__.create( )
tableplot = __tableplothome__.create( )
msplot = __msplothome__.create( )
calplot = __calplothome__.create( )
table = __tablehome__.create( )
flagger = __flaggerhome__.create( )
autoflag = __autoflaghome__.create( )
image = __imagehome__.create( )
imagepol = __imagepolhome__.create( )
simulator = __simulatorhome__.create( )
componentlist = __componentlisthome__.create( )
coordsys = __coordsyshome__.create( )
regionmanager = __regionmanagerhome__.create( )
utils = __utilshome__.create( )
deconvolver = __deconvolverhome__.create( )
vpmanager = __vpmanagerhome__.create( )
vlafillertask = __vlafillertaskhome__.create( )
atmosphere = __atmospherehome__.create( )
cu = __utilstool__.create()
#plotms = __plotmshome__.create( )


from imstat_pg import imstat_pg as imstat
from fringecal_pg import fringecal_pg as fringecal
from flagdata_pg import flagdata_pg as flagdata
from split_pg import split_pg as split
from importvla_pg import importvla_pg as importvla
from hanningsmooth_pg import hanningsmooth_pg as hanningsmooth
from widefield_pg import widefield_pg as widefield
from immoments_pg import immoments_pg as immoments
from uvcontsub_pg import uvcontsub_pg as uvcontsub
from listhistory_pg import listhistory_pg as listhistory

from mosaic_pg import mosaic_pg as mosaic
from importfits_pg import importfits_pg as importfits
from imfit_pg import imfit_pg as imfit
from deconvolve_pg import deconvolve_pg as deconvolve

from simdata_pg import simdata_pg as simdata
from simdata2_pg import simdata2_pg as simdata2

from importasdm_pg import importasdm_pg as importasdm
from uvmodelfit_pg import uvmodelfit_pg as uvmodelfit
#from polcal_pg import polcal_pg as polcal

from clearcal_pg import clearcal_pg as clearcal
from bandpass_pg import bandpass_pg as bandpass
#from clearplot_pg import clearplot_pg as clearplot
from listvis_pg import listvis_pg as listvis_cl
from accum_pg import accum_pg as accum
from concat_pg import concat_pg as concat
from listobs_pg import listobs_pg as listobs
from feather_pg import feather_pg as feather
from imregrid_pg import imregrid_pg as imregrid
from listcal_pg import listcal_pg as listcal
#from plotxy_pg import plotxy_pg as plotxy
from ft_pg import ft_pg as ft
from importuvfits_pg import importuvfits_pg as importuvfits
from setjy_pg import setjy_pg  as setjy
from clean_pg import clean_pg as clean
from immath_pg import immath_pg as immath
from imhead_pg import imhead_pg as imhead
from find_pg import find_pg as find
from gaincal_pg import gaincal_pg as gaincal
from fluxscale_pg import fluxscale_pg as fluxscale
from applycal_pg import applycal_pg as applycal
#from plotants_pg import plotants_pg as plotants
from exportuvfits_pg import exportuvfits_pg as exportuvfits
#from plotcal_pg import plotcal_pg as plotcal
from flagmanager_pg import flagmanager_pg as flagmanager
from specfit_pg import specfit_pg  as specfit
#from viewer_pg import viewer_pg as viewer
from exportfits_pg import exportfits_pg as exportfits
from blcal_pg import blcal_pg as blcal
from uvsub_pg import uvsub_pg as uvsub
from imcontsub_pg import imcontsub_pg as imcontsub
from imstat_pg import imstat_pg as imstat
from newflagdata_pg import newflagdata_pg as newflagdata
from clearstat_pg import clearstat_pg as clearstat
from flagautocorr_pg import flagautocorr_pg as flagautocorr
from browsetable_pg import browsetable_pg as browsetable
from smoothcal_pg import smoothcal_pg as smoothcal
from imval_pg import imval_pg as imval
from vishead_pg import vishead_pg as vishead
from visstat_pg import visstat_pg as visstat
from calstat_pg import calstat_pg as calstat
from msmoments_pg import msmoments_pg as msmoments

##
## asap may not be available with every casa installation
##
try:
    x=1
#    from sdcoadd_pg import sdcoadd_pg as sdcoadd
#    from sdscale_pg import sdscale_pg as sdscale 
#    from sdtpimaging_pg import sdtpimaging_pg as sdtpimaging
#    from sdlist_pg import sdlist_pg as sdlist
#    from sdfit_pg import sdfit_pg as sdfit
#    from sdbaseline_pg import sdbaseline_pg as sdbaseline
#    from sdstat_pg import sdstat_pg as sdstat
#    from sdsave_pg import sdsave_pg as sdsave
#    from sdflag_pg import sdflag_pg as sdflag
#    from sdaverage_pg import sdaverage_pg as sdaverage
#    from sdplot_pg import sdplot_pg as sdplot
#    from sdsmooth_pg import sdsmooth_pg as sdsmooth
#    from sdcal_pg import sdcal_pg as sdcal
except ImportError:
    sdcoadd = None
    sdscale = None
    sdtpimaging = None
    sdlist = None
    sdfit = None
    sdbaseline = None
    sdstat = None
    sdsave = None
    sdflag = None
    sdaverage = None
    sdplot = None
    sdsmooth = None
    sdcal = None
     

#from tasks import *

#
# Check if the display environment is set if not
# switch the backend to Agg only if it's TkAgg
#

if not os.environ.has_key('DISPLAY'):
   nodisplay='no display'
   #if matplotlib.get_backend() == "TkAgg" :
   #   matplotlib.use('Agg')

from time import strftime
stamp=strftime("%Y%m%d%H%M%S")
contrid='0'

try:
   contrid=os.environ['contrid']
   stamp=os.environ['stamp']
except:
   pass 

#thisproc='casaengine-'+stamp+'-'+contrid+'-'
thispid=os.getpid()

#thelogfile='casapy-'+stamp+'-'+contrid+'-'+str(thispid)+'.log'

#is this sufficient?
thelogfile='casapy-'+stamp+'-'+str(id)+'.log'

def get_logs():
   return work_dir+'/'+thelogfile

def clear_logs():
   contents = os.listdir(work_dir)
   for f in contents:
      if os.path.isfile(work_dir+'/'+f) and f.startswith('casapy-') and f.endswith('.log'):
         os.remove(work_dir+'/'+f)

def mkdir(p):
   dirsep = '/'
   pv = p.split(dirsep)
   path = ""
   for i in pv:
      if i:
         path = path + "/" + i
         mkdir1(path)

def mkdir1(p):
   if os.path.exists(p):
      if not os.path.isdir(p):
         cnt = 1
         tmp = p + "."
         while os.path.exists(tmp + str(cnt)):
            cnt += 1
            os.rename(p,tmp)
            os.mkdir(p)
   else:
      os.mkdir(p)

def rmdirs(p):
   contents = os.listdir(work_dir)
   for f in contents:
      #if os.path.isdir(work_dir+'/'+f) and f.startswith(p):
      if f.startswith(p):
         os.system("rm -rf %s" % f)
         #rmdir(f)

# looks all correct, but it complains non-empty dir
def rmdir(p):
   dirsep = '/'
   if os.path.isdir(p):
      contents = os.listdir(p)
      for f in contents:
         if os.path.isfile( p + dirsep + f ) or os.path.islink( p + dirsep + f ):
            os.remove( p + dirsep + f )
         elif os.path.isdir( p + dirsep + f ):
            rmdir( p + dirsep + f )
         os.rmdir(p)


####################
# Task Interface

from parameter_check import *
import inspect 

####################
def go(taskname=None):
    """ Execute taskname: """
    a=inspect.stack()
    stacklevel=0
    for k in range(len(a)):
        if (string.find(a[k][1], 'ipython console') > 0):
            stacklevel=k
            break
    myf=sys._getframe(stacklevel).f_globals
    if taskname==None: taskname=myf['taskname']
    oldtaskname=myf['taskname']
    #myf['taskname']=taskname
    if type(taskname)!=str:
        taskname=taskname.__name__
        myf['taskname']=taskname
    try:
        parameter_checktype(['taskname'],[taskname],str)
    except TypeError, e:
        print "go -- TypeError: ",e
        return
    fulltaskname=taskname+'()'
    print 'Executing: ',fulltaskname
    exec(fulltaskname)
    myf['taskname']=oldtaskname

#def selectfield(vis,minstring):
#    """Derive the fieldid from  minimum matched string(s): """
#
#    tb.open(vis+'/FIELD')
#    fields=list(tb.getcol('NAME'))#get fieldname list
#    tb.close()          #close table
#    indexlist=list()    #initialize list
#    stringlist=list()
#
#    fldlist=minstring.split()#split string into elements
#    print 'fldlist is ',fldlist
#    for fld in fldlist:     #loop over fields
#        _iter=fields.__iter__() #create iterator for fieldnames
#        while 1:
#            try:
#                x=_iter.next() # has first value of field name
#            except StopIteration:
#                break
#            #
#            if (x.find(fld)!=-1): 
#                indexlist.append(fields.index(x))
#                stringlist.append(x)
#
#    print 'Selected fields are: ',stringlist
#    return indexlist
#
#def asap_init():
#    """ Initialize ASAP....: """
#    a=inspect.stack()
#    stacklevel=0
#    for k in range(len(a)):
#        if (string.find(a[k][1], 'ipython console') > 0):
#            stacklevel=k
#            break
#    myf=sys._getframe(stacklevel).f_globals
#    casapath=os.environ['CASAPATH']
#    print '*** Loading ATNF ASAP Package...'
#    import asap as sd
#    print '*** ... ASAP (%s rev#%s) import complete ***' % (sd.__version__,sd.__revision__)
#    os.environ['CASAPATH']=casapath
#    from sdaverage_pg import sdaverage_pg as sdaverage
#    from sdsmooth_pg import sdsmooth_pg as sdsmooth
#    from sdbaseline_pg import sdbaseline_pg as sdbaseline
#    from sdcal_pg import sdcal_pg as sdcal
#    from sdcoadd_pg import sdcoadd_pg as sdcoadd
#    from sdsave_pg import sdsave_pg as sdsave
#    from sdscale_pg import sdscale_pg as sdscale
#    from sdfit_pg import sdfit_pg as sdfit
#    from sdplot_pg import sdplot_pg as sdplot
#    from sdstat_pg import sdstat_pg as sdstat
#    from sdlist_pg import sdlist_pg as sdlist
#    from sdflag_pg import sdflag_pg as sdflag
#    from sdtpimaging_pg import sdtpimaging_pg as sdtpimaging
#    myf['sd']=sd
#    myf['sdaverage']=sdaverage
#    myf['sdsmooth']=sdsmooth
#    myf['sdbaseline']=sdbaseline
#    myf['sdcal']=sdcal
#    myf['sdcoadd']=sdcoadd
#    myf['sdsave']=sdsave
#    myf['sdscale']=sdscale
#    myf['sdfit']=sdfit
#    myf['sdplot']=sdplot
#    myf['sdstat']=sdstat
#    myf['sdlist']=sdlist
#    myf['sdflag']=sdflag
#    myf['sdtpimaging']=sdtpimaging
#
#
#def selectantenna(vis,minstring):
#    """Derive the antennaid from matched string(s): """
#
#    tb.open(vis+'/ANTENNA')
#    ants=list(tb.getcol('NAME'))#get fieldname list
#    tb.close()          #close table
#    indexlist=list()    #initialize list
#    stringlist=list()
#
#    antlist=minstring.split()#split string into elements
#    for ant in antlist:     #loop over fields
#        try:
#            ind=ants.index(ant)
#            indexlist.append(ind)
#            stringlist.append(ant)
#        except ValueError:
#            pass
#
#    print 'Selected reference antenna: ',stringlist
#    print 'indexlist: ',indexlist
#    return indexlist[0]
#
#def readboxfile(boxfile):
#    """ Read a file containing clean boxes (compliant with AIPS BOXFILE)
#
#    Format is:
#    #FIELDID BLC-X BLC-Y TRC-X TRC-Y
#    0       110   110   150   150 
#    or
#    0       hh:mm:ss.s dd.mm.ss.s hh:mm:ss.s dd.mm.ss.s
#
#    Note all lines beginning with '#' are ignored.
#
#    """
#    union=[]
#    f=open(boxfile)
#    while 1:
#        try: 
#            line=f.readline()
#            if (line.find('#')!=0): 
#                splitline=line.split('\n')
#                splitline2=splitline[0].split()
#                if (len(splitline2[1])<6): 
#                    boxlist=[int(splitline2[1]),int(splitline2[2]),
#                    int(splitline2[3]),int(splitline2[4])]
#                else:
#                    boxlist=[splitline2[1],splitline2[2],splitline2[3],
#                    splitline2[4]]
#    
#                union.append(boxlist)
#    
#        except:
#            break
#
#    f.close()
#    print 'union is: ',union
#    return union

def inp(taskname=None):
    try:
        a=inspect.stack()
        stacklevel=0
        for k in range(len(a)):
            if (string.find(a[k][1], 'ipython console') > 0):
                stacklevel=k
        myf=sys._getframe(stacklevel).f_globals
        if((taskname==None) and (not myf.has_key('taskname'))):
            print 'No task name defined for inputs display'
            return
        if taskname==None: taskname=myf['taskname']
        myf['taskname']=taskname
        if type(taskname)!=str:
            taskname=taskname.__name__
            myf['taskname']=taskname

        try:
            parameter_checktype(['taskname'],taskname,str)
        except TypeError, e:
            print "inp -- TypeError: ", e
            return
        except ValueError, e:
            print "inp -- OptionError: ", e
            return

        ###Check if task exists by checking if task_defaults is defined
        if ( not myf.has_key(taskname) and
             str(type(myf[taskname])) != "<type 'instance'>" and
             not hasattr(myf[taskname],"defaults") ):
            raise TypeError, "task %s is not defined " %taskname
        if(myf.has_key('__last_taskname')):
            myf['__last_taskname']=taskname
        else:
            myf.update({'__last_taskname':taskname})

        #print '# ',myf['taskname']+' :: '+(eval(myf['taskname']+'.description()'))
        print '# ',myf['taskname']+' input on engine', id
        update_params(myf['taskname'])
    except TypeError, e:
        print "inp --error: ", e
    except Exception, e:
        print "---",e

def update_params(func, printtext=True):
    from odict import odict
    a=inspect.stack()
    stacklevel=0
    for k in range(len(a)):
        if (string.find(a[k][1], 'ipython console') > 0):
            stacklevel=k
    myf=sys._getframe(stacklevel).f_globals
    ### set task to the one being called
    myf['taskname']=func
    obj=myf[func]

    if ( str(type(obj)) == "<type 'instance'>" and
         hasattr(obj,"check_params") ):
        hascheck = True
    else:
        hascheck = False

    noerror=True
    ###check if task has defined a task_check_params function

    if (hascheck):
	has_othertasks = myf.has_key('task_location')
	if(has_othertasks) :
	   has_task = myf['task_location'].has_key(myf['taskname'])
	   if (has_task) :
		pathname=myf['task_location'][myf['taskname']]
	   else :
	        pathname = os.environ.get('CASAPATH').split()[0]+'/share/xml'
	else :
	   pathname = os.environ.get('CASAPATH').split()[0]+'/share/xml'
        xmlfile=pathname+'/'+myf['taskname']+'.xml'
        if(os.path.exists(xmlfile)) :
            cu.setconstraints('file://'+xmlfile);

    a=myf[myf['taskname']].defaults("paramkeys")

    params=a
    itsparams = {}
    for k in range(len(params)):
        paramval = obj.defaults(params[k])

        notdict=True
        ###if a dictionary with key 0, 1 etc then need to peel-open
        ###parameters
        if(type(paramval)==dict):
            if(paramval.has_key(0)):
                notdict=False
        
        if (notdict ):
            if(not myf.has_key(params[k])):
                myf.update({params[k]:paramval})
            if(printtext):
                if(hascheck):
                    noerror = obj.check_params(params[k],myf[params[k]])
                if(myf[params[k]]==paramval):
                    print_params_col(params[k],myf[params[k]],obj.description(params[k]), 'ndpdef', 'black',noerror)
                else:
                    print_params_col(params[k],myf[params[k]],obj.description(params[k]), 'ndpnondef', 'black', noerror)
		itsparams[params[k]] = myf[params[k]]
        else:
            subdict=odict(paramval)
            ##printtext is False....called most probably to set
            ##undefined params..no harm in doing it anyways
            if(not printtext):
                ##locate which dictionary is user selected
                userdict={}
                subkeyupdated={}
                for somekey in paramval:
                    somedict=dict(paramval[somekey])
                    subkeyupdated.update(dict.fromkeys(somedict, False))
                    if(somedict.has_key('value') and myf.has_key(params[k])):
                        if(somedict['value']==myf[params[k]]):
                            userdict=somedict
                    elif(somedict.has_key('notvalue') and myf.has_key(params[k])):
                        if(somedict['notvalue']!=myf[params[k]]):
                            userdict=somedict
                ###The behaviour is to set to the first default
                ### all non set parameters and parameters that
                ### have no meaning for this selection
                for j in range(len(subdict)):
                    subkey=subdict[j].keys()
                    for kk in range(len(subkey)):
                        
                        if( (subkey[kk] != 'value') & (subkey[kk] != 'notvalue') ):
                            #if user selecteddict
                            #does not have the key
                            ##put default
                            if((not userdict.has_key(subkey[kk])) and (not subkeyupdated[subkey[kk]])):
                                myf.update({subkey[kk]:subdict[j][subkey[kk]]})
                                subkeyupdated[subkey[kk]]=True
                                
                    ###put default if not there
                            if(not myf.has_key(subkey[kk])):
                                myf.update({subkey[kk]:subdict[j][subkey[kk]]})
                        
            ### need to do default when user has not set val
            if(not myf.has_key(params[k])):
                if(paramval[0].has_key('notvalue')):
                    myf.update({params[k]:paramval[0]['notvalue']})
                else:
                    myf.update({params[k]:paramval[0]['value']})
            userval=myf[params[k]]
            choice=0
            notchoice=-1
            valuekey='value'
            for j in range(len(subdict)):
                if(subdict[j].has_key('notvalue')):
                    valuekey='notvalue'
                    if(subdict[j]['notvalue'] != userval):
                        notchoice=j;
                        break
                else:
                    if(subdict[j]['value']==userval):
                        choice=j
                        notchoice=j
                        break
            subkey=subdict[choice].keys()
            if(hascheck):
                noerror=obj.check_params(params[k],userval)
            if(printtext):
                if(myf[params[k]]==paramval[0][valuekey]):
                    print_params_col(params[k],myf[params[k]],obj.description(params[k]),'dpdef','black', noerror)
                else:
                    print_params_col(params[k],myf[params[k]],obj.description(params[k]),'dpnondef','black', noerror)
		itsparams[params[k]] = myf[params[k]]
            for j in range(len(subkey)):
                if((subkey[j] != valuekey) & (notchoice > -1)):
                    ###put default if not there
                    if(not myf.has_key(subkey[j])):
                        myf.update({subkey[j]:subdict[choice][subkey[j]]})
                    paramval=subdict[choice][subkey[j]]
                    if (j==(len(subkey)-1)):
                        # last subparameter - need to add an extra line to allow cut/pasting
                        comment='last'
                    else:
                        comment='blue'
                    if(hascheck):
                        noerror = obj.check_params(subkey[j],myf[subkey[j]])
                    if(printtext):
                        if(myf[subkey[j]]==paramval):
                            print_params_col(subkey[j],myf[subkey[j]],obj.description(subkey[j],userval),'spdef',comment, noerror)
                        else:
                            print_params_col(subkey[j],myf[subkey[j]],obj.description(subkey[j],userval),'spnondef',comment, noerror)
		        itsparams[params[k]] = myf[params[k]]
    #print itsparams                    
    #
    # Verify the complete record, with errors being reported to the user
    #
    #cu.verify(itsparams, cu.torecord('file://'+xmlfile)[myf['taskname']]);

####function to print inputs with coloring
####colorparam 'blue'=> non-default, colorcomment 'green'=> can have sub params
#### 'blue' => is a sub-parameter 
# blue = \x1B[94m
# bold = \x1B[1m
# red  = \x1B[91m
# cyan = \x1B[96m
# green= \x1B[92m
# normal   = \x1B[0m
# underline= \x1B[04m
# reverse = \x1B[7m
# highlight with black = \x1B[40s

def print_params_col(param=None, value=None, comment='', colorparam=None,
                     colorcomment=None, noerrorval=True):
    try:
        from TerminalController import TerminalController
        term = TerminalController()
        cols = term.COLS
        del term
    except:
        cols = 80
    #
    #print 'colorparam is: ', colorparam
    #
    if type(value) == str:
        printval = "'" + value + "'"
    else:
        printval = value

    if colorparam == 'ndpnondef':
        firstcol = '\x1B[0m'
        valcol   = '\x1B[94m'
    elif colorparam == 'dpdef':
        firstcol = '\x1B[1m' + '\x1B[47m'
        valcol   = '\x1B[1m' + '\x1B[0m'
    elif colorparam == 'dpnondef':
        firstcol = '\x1B[1m' + '\x1B[47m'
        valcol   = '\x1B[1m' + '\x1B[94m'
    elif colorparam == 'spdef':
        firstcol = '\x1B[32m'
        valcol   = '\x1B[0m'
    elif colorparam == 'spnondef':
        firstcol = '\x1B[32m'
        valcol   = '\x1B[94m'
    else:
        firstcol = '\x1B[0m'
        valcol   = '\x1B[0m'

    if not noerrorval:
        valcol = '\x1B[1m' + '\x1B[91m'

    if colorcomment == 'green':
        secondcol = '\x1B[102m'
    elif colorcomment == 'blue':
        #secondcol='\x1B[104m'
        secondcol = '\x1B[0m'
    else:
        secondcol = '\x1B[0m'

    # RR: I think colorcomment should really be called submenu.
    #     Since these are left justified, I've absorbed the right space into
    #     the %s's, in order to handle as long a parameter name as possible.
    #     (The uvfilterb* params were busting out of %-10s.)
    if colorcomment in ('last', 'blue'):
        parampart = firstcol + '     %-14s ='
    else:
        parampart = firstcol + '%-19s ='
    parampart %= param

    valpart = valcol + ' %10s \x1B[0m' % printval + secondcol
    # Well behaved (short) parameters and values tally up to 33 characters
    # so far.  Pad them up to 40, assuming the param is short enough.
    pad = 7
    paramlen = len(str(param))
    if colorcomment in ('last', 'blue') and paramlen > 14:
        pad -= paramlen - 14
    elif paramlen > 19:
        pad -= paramlen - 19
    valuelen = len(str(printval))
    if valuelen > 10:
        pad -= valuelen - 10
    if pad > 0:
        valpart += ' ' * pad

    try:
        from textwrap import fill
        if pad < 0:
            firstskip = 40 - pad
            firstfiller = ' ' * firstskip + '#  '
            afterfiller = ' ' * 40 + '#   '
        else:
            firstskip = 40
            firstfiller = ' ' * 40 + '#  '
            afterfiller = firstfiller + ' '
        commentpart = fill(comment, cols, initial_indent=firstfiller,
                           subsequent_indent=afterfiller)[firstskip:]
    except:
        if comment:
            commentpart = '#  ' + comment
        else:
            commentpart = ''
    commentpart += '\x1B[0m'          # RR: I think this might be redundant.
    if colorcomment == 'last':        #     (Is colorcomment ever green?)
        commentpart += "\n"

    #print parampart + valpart + commentpart
    print parampart + valpart

def __set_default_parameters(b):
    a=inspect.stack()
    stacklevel=0
    for k in range(len(a)):
        if (string.find(a[k][1], 'ipython console') > 0):
            stacklevel=k
    myf=sys._getframe(stacklevel).f_globals
    a=b
    elkey=a.keys()
    for k in range(len(a)):
        if (type(a[elkey[k]]) != dict):
            myf[elkey[k]]=a[elkey[k]]
        elif (type(a[elkey[k]]) == dict and len(a[elkey[k]])==0):
            myf[elkey[k]]=a[elkey[k]]
        else:
            subdict=a[elkey[k]]
            ##clear out variables of other options if they exist
            for j in range(1,len(subdict)):
                subkey=subdict[j].keys()
                for kk in range(len(subkey)):
                    if((subkey[kk] != 'value') & (subkey[kk] != 'notvalue') ):
                        if(myf.has_key(subkey[kk])):
                            del myf[subkey[kk]]
            ###
            if(subdict[0].has_key('notvalue')):
                myf[elkey[k]]=subdict[0]['notvalue']
            else:
                myf[elkey[k]]=subdict[0]['value']
            subkey=subdict[0].keys()
            for j in range(0, len(subkey)):
                if((subkey[j] != 'value') & (subkey[j] != 'notvalue')):
                    myf[subkey[j]]=subdict[0][subkey[j]]

def saveinputs(taskname=None, outfile='', myparams=None):
    #parameter_printvalues(arg_names,arg_values,arg_types)
    """ Save current input values to file on disk for a specified task:

    taskname -- Name of task
        default: <unset>; example: taskname='bandpass'
        <Options: type tasklist() for the complete list>
    outfile -- Output file for the task inputs
        default: taskname.saved; example: outfile=taskname.orion

    """

    try:
        a=inspect.stack()
        stacklevel=0
        for k in range(len(a)):
            if (string.find(a[k][1], 'ipython console') > 0):
                stacklevel=k
        myf=sys._getframe(stacklevel).f_globals
        if taskname==None: taskname=myf['taskname']
        myf['taskname']=taskname
        if type(taskname)!=str:
            taskname=taskname.__name__
            myf['taskname']=taskname

        parameter_checktype(['taskname','outfile'],[taskname,outfile],[str,str])

        ###Check if task exists by checking if task_defaults is defined
        obj = False
        if ( not myf.has_key(taskname) and
             str(type(myf[taskname])) != "<type 'instance'>" and
             not hasattr(myf[taskname],"defaults") ):
            raise TypeError, "task %s is not defined " %taskname
        else:
            obj = myf[taskname]

        if taskname==None: taskname=myf['taskname']
        myf['taskname']=taskname
        if outfile=='': outfile=taskname+'.saved'
        ##make sure unfolded parameters get their default values
        myf['update_params'](func=myf['taskname'], printtext=False)
        ###
        taskparameterfile=open(outfile,'w')
        print >>taskparameterfile, '%-15s    = "%s"'%('taskname', taskname)
        print >>taskparameterfile,myf[taskname].__call__.func_defaults
        print >>taskparameterfile,myf[taskname].__call__.func_code.co_varnames

        f=zip(myf[taskname].__call__.func_code.co_varnames,myf[taskname].__call__.func_defaults)
        scriptstring='#'+str(taskname)+'('
	if myparams == None :
		myparams = {}
        for j in range(len(f)):
            k=f[j][0]
	    if not myparams.has_key(k) and k != 'self' :
		    myparams[k] = myf[k]
            if(k != 'self' and type(myparams[k])==str):
                if ( myparams[k].count( '"' ) < 1 ):
                    # if the string doesn't contain double quotes then
                    # use double quotes around it in the parameter file.
                    print >>taskparameterfile, '%-15s    =  "%s"'%(k, myparams[k])
                    scriptstring=scriptstring+k+'="'+myparams[k]+'",'
                else:
                    # use single quotes.
                    print >>taskparameterfile, "%-15s    =  '%s'"%(k, myparams[k])
                    scriptstring=scriptstring+k+"='"+myparams[k]+"',"
            else :
                if ( j != 0 or k != "self" or
                     str(type(myf[taskname])) != "<type 'instance'>" ) :
                    print >>taskparameterfile, '%-15s    =  %s'%(k, myparams[k])
                    scriptstring=scriptstring+k+'='+str(myparams[k])+','

            ###Now delete varianle from global user space because
            ###the following applies: "It would be nice if one
            ### could tell the system to NOT recall
            ### previous non-default settings sometimes."
            if(not myf['casaglobals']):
                del myf[k]
        scriptstring=scriptstring.rstrip(',')
        scriptstring=scriptstring+')'        
        print >>taskparameterfile,scriptstring
        taskparameterfile.close()
    except TypeError, e:
        print "saveinputs --error: ", e

def default(taskname=None):
    """ reset given task to its default values :

    taskname -- Name of task


    """

    try:
        a=inspect.stack()
        stacklevel=0
        for k in range(len(a)):
            if (string.find(a[k][1], 'ipython console') > 0):
                stacklevel=k
                break
        myf=sys._getframe(stacklevel).f_globals
        if taskname==None: taskname=myf['taskname']
        myf['taskname']=taskname
        if type(taskname)!=str:
            taskname=taskname.__name__
            myf['taskname']=taskname

        ###Check if task exists by checking if task_defaults is defined
        if ( not myf.has_key(taskname) and
             str(type(myf[taskname])) != "<type 'instance'>" and
             not hasattr(myf[taskname],"defaults") ):
            raise TypeError, "task %s is not defined " %taskname
        eval(myf['taskname']+'.defaults()')

        casalog.origin('default')
        taskstring=str(taskname).split()[0]
        casalog.post(' #######  Setting values to default for task: '+taskstring+'  #######')


    except TypeError, e:
        print "default --error: ", e

####################

#def exit():
#    __IPYTHON__.exit_now=True
#    #print 'Use CNTRL-D to exit'
#    #return

#import pylab as pl

#
# 
import platform
##
## CAS-951: matplotlib unresponsive on some 64bit systems
##
#if (platform.architecture()[0]=='64bit'):
#    pl.ioff( )
#    pl.clf( )
#    pl.ion( )
##
##

# Provide flexibility for boolean representation in the CASA shell
true  = True
T     = True
false = False
F     = False

# run casapy non-interactively on engine
ipython=False

# setup available tasks
#
from math import *

#from tasks import *
#cannot do this because
# tasks imports xxx_cli
# xxx_pg imports taskmanager
# taskmanager starts a cluster and 4 engines


from parameter_dictionary import *
from task_help import *
#
#home=os.environ['HOME']
#if ipython:
#    startup()

# assignment protection
#
#pathname=os.environ.get('CASAPATH').split()[0]
#uname=os.uname()
#unameminusa=str.lower(uname[0])
#fullpath=pathname+'/'+unameminusa+'/python/2.5/assignmentFilter.py'
casalog.origin('casa')
##
## /home/casa-dev-01/hye/gnuactive/linux_64b/python/2.5  is substitued at build time
##
#fullpath='/home/casa-dev-01/hye/casapy-casapy-23.1.6826-006-64b/lib64/python2.5'
#sys.path.insert(2, fullpath)
#fullpath='/usr/lib64/casapy/23.1.6826-006/lib/python2.5/assignmentFilter.py'

#if os.environ.has_key('__CASAPY_PYTHONDIR'):
#    fullpath=os.environ['__CASAPY_PYTHONDIR'] + '/assignmentFilter.py'

#utilstool = casac.homefinder.find_home_by_name('utilsHome')
#cu = casac.cu = utilstool.create()

casalog.setlogfile(thelogfile)
casalog.showconsole(False)
casalog.version()

def stop_all():
    #os.kill(pid,9)
    print "leaving casapy..."

def setcwd(dir='.'):
    os.chdir(dir)

def get_host():
    import commands
    return commands.getoutput("uname -n")

import shutil

def delete_scratch():
    for x in os.listdir('.'):
       if x.lower().startswith('casapy.scratch-'):
          if os.path.isdir(x):
             #shutil.rmtree(x, ignore_errors=True)
             os.system("rm -rf %s" % x)
             #print "Removed: ", x, "\n"

if __name__ == "__main__":
   print "start casapy engine---"
