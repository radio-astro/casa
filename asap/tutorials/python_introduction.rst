====================
Python in 20 minutes
====================

.. sectionauthor:: Malte Marquarding

**Main goal:** To get a basic understanding of the python programming language


This is a very quick introduction to the python programming language to get started with ASAP,
which behaves just like any other python module you can find.
It introduces basic programming concepts. 


Variables
=========

A variable is just an alias/handle to a value. The value can be anything understood by python

Example::

	# an integer
	x = 1
	# a string
	y = 'Hello World!'
	# a boolean
	z = True
	# list of ...
	a = [0, 1, 2]
 	b = ['a', 'b']


Syntax
======

Python uses **identation** to define blocks, where other proogramming language often use
curly brackets, e.g.:

in *c*:

.. code-block:: c

   while (i<10) {
     j += il
   }
   // block ends

in *python*:

.. code-block:: python
   
   while i<10:
       j += 10 
   # block ends


Functions
=========

When you need to repeat common behaviour you are better of defining a function, just like it would be
in mathematics. A function can return something ot do no return anything but doing something implictly.

Examples::

	def squared(x):
	    return x*x

	result = squared(2)
	print result
	
	def prefix_print(value):
	    print 'My value:', value

	prefix_print('Hello')

Statements
==========

Often you find you will want to do something conditionally.
For this you can use `if` statements.

Example::

	a = 1
	if a == 1:
	    print 'Match'
	else:

	    print 'No match'

To apply a function to a range of values you can use `for` or `while`

Example::

	i = 0
	while i < 10:
	    print i
	    i +=1
  
	for i in [0, 1, 2]:
	    print i 


Objects
=======

Objects are basically values with certain attributes, which are specific to that type of the object.
For example strings in python have attribute functions which can perform operations on the string:: 

	x = 'Test me'
	print x.upper()
