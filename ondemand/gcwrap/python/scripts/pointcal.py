import sys
import string
import inspect
from odict import odict

from parameter_check import *

def pointcal(vis=None,model=None,caltable=None,
	     field=None,spw=None,
	     selectdata=None,timerange=None,uvrange=None,antenna=None,scan=None,msselect=None,
	     solint=None):
	"""Solve for pointing error calibration:

	This program is under development.  Please do not use.

	Keyword arguments:
	vis -- Name of input visibility file (MS)
		default: <unset>; example: vis='ngc5921.ms'
	model -- Name of input model (component list or image)
		default: <unset>; example: model='ngc5921.im'
	caltable -- Name of output Pointing calibration table
		default: <unset>; example: caltable='ngc5921.gcal'

	--- Data Selection (see help par.selectdata for more detailed information)

       field -- Select field using field id(s) or field name(s).
                  [run listobs to obtain the list id's or names]
               default: ''=all fields
               If field string is a non-negative integer, it is assumed a field index
                 otherwise, it is assumed a field name
               field='0~2'; field ids 0,1,2
               field='0,4,5~7'; field ids 0,4,5,6,7
               field='3C286,3C295'; field named 3C286 adn 3C295
               field = '3,4C*'; field id 3, all names starting with 4C
       spw -- Select spectral window/channels
               default: ''=all spectral windows and channels
               spw='0~2,4'; spectral windows 0,1,2,4 (all channels)
               spw='<2';  spectral windows less than 2 (i.e. 0,1)
               spw='0:5~61'; spw 0, channels 5 to 61
               spw='0,10,3:3~45'; spw 0,10 all channels, spw 3, channels 3 to 45.
               spw='0~2:2~6'; spw 0,1,2 with channels 2 through 6 in each.
               spw='0:0~10;15~60'; spectral window 0 with channels 0-10,15-60
               spw='0:0~10,1:20~30,2:1;2;3'; spw 0, channels 0-10,
                        spw 1, channels 20-30, and spw 2, channels, 1,2 and 3

       selectdata -- Other data selection parameters
               default: True
       timerange  -- Select data based on time range:
               default = '' (all); examples,
               timerange = 'YYYY/MM/DD/hh:mm:ss~YYYY/MM/DD/hh:mm:ss'
               Note: if YYYY/MM/DD is missing dat defaults to first day in data set
               timerange='09:14:0~09:54:0' picks 40 min on first day
               timerange= '25:00:00~27:30:00' picks 1 hr to 3 hr 30min on next day
               timerange='09:44:00' data within one integration of time
               timerange='>10:24:00' data after this time
       uvrange -- Select data within uvrange (default units kilo-lambda)
               default: '' (all); example:
               uvrange='0~1000'; uvrange from 0-1000 kilo-lamgda
               uvrange='>4';uvranges greater than 4 kilo lambda
       antenna -- Select data based on antenna/baseline
               default: '' (all)
               If antenna string is a non-negative integer, it is assumed an antenna index
                 otherwise, it is assumed as an antenna name
               antenna='5&6'; baseline between antenna index 5 and index 6.
               antenna='VA05&VA06'; baseline between VLA antenna 5 and 6.
               antenna='5&6;7&8'; baseline 5-6 and 7-8
               antenna='5'; all baselines with antenna 5
               antenna='5,6,10'; all baselines with antennas 5 and 6
       scan -- Scan number range - New, under developement
       msselect -- Optional complex data selection (ignore for now)

	solint --  Solution interval (sec)
		default: 0.0 (scan based); example: solint=60.

	"""
        a=inspect.stack()
        stacklevel=0
        for k in range(len(a)):
          if (string.find(a[k][1], 'ipython console') > 0):
                stacklevel=k
        myf=sys._getframe(stacklevel).f_globals
        myf['taskname']='pointcal'
	###fill unfilled parameters with defaults
	myf['update_params'](func=myf['taskname'], printtext=False)
	####local params 
        cb = myf['cb']
	selectantenna=myf['selectantenna']
        ###
        #Handle globals or user over-ride of arguments
        #
        function_signature_defaults=dict(zip(pointcal.func_code.co_varnames,pointcal.func_defaults))
        for item in function_signature_defaults.iteritems():
                key,val = item
                keyVal = eval(key)
                if (keyVal == None):
                        #user hasn't set it - use global/default
                        pass
                else:
			myf[key]=keyVal
                        #user has set it - use over-ride

        vis=myf['vis']
	model=myf['model']
        caltable=myf['caltable']
        field=myf['field']
        spw=myf['spw']
        selectdata=myf['selectdata']
	if (selectdata):
		timerange=myf['timerange']
		uvrange=myf['uvrange']
		antenna=myf['antenna']
		scan=myf['scan']
		msselect=myf['msselect']
	else:
		timerange=''
		uvrange=''
		antenna=''
		scan=''
		msselect=''
        solint=myf['solint']
        #sys._getframe(1).f_globals[key]=keyVal

        #Add type/menu/range error checking here
        arg_names=['vis','model','caltable',
		   'field','spw',
		   'selectdata','timerange','uvrange','antenna','scan','msselect',
		   'solint']
        arg_values=[vis,model,caltable,
		    field,spw,
		    selectdata,timerange,uvrange,antenna,scan,msselect,
		    solint]
        arg_types=[str,str,str,
		   str,str,
		   bool,str,str,str,str,str,
		   float]
        try:
                parameter_checktype(arg_names,arg_values,arg_types)
        except TypeError, e:
                print "pointcal -- TypeError: ", e
		return
        except ValueError, e:
                print "pointcal -- OptionError: ", e
                return
        ###


	#Python script
	#parameter_printvalues(arg_names,arg_values,arg_types)
	try:
		cb.open(vis)
		cb.reset()

		cb.selectvis(time=timerange,spw=spw,scan=scan,field=field,
			     baseline=antenna,uvrange=uvrange,chanmode='none',
			     nchan=nchan,start=start,step=step,
			     msselect=msselect);

		cb.setsolve2(type='POINTCAL',t=solint,refant='',table=caltable)
		#cb.state()
		cb.solve()
		cb.close()
	except Exception, instance:
		print '*** Error ***',instance
        saveinputs=myf['saveinputs']
        saveinputs('pointcal','pointcal.last')


def pointcal_defaults(param=None):
        a=inspect.stack()
        stacklevel=0
        for k in range(len(a)):
          if (string.find(a[k][1], 'ipython console') > 0):
                stacklevel=k
        myf=sys._getframe(stacklevel).f_globals
	a=odict()
        a['vis']=''
        a['model']=''
        a['caltable']=''
	a['field']=''
	a['spw']=''
        a['selectdata']={1:{'value':False},
			 0:odict([{'value':True},
				  {'timerange':''},
				  {'uvrange':''},
				  {'antenna':''},
				  {'scan':''},
				  {'msselect':''}])
                         }
        a['solint']=0.0
	if(param == None):
		myf['__set_default_parameters'](a)
	elif(param == 'paramkeys'):
		return a.keys()			    
        else:
		if(a.has_key(param)):
			return a[param]


def pointcal_description(key='pointcal',subkey=None):
        desc={'pointcal': 'Solve for pointing error calibration:\n',
	'vis': 'This program is under development.  Please do not use. \n\n\t\t\t\t\t\t Name of input visibility file (MS)',
        'model': 'Name of input model (component list or image)',
        'caltable': 'Name of output Gain calibration table',
        'field': 'Select data based on field name or index',
        'spw': 'Select data based on spectral window',
        'selectdata': 'Activate data selection details',
        'timerange': 'Select data based on time',
        'uvrange': 'Select data based on uv range',
        'antenna': 'Select data based on antenna/baseline',
        'scan': 'Select data based on scan number',
        'msselect': 'Optional data selection (see help)',
        'gaintable': 'Gain calibration solutions to apply',
        'gainselect': 'Select subset of calibration solutions from gaintable',
        'solint': 'Solution interval (sec)',
        }
	if(desc.has_key(key)):
		return desc[key]
	return ''
