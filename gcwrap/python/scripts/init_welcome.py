from casa_builtin import enable_builtin_protection,register_builtin

register_builtin("cu")
register_builtin(["viewer","imview","msview"])

enable_builtin_protection( )
print "CASA Version " + casa['build']['version'] + "\n  Compiled on: " + casa['build']['time']

