"""
<?xml version="1.0" encoding="UTF-8"?>
<?xml-stylesheet type="text/xsl" ?>
<casaxml xmlns="http://casa.nrao.edu/schema/psetTypes.html"
xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
xsi:schemaLocation="http://casa.nrao.edu/schema/casa.xsd
file:///opt/casa/code/xmlcasa/xml/casa.xsd">

 
        <tool name="image" module="images">
		<needs>coordsys</needs>
        <shortdescription>Operations on images</shortdescription>
       

include "image.g"



<keyword>image</keyword>
<keyword>calculator</keyword>
<keyword>FITS</keyword>
<keyword>pixels</keyword>
<keyword>statistics</keyword>
<keyword>histograms</keyword>
<keyword>moments</keyword>
<keyword>convolution</keyword>
<keyword>smoothing</keyword>
<keyword>display</keyword>
<code>
	<include>xmlcasa/images/image_forward.h</include>
	<private>
		#include <xmlcasa/images/image_private.h>	</private>
</code>


<description>
\medskip
\noindent{\bf Summary}

Image tools provides access to \casa\ images.    Currently only single
precision floating point \casa\ images are supported by the Image \tool. In the
future, complex images will also be supported.

Image tools also provide direct (native) access to \fits\ and Miriad images.
You can also convert these foreign formats to \casa\ format (for
optimum processing speed).

\bigskip
{\bf Overview of Image \tool\ functionality}

\begin{itemize}

\item {\bf Conversion - } There is functionality to interconvert between
\casa\ images and FITS files. There is also  native access to
a \fits\ file:

\begin{itemize}

\item <link anchor="images:image.fromfits.constructor">fromfits</link> - Convert a FITS image file to a \casa\ image

\item <link anchor="images:image.tofits.function">tofits</link> - convert the image to a \fits\ file

\item <link anchor="images:image.image.constructor">image</link> - native access to a \fits\ file

\end{itemize}

\begin{itemize}

\item <link anchor="images:image.fromascii.constructor">fromascii</link> - Convert an ascii image file to a \casa\ image

\item <link anchor="images:image.fromarray.constructor">fromarray</link> - Convert an array into a \casa\ image

\item <link anchor="images:image.fromshape.constructor">fromshape</link> - Convert a shape into a \casa\ image

\end{itemize}

\end{itemize}

\item {\bf Analysis - } 

\begin{itemize}

\item <link anchor="images:image.decompose.function">decompose</link> - separate a complex image into individual components

\item <link anchor="images:image.deconvolvecomponentlist.function">deconvolvecomponentlist</link> - deconvolve a Componentlist from the restoring beam

\item <link anchor="images:image.fft.function">fft</link> - FFT the image

\item <link anchor="images:image.findsources.function">findsources</link> - Find strong point sources in sky

\item <link anchor="images:image.fitsky.function">fitsky</link> - fit 2-dimensional models to the sky
(see also the <link anchor="images:imagefitter">imagefitter</link> \tool.

\item <link anchor="images:image.fitpolynomial.function">fitpolynomial</link> - fit 1-d polynomials 
to profiles and subtract (see also the <link anchor="images:imageprofilefitter">imageprofilefitter</link> \tool.

\item <link anchor="images:image.fitprofile.function">fitprofile</link> - fit a 1-d profile with varying
combinations of functional forms (see also the <link anchor="images:imageprofilefitter">imageprofilefitter</link> \tool.

\item <link anchor="images:image.fitallprofiles.function">fitallprofile</link> - fit all the
1-d profiles in a region with varying combinations of functional forms 
and write out the fitted and residual images (see also the
<link anchor="images:imageprofilefitter">imageprofilefitter</link> \tool.

\item <link anchor="images:image.histograms.function">histograms</link> - compute histograms from the image

\item <link anchor="images:image.insert.function">insert</link> - insert specified image into this image 

\item <link anchor="images:image.maxfit.function">maxfit</link> - Find maximum and do simple parabolic fit to sky

\item <link anchor="images:image.modify.function">modify</link> - modify image by a model

\item <link anchor="images:image.moments.function">moments</link> - compute moments from image

<!--\item <link anchor="images:image.momentsgui.function">momentsgui</link> - compute moments from image via custom GUI interface-->

\item <link anchor="images:image.regrid.function">regrid</link> - regrid the image to the
specified Coordinate System 

\item <link anchor="images:image.rotate.function">rotate</link> - rotate the coordinate system
and regrid the image to the rotated Coordinate System 

\item <link anchor="images:image.rebin.function">rebin</link> - rebin an image by the specified binning factors

\item <link anchor="images:image.statistics.function">statistics</link> - compute statistics from the image

\item <link anchor="images:image.twopointcorrelation.function">twopointcorrelation</link> - compute
two point autocorrelation functions from the image

\item <link anchor="images:image.subimage.function">subimage</link> (function) -
Create a (sub)image from a region of the image

\end{itemize}

\item {\bf Coordinates - } Manipulation of the coordinate system is handled through

\begin{itemize}

\item <link anchor="images:image.coordmeasures.function">coordmeasures</link> - convert from
pixel to world coordinate wrapped as Measures

\item <link anchor="images:image.coordsys.function">coordsys</link> - recover the Coordinate System into
a <link anchor="images:coordsys">Coordsys</link> \tool.

\item <link anchor="images:image.setcoordsys.function">setcoordsys</link> - set a new Coordinate System

\item <link anchor="images:image.topixel.function">topixel</link> - convert from world coordinate to pixel coordinate

\item <link anchor="images:image.toworld.function">toworld</link> - convert from pixel coordinate to world coordinate

\end{itemize}

The  <link anchor="images:coordsys">coordsys</link> \tool\ provides more extensive
coordinate system manipulation.

\item {\bf Filtering - } Images may be filtered via

\begin{itemize}

\item <link anchor="images:image.convolve.function">convolve</link> - Convolve image with an array or by another image

\item <link anchor="images:image.convolve2d.function">convolve2d</link> - Convolve image by a 2D kernel

\item <link anchor="images:image.sepconvolve.function">sepconvolve</link> - Separable convolution

<!-- \item <link anchor="images:image.sepconvolvegui.function">sepconvolvegui</link> - Separable convolution via custom GUI interface -->

\item <link anchor="images:image.hanning.function">hanning</link> - Hanning convolution along one axis

\end{itemize}

In the future filtering other than convolution will be provided

<!--
\item {\bf Display - } 

\begin{itemize}

\item <link anchor="images:image.view.function">view</link> - display image with the  \viewer\

\end{itemize}
-->

\item {\bf Masks - }   Masks may be manipulated via

\begin{itemize}

\item <link anchor="images:image.calcmask.function">calcmask</link> - Image mask calculator 

\item <link anchor="images:image.maskhandler.function">maskhandler</link> - handle masks (set, copy, delete, recover names)

<!--\item <link anchor="images:image.maskhandlergui.function">maskhandlergui</link> - handle masks via a GUI interface-->

\item <link anchor="images:image.replacemaskedpixels.function">replacemaskedpixels</link> - replace the values of pixels
which are masked bad

\item <link anchor="images:image.set.function">set</link> - set pixel and/or mask values with
a scalar in a \region\ of the image

\item <link anchor="images:image.summary.function">summary</link> - lists the mask names
\end{itemize}

\item {\bf Pixel access - }  The pixel and mask values for an image may be
accessed and calculated with via

\begin{itemize}

\item <link anchor="images:image.imagecalc.constructor">imagecalc</link> - Create image tool with image calculator

\item <link anchor="images:image.image.constructor">image</link> - Create an image tool from a \casa\ image

\item <link anchor="images:image.calc.function">calc</link> - Image pixel calculator 

\item <link anchor="images:image.calcmask.function">calcmask</link> - Image mask calculator 

\item <link anchor="images:image.getchunk.function">getchunk</link> - get the pixel values
from a regular region of the image into an array

\item <link anchor="images:image.getregion.function">getregion</link> - get pixels and mask from a \region\ of the image

\item <link anchor="images:image.getslice.function">getslice</link> - get a 1-D slice from the image

\item <link anchor="images:image.pixelvalue.function">pixelvalue</link> - get image value for specified pixel

\item <link anchor="images:image.putchunk.function">putchunk</link> - put pixels from an array into a regular region of the image

\item <link anchor="images:image.putregion.function">putregion</link> - put pixels and mask
into a \region\ of the image

\item <link anchor="images:image.set.function">set</link> - set pixel and/or mask values with
a scalar in a \region\ of the image

\end{itemize}

\item {\bf Inquiry - } Functions to report basic information about the
image are
  
\begin{itemize}
\item <link anchor="images:image.boundingbox.function">boundingbox</link> - find bounding box of a \region.

\item <link anchor="images:image.brightnessunit.function">brightnessunit</link> - Get image brightness unit

\item <link anchor="images:image.haslock.function">haslock</link> - does this image have a lock set

\item <link anchor="images:image.history.function">history</link> - recover/list history file

<!--\item <link anchor="images:image.id.function">id</link> - Return the fundamental identifier of this \tool\-->

\item <link anchor="images:image.ispersistent.function">ispersistent</link> - is the image persistent (on disk)

\item <link anchor="images:image.name.function">name</link> - name of the \imagefile\ this tool is attached to

\item <link anchor="images:image.restoringbeam.function">restoringbeam</link> - Get restoring beam

\item <link anchor="images:image.shape.function">shape</link> - the length of each axis in the image

\item <link anchor="images:image.summary.function">summary</link> - summarize basic information about the image

\item <link anchor="images:image.type.function">type</link> - the type of this Image \tool\

\end{itemize}

\item {\bf Utility - }  There is wide range of utility services available
through the functions

\begin{itemize}

\item <link anchor="images:image.adddegaxes.function">adddegaxes</link> - Add degenerate axes

\item <link anchor="images:image.addnoise.function">addnoise</link> - Add noise to the image

\item <link anchor="images:image.brightnessunit.function">brightnessunit</link> - Get image brightness unit

\item <link anchor="images:image.close.function">close</link> - Close the \imagetool\ (but don't destroy it)

\item <link anchor="images:image.convertflux.function">convertflux</link> Convert flux density between
peak and integral

\item <link anchor="images:image.close.function">close</link> - close this \imagetool\

\item <link anchor="images:image.haslock.function">haslock</link> - does this image have a lock set

\item <link anchor="images:image.history.function">history</link> - recover/list history file

\item <link anchor="images:image.imagefiles.function">imagefiles</link> - Find the names of all image files in the given directory

\item <link anchor="images:image.imagetools.function">imagetools</link> - Find the names of all global image \tools\

\item <link anchor="images:image.is_image.function">is\_image</link> - Is this variable an Image \tool\

\item <link anchor="images:image.isopen.function">isopen</link> - Is this Image \tool\ open?

\item <link anchor="images:image.lock.function">lock</link> - acquire a lock on the image

\item <link anchor="images:image.makecomplex.function">makecomplex</link> - make a complex image from two real images

\item <link anchor="images:image.miscinfo.function">miscinfo</link> - recover miscellaneous information record

\item <link anchor="images:image.open.function">open</link> - open a new \imagefile\ with this image tool

\item <link anchor="images:image.rename.function">rename</link> - rename the \imagefile\ associated with this Image \tool\

\item <link anchor="images:image.restoringbeam.function">restoringbeam</link> - Get restoring beam

\item <link anchor="images:image.remove.function">remove</link> - remove the \imagefile\ associated with this Image \tool\

\item <link anchor="images:image.setbrightnessunit.function">setbrightnessunit</link> - Set image brightness unit

\item <link anchor="images:image.sethistory.function">sethistory</link> - set the history file

\item <link anchor="images:image.setmiscinfo.function">setmiscinfo</link> - set the miscellaneous information record

\item <link anchor="images:image.setrestoringbeam.function">setrestoringbeam</link> - Set new restoring beam

\item <link anchor="images:image.unlock.function">unlock</link> - release lock on this \imagefile\

\end{itemize}

\item {\bf Reshaping - }  Images can be reshaped via

\begin{itemize}

\item <link anchor="images:image.fromimage.constructor">fromimage</link>
- Create a (sub)image from a region of a \casa\ image

\item <link anchor="images:image.subimage.function">subimage</link> -
Create a (sub)image from a region of the image

\item <link anchor="images:image.insert.function">insert</link> - insert specified image into this image 

\item <link anchor="images:image.imageconcat.constructor">imageconcat</link> - Concatenate \casa\ images

\end{itemize}

<!--\item {\bf Tests and demos - }  

\begin{itemize}

\item <link anchor="images:imagetest.function">imagetest</link> - Run test suite for Image \tool\

\item <link anchor="images:imagedemo.function">imagedemo</link> - Demonstrate the use of an image

\end{itemize}-->

\end{itemize}

\medskip
\noindent{\bf General}

We refer to a \casa\ \imagefile\ when we are referring to the actual
data stored on disk.  The name that you give a \casa\ \imagefile\ is
actually the name of a directory containing a collection of \casa\
tables which together constitute the \imagefile.  But you only need to
refer to the directory name and you can think of it as one {\it logical}
file.  

Whenever we use the word ``image'', we are just using it in a generic
sense.  \casa\ images are manipulated with an <link
anchor="images:image.image.constructor">Image</link> tool <!--.  An
\imagetool\ is--> associated with, or bound to, the actual
\imagefile.\ <!-- via a \constructor.--> Note that some \imagetools\
don't have a disk file associated with them.  These are called
``virtual'' images and are discussed
\htmlref{below}{IMAGES:VIRTUALIMAGES}

When an image is stored on disk, it can, in principle, be stored in a
variety of ways.  For example, the image could be stored row by row;
this is the way that most older generation packages store images.  It
makes for very fast row by row access, but very slow in other
directions (e.g.  extract all the profiles along the third axis of an
image).  A \casa\ \imagefile\ is stored with what is called tiling.
This means that small multi-dimensional chunks (a tile) are stored
sequentially.  It means that row by row access is a little slower, but
access speed is essentially the same in all directions.  This in turn
means that you don't need to (and can't !) reorder images.

<example> 
\bigskip
Here are some simple examples using image tools.

\begin{verbatim}
"""
#
print "\t----\t Intro Ex 1 \t----"
ia.maketestimage('zz',overwrite=true)# Make test image; writes disk file called 'zz'
print ia.summary()                   # Summarize (to logger)
print ia.statistics()                # Evaluate statistics over entire image
#box = ia.setboxregion([10,10],[50,50])  # Make a pixel box region
box = rg.box([10,10], [50,50])       # Make a pixel box region with regionmanager
im2 = ia.subimage('zz2', box, overwrite=true)  # Make a subimage  called 'zz2'
print im2.statistics()                    # Evaluate statistics
print "CLEANING UP OLD zz2.amp/zz2.phase IF THEY EXIST.  IGNORE WARNINGS!"
ia.removefile('zz2.amp')
ia.removefile('zz2.phase')
im2.fft(amp='zz2.amp',phase='zz2.phase')  # FFT subimage and store amp and phase
im2.done()      # Release tool resources - disk file unaffected
ia.close()      # DO NOT DONE DEFAULT IMAGE TOOL ia!!!
#
"""
\end{verbatim}
<!--#ia.view()                           # Display image-->
</example>

\medskip
\noindent {\label{IMAGE:FOREIGNIMAGES}\bf Foreign Images}

The Image \tool\ also provides you with native access to some foreign
image formats.  Presently, these are \fits\ (Floar, Double, Short and Long
are supported) and Miriad.   This means
that you don't have to convert  the file to native \casa\ format
in order to access the image.   For example:

<example> 
\begin{verbatim}
"""
#
print "\t----\t Intro Ex 2 \t----"
pathname=os.environ.get("CASAPATH") # Assumes environment variable is set
pathname=pathname.split()[0]
datapath1=pathname+"/data/demo/Images/imagetestimage.fits"
datapath2=pathname+"/data/demo/Images/test_image"
ia.open(datapath1)              # Access FITS image
#ia.open('im.mir')              # Access Miriad image (no image in repository)
ia.open(datapath2)              # Access casa image
#
#ims = ia.newimagefromimage(infile=datapath1, region=rg.quarter())
# rg.quarter() not implemented yet so has grabbed entire image
ims = ia.newimagefromimage(infile=datapath1)
innerquarter=ims.setboxregion([0.25,0.25],[0.75,0.75],frac=true)
subim = ims.subimage(region=innerquarter)
print ia.name()
print ims.name()
print subim.name()
ims.done()                      # done on-the-fly image tool
subim.done()                    # done on-the-fly image tool
ia.close()                      # close (not done) default image analysis tool
#
"""
\end{verbatim}
Each of these Image tools has access to all the same \toolfunctions.
</example>

Where ever you see an argument in an Image \tool\ function which is
an input image disk file, that disk file can be a \casa, \fits, or
Miriad image file. 

There are some performance penalties that you should be aware of. 
Firstly, because \casa\ images are tiled (see above) you get the same
access speed regardless of how you access the image.  \fits\ and Miriad
images are not tiled.  This means that the performance for these Image
tools will be poorer for certain operations.  For example, extracting a
profile along the third axis of an image, or re-ordering an image with
the display library. 

Secondly, for \fits\ images, masked values are indicated via ``magic
value''.  This means that the mask is worked out on the fly every time
you access the image. 

If you find performance is not good enough or you want a writable image,
then use appropriate function
(<link anchor="images:image.fromfits.constructor">fromfits</link> to
convert to a native \casa\ image. 

\medskip
\noindent {\label{IMAGE:VIRTUALIMAGES}\bf Virtual Images}

We also have Image \tools\ that are not associated one-to-one with disk
files; these are called ``virtual'' images (see also the article in the
\htmladdnormallink{AugustNewsLetter}{../../newsletters/aug00/aug00.html}). 
For example, with the image calculator,
<link anchor="images:image.imagecalc.constructor">imagecalc</link>, one can create
an expression which may contain many images.  You can write the result
of the expression out to a disk \imagefile, but if you wish, you can
also just maintain the expression, evaluating it each time it is needed
- nothing is ever written out to disk in this case.  There are other
Image <!-- constructors and--> functions like this (the documentation for each
one explains what it does).  The rules are:

\begin{itemize}

\item  If you specify the {\stfaf outfile} argument, then
the image is always written to the specified disk \imagefile. 

\item If you leave the {\stfaf outfile} argument unset, then if
possible, a virtual image will be created.  Sometimes this virtual image
will be an expression as in the example above (i.e.  it references other
images) or a temporary image in memory, or a temporary image on disk. 
(the <link anchor="images:image.summary.function">summary</link> function will list for you
the type of image you have). When you destroy that Image tool, the
virtual image will be destroyed as well.   

\item If you leave {\stfaf outfile} unset, and the function <!-- or
constructor --> cannot make a virtual image, it will create a disk file for
you with a name of its choice (usually input plus function name). 

\item You can always write a virtual image to disk with the
\htmlref{subimage}{images:image.subimage.function} \toolfunction.  

\end{itemize}

\medskip
\noindent {\bf Coordinate Systems}

An image contains a Coordinate System.  A
<link anchor="images:coordsys">Coordsys</link> \tool\ is used to manipulate the
Coordinate System.  An Image \tool\ allows you to recover the Coordinate
System into a Coordsys \tool\ through the
<link anchor="images:image.coordsys.function">coordsys</link> function.  You can set a new
Coordinate System with the
<link anchor="images:image.setcoordsys.function">setcoordsys</link> function. 

You can do some direct coordinate conversion via the Image \tool\
functions <link anchor="images:image.toworld.function">toworld</link>,
<link anchor="images:image.topixel.function">topixel</link>, and
<link anchor="images:image.coordmeasures.function">coordmeasures</link>.  The actual work is
done by a Coordsys \tool, for which these Image \tool\ functions are
just wrappers.

\medskip
\noindent {\bf Lattice Expression Language (LEL)}

LEL allows you to manipulate expressions involving images.  For
example, add this image to that image, or multiply the miniumum value of
that image by the square root of this image.  The LEL syntax is quite
rich and is described in detail in \htmladdnormallink{note
223}{../../notes/223/223.html}. 
          
LEL is accessed via the <link
anchor="images:image.imagecalc.constructor">imagecalc</link> <!--
constructor --> and the <link
anchor="images:image.calc.function">calc</link> \tool\ functions.
Here are some examples.

<example> 
\begin{verbatim}
"""
#
print "\t----\t Intro Ex 3 \t----"
ia.maketestimage('zz', overwrite=true) # Make nonvirtual test image
ia.calc('zz + min(zz)')                # Make the minimum value zero
ia.close()
#
"""
\end{verbatim}
<!--
#myim = ia.maketestimage()         # Make virtual test image
#ia.calc('$im + min($im)')         # Make the minimum value zero
#myim.done()
-->

<!-- In the first example, the Image \tool\ {\stf im} is associated with
a virtual image.  We use the \$ syntax in the expression accessing
this image via its \tool.  In the second example, the Image \tool\
{\stf im2} is associated with the non-virtual disk file {\sff zz}.
We could still have used the \$ syntax if we wanted, but we show
here how you can also use the \imagefile\ name in the expression.-->

In this example the Image \tool\ is associated with the non-virtual
disk file {\sff zz}.  This \imagefile\ name is used in an LEL
expression.

</example>

\bigskip
<example> 
Note that for image file names with special characters in them (like a
dash for example), you should (double) escape those characters or put
the file name in double quotes. E.g.

\begin{verbatim}
"""
#
print "\t----\t Intro Ex 4 \t----"
ia.maketestimage("test-im", overwrite=true)
im1 = ia.imagecalc(pixels='test\\-im')  # Note double escape required
im2 = ia.imagecalc(pixels='"test-im"')
im1.done()
im2.done()
ia.close()
#
"""
\end{verbatim}
</example>
          

\medskip
\noindent{\bf Region-of-interest}

A \region\, or simply, region, designates which pixels of the image you
are interested in for some (generally) astrophysical reason.  This
complements the \pixelmask\ (see below) which specifies which pixels are
good or bad (for statistical reasons).  \Regions\ are generated and
manipulated with the <link anchor="images:regionmanager">Regionmanager</link> tool.

Briefly, a \region\ may be either a simple shape such as a
multi-dimensional box, or a 2-D polygon, or some compound combination of
\regions.  For example, a 2-D polygon defined in the X and Y axes
extended along the Z axis, or perhaps a union or intersection of
regions. 

See the <link anchor="images:regionmanager">Regionmanager</link> documentation for
more details on regions.

Regions are always supplied to <!-- constructors and --> \toolfunctions\
via the {\stfaf region} argument.

\medskip
\noindent{\bf Pixel mask}

A \pixelmask\ specifies which pixels are to be considered good (value
{\cf T}) or bad (value {\cf F}).  For example, you may have  imported a
\fits\ file which has blanked pixels in it.  These will be converted
into \pixelmask\ elements whose values are bad ({\cf F}).  Or  you may
have made an error analysis of an image and computed via a statistical
test that certain pixels should be masked out for future analysis.

If there is no \pixelmask, all pixels are considered good (if you
retrieve the \pixelmask\ when there is none, you will get an all good
mask).  Pixels for which the \pixelmask\ value is bad are not used in
computations (e.g. in the calculation of statistics, moments or
convolution).

The image may contain zero, one, or more \pixelmasks.  However, only one
mask will be designated as the default mask.  This is the \pixelmask\
that is actually applied to the data.  You can also indicate that none
of the \pixelmasks\ are the default, so that effectively an all good
\pixelmask\ is applied.  The function
<link anchor="images:image.summary.function">summary</link> includes in its summary of the
image the names of the masks (the first listed, if not in square
brackets, is the default). 

\Pixelmasks\ are handled with the function
<link anchor="images:image.maskhandler.function">maskhandler</link>.  This allows you to find
the names of \pixelmasks, delete them, copy them, nominate the default and so
on.  It is not used to change the value of \pixelmasks. 

The functions with which you can change \pixelmask\ values are
<link anchor="images:image.putregion.function">putregion</link> (put Boolean array),
<link anchor="images:image.calcmask.function">calcmask</link> (put result of Boolean
LEL expression), and <link anchor="images:image.set.function">set</link> (put scalar Boolean). 

\medskip
\noindent {\bf The argument 'mask'}

There is an argument, {\stfaf mask}, which can be supplied to many
<!-- constructors and --> functions.  It is supplied with either a
mask \region\ (generated via the function <link
anchor="images:regionmanager.wmask.function">wmask</link>) or a
\htmladdnormallink{LEL}{../../notes/223/223.html} Boolean expression
string (the same string you would have supplied to the above
Regionmanager function).  Generally, one just supplies the expression
string.

The LEL expression is simply used to generate a \pixelmask\ which is then applied
in addition to any default \pixelmask\ in the image (a logical OR). For example

\begin{verbatim}
"""
#
print "\t----\t Intro Ex 5 \t----"
ia.maketestimage('zz', overwrite=true)
ia.statistics(mask='zz > 0')      # Only evaluate for positive values
ia.calcmask (mask='(2*zz) > 0')   # Create a new mask which is T (good)
                                  # when twice the image values are
                                  # positive, else F
ia.close()
#
"""
\end{verbatim}
<!--#ia.calcmask (mask='(2*$im) > 0') # Create a new mask which is T (good)-->

The {\stfaf mask} expression must in general conform (shape and
coordinates) with the image (i.e.  that associated with the Image tool). 
<!--You can use the \$ substitution syntax for Image tools (see example
above) which is necessary for virtual images and useful otherwise. -->

When {\stfaf mask} is used with function
<link anchor="images:image.calcmask.function">calcmask</link>, a persistent \pixelmask\ is
created and stored with the image.  With all other functions, <!-- and
constructors, --> the {\stfaf mask} argument operates as a transient (or
On-The-Fly [OTF]) \pixelmask.  It can be very handy for analysing or
displaying images with different masking criteria. 

Often I will refer to the ``total input mask''.  This is the
combination (logical OR) of the default \pixelmask\ (if any)
and the OTF mask (if any).

\bigskip
In the following example we open a Rotation Measure image.  We then
evaluate statistics and display it where only those pixels whose error
in the Rotation Measure (\imagefile\ {\sff rmerr}) is less than the specified
value are shown; the others are masked.  The nice thing is you can
experiment with different \pixelmasks\ until you are satisfied, whereupon you
might then make the \pixelmask\ persistent with the
<link anchor="images:image.calcmask.function">calcmask</link> function. 

<example> 
\begin{verbatim}
"""
#
print "\t----\t Intro Ex 6 \t----"
#myim = ia.newimagefromimage('rm')
#myim.statistics(mask='rmerr<10')
#myim.calcmask (mask='rmerr<20')     # Make persistent mask
#
"""
\end{verbatim}
<!-- #myim.view(mask='rmerr<20')-->
</example>

\medskip
Finally, a subtlety that is worth explaining.

<example> 
\begin{verbatim}
"""
#
print "\t----\t Intro Ex 7 \t----"
ia.maketestimage('zz', overwrite=true)
ia.statistics(mask='zz>0')                 # Mask of zz ignored
ia.statistics(mask='mask(zz) && zz>0')     # Mask of zz used
ia.close()
#
"""
\end{verbatim}

In the first example, any default mask associated with the image {\sff
zz} is ignored.  Only the pixel values are looked at.  In the second
example, the mask of {\sff zz} is also taken into account via the LEL
{\cf mask} function. That is, the transient output mask is T (good) only when
the mask of {\sff zz} is T and the expression {\cf zz>0} is T.

</example>


\bigskip
<example> 

A useful part of LEL to use with the {\stfaf mask} argument is the
{\cf indexin} function.  This enables the user to specify a mask based
upon selected pixel coordinates or indices (specified 0-rel) rather
than image values.  For example

\goodbreak
\begin{verbatim}
"""
#
print "\t----\t Intro Ex 8 \t----"
ia.fromshape(shape=[20])
print ia.getregion(mask='indexin(0, [4:9, 14, 18:19])',getmask=true)
#[False False False False True True True True True True False False False
# False True False False False True True]
ia.close()
#
"""
\end{verbatim}

You can see the mask is good (T) for the specified indices along the
specified axis.  You can also pass in a premade variable for the
specification if you like, viz.

\goodbreak
\begin{verbatim}
"""
#
print "\t----\t Intro Ex 9 \t----"
ia.fromshape(shape=[20])
axis = "0"
sel = "[4:9, 14, 18:19]"
print ia.getregion(mask='indexin('+axis+','+sel+')',getmask=true)
#[False False False False True True True True True True False False False
# False True False False False True True]
ia.close()
#
"""
\end{verbatim}

This capability is useful for fitting functions.
See the example in the <link anchor="images:image.fitpolynomial.function">fitpolynomial</link> function. 
</example>

\bigskip
\noindent{\label{IMAGE:MASKSANDREGIONS}\bf Pixel masks and Regions}

Some comment about the combination of \pixelmasks\ and \regions\ is useful
here.    See the <link anchor="images:regionmanager">Regionmanager</link> tool
for basic information about \regions\ first.  

Regions are provided to Image \toolfunctions\ via the standard {\stfaf region}
<!--constructor and--> function argument.

Consider a simple polygonal region.  This \region\ is defined by
a bounding box, the polygonal vertices, and a mask called a \regionmask.
The \regionmask\ specifies whether a pixel within the bounding box is
inside or outside the polygon.  For a simple box \region, there is
obviously no need for a \regionmask.

Now imagine that you wish to recover the \pixelmask\ of an image from a
polygonal \region.  The mask is returned to you in
regular Boolean array.  Thus, the shape of the returned mask array reflects the
<link anchor="images:image.boundingbox.function">bounding-box</link> of the polygonal region. 
If the actual \pixelmask\ that you apply is all good, then the retrieved
mask would be good inside of the polygonal region and bad outside of it. 
If the actual \pixelmask\ had some bad values in it as well, the
retrieved mask would be bad outside of the polygonal region.  Inside the
polygonal region it would be bad if the \pixelmask\ was bad. 
More simply put, the mask that you recover is just a logical ``and'' of
the \pixelmask\ and the \regionmask; if the \pixelmask\ is T {\it and} the
\regionmask\ is T then the retrieved mask is T (good), else it is F (bad).

Finally, note that if you use the {\stfaf region} and {\stfaf mask} (the OTF
mask) arguments together then they operate as follows.  The shape
of the Boolean expression provided by  {\stfaf mask} must be the same
shape as the image to which it is being applied.  The {\stfaf region}
is applied equally to the image and the {\stfaf mask} expression.
For example

<example> 
\begin{verbatim}
"""
#
print "\t----\t Intro Ex 10 \t----"
#rm1 = ia.newimagefromimage('rm')
#rm2 = ia.newimagefromimage('rmerr')
#rm1.shape()
#[128 128]
#rm2.shape()
#[128 128]
#r = rg.box([10,10], [50,50])
#rm1.statistics(region=r, mask='rmerr<10') # region applied to
                                              # 'rmerr' and 'rm'
#
"""
\end{verbatim}
</example>

<!--
\bigskip
\noindent{\bf Error Handling}

All the constructors and functions for Image \tools\ return a variable by
value.  Because of the weak typing of \glish, this variable may take on
different forms depending upon the outcome.  For example, a constructor
such as {\cf ia.image('hcn.xyv')} normally returns an \imagetool. 
However, if the operation failed (e.g.  the \imagefile\ {\sff hcn.xyv})
does not exist, them the return variable will be what is called a
\glish\ fail.  You can check any \glish\ variable with the {\cf
is\_fail(variable)} function; it returns T or F.  With an Image tool,
all errors will cause the returned variable, whether it comes from a
constructor or a method, to be a fail.  You will not get a false ({\cf
F}) return unless this is a legal return value for the function.  This is so
that you always know what to check the return variables value for if
you are doing careful error checking in a script.  The documentation for
each constructor and function always tells you what the return variable
type could potentially be. 

<example> 
\begin{verbatim}
"""
#
#print "\t\t Intro Ex 11 \t"
# ia.image('rm')
# if (is_fail(im)) {
#  note (im::message, origin='myscript', priority='SEVERE')
#  return;
# }
#
"""
\end{verbatim}
The scoping operator :: enables to extract out a diagnostic
message from the variable {\cf im} when it is a fail.
</example>
-->
<!--
\bigskip
\noindent{\label{IMAGE:VECTORINPUT}\bf Vector Inputs}

Many of the functions of an Image \tool\ take vectors (numeric or
strings) as their arguments.  We take advantage of the weak-typing of
Glish so that whenever you see a vector argument, you can assume that

\begin{itemize}

\item For numeric vectors you can enter as an actual numeric vector
(e.g. [1.2,2.5,3]), a vector of strings (e.g. "1.2 2.5 3") or even
a string with white space and/or comma delimiters (e.g. '1.2 2.5 3').

\item For string vectors you can enter as an actual vector of strings
(e.g.  "1.2 2.5 3") or a string with white space or comma delimiters
(e.g.  '1.2 2.5 3'). 

\end{itemize}
-->

<!--
\bigskip
\noindent{\bf Events}

An \imagetool\ may emit five events.  These are generated by the
<link anchor="images:image.view.function">view</link> display function.  See it for details
of the event values. 

\begin{itemize}
\item A {\tt region} event.   This signifies that
a \region\ has been interactively generated 
via the \viewer.

\item A {\tt position} event.   This signifies that
a position has been interactively generated 
via the \viewer.

\item A {\tt statistics} event.   This signifies that
statistics have been interactively generated 
via the \viewer.

\item a {\tt breakfromviewer} event.  This signifies that
the user has pressed the `break' button on the 
\viewer\ display panel.

\item a {\tt viewerdone} event.  This signifies that
the private \viewer\ inside the Image \tool\ has been
destroyed.

\end{itemize}

\bigskip
{\bf Short-hand names for functions}

Many of the function names are rather long to type (e.g.  boundingbox,
replacemaskedpixels).  This doesn't matter with a GUI interface, but can
be annoying if you are writing scripts or using \casa\ interactively. 
For such long-winded function names, a short-hand synonym is defined and
given at the beginning of the documentation for each function.

-->

<!--
\bigskip
{\bf Miscellaneous}

At present only real-valued (single precision) images are supported. 
Complex images will be supported in the future. 

Besides the functions available as \toolfunctions, you may of
course add functionality through Python programming.  Python computing
speed will be adequate for many ``chunk-by-chunk'' algorithms.  Beware
putting large images into an array - this can use up a lot of memory!

Some of the \toolfunctions\ in this module can be run asynchronously
(usually things that might take a long time).  This is controlled by the
argument {\stfaf async} which defaults to {\stfaf !dowait} (i.e.  the
opposite to the value of this Python Boolean variable).  By default
when you start \casa\ {\stfaf dowait=F}.  You can enter {\stfaf dowait
= T} if you wish the default to be that these tool functions run
synchronously.  If you don't want to type this in every time you start
\casa\, then put this statement in your {\sff .glishrc} resource file
that lives in your home directory. 

Note that if the \toolfunction\ is run asynchronously, you have to recover the
result in a more indirect fashion as follows in the following example.

\begin{verbatim}
- jobnum = ia.statistics(async=T) 
- statsout = defaultservers.result(jobnum) 
\end{verbatim}

-->

</description> 

<!--
   <method type="constructor" name="image">
   <shortdescription>Construct an image tool from a \casa\ image file</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="infile">
     <description>Input image file name</description>
     </param>
</input>
<returns type="casaimage"/>
<description>

This is the most commonly used constructor.  It constructs (creates) a 
\imagetool\ associated with disk \imagefile.

The disk image file may be a native \casa, \fits\ (Float, Double,
Short, Long are supported) , or Miriad
\imagefile.  Look \htmlref{here}{IMAGES:FOREIGNIMAGES}  for more
information on foreign images.

</description>

<example>
\begin{verbatim}
"""
#
#ia.image('myimage')         # Access native aips++ image
# ia.view()
#
#im2 = image('hcn.fits')       # Access FITS image
#im2.view()
#
"""
\end{verbatim}
</example>
</method>
-->

   <method type="function" name="newimage">
   <shortdescription>Construct a new image tool from a \casa\ image file.  
(Also know as newimagefromfile.)</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="infile">
     <description>Input image file name</description>
     </param>
  
</input>
<returns type="casaimage"/>
<description>
This is the most commonly used function to create an on-the-fly image
tool.  It constructs (creates) a \imagetool\ associated with a disk
\imagefile.

The disk image file may be a native \casa, \fits\ (Float, Double,
Short, Long are supported) , or Miriad
\imagefile.  Look \htmlref{here}{IMAGES:FOREIGNIMAGES}  for more
information on foreign images.
</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t newimage Ex 1 \t----"
ia.maketestimage('test1',overwrite=true)
ia.maketestimage('test2',overwrite=true)
print ia.name()
#[...]test2
im1=ia.newimage('test1')
print im1.name()
#[...]test1
print im1.summary()
im2=ia.newimage('test2')
print im2.name()
#[...]test2
print im1.name()
#[...]test1
im1.done()
im2.done()
ia.close()
#
"""
\end{verbatim}
</example>
</method>


   <method type="function" name="newimagefromfile">
   <shortdescription>Construct a new image tool from a \casa\ image file.
(Also know as newimage.)</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="infile">
     <description>Input image file name</description>
     </param>
  
</input>
<returns type="casaimage"/>
<description>
This is the most commonly used function to create an on-the-fly image
tool.  It constructs (creates) a \imagetool\ associated with a disk
\imagefile.

The disk image file may be a native \casa, \fits\ (Float, Double,
Short, Long are supported) , or Miriad
\imagefile.  Look \htmlref{here}{IMAGES:FOREIGNIMAGES}  for more
information on foreign images.
</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t newimagefromfile Ex 1 \t----"
ia.maketestimage('test1',overwrite=true)
ia.maketestimage('test2',overwrite=true)
print ia.name()
#[...]test2
im1=ia.newimagefromfile('test1')
print im1.name()
#[...]test1
print im1.summary()
im2=ia.newimagefromfile('test2')
print im2.name()
#[...]test2
print im1.name()
#[...]test1
im1.done()
im2.done()
ia.close()
#
"""
\end{verbatim}
</example>
</method>

   <method type="function" name="imagecalc">
   <shortdescription>Construct a \casa\ image via the image calculator</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="string" direction="in" name="pixels">
     <description>LEL expression.  Default is empty expression.</description>
     <value></value>
     </param>
  
     <param type="bool" direction="in" name="overwrite">
     <description>Overwrite (unprompted) pre-existing output file?</description>
     <value>false</value>
     </param>
</input>
<returns type="casaimage"/>

<description> 

This function is used to evaluate a mathematical expression involving
existing \imagefiles, assigning the result to the newly created output
image.  The syntax of the expression supplied via the {\stfaf pixels}
argument (in what is called the Lattice Expression Language, or LEL) is
explained in detail in \htmladdnormallink{note
223}{../../notes/223/223.html}.

Any image files embedded in the expression may be native \casa\ or
\fits\ (but not yet Miriad) \imagefiles.  Look
\htmlref{here}{IMAGES:FOREIGNIMAGES} for more information on foreign
images. 

If {\stfaf outfile} is given, the image is written to the specified
disk file.  If {\stfaf outfile} is unset, the on-the-fly Image \tool\
returned by this function actually references all of the input files
in the expression.  So if you deleted any of the input image disk
files, it would render this \tool\ useless.  When you destry this
on-the-fly \tool\ (with the <link
anchor="images:image.done.function">done</link> function), the
reference connections are broken.

If you make a reference \tool, the expression is re-evaluated every time
you use it.   So it takes no extra storage, but takes longer
to access the pixel values (they must be recomputed).

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t imagecalc Ex 1 \t----"
ia.fromshape('aF',[10,10],overwrite=true)
ia.fromshape('bF',[10,20,30],overwrite=true)
myim= ia.imagecalc(outfile='cF', pixels='aF + min(float(pi()), mean(bF))',
                   overwrite=true)
myim.done()
ia.close()
#
"""
\end{verbatim}

Suppose {\sff aF} and {\sff bF} are \imagefiles\ with single precision
data.  They do not need to have conforming shapes and coordinates,
because only the mean of {\sff bF} is used.  Note that {\tt pi} is
explicitly converted to single precision, because {\cf pi()} results in
a Double.  If that was not done, the expression result would be a Double
with the effect that all data of {\sff aF} had to be converted to
Double.  So this expression first computes a scalar, the minimum of
$\pi$ and the mean of the \imagefile\ {\sff bF}.    That scalar is then
added to the value of each pixel in the \imagefile\ {\sff aF}.  The
resultant is put in the \imagefile\ called {\sff cF} to which access is
provided via the \imagetool\ called {\stf myim}.

If the expression is masked, that mask will be copied to the new image.

</example>

<example>
\begin{verbatim}
"""
#
print "\t----\t imagecalc Ex 2 \t----"
ia.fromshape('aF',[10,10],overwrite=true)
myim=ia.imagecalc('cF', 'min(aF, (min(aF)+max(aF))/2)', overwrite=true)
myim.done()
ia.close()
#
"""
\end{verbatim}

This example shows that there are 2 {\cf min} functions.  One with a
single argument returning the minimum value of that image.  The other
with 2 arguments returning an image containing {\sff aF} data clipped at
the value of the 2nd argument. 

</example>

<example>
\begin{verbatim}
"""
#
print "\t----\t imagecalc Ex 3 \t----"
ia.fromshape('aD',[10,10],overwrite=true)
ia.fromshape('aF',[10,10],overwrite=true)
ia.fromshape('bF',[10,10],overwrite=true)
ia.fromshape('aC',[10,10],overwrite=true)
myim = ia.imagecalc('eF', 'sin(aD)+(aF*2)+min(bF)+real(aC)', overwrite=true)
myim.done()
ia.close()
#
"""
\end{verbatim}
This shows a mixed type expression.  The real part of the Complex image
{\sff aC}  is used in an expression that otherwise uses Float
type.  The result of the expression is written out to the
disk file called {\sff eF}.
</example>

<example>
\begin{verbatim}
"""
#
print "\t----\t imagecalc Ex 4 \t----"
ia.fromshape('aD',[10,10],overwrite=true)
ia.fromshape('aF',[10,10],overwrite=true)
ia.fromshape('bF',[10,10],overwrite=true)
ia.fromshape('aC',[10,10],overwrite=true)
myim = ia.imagecalc(pixels='sin(aD)+(aF*2)+min(bF)+real(aC)')
myim.done()
ia.close()
#
"""
\end{verbatim}
This shows a mixed type expression.  The real part of the Complex image
{\sff aC}  is used in an expression that otherwise uses Float
type.  The result of the expression is not written out to disk
in this case.  It is evaluated each time it is needed.
</example>
</method>

   <method type="function" name="imageconcat">
   <shortdescription>Construct a \casa\ image by concatenating images</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="infiles">
     <any type = "variant"/>
     <description>List of input \casa\ image files to concatenate;
wild cards accepted.  Default is empty string.</description>
     <value></value>
     </param>
  
     <param type="int" direction="in" name="axis">
	     <description>Concatenation pixel axis.  Default is spectral axis if there is one.</description>
     <value>-10</value>
     </param>
  
     <param type="bool" direction="in" name="relax">
     <description>Relax constraints that axis coordinate descriptors match</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="tempclose">
     <description>Keep all lattices closed until needed</description>
     <value>true</value>
     </param>
  
     <param type="bool" direction="in" name="overwrite">
     <description>Overwrite (unprompted) pre-existing output file?</description>
     <value>false</value>
     </param>
</input>
<returns type="casaimage"/>

<description> 

This function is used to concatenate two or more input \casa\
images into one output image.  For example, if you have two image cubes
which are contiguous along one axis (say a spectral axis) and you would
like to glue them together along this axis, then this function is the
appropriate thing to use. 

If {\stfaf outfile} is given, the image is written to the specified
disk file.  If {\stfaf outfile} is unset, the on-the-lfy Image \tool\
created by the function actually references all of the input files.
So if you deleted any of the input image disk files, it would render
this \tool\ useless.  When you destroy this tool (with the <link
anchor="images:image.done.function">done</link> function) the
reference connections are broken.

The input and output images must be of the same dimension.  Therefore,
if you wish to concatenate 2-D images into a 3-D image, the 2-D images
must have a third axis (of length unity) so that the output image
coordinates are known along the concatenation axis. 

The input images are concatenated in the order in which they are listed. 
Therefore, the Coordinate System of the output image is given by that of the
first input image.  The input images are checked to see that they are
contiguous along the concatenation axis and an error is generated if
they are not.  In addition, the coordinate descriptors (e.g.  reference
pixel, reference value etc) for the non-concatenation axes must be the
same or an error will result. 

The input disk image files may be in native \casa, \fits, or Miriad
formats.  Look \htmlref{here}{IMAGES:FOREIGNIMAGES}  for more
information on foreign images.

The contiguous criterion and coordinate descriptor equality criteria can
be relaxed by setting {\stfaf relax=T} whereupon only warnings will be
issued.  Dimension and shape must still be the same though.  When the
concatenation axis is not contiguous (but still monotonically increasing
or decreasing) and {\stfaf relax=T}, a tabular coordinate will be used
to correctly describe the axis.  But be aware that it means adjacent
pixels are not regularly spaced.  However, functions like
<link anchor="images:image.toworld.function">toworld</link> and
<link anchor="images:image.topixel.function">topixel</link> will correctly interconvert world
and pixel coordinates. 

In giving the input image names, the {\stfaf infiles} argument can be a
single string if you wild card it with standard shell symbols.  For
example, {\stfaf infiles='cena\_???.*'}, where the ``?'' represents one
character and ``*'' any number of characters. 

Otherwise, you must input a vector of strings such as {\stfaf
infiles="cena1 cena2 cena3"}.  An input such as {\stfaf
infiles='files1,file2'} will be interpreted as one string naming one
file and you will get an error.  The reason for this is that although
the latter could be parsed to extract two file names by recognizing
comma delimiters, it is not possible because an expression such as
{\stfaf infiles='cena.\{a,b\}'} (meaning files of name ``cena.a'' and
``cena.b'') would confuse such parsing (you would get two files of name
{\sff cena.\{a} and {\sff b\}}. 

You can look at the Coordinate System of the output image with
<link anchor="images:image.summary.function">summary</link> to make sure it's correct.

The argument {\stfaf tempclose} is, by default, True.  This means that
all internal reference copies of the input images are  kept closed until
they are needed. Then they are opened temporarily and then closed again.
This enables you to effectively concatenate as many images as you like
without encountering any operating system open file number limits.  However, it
comes at some performance loss, because opening and closing all those
files takes time.  If you are concatenating a smallish number of files,
you might use {\stfaf tempclose=F}.  This will leave all internal
reference copies permanently open, but performance, if you don't hit the
file limit, will be better.

</description>
<example>
\begin{verbatim}
"""
#
print "\t----\t imageconcat Ex 1 \t----"
ia.fromshape('im.1',[10,10,10],overwrite=T)
ia.fromshape('im.2',[10,10,10],overwrite=T)
ia.fromshape('im.3',[10,10,10],overwrite=T)
bigim = ia.imageconcat(outfile='bigimage', infiles='im.1 im.2 im.3',
                       axis=2, relax=T, tempclose=F, overwrite=T)
bigim.done()
ia.close()
#
"""
\end{verbatim}
The \imagefiles\ {\sff im1}, {\sff im2}, and {\sff im3} are concatenated along
the third axis to form the output \imagefile\ {\sff bigimage}.
Only warnings are issued if axis descriptors mismatch.
Access to the output image is provided via the \tool\ called {\stf bigim}.
</example>

<example>
\begin{verbatim}
"""
#
print "\t----\t imageconcat Ex 2 \t----"
bigim = ia.imageconcat(infiles="im.*",relax=T)
bigim.done()
ia.close()
#
"""
\end{verbatim}
All images whose file names begin with {\sff im} and terminate with
any valid filename extension that reside in the current directory <!--
given by the translation of the environment variable {\tt \$ADATA} -->
are concatenated along the spectral axis if there is one.  The image
must be contiguous along the spectral axis and all image coordinate
descriptors must match.  If there is no spectral axis an error will
result.  Because an output file name is not given, the Image tool just
references the input files, it does not copy them.

</example>
</method>

 
 
   <method type="function" name="fromarray">
   <shortdescription>Construct a \casa\ image from an array</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="pixels">
	     <any type="variant"/>
     <description>Numeric array</description>
     </param>
  
     <param type="any" direction="in" name="csys">
     <any type="record"/>
     <description>Coordinate System.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="bool" direction="in" name="linear">
     <description>Make a linear Coordinate System if csys not given</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="overwrite">
     <description>Overwrite (unprompted) pre-existing output file?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="log">
     <description>Write image creation messages to logger</description>
     <value>true</value>
     </param>
</input>
<returns type="bool"/>

<description>

This function converts an array of any size into a \casa\
\imagefile. 

If {\stfaf outfile} is given, the image is written to the specified disk
file.  If {\stfaf outfile} is unset, the Image \tool\ is associated
with a temporary image.  This temporary image may be in memory or on
disk, depending on its size.  <!-- When you destroy the Image \tool\ 
(with the <link anchor="images:image.close.function">close</link> function) this
temporary image is deleted.-->  When you close the Image \tool\
with the close function this temporary image is deleted.

At present, no matter what type the {\stfaf pixels} array is, a
real-valued image will be written (the input pixels will be converted to
Float).  In the future, Complex images will be supported.

The Coordinate System, provided as a <link
anchor="images:coordsys">Coordsys</link> \tool  (converted to a record)
is optional.  If you provide it, it must be dimensionally consistent
with the pixels array you give (see also <link
anchor="images:image.coordsys.function">coordsys</link>).

If you don't provide the Coordinate System (unset), a default Coordinate System
is made for you.  If {\stfaf linear=F} (the default) then it is a
standard RA/DEC/Stokes/Spectral Coordinate System depending exactly upon
the shape of the {\stfaf pixels} array (Stokes axis must be no longer
than 4 pixels and you may find the spectral axis coming out before the
Stokes axis if say, {\cf shape=[64,64,32,4]}).  Extra dimensions are
given linear coordinates.  If {\stfaf linear=T} then you get a linear
Coordinate System. 

</description>
<example>
\begin{verbatim}
"""
#
print "\t----\t fromarray Ex 1 \t----"
ia.fromarray(outfile='test.data', pixels=ia.makearray(0, [64, 64, 4, 128]),
             overwrite=true)
mycs = ia.coordsys(axes=[0,1])
ia.close()
ia.fromarray(pixels=ia.makearray(1.0, [32, 64]), csys=mycs.torecord())
mycs.done()
ia.close()
#
"""
\end{verbatim}

The first example creates a zero-filled \imagefile\ named {\sff
test.data} which is of shape [64,64,4,128].  If you examine the header with
{\stff ia.summary()} you will see the default RA/DEC/Stokes/Frequency coordinate information. 
In the second example, a Coordinate System describing the first two axes
of the image {\sff test.data} is created and used to create a 2D
temporary image.

</example>
</method>

 
   <method type="function" name="fromascii">
   <shortdescription>Construct a \casa\ image by conversion from an ascii file</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="string" direction="in" name="infile">
     <description>Input ascii disk file name.  Must be specified..</description>
     <value></value>
     </param>
  
     <param type="intArray" direction="in" name="shape">
     <description>Shape of image.  Must be specified.</description>
     <value>-1</value>
     </param>
  
     <param type="string" direction="in" name="sep">
     <description>Separator in ascii file.  Default is space character.</description>
     <value> </value>
     </param>
  
     <param type="any" direction="in" name="csys">
     <any type="record"/>
     <description>Coordinate System record from coordsys torecord().
  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="bool" direction="in" name="linear">
     <description>Make a linear Coordinate System if csys not given</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="overwrite">
     <description>Overwrite (unprompted) pre-existing output file?</description>
     <value>false</value>
     </param>
</input>
<returns type="bool"/>
<description>

This function is used to convert an ascii disk file to a \casa\
\imagefile.  If {\stfaf outfile} is given, the image is written to the
specified disk file.  If {\stfaf outfile} is unset, the Image \tool\
is associated with a temporary image.  This temporary image may be in
memory or on disk, depending on its size.  When you close the Image
\tool\ (with the <link
anchor="images:image.close.function">close</link> function) this
temporary image is deleted.

You must specify the shape of the image.  The image must be stored
in the ascii file, one row at a time.  This row must be of the
correct length and there must be the correct number of rows.
For example, let us say the shape of your image is 
{\cf [nx,ny,nz] = [3,4,2]}, then
the image should be stored as 

\begin{verbatim}
1 1 1                   # row 1; y = 1, z = 1
2 2 2                   # row 2; y = 2, z = 1
3 3 3                   # row 3; y = 3, z = 1
4 4 4                   # row 4; y = 4, z = 1
1.5 1.5 1.5             # row 5; y = 1, z = 2
2.5 2.5 2.5             # row 6; y = 2, z = 2
3.5 3.5 3.5             # row 7; y = 3, z = 2
4.5 4.5 4.5             # row 8; y = 4, z = 2
\end{verbatim}

The Coordinate System, provided as a <link
anchor="images:coordsys">Coordsys</link> {\tool} converted to a record
with coordsys torecord, is optional.  If you provide it, it must be
dimensionally consistent with the pixels array you give (see also
<link anchor="images:image.coordsys.function">coordsys</link>).

If you don't provide the Coordinate System (unset), a default Coordinate System
is made for you.  If {\stfaf linear=F} (the default) then it is a
standard RA/DEC/Stokes/Spectral Coordinate System depending exactly upon
the shape of the {\stfaf pixels} array (Stokes axis must be no longer
than 4 pixels and you may find the spectral axis coming out before the
Stokes axis if say, {\cf shape=[64,64,32,4]}).  Extra dimensions are
given linear coordinates.  If {\stfaf linear=T} then you get a linear
Coordinate System. 

</description>

<example>
The ascii image file shown above can be read as
an image this way:
\begin{verbatim}
"""
#
print "\t----\t fromascii Ex 1 \t----"
a = [[[ 1. ,  1.5],
      [ 2. ,  2.5],
      [ 3. ,  3.5],
      [ 4. ,  4.5]],
     [[ 1. ,  1.5],
      [ 2. ,  2.5],
      [ 3. ,  3.5],
      [ 4. ,  4.5]],
     [[ 1. ,  1.5],
      [ 2. ,  2.5],
      [ 3. ,  3.5],
      [ 4. ,  4.5]]]
ia.fromarray('outimage.im',a,overwrite=true)
ia.toASCII('myimage.ascii',overwrite=true)     # write ascii image file
ia.close()                                     # then read ascii image file
ia.fromascii('myimage.app', 'myimage.ascii', shape=[3,4,2], overwrite=true)
ia.toASCII('myimage2.ascii',overwrite=true)
ia.close()
#!diff myimage.ascii myimage2.ascii
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="fromfits">
   <shortdescription>Construct a \casa\ image by conversion from a FITS image file</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="string" direction="in" name="infile">
     <description>Input FITS disk file name.  Must be specified.</description>
     <value></value>
     </param>
  
     <param type="int" direction="in" name="whichrep">
     <description>If this FITS file contains multiple coordinate representations, which one should we read (0-based)</description>
     <value>0</value>
     </param>
  
     <param type="int" direction="in" name="whichhdu">
     <description>If this FITS file contains multiple images, which one should we read (0-based).</description>
     <value>0</value>
     </param>
  
     <param type="bool" direction="in" name="zeroblanks">
     <description>If there are blanked pixels, set them to zero instead of NaN</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="overwrite">
     <description>Overwrite (unprompted) pre-existing output file?</description>
     <value>false</value>
       </param>
  
     <param type="bool" direction="in" name="old">
     <description>Use old non-WCS based FITS parser?</description>
     <value>false</value>
     </param>
</input>
<returns type="bool"/>
<description>

This function is used to convert a FITS disk image file (Float,
Double, Short, Long are supported) to an
\casa\ \imagefile.  If {\stfaf outfile} is given, the image is written
to the specified disk file.  If {\stfaf outfile} is unset, the Image
\tool\ is associated with a temporary image.  This temporary image may
be in memory or on disk, depending on its size.  When you close the
Image \tool\ (with the <link anchor="images:image.close.function">close</link> function) this
temporary image is deleted. 

This function reads from the FITS primary array (when the image is at
the beginning of the FITS file; {\stfaf whichhdu=0}), or an image
extension (when the image is elsewhere in the FITS file, {\stfaf
whichhdu $>$ 0}). 

By default, any blanked pixels will be converted to a mask value which
is false, and a pixel value that is NaN.  If you set {\stfaf
zeroblanks=T} then the pixel value will be zero rather than NaN.  The
mask will still be set to false.  See the function
<link anchor="images:image.replacemaskedpixels.function">replacemaskedpixels</link> if you
need to replace masked pixel values after you have created the image. 

</description>
<example>
\begin{verbatim}
"""
#
print "\t----\t fromfits Ex 1 \t----"
datapath=pathname+'/data/demo/Images/imagetestimage.fits'
ia.fromfits('./myimage', datapath, overwrite=true)
print ia.summary()
s = ia.miscinfo()
print s.keys()  #  prints any unrecognized field names
ia.close()
#
"""
\end{verbatim}

The FITS image is converted to a \casa\ \imagefile\ and access is
provided via the default \imagetool\ called {\stf ia}.  Any FITS
header keywords which were not recognized or used are put in the
miscellaneous information bucket accessible with the <link
anchor="images:image.miscinfo.function">miscinfo</link> function.  In
the example we list the names of the fields in this record.

</example>
</method>

 
 
   <method type="function" name="fromimage">
   <shortdescription>Construct a (sub)image from a region of a \casa\ image</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="outfile">
     <description>Output sub-image file name.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="string" direction="in" name="infile">
     <description>Input image file name.  Must be specified.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="region">
     <any type="record"/>
     <description>The region of interest.  Default is whole image.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="mask">
     <any type="variant"/>    
     <description>OTF mask, Boolean LEL expression or mask region.  Default is none.</description>
     <value></value>
    </param>
  
     <param type="bool" direction="in" name="dropdeg">
     <description>Drop degenerate axes</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="overwrite">
     <description>Overwrite (unprompted) pre-existing output file?</description>
     <value>false</value>
     </param>
</input>
<returns type="bool"/>
<description>

This function applies a \region\ to an \imagefile, creates a new
\imagefile\ containing the (sub)image, and associates the \imagetool\
with it. 

The input image file may be in native \casa, \fits, or Miriad
format.  Look \htmlref{here}{IMAGES:FOREIGNIMAGES}  for more
information on foreign images.

If {\stfaf outfile} is given, the (sub)image is written to the specified
disk file.  

If {\stfaf outfile} is unset, the Image \tool\ actually references
the input image file.  So if you deleted the input image disk file, it
would render this \tool\ useless.  When you close this \tool\
(with the <link anchor="images:image.close.function">close</link> function)
the reference connection is broken.  

Sometimes it is useful to drop axes of length one (degenerate axes).
Use the {\stfaf dropdeg} argument if you want to do this.

The output mask is the combination (logical OR) of the default input
\pixelmask\ (if any) and the OTF mask.  Any other input \pixelmasks\
will not be copied.  Use function
<link anchor="images:image.maskhandler.function">maskhandler</link> if you need to copy other
masks too. 

See also the <link anchor="images:image.subimage.function">subimage</link> function.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t fromimage Ex 1 \t----"
# Unfortunately, rg.quarter() wasn't implemented when this example was written.
#ia.fromimage(outfile='image.small', infile='image.large', region=rg.quarter())
ia.open(infile='test.data')  # needs to be open for ia.setboxregion()
innerquarter=ia.setboxregion([0.25,0.25],[0.75,0.75],frac=true)
ia.close()
ia.fromimage(outfile='image.small', infile='test.data', region=innerquarter, overwrite=true)
ia.close()
#
"""
\end{verbatim}
The specified \region\ takes a quarter by area of the first two axes
of the image, and all pixels of other axes.
</example>
</method>

 
<method type="function" name="fromshape">
   <shortdescription>Construct an empty \casa\ image from a shape</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="outfile">
     <description>Name of output image file.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="intArray" direction="in" name="shape">
     <description>Shape of image.  Must be specified.</description>
     <value>0</value>
     </param>
  
     <param type="any" direction="in" name="csys">
	     <any type="record"/>
     <description>Coordinate System.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="bool" direction="in" name="linear">
     <description>Make a linear Coordinate System if csys not given?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="overwrite">
     <description>Overwrite (unprompted) pre-existing output file?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="log">
     <description>Write image creation messages to logger</description>
     <value>true</value>
     </param>
</input>
<returns type="bool">Image tool</returns>
<description>

This function creates a \casa\ \imagefile\ with the specified shape.  All
the pixel values in the image are set to 0.  

If {\stfaf outfile} is given, the image is written to the specified disk
file.  If {\stfaf outfile} is unset, the Image \tool\ is associated with
a temporary image.  This temporary image may be in memory or on disk,
depending on its size.  When you close the Image \tool\  (with the
<link anchor="images:image.close.function">close</link> function) this temporary image is
deleted.

The Coordinate System, provided as a <link anchor="images:coordsys">Coordsys</link>
\tool, is optional.  If you provide it, it must be dimensionally
consistent with the shape that you specify (see also
<link anchor="images:image.coordsys.function">coordsys</link>).

If you don't provide the Coordinate System, a default Coordinate System
is made for you.  If {\stfaf linear=F} (the default) then it is a
standard RA/DEC/Stokes/Spectral Coordinate System depending exactly upon
the shape (Stokes axis must be no longer than 4 pixels and you may find
the spectral axis coming out before the Stokes axis if say, {\cf
shape=[64,64,32,4]}).  Extra dimensions are given linear coordinates. 
If {\stfaf linear=T} then you get a linear Coordinate System. 

</description>
<example>
\begin{verbatim}
"""
#
print "\t----\t fromshape Ex 1 \t----"
ia.fromshape('test2.data', [64,64,128], overwrite=true)
mycs = ia.coordsys(axes=[0,2])
ia.close()
ia.fromshape(shape=[10, 20], csys=mycs.torecord())
mycs.done()
ia.close()
#
"""
\end{verbatim}

The first example creates a zero-filled \imagefile\ named {\sff
test.data} of shape [64,64,128].  If you examine the header with {\stff
ia.summary()} you will see the RA/DEC/Spectral coordinate information. 
In the second example, a Coordinate System describing the first and third axes
of the image {\sff test.data} is created and used to create a 2D temporary image.
</example>
</method>



   <method type="function" name="maketestimage">
   <shortdescription>Construct a \casa\  image from a test FITS file
   </shortdescription>

<input>
  
     <param type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>

     <param type="bool" direction="in" name="overwrite">
     <description>Overwrite (unprompted) pre-existing output file?
      </description>
     <value>false</value>
     </param>

</input>
<returns type="bool"/>
<description>
This function converts a FITS file resident in the \casa\  system into
a \casa\  image.

If outfile is given, the image is written to the specified disk
file. If outfile is unset, the Image tool is associated with a
temporary image. This temporary image may be in memory or on disk,
depending on its size. When you close the Image tool (with the close
function) this temporary image is deleted.
</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t maketestimage Ex 1 \t----"
ia.maketestimage()     # make virtual image
ia.close()
ia.maketestimage('tmp', overwrite=true)
ia.close()             # close to unlock disk image
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="adddegaxes">
   <shortdescription>Add degenerate axes of the specified type to the image</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="bool" direction="in" name="direction">
     <description>Add direction axes?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="spectral">
     <description>Add spectral axis?</description>
     <value>false</value>
     </param>
  
     <param type="string" direction="in" name="stokes">
     <description>Add Stokes axis?  Default is empty string.</description>
     <value></value>
     </param>
  
     <param type="bool" direction="in" name="linear">
     <description>Add linear axis?</description>
     <value>false</value>
     </param>  
  
     <param type="bool" direction="in" name="tabular">
     <description>Add tabular axis?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="overwrite">
     <description>Overwrite (unprompted) pre-existing output file?</description>
     <value>false</value>
     </param>
</input>
<returns type="casaimage"/>

<description>

This function <!-- (short-hand name {\stff ada}) --> adds degenerate axes (i.e.
axes of length 1) of the specified type.  Sometimes this can be useful
although you will generally need to modify the coordinate system of the
added axis to give it the coordinate you want (do this with the
<link anchor="images:coordsys">Coordsys</link> \tool).

You specify which type of axes you want to add.  You can't add
an axis type that already exists in the image.  For the Stokes axis,
the allowed value (a string such as  I, Q, XX, RR) can be found in the
<link anchor="images:coordsys.newcoordsys.function">Coordsys newcoordsys</link> function documentation.

If {\stfaf outfile} is given, the image is written to the specified
disk file.  If {\stfaf outfile} is unset, the on-the-fly Image \tool\
returned by the function is associated with a temporary image.  This
temporary image may be in memory or on disk, depending on its size.
When you destroy the generated Image \tool\ (with the <link
anchor="images:image.done.function">done</link> function) this
temporary image is deleted.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t adddegaxes Ex 1 \t----"
ia.maketestimage() 
print ia.shape()
#[113L, 76L]
mycs=ia.coordsys()
print mycs.axiscoordinatetypes()
#['Direction', 'Direction']
mycs.done()
im2 = ia.adddegaxes(spectral=T)
print im2.shape()
#[113L, 76L, 1L]
mycs=im2.coordsys()
print mycs.axiscoordinatetypes()
['Direction', 'Direction', 'Spectral']
mycs.done()
im3 = im2.adddegaxes(stokes='Q')
print im3.shape()
#[113L, 76L, 1L, 1L]
mycs = im3.coordsys()
print mycs.axiscoordinatetypes()
#['Direction', 'Direction', 'Spectral', 'Stokes']
mycs.done()
im2.done()
im3.done()
ia.close()
#
"""
\end{verbatim}
In this example, all the images are virtual (temporary images).
</example>

</method>


   <method type="function" name="addnoise">
   <shortdescription>Add noise to the image</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="type">
     <description>Type of distribution, normal</description>
     <value>normal</value>
     </param>
  
     <param type="doubleArray" direction="in" name="pars">
     <description>Parameters of distribution</description>
     <value type="vector"><value>0.0</value> <value>1.0</value></value>
     </param>
  
     <param type="any" direction="in" name="region">
     <any type="record"/>
     <description>Region of interest.  Default is whole image.</description>
     <value></value>
     </param>
  
     <param type="bool" direction="in" name="zero">
     <description>Zero image first?</description>
     <value>false</value>
     </param>
</input>
<returns type="bool">T or fail</returns>

<description>

This function adds noise to the image.  You may zero the image first
before the noise is added if you wish.

The noise can be drawn from one of many distributions. <!--, which are also
available in the <link anchor="mathematics.randomnumbers">randomnumbers</link> \tool. -->

For each distribution, you must supply the type via the {\stfaf type}
argument (minimum match is active) and parameters via the {\stfaf
pars} argument.  <!-- Each distribution type is described in detail in
the <link anchor="mathematics.randomnumbers">randomnumbers</link>
\tool.--> Briefly:

\begin{itemize}

\item {binomial} -- the binomial distribution models successfully drawing
items from a pool. Specify two parameters, $n$ and $p$, respectively.
$n$ is the number of items in the pool, and $p$, is the probability of
each item being successfully drawn. It is required that $n > 0$ and 
$0 \le p \le 1$.

\item {discreteuniform} -- models a uniform random variable over the closed interval. Specify 
two parameters, the low and high values, respectively.
The low parameter is the lowest possible return value and
the high parameter is the highest. It is required that $low < high$.

\item {erlang} -- Specify two parameters, the  mean and variance,
respectively. It is required that the mean is non-zero and the variance
is positive.

\item {geometric} -- Specify one parameter, the probability.
It is required that $0 \le probability < 1$.

\item {hypergeometric} -- Specify two parameters, the mean and the variance.
It is required that the variance is positive and that the mean is non-zero
and not bigger than the square-root of the variance.

\item {normal} -- Specify two parameters, the mean and the variance.
It is required that the variance is positive.   

\item {lognormal} -- Specify two parameters, the mean and the variance.   
It is required that the supplied variance is positive and that the mean is non-zero.

\item {negativeexponential} -- Supply one parameter, the mean.

\item {poisson} -- Specify one parameter, the mean.
It is required that the mean is non-negative.

\item {uniform} -- Model a uniform random variable over a closed
interval. Specify two parameters, the low and high values.    The low
parameter is the lowest possible return value and the high parameter can
never be returned. It is required that $low < high$.

\item {weibull} -- Specify two parameters, alpha and beta.
It is required that the alpha parameter is not zero.

\end{itemize}

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t addnoise Ex 1 \t----"
ia.maketestimage()          # im1 = maketestimage() 
ia.addnoise(type='normal', pars=[0.5, 1], zero=T)
ia.statistics()
ia.close()
#
"""
\end{verbatim}

A test image is created, zeroed, and noise of mean 0.5 and variance 1
from a normal distribution added.

</example>

</method>

 
   <method type="function" name="convolve">
   <shortdescription>Convolve image with an array or another image</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="kernel">
     <any type="variant"/>
     <description>Convolution kernel - array or image filename.  Must be specified.</description>
     <value></value>
     </param>
  
     <param type="double" direction="in" name="scale">
     <description>Scale factor.  Default is autoscale.</description>
     <value>-1.0</value>
     </param>
  
     <param type="any" direction="in" name="region">
     <any type="record"/>
     <description>Region of interest.  Default is whole image.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="mask">
     <any type="variant"/>
     <description>OTF mask, Boolean LEL expression or mask region.  Default is none.</description>
     <value></value>
     </param>
  
     <param type="bool" direction="in" name="overwrite">
     <description>Overwrite (unprompted) pre-existing output file?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="async">
     <description>Run asynchronously?</description>
     <value>false</value>
     </param>
</input>
<returns type="casaimage"/>

<description>

This function does Fourier-based convolution of an \imagefile\ by the
given kernel.  

If {\stfaf outfile} is given, the image is written to the specified
disk file.  If {\stfaf outfile} is unset, the on-the-fly Image \tool\
generated by this function is associated with a temporary image.  This
temporary image may be in memory or on disk, depending on its size.
When you destroy the generated Image \tool\ (with the <link
anchor="images:image.done.function">done</link> function) this
temporary image is deleted.

The kernel is provided as a multi-dimensional array or as the 
filename of a disk-based \imagefile.   The provided kernel can have fewer
dimensions than the image being convolved.  In this case, it will be
padded with degenerate axes.  An error will result if the kernel has
more dimensions than the image.   No additional scaling of the kernel is
provided yet.

The scaling of the output image is determined by the argument {\stfaf scale}.
If you leave it unset, then the kernel is normalized to unit sum.
If you do not leave {\stfaf scale} unset, then the convolution kernel
will be scaled (multipled) by this value.

Masked pixels will be assigned the value 0.0 before convolution. 

The output mask is the combination (logical OR) of the default input 
\pixelmask\ (if any) and the OTF mask.  Any other input \pixelmasks\
will not be copied.  Use function
<link anchor="images:image.maskhandler.function">maskhandler</link> if you need to copy other
masks too.

See also the other convolution functions
<link anchor="images:image.convolve2d.function">convolve2d</link>,
<link anchor="images:image.sepconvolve.function">sepconvolve</link> and
<link anchor="images:image.hanning.function">hanning</link>. 

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t convole Ex 1 \t----"
ia.open(infile='test.data')
# r1=rg.quarter()
# kernel = mykernel(...)                     # Your code to generate the kernel
# im2=ia.convolve(kernel=kernel, region=r1)  # Makes temporary image
r1=ia.setboxregion([0.45,0.45],[0.55,0.55],frac=true)
print "Convolving... [This takes awhile]"
im3 = ia.convolve (kernel=ia.name(F), region=r1) # Convolve by self
ia.close()
im3.done()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="boundingbox">
   <shortdescription>Get the bounding box of the specified region</shortdescription>
   
<input>
  
     <param type="any" direction="in" name="region">
     <any type="record"/>
     <description>The region of interest.  Default is whole image.</description>
     <value></value>
     </param>
</input>
<returns type="any"><any type="record"/>Record or fail</returns>
<description>

This function <!-- (short-hand name {\stff bb}) --> finds the bounding box of a
region of interest when it is applied to a particular image.  It is
returned in a record which has fields {\cf `blc', `trc', `inc',
`bbShape', `regionShape', `imageShape', `blcf'} and {\cf `trcf'}
containing the bottom-left corner, the top-right corner (in absolute
image pixel coordinates), the increment (stride) of the region, the
shape of the boundingbox, the shape of the region, the shape of the
image,  the blc in formatted absolute world coordinates and the trc in
formatted absolute world  coordinates, respectively. 

Note that the shape of the bounding box will be different from the shape
of the region if a non-unit stride (increment) is involved (see the example
below).

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t boundingbox Ex 1 \t----"
ia.maketestimage()                           # Create image tool
x=['3pix','6pix','9pix','6pix','5pix','5pix','3pix'] # X vector in abs pixels
y=['3pix','4pix','7pix','9pix','7pix','5pix','5pix'] # Y vector in abs pixels
mycs = ia.coordsys()
r1=rg.wpolygon(x=x,y=y,csys=mycs.torecord()) # Create polygonal world region
mycs.done()
bb = ia.boundingbox(r1)                      # Find bounding box
print bb
#{'regionShape': array([7, 7]), 'trc': array([9, 9]),
# 'imageShape': array([113, 76]),
# 'blcf': '00:00:27.733, -00.06.48.000',
# 'trcf': '00:00:24.533, -00.05.36.000', 'bbShape': array([7, 7]),
# 'blc': array([3, 3]), 'inc': array([1, 1])}
ia.close()
#
"""
\end{verbatim}
</example>

<example>
\begin{verbatim}
"""
#
print "\t----\t boundingbox Ex 2 \t----"
ia.maketestimage()
b = rg.box([10,10],[20,20],[2,3])
print ia.boundingbox(b)
#{'regionShape': array([6, 4]), 'trc': array([20, 19]),
# 'imageShape': array([113,  76]),
# 'blcf': '00:00:24.000, -00.05.24.000',
# 'trcf': '00:00:18.667, -00.03.36.000', 'bbShape': array([11, 10]),
# 'blc': array([10, 10]), 'inc': array([2, 3])}

ia.close()
#
"""
\end{verbatim}
In this example we see the difference between bbShape and regionShape because
of the increment (stride).  See also that the trc is modified by
the increment.
</example>
</method>

 
   <method type="function" name="brightnessunit">
   <shortdescription>Get the image brightness unit</shortdescription>
   
<returns type="string"/>
<description>

This function <!-- (short-hand name {\stff bu}) --> gets the image brightness unit.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t brightnessunit Ex 1 \t----"
ia.maketestimage()
print ia.brightnessunit()
#Jy/beam
ia.close()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="calc">
   <shortdescription>Image calculator</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="pixels">
     <description>LEL expression</description>
     </param>
</input>
<returns type="bool"/>

<description> 

This function is used to evaluate a mathematical expression involving
\casa\ images, assigning the result to the current (already existing) 
image.  It complements the <link anchor="images:image.imagecalc.constructor">imagecalc</link>
function which returns a newly constructed on-the-fly image tool.  See \htmladdnormallink{note 223}{../../notes/223/223.html}
which describes the the syntax and functionality in detail.

If the expression, supplied via the {\stfaf pixels} argument, is not a
scalar, the shapes and coordinates of the image and expression must
conform. 

If the image (that associated with the tool) has a \pixelmask, then only
pixels for which that mask is good will be changed.  See the function
<link anchor="images:image.maskhandler.function">maskhandler</link> for managing image \pixelmasks. 

See the related functions <link anchor="images:image.set.function">set</link> and
<link anchor="images:image.putregion.function">putregion</link>.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t calc Ex 1 \t----"
ia.maketestimage('aF', overwrite=true)
ia.calc('min(aF, (min(aF)+max(aF))/2)')
ia.calc('1.0')
ia.close()
#
"""
\end{verbatim}

The first example shows that there are 2 {\cf min} functions.  One with a
single argument returning the minimum value of that image.  The other
with 2 arguments returning an image containing ``aF'' data clipped at
the value of the 2nd argument.   The second example sets all good
pixels to unity.

\begin{verbatim}
"""
#
print "\t----\t calc Ex 2 \t----"
ia.maketestimage('aD', overwrite=true)       # create some
ia.close()
ia.maketestimage('aF', overwrite=true)       # image files
ia.close()
ia.maketestimage('bF', overwrite=true)       # for use
ia.close()
ia.maketestimage('aC', overwrite=true)       # in
ia.close()
ia.maketestimage()
ia.calc('sin(aD)+(aF*2)+min(bF)+real(aC)')   # the example
ia.close()
#
"""
\end{verbatim}
This shows a mixed type expression.  The real part of the complex image
``aC''  is used in an expression that otherwise uses Float type.
</example>
</method>

   <method type="function" name="calcmask">
   <shortdescription>Image mask calculator</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="mask">
     <description>Boolean LEL expression or mask region.  Must be specified.</description>
     <value></value>
     </param>
  
     <param type="string" direction="in" name="name">
     <description>Mask name.  Default is auto new name.</description>
     <value></value>
     </param>
  
     <param type="bool" direction="in" name="asdefault">
     <description>Make specified mask the default mask?</description>
     <value>true</value>
     </param>
</input>
<returns type="bool"/>

<description> 

This function is used to create a new \pixelmask\ via a Boolean LEL expression.
This gives you much more scope than the simple
<link anchor="images:image.set.function">set</link> and
<link anchor="images:image.putregion.function">putregion</link> functions. 

See \htmladdnormallink{note 223}{../../notes/223/223.html}
which describes the the syntax and functionality of LEL in detail. 

If the expression is not a scalar, the shapes and coordinates of the
image and expression must conform.    If the expression is a scalar
then the entire \pixelmask\ will be set to that value.

By default (argument {\stfaf name}) the name of a new \pixelmask\ is made up
for you.  However, if you specify a \pixelmask\ name (use function
<link anchor="images:image.summary.function">summary</link> or
<link anchor="images:image.maskhandler.function">maskhandler</link> to see the mask names)
then it is used.  If the \pixelmask\ already exists, it is overwritten. 

You can specify whether the new \pixelmask\ should be the default mask or not.
By default, it is made the default \pixelmask\ !

</description>

<example>
\begin{verbatim} 
"""
#
print "\t----\t calcmask Ex 1 \t----"
ia.maketestimage('zz', overwrite=true)
subim = ia.subimage()                # Make "another" image
ia.calcmask('T')                     # Specify 'True' mask as a string
ia.calcmask('zz>0')                  # Mask of zz ignored
ia.calcmask('mask(zz) && zz>0')      # Mask of zz included
ia.calcmask(subim.name(true)+'>min('+subim.name(true)+')') # Use tool names
ia.calcmask('zz>min(zz:nomask)')  # Mask of zz not used in scalar function
subim.done()
ia.close()
#
"""
\end{verbatim}

The first calcmask example is the equivalent of {\cf
ia.set(pixelmask=1)}.  It sets the entire mask to True.

The second example creates a new \pixelmask\ which is True when
the pixel values in image {\sff zz} are greater than 0.  

Now for some subtlety.  Read carefully !  Any LEL expression can be
thought of as having a value and a mask.  Usually the value is Float and
the mask Boolean.  In this case, because the expression is Boolean
itself, the value is also Boolean.  The expression mask would just be
the mask of {\sff zz}.  Now what {\stfaf calcmask} does is create a mask
from the expression value (which is Boolean) and discards the expression
mask.  Therefore, the resulting mask is independent of any mask
that {\sff zz} might have.

If you wish the mask of the expression be honoured as well,
then you can do as in the third example.   It says the output \pixelmask\ 
will be True if the current \pixelmask\ of {\sff zz} is True and the expression
value is True.

The fourth example is like the second, except that we use the pixel
values associated with the on-the-fly {\stf subim} Image tool <!--
itself rather than using its disk name (the Image tool might be
virtual so there might not be a --> disk file.  Note one further
subtlety here.  When the scalar function {\cf min} evaluates a value
from {\cf subim.name()}, which in this case is just {\cf zz}, the default
mask of {\cf subim.name()} {\it will} be used.  All the scalar
functions look at the mask.  If you didn't want the mask to be used
you can use the special {\cf :nomask} syntax shown in the final
example.

</example>
</method>

 
   <method type="function" name="close">
   <shortdescription>Close the image tool</shortdescription>
   
<returns type="bool"/>
<description>

This function closes the \imagetool.  This means that it detaches the
tool from its \imagefile\ (flushing all the changes first).  The
\imagetool\ is ``null'' after this change (it is not destroyed) and
calling any \toolfunction\ other than <link anchor="images:image.open.function">open</link>
will result in an error. 

</description>
<example>
\begin{verbatim}
"""
#
print "\t----\t close Ex 1 \t----"
# First create image and attach image tool to it.
ia.maketestimage('myimage',overwrite=true)
ia.close()              # Detaches image from Image tool
print "!!!EXPECT ERROR HERE!!!"
ia.summary()            # No image so this results in an error.
ia.open('myimage')      # Image is reattached
ia.summary()            # No error
ia.close()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="continuumsub">
   <shortdescription>Image plane continuum subtraction</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="outline">
     <description>Output line image filename.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="string" direction="in" name="outcont">
     <description>Output continuum image filename</description>
     <value>continuumsub.im</value>
     </param>
  
     <param type="any" direction="in" name="region">
     <any type="record"/>
     <description>Region over which continuum subtraction is desired.  Deafault is whole image.</description>
     <value></value>
     </param>
  
     <param type="intArray" direction="in" name="channels">
     <description>Channels to use for continuum estimation.  Default is all.</description>
     <value>-1</value>
     </param>
  
     <param type="string" direction="in" name="pol">
     <description>Polarization to process.  String from
     "I Q U V XX RR" etc.  Default is unset.</description>
     <value>I</value>
     <value>Q</value>
     <value>U</value>
     <value>V</value>
     <value>XX</value>
     <value>RR</value>
     <value></value>
     </param>
  
     <param type="int" direction="in" name="fitorder">
     <description>Polynomial order for continuum estimation</description>
     <value>0</value>
     </param>
  
     <param type="bool" direction="in" name="overwrite">
     <description>Auto-overwrite output files if they exist?</description>
     <value>false</value>
     </param>
</input>
<returns type="casaimage"/>

<description> 

This function packages the relevant image tool functionality for simple
specification and application of image plane continuum subtraction.  All
that is required of the input image is that it have a non-degenerate
Spectral axis.  

The user specifies region, the region of the input image over which
continuum subtraction is desired (otherwise the whole image will be
treated); channels, the subset of channels on the spectral axis to use
in the continuum estimation, specified as a vector; pol, the desired
single polarization to treat (otherwise all pols, if a polarization axis
exists, will be treated); fitorder, the polynomial order to use in the
estimation.  Optionally, output line and continuum images may be written
by specifying outline and outcont, respectively.  If outline is not
specified, a virtual image tool is all that is produced.  If outcont is
not specified, the output continuum image will be written in
'continuumsub.im'. 

<!-- In principle, the region may be specified interactively, e.g., using
the viewer. At the moment, such regions are typically degenerate on
the spectral axis, and so must be manually extended along the this
axis, or continuumsub will complain about insufficient spectral
channels in the region of interest.  Interface improvements in the
management of regions currently under consideration will improve the
mechanism for doing this. -->

</description>

<example>
\begin{verbatim} 
"""
#
print "\t----\t continuumsub Ex 1 \t----"
ia.fromarray(outfile='test.data',
             pixels=ia.makearray(0, [64, 64, 4, 128]), overwrite=true)
#im1csub=ia.continuumsub(region=rg.quarter(),
#                        channels=range(3,9)+range(54,61),fitorder=0);
innerquarter=ia.setboxregion([0.25,0.25],[0.75,0.75],frac=true)
im1csub=ia.continuumsub(region=innerquarter,
                        channels=range(3,9)+range(54,61),fitorder=0);
im1csub.done()      # done the on-the-fly image tool
ia.close()
#
"""
\end{verbatim}
In this first example, the continuum emission in each pixel of the
inner quarter of the input image is estimated by averaging
(fitorder=0) channels 3-8 and 54-60, inclusive.  (Python's range
function includes the lower limit and excludes the upper limit.)
im1csub is an image tool containing the result (only the inner quarter
of the original image).

\begin{verbatim} 
"""
#
print "\t----\t continuumsub Ex 2 \t----"
ia.fromarray(outfile='test.data',
             pixels=ia.makearray(0, [64, 64, 4, 128]), overwrite=true)
im2csub=ia.continuumsub(channels=range(3,9)+range(54,61),pol='I',fitorder=2)
im2csub.done()
ia.close()
#
"""
\end{verbatim}
In this second example, the Stokes I continuum emission in each pixel 
of the whole input image is estimated with a 2nd-order polynomial
fit to channels 3-8 and 54-60.  The output image tool (im2csub) contains
only Stokes I, even if the original image had other stokes planes.

</example>
</method>

 
   <method type="function" name="convertflux">
   <shortdescription>Convert flux density between peak and integral</shortdescription>
   
<input>
  
     <param type="any" direction="in" name="value">
     <any type="variant"/>
     <description>Flux density to convert.  Must be specified.</description>
     <value>0Jy/beam</value>
     </param>
  
     <param type="any" direction="in" name="major">
     <any type="variant"/>
     <description>Major axis of component.  Must be specified.</description>
     <value>1arcsec</value>
     </param>
  
     <param type="any" direction="in" name="minor">
     <any type="variant"/>
     <description>Minor axis of component.  Must be specified.</description>
     <value>1arcsec</value>
     </param>
  
     <param type="string" direction="in" name="type">
     <description>Type of component.  String from Gaussian, Disk.</description>
     <value>Gaussian</value>
     </param>
  
     <param type="bool" direction="in" name="topeak">
     <description>Convert to peak or integral flux desnity</description>
     <value>true</value>
     </param>
</input>
<returns type="any"><any type="record"/></returns>

<description> 

This function interconverts between peak and integral flux density for a
Gaussian or Disk component.  The image must hold a restoring beam. 

</description>

<example>
\begin{verbatim} 
"""
#
print "\t----\t convertflux Ex 1 \t----"
ia.maketestimage('in.im', overwrite=true);
p1 = qa.quantity('1mJy/beam')
i1 = ia.convertflux(p1, major='30arcsec', minor='10arcsec', topeak=F);
p2 = ia.convertflux(i1, major='30arcsec', minor='10arcsec', topeak=T)
print 'peak, integral, peak = ', p1, i1, p2
#peak, integral, peak =  {'value': 1.0, 'unit': 'mJy/beam'}
#                        {'value': 0.00016396129551656742, 'unit': 'Jy'}
#                        {'value': 0.0010000000000000002, 'unit': 'Jy/beam'}

ia.close()
#
"""
\end{verbatim}

</example>
</method>

 
 
   <method type="function" name="convolve2d">
   <shortdescription>Convolve image by a 2D kernel</shortdescription>
   
 <input>
  
     <param type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="intArray" direction="in" name="axes">
     <description>Axes to convolve.  Default is [0,1].</description>
     <value type="vector"><value>0</value><value>1</value></value>
     </param>
  
     <param type="string" direction="in" name="type">
     <description>Type of convolution kernel</description>
     <value>gaussian</value>
     </param>
  
     <param type="any" direction="in" name="major">
     <any type="variant"/>
     <description>Major axis, Quantity, string, numeric.  Must be specified.</description>
     <value>0deg</value>
     </param>
  
     <param type="any" direction="in" name="minor">
     <any type="variant"/>
     <description>Minor axis, Quantity, string, numeric.  Must be specified.</description>
     <value>0deg</value>
     </param>
  
     <param type="any" direction="in" name="pa">
     <any type="variant"/>
     <description>Position Angle, Quantity, string, numeric.  Default is 0deg.</description>
     <value>0deg</value>
     </param>
  
     <param type="double" direction="in" name="scale">
     <description>Scale factor.  Default is autoscale.</description>
     <value>-1</value>
     </param>
  
     <param type="any" direction="in" name="region">
     <any type="record"/>
     <description>Region of interest.  Default is whole image.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="mask">
     <any type="variant"/>
     <description>OTF mask, Boolean LEL expression or mask region.  Default is none.</description>
     <value></value>
     </param>
  
     <param type="bool" direction="in" name="overwrite">
     <description>Overwrite (unprompted) pre-existing output file?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="async">
     <description>Run asynchronously?</description>
     <value>false</value>
     </param>
</input>
<returns type="casaimage"/>

<description>

This function <!-- (short-hand name {\stff c2d}) --> does Fourier-based
convolution of an \imagefile\ by the given 2D kernel.  

If {\stfaf outfile} is unset, the image is written to the specified
disk file.  If {\stfaf outfile} is not given, the newly constructed
on-the-fly Image \tool\ is associated with a temporary image.  This
temporary image may be in memory or on disk, depending on its size.
When you destroy the on-the-fly Image \tool\ (with the <link
anchor="images:image.done.function">done</link> function) this
temporary image is deleted.

You specify which 2 pixel axes of the image you wish to convolve via the
{\stfaf axes} argument.

You specify the type of convolution kernel with {\stfaf type} (minimum
match is active); currently only {\cf 'gaussian'}  is available.  With
time others will be implemented.

You specify the parameters of the convolution kernel via the arguments
{\stfaf major}, {\stfaf minor}, and {\stfaf pa}.   These arguments can
be specified in one of three ways

\begin{itemize}

\item Quantity - for example {\stfaf major=qa.quantity(1, 'arcsec')}
Note that you can use pixel units, viz. {\stfaf major=qa.quantity(1, 'pix')},
see below.

\item String - for example {\stfaf minor='1km'} (i.e. one that the
Quanta <link anchor="quanta:quanta">quantity function</link> accepts).
  
\item Numeric - for example {\stfaf major=10}.  In this case, the units
of {\stfaf major} and {\stfaf minor} are assumed to be in pixels.  Using
pixel units allows you to convolve unlike axes. For the position angle,
units of degrees are assumed.
           
\end{itemize}

The interpretation of {\stfaf major} and {\stfaf minor} depends upon the
kernel type.

\begin{itemize}

\item Gaussian - {\stfaf major} and {\stfaf minor} are
the Full Width at Half Maxiumum (FWHM) of the major and minor
axes of the Gaussian.

\end{itemize}

The position angle is measured North through East when you convolve a
plane holding a celestial coordinate (the usual astronomical
convention).  For other  axis/coordinate combinations, a positive
position angle is measured  from +x to +y in the
absolute pixel coordinate frame  (x is the first axis that you
specify with argument {\stfaf axes}). 

The scaling of the output image is determined by the argument {\stfaf scale}.
If you leave it unset, then autoscaling will be invoked.  

If you are not convolving the sky, then autoscaling means that the convolution
kernel will be normalized to have  unit volume so as to conserve flux.

If you are convolving the sky, then there are two cases 
for which autoscaling is useful.

Firstly, if the input image units are Jy/pixel, then the output image
will have units of Jy/beam and be appropriately scaled.  In addition,
the restoring beam of the output image will be the same as the
convolution kernel.

Secondly,if the input image units are Jy/beam, then
the output image will also have units of Jy/beam and be appropriately
scaled.  In addition, the restoring beam of the output image
will be the convolution of the input image restoring beam and the
convolution kernel.

If you do not leave {\stfaf scale} unset, then the convolution kernel
will be scaled by this value (it has peak unity before application
of this scale factor).

Masked pixels will be assigned the value 0.0 before convolution. 
The output mask is the combination (logical OR) of the default input 
\pixelmask\ (if any) and the OTF mask.  Any other input \pixelmasks\
will not be copied.  Use function
<link anchor="images:image.maskhandler.function">maskhandler</link> if you need to copy other
masks too.

See also the other convolution functions
<link anchor="images:image.convolve.function">convolve</link>,
<link anchor="images:image.hanning.function">hanning</link>, and
<link anchor="images:image.sepconvolve.function">sepconvolve</link>.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t convolve2d Ex 1 \t----"
ia.maketestimage('xy',overwrite=true)         # RA/DEC
im2 = ia.convolve2d(outfile='xy.con', axes=[0,1], type='gauss',
                    major='20arcsec', minor='10arcsec', pa='45deg',
                    overwrite=true);
im2.done()
ia.close()
#
ia.fromarray(outfile='xypf', pixels=ia.makearray(0, [64, 64, 4, 64]),
             overwrite=true)         # RA/DEC/Pol/Freq
print "!!!EXPECT WARNING REGARDING INVALID SPATIAL RESTORING BEAM!!!"
im2 = ia.convolve2d(outfile='xypf.con', axes=[0,3], type='gauss',
                    major='20pix', minor='10pix', pa='45deg',
                    overwrite=true);
im2.done()
ia.close()
#
"""
\end{verbatim}
In the second example we must use pixel units because axes 0 and 3 (0-rel)
are unlike.
</example>
</method>

 
   <method type="function" name="coordsys">
   <shortdescription>Get the Coordinate System of the image</shortdescription>
   
<input>
  
     <param type="intArray" direction="in" name="axes">
     <description>Axes to which the Coordinate System pertains.  Default is all axes.</description>
     <value>-1</value>
     </param>
</input>

<returns type="casacoordsys"/>
<description>

This function stores the Coordinate System of an image in a {\stf
Coordsys} tool.  

By default, the Coordinate System describes all of the axes in the
image.  If you desire, you can select a subset of the axes, thus
reducing the dimensionality of the Coordinate System.   This may be
useful if you are supplying a Coordinate System to the
functions <link anchor="images:image.fromarray.constructor">fromarray</link> or
<link anchor="images:image.fromshape.constructor">fromshape</link>.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t coordsys Ex 1 \t----"
ia.maketestimage('hcn',overwrite=true)
ia.summary()
mycs = ia.coordsys([0,1])
imshape = ia.shape()
ia.fromshape(outfile='test', shape=imshape, csys=mycs.torecord(), overwrite=true)
ia.summary()
mycs.done()
ia.close()
#
"""
\end{verbatim}

In this example, we create a Coordinate System pertaining to the first
two axes of the image and then we create a new (empty) 2D image with
this Coordinate System using the {\cf fromshape} function.
</example>
</method>

 <!--
   <method type="function" name="csys">
   <shortdescription>Get the Coordinate System of the image</shortdescription>
   
<input>
  
     <param type="intArray" direction="in" name="axes">
     <description>Axes to which the Coordinate System pertains</description>
     <value>-1</value>
     </param>
</input>

<returns type="casacoordsys"/>
<description>

This function stores the Coordinate System of an image in a {\stf
Coordsys} tool.  

By default, the Coordinate System describes all of the axes in the
image.  If you desire, you can select a subset of the axes, thus
reducing the dimensionality of the Coordinate System.   This may be
useful if you are supplying a Coordinate System to the
constructors  <link anchor="images:image.fromarray.constructor">fromarray</link> or
<link anchor="images:image.fromshape.constructor">fromshape</link>.

</description>

<example>
\begin{verbatim}
#
print "\t\t csys Ex 1 \t"
ia.maketestimage('hcn',overwrite=true)
ia.summary()
mycs = ia.csys([0,1])
imshape = ia.shape()
ia.fromshape(outfile='test', shape=imshape, csys=mycs.torecord(), overwrite=true)
ia.summary()
mycs.done()
ia.close()
#
\end{verbatim}

In this example, we create a Coordinate System pertaining to the first two axes
of the image and create a new (empty) 2D image with it with the {\cf fromshape}
constructor.
</example>
</method>
-->

 
   <method type="function" name="coordmeasures">
   <shortdescription>Convert from pixel to world coordinate wrapped as Measures</shortdescription>
<!--   
<output>
     <param type="any" direction="out" name="intensity">
     <any type="record"/>
     <description>Returned intensity quantity for pixel</description>
     </param>

     <param type="any" direction="out" name="direction">
     <any type="record"/>
     <description>Returned direction measure for pixel</description>
     </param>

     <param type="any" direction="out" name="frequency">
     <any type="record"/>
     <description>Returned frequency measure for pixel</description>
     </param>

     <param type="any" direction="out" name="velocity">
     <any type="record"/>
     <description>Returned radial velocity measure for pixel</description>
     </param>
</output>
-->
<input>

     <param type="doubleArray" direction="in" name="pixel">
     <description>Absolute pixel coordinate.  Default is reference pixel.</description>
     <value>-1</value>
     </param>
</input>
<returns type="any"><any type="record"/></returns>

<description>

You can use this function to get the world coordinates for a specified
absolute pixel coordinate in the image.  You specify a pixel coordinate
(0-rel) for each axis in the image.    

If you supply fewer pixel values then there are axes in the image, your
value will be padded out with the reference pixel for the missing axes. 
Excess values will be ignored. 

The world coordinate is returned as a record of measures.  This
function is just a wrapper for the Coordsys tool <link
anchor="images:coordsys.toworld.function">toworld</link> function
(invoked with argument {\stfaf format='m'}).  Please see its
documentation for discussion about the formatting and meaning of the
measures.

This Image \tool\ function adds two additional fields to the return record.

The {\cf mask} field contains the value of the image \pixelmask\ at the
specified position. It is either T (pixel is good) or F (pixel is masked
as bad or the specified position was off the image). 

The {\cf intensity} field contains the value of the image (at the
nearest pixel to that given) and its units.  This is actually stored
as a <link anchor="quanta:quanta">Quantity</link>. This field does not exist
if the specified pixel coordinate is off the image.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t coordmeasures Ex 1 \t----"
ia.maketestimage('myimage',overwrite=true)
s = ia.shape()
for i in range(len(s)):
  s[i] = 0.5*s[i]
meas = ia.coordmeasures(s)
print meas.keys()                   # Get names of fields in record
#['intensity', 'mask', 'measure']
print meas['intensity']
#{'value': 1.39924156665802, 'unit': 'Jy/beam'}
print meas['measure']['direction']
#{'type': 'direction',
# 'm1': {'value': 5.817764003289323e-05, 'unit': 'rad'},
# 'm0': {'value': -5.8177644130875234e-05, 'unit': 'rad'}, 'refer': 'J2000'}
dir = meas['measure']['direction']  # Get direction coordinate
me.doframe(me.observatory('ATCA'))  # Set location on earth
me.doframe(me.epoch('utc','16jun1999/12:30:20'))  # Set epoch
azel = me.measure(dir,'azel')       # Convert to azimuth/elevation
print 'az,el=', qa.angle(azel['m0']), qa.angle(azel['m1'])  # Format nicely
#az,el= +105.15.47 -024.22.57
meas2=ia.coordmeasures()            # defaults to reference pixel
print meas2['intensity']
#{'value': 2.5064315795898438, 'unit': 'Jy/beam'}
print meas2['measure']['direction']
#{'type': 'direction',
# 'm1': {'value': 0.0, 'unit': 'rad'},
# 'm0': {'value': 0.0, 'unit': 'rad'}, 'refer': 'J2000'}
dir = meas2['measure']['direction'] # Get direction coordinate
me.doframe(me.observatory('ATCA'))  # Set location on earth
me.doframe(me.epoch('utc','16jun1999/12:30:20'))   # Set epoch
azel = me.measure(dir,'azel')       # Convert to azimuth/elevation
print 'az,el=', qa.angle(azel['m0']), qa.angle(azel['m1'])
#az,el= +105.16.05 -024.23.00
#
"""
\end{verbatim}

In this example we first find the world coordinates of the centre of the
image.  Then we use the Measures \tool\ {\stf me} to convert the
{\cf direction coordinate} field from J2000 to an azimuth and elevation
at a particular location at a particular time. 

</example>
</method>
 
   <method type="function" name="decompose">
   <shortdescription>Separate a complex image into individual components</shortdescription>
   
<input>
  
     <param type="any" direction="in" name="region">
	     <any type="record"/>
     <description>Region of interest.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="mask">
	     <any type="variant"/>
     <description>Boolean LEL expression or mask region.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="bool" direction="in" name="simple">
     <description>Skip contour deblending and scan for local maxima</description>
     <value>false</value>
     </param>
  
     <param type="double" direction="in" name="threshold">
     <description>Value of minimum positive contour.  Default is unset.</description>
     <value>-1</value>
     </param>
  
     <param type="int" direction="in" name="ncontour">
     <description>Number of contours to use in deblending (>= 2)</description>
     <value>11</value>
     </param>
  
     <param type="int" direction="in" name="minrange">
     <description>Minimum number of closed contours in a component (> 0)</description>
     <value>1</value>
     </param>
  
     <param type="int" direction="in" name="naxis">
     <description>Max number of perpendicular steps between contiguous pixels.  Values of 1, 2 or 3 are allowed.</description>
     <value>2</value>
     </param>
  
     <param type="bool" direction="in" name="fit">
     <description>Fit to the components after deblending?</description>
     <value>true</value>
     </param>
  
     <param type="double" direction="in" name="maxrms">
     <description>Maximum RMS of fit residuals to not retry fit (> 0).  Default is unset.</description>
     <value>-1</value>
     </param>
  
     <param type="int" direction="in" name="maxretry">
     <description>Maximum number of times to retry the fit (>= 0).  Default is unset.</description>
     <value>-1</value>
     </param>
  
     <param type="int" direction="in" name="maxiter">
     <description>Maximum number of iterations allowed in a single fit (> 0)</description>
     <value>256</value>
     </param>
  
     <param type="double" direction="in" name="convcriteria">
     <description>Criterion to establish convergence (>=0)</description>
     <value>0.0001</value>
     </param>
</input>
<returns type="any"><any type="variant"/>Returns component list as a Matrix.</returns>

<description>
This function is an image decomposition tool that performs several tasks,
with the end result being that a strongly blended image is separated into
components - both in the sense that it determines the parameters for each
component (assuming a Gaussian model) and that it physically assigns each
pixel in the image to an individual object.  The products of these two
operations are called the component list and the component map, 
respectively.  The fitting process (which determines the component list) and 
the pixel-decomposition process (which determines the component map) are
designed to work cooperatively to increase the efficiency and accuracy of
both.
 
The algorithm behind the decomposition is based on the function clfind,
described in Williams et al 1994, which uses a contouring procedure whereby
a closed contour designates a separate component.  The program first 
separates the image into clearly distint 'regions' of blended emission, then
contours each region to determine the areas constituting each component and
passes this information on to the fitter, which determines the component 
list.  

The contour deblending can optionally be replaced with a simpler local maximum
scan, and the fitting can be replaced with a moment-based estimation method to
speed up calculations on very large images or if either primary method causes
trouble, but in general this will impede the accuracy of the fit.

The function works with both two and three dimensional images.
</description>
<example>
\begin{verbatim}
"""
#
print "\t----\t decompose Ex 1 \t----"
ia.maketestimage()
print ia.decompose(threshold=2.5, maxrms=1.0)
#Attempt 1: Converged after 22 iterations
#Attempt 1: Converged after 16 iterations
#1: Peak: 17.955  Mu: [0.000327928, 8.62573e-05]
#               Axes: [0.00175981, 0.00142841]  Rotation: 1.29539
#2: Peak: 19.8093  Mu: [1.67927e-06, -0.000374393]
#                Axes: [0.00179054, 0.00132541]  Rotation: 1.78404
#3: Peak: 10.1155  Mu: [6.28252, -7.09688e-05]
#                Axes: [0.00180877, 0.00104523]  Rotation: 1.78847
#[[  1.79549522e+01   3.27928370e-04   8.62573434e-05   1.75980886e-03
#    8.11686337e-01   1.29538655e+00]
# [  1.98093319e+01   1.67927124e-06  -3.74393392e-04   1.79054437e-03
#    7.40229547e-01   1.78403902e+00]
# [  1.01155214e+01   6.28252172e+00  -7.09688029e-05   1.80877140e-03
#    5.77867746e-01   1.78847444e+00]]
ia.close()
#
"""
\end{verbatim}
</example>
</method> 

 
   <method type="function" name="deconvolvecomponentlist">
   <shortdescription>Deconvolve a componentlist from the restoring beam</shortdescription>
   
<input>
  
     <param type="any" direction="in" name="complist">
	     <any type="record"/>
     <description>Componentlist to deconvolve</description>
     </param>
</input>
<returns type="any"><any type="record"/>Componentlist tool or fail</returns>

<description>

This function <!-- (short-hand name {\stff dcl}) --> deconvolves (a
record representation of) a Componentlist \tool\ from the restoring
beam, returning (a record representation of) a new Componentlist \tool.
If there is no restoring beam, a fail is generated.

Currently, only deconvolution of Gaussian components is supported.

See also functions <link anchor="images:image.setrestoringbeam.function">setrestoringbeam</link> and
<link anchor="images:image.restoringbeam.function">restoringbeam</link>.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t deconvolvecomponentlist Ex 1 \t----"
ia.maketestimage()
r = ia.fitsky(deconvolve=F)
cl1 = r['return']                      # cl1 and cl2 are record representations
cl2 = ia.deconvolvecomponentlist(cl1)  #   of componentlists
print cl1, cl2
cl.fromrecord(cl2)                     # set componentlist tool with record
ia.close()
cl.close()
#
"""
\end{verbatim}
</example>
</method>


<method type="function" name="deconvolvefrombeam">
   <shortdescription> Helper function to deconvolve the given source Gaussian
   from a beam Gaussian to return a model Gaussian
   </shortdescription>
    
<returns type="any">
  <any type="record"/>
  <description> record containing the model Gaussian parameters</description>
</returns>
  
<description>

   This is a helper function. It is to provide a way to deconvolve gaussians from other gaussians if that is what is needed for example removing a beam Gaussian from a Gaussian source. To run this function the tool need not be attached to an image. 

The return value is a record that contains the fit param and the return value is a boolean which is set to true if fit model is a point source

<!--These functions are not for general user use.-->

</description>

<input>
  <param type="any" direction="in" name="source">
    <any type="variant"/>
    <description>Three quantities that define the source majoraxis, minoraxis and Position angle</description>
    <value></value>
  </param>
  <param type="any" direction="in" name="beam">
    <any type="variant"/>
    <description>Three quantities that define the beam majoraxis, minoraxis and Position angle</description>
    <value></value>
  </param>
</input>


<example>
\begin{verbatim}
"""
#
print "\t----\t deconvolvefrombeam Ex 1 \t----"
ia.maketestimage()
recout=ia.deconvolvefrombeam(source=['5arcmin', '3arcmin', '20.0deg'], beam=['50arcsec','30arcsec', '15deg'])
ia.close()
print 'Is pointsource ', recout['return']
print 'major=',recout['fit']['major']
print 'minor=',recout['fit']['minor']
print 'pa=',recout['fit']['pa']




"""
\end{verbatim}
</example>
</method>



   <method type="function" name="remove">
   <shortdescription>Delete the image file associated with this image tool</shortdescription>
   
<input>
     <param type="bool" direction="in" name="done">
     <description>Destroy this tool after deletion</description>
     <value>false</value>
     </param>
</input>

<returns type="bool"/>
<description>

This function first <link anchor="images:image.close.function">closes</link> the
\imagetool\ which detaches it from its underlying \imagefile.  It then
deletes that \imagefile.  If {\stfaf done=F}, the \imagetool\ is still
viable, and can be used with function <link anchor="images:image.open.function">open</link>
to open a new \imagefile.  Otherwise the \imagetool\ is destroyed. 

</description>
<example>
\begin{verbatim}
"""
#
print "\t----\t remove Ex 1 \t----"
ia.maketestimage('myimage',overwrite=true)
ia.close()
ia.maketestimage('myotherimage',overwrite=true)
ia.close()
ia.open('myimage')          # Attach to `myimage'
ia.remove(F)                # Close imagetool and delete `myimage'
ia.open('myotherimage')     # Open new imagefile `myotherimage'
ia.remove()
print "!!!EXPECT THE FOLLOWING TO GENERATE AN ERROR MESSAGE!!!"
ia.open('myimage')          # 'myimage' was deleted above
ia.close()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="removefile">
   <shortdescription>Delete an unattached image file from disk.
Note: use remove() if the image file is attached to the image tool.
</shortdescription>
   
<input>
     <param type="string" direction="in" name="file">
     <description>Name of image file/directory to be removed.  Must be specified.</description>
     </param>
</input>

<returns type="bool"/>
<description>
This function deletes the specified image file.
</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t removefile Ex 1 \t----"
ia.maketestimage('myimage',overwrite=true)
ia.close()
ia.removefile('myimage')                    # remove image 'myimage'
ia.maketestimage('myimage',overwrite=false) # error here if 'myimage' exists
ia.close()
ia.removefile('myimage')
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="done">
   <shortdescription>Destroy this image tool</shortdescription>
   
<input>
  
     <param type="bool" direction="in" name="remove">
     <description>Delete disk file as well?</description>
     <value>false</value>
     </param>
</input>
<returns type="bool"/>
<description>

If you no longer need to use an \imagetool, calling this function will
free up its resources.  That is, it destroys the \tool.  You can no
longer call any functions on the \tool\ after it has been {\stff done}.
DO NOT done THE DEFAULT IMAGE TOOL ia!

If the Image \tool\ is associated with a disk file,
you can also delete that if you wish by setting
{\stfaf delete=T}.  By default any associated disk
file is not deleted.

This function is different from the {\stff close} function
because the latter does not destroy the \imagetool.  For example, you
can use function {\stff open} straight after {\stff close} on the
same \tool. 

</description>
<example>
\begin{verbatim}
"""
#
print "\t----\t done Ex 1 \t----"
ia.maketestimage('myfile',overwrite=true)
subim = ia.subimage('myfile2',overwrite=true)
subim.summary()
subim.done()
subim.open('myfile')      # Error: tool `subim' has been destroyed
ia.close()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="fft">
   <shortdescription>FFT the image</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="real">
     <description>Output real image file name</description>
     <value></value>
     </param>
  
     <param type="string" direction="in" name="imag">
     <description>Output imaginary image file name</description>
     <value></value>
     </param>
  
     <param type="string" direction="in" name="amp">
     <description>Output amplitude image file name</description>
     <value></value>
     </param>
  
     <param type="string" direction="in" name="phase">
     <description>Output phase image file name</description>
     <value></value>
     </param>
  
     <param type="intArray" direction="in" name="axes">
     <description>Specify pixel axes to FFT.  Default is sky plane(s).</description>
     <value>-1</value>
     </param>
  
     <param type="any" direction="in" name="region">
	     <any type="record"/>
     <description>The region of interest.  Default is whole image.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="mask">
	     <any type="variant"/>
     <description>OTF mask, Boolean LEL expression or mask region.  Default is none.</description>
     <value></value>
     </param>
</input>
<returns type="bool"/>

<description>

This function fast Fourier Transforms the image to the Fourier plane. 
If you leave {\stfaf axes} unset, then the sky plane of the image (if
there is one) is transformed.  Otherwise, you can specify which
axes you wish to transform.  Note that if you specify a sky axis, you
must specify both of them. 

You specify which form you wish to see the result in by specifying the
desired output image file name(s).  At some point, an output complex
image will also be allowed (when the Image \tool\ can deal with it). 

Before the FFT is taken, any masked pixels are replaced by zero. 
The output mask is the combination (logical OR) of the default input
\pixelmask\ (if any) and the OTF mask.  Any other input \pixelmasks\
will not be copied.  Use function
<link anchor="images:image.maskhandler.function">maskhandler</link> if you need to copy other
masks too.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t fft Ex 1 \t----"
ia.maketestimage('gc.small', overwrite=true)
ia.fft(real='r.im', amp='a.im')
ia.close()
ia.removefile('r.im')     # clean up example ouptut files
ia.removefile('a.im')
#
"""
\end{verbatim}
This transforms only the sky plane(s).  The real and amplitude 
images only are written out.
</example>

<example>
\begin{verbatim}
"""
#
print "\t----\t fft Ex 2 \t----"
ia.fromshape('gc.small', [64,64,128], overwrite=true)
ia.fft(amp='amp.im', phase='p.im', axes=[2])
ia.close()
ia.removefile('amp.im')   # clean up example output files
ia.removefile('p.im')
#
"""
\end{verbatim}
This transforms only the third axis of the image. The amplitude and phase
images only are written out.
</example>
</method>

 
   <method type="function" name="findsources">
   <shortdescription>Find point sources in the sky</shortdescription>
   
<input>
  
     <param type="int" direction="in" name="nmax">
	     <description>Maximum number of sources to find, > 0</description>
     <value>20</value>
     </param>
  
     <param type="double" direction="in" name="cutoff">
     <description>Fractional cutoff level</description>
     <value>0.1</value>
     </param>
  
     <param type="any" direction="in" name="region">
	     <any type="record"/>
     <description>The 2-D region of interest to fit.  Default is whole image.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="mask">
	     <any type="variant"/>
     <description>OTF mask, Boolean LEL expression or mask region.  Default is none.</description>
     <value></value>
     </param>
  
     <param type="bool" direction="in" name="point">
     <description>Find only point sources?</description>
     <value>true</value>
     </param>
  
     <param type="int" direction="in" name="width">
     <description>Half-width of fit grid when point=F</description>
     <value>5</value>
     </param>
  
     <param type="bool" direction="in" name="negfind">
     <description>Find negative sources as well as positive?</description>
     <value>false</value>
     </param>
</input>
<returns type="any"><any type="record"/>Componentlist tool or fail</returns>

<description>

This function <!-- (short-hand name {\stff fs}) --> finds strong point sources in
the image.  The sources are returned in a record that can be used by a
<link anchor="componentmodels:componentlist">Componentlist</link> \tool. 

An efficient method is used to locate sources under the assumption that
they are point-like and not too close to the noise.  Only sources with a
peak greater than the {\stfaf cutoff} fraction of the strongest source
will be found.  Only positive sources will be found, unless the {\stfaf
negfind=T} whereupon positive and negative sources will be found. 

After the list of point sources has been made, you may choose to make a
Gaussian fit for each one ({\stfaf point=F}) so that shape information
can be recovered as well.    You can specify the half-width of the
fitting grid with argument {\stfaf width} which defaults to 5 (fitting
grid would then be [11,11] pixels). If you set {\stfaf width=0}, this is
a signal that you would still like Gaussian components returned, but a
default  width should be used for the Gaussian shapes.  The default is
such that the component is circular with a FWHM of {\stfaf width}
pixels.

Thus, if {\stfaf point=T}, the components in the returned Componentlist
are Point components.  If {\stfaf point=F}  then Gaussian components are
returned.

The \region\ must be 2-dimensional and it must hold a region of the sky.
Any degenerate trailing dimensions in the region are discarded.

See also the function <link
anchor="images:image.fitsky.function">fitsky</link> (for which {\stff
findsources} can provide an initial estimate). <!-- and the <link
anchor="images:imagefitter">Imagefitter</link> \tool.-->

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t findsources Ex 1 \t----"
ia.maketestimage()
clrec = ia.findsources(nmax=5, cutoff=0.5)
print clrec
#
"""
\end{verbatim}

All sources stronger than 0.5 of the strongest will be found.
We use the Componentlist GUI to look at the strongest component.

</example>
</method>

 
   <method type="function" name="fitallprofiles">
   <shortdescription>Fit all 1-dimensional profiles in a region</shortdescription>
   
<input>
  
     <param type="any" direction="in" name="region">
	     <any type="record"/>
     <description>The region of interest.  Default is whole image.</description>
     <value></value>
     </param>
  
     <param type="int" direction="in" name="axis">
	     <description>The profile axis. Default is spectral axis.</description>
     <value>-1</value>
     </param>
  
     <param type="any" direction="in" name="mask">
	     <any type="variant"/>
     <description>OTF mask, Boolean LEL expression or mask region.  Default is none.</description>
     <value></value>
     </param>
  
     <param type="int" direction="in" name="ngauss">
     <description>Number of Gaussian elements</description>
     <value>1</value>
     </param>
  
     <param type="int" direction="in" name="poly">
     <description>Order of polynomial element.  Default is none.</description>
     <value>-1</value>
     </param>
  
     <param type="string" direction="in" name="sigma">
     <description>Weights image.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="string" direction="in" name="fit">
     <description>Output fitted image file name.  Default is none.</description>
     <value></value>
     </param>
  
     <param type="string" direction="in" name="resid">
     <description>Output fitted residuals image file name. Default is none.</description>
     <value></value>
     </param>
</input>
<returns type="bool"/>

<description>
This function fits all of the 1-dimensional profiles in the
specified region with a given combination of Gaussians and a polynomial.
It makes a separate automatic estimate for each profile before making
the fit.

Any masked input pixels do not contribute to the fit. The output mask is
a copy of the default input \pixelmask\ (if any); the OTF mask does not
contribute. Any other input \pixelmasks\ will not be copied.  Use
function <link anchor="images:image.maskhandler.function">maskhandler</link> if you need to
copy other masks too.  Use function
<link anchor="images:image.calcmask.function">calcmask</link> if you need to  apply the OTF
mask to the output.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t fitallprofiles Ex 1 \t----"
ia.fromshape('gc.small', [64,64,128], overwrite=true)
ia.fitallprofiles (axis=2, fit='gc.fit')
ia.close()
ia.removefile('gc.fit')
ia.removefile('gc.small')
#
"""
\end{verbatim}
</example>

</method>

 
   <method type="function" name="fitprofile">
   <shortdescription>Fit 1-dimensional profile with functional forms</shortdescription>
   
<output>  
     <param type="doubleArray" direction="out" name="values">
     <description>Output fitted or estimated profile</description>
     </param>
  
     <param type="doubleArray" direction="out" name="resid">
     <description>Output residual profile</description>
     </param>
</output>  

<input>
  
     <param type="any" direction="in" name="region">
	     <any type="record"/>
     <description>The region of interest.  Default is whole image.</description>
     <value></value>
     </param>
  
     <param type="int" direction="in" name="axis">
     <description>The profile axis. Default is spectral axis.</description>
     <value>-1</value>
     </param>
  
     <param type="any" direction="in" name="mask">
	     <any type="variant"/>
     <description>OTF mask, Boolean LEL expression or mask region.  Default is none.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="estimate">
	     <any type="record"/>
     <description>Initial estimate.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="int" direction="in" name="ngauss">
     <description>Number of Gaussian elements.  Default is one.</description>
     <value>-1</value>
     </param>
  
     <param type="int" direction="in" name="poly">
     <description>Order of polynomial element.  Default is unset.</description>
     <value>-1</value>
     </param>
  
     <param type="bool" direction="in" name="fit">
     <description>Do fit or get estimate</description>
     <value>true</value>
     </param>
  
     <param type="string" direction="in" name="sigma">
     <description>Weights image.  Default is unset.</description>
     <value></value>
     </param>
</input>
<returns type="any"><any type="record"/></returns>

<description>
This function fits 1-dimensional models to profiles.  All data in the specified
region (except along the fitting axis) are averaged.

The behaviour is primarily controlled by argument {\stfaf fit}.

If {\stfaf fit==T} then you need an estimate.  If {\stfaf estimate}
is unset, then that estimate is automatically provided by
fitting {\stfaf ngauss} gaussians.  If {\stfaf estimate}
is set then it is used to provide the initial estimate.

The functional form that is actually fit for will be any polynomial
specified by the {\stfaf poly} argument, plus the number of gaussians 
present in the estimate.

If {\stfaf fit==F} then an estimate is created and returned.
If {\stfaf estimate} is empty, than an estimate for {\stfaf ngauss}
gaussians is made.  If {\stfaf estimate} is not empty, then
it is just evaluated.

After calling this function, argument {\stfaf values} holds the fitted or estimated 
profile values, and argument  {\stfaf resid} holds the fitted or estimated residual 
profile values.  Look at the example for information on accessing returned values.

The output mask is a copy of the default input \pixelmask\ (if any); the
OTF mask does not contribute. Any other input \pixelmasks\ will not be
copied.  Use function <link anchor="images:image.maskhandler.function">maskhandler</link> if
you need to copy other masks too.  Use function
<link anchor="images:image.calcmask.function">calcmask</link> if you need to  apply the OTF
mask to the output.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t fitprofile Ex 1 \t----"
ia.maketestimage('gc.small', overwrite=true)
fit = ia.fitprofile(axis=1, fit=T)     # Auto-est and fit
print fit.keys()
#print fit['return']
#print fit['values']
#print fit['resid']

ia.close()
#
"""
\end{verbatim}
</example>

</method>

 
   <method type="function" name="fitpolynomial">
   <shortdescription>Fit 1-dimensional polynomials to profiles</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="residfile">
     <description>Output fitted residuals image file name.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="string" direction="in" name="fitfile">
     <description>Output fitted image file name.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="string" direction="in" name="sigmafile">
     <description>Input weights image for fitting.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="int" direction="in" name="axis">
     <description>The profile axis to fit, Default is spectral axis if it exists else last axis.</description>
     <value>-1</value>
     </param>
  
     <param type="int" direction="in" name="order">
     <description>The order of the polynomial</description>
     <value>0</value>
     </param>
  
     <param type="any" direction="in" name="region">
	     <any type="record"/>
     <description>The region of interest.  Default is whole image.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="mask">
	     <any type="variant"/>
     <description>OTF mask, Boolean LEL expression or mask region.  Default is none.</description>
     <value></value>
     </param>
  
     <param type="bool" direction="in" name="overwrite">
     <description>Overwrite (unprompted) pre-existing output files?</description>
     <value>false</value>
     </param>
</input>
<returns type="casaimage"/>
<description>

This function fits 1-dimensional polynomials to profiles and subtracts
the fits.  You specify the order of the polynomial via the {\stfaf
order} argument.  The returned on-the-fly Image tool holds the residual
image. You can optionally also write out the fitted image.

The output mask is a copy of the default input \pixelmask\ (if any); the
OTF mask does not contribute to the output mask. Any other input
\pixelmasks\ will not be copied.  Use function
<link anchor="images:image.maskhandler.function">maskhandler</link> if you need to copy other
masks too.  Use function <link anchor="images:image.calcmask.function">calcmask</link> if you
need to  apply the OTF mask to the output.

<example>
\begin{verbatim}
"""
#
print "\t----\t fitpolynomial Ex 1 \t----"
ia.maketestimage('hcn',overwrite=true)
myim = ia.fitpolynomial(order=2, axis=1)  # OTF Image tool is virtual
print myim.summary()
myim.done()
ia.close()
#
"""
\end{verbatim}

\begin{verbatim}
"""
#
print "\t----\t fitpolynomial Ex 2 \t----"
ia.fromshape(shape=[128,128,64,4])                  # RA/DEC/FREQ/STOKES
# Specify pixels to fit via mask
mask = 'indexin(2, [0:19, 29, 34, 49:59, 89:127])'  # LEL's are 0-rel
myim = ia.fitpolynomial(order=2, mask=mask, axis=2) # axis 3 (0-rel)
print myim.summary()
myim.done()
ia.close()
#
"""
\end{verbatim}

This example can be thought of as an image-based continuum
substraction.  You specify the axis and the pixels where the spectrum
is line free (continuum), do the fit and create an on-the-fly image
tool attached to the residual (continuum subtracted) image.

</example>
</description>

</method>

 
   <method type="function" name="fitsky">
   <shortdescription>Fit 2-dimensional models to the sky</shortdescription>
   

<input>
  
     <param type="any" direction="in" name="region">
	     <any type="record"/>
     <description>The 2-D region of interest to fit. Default is whole image.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="mask">
	     <any type="variant"/>
     <description>OTF mask, Boolean LEL expression or mask region.  Default is none.</description>
     <value></value>
     </param>
  
     <param type="stringArray" direction="in" name="models">
     <description>The model types to fit simultaneously.  Default is ['gaussian']</description>
     <value>gaussian</value>
     </param>
  
     <param type="any" direction="in" name="estimate">
	     <any type="record"/>
     <description>Initial estimate for the model (from componentlist.torecord()). Default is to use an internal estimate.</description>
     <value></value>
     </param>
  
     <param type="stringArray" direction="in" name="fixedparams">
     <description>Parameters to hold fixed per model, String or vector of strings, one per model. Choose from ``fxyabp''.  Default is all parameters vary.</description>
     <value></value>
     </param>
  
     <param type="doubleArray" direction="in" name="includepix">
     <description>Range of pixel values to include.  For default behaviour see discussion.</description>
     <value>-1</value>
     </param>
  
     <param type="doubleArray" direction="in" name="excludepix">
     <description>Range of pixel values to exclude.  For default behaviour see discussion.</description>
     <value>-1</value>
     </param>
  
     <param type="bool" direction="in" name="fit">
     <description>Do fit or get estimate</description>
     <value>true</value>
     </param>
  
     <param type="bool" direction="in" name="deconvolve">
     <description>Deconvolve from restoring beam</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="list">
     <description>List informative messages to the logger</description>
     <value>true</value>
     </param>
</input>
<returns type="any"><any type="record"/>
Componentlist tool or fail</returns>

<description>

This function fits 2-dimensional models to regions of the sky.  You
can fit multiple models simulataneously, although only Gaussian models
are currently implemented.  The fit is returned as a record that can
be loaded into a <link
anchor="componentmodels:componentlist">Componentlist</link> \tool\
with the fromrecord() function.  <!-- See also the <link
anchor="images:imagefitter">imagefitter</link> \tool\ which is an
interactive high-level \tool\ based around this function.-->

\begin{itemize}

\item If {\stfaf fit=T} the fit is done and returned.
If {\stfaf fit=F} the fit is not done.  Instead, the initial
auto-determined estimate is returned (this is used as the
starting guess of the actual fit); in this case, arguments
{\stfaf pixels, pixelmask, converged, estimate} and {\stfaf fixed} are
ignored.   The auto-fit can only be determined for a single
model.

\item The {\stfaf pixels} array is returned to you (in the return
record) with the residual of the fit.  The {\stfaf pixelmask} array is
returned to you (also in the return record) with the \pixelmask\ of
the specified region (this reflects both the underlying \pixelmask\
that you applied [if any] as well as the shape of the region).

\item If the fit converged, {\stfaf converged=T} in the return record.

\item The \region\ must be 2-dimensional and it must hold a region of the
sky.   Any degenerate trailing dimensions in the region are discarded.

\item The {\stfaf models} argument specifies the simultaneous model
types you would like to fit to the specified region.  Currently you
can only choose from `gaussian' (minimum match).  Specify the {\stfaf
models} argument as a vector of strings. <!--, or a string with comma
or space delimiters. --> Thus, to fit 3 simultaneous gaussians, you
would give <!-- {\stfaf models='gaussian gaussian gaussian'}.-->
{\stfaf models=['gaussian','gaussian','gaussian']}.

\item An initial automatically determined estimate of the parameters
of the model can be made for you (only for single model fits).
However, if this turns out to be poor, or if you want to hold some
parameters fixed, then you can enter your estimate with the {\stfaf
estimate} argument to which you supply a record associated with a
<link anchor="componentmodels:componentlist">Componentlist</link>
\tool\ (the same type of record that is returned by this function).
Note that an integrated flux is specified in the estimate of a
Gaussian component.  The estimate doesn't have to be the same type as
the model you are fitting.  For example you can give a Point model
estimate for a Gaussian fit.  The other parameter estimates will be
filled in for you.  The function <link
anchor="images:image.findsources.function">findsources</link> can be
used to get rough point source estimates.  You must supply one
estimate component for every model component that you wish to fit when
fitting multiple simultaneous models.

\item When you fit a model, you can choose which parameters you hold
fixed in the fit and which you solve for.  The {\stfaf fixed} argument
enables you to specify which parameters of the fit you hold fixed.  It
is entered as a vector of strings, one string for each model specifying
which parameters of the fit are fixed.

For gaussian models, that string should contain letters chosen from

\begin{itemize}
\item `f' - hold the peak flux (intensity) of the model fixed
\item `x' - hold the X location of the model fixed
\item `y' - hold the Y location of the model fixed
\item `a' - hold the major axis width fixed
\item `b' - hold the minor axis width fixed
\item `p' - hold the position angle fixed
\end{itemize}

If you hold any parameters fixed, you may need to specify an input
estimate.  As currently implemented, the effect of holding constant
the flux of a Gaussian model is confusing.  The input estimate is
specified as an integral flux.  This is used to determine the peak
flux of the Gaussian model.  It is this peak flux which is kept
constant.  The returned model's integral flux may be different than
the specified integral flux because other parameters (such as the
shape) of the Gaussian model may have changed.

\item Often when fitting, it is advisable to exclude noise pixels.  The
{\stfaf includepix} (pixel value range to include) or {\stfaf
excludepix} (pixel value range to exclude) arguments (you can't specify
them both) allow you to select a subset of the pixels for the fit.  If
you only give one value for either of these vectors, say {\stfaf
includepix=[b]}, then this is interpreted as {\stfaf
includepix=[-abs(b),abs(b)]}.  

For total intensity (Stokes I) images, if neither of these are specified
and  the peak in the region is positive, then all
pixels below zero are discarded.  If the peak is negative (a negative
source), all pixels above zero are discarded.  For other Stokes
parameters all pixels are included by default. 

\end{itemize}

Return value is a record with 4 elements. The keys are 'return',
'pixelmask', 'pixels' and 'converged'.

\begin{itemize}
\item {\tt 'return'} is a componentlist record which can be used with the componentlist tool.  (The returned flux values are integral, not peak values.)
\item {\tt 'pixelmask'} is a boolean array that defines on the image where the residual values  are invalid
\item {\tt 'pixels'} is a float array containing the residuals after subtracting the fitted components
\item {\tt 'converged'} is a boolean value representing the convergence of the fit.

\end{itemize} 

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t fitsky Ex 1 \t----"
ia.maketestimage('image.large', overwrite=true)
innerquarter=ia.setboxregion([0.25,0.25],[0.75,0.75],frac=true)
ia.fromimage(outfile='image.small', infile='image.large',
             region=innerquarter, overwrite=true)
ia.close()
ia.open('image.small')
cl1 = ia.fitsky(models=["gaussian"])
print cl1.keys()
# 'pixels','pixelmask','converged','return'
print cl1['return'] # 'return' field holds a componentlist record
if cl1['converged']:
  ia.close()
  ia.open('image.large')
  cl2 = ia.fitsky(models=["gaussian"], estimate=cl1['return'])
  print cl2['return']
ia.close()
#
"""
\end{verbatim}

In this example we assume the image is 2-dimensional.  We fit the first
image using the internal starting estimate for the model parameters.  We
fit the second image using the result of the first fit as the starting
estimate. 

</example>
</method>




 
<method type="function" name="fromrecord">
   <shortdescription>Generate an image from a record</shortdescription>
   
<input>  
  
     <param type="any" direction="in" name="record">
     <any type="record"/>
     <description>Record containing the image</description>
     </param>
     <param type="string" direction="in" name="outfile">
     <value></value>
     <description>The name of the diskfile to be created for image from 
     record
     </description>
     </param>
</input>    
<returns type="bool"/>
  
<description>
You can convert an associated image to a record 
(<link anchor="images:image.torecord.function">torecord</link>) or  imagepol tool functions  will sometimes give you a record.  This function
(fromrecord) allows you to set the contents of an image tool to the content of the record

<!--These functions are not for general user use.-->

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t fromrecord Ex 1 \t----"
ia.maketestimage('image.large', overwrite=true)
rec=ia.torecord()
ia.close()
ia.fromrecord(rec, "testimage")


"""
\end{verbatim}
</example>

</method>





 
   <method type="function" name="getchunk">
   <shortdescription>Get the pixel values from a regular region of the image into an array</shortdescription>
   
<input>
  
     <param type="intArray" direction="in" name="blc">
     <description>Bottom-Left-Corner (beginning) of pixel section.  Default is start of image.</description>
     <value>-1</value>
     </param>
  
     <param type="intArray" direction="in" name="trc">
     <description>Top-Right-Corner (end) of pixel section.  Default is end of image.</description>
     <value>-1</value>
     </param>
  
     <param type="intArray" direction="in" name="inc">
     <description>increment (stride) along axes</description>
     <value>1</value>
     </param>
  
     <param type="intArray" direction="in" name="axes">
     <description>Axes to average over.  Default is none.</description>
     <value>-1</value>
     </param>
  
     <param type="bool" direction="in" name="list">
     <description>List bounding box to logger?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="dropdeg">
     <description>Drop degenerate axes?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="getmask">
     <description>Get the pixel mask rather than the pixel values</description>
     <value>false</value>
     </param>
</input>
<returns type="any"><any type="variant"/></returns>
<description>

This function returns the pixels (or optionally the pixel mask) from the
\imagefile\ between {\stfaf blc} and {\stfaf trc} inclusively.  An
increment may be specified with {\stfaf inc}.  Note that if you retrieve
too many pixels, you might cause swapping since the pixels are kept in
memory. 

Any illegal {\stfaf blc} values are set to zero.  Any illegal {\stfaf
trc} values are set to the end of the image.  If any {\stfaf trc $<$
blc}, you get the whole image for that axis.  Any illegal {\stfaf inc}
values are set to unity. 

The argument {\stfaf axes} can be used to reduce the dimensionality of
the output array. It specifies which pixel axes of the image to
{\bf average} the data over.  For example, consider a 3-D image.
With {\stfaf axes=[0,1]} and all other arguments left at their defaults,
the result would be a 1-D vector, a profile along the third axis,
with the data averaged over the first two axes.

A related function is <link
anchor="images:image.getregion.function">getregion</link> which
retrieves the pixels or \pixelmask\ from a potentially more complex
\region.  Function {\stff getchunk} is retained because it is faster
and therefore preferable for repeated operation in loops if the
\pixelmask\ is not required and the region is a simple box.

<!--
If you ask for the pixel mask as well, the return value is
a record holding two fields, 'pixel' and 'pixelmask'.  If
you do not ask for the pixel mask (default), the pixels
are returned directly. -->

If you set getmask=T, the return value is the 'pixelmask' rather than
the 'pixel' image.

</description>

<example>
Suppose that we have a 3-dimensional image called {\sff im}. Then:
\begin{verbatim}
"""
#
print "\t----\t getchunk Ex 1 \t----"
ia.fromshape(shape=[64,64,128])
pix = ia.getchunk()                      # all pixels
ia.calcmask('T')                         # give image a mask
pix = ia.getchunk([1,1,1], [10,10,1])    # 10 by 10 section of plane # 1
pix = ia.getchunk([1,1], [1,1])          # first spectrum
pix = ia.getchunk(inc=[1,5])             # all planes, decimated by 5 in y
mask = ia.getchunk(getmask=T)            # Get pixelmask
ia.close()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="getregion">
   <shortdescription>Get pixels or mask from a region-of-interest of the image</shortdescription>

<input>

     <param type="any" direction="in" name="region">
             <any type="record"/>
     <description>The region of interest.  Default is whole image.</description>
     <value></value>
     </param>

     <param type="intArray" direction="in" name="axes">
     <description>Axes to average over.  Default is none.</description>
     <value>-1</value>
     </param>

     <param type="any" direction="in" name="mask">
             <any type="variant"/>
     <description>OTF mask, Boolean LEL expression or mask region.  Default is none.</description>
     <value></value>
     </param>

     <param type="bool" direction="in" name="list">
     <description>List the bounding box to the logger</description>
     <value>false</value>
     </param>

     <param type="bool" direction="in" name="dropdeg">
     <description>Drop degenerate axes</description>
     <value>false</value>
     </param>

     <param type="bool" direction="in" name="getmask">
     <description>Get the pixel mask rather than pixel values</description>
     <value>false</value>
     </param>
</input>
<returns type="any"><any type="variant"/></returns>
<description>

This function recovers the image pixel or \pixelmask\ values in the
given <link anchor="images:regionmanager">region-of-interest</link>.  <!-- You can
recover either or both of these by specifying the appropriate argument.-->
Regardless of the shape of the {\stfaf region} you have specified, the
shape of the {\stfaf pixels} and {\stfaf pixelmask} arrays must
necessarily be the bounding box of the specified region.  If the region
extends beyond the image, it is truncated.

Recall that the recovered \pixelmask\ will reflect both the \pixelmask\
stored in the image, and the \region\ (their masks are `anded') -- see
the \htmlref{discussion}{IMAGE:MASKSANDREGIONS} in the introduction
about this. 

The argument {\stfaf axes} can be used to reduce the dimensionality of
the output array. It specifies which pixel axes of the image to
average the data over.  For example, consider a 3-D image.  With
{\stfaf axes=[0,1]} and all other arguments left at their defaults,
the result would be a 1-D vector, a profile along the third axis, with
the data averaged over the first two axes.

This function differs in three ways from {\stff getchunk}.  First, the
region can be much more complex (e.g.  a union of polygons) than the
simple {\stfaf blc}, {\stfaf trc}, and {\stfaf inc} of {\stff
getchunk} (although such a region can be created of course).  Second,
it can be used to recover the \pixelmask\ or the pixels.  Third, it is
less efficient than {\stff getchunk} for doing the same thing as
{\stff getchunk}.  So if you are interested in say, iterating through
an image, getting a regular hyper-cube of pixels and doing something
with them, then {\stff getchunk} will be faster.  This would be
especially noticeable if you iterated line by line through a large
image.

</description>

<example>
Suppose that we have a 3-dimensional image called {\sff cube} and wish
to recover the pixel from a simple regular region.
\begin{verbatim}
"""
#
print "\t----\t getregion Ex 1 \t----"
ia.fromshape('cube', [64,64,64], overwrite=true)
#r1=rg.box(blc=[10,10,10],trc=[30,40]) # Create region
r1=ia.setboxregion([10,10,10],[30,40,40]) # Create region
pixels=ia.getregion(r1)
ia.close()
#
"""
\end{verbatim}
</example>

<example>
\begin{verbatim}
"""
#
print "\t----\t getregion Ex 2 \t----"
ia.fromshape('cube', [64,64,64], overwrite=true)
pixels = ia.getregion()
pixelmask = ia.getregion(getmask=T)  
#
"""
\end{verbatim}
In this example we recover first the pixels and then the pixel mask.
</example>
</method>

 
   <method type="function" name="getslice">
   <shortdescription>Get 1-D slice from the image</shortdescription>
   
<input>
  
     <param type="doubleArray" direction="in" name="x">
     <description>Polyline x vertices in absolute pixel coordinates</description>
     </param>
  
     <param type="doubleArray" direction="in" name="y">
     <description>Polyline y vertices in absolute pixel coordinates</description>
     </param>
  
     <param type="intArray" direction="in" name="axes">
     <description>Pixel axes of plane holding slice.  Default is first two axes.</description>
     <value type="vector"><value>0</value><value>1</value></value>
     </param>
  
     <param type="intArray" direction="in" name="coord">
     <description>Specify pixel coordinate for other axes.  Default is first pixel.</description>
     <value>-1</value>
     </param>
  
     <param type="int" direction="in" name="npts">
     <description>Number of points in slice.  Default is auto determination.</description>
     <value>0</value>
     </param>
  
     <param type="string" direction="in" name="method">
     <description>The interpolation method, String from 'nearest', 'linear', 'cubic'</description>
     <value>linear</value>
     </param>
  
</input>
<returns type="any"><any type="record"/></returns>
<description>

This function returns a 1-D slice (the pixels and opionally the pixel mask) from the
\imagefile.   The slice is constrained to lie in a plane of two  cardinal axes
(e.g. XY or YZ).  At some point this constraint will be relaxed.
A range of interpolation schemes are available.

You specify the slice as a polyline giving the x ({\stfaf x}) and y
({\stfaf y}) coordinates and the axes of the plane holding that slice
({\stfaf axes}).  As well, you must specify the absolute pixel
coordinates of the other axes ({\stfaf coord}).  This defaults to the
first pixel (e.g. first plane).

The return value is a record with fields 'pixels' (interpolated intensity), 
'mask' (interpolated mask), 'xpos' (x-location in absolute pixel coordinates), 
'ypos' (y-location in absolute pixel coordinates), 'distance' (distance along
slice in pixels), 'axes' (the x and y axes of slice).

<!-- You can optionally plot the slice via argument {\stfaf plot}
which defaults to False. -->

</description>

<example>
Suppose that we have a 2-dimensional image. Then:
\begin{verbatim}
"""
#
print "\t----\t getslice Ex 1 \t----"
ia.maketestimage();
rec = ia.getslice (x=[1,20], y=[2,30])     # SLice from [1,2] -> [20,30]
print rec.keys()
#['distance', 'xpos', 'axes', 'mask', 'ypos', 'pixel']
rec = ia.getslice (x=[1,20,25,11], y=[2,30,32,40]) # Polyline slice
ia.close()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="hanning">
   <shortdescription>Convolve one axis of image with a Hanning kernel</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="region">
	     <any type="record"/>
     <description>Region of interest.  Default is whole image.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="mask">
	     <any type="variant"/>
     <description>OTF mask, Boolean LEL expression or mask region.  Default is none.</description>
     <value></value>
     </param>
  
     <param type="int" direction="in" name="axis">
	     <description>Axis to convolve. Default is spectral axis if there is one</description>
     <value>-10</value>
     </param>
  
     <param type="bool" direction="in" name="drop">
     <description>Drop every other pixel on output?</description>
     <value>true</value>
     </param>
  
     <param type="bool" direction="in" name="overwrite">
     <description>Overwrite (unprompted) pre-existing output file?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="async">
     <description>Run asynchronously?</description>
     <value>false</value>
     </param>
</input>
<returns type="casaimage"/>

<description>

This function performs Hanning convolution ($z[i] = 0.25*y[i-1] +
0.5*y[i] + 0.25*y[i+1]$) of one axis of an image.  The convolution is
done in the image domain (i.e., not with an FFT). You can optionally drop
every other pixel on output. If you do this, the first output
pixel is centered on the second input pixel. 

If {\stfaf outfile} is given, the image is written to the specified
disk file.  If {\stfaf outfile} is unset, the Image \tool\ is
associated with a temporary image.  This temporary image may be in
memory or on disk, depending on its size.  When you destroy the
on-the-fly Image \tool\ created by this function (with the <link
anchor="images:image.done.function">done</link> function) this
temporary image is deleted.

Masked pixels will be assigned the value 0.0 before convolution. 
The output mask is the combination (logical OR) of the default input
\pixelmask\ (if any) and the OTF mask.  Any other input \pixelmasks\
will not be copied.  Use function
<link anchor="images:image.maskhandler.function">maskhandler</link> if you need to copy other
masks too.

See also the other convolution functions
<link anchor="images:image.convolve2d.function">convolve2d</link>,
<link anchor="images:image.sepconvolve.function">sepconvolve</link> and
<link anchor="images:image.convolve.function">convolve</link>. 

</description>

<example>
\begin{verbatim}
"""
print "\t----\t hanning Ex 1 \t----"
ia.fromshape('x', shape=[64,64,128], overwrite=true)
print ia.summary()
im2 = ia.hanning(outfile='x.hann')
print im2.summary()
im2.done(remove=T)
ia.close()
#
"""
\end{verbatim}

This would Hanning smooth the spectral axis of the image associated with
the default \imagetool\ {\stf ia}, if there is one.  Every other pixel on the
convolution axis is dropped on output, by default. 

</example>

</method>

 
   <method type="function" name="haslock">
   <shortdescription>Does this image have any locks set?</shortdescription>
   
   <returns type="boolArray"/>
<description>

This function can be used to find out whether the image has a read or a
write lock set.  It is not of general user interest.   It returns
a vector of Booleans of length 2.  Position 1 says whether
a read lock is set, position 2 says whether a write lock is set.

In general locking is handled automatically, with a built in lock
release cycle.  However, this function can be useful in scripts when a
file is being shared between more than one process.  See also functions
<link anchor="images:image.unlock.function">unlock</link> and
<link anchor="images:image.lock.function">lock</link>. 

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t haslock Ex 1 \t----"
ia.maketestimage('xx',overwrite=true)
ia.lock(writelock=T)
print ia.haslock()
#[True, True]
ia.unlock()
print ia.haslock()
#[False, False]
ia.lock(F)
print ia.haslock()
#[True, False]
ia.close()
#
"""
\end{verbatim}
This example acquires a read/write lock on the file and then unlocks it
and acquires just a read lock.
</example>
</method>

 
   <method type="function" name="histograms">
   <shortdescription>Compute histograms from the image</shortdescription>
   

<output>  
     <param type="any" direction="out" name="histout">
	     <any type="record"/>
     <description>record containing the histograms</description>
     </param>
</output>  

<input>
  
     <param type="intArray" direction="in" name="axes">
     <description>List of axes to compute histograms over.  Default is all axes.</description>
     <value>-1</value>
     </param>
  
     <param type="any" direction="in" name="region">
	     <any type="record"/>
     <description>Region of interest.  Default is whole image.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="mask">
	     <any type="variant"/>
     <description>OTF mask, Boolean LEL expression or mask region.  Default is none.</description>
     <value></value>
     </param>
  
     <param type="int" direction="in" name="nbins">
     <description>Number of bins in histograms, > 0</description>
     <value>25</value>
     </param>
  
     <param type="doubleArray" direction="in" name="includepix">
     <description>Range of pixel values to include.  Default is to include all pixels.</description>
     <value>-1</value>
     </param>
  
     <param type="bool" direction="in" name="gauss">
     <description>If T overlay a Gaussian on each histogram</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="cumu">
     <description>If T plot cumulative histograms, otherwise plot non-cumulatively</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="log">
     <description>If T plot the ordinate logarithmically, otherwise plot linearly</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="list">
     <description>If T then list some statistics as well</description>
     <value>true</value>
     </param>
  
     <param type="string" direction="in" name="plotter">
     <description>The PGPLOT device name to make plots on.  Default is no plotting.</description>
     <value></value>
     </param>
  
     <param type="int" direction="in" name="nx">
     <description>The number of subplots per page in the x direction, > 0</description>
     <value>1</value>
     </param>
  
     <param type="int" direction="in" name="ny">
	     <description>The number of subplots per page in the y direction; > 0</description>
     <value>1</value>
     </param>
  
     <param type="intArray" direction="in" name="size">
     <description>Size of plotter.  Default is [600, 450].</description>
     <value type="vector"><value>600</value><value>450</value></value>
     </param>
  
     <param type="bool" direction="in" name="force">
     <description>If T then force the stored statistical accumulations to be regenerated</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="disk">
     <description>If T then force the storage image to disk</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="async">
     <description>Run asynchronously?</description>
     <value>false</value>
     </param>
</input>
<returns type="bool"/>

<description>

This function <!-- (short-hand name is {\stff histo}) --> computes
histograms of the pixel values in the \imagefile.  You can <!-- plot them
and --> retrieve them (into a record) for further analysis or display.

The chunk of the image over which you compute the histograms is
specified by a vector of axis numbers (argument {\stfaf axes}).  For
example, consider a 3-dimensional image for which you specify {\stfaf
axes=[0,2]}.  The histograms would be computed for each XZ (axes 0 and
2) plane in the image.  You could then examine those histograms as a
function of the Y (axis 1) axis.  Or perhaps you set {\stfaf axes=[2]},
whereupon you could examine the histogram for each Z (axis 2) profile as
a function of X and Y location in the image. 

You have control over the number of bins for each histogram ({\stfaf 
nbins}).  The bin width is worked out automatically for each histogram
and may vary from histogram to histogram (the range of pixel values is
worked out for each chunk being histogrammed). 

You have control over which pixels are included in the histograms via
the {\stfaf includepix} argument.  This vector specifies a range of
pixel values to be included in the histograms.  If you only give one
value for this, say {\stfaf includepix=[b]}, then this is interpreted as
{\stfaf includepix=[-abs(b),abs(b)]}.  If you specify an inclusion
range, then the range of pixel intensities over which the histograms are
binned is given by this range too.  This is a way to make the bin width
the same for each histogram. 

You have control over the form of the histogram; the ordinate can be
linear or logarithmic ({\stfaf log=T}), and cumulative or non-cumulative
({\stfaf cumu=F}). 

You can overlay a Gaussian on each histogram ({\stfaf gauss=T}). It has
the same mean and standard deviation about the mean of the pixels that
were binned and the same integral as the histogram. 

You can list some additional statistical information about the data in
each image chunk that is being histogrammed ({\stfaf list=T}). 

You can retrieve the histograms into a record (the counts and
the abcissa values for each histogram).  The names of the fields in the
record are {\cf `values'} and {\cf `counts'}.  The shape of the first
dimension of those arrays contained in those fields is {\stfaf nbins}. 
The number and shape of the remaining dimensions are those in the image
for which you did not compute the histograms.  For example, in the
second example above, we set {\stfaf axes=[2]} and asked for histograms
as a function of the remaining axes, in this case, the X and Y (axes 0
and 1) axes.  The shape of each histogram array is then [nbins,nx,ny]. 
The form of the retrieved histograms is that specified by the {\stfaf
cumu} and {\stfaf log} arguments. 

<!--
The plotting is done directly on a PGPLOT plotting device (function
is always run synchronously when plotting). 
The syntax is {\stfaf plotter=name/type}.  For example
{\stfaf plotter='plot1.ps/ps'} (disk postscript file)
or {\stfaf plotter='plot1/glish'} (Glish PGplotter). -->

This function generates ``storage'' images, into which the histograms
and statistics are written.  They are only regenerated when necessary.
For example, if you run the function twice successively with identical arguments,
the histograms will be directly retrieved from the storage image.
However, you can force regeneration of the storage image
if you set {\stfaf force=T}.   The storage medium is either in memory
or on disk, depending upon its size.  You can force it to
disk if you set {\stfaf disk=T}, otherwise it decides for itself.

</description>

<example> 
\begin{verbatim}
"""
#
print "\t----\t histograms Ex 1 \t----"
ia.maketestimage()
ia.histograms()
ia.close()
#
"""
\end{verbatim} 
<!-- In this example we plot the histogram of the entire image
on a PGPLOT X-windows device.-->
In this example we obtain the histogram of the entire image.
</example>

<example> 
\begin{verbatim} 
"""
#
print "\t----\t histograms Ex 2 \t----"
ia.fromshape(shape=[128,128,64]) #attach your 3-dim image instead
r= ia.histograms(axes=[0,2],nbins=30,includepix=1e-3,cumu=T, gauss=T)
print r.keys()
#['return', 'histout']
ia.close()
#
"""
\end{verbatim} 

<!--  cumu=T,gauss=T,plotter='',nx=4,ny=3) -->


In this example, let us assume the image has 3 dimensions.  We
generate cumulative histograms of XZ (axes 0 and 2) planes for pixels
with values in the range $-0.001$ to $0.001$ and plot them with Gaussian
overlays as a function of Y-axis location. <!-- on the standard PGPLOT
X-windows device with 12 plots per page.-->  The histograms are retrieved
into a record {\gvf r['histout']} for further use. <!-- (because
we are plotting the function is run synchronously).-->

</example>

</method> 

 
   <method type="function" name="history">
   <shortdescription>Recover and/or list the history file</shortdescription>
   

<input>
  
     <param type="bool" direction="in" name="list">
     <description>List history to the logger?</description>
     <value>true</value>
     </param>
  
     <param type="bool" direction="in" name="browse">
     <description>Browse history with table browser? (NOT IMPLEMENTED YET!)</description>
     <value>false</value>
     </param>
</input>
<returns type="stringArray"/>

<description>

This function allows you to access the history file.

<!--
By default, you browse it with the Table browser ({\stfaf browse=T})
and the return value is T or a fail. -->

If {\stfaf browse=F} and {\stfaf list=F}, the history is returned by
the function as a vector of strings.  If {\stfaf list=T}, the history
is sent to the logger.  <!-- and the function returns T or a fail.-->

\casa\ tools that modify the MeasurementSet or an image file will save
history information.  Also, you can directly annotate the history file
with the function <link
anchor="images:image.sethistory.function">sethistory</link>.  History
from \fits\ file conversions is also stored and listable here.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t history Ex 1 \t----"
ia.maketestimage()
ia.history()                       # List history to logger
h = ia.history(list=F)             # Recover history in variable h
ia.history(list=T, browse=F)       # List history to logger
#
"""
\end{verbatim}
</example>
</method>

 
 
   <method type="function" name="insert">
   <shortdescription>Insert specified image into this image</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="infile">
     <description>Image file name.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="region">
	     <any type="record"/>
     <description>The region of interest of the input image.  Default is whole image.</description>
     <value></value>
     </param>
  
     <param type="doubleArray" direction="in" name="locate">
     <description>Location of input image in output image.  Default is centrally located.</description>
     <value>-1</value>
     </param>
</input>
<returns type="casaimage"/>

<description>

This function inserts the specified image (or part of it) into this image.
The specified image may be given via argument {\stfaf infile}
as a disk file name (it may be in native \casa, \fits, or Miriad
format; Look \htmlref{here}{IMAGES:FOREIGNIMAGES}  for more
information on foreign images). <!-- or directly as an Image tool. -->

If the {\stfaf locate} vector is not given, then the images are
aligned (to an integer pixel shift) by their reference pixels.

If {\stfaf locate} vector is given, then those values that are given,
give the absolute pixel in the output (this) image of the bottom left
corner of the input (sub)image.  For those values that are not given,
the input image is symmetrically placed in the output image. 

</description>
<example>
\begin{verbatim}
"""
#
print "\t----\t insert Ex 1 \t----"
ia.maketestimage('myfile.insert',overwrite=true)
ia.close()
ia.fromshape(shape=[200,200])
im1 = ia.insert(infile='myfile.insert')       # Align by reference pixel
ia.set(0.0)
im2 = ia.newimagefromfile('myfile.insert')
im3 = ia.insert(infile=im2.name(), locate=[]) # Align centrally
ia.set(0.0)
# This time align axis 0 as given and axis 1 centrally
im4 =ia.insert(infile='myfile.insert', locate=[20])
ia.close()                                    # close default tool and
im1.done()                                    # destroy on-the-fly images
im2.done()
im3.done()
im4.done()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="isopen">
   <shortdescription>Is this Image \tool\ open?</shortdescription>
   
<returns type="bool">Bool</returns>
<description>

This function can be used to find out whether the Image \tool\
is associated with an image or not.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t isopen Ex 1 \t----"
ia.maketestimage('zz',overwrite=true)
print ia.isopen()
#True
ia.close()
print ia.isopen()
#False
ia.open('zz')
print ia.isopen()
#True
ia.close()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="ispersistent">
   <shortdescription>Is the image persistent?</shortdescription>
   
<returns type="bool">Bool or fail</returns>
<description>

This function can be used to find out whether the image is persistent on
disk or not.  There is a subtle difference from the image being
virtual.  For example, a virtual image which references another
which is on disk is termed persistent.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t ispersistent Ex 1 \t----"
ia.fromshape(outfile='tmp', shape=[10,20], overwrite=true)
print ia.ispersistent()
#True
r = ia.setboxregion()                #rg.box() isn't implemented yet
ia.close()
#ia.fromimage(infile='tmp', region=rg.box())
ia.fromimage(infile='tmp', region=r)
print ia.ispersistent()
#True
im3 = ia.subimage()
print im3.ispersistent()            # Persistent virtual image !
#True
im4 = ia.imagecalc(pixels='tmp+tmp')
print im4.ispersistent()
#False
im3.done()
im4.done()
ia.close(remove=true)
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="lock">
   <shortdescription>Acquire a lock on the image</shortdescription>
   

<input>
  
     <param type="bool" direction="in" name="writelock">
     <description>Acquire a read/write (T) or a readonly (F) lock</description>
     <value>false</value>
     </param>
  
     <param type="int" direction="in" name="nattempts">
	     <description>Number of attempts, > 0.  Default is unlimiited.</description>
     <value>0</value>
     </param>
</input>
<returns type="bool">T or fail</returns>

<description>

This function can be used to acquire a Read or a Read/Write lock
on the \imagefile.   It is not of general user interest.  

In general locking is handled automatically, with a built in lock
release cycle.  However, this function can be useful in scripts when a
file is being shared between more than one process.  See also functions
<link anchor="images:image.unlock.function">unlock</link> and <link anchor="images:image.haslock.function">haslock</link>.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t lock Ex 1 \t----"
ia.maketestimage('xx', overwrite=true)
ia.lock(writelock=T)
ia.unlock()
ia.lock(writelock=F)
ia.close(remove=true)
#
"""
\end{verbatim}
This acquires a read/write lock on the file. Then we unlock it
and acquire a readonly lock.
</example>

</method>

 
   <method type="function" name="makecomplex">
   <shortdescription>Make a complex image</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="outfile">
     <description>Output Complex (disk) image file name</description>
     </param>
  
     <param type="string" direction="in" name="imag">
     <description>Imaginary image file name</description>
     </param>
  
     <param type="any" direction="in" name="region">
	     <any type="record"/>
     <description>The region of interest.  Default is whole image.</description>
     <value></value>
     </param>
  
     <param type="bool" direction="in" name="overwrite">
     <description>Overwrite (unprompted) pre-existing output file?</description>
     <value>false</value>
     </param>
</input>
<returns type="bool">T or fail</returns>

<description>

This function combines the current image with another image to make
a complex image.  The current image (i.e. that associated with this
Image \tool\ is assumed to be the Real image).  You supply
the Imaginary image; it must be disk-based at this time.

The output image cannot be associated with an Image \tool\ (does
not handle Complex images yet) and so the best you can do is
write it to disk.  The Viewer can view it.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t makecomplex Ex 1 \t----"
ia.maketestimage('ia.imag',overwrite=true)  #imaginary image
ia.close()
ia.maketestimage('ia.real',overwrite=true)  #assoc. real image
ia.makecomplex('ia.complex', 'ia.imag', overwrite=true)
ia.removefile('ia.imag')                    #cleanup
ia.removefile('ia.complex')
ia.close(remove=true)
#
"""
\end{verbatim}
</example>

</method>

 
   <method type="function" name="maskhandler">
   <shortdescription>Handle pixel masks</shortdescription>
   

<input>
  
     <param type="string" direction="in" name="op">
     <description>The operation.  One of 'set', 'delete', 'rename', 'get', 'copy' or 'default'</description>
     <value>default</value>
     </param>
  
     <param type="stringArray" direction="in" name="name">
     <description>Name of mask or masks.</description>
     <value></value>
     </param>
</input>
<returns type="stringArray"/>

<description>

This function <!-- (short-hand name {\stff mh}) --> is used to manage
or handle \pixelmasks\ . <!-- (there is also a custom GUI interface
through the <link
anchor="images:image.maskhandlergui.function">maskhandlergui</link>
function). --> A \casa\ image may contain zero, one or more
\pixelmasks.  Any of these masks can be designated the default
\pixelmask.  The default mask is acted upon by \casa\ applications.
For example, if you ask for statistics from an image, pixels which are
masked as bad (F) will be excluded from the calculations.

This function has an argument ({\stfaf op}) that specifies the
behaviour.  In all cases, you can shorten the operation string to three
characters.  It is not the job of this function to modify the values of
masks. 

\begin{itemize} 

\item{default - } this retrieves  the name of the default \pixelmask\
as the return value of the function call.

\item{get - } this retrieves the name(s) of the existing \pixelmasks\
as the return value of the function call (string or vector of strings).

\item{set - } this lets you change the default \pixelmask\ to that given by the
{\stfaf name} argument.  If {\stfaf name} is empty, then the default
mask is unset (i.e. an all good mask is effectively applied).

\item{delete - } this lets you delete the \pixelmasks\ specified by the
{\stfaf name} argument.  To delete more than one mask, {\stfaf name} can
be a vector of strings.    Any supplied \pixelmask\ name that does not
exist is silently ignored.

\item{rename - } this lets you rename the mask specified by {\stfaf name[0]}
to {\stfaf name[1]}.  Thus the {\stfaf name} argument must be a vector of
length 2.

\item{copy - } this lets you copy a mask to another in the same image, or
copy a mask from another image into this image.  Thus the {\stfaf name}
argument must be a vector of length 2.

For the first case, the first element of {\stfaf name} must be the name
of the mask to copy, and the second element must be the name of the
\pixelmask\ to which it will be copied. 

For the second case, the first element of {\stfaf name} must be the name
of the input image and \pixelmask\ with a colon delimiter (e.g. {\cf
hcn:mask2}). The second element must be the name of the \pixelmask\ to
which the input \pixelmask\ will be copied. 

\end{itemize}

Use the <link anchor="images:image.summary.function">summary</link> function to see the
available \pixelmasks.  You can do this either via the logger display, or via
the returned record, which contains the mask names. In the logger display,
any \pixelmask\ which is not the default mask is listed in square brackets.  If
a default mask is set, it is listed first, and is not enclosed in square
brackets. 

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t maskhandler Ex 1 \t----"
ia.maketestimage('g1.app', overwrite=true)
ia.calcmask('T', name='mask1')
ia.close()
ia.maketestimage('myimage', overwrite=true)
ia.calcmask('T')                         # Create some masks
ia.calcmask('T', name='mask1')
ia.calcmask('T', name='mask2')
names = ia.maskhandler('get')            # Get the mask names
print names
#['mask0', 'mask1', 'mask2']
name = ia.maskhandler('default')         # Get the default mask name
print name
#mask2
ia.maskhandler('set', ['mask1'])         # Make 'mask1' the default mask
ia.maskhandler('set', [''])              # Unset the default mask
ia.maskhandler('delete', ['mask1'])      # Delete 'mask1'
ia.calcmask('T', name='mask1')           # Make another 'mask1'
ia.maskhandler('delete', ['mask0', 'mask1'])# Delete 'mask0' and 'mask1'
ia.calcmask('T', name='mask1')
ia.maskhandler('rename', ['mask1', 'mask0'])# Rename 'mask1' to 'mask0'

# Copy 'mask1' from image 'g1.app'  to 'mask10' in image 'myimage'
ia.maskhandler('copy', ['g1.app:mask1', 'mask10'])
ia.removefile('g1.app')                  # Cleanup
ia.close()
#
"""
\end{verbatim}
</example>

</method>

 
   <method type="function" name="miscinfo">
   <shortdescription>Get the miscellaneous information record from an image</shortdescription>
   
   <returns type="any"><any type="record"/>record or fail</returns>

<description>

A \casa\ \imagefile\ can accumulate miscellaneous information
during its lifetime.  This information is stored in a record called the {\stff
miscinfo} record.  For example, the \fits\ filler puts header keywords
it doesn't otherwise use into the {\stff miscinfo} record.  This {\stff
miscinfo} record is not guaranteed to have any entries, so it's up to
you to check for any fields that you require. 

You can also put things into this record (see
<link anchor="images:image.setmiscinfo.function">setmiscinfo</link>) yourself, to keep
information that the system might not otherwise store for you. 

When the image is written out to \fits, the items in the
{\stff miscinfo} record are written to the \fits\ file
as keywords with the corresponding record field name.

</description>
<example>
\begin{verbatim}
"""
#
print "\t----\t miscinfo Ex 1 \t----"
ia.maketestimage()
print ia.miscinfo()       # print the record
ia.setmiscinfo("testing")
print ia.miscinfo()
header = ia.miscinfo()    # capture the record for further use
print header
ia.close()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="modify">
   <shortdescription>Modify image with a model</shortdescription>
   
<input>
  
     <param type="any" direction="in" name="model">
	     <any type="record"/>
     <description>Record representation of a ComponentList model</description>
     </param>
  
     <param type="any" direction="in" name="region">
	     <any type="record"/>
     <description>The 2-D region of interest.  Default is whole image.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="mask">
     <description>OTF mask, Boolean LEL expression or mask region.  Default is none.</description>
	     <any type="variant"/>
     <value></value>
     </param>
  
     <param type="bool" direction="in" name="subtract">
     <description>Subtract or add the model</description>
     <value>true</value>
     </param>
  
     <param type="bool" direction="in" name="list">
     <description>List informative messages to the logger</description>
     <value>true</value>
     </param>
  
     <param type="bool" direction="in" name="async">
     <description>Run asynchronously?</description>
     <value>false</value>
     </param>
</input>
<returns type="bool"/>

<description>

This function applies a model of the sky to the image. You can add or
subtract the model which is contained in a
<link anchor="componentmodels:componentlist">Componentlist</link> \tool.

The pixel values are only changed where the total mask
(combination of the default \pixelmask\ [if any] and the OTF mask)
is good (True).   If the computation fails for a particular
pixel (e.g. coordinate undefined) that pixel will be
masked bad.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t modify Ex 1 \t----"
ia.maketestimage()
clrec = ia.fitsky()
ia.modify(clrec['return'])
ia.close()
#
"""
\end{verbatim}

<!-- 
# ia.image('gc.small')
# cl = componentlist('list')
# ia.modify(cl)
In this example we subtract the model contained in the Table
called {\sff list}. -->

In this example we subtract the model returned by the fitsky function.

</example>
</method>

 
   <method type="function" name="maxfit">
   <shortdescription>Find maximum and do parabolic fit in the sky</shortdescription>
   
<input>
  
     <param type="any" direction="in" name="region">
	     <any type="record"/>
     <description>The 2-D region of interest to fit.  Default is whole image.</description>
     <value></value>
     </param>
  
     <param type="bool" direction="in" name="point">
     <description>Find only point sources?</description>
     <value>true</value>
     </param>
  
     <param type="int" direction="in" name="width">
     <description>Half-width of fit grid when point=F</description>
     <value>5</value>
     </param>
  
     <param type="bool" direction="in" name="negfind">
     <description>Find negative sources as well as positive?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="list">
     <description>List the fitted parameters to the logger?</description>
     <value>true</value>
     </param>
</input>
<returns type="any"><any type="record"/>
Componentlist tool or fail</returns>

<description>

This function finds the pixel with the maximum value in the region, and
then uses function <link anchor="images:image.findsources.function">findsources</link> 
to generate a Componentlist with one component.   The component
will be of type Point ({\stfaf point=T}) or Gaussian ({\stfaf point=F}).

If {\stfaf negfind=F} the maximum pixel value is found in the region and fit.
If {\stfaf negfind=T} the absolute maximum pixel value is found in the region
and fit.

See function <link anchor="images:image.findsources.function">findsources</link> for
a description of arguments {\stfaf point} and {\stfaf width}.

See also the function <link anchor="images:image.fitsky.function">fitsky</link>.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t maxfit Ex 1 \t----"
ia.maketestimage()
clrec = ia.maxfit()
print clrec          # There is only one component
ia.close()
#
"""
\end{verbatim}

</example>
</method>

 
   <method type="function" name="moments">
   <shortdescription>Compute moments from an image</shortdescription>
   
<input>
  
     <param type="intArray" direction="in" name="moments">
     <description>List of moments that you would like to compute.  Default is integrated spectrum.</description>
     <value>0</value>
     </param>
  
     <param type="int" direction="in" name="axis">
     <description>The moment axis. Default is the spectral axis if there is one.</description>
     <value>-10</value>
     </param>
  
     <param type="any" direction="in" name="region">
	     <any type="record"/>
     <description>Region of interest.  Default is whole image.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="mask">
	     <any type="variant"/>
     <description>OTF mask, Boolean LEL expression or mask region.  Default is none.</description>
     <value></value>
     </param>
  
     <param type="stringArray" direction="in" name="method">
     <description>List of windowing and/or fitting functions you would
     like to invoke.  Vector of strings from 'window', 'fit' and
     'interactive'.  The default is to not invoke the window or fit
     functions, and to not invoke any interactive functions.</description>
     <value></value>
     </param>
  
     <param type="intArray" direction="in" name="smoothaxes">
     <description>List of axes to smooth.  Default is no smoothing.</description>
     <value>-1</value>
     </param>
  
     <param type="any" direction="in" name="smoothtypes">
     <any type="variant"/>
     <description>List of smoothing kernel types, one for each axis to
     smooth.  Vector of strings from 'gauss', 'boxcar', 'hanning'.
     Default is no smoothing.</description>
     <value></value>
     </param>
  
     <param type="doubleArray" direction="in" name="smoothwidths">
     <description>List of widths (full width for boxcar, full width at
     half maximum for gaussian, 3 for Hanning) in pixels for the
     smoothing kernels. Vector of numeric. Default is no
     smoothing.</description>
     <value>0.0</value>
     </param>
  
     <param type="doubleArray" direction="in" name="includepix">
     <description>Range of pixel values to include.  Vector of 1 or 2 doubles.
      Default is include all pixel.</description>
     <value>-1</value>
     </param>
  
     <param type="doubleArray" direction="in" name="excludepix">
     <description>Range of pixel values to exclude.
      Default is exclude no pixels.</description>
     <value>-1</value>
     </param>
  
     <param type="double" direction="in" name="peaksnr">
     <description>The SNR ratio below which the spectrum will be rejected as noise (used by the window and fit functions only)</description>
     <value>3.0</value>
     </param>
  
     <param type="double" direction="in" name="stddev">
     <description>Standard deviation of the noise signal in the image (used by the window and fit functions only)</description>
     <value>0.0</value>
     </param>
  
     <param type="string" direction="in" name="doppler">
     <description>Velocity doppler definition for velocity computations along spectral axes</description>
     <value>RADIO</value>
     </param>
  
     <param type="string" direction="in" name="outfile">
     <description>Output image file name (or root for multiple moments).
     Default is input + an auto-determined suffix.</description>
     <value></value>
     </param>
  
     <param type="string" direction="in" name="smoothout">
     <description>Output file name for convolved image.  Default is don't
     save the convolved image.</description>
     <value></value>
     </param>
  
     <param type="string" direction="in" name="plotter">
     <description>The PGPLOT device name to make plots on.
      Default is no plotting.</description>
     <value></value>
     </param>
  
     <param type="int" direction="in" name="nx">
     <description>The number of subplots per page in the x direction.</description>
     <value>1</value>
     </param>
  
     <param type="int" direction="in" name="ny">
     <description>The number of subplots per page in the y direction.</description>
     <value>1</value>
     </param>
  
     <param type="bool" direction="in" name="yind">
     <description>Scale the y axis of the profile plots independently</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="overwrite">
     <description>Overwrite (unprompted) pre-existing output file?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="drop">
     <description>Drop moments axis from output images?</description>
     <value>true</value>
     </param>
  
     <param type="bool" direction="in" name="async">
     <description>Run asynchronously?</description>
     <value>false</value>
     </param>
</input>
<returns type="casaimage"/>

<description>

\noindent{\bf Summary}

The primary goal of this function is to enable you to analyze a
multi-dimensional image by generating moments of a specified axis.
This is a time-honoured spectral-line analysis technique used for
extracting information about spectral lines. <!-- There is also a
custom GUI interface available via the <link
anchor="images:image.moments.function">momentsgui</link> function as
the command line interface is presently rather cumbersome. -->

You can generate one or more output moment images.  The return value
of this function is an on-the-fly Image \tool\ holding the {\bf first}
of the output moment images.

The word `moment' is used loosely here.  It refers to collapsing an axis
(the moment axis) to one pixel and setting the value of that pixel (for
all of the other non-collapsed axes) to something computed from the data
values along the moment axis.  For example, take an RA-DEC-Velocity
cube, collapse the velocity axis by computing the mean intensity at each
RA-DEC pixel.  This function offers many different moments and a variety
of <!-- interactive and --> automatic methods to compute them. 

We try to make a distinction between a `moment' and a `method'.  This
boundary is a little blurred, but it claims to refer to the distinction
between what you are computing, and how the pixels that were included in
that computation were selected.  For example, a `moment' would be the
average value of some pixel values in a spectrum.  A `method' for
selecting those pixels would be a simple pixel value range specifying
which pixels should be included. 

There are many available moments, and you specify each one with an
integer code as it would get rather cumbersome to refer to them via
strings.  In the list below, the value of the $i$th pixel of the
spectrum is $I_i$, the coordinate of this pixel is $v_i$ (of course it
may not be velocity), and there are $n$ pixels in the spectrum.  The
available moments are:

\begin{itemize}
\item{$-1$} -- the mean value of the spectrum 
\begin{displaymath}
{ {1\over n}  {\sum {I_i}}}
\end{displaymath}
\medskip

\item{0} -- the integrated value of the spectrum 
\begin{displaymath}
M_0 = \Delta v \sum I_i
\end{displaymath}

where $\Delta v$ is the width (in world coordinate units) of a pixel
along the moment axis  
\medskip

\item{1} -- the intensity weighted coordinate (this is
traditionally used to get 'velocity fields')

\begin{displaymath}
M_1 = { {\sum {I_i v_i}} \over {M_0}}
\end{displaymath}
\medskip

\item{2} -- the intensity weighted dispersion of the coordinate 
(this is traditionally used to get 'velocity dispersion fields')

\begin{displaymath}
\sqrt{ { {\sum {I_i \left(v_i - M_1\right)^2}} \over {M_0}}}
\end{displaymath}
\medskip

\item{3} -- the median of $I$
\medskip

\item{4} -- the median coordinate.  Here we treat the spectrum as a
probability distribution, generate the cumulative distribution, and then
find the coordinate corresponding to the 50\% value.  This moment is not
very robust, but it is useful for quickly generating a velocity field in
a way that is not sensitive to noise.  However, it will only give
sensible results under certain conditions.  The generation of the
cumulative distribution and the finding of the 50\% level really only
makes sense if the cumulative distribution is monotonic.  This
essentially means only selecting pixels which are positive or negative. 
For this reason, this moment type is only supported with the basic
method (see below -- i.e.  no smoothing, no windowing, no fitting) with
a pixel selection range that is either all positive, or all negative
\medskip

\item{5} -- the standard deviation about the mean of the spectrum 
\begin{displaymath}
\sqrt{ {1\over {\left(n-1\right)}}  \sum{\left(I_i - \bar{I}\right)^2 }}
\end{displaymath}
\medskip

\item{6} -- the root mean square of the spectrum 
\begin{displaymath}
\sqrt{ {1 \over n} \sum{I_i^2}}
\end{displaymath}
\medskip

\item{7} -- the absolute mean deviation of the spectrum 
\begin{displaymath}
{1 \over n} \sum {|(I_i - \bar{I})|}
\end{displaymath}
\medskip

\item{8} -- the maximum value of the spectrum
\medskip
\item{9} -- the coordinate of the maximum value of the spectrum
\medskip
\item{10} -- the minimum value of the spectrum
\medskip
\item{11} -- the coordinate of the minimum value of the spectrum
\medskip
\end{itemize}

\bigskip
\noindent {Smoothing}

The purpose of the smoothing functionality is purely to provide
a mask.  Thus, you can smooth the input image, apply a pixel
include or exclude range, and generate a smoothed mask which is then
applied before the moments are generated.  The smoothed data
are not used to compute the actual moments; that is always done
from the original data.

\bigskip
\noindent{\bf Basic Method}
 
The basic method is to just compute moments directly from the pixel
values.  This can be modified by applying pixel value inclusion or
exclusion ranges (arguments {\stfaf includepix} and {\stfaf excludepix}).  

You can then also convolve the image (arguments {\stfaf smoothaxes}, {\stfaf
smoothtypes}, and {\stfaf smoothwidths}) and find a mask based on the inclusion
or exclusion ranges applied to the convolved image.  This mask is then
applied to the unsmoothed data for moment computation. 

\bigskip
\noindent{\bf Window Method}

The window method (invoked with argument {\stfaf method='window'}) does
no pixel-value-based selection.  Instead a window is found (hopefully
surrounding the spectral line feature) and only the pixels in that
window are used for computation.  This window can be found from the
convolved or unconvolved image (arguments {\stfaf smoothaxes}, {\stfaf
smoothtypes}, and {\stfaf smoothwidths}).

The moments are always computed from the unconvolved data.  The window
can be found (for each spectrum) <!-- interactively or --> automatically.  The
automatic methods are via Bosma's converging mean algorithm ({\stfaf
method='window'}) or by fitting Gaussians and taking $\pm 3\sigma$ as
the window ({\stfaf method='window,fit'}).  <!-- The interactive methods 
are a direct specification of the window with the cursor ({\stfaf
method='window,interactive'}) and by interactive fitting of Gaussians
and taking $\pm 3$-sigma as the window ({\stfaf
method='window,fit,interactive'}). -->

In Bosma's algorithm, an initial guess for a range of pixels surrounding
a spectral feature is refined by widening until the mean of the pixels
outside of the range converges (to the noise). 

\bigskip
\noindent{\bf Fit Method}

The fit method ({\stfaf method='fit'}) fits Gaussians to spectral
features automatically.  <!-- either automatically or interactively ({\stfaf
method='fit,interactive'}).-->  The moments are then computed from the
Gaussian fits (not the data themselves). 

<!-- The interactive methods are very user intensive.  You have to do
something for each spectrum.  These are really only useful for images
with a manageably small number of spectra. -->

\bigskip
\noindent{\bf Other Arguments}

\begin{itemize} 

\item {\stfaf outfile} - If you are creating just one moment image,
and you specify {\stfaf outfile}, then the image is created
on disk with this name.  If you leave {\stfaf outfile} empty
then a temporary image is created.  In both cases, you can
access this image with the returned Image \tool.  If you are
making more than one moment image, then theses images are always
created on disk.  If you specify {\stfaf outfile} then this is
the root for the output file names.  If you don't specify it,
then the input image name is used as the root.

\item {\stfaf smoothing} - If you smooth the image to generate a
mask, you  specify the kernel widths via the {\stfaf smoothwidths}
argument in the same way as in the
<link anchor="images:image.sepconvolve.function">sepconvolve</link> function.  See it for
details.

<!--
\item {\stfaf plotter} - The plotting is done directly on a PGPLOT
plotting device.  The syntax is {\stfaf plotter=name/type}.  For
example {\stfaf plotter='plot1.ps/ps'} (disk postscript file) or
{\stfaf plotter='plot/glish'} (\glish\ PGplotter).

Note also that if you specify a plotting device but do not ask for an
interactive option, then plots will be made showing you the effect of the
algorithm on each spectrum.  For example, you may have selected the
automatic windowing method.  If you have specified a plotting device,
then each spectrum will be plotted and the window marked on each plot. 
-->

\item {\stfaf stddev} - Some of the automatic methods also require an
estimate of the noise level in the image.  This is used to assess
whether a spectrum is purely noise or not, and whether there is any
signal worth digging out.  If you don't give it via the {\stfaf stddev}
argument, it will be worked out automatically from a Gaussian fit to the
bins above 25\% from a histogram of the entire image.  <!-- If you have
specified the plotting device as well, you get the chance to interact
with this fitting process. -->

\item {\stfaf includepix, excludepix} - The vectors given by arguments
{\stfaf includepix} and {\stfaf excludepix} specify a range of pixel
values for which pixels are either included or excluded.  They are
mutually exclusive; you can specify one or the other, but not both.  If
you only give one value for either of these, say {\stfaf includepix=b},
then this is interpreted as {\stfaf includepix=[-abs(b),abs(b)]}. 

The convolving point-spread function is normalized to have a volume of
unity.  This means that point sources are depressed in value, but
extended sources that are large with respect to the PSF remain
essentially on the same intensity scale; these are the structures you
are trying to find with the convolution so this is what you want. 
If you convolve the image, then arguments like {\stfaf includepix} select
based upon the convolved image pixel values.  If you are having trouble
getting these right, you can output the convolved image ({\stfaf smoothout})
and assess the validity of your pixel ranges.  Note also that if you are
Hanning convolving (usually used on a velocity axis), then the width for
this kernel must be 3 pixels (triangular smoothing kernels of other
widths have no valid theoretical basis). 

\item {\stfaf doppler} - If you compute the moments along a spectral
axis, it is conventional to compute the world coordinate (needed for
moments 0, 1 and 2) along that axis in "km/s".   The argument {\stfaf
doppler} lets you specify what doppler convention the velocity will be
calculated in. You can choose from {\stfaf doppler=radio, optical,
true}.   See function <link anchor="images:image.summary.function">summary</link> for the
definitions of these codes.  For other moment-axis types, the world coordinate
is computed in the native units.

\item {\stfaf mask} - The total input mask is the combination  of the
default \pixelmask\ (if any) and the OTF mask.  Once this mask
has been established, then the moment method may make additional
pixel selections.   

\item {\stfaf drop} - If this is true (the default) then the moment axis
is dropped from the output image.  Otherwise, the output images have  a
moment axis of unit length and coordinate information that is the same
as for the input image.  This coordinate information may be totally
meaningless for the moment images.

\end{itemize}

Finally, if you ask for a moment which requires the coordinate to be
computed for each profile pixel (these are the intensity weighted mean
coordinate [moment 1] and the intensity weighted dispersion of the
coordinate [moment 2]), and the profile axis is not separable then there
will be a performance loss.  Examples of non-separable axes are RA and
Dec.  If the axis is separable (e.g.  a spectral axis) there is no
penalty.  In the latter case, the vector of coordinates for one profile
is the same as the vector for another profile, and it can be precomputed
(once). 

Note that this function has no ``virtual'' output file capability. All
output files are written to disk.   The output mask for these images is
good (T) unless the moment method fails to generate a value (e.g.  the
total input pixel mask was all bad for the profile) in which case it will be bad (F).

<example> 
\begin{verbatim} 
"""
#
print "\t----\t moments Ex 1 \t----"
ia.fromshape(shape=[32,32,32,32]) # replace with your own cube
im2 = ia.moments(moments=[-1,1,2], axis=2, smoothaxes=[0,1,2],
                 smoothtypes=["gauss","gauss","hann"],
                 smoothwidths=[5.0,5.0,3], excludepix=[1e-3],
                 smoothout='smooth', overwrite=true)
im2.done()
ia.close()
#
"""
\end{verbatim} 

In this example, standard moments (average intensity, weighted velocity
and weighted velocity dispersion) are computed via the convolve (spatially
convolved by gaussians and spectrally by a Hanning kernel) and clip
method (we exclude any pixels with absolute value less than $0.001$).
The output file names are automatically created for us and
the convolved image is saved.   The returned image tool holds the first
moment image.

</example>

<example> 
\begin{verbatim} 
"""
#
print "\t----\t moments Ex 2 \t----"
ia.fromshape(shape=[32,32,32,32])
im2 = ia.moments(moments=[3], method=["window"])
im2.done()
ia.close()
#
"""
\end{verbatim} 

In this example, the median of each spectrum is computed, after pixel
selection by the automatic window method.  <!-- Each spectrum and the window
are plotted (25 plots per page) on a PGPLOT X-window device.  Because the
plotting device is given and we did not give the noise level of the
image, a histogram of the image is made and fit and we get to
interactively do the Gaussian fit to the histogram. --> The output 
image is temporary and accessed via the returned Image tool.

</example>
</description>
</method> 

 
 
   <method type="function" name="name">
   <shortdescription>Name of the image file this tool is attached to</shortdescription>
   
<input>
  
     <param type="bool" direction="in" name="strippath">
     <description>Strip off the path before the actual file name?</description>
     <value>false</value>
     </param>
</input>
<returns type="string">String or fail</returns>

<description>

This function returns the name of the \imagefile\ By default, this
function returns the full absolute path of the \imagefile.  You can
strip this path off if you wish with the {\stfaf strippath} argument and
just recover the \imagefile\ name itself. 

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t name Ex 1 \t----"
ia.maketestimage('g1.app', overwrite=true)
print ia.name(strippath=F)
#/casa/code/xmlcasa/implement/images/scripts/g1.app
print ia.name(strippath=T)
#g1.app
ia.close()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="open">
   <shortdescription>Open a new image file with this image tool</shortdescription>
   

<input>
  
     <param type="string" direction="in" name="infile">
     <description>image file name</description>
     </param>
</input>
<returns type="bool">T or fail</returns>
<description>

Use this function when you are finished analyzing the current
\imagefile\ and want to attach to another one.  This function detaches the
\imagetool\ from the current \imagefile, and reattaches it (opens) to
the new \imagefile. 

The input image file may be in native \casa, \fits, or Miriad  
format.  Look \htmlref{here}{IMAGES:FOREIGNIMAGES}  for more
information on foreign images.

<!--
You could, of course, also create a new \imagetool\ and associate that
with the new \imagefile, but this saves you the \tool\ creation. 
-->

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t open Ex 1 \t----"
ia.maketestimage('anotherimage',overwrite=true) #first make 2nd image
ia.close()
ia.maketestimage('myimage',overwrite=true)      #open image myimage
ia.open('anotherimage')               # attach tool to 'anotherimage'
ia.close()
#
"""
\end{verbatim}
The {\stff open} function first closes the old \imagefile.
</example>
</method>

 
   <method type="function" name="pixelvalue">
   <shortdescription>Get value of image and mask at specified pixel coordinate</shortdescription>
   
<input>
  
     <param type="intArray" direction="in" name="pixel">
     <description>Pixel coordinate</description>
     <value>-1</value>
     </param>
</input>
<returns type="any"><any type="record"/>record, unset, or fail</returns>
<description>

This function gets the value of the image and the mask at the specified
pixel coordinate.  The values are returned in a record with fields
'value', 'mask' and 'pixel'.  The value is returned as a quantity, the mask
as a Bool (T is good).  The 'pixel' field holds the actual
pixel coordinate used.

If the specified pixel coordinate is off the image, "\{\}" is returned.

Excessive elements in {\stfaf pixel} are silently discarded.
Missing elements are given the (nearest integer) value of the reference pixel.
This is reflected in the output record 'pixel' field.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t pixelvalue Ex 1 \t----"
ia.maketestimage();
ia.pixelvalue()
#{'mask': True,
# 'pixel': array([55, 37]),
# 'value': {'unit': 'Jy/beam', 'value': 2.5064315795898438}}
print ia.pixelvalue([-1,-1])
# {}
print ia.pixelvalue([9])
#{'mask': True,
# 'pixel': array([ 9, 37]),
# 'value': {'unit': 'Jy/beam', 'value': 0.14012207090854645}}
print ia.pixelvalue([9,9,9])
#{'mask': True,
# 'pixel': array([9, 9]),
# 'value': {'unit': 'Jy/beam', 'value': -0.45252728462219238}}
ia.close()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="putchunk">
   <shortdescription>Put pixels from an array into a regular region of the image</shortdescription>
   
<input>
  
     <param type="any" direction="in" name="pixels">
	     <any type="variant"/>
     <description>Numeric array.  Required input.</description>
     </param>
  
     <param type="intArray" direction="in" name="blc">
     <description>Bottom-Left-Corner (start) of location in image.
     Default is start of image.</description>
     <value>-1</value>
     </param>
  
     <param type="intArray" direction="in" name="inc">
     <description>increment (stride) along axes</description>
     <value>1</value>
     </param>
  
     <param type="bool" direction="in" name="list">
     <description>List bounding box to logger?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="locking">
     <description>Unlock image after use?</description>
     <value>true</value>
     </param>
  
     <param type="bool" direction="in" name="replicate">
     <description>Replicate array through image</description>
     <value>false</value>
     </param>
</input>
<returns type="bool">T or fail</returns>
<description>

This function puts an array into the \imagefile.  If there is a
default \pixelmask\ it is ignored in this process.  It is the complement of the
<link anchor="images:image.getchunk.function">getchunk</link> function.  You can specify the {\stfaf
blc} and {\stfaf inc} if desired.  If they are unspecified, they default
to the beginning of the image and an increment of one. 

Any illegal {\stfaf blc} values are set to zero.  Any illegal {\stfaf
inc} values are set to unity. 

An error will result if you attempt to put an array beyond the extent of
the image (i.e., it is not truncated or decimated). 

If there are fewer axes in the array than in the image, the array is
assumed to have trailing axes of length unity.  Thus, if you have a 2D
array and want to put it in as the YZ plane rather than the XY plane,
you must ensure that the shape of the array is [1,nx,ny].  

However, the argument {\stfaf replicate} can be used to replicate the array
throughout the image (from the blc to the trc).  For example, if you
provide a 2D array to a 3D image, you can replicate it through the third
axis by setting {\stfaf replicate=T}.   The replication is done
from the specified {\stfaf blc} to the end of the image.
Use function <link anchor="images:image.putregion.function">putregion</link>  if you
want to terminate the replication at a {\stfaf trc} value.

The argument {\stfaf locking} controls two things.  If True, then
after the function is called, the image is unlocked (so some other
process can acquire a lock) and it is indicated that the image has
changed. <!-- (causes function <link
anchor="images:image.view.function">view</link> to redisplay the image
if it is has been called).--> The reason for having this argument is
that the unlocking and updating processes are quite expensive.  If you
are repeatedly calling {\stff putchunk} in a for loop, you would be
advised to use this switch.

A related function is <link anchor="images:image.putregion.function">putregion</link> 
which puts the pixels and masks into a more complex \region.
Function {\stff putchunk} is retained because it is faster and therefore
preferable for repeated operation in loops if the \pixelmask\ is not required.

See also the functions <link anchor="images:image.set.function">set</link> and 
<link anchor="images:image.calc.function">calc</link> which can also change pixel values.

</description>

<example>

We can clip all pixels to be {\tt <= } 5 as follows.
\begin{verbatim}
"""
#
print "\t----\t putchunk Ex 1 \t----"
ia.fromshape(shape=[10,10])   # create an example image
pix = ia.getchunk()           # get pixels to modify from example image
for i in range(len(pix)):
  pix[i] = list(pix[i])       # convert tuple to list so it can be modified
  for j in range(len(pix[i])):
    pix[i][j] = i*10 + j
  pix[i] = tuple(pix[i])      # convert list back to tuple
ia.putchunk(pix)              # put pixels back into example image
print pix                     # pixels have values 0-99
pix2 = ia.getchunk()          # get all pixels into an array (again)
for i in range(len(pix2)):
  pix2[i] = list(pix2[i])     # convert tuple to list so it can be modified
  for j in range(len(pix2[i])):
    if pix2[i][j] > 5:
      pix2[i][j] = 5          # clip values to 5
  pix2[i] = tuple(pix2[i])    # convert list back to tuple
ia.putchunk(pix2)             # put array back into image
print ia.getchunk()
ia.close()
#
"""
\end{verbatim}

The above example shows how you could clip an image to a value.  If
all the pixels didn't easily fit in memory, you would iterate through
the image chunk by chunk to avoid exhausting virtual memory.  Better
would be to do this via LEL through function <link
anchor="images:image.calc.function">calc</link>.

Suppose we wanted to set the fifth XY plane to 1.

We could do so as follows:
\begin{verbatim}
"""
#
print "\t----\t putchunk Ex 2 \t----"
ia.fromshape(shape=[10,10,10])
imshape = ia.shape()
pix = ia.makearray(1, [imshape[0],imshape[1]])
ia.putchunk(pix, blc=[0,0,4])
print ia.getchunk()[0:3]
ia.close()
#
"""
\end{verbatim}

Suppose we wanted to set the first YZ plane to 2.

\begin{verbatim}
"""
#
print "\t----\t putchunk Ex 3 \t----"
ia.fromshape(shape=[10,10,10])
imshape = ia.shape()
pix = ia.makearray(2, [1,imshape[1],imshape[2]])
ia.putchunk(pix)
print ia.getchunk()[0:3]
ia.close()
#
"""
\end{verbatim}

</example>
</method>

 
   <method type="function" name="putregion">
   <shortdescription>Put pixels and mask into a  region-of-interest of the image</shortdescription>
   
<input>
  
     <param type="any" direction="in" name="pixels">
	     <any type="variant"/>
     <description>The pixel values.  Default is none.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="pixelmask">
	     <any type="variant"/>
     <description>The pixel mask values.  Default is none.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="region">
	     <any type="record"/>
     <description>The region of interest.  Default is whole image.</description>
     <value></value>
     </param>
  
     <param type="bool" direction="in" name="list">
     <description>List the bounding box and any mask creation to the logger</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="usemask">
     <description>Honour the mask when putting pixels</description>
     <value>true</value>
     </param>
  
     <param type="bool" direction="in" name="locking">
     <description>Unlock image after use?</description>
     <value>true</value>
     </param>
  
     <param type="bool" direction="in" name="replicate">
     <description>Replicate array through image</description>
     <value>false</value>
     </param>
</input>
<returns type="bool">T or fail</returns>
<description>

This function replaces data and/or \pixelmask\ values in the image in
the specified \region.  The {\stfaf pixels} and/or {\stfaf pixelmask}
arrays must be the shape of the bounding box, and the whole bounding box
is replaced in the image.  The \region\ is really only used to specify
the bounding box.  If the region extends beyond the image, it is
truncated.  If the {\stfaf pixels} or {\stfaf pixelmask} array shapes do not
match the bounding box, an error will result. 

When you put a \pixelmask, it either replaces the current default \pixelmask, or
is created.  The \pixelmask\ is put before the pixels.

The argument {\stfaf usemask} is only relevant when you are putting
pixel values and there is a \pixelmask\ (meaning also the one you might have
just put in place).  If {\stfaf usemask=T} then only pixels for which
the mask is good (T) are altered.  If {\stfaf usemask=F} then all the
pixels in the region are altered - the mask is ignored. 

The argument {\stfaf replicate} can be used to replicate the array
throughout the image (from the blc to the trc).  For example, if you
provide a 2D array to a 3D image, you can replicate it through the third
axis by setting {\stfaf replicate=T}.   The replication
is done in the specified {\stfaf region}.

The argument {\stfaf locking} controls two things.  If True, then
after the function is called, the image is unlocked (so some other
process can acquire a lock) and it is indicated that the image has
changed. <!-- (causes function <link
anchor="images:image.view.function">view</link> to redisplay the image
if it is has been called).--> The reason for having this argument is
that the unlocking and updating processes are quite expensive.  If you
are repeatedly calling {\stff putregion} in a for loop, you would be
advised to use this switch (and to consider using {\stff putchunk}).

See the related functions <link
anchor="images:image.putchunk.function">putchunk</link>, <link
anchor="images:image.set.function">set</link> and <link
anchor="images:image.calc.function">calc</link>.

</description>

<example>

Suppose that we have a 2-dimensional image.  First we recover the pixel
and \pixelmask\ values from a polygonal region.  Then we change the values in
the array that are within the region to zero and replace the data. 

\begin{verbatim}
"""
#
print "\t----\t putregion Ex 1 \t----"
ia.maketestimage()                         # Attach an image to image tool
x = ['3pix','6pix','9pix','6pix','5pix','5pix','3pix'] # X vector abs pixels
y = ['3pix','4pix','7pix','9pix','7pix','5pix','3pix'] # Y vector abs pixels
mycs = ia.coordsys()
r1 = rg.wpolygon(x,y,csys=mycs.torecord()) # Create polygonal world region
mycs.done()
pixels = ia.getregion(r1)                  # Recover pixels
pixelmask = ia.getregion(r1, getmask=T)    # and mask
for i in range(len(pixels)):
  pixels[i] = list(pixels[i])              # convert tuple to list for mods
  for j in range(len(pixels[i])):
    if pixelmask[i][j]:
      pixels[i][j] = 0                     # Set pixels where mask is T to zero
  pixels[i] = tuple(pixels[i])             # convert list back to tuple
ia.putregion(pixels=pixels, pixelmask=pixelmask,
             region=r1)                    # Replace pixels only
ia.close()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="rebin">
   <shortdescription>rebin the image by the specified factors</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="intArray" direction="in" name="bin">
     <description>Binning factors for each axis</description>
     </param>
  
     <param type="any" direction="in" name="region">
	     <any type="record"/>
     <description>The region of interest.  Default is whole image.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="mask">
	     <any type="variant"/>
     <description>OTF mask, Boolean LEL expression or mask region.  Default is none.</description>
     <value></value>
     </param>
  
     <param type="bool" direction="in" name="dropdeg">
     <description>Drop degenerate axes</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="overwrite">
     <description>Overwrite (unprompted) pre-existing output file?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="async">
     <description>Run asynchronously?</description>
     <value>false</value>
     </param>
</input>
<returns type="casaimage"/>

<description>

This function rebins the current image by the specified integer binning
factors for each axis.  The output pixel value is just the average of the
input pixel values. The output pixel will be masked bad if there
were no good input pixels.  You cannot rebin a Stokes axis.

If {\stfaf outfile} is given, the image is written to the specified
disk file.  If {\stfaf outfile} is unset, the Image \tool\ is
associated with a temporary image.  This temporary image may be in
memory or on disk, depending on its size.  When you destroy the
on-the-fly Image \tool\ returned by this function (with the <link
anchor="images:image.done.function">done</link> function) this
temporary image is deleted.

Sometimes it is useful to drop axes of length one (degenerate axes). 
Use the {\stfaf dropdeg} argument if you want to do this.  It will
discard the axes from the input image.  Therefore the output shape and
Coordinate System that you supply must be consistent with the input
image after the degenerate axes are dropped. 

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t rebin Ex 1 \t----"
ia.maketestimage();
im2 = ia.rebin(bin=[2,3]);
im2.done()
ia.close()
#
"""
\end{verbatim}
</example>

</method>

 
   <method type="function" name="regrid">
   <shortdescription>regrid this image to the specified Coordinate System</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="intArray" direction="in" name="shape">
     <description>Shape of output image.  Default is input shape.</description>
     <value>-1</value>
     </param>
  
     <param type="any" direction="in" name="csys">
	     <any type="record"/>
     <description>Coordinate System for output image.  Default is input image coordinate system.</description>
     <value></value>
     </param>
  
     <param type="intArray" direction="in" name="axes">
     <description>The output pixel axes to regrid.  Default is all.</description>
     <value>-1</value>
     </param>
  
     <param type="any" direction="in" name="region">
	     <any type="record"/>
     <description>The region of interest.  Default is the whole image.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="mask">
	     <any type="variant"/>
	     <description>OTF mask, Boolean LEL expression or mask region.
      Default is none.</description>
     <value></value>
     </param>
  
     <param type="string" direction="in" name="method">
     <description>The interpolation method.  String from 'nearest', 'linear', 'cubic'.</description>
     <value>linear</value>
     </param>
  
     <param type="int" direction="in" name="decimate">
     <description>Decimation factor for coordinate grid computation</description>
     <value>10</value>
     </param>
  
     <param type="bool" direction="in" name="replicate">
     <description>Replicate image rather than regrid?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="doref">
     <description>Turn on reference frame changes</description>
     <value>true</value>
     </param>
  
     <param type="bool" direction="in" name="dropdeg">
     <description>Drop degenerate axes</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="overwrite">
     <description>Overwrite (unprompted) pre-existing output file?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="force">
     <description>Force specified axes to be regridded</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="async">
     <description>Run asynchronously?</description>
     <value>false</value>
     </param>

</input>
<returns type="casaimage"/>

<description>

This function regrids the current image onto a grid specified by the
given Coordinate System.   You can also specify the shape of the
output image.

The Coordinate System must be given via a <link
anchor="images:coordsys">Coordsys</link> \tool\ (using
coordsys.torecord()).  It is optional; if not specified, the Coordinate
System from the input image (i.e.  the one to which you are applying
the regrid function) is taken.  The order of the coordinates and axes
in the output image is always the same as the input image.  It simply
'finds' the relevant coordinate in the supplied Coordinate System in
order to figure out the regridding parameters.  The supplied
Coordinate System must have at least as many coordinates as are
required to accomodate the axes you are regridding (e.g.  if you
regrid the first two axes, and these belong to a Direction Coordinate,
you need one Direction Coordinate in the supplied Coordinate System).
Coordinates pertaining to axes that are not being regridded are
supplied from the input image, not the given Coordinate System.

Reference changes are handled (e.g.  J2000 to B1950, LSR to TOPO).  In
general, the conversion machinery attempts to work out how sophisticated
it needs to be (e.g.  am I regridding LSR to LSR or LSR to TOPO). 
However, it errs on the side of conservatism so that it can be that the
conversion machine requires more information than it actually needs. 
For full frame conversions, one needs to know things like location on
earth (e.g.  observatory), direction of observation, and time of
observation.  

If you get the above errors and you {\bf are} doing a frame conversion,
then that means you must insert some extra information into the
Coordinate System of your image.  Most likely it's the time
(<link anchor="images:coordsys.setepoch.function">coordsys.setepoch</link>) and location
(<link anchor="images:coordsys.settelescope.function">coordsys.settelescope</link>) that are
missing. If you get these errors and you {\bf know} that you are not
specifying a frame change (e.g.  regrid LSR to LSR) then try setting
{\stfaf doref=F}.  This will (silently) bypass all possible frame
conversions.  Note that if you {\bf are} requesting a frame conversion
and you set {\stfaf doref=F} you are doing a bad thing (and you will
get no warnings).

If you regrid a plane holding a Direction Coordinate and the units are
Jy/pixel then the output is scaled to conserve flux (roughly; just one
scale factor at the reference pixel is computed). 

A variety of interpolation schemes are provided (you need only specify
the first three characters to {\stfaf method}).  The cubic interpolation
is substantially slower than linear, and often the improvement is
modest.  By default you get linear interpolation. 

You specify the shape of the output image ({\stfaf shape}) and which
output axes you want to regrid ({\stfaf axes}).  Note that a Stokes axis
cannot be regridded (you will get a warning if you try).

The {\stfaf axes} argument cannot be used to discard axes from the
output image; it can only be used to specify which {\bf output} axes are
going to be regridded and which are not.  Any axis that you are not
regridding must have the same output shape as the input image shape for
that axis. 

The {\stfaf axes} argument can also be used to specify the order in
which the {\bf output} axes are regridded.  This may give you
significant performance benefits.  For example, imagine we are going to
regrid a spectral-line cube of shape [512,512,1204] to shape
[256,256,32].  If you specified {\stfaf axes=[0,1,2]} then first, the
Direction axes would be regridded for each of the 1024 pixels (and
stored in a temporary image).  Then each profile at each spatial
location in the temporary image would be regridded to 32 pixels.  You
could speed this process up significantly by setting {\stfaf
axes=[2,0,1]}.  In this case, first each profile would be regridded to
32 pixels, and then each plane of the 32 pixels would be regridded. 
Note that the order of {\stfaf axes} does not affect the order of the
{\stfaf shape} argument.  I.e.  it should be given in the natural pixel
axis order of the image {\stfaf [256,256,32]} in both cases. 

You can also specify a \region\ to be applied to the input image.  If
you do this, you need to be careful with the output shape for
non-regridded axes (must match that of the region - use function
<link anchor="images:image.boundingbox.function">boundingbox</link> to find that out). 

If {\stfaf outfile} is given, the image is written to the specified
disk file.  If {\stfaf outfile} is unset, the on-the-fly Image \tool\
returned by this function is associated with a temporary image.  This
temporary image may be in memory or on disk, depending on its size.
When you destroy the on-the-fly Image \tool\ (with the <link
anchor="images:image.done.function">done</link> function) this
temporary image is deleted.

The argument {\stfaf replicate} can be used to simply replicate pixels
rather than regridding them.  Normally ({\stfaf replicate=F}), for every
output pixel, its world coordinate is computed and the corresponding
input pixel found (then a little interpolation grid is generated).  If
you set {\stfaf replicate=T}, then what happens is that for every output
axis, a vector of regularly sampled input pixels is generated (based on
the ratio of the output and input axis shapes).  So this just means the
pixels get replicated (by whatever interpolation scheme you use) rather
than regridded in world coordinate space.  This process is much faster,
but its not a true world coordinate based regrid. 

As decribed above, when {\stfaf replicate} is False, a coordinate is
computed for each output pixel; this is an expensive operation.  The
argument {\stfaf decimate} allows you to decimate the computation of
that coordinate grid to a sparse grid, which is then filled in via fast
interpolation.  The default for {\stfaf decimate} is 10.  The number of
pixels per axis in the sparse grid is the number of output pixels for
that axis divided by the decimation factor.  A factor of 10 does pretty
well.  You may find that for very non-linear coordinate systems (e.g. 
very close to the pole) that you have to reduce the decimation factor. 

Sometimes it is useful to drop axes of length one (degenerate axes). 
Use the {\stfaf dropdeg} argument if you want to do this.  It will
discard the axes from the input image.  Therefore the output shape and
Coordinate System that you supply must be consistent with the input
image after the degenerate axes are dropped. 

Argument {\stfaf force} can be used to force all specified axes to be
regridded, even if the algorithm determines that they don't need to be (because
the input and output coordinate information is identical).

There is a useful function
<link anchor="images:coordsys.setreferencelocation.function">setreferencelocation</link> that
you can use to keep a specific world coordinate in the center of an
image when regridding (see example below). 

The output \pixelmask\ will be good (T) unless the regridding failed to
find a value for that output pixel in which case it will be bad (F).
For example, if the total input mask (default input \pixelmask\ plus OTF
mask) for all of the relevant input pixels were masked bad 
then the output pixel would be masked bad (F).

{\bf Multiple axis Coordinates limitation} -- Some cooordinates pertain
to more than one axis.  E.g.  a Direction Coordinate holds longitude and
latitude.  A Linear Coordinate can also hold many axes.  When you regrid
*any* axis from a Coordinate which holds multiple axes, you must fully
specify the coordinate information for all axes in that Coordinate in
the Coordinate System that you provide.  For example, you have a Linear
Coordinate with two axes and you want to regrid axis one only.  In the
Coordinate System you provide, the coordinate information for axis two
(not being regridded) must correctly be a copy from the input coordinate
system (it won't be filled in for you). 

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t regrid Ex 1 \t----"
ia.maketestimage('radio.image', overwrite=true)  
ia.maketestimage('optical.image', overwrite=true)
mycs = ia.coordsys();     # get optical image co-ordinate system
ia.open('radio.image')
imrr = ia.regrid(outfile='radio.regridded', csys=mycs.torecord(),
                  shape=ia.shape(), overwrite=true)
#viewer()
mycs.done()
imrr.done()
ia.close()
#
"""
\end{verbatim}

In this example, we regrid a radio image onto the grid of an optical
image - this probably (if the optical FITS image was correctly labelled
!!) will involve a projection change (optical images are usually TAN
projection, radio usually SIN).  
</example>

<example>
\begin{verbatim}
"""
#
print "\t----\t regrid Ex 2 \t----"
ia.maketestimage('radio.image',overwrite=true)  
mycs = ia.coordsys();
print mycs.referencecode('dir')
#J2000
mycs.setreferencecode(value='B1950', type='dir', adjust=T)
im3 = ia.regrid(outfile='radio.regridded', csys=mycs.torecord(),
                shape=ia.shape(), overwrite=true)
mycs.done()
im3.done()
ia.close()
#
"""
\end{verbatim}

In this example, we regrid a radio image from J2000 to B1950. This is
accomplished by first recovering the Coordinate System into a
<link anchor="images:coordsys">Coordsys</link> tool, manipulating the reference code
with that \tool, and then supplying the new Coordinate System to the
regrid function.
</example>

<example>
\begin{verbatim}
"""
#
print "\t----\t regrid Ex 3 \t----"
ia.maketestimage('zz', overwrite=true)  
mycs = ia.coordsys();
p = ia.shape()
for i in range(len(p)):
  p[i] = p[i]/2.0 + 10
refval = ia.toworld(value=p, format='n') # Location of interest
inc = mycs.increment()
incx = inc['numeric']
for i in range(len(incx)):
  incx[i] = incx[i]/2.0                  # Halve increment
inc['numeric']=incx
mycs.setincrement(value=inc)             # Set increment
shp = ia.shape()
refpix=refval['numeric'][:]
refpix=list(refpix)                      # numpy makes this necessary
for i in range(len(shp)):
  shp[i] = shp[i] *2                     # Double shape
  refpix[i] = int((shp[i]-1)/2.0 + 1);   # New ref pix
# Center image on location of interest
mycs.setreferencelocation(pixel=refpix, world=refval)
imr = ia.regrid(csys=mycs.torecord(), shape=shp, overwrite=true)# Regrid
mycs.done()
imr.done()
ia.close()
#
"""
\end{verbatim}
</example>

</method>

 
   <method type="function" name="rotate">
   <shortdescription>rotate the direction coordinate axes attached to the image and regrid the image to the rotated Coordinate System</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="intArray" direction="in" name="shape">
     <description>Shape of output image.  Default is shape of input image.</description>
     <value>-1</value>
     </param>
  
     <param type="any" direction="in" name="pa">
     <any type="variant"/>
     <description>Angle by which to rotate.  Default is no rotation.</description>
     <value>0deg</value>
     </param>
  
     <param type="any" direction="in" name="region">
     <any type="record"/>
     <description>The region of interest.  Default is whole image.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="mask">
     <any type="variant"/>
     <description>OTF mask, Boolean LEL expression or mask region.  Default is none.</description>
     <value></value>
     </param>
  
     <param type="string" direction="in" name="method">
     <description>The interpolation method.  String from 'nearest',
      'linear', or 'cubic'.</description>
     <value>cubic</value>
     </param>
  
     <param type="int" direction="in" name="decimate">
     <description>Decimation factor for coordinate grid computation</description>
     <value>0</value>
     </param>
  
     <param type="bool" direction="in" name="replicate">
     <description>Replicate image rather than regrid?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="dropdeg">
     <description>Drop degenerate axes</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="overwrite">
     <description>Overwrite (unprompted) pre-existing output file?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="async">
     <description>Run asynchronously?</description>
     <value>false</value>
     </param>

</input>
<returns type="casaimage"/>

<description>

This function rotates two axes of an image.  These axes are either
those associated with a Direction coordinate or with a Linear 
coordinate. The Direction coordinate takes precedence.
If rotating a Linear coordinate, it must hold precisely two axes.

The method is that the Coordinate is rotated and then the input
image is regridded to the rotated Coordinate System.

If the image brightness units are Jy/pixel then the output is scaled to
conserve flux (roughly; just one scale factor at the reference pixel is
computed).

A variety of interpolation schemes are provided (you need only specify
the first three characters to {\stfaf method}).  The cubic
interpolation is substantially slower than linear.  By default you get
cubic interpolation.

You can specify the shape of the output image ({\stfaf shape}).
However, all axis that are not regrided retain the same output shape
as the input image shape for that axis.  Only the direction coordinate
axes are regridded.

You can also specify a \region\ to be applied to the input image.  If
you do this, you need to be careful with the output shape for
non-regridded axes (must match that of the region - use function
<link anchor="images:image.boundingbox.function">boundingbox</link> to find that out).

If {\stfaf outfile} is given, the image is written to the specified
disk file.  If {\stfaf outfile} is unset, the on-the-fly Image \tool\
returned by this function is associated with a temporary image.  This
temporary image may be in memory or on disk, depending on its size.
When you destroy the on-the-fly Image \tool\ (with the <link
anchor="images:image.done.function">done</link> function) this
temporary image is deleted.

The argument {\stfaf replicate} can be used to simply replicate pixels
rather than regridding them.  Normally ({\stfaf replicate=F}), for every
output pixel, its world coordinate is computed and the corresponding
input pixel found (then a little interpolation grid is generated).  If
you set {\stfaf replicate=T}, then what happens is that for every output
axis, a vector of regularly sampled input pixels is generated (based on
the ratio of the output and input axis shapes).  So this just means the
pixels get replicated (by whatever interpolation scheme you use) rather
than regridded in world coordinate space.  This process is much faster,
but its not a true world coordinate based regrid. 

As decribed above, when {\stfaf replicate} is False, a coordinate is
computed for each output pixel; this is an expensive operation.  The
argument {\stfaf decimate} allows you to decimate the computation of
that coordinate grid to a sparse grid, which is then filled in via
fast interpolation.  The default for {\stfaf decimate} is 0 (no
decimation).  The number of pixels per axis in the sparse grid is the
number of output pixels for that axis divided by the decimation
factor.  A factor of 10 does pretty well.  You may find that for very
non-linear coordinate systems (e.g.  very close to the pole) that you
have to reduce the decimation factor.

The output \pixelmask\ will be good (T) unless the regridding failed to
find a value for that output pixel in which case it will be bad (F).
For example, if the total input mask (default input \pixelmask\ plus OTF
mask) for all of the relevant input pixels were masked bad 
then the output pixel would be masked bad (F).

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t rotate Ex 1 \t----"
ia.maketestimage();
imr=ia.rotate(pa='45deg');
imr.done()
ia.close()
#
"""
\end{verbatim}

In this example, we rotate the direction coordinate axes (RA/Dec) of a
test image by 45 degress and regrid the image onto the axes.  <!-- The viewer
displays the before and after images.
#imr.view(axislabels=T);-->
</example>

</method>

 
   <method type="function" name="rename">
   <shortdescription>Rename the image file associated with this image tool</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="name">
     <description>The new image file name</description>
     </param>
  
     <param type="bool" direction="in" name="overwrite">
     <description>Overwrite target file if it already exists</description>
     <value>false</value>
     </param>
</input>
<returns type="bool">T or fail</returns>
<description>

This function renames the \imagefile\ associated with the \imagetool. 
If a file with name {\stfaf name} already exists, you can overwrite it
with the argument {\stfaf overwrite}; otherwise a fail will
result. 

</description>
<example>
\begin{verbatim}
"""
#
print "\t----\t rename Ex 1 \t----"
ia.maketestimage('myimage',overwrite=T)  
print ia.name(strippath=T)
#myimage
ia.rename('newimage', overwrite=T)
print ia.name(strippath=T)
#newimage
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="replacemaskedpixels">
   <shortdescription>replace the values of pixels which are masked bad</shortdescription>
   
<input>
  
     <param type="any" direction="in" name="pixels">
	     <any type="variant"/>
     <description>The new value(s), Numeric scalar or LEL expression</description>
     </param>
  
     <param type="any" direction="in" name="region">
	     <any type="record"/>
     <description>The region of interest.  Default is whole image.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="mask">
	     <any type="variant"/>
     <description>OTF mask, Boolean LEL expression or mask region.  Default is none.</description>
     <value></value>
     </param>
  
     <param type="bool" direction="in" name="update">
     <description>Update mask as well?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="list">
     <description>List the bounding box to the logger</description>
     <value>false</value>
     </param>
</input>
<returns type="bool">T or fail</returns>
<description>

This function <!-- (short-hand name {\stff rmp}) --> replaces the values of all
pixels whose total input mask (default input \pixelmask\ and OTF mask)
is bad (F) with the specified value.    

If the argument {\stfaf update} is F (the default), the actual \pixelmask\
is left unchanged.  That is, masked pixels remain masked.   However, if
you set {\stfaf update=T} then the \pixelmask\ will be updated so that the
\pixelmask\ will now be T (good) where the {\bf total} input mask was F
(bad).

See <link anchor="images:image.maskhandler.function">maskhandler</link> for information
on how to set the default \pixelmask.

There are a few ways in which you can specify what to replace the
masked pixel values by.

\begin{itemize}

\item First, you can give the {\stfaf pixels} argument a simple numeric
scalar (e.g.  {\cf pixels=1.0}).  Then, all masked values will be
replaced by that value. 

\item Second, you can give a scalar
\htmladdnormallink{LEL}{../../notes/223/223.html} expression string
(e.g.  {\cf pixels='min(myimage)'}).  Then, all masked values will be
replaced by the scalar that results from the expression.  If the scalar expression
is illegal (e.g.  in the expression {\cf pixels='min(myimage)'} there
were no good pixels in {\sff myimage}) then the value 0 is used for
replacement. 

\item Third, you can give a
\htmladdnormallink{LEL}{../../notes/223/223.html} expression string
which has the same shape as the \imagefile\ you are applying the
function to.  For example, putting {\cf pixels='myotherimage'} means
replace all masked pixels in this \imagefile\ with the equivalent pixel
in the \imagefile\ called {\sff myotherimage}. 

Your expression might be quite complex, and you can think of it as
producing another masked lattice.  However, in the replace process, the
mask of that expression lattice is ignored.  Thus, only the mask of
the \imagefile\ you are replacing and the pixel values of the expression
lattice are relevant. 

The expression must conform with the subimage formed by applying the
\region\ to the image (i.e.  that associated with this Image \tool).  If
you use the {\stfaf mask} argument as well, the \region\ is applied to
it as well (see examples). 

\end{itemize}
</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t replacemaskedpixels Ex 1 \t----"
ia.maketestimage('zz1',overwrite=true)
ia.calcmask('zz1<0')
ia.replacemaskedpixels(0.0)
ia.replacemaskedpixels('min(zz1)')
ia.close()
#
"""
\end{verbatim}

These examples replace all masked pixels by the specified scalar.  In
the second case, the scalar comes from a LEL expression operating on
{\sff zz1} (or it could be from an LEL expression operating on some
other image).

</example>

<example>
\begin{verbatim}
"""
#
print "\t----\t replacemaskedpixels Ex 2 \t----"
ia.maketestimage('zz2',overwrite=true)
ia.close()
ia.maketestimage('zz1',overwrite=true)  
#ia.calcmask('zz1<0')
ia.replacemaskedpixels(0.0, mask='zz2>0')
ia.close()
#
"""
\end{verbatim}

Let us say that {\sff zz1} has no mask.  By using the {\stfaf mask}
argument, we generate a transient mask which is T (good) when the pixel
values are positive.  This means that all non-positive values (when that
mask is F [bad]) will be replaced with the value 0.  If {\sff zz1} did
have a mask it would be applied as well as the transient mask (the masks
would be logically ORed). 

</example>

<example>
\begin{verbatim}
"""
#
print "\t----\t replacemaskedpixels Ex 3 \t----"
ia.maketestimage('zz1',overwrite=true)  
ia.calcmask('zz1<0')
im2 = ia.subimage(outfile='zz2',overwrite=true)
# r = rg.quarter()
r=ia.setboxregion([0.25,0.25],[0.75,0.75],frac=true)
ia.replacemaskedpixels(0.0, region=r, mask=im2.name(strippath=T)+'>0')
# same as ia.replacemaskedpixels(0.0, region=r, mask='zz2>0')
im2.done()
ia.close()
#
"""
\end{verbatim}

The specified region takes one quarter of the image by area centered on
the image center.  The region is applied to the {\stfaf mask} expression
as well - this means that any images in the {\stfaf mask} expression
must conform with the {\sff zz1} image.   The replacement of the
scalar is then done only within that region.  Note that in
the {\stfaf mask} expression we have specified the image with
the Image tool {\stf im2} via im2.name() (rather than referring
to its disk file name {\sff zz2}).

</example>

<example>
\begin{verbatim}

#
print "\----\t replacemaskedpixels Ex 4 \t----"
ia.maketestimage('zz3',overwrite=true)
ia.maketestimage('zz2',overwrite=true)
ia.maketestimage('zz1',overwrite=true)
ia.calcmask('zz1<0')
ia.replacemaskedpixels('zz2+zz3')
ia.close()
#

\end{verbatim}

In this example, the replacement values are taken
from a LEL expression adding two other images
together.  The expression must conform with the
image {\sff zz1}.
</example>

<!-- 
<example>
\begin{verbatim}
"""
#
print "\t\t replacemaskedpixels Ex 5 \t"
ia.maketestimage('zz3',overwrite=true)  
ia.maketestimage('zz2',overwrite=true)  
ia.maketestimage('zz1',overwrite=true)  
ia.calcmask('zz1<0')
#r = drm.quarter()
r=ia.setboxregion([0.25,0.25],[0.75,0.75],frac=true)
#ia.replacemaskedpixels('zz2[$r]+zz3[$r]', region=r)
ia.replacemaskedpixels('zz2+zz3')
ia.close()
#
"""
\end{verbatim}

In this example, the replacement values are taken
from a LEL expression adding two other images
together.  Because expression must conform with the
image {\sff zz1} after application of the region,
we must specify the region in the expression as well.

</example>
-->

</method>

 
   <method type="function" name="restoringbeam">
   <shortdescription>Get the restoringbeam</shortdescription>
   
   <returns type="any"><any type="record"/>record or fail</returns>
<description>

This function <!-- (short-hand name {\stff rb}) --> gets the restoring beam, if any.
It is returned in a record with fields 'major', 'minor' and
'postionangle'.   Each of these fields contains a quantity. If there
is no restoring beam, this function returds an empty record.

You can set the restoring beam with function 
<link anchor="images:image.setrestoringbeam.function">setrestoringbeam</link>.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t restoringbeam Ex 1 \t----"
ia.maketestimage()      
print ia.restoringbeam()
#{'major': {'unit': 'arcsec', 'value': 53.500004857778549},
# 'minor': {'unit': 'arcsec', 'value': 34.199998900294304},
# 'positionangle': {'unit': 'deg', 'value': 6.0}}
ia.close()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="sepconvolve">
   <shortdescription>Separable convolution</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="intArray" direction="in" name="axes">
     <description>Axes to convolve.  Default is [0,1,...].</description>
     <value>-1</value>
     </param>
  
     <param type="stringArray" direction="in" name="types">
     <description>Type of convolution kernel.
     Vector of strings from 'boxcar', 'gaussian', and 'hanning'.
     Default is appropriately sized vector of 'gaussian'.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="widths">
     <any type="variant"/>
     <description>Convolution kernel widths, Vector of numeric, quantity or string</description>
     <value></value>
     </param>
  
     <param type="double" direction="in" name="scale">
     <description>Scale factor.  Default is autoscale.</description>
     <value>-1</value>
     </param>
  
     <param type="any" direction="in" name="region">
	     <any type="record"/>
     <description>Region of interest.  Default is whole image.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="mask">
	     <any type="variant"/>
	     <description>OTF mask, Boolean LEL expression or mask region.
       Default is none.</description>
     <value></value>
     </param>
  
     <param type="bool" direction="in" name="overwrite">
     <description>Overwrite (unprompted) pre-existing output file?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="async">
     <description>Run asynchronously?</description>
     <value>false</value>
     </param>
</input>
<returns type="casaimage"/>

<description>

This function <!-- (short-hand name {\stff sc}) --> does Fourier-based 
convolution of an \imagefile\ by a specified separable kernel.  

If {\stfaf outfile} is given, the image is written to the specified
disk file.  If {\stfaf outfile} is unset, the on-the-fly Image \tool\
returned by this function is associated with a temporary image.  This
temporary image may be in memory or on disk, depending on its size.
When you destroy the Image \tool\ (with the <link
anchor="images:image.done.function">done</link> function) this
temporary image is deleted.

You specify which axes of the image you wish to convolve, by what kernel
of what width. The kernel types can be shortened to {\cf `gauss',
`hann'} and {\cf `box'}. 

You specify the widths of the convolution kernels via the argument
{\stfaf widths}.  The values can be specified as a vector of three
different types.

\begin{itemize}

\item Quantity - for example {\stfaf widths=qa.quantity("1arcsec 0.00001rad")}.
Note that you can use pixel units, viz. {\stfaf widths=qa.quantity("10pix 0.00001rad")}
see below.

\item String - for example {\stfaf widths="1km 2arcsec"} (i.e. a string that 
qa.quantity() accepts).

<!-- the Quanta <link anchor="quanta:quanta">Constructor</link> accepts).-->
  
\item Numeric - for example {\stfaf widths=[10,20]}.  In this case,
the units of the widths are assumed to be in pixels.
           
\end{itemize}

The interpretation of {\stfaf widths} depends upon the kernel type.

\begin{itemize}

\item Gaussian - the specified width is the full-width at
half-maximum.  

\item Boxcar (tophat) - the specified width is
the full width.  

\item Hanning - The kernel is $z[i] = 0.25*y[i-1] + 0.5*y[i] +
0.25*y[i+1]$.  The width is always 3 pixels, regardless of what
you give (but you still have to give it !).

\end{itemize}

The scaling of the output image is determined by the argument {\stfaf scale}.
If you leave it unset, then autoscaling will be invoked which means that
the convolution kernels will all be normalized to have unit volume
to as to conserve flux.  

If you do not leave {\stfaf scale} unset, then the convolution kernel  
will be scaled by this value (it has peak unity before application
of this scale factor).

Masked pixels will be assigned the value 0.0 before convolution. 
The output mask is the combination (logical OR) of the default input 
\pixelmask\ (if any) and the OTF mask.  Any other input \pixelmasks\
will not be copied.  Use function
<link anchor="images:image.maskhandler.function">maskhandler</link> if you need to copy other
masks too.

See also the other convolution functions
<link anchor="images:image.convolve2d.function">convolve2d</link>,
<link anchor="images:image.convolve.function">convolve</link> and
<link anchor="images:image.hanning.function">hanning</link>. 

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t sepconvolve Ex 1 \t----"
ia.maketestimage('xyv',overwrite=true)
im2 = ia.sepconvolve(outfile='xyv.con', axes=[0,1], types=["gauss","box"], widths=[10,20], overwrite=true)
im2.done()
ia.close()
#
"""
\end{verbatim}

</example>
</method>

 
   <method type="function" name="set">
   <shortdescription>Set pixel and/or mask values with a scalar in a region-of-interest of the image</shortdescription>
   
<input>
  
     <param type="any" direction="in" name="pixels">
	     <any type="variant"/>
     <description>The pixel value, LEL scalar expression or numeric scalar.
      Default is unset.</description>
     </param>
  
     <param type="int" direction="in" name="pixelmask">
     <description>The pixel mask value.  Either 0 or 1 if set.
      Default is unset.</description>
     <value>-1</value>
     </param>
  
     <param type="any" direction="in" name="region">
	     <any type="record"/>
     <description>The region of interest.  Default is whole image.</description>
     <value></value>
     </param>
  
     <param type="bool" direction="in" name="list">
     <description>List the bounding box and any mask creation to the logger</description>
     <value>false</value>
     </param>
</input>
<returns type="bool">T or fail</returns>
<description>

This function replaces data and/or mask values within the image in the
specified \region.  You can think of it as a simplified version of the
image calculator. 

Unlike the <link anchor="images:image.calc.function">calc</link> function, you can
only set a scalar value for all pixels in the specified \region.  For
example, it can be useful to set a whole image to one value, or a mask
in a \region\ to one value.  Although you could do that with the related
functions <link anchor="images:image.putregion.function">putregion</link> and
<link anchor="images:image.putchunk.function">putchunk</link>, you would have to make an
array of the shape of the image and if that is large, it could be
resource expensive. 

The value for the pixels is specified with the {\stfaf pixels} argument.  It can
be given as either a Lattice Expression Language (or LEL) expression
string or a simple numeric scalar.  See \htmladdnormallink{note
223}{../../notes/223/223.html} for a detailed description of the LEL
expression syntax.  If you give a LEL expression it must be a scalar
expression.   

Note that any default mask is {\em ignored} by this function when you
set pixel values.  This is different from
<link anchor="images:image.calc.function">calc</link> where the extant mask is
honoured. 

The value for the pixel mask is specified with the {\stfaf pixelmask}
argument ({\cf T, F, unset}).  If it's {\cf unset} then the mask is not
changed.  

If you specify {\stfaf pixelmask=} T or F, then the mask that is affected is
the current default mask (see
<link anchor="images:image.maskhandler.function">maskhandler</link>).  If there is no mask, a
mask is created for you and made the default mask. 

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t set Ex 1 \t----"
ia.maketestimage('yy',overwrite=true)
ia.fromshape('xx', [10,20], overwrite=true)
#r1 = ia.setboxregion([2,2],[6,8])
r1 = rg.box([2,2],[6,8])         # Make a box region
ia.set(pixels=1.0)               # Set all pixels to 1
ia.set(pixels='2.0', region=r1)  # Set all pixels to 2 in the region
ia.set(pixels='min(yy)')         # Set all pixels to minimum of image yy
                                 # Set pixels in region to minimum of image xx
ia.set(pixels='min('+ia.name(strippath=T)+')', region=r1)
ia.set(pixelmask=T)              # Set mask to all T
ia.set(pixels=0, pixelmask=F, region=r1)  #Set pixels and mask in region
ia.close()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="setbrightnessunit">
   <shortdescription>Set the image brightness unit</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="unit">
     <description>New brightness unit</description>
     </param>
</input>
<returns type="bool">T or fail</returns>
<description>

This function <!-- (short-hand name {\stff sbu}) --> sets the image brightness unit.
You can get the brightness unit with function
<link anchor="images:image.brightnessunit.function">brightnessunit</link>.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t setbrightnessunit Ex 1 \t----"
ia.fromshape(shape=[10,10])
ia.setbrightnessunit('km')
print ia.brightnessunit()
#km
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="setcoordsys">
   <shortdescription>Set new Coordinate System</shortdescription>
   
<input>
  
     <param type="any" direction="in" name="csys">
     <any type="record"/>
     <description>Record describing new Coordinate System</description>
     </param>
</input>
<returns type="bool">T or fail</returns>
<description>

This function replaces the Coordinate System in the image. Coordinate
System are stored in and manipulated with the
<link anchor="images:coordsys">Coordsys</link> \tool.  The Coordinate System can be
recovered from an image via the <link anchor="images:coordsys">coordsys</link>
function.

Note that changing the Coordsys \tool\ has no effect on the original
image, until it is replaced with this function.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t setcoordsys Ex 1 \t----"
ia.fromshape(shape=[10,20])          # Make image
mycs = ia.coordsys();                # Recover Coordinate System
incr = mycs.increment('n');          # Get increment as numeric vector
incrn = incr['numeric']
for i in range(len(incrn)):
  incrn[i] = 2*incrn[i]
incr['numeric']=incrn
mycs.setincrement(value=incr);       # Set new increment in Coordinate System
ia.setcoordsys(mycs.torecord());     # Set new Coordinate System in image
mycs.done()
ia.close()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="sethistory">
   <shortdescription>Set the history for an image</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="origin">
     <description>Used to set message origin.  Default is image::sethistory.</description>
     <value></value>
     </param>

     <param type="stringArray" direction="in" name="history">
     <description>New history</description>
     <value></value>
     </param>
</input>
<returns type="bool">T or fail</returns>

<description>

A \casa\ \imagefile\ can accumulate history information
from  an input \fits\ file or by you writing something into
it explicitly with this function.     Each element of
the input vector is one line of history.  The new
history is appended to the old.

<!-- Presently, \casa\  functions do not automatically add history records.-->
You can recover the history information with function
<link anchor="images:image.history.function">history</link>.

</description>

 
<example>
\begin{verbatim}
"""
#
print "\t----\t sethistory Ex 1 \t----"
ia.maketestimage('myfile',overwrite=true)  
h = ia.history()
# Adds three lines, 'I', 'like' and 'fish'
ia.sethistory(origin="sethistory", history=["I","like","fish"])
ia.close()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="setmiscinfo">
   <shortdescription>Set the miscellaneous information record for an image</shortdescription>
   
<input>
  
     <param type="any" direction="in" name="info">
	     <any type="record"/>
     <description>Miscellaneous REPLACEMENT header</description>
     </param>
</input>
<returns type="bool">T or fail</returns>

<description>

A \casa\ \imagefile\ can accumulate miscellaneous information
during its lifetime; it is stored in a record called the {\stff
miscinfo} record.  For example, the \fits\ reader
(<link anchor="images:image.fromfits.constructor">fromfits</link>) puts header keywords
it doesn't otherwise use into the {\stff miscinfo} record.  The {\stff
miscinfo} record is not guaranteed to have any entries, so it's up to
you to check for any fields that you require. 

This function sets the {\cf miscinfo} record of the \imagefile.  Note
that this function {\em replaces} the record, it doesn't add to it, so
if you want to augment the existing record, you should first capture
it with the <link
anchor="images:image.miscinfo.function">miscinfo</link> function, add
to the record, and then put it back.  The \fits\ writer will attempt
to write all the fields in the {\stff miscinfo} record to the \fits\ file.
It can do so for scalars and 1-dimensional arrays.  Records
will be omitted, and multi-dimensional arrays will be flattened into
1-dimensional arrays.

</description>

 
<example>
\begin{verbatim}
"""
#
print "\t----\t setmiscinfo Ex 1 \t----"
ia.maketestimage('myfile',overwrite=true)  
info = ia.miscinfo()            # capture the miscinfo record
info['extra'] = 'a test entry'  # add our information
ia.setmiscinfo(info)            # put it back into the image
ia.close()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="shape">
   <shortdescription>Length of each axis in the image</shortdescription>
   
<returns type="intArray">Vector of integers or fail</returns>
<description>

The shape of an image is a vector holding the length of each axis of
the image.  Although this information is also available in the <link
anchor="images:image.summary.function">summary</link> function, it is
so useful that it can be obtained directly.

</description>
<example>
\begin{verbatim}
"""
#
print "\t----\t shape Ex 1 \t----"
ia.fromshape(shape=[10,20,30])
imshape = ia.shape()
print imshape
#[10L, 20L, 30L]
# npixels = imshape[0]*imshape[1]*...*imshape[n-1]
npixels=1
for i in range(len(imshape)):
  npixels=npixels*imshape[i]
ia.close()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="setrestoringbeam">
   <shortdescription>Set the restoringbeam</shortdescription>
   
<input>
  
     <param type="any" direction="in" name="major">
     <any type="variant"/>
     <description>Major axis FWHM, Quantity or float (e.g., 1arcsec).  Default is unset.</description>
     <value>1arcsec</value>
     </param>
  
     <param type="any" direction="in" name="minor">
     <any type="variant"/>
     <description>Minor axis FWHM, Quantity or float (e.g., 1arcsec).
      Default is unset.</description>
     <value>1arcsec</value>
     </param>
  
     <param type="any" direction="in" name="pa">
     <any type="variant"/>
     <description>Position angle, Quantity or float (e.g., '5deg').
      Default is unset.</description>
     <value>0deg</value>
     </param>
  
     <param type="any" direction="in" name="beam">
		<any type="record"/>
     <description>The complete restoring beam (output of restoringbeam()).  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="bool" direction="in" name="remove">
     <description>Delete the restoring beam?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="log">
     <description>Write new beam values to the logger?</description>
     <value>true</value>
     </param>
</input>

<returns type="bool">T or fail</returns>
<description>

This function <!-- (short-hand name {\stff srb}) --> sets the restoring beam.
Do not do this unless it is a correct thing to do !

You may supply the beam in one of two ways.  

First, you can use the argument {\stfaf beam} which you must assign to a
record containing  fields 'major', 'minor' and 'positionangle'.  
Each of these fields contains a quantity.   This record is in the same
format as  one returned by function
<link anchor="images:image.restoringbeam.function">restoringbeam</link>. If you leave {\stfaf
beam} unset, no action with it is taken. If the function argument
{\stfaf beam} is used, the arguments {\stfaf major, minor, \& pa} are
ignored.

Second, you can use the arguments {\stfaf major}, {\stfaf minor}
and {\stfaf pa}.   Only the ones that you assign are used.
Each argument should be assigned either a quantity or a float (units
are implicitly those of the current beam - or if none, arcsec for
the axes and degrees for the position angle).

You may also delete the restoring beam by setting {\stfaf delete=T}
(all other arguments are then ignored).

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t setrestoringbeam Ex 1 \t----"
ia.maketestimage('hcn',overwrite=true)      
rb = ia.restoringbeam()        # returns beam in record
print rb
#{'major': {'unit': 'arcsec', 'value': 53.500004857778549},
# 'minor': {'unit': 'arcsec', 'value': 34.199998900294304},
# 'positionangle': {'unit': 'deg', 'value': 6.0}}
rb['minor']['value'] = 12.5
ia.setrestoringbeam(beam=rb)   # new beam specified in record
print ia.restoringbeam()
#{'major': {'unit': 'arcsec', 'value': 53.500004857778549},
# 'minor': {'unit': 'arcsec', 'value': 12.5},
# 'positionangle': {'unit': 'deg', 'value': 6.0}}
ia.setrestoringbeam(major='36arcsec')  # beam specified using parameter
print ia.restoringbeam()
#{'major': {'unit': 'arcsec', 'value': 36.0},
# 'minor': {'unit': 'arcsec', 'value': 12.5},
# 'positionangle': {'unit': 'deg', 'value': 6.0}}
ia.setrestoringbeam(remove=true)
print ia.restoringbeam()
#{}
ia.setrestoringbeam(major='53.5arcsec',minor='34.2arcsec',pa='6deg')
print ia.restoringbeam()
#{'major': {'unit': 'arcsec', 'value': 53.5},
# 'minor': {'unit': 'arcsec', 'value': 34.200000000000003},
# 'positionangle': {'unit': 'deg', 'value': 6.0}}
ia.close()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="statistics">
   <shortdescription>Compute statistics from the image</shortdescription>
   
<input>
     <param type="intArray" direction="in" name="axes">
     <description>List of axes to evaluate statistics over.
     Default is all axes.</description>
     <value>-1</value>
     </param>
  
     <param type="any" direction="in" name="region">
	     <any type="record"/>
     <description>Region of interest.  Default is whole image.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="mask">
	     <any type="variant"/>
     <description>OTF mask, Boolean LEL expression or mask region.
     Default is none.</description>
     <value></value>
     </param>
  
     <param type="stringArray" direction="in" name="plotstats">
     <description>List of statistics to plot
     String containing any
           combination of 'npts', 'sum', 'flux', 'sumsq', 'min', 
           'max', 'mean', 'sigma' (or 'stddev'), 'rms', 'median',
           'medabsdevmed', 'quartile'
     Default is ['mean','sigma'].
     </description>
     <value type="vector"></value>
     </param>
  
     <param type="doubleArray" direction="in" name="includepix">
     <description>Range of pixel values to include.  Vector of 1 or 2 doubles.
     Default is to include all pixels.</description>
     <value>-1</value>
     </param>
  
     <param type="doubleArray" direction="in" name="excludepix">
     <description>Range of pixel values to exclude.  Vector of 1 or 2 doubles.
     Default is exclude no pixels.</description>
     <value>-1</value>
     </param>
  
     <param type="string" direction="in" name="plotter">
     <description>The PGPLOT device name to make plots on.  Default is no plotting.</description>
     <value></value>
     </param>
  
     <param type="int" direction="in" name="nx">
     <description>The number of subplots per page in the x direction, > 0</description>
     <value>1</value>
     </param>
  
     <param type="int" direction="in" name="ny">
     <description>The number of subplots per page in the y direction, > 0</description>
     <value>1</value>
     </param>
  
     <param type="bool" direction="in" name="list">
     <description>If T then list the statistics as well</description>
     <value>true</value>
     </param>
  
     <param type="bool" direction="in" name="force">
     <description>If T then force the stored statistical accumulations to be regenerated</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="disk">
     <description>If T then force the storage image to disk</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="robust">
     <description>If T then compute robust statistics as well</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="verbose">
     <description>If T then print useful messages to the logger</description>
     <value>true</value>
     </param>
  
     <param type="bool" direction="in" name="async">
     <description>Run asynchronously?</description>
     <value>false</value>
     </param>
</input>

<returns type="any">
  <any type="record"/>
  <description> record containing all statistics</description>
</returns>

<description>

This function <!-- (short-hand name is {\stff stats}) --> computes statistics
from the pixel values in the image.  You can then <!-- plot them,--> list them
and retrieve them (into a record) for further analysis. 

The chunk of the image over which you evaluate the statistics is
specified by an array of axis numbers (argument {\stfaf axes}).  For example,
consider a 3-dimensional image for which you specify {\stfaf axes=[0,2]}. 
The statistics would be computed for each XZ (axes 0 and 2) plane in the
image.  You could then examine those statistics as a function of the Y
(axis 1) axis.  Or perhaps you set {\stfaf axes=[2]}, whereupon you could
examine the statistics for each Z (axis 2) profile as a function of X
and Y location in the image. 

<!--
You have control over how the statistics are presented; whether they are
listed, or plotted, or retrieved into records or all of the above. 

A variety of statistics are offered for plotting and you can plot any
combination of them simultaneously (argument {\stfaf plotstats}).   
-->

If you retrieve the statistics <!-- into a record (argument {\stfaf
statsout}) --> from the returned record, each statistic is stored in
an array in one named field in the record.  The shape of that array is
that of the axes which you did {\bf not} evaluate the statistics over.
For example, in the second example above, we set {\stfaf axes=[2]} and
asked for statistics as a function of the remaining axes, in this
case, the X and Y (axes 0 and 1) axes.  The shape of each statistics
array is then [nx,ny].

The names of the fields in this record are the same as the names of the
statistics that you can plot: 

\begin{itemize}

\item {\stfaf npts} - the number of unmasked points used

\item {\stfaf sum} - the sum of the pixel values: $\sum I_i$

\item {\stfaf flux} - the integrated flux density if the beam is defined and
the brightness units are $Jy/beam$

\item {\stfaf sumsq} - the sum of the squares of the pixel values: $\sum I_i^2$

\item {\stfaf mean} - the mean of pixel values: $\bar{I} = \sum I_i / n$

\item {\stfaf sigma} - the standard deviation about the
mean: $\sigma^2 = (\sum I_i - \bar{I})^2 / (n-1)$

\item {\stfaf rms} - the root mean square: $\sqrt {\sum I_i^2 / n}$

\item  {\stfaf min} - minimum pixel value

\item {\stfaf max} - the maximum pixel value

\item {\stfaf median} - the median pixel value (if {\stfaf robust=T})

\item {\stfaf medabsdevmed} - the median of the absolute deviations from the median
(if {\stfaf robust=T})

\item {\stfaf quartile} - the inter-quartile range (if {\stfaf
robust=T}).  Find the points which are 25\% largest and 75\% largest
(the median is 50\% largest), find their difference and divide that
difference by 2.

\item {\stfaf blc} - the absolute pixel coordinate of the bottom left
corner of the bounding box of the region of interest.  If 'region' is
unset, this will be the bottom left corner of the whole image.

\item {\stfaf blcf} - the formatted absolute world coordinate of the bottom left corner of the bounding box of the region of interest.

\item {\stfaf trc} - the absolute pixel coordinate of the top right corner of the bounding box of the region of interest.

\item {\stfaf trcf} - the formatted absolute world coordinate of the top right corner of the bounding box of the region of interest.

\item {\stfaf minpos} - absolute pixel coordinate of minimum pixel value

\item {\stfaf maxpos} - absolute pixel coordinate of maximum pixel value

\item {\stfaf minposf} - formatted string of the world coordinate of
the minimum pixel value

\item  {\stfaf maxposf} - formatted string of the world coordinate of
the maximum pixel value

\end{itemize}

The last four fields only appear if you evaluate the statistics over all
axes in the image.  As an example, if the returned record is saved in
{\stfaf `mystats'}, then you could access the `mean' field via
{\cf print mystats['mean']}. 

If there are no good points (e.g.  all pixels are masked bad in the
region), then the length of these fields will be 0 (e.g.  {\cf
len(mystats['npts'])==0}). 

<!--
If you retrieve statistics all of the available statistics are retrieved
into the record regardless of what you might have asked to plot. -->
You can of course plot the statistics by accessing the arrays in
the record. <!--, rather than using the built in plotting
capability. -->

You have no control over which statistics are listed to the logger,
you always get the same selection.  You can choose to list the
statistics or not (argument {\stfaf list}).

As well as the simple (and faster to calculate) statistics like means
and sums, you can also compute some robust statistics.  Currently
these are the median, median absolute deviations from the median and
the inter-quartile range.  Because these are computationally
expensive, they are only computed if you directly ask for them.  You
can do this in two ways.  First by requesting a robust statistic in
the string array assigned to {\stfaf plotstats}.  Second by setting
{\stfaf robust=T} (it defaults to F).  <!-- For example, it may be that you
are not making any plots but wish to recover the robust statistics
into the output record.  This is why this extra argument is needed.-->

Note that if the axes are set to all of the axes in the image (which is
the default) <!-- , no plotting is available as--> there is just one point
per statistic. 

You have control over which pixels are included in the statistical sums
via the {\stfaf includepix} and {\stfaf excludepix} arguments.  These vectors
specify a range of pixel values for which pixels are either included or
excluded.  They are mutually exclusive; you can specify one or the
other, but not both.  If you only give one value for either of these,
say {\stfaf includepix=b}, then this is interpreted as {\stfaf
includepix=[-abs(b),abs(b)]}.  

<!--
The plotting is done directly on a PGPLOT plotting device.  The syntax
is {\stfaf plotter=name/type}.  For example {\stfaf
plotter='plot1.ps/ps'} (disk postscript file) or {\stfaf plotter='plot/glish'}
(\glish\ pgplotter). 
-->

This function generates a `storage' lattice, into which the statistics
are written.  It is only regenerated when necessary.  For example, if
you run the function twice with identical arguments, the statistics will
be directly retrieved from the storage lattice the second time. 
However, you can force regeneration of the storage image if you set
{\stfaf force=T}.  The storage medium is either in memory or on disk,
depending upon its size.  You can force it to disk if you set {\stfaf
disk=T}, otherwise it decides for itself.    

</description>

<example> 
\begin{verbatim} 
"""
#
print "\t----\t statistics Ex 1 \t----"
ia.maketestimage()
ia.statistics()
ia.close()
#
"""
\end{verbatim} 
In this example, we ask to see statistics evaluated over the
entire image. <!-- No plotting is available under these conditions.-->
</example>

<example> 
\begin{verbatim}
"""
#
print "\t----\t statistics Ex 2 \t----"
ia.maketestimage()
stats = ia.statistics(axes=[1],plotstats=["sigma","rms"],
                      includepix=[0,100],list=F)
#
"""
\end{verbatim} 

In this example, let us assume the image has 2 dimensions.  We want
the standard deviation about the mean and the rms of Y (axes 1) for
pixels with values in the range 0 to 100 as a function of the X-axis
location.  The statistics are not listed to the logger but are saved
in the record {\stfaf 'stats'}.

<!--
In this example, let us assume the image has 3 dimensions.  We plot the
standard deviation about the mean and the rms of YZ (axes 2 and 3)
planes for pixels with values in the range 0 to 100 as a function of
X-axis location on the standard PGPLOT X-windows device.  The statistics
are not listed to the logger.  Because we are making plots, the tool
function is automatically run synchronously so we can directly recover
the the record containing the statistics arrays. 
-->

</example>
</method> 

 
   <method type="function" name="twopointcorrelation">
   <shortdescription>Compute two point correlation function from the image</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="region">
	     <any type="record"/>
     <description>The region of interest.  Default is whole image.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="mask">
	     <any type="variant"/>
     <description>OTF mask, Boolean LEL expression or mask region.
     Default is none.</description>
     <value></value>
     </param>
  
     <param type="intArray" direction="in" name="axes">
     <description>The pixel axes to compute structure function over.
     The default is sky or first two axes.</description>
     <value>-1</value>
     </param>
  
     <param type="string" direction="in" name="method">
     <description>The method of computation.
     String from 'structurefunction'.</description>
     <value>structurefunction</value>
     </param>
  
     <param type="bool" direction="in" name="overwrite">
     <description>Overwrite (unprompted) pre-existing output file?</description>
     <value>false</value>
     </param>
</input>
<returns type="bool">T or fail</returns>

<description>

This function <!-- (short-hand name is {\stff tpc}) --> computes
two-point auto-correlation functions from an image.

By default, the auto-correlation function is computed for the Sky axes.
If there is no sky in the image, then the first two axes are used.
Otherwise you can specify which axes the auto-correlation function lags
are computed over with the {\stfaf axes} argument (must be of length 2).

Presently, only the Structure Function is implemented.  This is defined as :

\begin{displaymath}
S(lx,ly) = < (data(i,j) - data(i+lx,j+ly))^2 >
\end{displaymath}

where $lx, ly$ are integer lags in the x (0-axis) and y (1-axis)
directions.  The ensemble average is over all the values at the same
lag pair.  This process is extremely compute intensive and so you may
have to be patient.

In an auto-correlation function image there are some symmetries.  The
first and third quadrants are symmetric, and the second and fourth are
symmetric.  So in principle, all the information is in the top or bottom
half of the image.  We just write it all out to look nice.  The long
lags don't have a lot of contributing values of course.  

</description>

<example> 
\begin{verbatim} 
"""
#
print "\t----\t twopointcorrelation Ex 1 \t----"
ia.maketestimage();        # Output image is virtual
ia.twopointcorrelation()   # Output image is virtual
#
"""
\end{verbatim} 
</example>

</method> 

 
   <method type="function" name="subimage">
   <shortdescription>Create a (sub)image from a region of the image</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="region">
	     <any type="record"/>
     <description>Region of interest.  Default is whole image.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="mask">
	     <any type="variant"/>
     <description>OTF mask, Boolean LEL expression or mask region.
     Default is none.</description>
     <value></value>
     </param>
  
     <param type="bool" direction="in" name="dropdeg">
     <description>Drop degenerate axes</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="overwrite">
     <description>Overwrite (unprompted) pre-existing output file?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="list">
     <description>List informative messages to the logger</description>
     <value>true</value>
     </param>
</input>
<returns type="casaimage"/>

<description>

This function <!-- (short-hand name {\stff subim}) --> copies all or
part of the image to another on-the-fly Image tool.
<!-- (there is also a
\htmlref{subimage}{images:image.subimage.constructor} constructor). -->

If {\stfaf outfile} is given, the subimage is written to the specified
disk file.  If {\stfaf outfile} is unset, the returned Image \tool\ actually
references the input image file (i.e.  that associated with the Image
\tool\ to which you are applying this function).  So if you deleted the
input image disk file, it would render this \tool\ useless.  When you
destroy this \tool\ (with the <link anchor="images:image.done.function">done</link> function)
the reference connection is broken. 

Sometimes it is useful to drop axes of length one (degenerate axes).
Use the {\stfaf dropdeg} argument if you want to do this.

The output mask is the combination (logical OR) of the default input
\pixelmask\ (if any) and the OTF mask.  Any other input \pixelmasks\
will not be copied.  Use function <link
anchor="images:image.maskhandler.function">maskhandler</link> if you
need to copy other masks too.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t subimage Ex 1 \t----"
ia.maketestimage('myfile',overwrite=true)
im2 = ia.subimage()                # a complete copy
r1 = rg.box([10,10],[30,40],[5,5]) # A strided pixel box region
im3 = ia.subimage(outfile='/tmp/foo', region=r1, overwrite=true)
                                   # Explicitly named subimage
im2.done()
im3.done()
ia.close()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="summary">
   <shortdescription>Summarize basic information about the image</shortdescription>
   
<output>  
     <param type="any" direction="out" name="header">
     <any type="record"/>
     <description>Header information</description>
     </param>
</output>  
  
<input>
     <param type="string" direction="in" name="doppler">
     <description>If there is a spectral axis, list velocity too, with this doppler definition</description>
     <value>RADIO</value>
     </param>
  
     <param type="bool" direction="in" name="list">
     <description>List the summary to the logger</description>
     <value>true</value>
     </param>
  
     <param type="bool" direction="in" name="pixelorder">
     <description>List axis descriptors in pixel or world axis order</description>
     <value>true</value>
     </param>
</input>
<returns type="stringArray">Array of Strings</returns>
<description>

This function summarizes miscellaneous information such as shape, Coordinate System,
restoring beams, and masks.

If called without any arguments, this function displays a summary of the
image header to the logger; where appropriate, values will be
formatted nicely (e.g.  HH:MM:SS.SS for the reference value of RA axes). 

For spectral axes, the information is listed as a velocity as well as
a frequency.  The argument {\stfaf doppler} allows you to specify what
velocity doppler convention it is listed in.  You can choose from
{\stfaf radio, optical} and {\stfaf true}.  Alternative names are
{\stfaf z} for {\stfaf optical}, and {\stfaf beta} or {\stfaf
relativistic} for {\stfaf true}.  The default is {\stfaf radio}.  The
definitions are

\begin{itemize}
\item radio: $1 - F$
\item optical: $-1 + 1/F$
\item true: $(1 - F^2)/(1 + F^2)$
\end{itemize}
where $F = \nu/\nu_0$ and $\nu_0$ is the rest frequency.  If the rest
frequency has not been set in your image, you can set it via
a Coordsys \tool\ with
the function <link anchor="images:coordsys.setrestfrequency.function">setrestfrequency</link>.

If the output of summary is saved to a variable, then the {\stfaf
header} field (for instance, {\stfaf mysummary['header']}) has the
following fields filled in:

\begin{description}
    \item[ndim]       Dimension of the image.
    \item[shape]      Length of each axis in the image.
    \item[tileshape]  Shape of the chunk which is most efficient for I/O.
    \item[axisnames]  Name of each axis.
    \item[refpix]     Reference pixel for each axis (0-relative)
    \item[refval]     Reference value for each axis.
    \item[incr]       Increment for each axis.
    \item[axisunits]  Unit name for each axis.
    \item[unit]       Brightness units for the pixels.
    \item[hasmask]    True if the image has a mask.
    \item[defaultmask]The name of the mask which is applied by default.
    \item[masks]      The names of all the masks stored in this image.
    \item[restoringbeam] The restoring beam if present.
    \item[imagetype]   The image type.
\end{description}

If you set {\stfaf list=F}, then the summary will not be written to
the logger.  <!-- However, in addition to the output record,--> The
return value of the function, in the {\stfaf header} field is a
vector string containing the formatted output that would normally
have gone to the logger. <!--  If {\stfaf list=T} the return value is
always T or a fail.-->

</description>
<example>
\begin{verbatim}
"""
#
print "\t----\t summary Ex 1 \t----"
ia.maketestimage('myim1', overwrite=true)
ia.summary()                 # summarize to logging only
s = ia.summary(list=F)       # store header in record
if s['header']['ndim'] == 2: # program using header values
  print s['header']['axisnames']
ia.close()
#
"""
\end{verbatim}

<!--
Note that the Spectral Coordinate is listed twice; once with frequency
and once with velocity units.  You can distinguish this double listing
because the shape of the axis is only listed once (in
this case on the Frequency line). 

Note also that this image has three masks present.  The first one is the
default which means that it is applied by applications.  Any of the
other masks, listed in square brackets, could be made the default mask
by application of the <link anchor="images:image.maskhandler.function">maskhandler</link>
function.   If there was no default mask (effectively that means
an all good mask would be applied), all the mask names would be 
listed in square brackets.
-->

</example>

</method>

 
   <method type="function" name="tofits">
   <shortdescription>Convert the image to a FITS file</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="outfile">
	     <description>FITS file name.
     Default is input name + '.fits'</description>
     <value></value>
     </param>
  
     <param type="bool" direction="in" name="velocity">
     <description>prefer velocity (rather than frequency) as primary spectral axis?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="optical">
     <description>use the optical (rather than radio) velocity convention?</description>
     <value>true</value>
     </param>
  
     <param type="int" direction="in" name="bitpix">
     <description>Bits per pixel, -32 (floating point) or 16 (integer)</description>
     <value>-32</value>
     </param>
  
     <param type="double" direction="in" name="minpix">
     <description>Minimum pixel value for BITPIX=16,
     Default is to autoscale if minpix > maxpix.</description>
     <value>1</value>
     </param>
  
     <param type="double" direction="in" name="maxpix">
     <description>Maximum pixel value for BITPIX=16, Default
     is to autoscale if maxpix < minpix.</description>
     <value>-1</value>
     </param>
  
     <param type="any" direction="in" name="region">
	     <any type="record"/>
     <description>The region of interest.  Default is whole image.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="mask">
	     <any type="variant"/>
     <description>OTF mask, Boolean LEL expression or mask region
     Default is none.</description>
     <value></value>
     </param>
  
     <param type="bool" direction="in" name="overwrite">
     <description>Overwrite (unprompted) pre-existing output file?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="dropdeg">
     <description>Drop degenerate axes?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="deglast">
     <description>Put degenerate axes last in header?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="async">
     <description>Run asynchronously?</description>
     <value>false</value>
     </param>
</input>
<returns type="bool"/>
<description>

This function converts the image into a \fits\ file.


If the image has a rest frequency associated with it, it will always
write velocity information into the \fits\ file.  By default the
frequency information will be primary as it is better defined than
velocity information.  If you select {\stfaf velocity=T} then by default
the velocity is written in the optical convention, but if {\stfaf
optical=F} it will use the radio convention instead. 

The \fits\ definition demands equal increment pixels.  Therefore, if you
write velocity information as primary and use the optical velocity
definition, the increment is computed at the spectral reference pixel. 
If the bandwidth is large, this may incur non-negligible coordinate
calculation errors far from the reference pixel. 

By default the image is written as a floating point \fits\ file
({\stfaf bitpix= -32}).  Under rare circumstances you might want to
save space and write it as scaled 16 bit integers ({\stfaf bitpix =
16}).  You can have {\stff tofits} calculate the scaling factors by
using the default {\stfaf minpix} and {\stfaf maxpix}.  If you set
{\stfaf minpix} and {\stfaf maxpix}, values outside of that range will
be truncated.  This can be useful if all of the \fits\ images dynamic
range is being used by a few high or low values and you are not
interested in preserving those values exactly.  Besides the factor of
two space savings you get by using 16 instead of 32 bits, integer
images usually also compress well (for example, with the standard GNU
software facility {\tt gzip}).

If the specified \region\ extends beyond the image, it is truncated.

The output mask is the combination (logical OR) of the default input 
\pixelmask\ (if any) and the OTF mask. 

Sometimes it is useful to drop axes of length one (degenerate axes)
because not all FITS readers can handle them.  Use the {\stfaf dropdeg}
argument if you want to do this. 

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t tofits Ex 1 \t----"
ia.maketestimage()
ok = ia.tofits('MYFILE.FITS',overwrite=true)
                       # write FITS image file
ok = ia.tofits('MYFILE2.FITS', bitpix=16, overwrite=true)
                       # Write as scaled 16 bit integers
ia.close()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="toASCII">
   <shortdescription>Convert the image to an ASCII file</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="outfile">
     <description>ASCII file name.
     Default is input name + '.ascii'.</description>
     </param>
  
     <param type="any" direction="in" name="region">
	     <any type="record"/>
     <description>The region of interest.
     Default is whole image.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="mask">
	     <any type="variant"/>
     <description>OTF mask, Boolean LEL expression or mask region.
     Default is none.</description>
     <value></value>
     </param>
  
     <param type="string" direction="in" name="sep">
     <description>Separator of data in ascii file.
     Default is space character.</description>
     <value> </value>
     </param>
  
     <param type="string" direction="in" name="format">
     <description>Format of data in ascii file</description>
     <value>%e</value>
     </param>
  
     <param type="double" direction="in" name="maskvalue">
     <description>Value to replace masked pixels by, -999 is no change.</description>
     <value>-999</value>
     </param>
  
     <param type="bool" direction="in" name="overwrite">
     <description>Overwrite (unprompted) pre-existing output file?</description>
     <value>false</value>
     </param>
</input>
<returns type="bool">T or fail</returns>
<description>

This function converts the image into an ascii file. The format is one
image row per line (see
<link anchor="images:image.fromascii.constructor">fromascii</link>).

The output mask is the combination (logical OR) of the default input 
\pixelmask\ (if any) and the OTF mask.  Because the mask is not
transferred to the ascii file, you must specify what data value to use
if a pixel is masked.  By default, the underlying data value in the
image is used. But this could be anything (and often it's a NaN), so you
could set, say, {\stfaf maskvalue=-10000} as a magic value.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t toASCII Ex 1 \t----"
ia.maketestimage()
ok = ia.toASCII('myfile.ascii', overwrite=true)
ia.close()
ia.fromascii('image.im','myfile.ascii', shape=[113,76], overwrite=true)
ia.toASCII('myfile2.ascii',overwrite=true)
#!diff myfile.ascii myfile2.ascii
#
"""
\end{verbatim}
</example>
</method>

<method type="function" name="torecord">
   <shortdescription>Return a record containg the image associated with this 
   tool
   </shortdescription>
    
<returns type="any">
  <any type="record"/>
  <description> record containing all the image data and informations</description>
</returns>
  
<description>

You can convert an associated image to a record for manipulation or passing it to inputs of other function of other tools

<!--These functions are not for general user use.-->

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t torecord Ex 1 \t----"
ia.maketestimage('image.large', overwrite=true)
rec=ia.torecord()
ia.close()



"""
\end{verbatim}
</example>
</method>



 
   <method type="function" name="type">
   <shortdescription>Return the type of this tool</shortdescription>
   
<returns type="string">String or fail</returns>

<description>

This function returns the string `image'.  It can be used in
a script to make sure this variable is an Image
\tool.

</description>
</method>

 
   <method type="function" name="topixel">
   <shortdescription>Convert from world to pixel coordinate</shortdescription>
   
<input>

     <param type="any" direction="in" name="value">
	     <any type="variant"/>
     <description>Absolute world coordinate,
      Numeric vector, vector of quantities or record.
      Default is reference value.</description>
     </param>
</input>
<returns type="any">Vector of float or fail<any type="record"/></returns>

<description>

This function converts from absolute world to pixel coordinate (0-rel). 
The world coordinate can be provided in many formats (numeric, string,
quantum etc.) via the argument {\stfaf value}.  These match the output
formats of function <link anchor="images:image.toworld.function">toworld</link>. 

This function is just a wrapper for the Coordsys \tool\ function
<link anchor="images:coordsys.topixel.function">topixel</link> so see the documentation there
for a description and more examples. 

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t topixel Ex 1 \t----"
ia.maketestimage();
w = ia.toworld([10,10], 'n')        # Numeric vector
ia.topixel(w)
#{'ar_type': 'absolute',
# 'numeric': array([10., 10.]), 'pw_type': 'pixel'}
w = ia.toworld([10,10], 'm')        # Record of measures
ia.topixel(w)
#{'ar_type': 'absolute',
# 'numeric': array([10., 10.]), 'pw_type': 'pixel'}
ia.close()
#
"""
\end{verbatim}
Convert a pixel coordinate to world as floats and then
back to pixel.  Do the same with the world coordinate
formatted as measures instead.
</example>

</method>

 
   <method type="function" name="toworld">
   <shortdescription>Convert from pixel to world coordinate</shortdescription>
   
<input>

     <param type="any" direction="in" name="value">
     <any type="variant"/>
     <description>Absolute pixel coordinate.  Numeric vector is allowed.
     Default is reference pixel.</description>
     <value></value>
     </param>

     <param type="string" direction="in" name="format">
     <description>What type of formatting?
     String from combination of 'n' (numeric), 'q' (quantity),
     'm' (measure), 's' (string).</description>
     <value>n</value>
     </param>
</input>
<returns type="any"><any type="record"/>Vector float, vector quantity, record, string or fail</returns>

<description>

This function converts between absolute pixel coordinate (0-rel)   
and world (physical coordinate).

This function is just a wrapper for the Coordsys \tool\ function
<link anchor="images:coordsys.toworld.function">toworld</link> so see the documentation there
for a description of the arguments and more examples. 

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t toworld Ex 1 \t----"
ia.maketestimage('hcn',overwrite=true)
w = ia.toworld([10,10], 'n')
print w
#{'numeric': array([ 0.00174533, -0.0015708 ])}
w = ia.toworld([10,10], 'nmq')
print w
#{'measure': {'direction': {'m0': {'unit': 'rad',
#                                  'value': 0.0017453323593185704},
#                           'm1': {'unit': 'rad',
#                                  'value': -0.0015707969259645381},
#                           'refer': 'J2000',
#                           'type': 'direction'}},
# 'numeric': array([ 0.00174533, -0.0015708 ]),
# 'quantity': {'*1': {'unit': 'rad', 'value': 0.0017453323593185704},
#              '*2': {'unit': 'rad', 'value': -0.0015707969259645381}}}
ia.close()
#
"""
\end{verbatim}
Convert to a vector of floats and then to a record
holding a vector of floats, a vector of quantities
and a subrecord of measures.
</example>
</method>

 
   <method type="function" name="unlock">
   <shortdescription>Release any lock on the image</shortdescription>
   <returns type="bool"/>

<description>

This function releases any lock set on the \imagefile\ (and also flushes
any outstanding I/O to disk).  It is not of general user interest.  It
can be useful in scripts when a file is being shared between more than
one process.  See also functions <link anchor="images:image.lock.function">lock</link> and
<link anchor="images:image.haslock.function">haslock</link>. 

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t unlock Ex 1 \t----"
ia.fromarray('xx', ia.makearray(0,[10,20]), overwrite=true)
ia.unlock()
ia.close()
#
"""
\end{verbatim}

This releases the write lock on the \imagefile.  Now some
other process can gain immediate access to the \imagefile.

</example>

</method>

   <method type="function" name="newimagefromarray">
   <shortdescription>Construct a \casa\ image from an array</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="pixels">
	     <any type="variant"/>
     <description>A numeric array is required.</description>
     </param>
  
     <param type="any" direction="in" name="csys">
     <any type="record"/>
     <description>Coordinate System.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="bool" direction="in" name="linear">
     <description>Make a linear Coordinate System if csys not given</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="overwrite">
     <description>Overwrite (unprompted) pre-existing output file?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="log">
     <description>Write image creation messages to logger</description>
     <value>true</value>
     </param>
</input>
<returns type="casaimage"/>

<description>

This function converts an array of any size into a \casa\
\imagefile. 

If {\stfaf outfile} is given, the image is written to the specified
disk file.  If {\stfaf outfile} is unset, the on-the-fly Image \tool\
returned by this function is associated with a temporary image.  This
temporary image may be in memory or on disk, depending on its size.
When you destroy the on-the-fly Image \tool\ (with the <link
anchor="images:image.done.function">done</link> function) this
temporary image is deleted.

At present, no matter what type the {\stfaf pixels} array is, a
real-valued image will be written (the input pixels will be converted
to Float).  In the future, Complex images will be supported.

The Coordinate System, provided as a record describing a <link
anchor="images:coordsys">Coordsys</link> \tool\ (via
coordsys.torecord(), for instance) is optional.  If you provide it, it
must be dimensionally consistent with the pixels array you give (see
also <link anchor="images:image.coordsys.function">coordsys</link>).

If you don't provide the Coordinate System (unset), a default Coordinate System
is made for you.  If {\stfaf linear=F} (the default) then it is a
standard RA/DEC/Stokes/Spectral Coordinate System depending exactly upon
the shape of the {\stfaf pixels} array (Stokes axis must be no longer
than 4 pixels and you may find the spectral axis coming out before the
Stokes axis if say, {\cf shape=[64,64,32,4]}).  Extra dimensions are
given linear coordinates.  If {\stfaf linear=T} then you get a linear
Coordinate System. 

</description>
<example>
\begin{verbatim}
"""
#
print "\t----\t newimagefromarray Ex 1 \t----"
im1=ia.newimagefromarray(outfile='test.data',
                         pixels=ia.makearray(0, [64, 64, 4, 128]),
                         overwrite=true)
cs1 = im1.coordsys(axes=[0,1])
im1.done()
im2 = ia.newimagefromarray(pixels=ia.makearray(1.0, [32, 64]),
                           csys=cs1.torecord())
cs1.done()
im2.done()
#
"""
\end{verbatim}

The first example creates a zero-filled \imagefile\ named {\sff
test.data} which is of shape [64,64,4,128].  If you examine the header
with {\stff ia.summary()} you will see the default
RA/DEC/Stokes/Frequency coordinate information.  In the second
example, a Coordinate System describing the first two axes of the
image {\sff test.data} is created and used to create a 2D image
temporary image.

</example>
</method>

   <method type="function" name="newimagefromfits">
   <shortdescription>Construct a \casa\ image by conversion from a FITS image file</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="string" direction="in" name="infile">
     <description>Input FITS disk file name.  Required.</description>
     <value></value>
     </param>
  
     <param type="int" direction="in" name="whichrep">
     <description>If this FITS file contains multiple coordinate representations, which one should we read</description>
     <value>0</value>
     </param>
  
     <param type="int" direction="in" name="whichhdu">
     <description>If this FITS file contains multiple images, which one should we read (0-based).</description>
     <value>0</value>
     </param>
  
     <param type="bool" direction="in" name="zeroblanks">
     <description>If there are blanked pixels, set them to zero instead of NaN</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="overwrite">
     <description>Overwrite (unprompted) pre-existing output file?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="old">
     <description>Use old non-WCS based FITS parser?</description>
     <value>false</value>
     </param>
</input>
<returns type="casaimage"/>
<description>

This function is used to convert a FITS disk image file (Float,
Double, Short, Long are supported) to an \casa\ \imagefile.  If
{\stfaf outfile} is given, the image is written to the specified disk
file.  If {\stfaf outfile} is unset, the on-the-fly Image \tool\
returned by this function is associated with a temporary image.  This
temporary image may be in memory or on disk, depending on its size.
When you destroy the on-the-fly Image \tool\ (with the <link
anchor="images:image.done.function">done</link> function) this
temporary image is deleted.

This function reads from the FITS primary array (when the image is at
the beginning of the FITS file; {\stfaf whichhdu=0}), or an image
extension (when the image is elsewhere in the FITS file, {\stfaf
whichhdu $>$ 0}). 

By default, any blanked pixels will be converted to a mask value which
is false, and a pixel value that is NaN.  If you set {\stfaf
zeroblanks=T} then the pixel value will be zero rather than NaN.  The
mask will still be set to false.  See the function
<link anchor="images:image.replacemaskedpixels.function">replacemaskedpixels</link> if you
need to replace masked pixel values after you have created the image. 

</description>
<example>
\begin{verbatim}
"""
#
print "\t----\t newimagefromfits Ex 1 \t----"
# Assume we can find test fits file using
# CASAPATH environment variable
pathname=os.environ.get("CASAPATH")
pathname=pathname.split()[0]
datapath=pathname+'/data/demo/Images/imagetestimage.fits'
im1=ia.newimagefromfits('./myimage', datapath, overwrite=true)
print im1.summary()
print im1.miscinfo()
print 'fields=', im1.miscinfo().keys()
im1.done()
#
"""
\end{verbatim}

The FITS image is converted to a \casa\ \imagefile\ and access is
provided via the \imagetool\ called {\stf im1}.  Any FITS header
keywords which were not recognized or used are put in the
miscellaneous information bucket accessible with the <link
anchor="images:image.miscinfo.function">miscinfo</link> function.  In
the example we list the names of the fields in this record.

</example>
</method>


   <method type="function" name="newimagefromimage">
   <shortdescription>Construct an on-the-fly image tool from a region of a \casa\ image file</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="infile">
     <description>Input image file name.  Required.</description>
     <value></value>
     </param>
  
     <param type="string" direction="in" name="outfile">
     <description>Output sub-image file name.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="region">
     <any type="record"/>
     <description>The region of interest.  Default is the whole image.</description>
     <value></value>
     </param>
  
     <param type="any" direction="in" name="mask">
     <any type="variant"/>    
     <description>OTF mask, Boolean LEL expression or mask region.  Default is none.</description>
     <value></value>
    </param>
  
     <param type="bool" direction="in" name="dropdeg">
     <description>Drop degenerate axes</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="overwrite">
     <description>Overwrite (unprompted) pre-existing output file?</description>
     <value>false</value>
     </param>
</input>
<returns type="casaimage"/>
<description>

This function applies a \region\ to a disk \imagefile, creates a new
\imagefile\ containing the (sub)image, and associates a new \imagetool\
with it. 

The input disk image file may be in native \casa, \fits (Float,
Double, Short, Long are supported), or Miriad format.  Look
\htmlref{here}{IMAGES:FOREIGNIMAGES} for more information on foreign
images.

If {\stfaf outfile} is given, the (sub)image is written to the specified
disk file.  

If {\stfaf outfile} is unset, the Image \tool\ actually references the
input image file.  So if you deleted the input image disk file, it
would render this \tool\ useless.  When you destroy this on-the-fly
\tool\ (with the <link anchor="images:image.done.function">done</link>
function) the reference connection is broken.

Sometimes it is useful to drop axes of length one (degenerate axes).
Use the {\stfaf dropdeg} argument if you want to do this.

The output mask is the combination (logical OR) of the default input
\pixelmask\ (if any) and the OTF mask.  Any other input \pixelmasks\
will not be copied.  Use function
<link anchor="images:image.maskhandler.function">maskhandler</link> if you need to copy other
masks too. 

See also the <link anchor="images:image.subimage.function">subimage</link> function.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t newimagefromimage Ex 1 \t----"
ia.maketestimage('test1',overwrite=true)
ia.maketestimage('test2',overwrite=true)
print ia.name()
#[...]test2
im1=ia.newimagefromimage('test1')
print im1.name()
#[...]test1
print im1.summary()
im2=ia.newimagefromimage('test2')
print im2.name()
#[...]test2
print im1.name()
#[...]test1
ia.close()
im1.done()
im2.done()
#
"""
\end{verbatim}
<!--The specified \region\ takes a quarter by area of the first two axes
of the image, and all pixels of other axes.-->
</example>
</method>

 
<method type="function" name="newimagefromshape">
   <shortdescription>Construct an empty \casa\ image from a shape</shortdescription>
   
<input>
  
     <param type="string" direction="in" name="outfile">
     <description>Name of output image file.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="intArray" direction="in" name="shape">
     <description>Shape of image.  Required.</description>
     <value>0</value>
     </param>
  
     <param type="any" direction="in" name="csys">
	     <any type="record"/>
     <description>Record describing Coordinate System.  Default is unset.</description>
     <value></value>
     </param>
  
     <param type="bool" direction="in" name="linear">
     <description>Make a linear Coordinate System if csys not given?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="overwrite">
     <description>Overwrite (unprompted) pre-existing output file?</description>
     <value>false</value>
     </param>
  
     <param type="bool" direction="in" name="log">
     <description>Write image creation messages to logger</description>
     <value>true</value>
     </param>
</input>
<returns type="casaimage"/>
<description>

This function creates a \casa\ \imagefile\ with the specified shape.  All
the pixel values in the image are set to 0.  

If {\stfaf outfile} is given, the image is written to the specified
disk file.  If {\stfaf outfile} is unset, the on-the-fly Image \tool\
returned by this function is associated with a temporary image.  This
temporary image may be in memory or on disk, depending on its size.
When you destroy the on-the-fly Image \tool\ (with the <link
anchor="images:image.done.function">done</link> function) this
temporary image is deleted.

The Coordinate System, provided as a record describing a <link
anchor="images:coordsys">Coordsys</link> \tool\ (created via
coordsys.torecord(), for instance), is optional.  If you provide it,
it must be dimensionally consistent with the pixels array you give
(see also <link
anchor="images:image.coordsys.function">coordsys</link>).

If you don't provide the Coordinate System, a default Coordinate System
is made for you.  If {\stfaf linear=F} (the default) then it is a
standard RA/DEC/Stokes/Spectral Coordinate System depending exactly upon
the shape (Stokes axis must be no longer than 4 pixels and you may find
the spectral axis coming out before the Stokes axis if say, {\cf
shape=[64,64,32,4]}).  Extra dimensions are given linear coordinates. 
If {\stfaf linear=T} then you get a linear Coordinate System. 

</description>
<example>
\begin{verbatim}
"""
#
print "\t----\t newimagefromshape Ex 1 \t----"
im1=ia.newimagefromshape('test2.data', [64,64,128], overwrite=true)
cs1 = im1.coordsys(axes=[0,2])
im1.done()
im2 = ia.newimagefromshape(shape=[10, 20], csys=cs1.torecord())
cs1.done()
im2.done()
#
"""
\end{verbatim}

The first example creates a zero-filled \imagefile\ named {\sff
test.data} of shape [64,64,128].  If you examine the header with
{\stff ia.summary()} you will see the RA/DEC/Spectral coordinate
information.  In the second example, a Coordinate System describing
the first and third axes of the image {\sff test2.data} is created and
used to create a 2D temporary image.
</example>
</method>

  
   <method type="function" name="makearray">
   <shortdescription>Construct an initialized multi-dimensional array.
   </shortdescription>

<input>
  
     <param type="double" direction="in" name="v">
     <description>Value with which to initial array elements</description>
     <value>0.0</value>
     </param>
     <param type="intArray" direction="in" name="shape">
     <description>Vector containing array dimensions.</description>
     <value>0</value>
     </param>

</input>
<returns type="any"><any type="variant"/></returns>
<description>
This function takes two arguments. The first argument is the initial
value for the new array.  The second is a vector giving the lengths of
the dimensions of the array.
</description>

<example>
A three dimensional array that is initialized to all zeros. Each of
the three dimensions of the cube has a length of four.
\begin{verbatim}
"""
#
print "\t----\t makearray Ex 1 \t----"
cube = ia.makearray(0,[4,4,4])
#
"""
\end{verbatim}
</example>
</method>


   <method type="function" name="setboxregion">
   <shortdescription>Set a pixel box region of interest in the image</shortdescription>
   
<input>
  
     <param type="doubleArray" direction="in" name="blc">
     <description>Bottom-Left-Corner (beginning) of pixel section.
     Default is blc of image.</description>
     <value>-1</value>
     </param>
  
     <param type="doubleArray" direction="in" name="trc">
     <description>Top-Right-Corner (end) of pixel section.
     Default is trc of image.</description>
     <value>-1</value>
     </param>
  
     <param type="bool" direction="in" name="frac">
     <description>Pixel or fractional coordinates</description>
     <value>false</value>
     </param>
  
     <param type="string" direction="in" name="infile">
     <description>Input image file name</description>
     <value></value>
     </param>
  
</input>
<returns type="any"><any type="record"/></returns>
<description>
Users should use rg.box() instead. The regionmanager wasn't
available when this method was created.
</description>

<example>
Make region box of inner quarter of the image.
\begin{verbatim}
"""
#
print "\t----\t setboxregion Ex 1 \t----"
ia.maketestimage()
box = ia.setboxregion([0.25,0.25],[0.75,0.75],frac=true)
ia.close()
#
"""
\end{verbatim}
</example>

</method>


   <method type="function" name="echo">
   <shortdescription>returns input record</shortdescription>

<input>
     <param type="any" direction="in" name="v">
     <any type="record"/>
     <description>a measure value</description>
     </param>

     <param type="bool" direction="in" name="godeep">
     <description>force the use of MeasureHolder record conversions</description>
     <value>false</value>
     </param>
</input>
<returns type="any">
<shortdescription>input measure</shortdescription>
<any type="record"/>
</returns>
<description>
echo returns input.  This method is for diagnostic purposes only.
</description>
<example>
\begin{verbatim}
"""
#
x=5
ia.echo(x)
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="outputvariant">
   <shortdescription>returns a variant</shortdescription>

<output>
     <param type="any" direction="out" name="v">
     <any type="variant"/>
     <description>a variant</description>
     </param>
</output>

<returns type="void">
<shortdescription>for testing variant as output parameter</shortdescription>
</returns>
<description>
returns internally generated v as a variant output parameter.
Exists for test purposes only.
</description>
<example>
\begin{verbatim}
"""
#
#ia.outputvariant()
exit() # This is last example so lets exit().
#
"""
\end{verbatim}
</example>
</method>

 
</tool>
</casaxml>
"""
