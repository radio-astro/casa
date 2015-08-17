# encoding: utf-8
# $Id: $
"""
assignmentFilter.py

IPython input prefilter -- capture assignment

We have found that users often re-assign built-in
CASA tasks etc. with other values, e.g.:

	importfits = 'myFile.fits'

Doing so re-binds the CASA procedure importfits
to a string (losing the procedure). Since this is a
Bad Thing, we add an input_prefilter that 



Created by Boyd Waters on 2007-04-04.
Copyright (c) 2007 __MyCompanyName__. All rights reserved.
"""

import sys
import types
import re
import IPython.ipapi
from IPython.ColorANSI import TermColors

_asf_debug = False
_asf_enabled = True

_af_msg_keys = {
	types.FunctionType		  : 0,
	types.MethodType		  : 0,
	types.BuiltinFunctionType         : 0,
	types.BuiltinMethodType           : 0,
	types.ObjectType		  : 1,
	types.InstanceType		  : 1,
	types.ModuleType		  : 1,
	"__casac__"				  : 2,
	"viewertool"                      : 2,
	"builtin"			  : 3,
	"parameter"			  : 4
}

_af_msgs = (
	("a protected CASA function", "%(tokcolor)s %(tok)s( %(normal)s ... %(tokcolor)s )%(normal)s"),
	("a protected CASA Module",   "%(tokcolor)s %(tok)s.someFunction( %(normal)s ... %(tokcolor)s ) %(normal)s"),
	("a protected CASA Tool",	 "%(tokcolor)s %(tok)s.someFunction( %(normal)s ... %(tokcolor)s ) %(normal)s"),
	("a reserved system value",  None),
	("an unknown CASA parameter", None)
)

_af_warning = "%(warncolor)s WARNING:%(normal)s Using %(tokcolor)s%(tok)s%(normal)s for a variable name is not allowed; it is %(type)s."
_af_suggest = "%(suggcolor)s SUGGEST:%(normal)s try "
_af_help	= " or type %(tokcolor)s help %(tok)s %(normal)s"
_af_param   = "%(warncolor)s WARNING:%(normal)s unknown parameter %(tokcolor)s%(tok)s%(normal)s. Are you sure you want to set this value? (y or n)"


def debugMsg(*args):
	try:
		if _asf_debug:
			print TermColors.Cyan, "DEBUG:",string.join(map(str,args)),TermColors.Normal
	except:
		pass



def enable_asf():
	global _asf_enabled
	debugMsg("input filter is enabled")
	_asf_enabled = True

def disable_asf():
	global _asf_enabled
	debugMsg("input filter is disabled")
	_asf_enabled = False



def global_lookup(token):
	"""
	find the string token 
	in the global namespace
	"""
	result = None
	a=inspect.stack()
	stacklevel=0
	for k in range(len(a)):
		if (string.find(a[k][1], 'ipython console') > 0):
			stacklevel=k
			break
	myf=sys._getframe(stacklevel).f_globals
	if(myf.has_key(token)):
		result = myf[token]
	return result


def _af_validate_parameter(token):
	if not token.startswith("par."):
		return True
	else:
		param = token[4:]  # 'par.' is four characters
		try:
			if par.__dict__.has_key(param):
				return True
			else:
				msgDict = {
				"tok":token,
				"tokcolor":TermColors.Purple,
				"warncolor":TermColors.Red,
				"normal":TermColors.Normal
				}
				print _af_param % msgDict
				y_or_n = raw_input()
				return (y_or_n and y_or_n.startswith("y"))
		except e:
			print TermColors.Cyan, "Exception: ", e
			pass
		return False


def assignmentFilter(self,line):
	"""
	IPython input prefilter -- capture assignment

	We have found that users often re-assign built-in
	CASA tasks etc. with other values, e.g.:

		importfits = 'myFile.fits'

	Doing so re-binds the CASA procedure importfits
	to a string. Generally this is a Bad Thing.
	"""
	lineOK = True
	global _asf_enabled
	if _asf_enabled:
		if line.find('(') > -1 or line.find('!') > -1:
			debugMsg("ignoring input with parethesis or sh command line ")
		else:
			debugMsg("scanning input")
			req=re.compile('([\.\w]*)\s*=[^=]')
			for token in req.findall(line):
				if len(token) > 0:
					debugMsg( "token is", token )
					if token.startswith("par."):
						lvar_type = "parameter"
						lineOK = _af_validate_parameter(token)
					else:
						debugMsg('builtin', type(__builtins__))
						isBuiltin = False
						try:
							isBuiltin = __builtins__.__dict__.has_key(token)
						except:
							try:
								isBuiltin = __builtins__.has_key(token)
							except:
								pass
						if isBuiltin:
							lvar_type = "builtin"
							lineOK = False
						else:
							lvar_type = type(global_lookup(token))
							debugMsg('global',lvar_type)
							if _af_msg_keys.has_key(lvar_type):
								lineOK = False
							else:
								try:
									lvar_type = string.split(lvar_type.__module__, '.')[0]
									if _af_msg_keys.has_key(lvar_type):
										lineOK = False
								except:
									pass
				if not lineOK:
					break	
			# end for
	#
	if lineOK:
		return line
	else:
		tokType, example = _af_msgs[_af_msg_keys[lvar_type]]
		msgDict = {
		"tok":token,
		"type":tokType,
		"tokcolor":TermColors.Purple,
		"suggcolor":TermColors.LightGreen,
		"warncolor":TermColors.Red,
		"normal":TermColors.Normal
		}
		print _af_warning % msgDict
		if example:
			suggestion = _af_suggest + example + _af_help
			print suggestion % msgDict
		return ""



ip = IPython.ipapi.get()
ip.set_hook('input_prefilter', assignmentFilter)


def _asf_generate_prompt(self, is_continuation):
	""" calculate and return a string with the prompt to display """
	if is_continuation:
		disable_asf()
		return str(ip.IP.outputcache.prompt2)
	else:
		enable_asf()
	return str(ip.IP.outputcache.prompt1)

ip.set_hook('generate_prompt',_asf_generate_prompt)
