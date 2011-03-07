def parameter_checktype(arg_names,arg_values,arg_types):
	for k in range(0,len(arg_names)):
		if not len(arg_names)==1:
			if not isinstance(arg_values[k],arg_types[k]): 
				print " *** "
				print " "
				raise TypeError, "Expected '%s' to be '%s'; was '%s'. \n  \n *** " % (arg_names[k],arg_types[k], type(arg_values[k]))
			else:
				pass
		else:
			if not isinstance(arg_values[0],arg_types):
				print " *** "
				print " "
				raise TypeError, "Expected '%s' to be '%s'; was '%s'. \n \n  *** " % (arg_names,arg_types,type(arg_values))
			else:
				pass

def parameter_checkmenu(arg_name,arg_value,arg_options):
	try:
		arg_options.index(arg_value)
	except ValueError, e:
		print " *** "
		print " "
		raise ValueError, "'%s' is not an option for '%s'; must be one of: %s. \n \n  *** " % (arg_value, arg_name, arg_options)


def parameter_checklist(arg_name,arg_value,arg_options):
	# We assume that arg_value is a string, we need to first break
	# make it into a list.
	arg_list=[arg_value];
	if ( arg_value.find( "," ) > -1 ):
		arg_list=arg_value.split( "," );
	elif ( arg_value.find( " " ) > -1 ):
		arg_list=arg_value.split( " " );

	
	# Now check each element to see if it is valid.
	# Note that we check with UPPERCASE values to eliminate case
	# senstivity, and remove trailing and leading spaces
	for value in arg_list[:] :
		try:
			arg_options.index(value.upper().strip())
		except ValueError, e:
			print " *** "
			print " "
			raise ValueError, "'%s' is not an option for '%s'; must be one of: %s. \n \n  *** " % (arg_value, value, arg_options)
	

#def parameter_checkrage(arg_name,arg_value,arg_range):

def parameter_printvalues(arg_names,arg_values,arg_types):
	for k in range(0,len(arg_names)):
		print "Parameter: %s is: %s and has type %s." % (arg_names[k],arg_values[k],type(arg_values[k]))
	
