#!/usr/bin/env python
"""
These little utilities are extracted from:

    Mickey's own dbus introspection utility.

See: http://git.freesmartphone.org/?p=python-helpers.git;a=summary
(C) 2008-2009 Michael 'Mickey' Lauer <mlauer@vanille-media.de>

GPLv2 or later
"""

__version__ = "0.9.9.10"

import types
import dbus

#----------------------------------------------------------------------------#
def dbus_to_python(v):
#----------------------------------------------------------------------------#
    class ObjectPath( object ):
        def __init__( self, path ):
            self.path = str( path )

        def __repr__( self ):
            return "op%s" % repr(self.path)

    if isinstance(v, dbus.Byte) \
        or isinstance(v, dbus.Int64) \
        or isinstance(v, dbus.UInt64) \
        or isinstance(v, dbus.Int32) \
        or isinstance(v, dbus.UInt32) \
        or isinstance(v, dbus.Int16) \
        or isinstance(v, dbus.UInt16) \
        or type(v) == types.IntType:
        return int(v)
    elif isinstance(v, dbus.Double) or type(v) == types.FloatType:
        return float(v)
    elif isinstance(v, dbus.String) or type(v) == types.StringType:
        return str(v)
    elif isinstance(v, dbus.Dictionary) or type(v) == types.DictType:
        return dict( (dbus_to_python(k), dbus_to_python(v)) for k,v in v.iteritems() )
    elif isinstance(v, dbus.Array) or type(v) == types.ListType:
        return [dbus_to_python(x) for x in v]
    elif isinstance(v, dbus.Struct) or type(v) == types.TupleType:
        return tuple(dbus_to_python(x) for x in v)
    elif isinstance(v, dbus.Boolean) or type(v) == types.BooleanType:
        return bool(v)
    elif isinstance(v, dbus.ObjectPath) or type(v) == ObjectPath:
        return ObjectPath(v)
    else:
        raise TypeError("Can't convert type %s to python object" % type(v))

#----------------------------------------------------------------------------#
def prettyPrint( expression ):
#----------------------------------------------------------------------------#
    from pprint import PrettyPrinter
    pp = PrettyPrinter( indent=4 )
    pp.pprint( dbus_to_python(expression) )

