# Python 3 conversion notes

This document provides notes on the conversion of the Pipeline code to become
Python 3 compatible.

Python 3 conversion is tracked in 
[CAS-11189](https://open-jira.nrao.edu/browse/CAS-11189)


## Updating code with '2to3' tool

The '2to3' tool has a series of rules to evaluate and convert code to Python 3. 
Below is a summary of which rules have been applied, and which still need to be done.

Rules that have been applied (guard against regression):

```
has_key, print
```

Rules that didn't require any changes (guard against regression):

```
apply, asserts, exitfunc, getcwdu, imports2, input, intern, itertools_imports,
nonzero, operator, paren, renames, sys_exc, standarderror , throw, xreadlines
```

Rules still to be applied:

```
basestring, dict, except, execfile, filter, funcattrs, future, idioms, import,
imports, isinstance, itertools, long, map, metaclass, methodattrs, ne, next,
numliterals, raise, reduce, set_literal, tuple_params, types, unicode, urllib,
ws_comma, xrange, zip
```

Rules still to be applied, affecting external dependencies only:

```
buffer, exec, raw_input, repr
```


## Examples best coding practices
Included below are a series of examples of best coding practices to use, to
ensure that the Pipeline stays Python 3 compatible.


### Print statements
Before:
```
print “bit of text”
```
After:
```
print(“bit of text”)
```

### 'has_key' method in dictionaries
Before:
```
if mydict.has_key(mykey):
```
After:
```
if mykey in mydict:
```

### Raising exceptions
Before:
```
raise Exception “Oh no!”
```
After:
```
raise Exception(“Oh no!”)
```

### Catching exceptions
Before:
```
except Exception, e:
```
After:
```
except Exception as e:
```

### Checking for type

Before:
```
if type(vis) is types.ListType:
```
After:
```
if isinstance(vis, list):
```

### Relative imports within package
Before:
```
import display
```
After:
```
from . import display
```

### Formatted strings
The following change is not necessary for Python 3 compatibility, 
but the "before" has been marked as deprecated in Python 3 and 
may be removed in the future.

Before:
```
“bit of %s” % “text”
```
After:
```
“bit of {}”.format(“text”)
```
