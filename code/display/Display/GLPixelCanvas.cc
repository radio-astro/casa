//# GLPixelCanvas.cc:	Implementation of PixelCanvas for OpenGL.
//# Copyright (C) 2001
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

//# GL files are ifdefed to avoid problems when OpenGL is not available.
#if defined(OGL)

/*
NOTES:
	Programs linked with GLPixelCanvas will also need:
		-lGLw -lGLU -lGL

 libGLw contains the OpenGL drawing area widget used by GLPixelCanvas. It
appears to be installed on Sun and SGI systems that also have OpenGL
installed. However, it is not typically installed on Linux systems. It can
be built from the Mesa source distribution. Look in the widgets-sgi
subdirectory. The widget's include files are also there.
 There are two widgets available:
	glwDrawingAreaWidgetClass
	glwMDrawingAreaWidgetClass

One is a Motif widget, the other an Xt widget. Either seems to work. There
is a #define a few lines below to choose which to use.

 GLPixelCanvas was derived from X11PixelCanvas.

 For multiple GLPixelCanvases, makeCurrent() must be called before calling
any other routines otherwise drawing will go to another window.
 ( makeCurrent() could be added to all routines making OpenGL calls at the
cost of increased overhead.

7/01
	There is a bug in SGI's (and other) GLX extensions that prevents
displaying textured images between systems that have the problem and those
that don't. (SGI <-> SGI works, SGI <-> Sun doesn't).

9/28/01
	Displaying from a Linux machine to another system using
 PseudoColor (Indexed) windows occasionally leads to a crash when
 displaying images. This may be a problem with NVIDIA's __glXFillImage
 routine.

*/

//# Includes
#include <strings.h>
#include <graphics/X11/X_enter.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
//# The motif widget comes in either Motif or non-Motif forms.
//#define MOTIF_WIDGET
#if defined(MOTIF_WIDGET)
#include <X11/GLw/GLwMDrawA.h>
#else
#include <X11/GLw/GLwDrawA.h>
#endif

#include <graphics/X11/X_exit.h>

#include <casa/aips.h>
#include <casa/Logging/LogIO.h>
#include <casa/BasicMath/Math.h>
#include <display/Display/ColorConversion.h>
#include <display/Display/GLPixelCanvas.h>
#include <display/Display/GLPCDisplayList.h>
#include <display/Display/GLPCTexture.h>

namespace casa { //# NAMESPACE CASA - BEGIN

////////////////////////////////////////////////////////////////

//# Print current read and write OGL buffers.
	void GLPixelCanvas::pRWbuf(const char *str=NULL, int nspaces=0) {
		int writeBuf, readBuf;

		glGetIntegerv(GL_DRAW_BUFFER, &writeBuf);
		glGetIntegerv(GL_READ_BUFFER, &readBuf);
		char buf[128];
		sprintf(buf, "Read Buffer: %s, write buffer: %s",
		        GLbufferToString(readBuf), GLbufferToString(writeBuf));
		log_.append(buf, nspaces);
	}

////////////////////////////////////////////////////////////////

//# Return the widget's width and height.
	void getwidthheight(Widget wid, uInt &width, uInt &height) {
		Arg     arg[5];
		Dimension       w,h;
		int             i;

		i = 0;
		XtSetArg(arg[i], XtNwidth, &w);
		i++;
		XtSetArg(arg[i], XtNheight, &h);
		i++;
		XtGetValues(wid, arg, i);
		width = (uInt)w;
		height = (uInt)h;
	}

///////////////////////////////////////////////////////////////////////
//# This is a display list entry that holds a GLPCTexturedImage object
//# which it destroys when it itself is destroyed.
	class TexturedEntry : public GLPCDisplayListEntry {
	public:
		TexturedEntry(GLPCTexturedImage *image,
		              const char * name = NULL,
		              GLenum mode=GL_COMPILE_AND_EXECUTE);
		virtual ~TexturedEntry();
	private:
		GLPCTexturedImage *image_;
	};

	TexturedEntry::TexturedEntry(GLPCTexturedImage *image,
	                             const char * name, GLenum mode) :
		GLPCDisplayListEntry(name, mode), image_(image) {
	}

	TexturedEntry::~TexturedEntry() {
		delete image_;
	}
////////////////////////////////////////////////////////////////

//# This is a display list element that holds data to redraw an image.
// It is used when drawing to Indexed windows (when textures aren't available).
	class ImageEntry : public GLPCDisplayListElement {
	public:
		ImageEntry(GLfloat x, GLfloat y, GLfloat z,
		           GLsizei width, GLsizei height,
		           GLfloat xscale, GLfloat yscale,
		           GLenum format, const GLushort *pixels,
		           const char *name=NULL);
		virtual void call(Bool force=False, const uInt nspaces=0);
		virtual ~ImageEntry();
	private:
		GLfloat	x_, y_, z_;
		GLsizei	width_, height_;
		GLfloat xscale_, yscale_;
		GLenum	format_;
		GLushort *pixels_;
	};

	ImageEntry::ImageEntry( GLfloat x, GLfloat y, GLfloat z,
	                        GLsizei width, GLsizei height,
	                        GLfloat xscale, GLfloat yscale,
	                        GLenum format, const GLushort *pixels,
	                        const char *name):
		GLPCDisplayListElement(name), x_(x), y_(y), z_(z),
		width_(width), height_(height), xscale_(xscale), yscale_(yscale),
		format_(format), pixels_(0) {
		int ncomponents, cnt;

		switch(format) {
		case GL_RGB:
			ncomponents = 3;
			break;
		case GL_LUMINANCE_ALPHA:
			ncomponents = 2;
			break;
		case GL_RGBA:
			ncomponents = 4;
		default:
			ncomponents = 1;
		}
		cnt = ncomponents*width*height;

		pixels_ = new GLushort [cnt];
		memcpy(pixels_, pixels, cnt*sizeof(*pixels));
	}

	void ImageEntry::call(Bool force, const uInt nspaces) {
		if(force || enabled()) {
			GLPixelCanvas::drawPixels(x_, y_, z_, width_, height_,
			                          xscale_, yscale_,
			                          format_, GL_UNSIGNED_SHORT, pixels_);
			traceCheck(nspaces);
		}
	}

	ImageEntry::~ImageEntry() {
		delete [] pixels_;
		pixels_ = NULL;
	}

////////////////////////////////////////////////////////////////

	GLPixelCanvas::GLPixelCanvas(Widget parent,
	                             GLPixelCanvasColorTable * glpcctbl,
	                             uInt width, uInt height)
		: PixelCanvas((PixelCanvasColorTable *) glpcctbl),
		  display_(XtDisplay(parent)),
		  parent_(parent),
		  visualInfo_(glpcctbl->visualInfo()),
		  visual_(glpcctbl->visual()),
		  glpcctbl_(glpcctbl),
		  context_(0),
		  width_(0),
		  height_(0),
		  depth_(glpcctbl->depth()),
		  doClipping_(False),
		  clipX1_(0),
		  clipY1_(0),
		  clipX2_(width-1),
		  clipY2_(height-1),
		  itsDeviceForegroundColor_("white"),
		  itsDeviceBackgroundColor_("black"),
		  lineStyle_(Display::LSSolid),
		  exposeHandlerFirstTime_(True),
		  listSize_(0), numinuse_(0), nextfree_(0), sizeincr_(16),
		  currentListID_(0),
		  dlists_(NULL), currentElement_(NULL),
		  trace_(False), traceLevel_(GLTraceNormal), autoRefresh_(True) {
		// What buffers to clear
		bufferMask_ = GL_COLOR_BUFFER_BIT;
		itsComponents_ = NULL;
		initComponents();
		currentColor_.index = 0;
		currentColor_.red = 0.0;
		currentColor_.green = 0.0;
		currentColor_.blue = 0.0;
		currentColor_.alpha = 1.0;
		clearColor_.index = 0;
		clearColor_.red = 0.0;
		clearColor_.green = 0.0;
		clearColor_.blue = 0.0;
		clearColor_.alpha = 1.0;

		nspaces_ = 0;
//  trace(True);		//# For debugging.
		makeList("RepaintList"); //# list 0 is used to repaint window.

		//# Pick up useful information.
		{
			int hasGL, isRGB, isdouble;
			glXGetConfig(display_, visualInfo_, GLX_USE_GL, &hasGL);
			if(!hasGL)
				throw(AipsError("GLPixelCanvas: visual does not support OpenGL."));

			glXGetConfig(display_, visualInfo_, GLX_RGBA, &isRGB);
			isRGB_ = (isRGB != 0) ? True : False;

			glXGetConfig(display_, visualInfo_, GLX_DOUBLEBUFFER, &isdouble);
			doubleBuffered_ = (isdouble != 0) ? True : False;
		}

		/////!! Should check '*color' resources for valid values.
		form_ = XtVaCreateManagedWidget("form",
		                                xmFormWidgetClass, parent_,
		                                XmNleftAttachment, XmATTACH_FORM,
		                                XmNrightAttachment, XmATTACH_FORM,
		                                XmNtopAttachment, XmATTACH_FORM,
		                                XmNbottomAttachment, XmATTACH_FORM,
		                                NULL);


//# The glw(M)DrawingAreaWidgetClass is used because it takes a visualInfo
//# arg. If a different kind of widget were used, it would be necessary
//# to create the top most widget with the correct visual.
#if defined(MOTIF_WIDGET)
		WidgetClass wclass = glwMDrawingAreaWidgetClass;
#else
		WidgetClass wclass = glwDrawingAreaWidgetClass;
#endif

		drawArea_ = XtVaCreateManagedWidget("GLDisplay",
		                                    wclass,
		                                    form_,
		                                    XmNleftAttachment, XmATTACH_FORM,
		                                    XmNrightAttachment, XmATTACH_FORM,
		                                    XmNtopAttachment, XmATTACH_FORM,
		                                    XmNbottomAttachment, XmATTACH_FORM,
		                                    XmNwidth, width,
		                                    XmNheight, height,
		                                    XmNcolormap, glpcctbl_->xcmap(),
		                                    GLwNvisualInfo, visualInfo_,
		                                    NULL);

		context_ = glXCreateContext(display_, visualInfo_, NULL, True);

		//# Button and Mouse Event Handler
		XtAddEventHandler (drawArea_,
		                   ButtonPressMask | ButtonReleaseMask
		                   | KeyPressMask | KeyReleaseMask | PointerMotionMask
		                   | ExposureMask | StructureNotifyMask,
		                   False,
		                   (XtEventHandler) GLPixelCanvas::handleEventsCB,
		                   this);

		//# Colortable resize callback
		glpcctbl->addResizeCallback
		((PixelCanvasColorTableResizeCB) colorTableResizeCB, this);

		if (glpcctbl->colorModel() == Display::Index)
			setColormap(glpcctbl->defaultColormap());
	}

	GLPixelCanvas::~GLPixelCanvas() {
		initComponents();
	}

	void GLPixelCanvas::initComponents() {
		itsComponentWidth_ = itsComponentHeight_ = 0;
		itsComponentX_ = itsComponentY_ = itsComponentZ_ = 0.0;
		itsComponentScaleX_ = itsComponentScaleY_ = 1.0;
		if(itsComponents_ != NULL)
			delete [] itsComponents_;
		itsComponents_ = NULL;
	}

	void GLPixelCanvas::enableMotionEvents() {
	}

	void GLPixelCanvas::disableMotionEvents() {
	}

	void GLPixelCanvas::enablePositionEvents() {
	}

	void GLPixelCanvas::disablePositionEvents() {
	}

////////////////////////////////////////////////////////////////
///		Display list support
////////////////////////////////////////////////////////////////

//# Does this canvas support cached display lists?  The user of the
//# canvas should always check this, because undefined behaviour can
//# result when an attempt is made to use a list on a PixelCanvas
//# which does not support lists.
	Bool GLPixelCanvas::supportsLists() {
		return True;
	}

//# Notes on Display lists.
//	OpenGL windows are not automatically redrawn when uniconified
//# or unobscured. To emulate the ability, GLPixelCanvas saves what
//# calls it can in a display list. When an expose event arrives, the
//# display list is called. The mechanism is complicated by the existence
//# of user display lists and the fact that display lists can call display
//# lists.
//# Each GLPixelCanvas display list consists of a list of 'entries' which
//# can contain either an OpenGL display list ID or another GLPC display list.
//# When a non GLPC display list element is created, it calls glNewList which
//# tells OpenGL to record succeeding OpenGL commands.
//# Each 'cacheable' GLPixelCanvas routine creates a new display list
//# element. This isn't necessarily efficient but it isn't possible to
//# control what the user does between calls.
//# Display list 0 is the list used to store repainting information.
//# Whenever the user calls 'drawList', that list's entry is called and
//# added to display list 0.
//# Deleting display lists is deferred until the window resizes or clear
//# is called since it's possible for a list that's been marked as deleted
//# to still be needed for refresh.

//# Return a pointer to a new GLPC display list.
	GLPCDisplayList *GLPixelCanvas::createList(const char *Name) {
		const char *name = (Name == NULL) ? "RepaintList" : Name;
		GLPCDisplayList *list = new GLPCDisplayList(name);

		numinuse_ += 1;			//# Count # of lists in use.
		list->ref();			//# Mark as in use.
		list->trace(trace_);		//# Trace calls?
		return list;
	}

//# Create a new display list and add to our list of lists.
	uInt GLPixelCanvas::makeList(const char *name) {
		uInt listid = ~0;
		String n;
		GLPCDisplayList *list;
		static uInt serial = 0;	//# Give each list we create a sn.

		//# Find a free slot.
		if(numinuse_ < nextfree_) {	//# Reuse an old slot?
			for(uInt i = 1; i < nextfree_; i++) //# Index 0 is special.
				if(dlists_[i] == NULL) {
					listid = i;
					break;
				}
		}

		//# Separate from above 'if' in case searching messed up somehow.
		if(listid  > nextfree_) {	//# Add to end.
			if(nextfree_ >= listSize_)
				resizeList();
			listid = nextfree_++;
		}

		if(name == NULL)
			n = "UserList";
		else
			n = name;

		//# Append listid and a serial number to name.
		{
			char buf[32];
			sprintf(buf,"[%02d,%02d]", listid, serial++);
			n += buf;
		}
		list = createList(n);

		dlists_[listid] = list;

		return listid;
	}

//# begin caching display commands - return list ID
	uInt GLPixelCanvas::newList() {
		uInt listid;

		if(currentListID_ != 0)
			endList();		//# Stop current recording.
		listid = makeList();
		currentListID_ = listid;
		dlists_[listid]->ref();		//# In use by caller.
		{
			char buf[32];
			sprintf(buf, "newList(%d)", listid);
			traceCheck(buf, GLTraceCalls, nspaces_++);
		}
		return listid;
	}

//# End caching display commands
	void GLPixelCanvas::endList() {
		currentListID_ = 0;		//# Go back to default list.
		nspaces_--;
	}

//# (Cacheable) recall cached display commands
	void GLPixelCanvas::drawList(uInt listid) {
		//# Check for valid list id (0 is reserved).
		if((listid == 0) || (listid >= nextfree_)) {
			char buf[32];
			sprintf(buf, "Invalid list id: %d\n", listid);
			warn("drawList", buf, WHERE);
			return;
		}

		GLPCDisplayList *list = dlists_[listid];

		if(list == NULL) {
			warn("drawList", "NULL displaylist.", WHERE);
			return;
		}

		//# In order to handle translations for display lists, we
		//# clone the list so any subsequent translations don't affect
		//# this call. NOTE: If the original list is deleted, this list
		//# will still exist. (Not the same semantics as OpenGL).
		GLPCDisplayList *wlist = new GLPCDisplayList(*list);
		wlist->trace(trace_);

		//# Add to current list.
		dlists_[currentListID_]->add(wlist);

		if(currentListID_ == 0)		//# Draw if not currently recording.
			wlist->call(False, nspaces_);
	}

//# translate all lists
	void GLPixelCanvas::translateAllLists(Int xt, Int yt) {
		for(uInt i=1; i< nextfree_; i++)
			translateList(i, xt, yt);
	}

//# translate the list
	void GLPixelCanvas::translateList(uInt list, Int xt, Int yt) {
		//# Check for valid list id (0 is reserved).
		if((list == 0) || (list >= nextfree_) || (dlists_[list] == NULL) ||
		        (dlists_[list]->disabled()))
			return;
		dlists_[list]->translate((GLfloat)xt, (GLfloat)yt);
	}

//# Make our list of display lists larger.
	void GLPixelCanvas::resizeList() {
		uInt newsize = listSize_ + sizeincr_;
		GLPCDisplayList **list = new GLPCDisplayList *[newsize];

		if(dlists_ != NULL) {
			memcpy(list, dlists_, sizeof(*list)*nextfree_);;
			delete [] dlists_;
		} else
			list[0] = NULL;

		dlists_ = list;
		listSize_ = newsize;
	}

//# remove list from cache
	void GLPixelCanvas::deleteList(uInt listID) {
		if(!validList(listID))
			return;

		dlists_[listID]->unref();	//# Remove user's ref.
		removeList(listID);		//# Remove from list table.
		cleanLists();			//# Any lists freed by this one.
	}

//# flush all lists from the cache
	void GLPixelCanvas::deleteLists() {
		for(uInt i=1; i< nextfree_; i++)
			deleteList(i);
	}

//# return True if the list exists
	Bool GLPixelCanvas::validList(uInt list) {
		//# Check for valid list id (0 is reserved).
		if((list == 0) || (list >= nextfree_) || (dlists_[list] == NULL))
			return False;
		else
			return True;
	}

//# Remove list from list of display lists.
	Boolean GLPixelCanvas::removeList(uInt listID) {
		//# Check for valid list id.
		if(listID >= nextfree_)		//# Valid value?
			return False;

		//# Make sure it hasn't already been deleted.
		if(dlists_[listID] == NULL)
			return False;

		dlists_[listID]->unref();	//# Remove dlists's ref.
		dlists_[listID] = NULL;
		//# Don't need to actually delete since displayListElements
		//# self delete when their reference counts go to 0.
		//# Also, it might be in another list.

		if(numinuse_ > 0)
			numinuse_ -= 1;
		else {
			log_ << LogIO::SEVERE
			     << LogOrigin( "GLPixelCanvas", "removeList", WHERE)
			     << "numinuse_ tried to go below 0 for listid "
			     << listID << LogIO::POST;
		}
		return True;
	}

//# Called when window gets resized or is cleared.
//# Delete & create a new repaint list (listid 0).
//# Delete lists that aren't being used.
	void GLPixelCanvas::purgeLists() {
		removeList(0);
		//# Remove any lists only needed by list 0.
		cleanLists();
		dlists_[0] = createList("RepaintList");
	}

//# Go through the list of display lists removing any unused list.
	void GLPixelCanvas::cleanLists() {
		uInt knt = numinuse_;

		for(uInt i=1; i < nextfree_; i++) {	//# 0 is special.
			//# Remove any list whose only reference count is the
			//# one for being in the table.
			if((dlists_[i] != NULL) && (dlists_[i]->useCount() <= 1))
				removeList(i);
		}

		//# A removed list might free a list earlier in dlists_.
		if((knt - numinuse_) > 0)	//# Keep trying until
			cleanLists();		//# no change.
	}

//# Add a new recording element to current list.
//# 'Cacheable' GLPixelCanvas routines call this before calling OpenGL
//# commands and call endListEntry() when done.
//# If drawing to the default list, commands are saved and executed.
//# Otherwise, they're just saved.
	void GLPixelCanvas::beginListEntry(GLPCDisplayListElement *e) {
		if(currentElement_ != NULL) {
			warn("beginListEntry", "beginListEntry called while active.",
			     WHERE);
		}

		currentElement_ = e;
		currentElement_->trace(trace_);	//# Debug?
		dlists_[currentListID_]->add(currentElement_);
		currentElement_->start();
	}

	void GLPixelCanvas::beginListEntry(const char *name) {
		// If recording, currentListID_ != 0, just compile. Otherwise,
		// compile and execute (display).
		GLenum mode = (currentListID_ == 0) ? GL_COMPILE_AND_EXECUTE
		              : GL_COMPILE;
		GLPCDisplayListEntry *e = new GLPCDisplayListEntry(name, mode);
		beginListEntry(e);
	}

//# Tell the current element to stop recording then reset current element ptr.
	void GLPixelCanvas::endListEntry() {
		if(currentElement_ == NULL) {
			warn("endListEntry", "endListEntry called when not caching.",
			     WHERE);
		}
		currentElement_->stop();
		currentElement_ = NULL;
	}

//# Add a note to be displayed when tracing is enabled.
	void GLPixelCanvas::note(const char *str) {
		if(str != NULL) {
			GLPCDisplayListElement *e = new GLPCDisplayListEntry(str);
			beginListEntry(e);	//# The note is the entry name.
			endListEntry();		//# So nothing to cache.
			traceCheck(str, GLTraceCalls);
		}
	}

////////////////////////////////////////////////////////////////
//# (Cacheable) Set the font to the recognizable font name
	Bool GLPixelCanvas::setFont(const String &fontName) {
		static Boolean print = True;

		if(print) {
			unimplemented("setFont", WHERE);
			print = False;
		}
		return False;
	}

//# (Cacheable) Draw text using that font aligned in some way to the
//# position
	void GLPixelCanvas::drawText(Int x, Int y, const String &text,
	                             Display::TextAlign alignment) {
		static Boolean print = True;

		if(print) {
			unimplemented("drawText", WHERE);
			print = False;
		}
	}
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
//			Images
//# Convert a float (0..1) to unsigned int (0..65535).
	static inline uShort f2us(const Float f) {
		static const Float SCL = 65535.0;
		return (uShort)((f*SCL) + 0.5);
	}

//# (Cacheable) Draw an array of 2D color data as a raster image
// for zoom = <1,1>
	void GLPixelCanvas::drawImage(const Matrix<uInt> &data, Int x, Int y) {
		uInt ny = data.nrow();
		uInt nx = data.ncolumn();
		Display::ColorModel cm = glpcctbl()->colorModel();
		uInt mult = (cm == Display::Index) ? 1 : 3;
		uShort *d = new uShort[nx*ny*mult], *dptr;
		dptr = d;
		GLenum format;

		if(mult == 1) {	//# Index
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uShort v = (uShort)data(x, y);
					*dptr++ = v;
				}
			}
			format = GL_COLOR_INDEX;
		} else {	//# RGB/HSV
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uLong v = (uLong)data(x, y);
					Float a, b, c;
					//# Unpack color information.
					glpcctbl_->pixelToComponents( v, a, b, c);
					*dptr++ = f2us(a);
					*dptr++ = f2us(b);
					*dptr++ = f2us(c);
				}
			}
			format = GL_RGB;
		}

		traceCheck("drawImage(M<uI>II)", GLTraceImages, nspaces_++);
		drawGLImage((GLfloat)x, (GLfloat)y, 0.0, nx, ny, 1.0, 1.0, format, d);
		nspaces_ -= 1;
		delete [] d;
	}

	void GLPixelCanvas::drawImage(const Matrix<Int> &data, Int x, Int y) {
		uInt ny = data.nrow();
		uInt nx = data.ncolumn();
		Display::ColorModel cm = glpcctbl()->colorModel();
		uInt mult = (cm == Display::Index) ? 1 : 3;
		uShort *d = new uShort[nx*ny*mult], *dptr;
		dptr = d;
		GLenum format;

		if(mult == 1) {	//# Index
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uShort v = (uShort)data(x, y);
					*dptr++ = v;
				}
			}
			format = GL_COLOR_INDEX;
		} else {	//# RGB/HSV
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uLong v = (uLong)data(x, y);
					Float a, b, c;
					//# Unpack color information.
					glpcctbl_->pixelToComponents( v, a, b, c);
					*dptr++ = f2us(a);
					*dptr++ = f2us(b);
					*dptr++ = f2us(c);
				}
			}
			format = GL_RGB;
		}

		traceCheck("drawImage(M<I>II)", GLTraceImages, nspaces_++);
		drawGLImage((GLfloat)x, (GLfloat)y, 0.0, nx, ny, 1.0, 1.0, format, d);
		nspaces_ -= 1;
		delete [] d;
	}

	void GLPixelCanvas::drawImage(const Matrix<uLong> &data, Int x, Int y) {
		uInt ny = data.nrow();
		uInt nx = data.ncolumn();
		Display::ColorModel cm = glpcctbl()->colorModel();
		uInt mult = (cm == Display::Index) ? 1 : 3;
		uShort *d = new uShort[nx*ny*mult], *dptr;
		dptr = d;
		GLenum format;

		if(mult == 1) {	//# Index
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uShort v = (uShort)data(x, y);
					*dptr++ = v;
				}
			}
			format = GL_COLOR_INDEX;
		} else {	//# RGB/HSV
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uLong v = (uLong)data(x, y);
					Float a, b, c;
					//# Unpack color information.
					glpcctbl_->pixelToComponents( v, a, b, c);
					*dptr++ = f2us(a);
					*dptr++ = f2us(b);
					*dptr++ = f2us(c);
				}
			}
			format = GL_RGB;
		}

		traceCheck("drawImage(M<uL>II)", GLTraceImages, nspaces_++);
		drawGLImage((GLfloat)x, (GLfloat)y, 0.0, nx, ny, 1.0, 1.0, format, d);
		nspaces_--;
		delete [] d;
	}

	void GLPixelCanvas::drawImage(const Matrix<Float> &data, Int x, Int y) {
		uInt ny = data.nrow();
		uInt nx = data.ncolumn();
		Display::ColorModel cm = glpcctbl()->colorModel();
		uInt mult = (cm == Display::Index) ? 1 : 3;
		uShort *d = new uShort[nx*ny*mult], *dptr;
		dptr = d;
		GLenum format;

		if(mult == 1) {	//# Index
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uShort v = (uShort)data(x, y);
					*dptr++ = v;
				}
			}
			format = GL_COLOR_INDEX;
		} else {	//# RGB/HSV
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uLong v = (uLong)data(x, y);
					Float a, b, c;
					//# Unpack color information.
					glpcctbl_->pixelToComponents( v, a, b, c);
					*dptr++ = f2us(a);
					*dptr++ = f2us(b);
					*dptr++ = f2us(c);
				}
			}
			format = GL_RGB;
		}

		traceCheck("drawImage(M<F>II)", GLTraceImages, nspaces_++);
		drawGLImage((GLfloat)x, (GLfloat)y, 0.0, nx, ny, 1.0, 1.0, format, d);
		nspaces_--;
		delete [] d;
	}

	void GLPixelCanvas::drawImage(const Matrix<Double> &data, Int x, Int y) {
		uInt ny = data.nrow();
		uInt nx = data.ncolumn();
		Display::ColorModel cm = glpcctbl()->colorModel();
		uInt mult = (cm == Display::Index) ? 1 : 3;
		uShort *d = new uShort[nx*ny*mult], *dptr;
		dptr = d;
		GLenum format;

		if(mult == 1) {	//# Index
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uShort v = (uShort)data(x, y);
					*dptr++ = v;
				}
			}
			format = GL_COLOR_INDEX;
		} else {	//# RGB/HSV
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uLong v = (uLong)data(x, y);
					Float a, b, c;
					//# Unpack color information.
					glpcctbl_->pixelToComponents( v, a, b, c);
					*dptr++ = f2us(a);
					*dptr++ = f2us(b);
					*dptr++ = f2us(c);
				}
			}
			format = GL_RGB;
		}

		traceCheck("drawImage(M<D>II)", GLTraceImages, nspaces_++);
		drawGLImage((GLfloat)x, (GLfloat)y, 0.0, nx, ny, 1.0, 1.0, format, d);
		nspaces_--;
		delete [] d;
	}

//# (Cacheable) Draw a masked array of 2D color data as a raster image,
	void GLPixelCanvas::drawImage(const Int &x, const Int &y,
	                              const Matrix<uInt> &data,
	                              const Matrix<Bool> &mask,
	                              Bool opaqueMask) {
		uInt ny = data.nrow();
		uInt nx = data.ncolumn();
		Display::ColorModel cm = glpcctbl()->colorModel();
		uInt mult = (cm == Display::Index) ? 1 : 3;
		uShort *d = new uShort[nx*ny*mult], *dptr;
		dptr = d;
		GLenum format;

		if(mult == 1) {	//# Index
			uShort backgroundColor = (uShort)getBackground();
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uShort v;
					if(mask(x, y))
						v = (uShort)data(x, y);
					else
						v = backgroundColor;
					*dptr++ = v;
				}
			}
			format = GL_COLOR_INDEX;
		} else {	//# RGB/HSV
			//# Assumes, probably incorrectly, background was set with a packed pixel.
			uLong backgroundColor = getBackground();
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uLong v;
					if(mask(x, y))
						v = (uLong)data(x, y);
					else
						v = backgroundColor;

					Float a, b, c;
					//# Unpack color information.
					glpcctbl_->pixelToComponents( v, a, b, c);
					*dptr++ = f2us(a);
					*dptr++ = f2us(b);
					*dptr++ = f2us(c);
				}
			}
			format = GL_RGB;
		}
		traceCheck("drawImage(M<D>II)", GLTraceImages, nspaces_++);
		drawGLImage((GLfloat)x, (GLfloat)y, 0.0, nx, ny, 1.0, 1.0, format, d);
		nspaces_--;
		delete [] d;
	}

//# (Cacheable) Draw an array of 2D color data as a raster image for
//# any positive integer zoom
	void GLPixelCanvas::drawImage(const Matrix<uInt> &data, Int x, Int y,
	                              uInt xzoom, uInt yzoom) {
		uInt ny = data.nrow();
		uInt nx = data.ncolumn();
		Display::ColorModel cm = glpcctbl()->colorModel();
		uInt mult = (cm == Display::Index) ? 1 : 3;
		uShort *d = new uShort[nx*ny*mult], *dptr;
		dptr = d;
		GLenum format;

		if(mult == 1) {	//# Index
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uShort v = (uShort)data(x, y);
					*dptr++ = v;
				}
			}
			format = GL_COLOR_INDEX;
		} else {	//# RGB/HSV
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uLong v = (uLong)data(x, y);
					Float a, b, c;
					//# Unpack color information.
					glpcctbl_->pixelToComponents( v, a, b, c);
					*dptr++ = f2us(a);
					*dptr++ = f2us(b);
					*dptr++ = f2us(c);
				}
			}
			format = GL_RGB;
		}

		traceCheck("drawImage(M<uI>IIuIuI", GLTraceImages, nspaces_++);
		drawGLImage((GLfloat)x, (GLfloat)y, 1.0, nx, ny,
		            (GLfloat) xzoom, (GLfloat)yzoom, format, d);
		nspaces_--;
		delete [] d;
	}

	void GLPixelCanvas::drawImage(const Matrix<Int> &data, Int x, Int y,
	                              uInt xzoom, uInt yzoom) {
		uInt ny = data.nrow();
		uInt nx = data.ncolumn();
		Display::ColorModel cm = glpcctbl()->colorModel();
		uInt mult = (cm == Display::Index) ? 1 : 3;
		uShort *d = new uShort[nx*ny*mult], *dptr;
		dptr = d;
		GLenum format;

		if(mult == 1) {	//# Index
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uShort v = (uShort)data(x, y);
					*dptr++ = v;
				}
			}
			format = GL_COLOR_INDEX;
		} else {	//# RGB/HSV
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uLong v = (uLong)data(x, y);
					Float a, b, c;
					//# Unpack color information.
					glpcctbl_->pixelToComponents( v, a, b, c);
					*dptr++ = f2us(a);
					*dptr++ = f2us(b);
					*dptr++ = f2us(c);
				}
			}
			format = GL_RGB;
		}

		traceCheck("drawImage(M<uI>IIuIuI", GLTraceImages, nspaces_++);
		drawGLImage((GLfloat)x, (GLfloat)y, 1.0, nx, ny,
		            (GLfloat) xzoom, (GLfloat)yzoom, format, d);
		nspaces_--;
		delete [] d;
	}

	void GLPixelCanvas::drawImage(const Matrix<uLong> &data, Int x, Int y,
	                              uInt xzoom, uInt yzoom) {
		uInt ny = data.nrow();
		uInt nx = data.ncolumn();
		Display::ColorModel cm = glpcctbl()->colorModel();
		uInt mult = (cm == Display::Index) ? 1 : 3;
		uShort *d = new uShort[nx*ny*mult], *dptr;
		dptr = d;
		GLenum format;

		if(mult == 1) {	//# Index
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uShort v = (uShort)data(x, y);
					*dptr++ = v;
				}
			}
			format = GL_COLOR_INDEX;
		} else {	//# RGB/HSV
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uLong v = (uLong)data(x, y);
					Float a, b, c;
					//# Unpack color information.
					glpcctbl_->pixelToComponents( v, a, b, c);
					*dptr++ = f2us(a);
					*dptr++ = f2us(b);
					*dptr++ = f2us(c);
				}
			}
			format = GL_RGB;
		}

		traceCheck("drawImage(M<uL>IIuIuI)", GLTraceImages, nspaces_++);
		drawGLImage((GLfloat)x, (GLfloat)y, 0.0, nx, ny,
		            (GLfloat) xzoom, (GLfloat)yzoom, format, d);
		nspaces_--;
		delete [] d;
	}

	void GLPixelCanvas::drawImage(const Matrix<Float> &data, Int x0, Int y0,
	                              uInt xzoom, uInt yzoom) {
		uInt ny = data.nrow();
		uInt nx = data.ncolumn();
		Display::ColorModel cm = glpcctbl()->colorModel();
		uInt mult = (cm == Display::Index) ? 1 : 3;
		const int NELEMENTS = nx*ny*mult;
		uShort *d = new uShort[NELEMENTS], *dptr;
		dptr = d;
		GLenum format;

		if(mult == 1) {	//# Index
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uShort v = (uShort)data(x, y);
					*dptr++ = v;
				}
			}
			format = GL_COLOR_INDEX;
		} else {	//# RGB/HSV
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uLong v = (uLong)data(x, y);
					Float a, b, c;
					//# Unpack color information.
					glpcctbl_->pixelToComponents( v, a, b, c);
					*dptr++ = f2us(a);
					*dptr++ = f2us(b);
					*dptr++ = f2us(c);
				}
			}
			format = GL_RGB;
		}

		traceCheck("drawImageM<F>", GLTraceImages, nspaces_++);
		drawGLImage((GLfloat)x0, (GLfloat)y0, 0.0, nx, ny,
		            (GLfloat) xzoom, (GLfloat)yzoom, format, d);
		nspaces_--;
		delete [] d;
	}

	void GLPixelCanvas::drawImage(const Matrix<Double> &data, Int x0, Int y0,
	                              uInt xzoom, uInt yzoom) {
		uInt ny = data.nrow();
		uInt nx = data.ncolumn();
		Display::ColorModel cm = glpcctbl()->colorModel();
		uInt mult = (cm == Display::Index) ? 1 : 3;
		const int NELEMENTS = nx*ny*mult;
		uShort *d = new uShort[NELEMENTS], *dptr;
		dptr = d;
		GLenum format;

		if(mult == 1) {	//# Index
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uShort v = (uShort)data(x, y);
					*dptr++ = v;
				}
			}
			format = GL_COLOR_INDEX;
		} else {	//# RGB/HSV
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uLong v = (uLong)data(x, y);
					Float a, b, c;
					//# Unpack color information.
					glpcctbl_->pixelToComponents( v, a, b, c);
					*dptr++ = f2us(a);
					*dptr++ = f2us(b);
					*dptr++ = f2us(c);
				}
			}
			format = GL_RGB;
		}

		traceCheck("drawImageM<D>", GLTraceImages, nspaces_++);
		drawGLImage((GLfloat)x0, (GLfloat)y0, 0.0, nx, ny,
		            (GLfloat) xzoom, (GLfloat)yzoom, format, d);
		nspaces_--;
		delete [] d;
	}

#if 0
	static int check(char *buf, unsigned int s) {
		if(strlen(buf) >= s) {
			fprintf(stderr, "%s (%u>%u)\n", buf, strlen(buf), s);
			return 0;
		}
		return 1;
	}
#endif

	/* Draws an image using glDrawPixels rather than using textures. It can
	   not be placed in an OpenGL display list.
	   It is a static class function.
	*/
	void GLPixelCanvas::drawPixels( GLfloat x, GLfloat y, GLfloat z,
	                                GLsizei width, GLsizei height,
	                                GLfloat xscale, GLfloat yscale,
	                                GLenum format, GLenum type,
	                                const GLvoid *pixels) {
		GLfloat oxscale, oyscale;
		GLushort *ptr = (GLushort *) pixels;

		glGetFloatv(GL_ZOOM_X, &oxscale);
		glGetFloatv(GL_ZOOM_Y, &oyscale);
		glRasterPos3f(x, y, z);
		if((oxscale != xscale) || (oyscale != yscale))
			glPixelZoom(xscale, yscale);
		glDrawPixels(width, height, format, type, ptr);
		if((oxscale != xscale) || (oyscale != yscale))
			glPixelZoom(oxscale, oyscale);
	}

	/* Draws an image using pixel drawing routines rather than textures by
	   creating a GLPC display list object. The actual drawing will be done
	   by drawPixels.
	*/
	void GLPixelCanvas::drawImagePixels(GLfloat x, GLfloat y, GLfloat z,
	                                    GLsizei width, GLsizei height,
	                                    GLfloat xscale, GLfloat yscale,
	                                    GLenum format, const GLushort *pixels) {
		const char *Name;
		char name[128];

		if(trace_) {
			const char *fmt = GLformatToString(format);

			sprintf(name,
			        "drawImagePixels(%.2f, %.2f, %.2f, %d, %d, %.2f, %.2f, %s)",
			        x, y, z, width, height, xscale, yscale, fmt);
//		check(name, sizeof(name));
			Name = name;
		} else
			Name = "drawImagePixels";

		traceCheck(Name, GLTraceImages, nspaces_++);

		ImageEntry *entry = new ImageEntry(x, y, z, width, height,
		                                   xscale, yscale, format,
		                                   pixels, Name);
		beginListEntry(entry);
		endListEntry();
		nspaces_--;
	}

	/* Draw an image using textures by creating a GLPCTexturedImage object. */
	void GLPixelCanvas::drawTexturedImage(
	    GLfloat x, GLfloat y, GLfloat z,
	    GLsizei width, GLsizei height,
	    GLfloat xscale, GLfloat yscale,
	    GLenum format, const GLushort *pixels,
	    GLPCTextureParams *params) {
		const char *Name;
		char name[128];

		if(trace_) {
			const char *fmt;

			fmt = GLformatToString(format);
			sprintf(name,
			        "drawTexturedImage(%.2f, %.2f, %.2f, %d, %d, %.2f, %.2f, %s)",
			        x, y, z, width, height, xscale, yscale, fmt);
			Name = name;
		} else
			Name = "drawTexturedImage";

		traceCheck(Name, GLTraceImages, nspaces_++);
		GLPCTexturedImage *image =
		    new GLPCTexturedImage(width, height,
		                          pixels, params);
		GLenum mode = (currentListID_ == 0) ? GL_COMPILE_AND_EXECUTE :
		              GL_COMPILE;
		TexturedEntry *entry = new TexturedEntry(image, Name, mode);

		beginListEntry(entry);
		image->draw(x, y, z, xscale, yscale);
		endListEntry();

		nspaces_--;
	}

	/* Draw an image using textures or pixel drawing routines depending on
	   the visual type.
	*/
	void GLPixelCanvas::drawGLImage(GLfloat x, GLfloat y, GLfloat z,
	                                GLsizei width, GLsizei height,
	                                GLfloat xscale, GLfloat yscale,
	                                GLenum format, const GLushort *pixels) {
		if(isRGB_)
			drawTexturedImage(x, y, z, width, height, xscale, yscale,
			                  format, pixels);
		else	/* Indexed windows aren't supported by textures. */
			drawImagePixels(x, y, z, width, height, xscale, yscale,
			                format, pixels);
	}

//# Similar to drawImage except that for RGB, the color values are retrieved
//# from the virtual color table by using the matrix values as indices.
	void GLPixelCanvas::drawIndexedImage(const Matrix<uInt> &data,
	                                     Float x, Float y, Float z,
	                                     Float xscl, Float yscl) {
		uInt ny = data.nrow();
		uInt nx = data.ncolumn();
		Display::ColorModel cm = glpcctbl()->colorModel();
		uInt mult = (cm == Display::Index) ? 1 : 3;
		uShort *d = new uShort[nx*ny*mult], *dptr;
		dptr = d;
		GLenum format;

		if(mult == 1) {	//# Index
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uShort v = (uShort)data(y, x);
					*dptr++ = v;
				}
			}
			format = GL_COLOR_INDEX;
		} else {	//# RGB/HSV	?? How to handle HSV?
			for(uInt y = 0; y < ny; y++) {
				for(uInt x = 0; x < nx; x++) {
					uInt v = (uInt)data(y, x);
					Float a, b, c;
					//# Unpack color information.
					glpcctbl_->indexToRGB( v, a, b, c);
					*dptr++ = f2us(a);
					*dptr++ = f2us(b);
					*dptr++ = f2us(c);
				}
			}
			format = GL_RGB;
		}

		traceCheck("drawIndexedImage(M<uInt>FFFFF)", GLTraceImages, nspaces_++);
		drawGLImage((GLfloat)x, (GLfloat)y, (GLfloat)z,
		            nx, ny, xscl, yscl, format, d);
		nspaces_--;
		delete [] d;
	}

//# (Cacheable) Draw a component of a multi-channel image, storing it
//# in buffers until flushComponentImages() is called.
	void GLPixelCanvas::drawImage(const Matrix<uInt> &data,
	                              const Int &x, const Int &y,
	                              const Display::ColorComponent &colorcomponent) {
		if(currentListID_ != 0) {	//# Building a display list?
			warn("drawImage", "Component caching is not implemented.",
			     WHERE);
		}

		bufferComponent(data, x, y, colorcomponent);
	}

//# Fill one of the channel buffers.
	void GLPixelCanvas::bufferComponent(const Matrix<uInt> &data,
	                                    const Int &x, const Int &y,
	                                    const Display::ColorComponent
	                                    &colorcomponent) {
		if(itsComponents_ == NULL) {
			itsComponentX_ = static_cast<Float>(x);
			itsComponentY_ = static_cast<Float>(y);
			itsComponentWidth_ = data.ncolumn();
			itsComponentHeight_ = data.nrow();
			long cnt = itsComponentHeight_*itsComponentWidth_*3;
			itsComponents_ = new uShort [cnt];
		}
		uShort *ptr = itsComponents_;
		//# Offsets: Red=0, Green=1, Blue=2.
		switch(colorcomponent) {
		case Display::Red:
		case Display::Hue:
			break;
		case Display::Green:
		case Display::Saturation:
			ptr += 1;
			break;
		case Display::Blue:
		case Display::Value:
			ptr += 2;
			break;
		}

		{
			uLong w, h;
			h = data.nrow();
			w = data.ncolumn();

			if((h != itsComponentHeight_) || (w != itsComponentWidth_)) {
				log_ << LogIO::WARN
				     << LogOrigin( "GLPixelCanvas",
				                   "bufferComponent", WHERE)
				     << "Component size (" << w << ", " << h
				     << ") does not match ("
				     << itsComponentWidth_ << ", " << itsComponentHeight_
				     << ")"
				     << LogIO::POST;
				return;
			}
		}

		// Copy columns from input array to rows of output array (transpose).
		for(uInt x = 0; x < itsComponentWidth_; x++)
			for(uInt y = 0; y < itsComponentHeight_; y++) {
				uShort v = (uShort)data(y,x);
				*ptr = v;
				ptr += 3;
			}
	}

//# (NOT CACHEABLE!) Flush the component buffers.
	static inline float clamp(const float x) {
		return (x <= 0.0) ? 0.0 : (x <= 1.0) ? x : 1.0;
	}

	void GLPixelCanvas::flushComponentBuffers() {
		uInt maxcolor1, maxcolor2, maxcolor3;
		uInt nelements = itsComponentWidth_*itsComponentHeight_*3;

		glpcctbl()->nColors(maxcolor1, maxcolor2, maxcolor3);
		maxcolor1 -= 1;	//# Max value for each component.
		maxcolor2 -= 1;
		maxcolor3 -= 1;

		if( itsComponents_ != NULL) {
			if(glpcctbl()->colorModel() == Display::HSV) {
				//# do HSV -> RGB
				for(uInt i=0; i < nelements; i += 3) {
					float h, s, v, r, g, b;
					//# Convert to floats in range of [0..1].
					h = (float)(itsComponents_[i])/maxcolor1;
					h = clamp(h);
					s = (float)(itsComponents_[i+1])/maxcolor2;
					s = clamp(s);
					v = (float)(itsComponents_[i+2])/maxcolor3;
					v = clamp(v);
					hsvToRgb(h, s, v, r, g, b);
					itsComponents_[i] = f2us(r);
					itsComponents_[i+1] = f2us(g);
					itsComponents_[i+2] = f2us(b);
				}
			} else {	//# Convert to GL_UNSIGNED_SHORT in range 0.. 65535.
				Float rscl = 65535.0/maxcolor1;
				Float gscl = 65535.0/maxcolor2;
				Float bscl = 65535.0/maxcolor3;

				for(uInt i=0; i < nelements; i += 3) {
					itsComponents_[i] = (uShort)(itsComponents_[i]*rscl + 0.5);
					itsComponents_[i+1] = (uShort)(itsComponents_[i+1]*gscl + 0.5);
					itsComponents_[i+2] = (uShort)(itsComponents_[i+2]*bscl + 0.5);
				}
			}

			traceCheck("flushComponentBuffers", GLTraceImages, nspaces_++);
			drawGLImage( itsComponentX_, itsComponentY_, itsComponentZ_,
			             itsComponentWidth_, itsComponentHeight_,
			             itsComponentScaleX_, itsComponentScaleY_,
			             GL_RGB, itsComponents_);
			nspaces_--;
			initComponents();
		}
	}

	void GLPixelCanvas::setPointSize(const uInt pointsize) {
		char Name[32];

		sprintf(Name,"setPointSize(%d)", pointsize);

		beginListEntry(Name);
		glPointSize((GLint)pointsize);
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

//# (Cacheable) Draw a single point using current color
	void GLPixelCanvas::drawPoint(Int x1, Int y1) {
		beginListEntry("drawPoint(II)");
		glBegin(GL_POINTS);
		glVertex2i(x1, y1);
		glEnd();
		endListEntry();
		traceCheck("drawPoint(II)", GLTraceGraphics);
	}

	void GLPixelCanvas::drawPoint(Float x1, Float y1) {
		beginListEntry("DrawPoint(FF)");
		glBegin(GL_POINTS);
		glVertex2f(x1, y1);
		glEnd();
		endListEntry();
		traceCheck("drawPoint(FF)", GLTraceGraphics);
	}

	void GLPixelCanvas::drawPoint3D(Float x1, Float y1, Float z1) {
		beginListEntry("drawPoint3D(FFF)");
		glBegin(GL_POINTS);
		glVertex3f(x1, y1, z1);
		glEnd();
		endListEntry();
		traceCheck("drawPoint3D(FFF)", GLTraceGraphics);
	}

	void GLPixelCanvas::drawPoint(Double x1, Double y1) {
		beginListEntry("drawPoint(DD)");
		glBegin(GL_POINTS);
		glVertex2d(x1, y1);
		glEnd();
		endListEntry();
		traceCheck("drawPoint(DD)", GLTraceGraphics);
	}

//# (Cacheable) Draw N points specified as a Nx2 matrix
	void GLPixelCanvas::drawPoints(const Matrix<Int> &verts) {
		uInt nPoints = verts.nrow();

		beginListEntry("drawPoints(M<I>)");
		glBegin(GL_POINTS);
		for(uInt i = 0; i < nPoints; i++)
			glVertex2i(verts(i, 0), verts(i, 1));
		glEnd();
		endListEntry();
		traceCheck("drawPoints(M<I>)", GLTraceGraphics);
	}

	void GLPixelCanvas::drawPoints(const Matrix<Float> &verts) {
		uInt nPoints = verts.nrow();

		beginListEntry("drawPoints(M<F>)");
		glBegin(GL_POINTS);
		for(uInt i = 0; i < nPoints; i++)
			glVertex2f(verts(i, 0), verts(i, 1));
		glEnd();
		endListEntry();
		traceCheck("drawPoints(M<F>)", GLTraceGraphics);
	}

	void GLPixelCanvas::drawPoints3D(const Matrix<Float> &verts) {
		uInt nPoints = verts.nrow();

		beginListEntry("drawPoints3D(M<F>)");
		glBegin(GL_POINTS);
		for(uInt i = 0; i < nPoints; i++)
			glVertex3f(verts(i, 0), verts(i, 1), verts(i, 2));
		glEnd();
		endListEntry();
		traceCheck("drawPoints3D(M<F>)", GLTraceGraphics);
	}

	void GLPixelCanvas::drawPoints(const Matrix<Double> &verts) {
		uInt nPoints = verts.nrow();

		beginListEntry("drawPointsM<D>");
		glBegin(GL_POINTS);
		for(uInt i = 0; i < nPoints; i++)
			glVertex2f(verts(i, 0), verts(i, 1));
		glEnd();
		endListEntry();
		traceCheck("drawPoints(M<D>)", GLTraceGraphics);
	}

//# (Cacheable) Draw a bunch of points using current color
	void GLPixelCanvas::drawPoints(const Vector<Int> &x1, const Vector<Int> &y1) {
		uInt nPoints = x1.nelements();

		beginListEntry("drawPointsV<I>");
		glBegin(GL_POINTS);
		for(uInt i = 0; i < nPoints; i++)
			glVertex2i(x1(i), y1(i));
		glEnd();
		endListEntry();
		traceCheck("drawPoints(V<I>,V<I>)", GLTraceGraphics);
	}

	void GLPixelCanvas::drawPoints( const Vector<Float> &x1,
	                                const Vector<Float> &y1) {
		uInt nPoints = x1.nelements();

		beginListEntry("drawPointsV<F>");
		glBegin(GL_POINTS);
		for(uInt i = 0; i < nPoints; i++)
			glVertex2f(x1(i), y1(i));
		glEnd();
		endListEntry();
		traceCheck("drawPoints(V<F>,V<F>)", GLTraceGraphics);
	}

	void GLPixelCanvas::drawPoints3D(const Vector<Float> &x1,
	                                 const Vector<Float> &y1, const Vector<Float> &z1) {
		uInt nPoints = x1.nelements();

		beginListEntry("drawPoints3DV<F>");
		glBegin(GL_POINTS);
		for(uInt i = 0; i < nPoints; i++)
			glVertex3f(x1(i), y1(i), z1(i));
		glEnd();
		endListEntry();
		traceCheck("drawPoints3D(V<F>,V<F>,V<F>)", GLTraceGraphics);
	}

	void GLPixelCanvas::drawPoints(const Vector<Double> &x1,
	                               const Vector<Double> &y1) {
		uInt nPoints = x1.nelements();

		beginListEntry("drawPointsV<D>V<D>");
		glBegin(GL_POINTS);
		for(uInt i = 0; i < nPoints; i++)
			glVertex2d(x1(i), y1(i));
		glEnd();
		endListEntry();
		traceCheck("drawPoints(V<D>,V<D>)", GLTraceGraphics);
	}

//# (Cacheable) Draw a single line using current color
	void GLPixelCanvas::drawLine(Int x1, Int y1, Int x2, Int y2) {
		beginListEntry("drawLine(IIII)");
		glBegin(GL_LINES);
		glVertex2i(x1, y1);
		glVertex2i(x2, y2);
		glEnd();
		endListEntry();
		traceCheck("drawLine(IIII)", GLTraceGraphics);
	}

	void GLPixelCanvas::drawLine(Float x1, Float y1, Float x2, Float y2) {
		char name[64];

		sprintf(name, "drawLine(%.2f, %.2f, %.2f, %.2f)", x1, y1, x2, y2);
//  check(name, sizeof(name));

		beginListEntry(name);
		glBegin(GL_LINES);
		glVertex2f(x1, y1);
		glVertex2f(x2, y2);
		glEnd();
		endListEntry();
		traceCheck(name, GLTraceGraphics);
	}

	void GLPixelCanvas::drawLine3D(Float x1, Float y1, Float z1,
	                               Float x2, Float y2, Float z2) {
		beginListEntry("drawLine3D(FFFFFF)");
		glBegin(GL_LINES);
		glVertex3f(x1, y1, z1);
		glVertex3f(x2, y2, z2);
		glEnd();
		endListEntry();
		traceCheck("drawLine3D(FFFFFF)", GLTraceGraphics);
	}

	void GLPixelCanvas::drawLine(Double x1, Double y1, Double x2, Double y2) {
		beginListEntry("drawLine(DDDD)");
		glBegin(GL_LINES);
		glVertex2d(x1, y1);
		glVertex2d(x2, y2);
		glEnd();
		endListEntry();
		traceCheck("drawLine(DDDD)", GLTraceGraphics);
	}

//# (Cacheable) Draw N/2 lines from an Nx2 matrix
	void GLPixelCanvas::drawLines(const Matrix<Int> &verts) {
		uInt nVerts = verts.nrow();
		beginListEntry("drawLines(M<I>)");
		glBegin(GL_LINES);
		for(uInt i=0; i < nVerts; i += 2) {
			glVertex2i(verts(i, 0), verts(i, 1));
			glVertex2i(verts(i+1, 0), verts(i+1, 1));
		}
		glEnd();
		endListEntry();
		traceCheck("drawLines(M<I>)", GLTraceGraphics);
	}

	void GLPixelCanvas::drawLines(const Matrix<Float> &verts) {
		uInt nVerts = verts.nrow();
		beginListEntry("drawLines(M<F>)");
		glBegin(GL_LINES);
		for(uInt i=0; i < nVerts; i += 2) {
			glVertex2f(verts(i, 0), verts(i, 1));
			glVertex2f(verts(i+1, 0), verts(i+1, 1));
		}
		glEnd();
		endListEntry();
		traceCheck("drawLines(M<F>)", GLTraceGraphics);
	}

	void GLPixelCanvas::drawLines3D(const Matrix<Float> &verts) {
		uInt nVerts = verts.nrow();
		beginListEntry("drawLines3D(M<F>)");
		glBegin(GL_LINES);
		for(uInt i=0; i < nVerts; i += 2) {
			glVertex3f(verts(i, 0), verts(i, 1), verts(i, 2));
			glVertex3f(verts(i+1, 0), verts(i+1, 1), verts(i+1, 2));
		}
		glEnd();
		endListEntry();
		traceCheck("drawLines3D(M<F>)", GLTraceGraphics);
	}

	void GLPixelCanvas::drawLines(const Matrix<Double> &verts) {
		uInt nVerts = verts.nrow();

		beginListEntry("drawLines(M<D>)");
		glBegin(GL_LINES);
		for(uInt i=0; i < nVerts; i += 2) {
			glVertex2d(verts(i, 0), verts(i, 1));
			glVertex2d(verts(i+1, 0), verts(i+1, 1));
		}
		glEnd();
		endListEntry();
		traceCheck("drawLines(M<D>)", GLTraceGraphics);
	}

//# (Cacheable) Draw a bunch of unrelated lines using current color
	void GLPixelCanvas::drawLines(const Vector<Int> &x1, const Vector<Int> &y1,
	                              const Vector<Int> &x2, const Vector<Int> &y2) {
		static const char *Name = "drawLines(V<I>,V<I>,V<I>,V<I>)";

		uInt nVerts = x1.nelements();
		beginListEntry(Name);
		glBegin(GL_LINES);
		for(uInt i=0; i < nVerts; i++) {
			glVertex2i(x1(i), y1(i));
			glVertex2i(x2(i), y2(i));
		}
		glEnd();
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

	void GLPixelCanvas::drawLines(const Vector<Float> &x1,
	                              const Vector<Float> &y1,
	                              const Vector<Float> &x2,
	                              const Vector<Float> &y2) {
		static const char *Name = "drawLines(V<F>,V<F>,V<F>,V<F>)";
		uInt nVerts = x1.nelements();
		beginListEntry(Name);
		glBegin(GL_LINES);
		for(uInt i=0; i < nVerts; i++) {
			glVertex2f(x1(i), y1(i));
			glVertex2f(x2(i), y2(i));
		}
		glEnd();
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

	void GLPixelCanvas::drawLines(const Vector<Double> &x1,
	                              const Vector<Double> &y1,
	                              const Vector<Double> &x2,
	                              const Vector<Double> &y2) {
		static const char *Name = "drawLines(V<D>,V<D>,V<D>,V<D>)";

		uInt nVerts = x1.nelements();
		beginListEntry(Name);
		glBegin(GL_LINES);
		for(uInt i=0; i < nVerts; i++) {
			glVertex2d(x1(i), y1(i));
			glVertex2d(x2(i), y2(i));
		}
		glEnd();
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

//# (Cacheable) Draw a single connected line between the points given
	void GLPixelCanvas::drawPolyline(const Vector<Int> &x1, const Vector<Int> &y1) {
		static const char *Name = "drawPolyline(V<I>,V<I>)";

		uInt nVerts = x1.nelements();
		beginListEntry(Name);
		glBegin(GL_LINE_STRIP);
		for(uInt i=0; i < nVerts; i++) {
			glVertex2i(x1(i), y1(i));
		}
		glEnd();
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

	void GLPixelCanvas::drawPolyline(const Vector<Float> &x1,
	                                 const Vector<Float> &y1) {
		static const char *Name = "drawPolyline(V<F>,V<F>)";

		uInt nVerts = x1.nelements();
		beginListEntry(Name);
		glBegin(GL_LINE_STRIP);
		for(uInt i=0; i < nVerts; i++) {
			glVertex2f(x1(i), y1(i));
		}
		glEnd();
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

	void GLPixelCanvas::drawPolyline3D(const Vector<Float> &x1,
	                                   const Vector<Float> &y1,
	                                   const Vector<Float> &z1) {
		static const char *Name = "drawPolyline3D(V<F>,V<F>,V<F>)";
		uInt nVerts = x1.nelements();
		beginListEntry(Name);
		glBegin(GL_LINE_STRIP);
		for(uInt i=0; i < nVerts; i++) {
			glVertex3f(x1(i), y1(i), x1(i));
		}
		glEnd();
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

	void GLPixelCanvas::drawPolyline(const Vector<Double> &x1,
	                                 const Vector<Double> &y1) {
		static const char *Name = "drawPolyline(V<D>,V<D>)";
		uInt nVerts = x1.nelements();
		beginListEntry(Name);
		glBegin(GL_LINE_STRIP);
		for(uInt i=0; i < nVerts; i++) {
			glVertex2d(x1(i), y1(i));
		}
		glEnd();
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

//# (Cacheable) Draw N-1 connected lines from Nx2 matrix of vertices
	void GLPixelCanvas::drawPolyline(const Matrix<Int> &verts) {
		static const char *Name = "drawPolyline(M<I>)";

		uInt nVerts = verts.nrow();
		beginListEntry(Name);
		glBegin(GL_LINE_STRIP);
		for(uInt i=0; i < nVerts; i++) {
			glVertex2i(verts(i,0), verts(i,1));
		}
		glEnd();
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

	void GLPixelCanvas::drawPolyline(const Matrix<Float> &verts) {
		static const char *Name = "drawPolyline(M<F>)";

		uInt nVerts = verts.nrow();
		beginListEntry(Name);
		glBegin(GL_LINE_STRIP);
		for(uInt i=0; i < nVerts; i++) {
			glVertex2f(verts(i,0), verts(i,1));
		}
		glEnd();
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

	void GLPixelCanvas::drawPolyline(const Matrix<Double> &verts) {
		static const char *Name = "drawPolyline(M<F>)";
		uInt nVerts = verts.nrow();
		beginListEntry(Name);
		glBegin(GL_LINE_STRIP);
		for(uInt i=0; i < nVerts; i++) {
			glVertex2d(verts(i,0), verts(i,1));
		}
		glEnd();
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

//# (Cacheable) Draw a closed polygon
	void GLPixelCanvas::drawPolygon(const Vector<Int> &x1, const Vector<Int> &y1) {
		static const char *Name = "drawPolygon(V<I>,V<I>)";
		uInt nVerts = x1.nelements();
		beginListEntry(Name);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBegin(GL_LINE_LOOP);
		for(uInt i=0; i < nVerts; i++) {
			glVertex2i(x1(i), y1(i));
		}
		glEnd();
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

	void GLPixelCanvas::drawPolygon(const Vector<Float> &x1,
	                                const Vector<Float> &y1) {
		static const char *Name = "drawPolygon(V<F>,V<F>)";
		uInt nVerts = x1.nelements();
		beginListEntry(Name);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBegin(GL_LINE_LOOP);
		for(uInt i=0; i < nVerts; i++) {
			glVertex2f(x1(i), y1(i));
		}
		glEnd();
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

	void GLPixelCanvas::drawPolygon3D(const Vector<Float> &x1,
	                                  const Vector<Float> &y1, const Vector<Float> &z1) {
		static const char *Name = "drawPolygon(V<F>,V<F>)";
		uInt nVerts = x1.nelements();
		beginListEntry(Name);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBegin(GL_LINE_LOOP);
		for(uInt i=0; i < nVerts; i++) {
			glVertex3f(x1(i), y1(i), z1(i));
		}
		glEnd();
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

	void GLPixelCanvas::drawPolygon(const Vector<Double> &x1,
	                                const Vector<Double> &y1) {
		static const char *Name = "drawPolygon(V<D>,V<D>)";
		uInt nVerts = x1.nelements();
		beginListEntry(Name);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBegin(GL_LINE_LOOP);
		for(uInt i=0; i < nVerts; i++) {
			glVertex2d(x1(i), y1(i));
		}
		glEnd();
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

//# (Cacheable) Draw and fill a closed polygon
	void GLPixelCanvas::drawFilledPolygon(const Vector<Int> &x1,
	                                      const Vector<Int> &y1) {
		static const char *Name = "drawFilledPolygon(V<I>,V<I>)";
		uInt nVerts = x1.nelements();
		beginListEntry(Name);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBegin(GL_POLYGON);		//# Default is filled.
		for(uInt i=0; i < nVerts; i++) {
			glVertex2i(x1(i), y1(i));
		}
		glEnd();
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

	void GLPixelCanvas::drawFilledPolygon(const Vector<Float> &x1,
	                                      const Vector<Float> &y1) {
		static const char *Name = "drawFilledPolygon(V<F>,V<F>)";
		uInt nVerts = x1.nelements();
		beginListEntry(Name);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBegin(GL_POLYGON);		//# Default is filled.
		for(uInt i=0; i < nVerts; i++) {
			glVertex2f(x1(i), y1(i));
		}
		glEnd();
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

	void GLPixelCanvas::drawFilledPolygon3D(const Vector<Float> &x1,
	                                        const Vector<Float> &y1,
	                                        const Vector<Float> &z1) {
		static const char *Name = "drawFilledPolygon3D(V<F>,V<F>)";
		uInt nVerts = x1.nelements();
		beginListEntry(Name);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBegin(GL_POLYGON);		//# Default is filled.
		for(uInt i=0; i < nVerts; i++) {
			glVertex3f(x1(i), y1(i), z1(i));
		}
		glEnd();
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

	void GLPixelCanvas::drawFilledPolygon(const Vector<Double> &x1,
	                                      const Vector<Double> &y1) {
		static const char *Name = "drawFilledPolygon(V<D>,V<D>)";
		uInt nVerts = x1.nelements();
		beginListEntry(Name);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBegin(GL_POLYGON);		//# Default is filled.
		for(uInt i=0; i < nVerts; i++) {
			glVertex2d(x1(i), y1(i));
		}
		glEnd();
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

//# (Cacheable) Draw a closed N-sided polygon from Nx2 matrix of vertices
	void GLPixelCanvas::drawPolygon(const Matrix<Int> &verts) {
		static const char *Name = "drawPolygon(M<I>)";
		uInt nVerts = verts.nrow();
		beginListEntry(Name);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBegin(GL_LINE_LOOP);
		for(uInt i=0; i < nVerts; i++) {
			glVertex2i(verts(i,0), verts(i,1));
		}
		glEnd();
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

	void GLPixelCanvas::drawPolygon(const Matrix<Float> &verts) {
		static const char *Name = "drawPolygon(M<F>)";
		uInt nVerts = verts.nrow();
		beginListEntry(Name);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBegin(GL_LINE_LOOP);
		for(uInt i=0; i < nVerts; i++) {
			glVertex2f(verts(i,0), verts(i,1));
		}
		glEnd();
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

	void GLPixelCanvas::drawPolygon3D(const Matrix<Float> &verts) {
		static const char *Name = "drawPolygon3D(M<F>)";
		uInt nVerts = verts.nrow();
		beginListEntry(Name);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBegin(GL_LINE_LOOP);
		for(uInt i=0; i < nVerts; i++) {
			glVertex3f(verts(i,0), verts(i,1), verts(i, 2));
		}
		glEnd();
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

	void GLPixelCanvas::drawPolygon(const Matrix<Double> &verts) {
		static const char *Name = "drawPolygon(M<D>)";
		uInt nVerts = verts.nrow();
		beginListEntry(Name);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBegin(GL_LINE_LOOP);
		for(uInt i=0; i < nVerts; i++) {
			glVertex2d(verts(i,0), verts(i,1));
		}
		glEnd();
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

//# (Cacheable) Draw a rectangle
	void GLPixelCanvas::drawRectangle(Int x1, Int y1, Int x2, Int y2) {
		static const char *Name = "drawRectangle(IIII)";
		beginListEntry(Name);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glRecti(x1, y1, x2, y2);
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

	void GLPixelCanvas::drawRectangle(Float x1, Float y1, Float x2, Float y2) {
		char Name[64];
		sprintf(Name, "drawRectangle(%.2f, %.2f, %.2f, %.2f)", x1, y1, x2, y2);
//	check(Name, sizeof(Name));

		beginListEntry(Name);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glRectf(x1, y1, x2, y2);
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

	void GLPixelCanvas::drawRectangle(Double x1, Double y1, Double x2, Double y2) {
		char Name[64];
		sprintf(Name, "drawRectangle(%.2f, %.2f, %.2f, %.2f)", x1, y1, x2, y2);
//  check(Name, sizeof(Name));

		beginListEntry(Name);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glRectd(x1, y1, x2, y2);
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

//# (Cacheable) Draw a filled rectangle
	void GLPixelCanvas::drawFilledRectangle(Int x1, Int y1, Int x2, Int y2) {
		char Name[64];
		sprintf(Name, "drawFilledRectangle(%d, %d, %d, %d)", x1, y1, x2, y2);
//  check(Name, sizeof(Name));

		beginListEntry("drawFilledRectangle(IIII)");
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glRecti(x1, y1, x2, y2);
		endListEntry();
		traceCheck("drawFilledRectangle(IIII)", GLTraceGraphics);
	}

	void GLPixelCanvas::drawFilledRectangle(Float x1, Float y1, Float x2, Float y2) {
		char Name[64];
		sprintf(Name, "drawFilledRectangle(%.2f, %.2f, %.2f, %.2f)",
		        x1, y1, x2, y2);
//  check(Name, sizeof(Name));

		beginListEntry(Name);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glRectf(x1, y1, x2, y2);
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

	void GLPixelCanvas::drawFilledRectangle(Double x1, Double y1, Double x2,
	                                        Double y2) {
		char Name[64];
		sprintf(Name, "drawFilledRectangle(%.2f, %.2f, %.2f, %.2f)",
		        x1, y1, x2, y2);
//  check(Name, sizeof(Name));

		beginListEntry(Name);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glRectd(x1, y1, x2, y2);
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

//# (Cacheable) Draw a set of points, specifying a color per point to be drawn.
	void GLPixelCanvas::drawColoredPoints_( const char *Name,
	                                        const Vector<Int> &x1,
	                                        const Vector<Int> &y1,
	                                        const Vector<uInt> &colors) {
		uInt nXPoints = x1.nelements();
		uInt nYPoints = y1.nelements();
		uInt nCPoints = colors.nelements();
		uInt nPoints;

		if((nXPoints != nYPoints) || (nXPoints != nCPoints)) {
			log_ << LogIO::WARN << LogOrigin( "GLPixelCanvas",
			                                  Name, WHERE)
			     << "Length of x, y and color vectors do not match ("
			     << nXPoints << ", " << nYPoints << ", " << nCPoints
			     << "). Using the smallest."
			     << LogIO::POST;
			nPoints = (nXPoints < nYPoints) ? nXPoints : nYPoints;
			nPoints = (nPoints < nCPoints) ? nPoints : nCPoints;
		} else
			nPoints = nXPoints;

		GLPCColorValue cv;
		beginListEntry(Name);
		glPushAttrib(GL_CURRENT_BIT);	// Save current color.
		glBegin(GL_POINTS);
		for(uInt i = 0; i < nPoints; i++) {
			uInt index = colors(i);
			//# This may need to be changed to setColorValue().
			storeColorIndex(cv, index);	//# Setup color info.
			setCurrentColorValue(cv);
			glVertex2f(x1(i), y1(i));
		}
		glEnd();
		glPopAttrib();
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

	void GLPixelCanvas::drawColoredPoints(const Vector<Int> &x1,
	                                      const Vector<Int> &y1,
	                                      const Vector<uInt> &colors) {
		static const char *Name = "drawColoredPoints(V<I>V<I>V<uI>)";
		drawColoredPoints_(Name, x1, y1, colors);
	}

	void GLPixelCanvas::drawColoredPoints_( const char *Name,
	                                        const Vector<Float> &x1,
	                                        const Vector<Float> &y1,
	                                        const Vector<uInt> &colors) {
		uInt nXPoints = x1.nelements();
		uInt nYPoints = y1.nelements();
		uInt nCPoints = colors.nelements();
		uInt nPoints;

		if((nXPoints != nYPoints) || (nXPoints != nCPoints)) {
			LogIO os;
			os << LogIO::WARN << LogOrigin( "GLPixelCanvas",
			                                Name, WHERE)
			   << "Length of x, y and color vectors do not match ("
			   << nXPoints << ", " << nYPoints << ", " << nCPoints
			   << "). Using the smallest."
			   << LogIO::POST;
			nPoints = (nXPoints < nYPoints) ? nXPoints : nYPoints;
			nPoints = (nPoints < nCPoints) ? nPoints : nCPoints;
		} else
			nPoints = nXPoints;

		//# Should consider saving and restoring existing color value.
		GLPCColorValue cv;
		beginListEntry(Name);
		glPushAttrib(GL_CURRENT_BIT);	// Save current color.
		glBegin(GL_POINTS);
		for(uInt i = 0; i < nPoints; i++) {
			uInt index = colors(i);
			//# This may need to be changed to setColorValue().
			storeColorIndex(cv, index);	//# Setup color info.
			setCurrentColorValue(cv);
			glVertex2f(x1(i), y1(i));
		}
		glEnd();
		glPopAttrib();
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

	void GLPixelCanvas::drawColoredPoints(  const Vector<Float> &x1,
	                                        const Vector<Float> &y1,
	                                        const Vector<uInt> &colors) {
		static const char *Name = "drawColoredPoints(V<F>V<F>V<uI>)";
		drawColoredPoints_(Name, x1, y1, colors);
	}

	void GLPixelCanvas::drawColoredPoints_( const char *Name,
	                                        const Vector<Double> &x1,
	                                        const Vector<Double> &y1,
	                                        const Vector<uInt> &colors) {
		uInt nXPoints = x1.nelements();
		uInt nYPoints = y1.nelements();
		uInt nCPoints = colors.nelements();
		uInt nPoints;

		if((nXPoints != nYPoints) || (nXPoints != nCPoints)) {
			LogIO os;
			os << LogIO::WARN << LogOrigin( "GLPixelCanvas",
			                                Name, WHERE)
			   << "Length of x, y and color vectors do not match ("
			   << nXPoints << ", " << nYPoints << ", " << nCPoints
			   << "). Using the smallest."
			   << LogIO::POST;
			nPoints = (nXPoints < nYPoints) ? nXPoints : nYPoints;
			nPoints = (nPoints < nCPoints) ? nPoints : nCPoints;
		} else
			nPoints = nXPoints;

		//# Should consider saving and restoring existing color value.
		GLPCColorValue cv;
		beginListEntry(Name);
		glPushAttrib(GL_CURRENT_BIT);	// Save current color.
		glBegin(GL_POINTS);
		for(uInt i = 0; i < nPoints; i++) {
			uInt index = colors(i);
			//# This may need to be changed to setColorValue().
			storeColorIndex(cv, index);	//# Setup color info.
			setCurrentColorValue(cv);
			glVertex2d(x1(i), y1(i));
		}
		glEnd();
		glPopAttrib();
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

	void GLPixelCanvas::drawColoredPoints(const Vector<Double> &x1,
	                                      const Vector<Double> &y1,
	                                      const Vector<uInt> &colors) {
		static const char *Name = "drawColoredPoints(V<D>V<D>V<uI>)";
		drawColoredPoints_(Name, x1, y1, colors);
	}

	void GLPixelCanvas::drawColoredPoints(const Matrix<Int> &xy,
	                                      const Vector<uInt> &colors) {
		static const char *Name = "drawColoredPoints(M<I>V<uI>)";
		drawColoredPoints_(Name, xy.column(0), xy.column(1), colors);
	}

	void GLPixelCanvas::drawColoredPoints(const Matrix<Float> &xy,
	                                      const Vector<uInt> &colors) {
		static const char *Name = "drawColoredPoints(M<F>V<uI>)";
		drawColoredPoints_(Name, xy.column(0), xy.column(1), colors);
	}

	void GLPixelCanvas::drawColoredPoints(const Matrix<Double> &xy,
	                                      const Vector<uInt> &colors) {
		static const char *Name = "drawColoredPoints(M<D>V<uI>)";
		drawColoredPoints_(Name, xy.column(0), xy.column(1), colors);
	}

// (Cacheable) Draw a set of lines, specifying a color per line to be drawn.
	void GLPixelCanvas::drawColoredLines(const Vector<Int> &x1,
	                                     const Vector<Int> &y1,
	                                     const Vector<Int> &x2,
	                                     const Vector<Int> &y2,
	                                     const Vector<uInt> &colors) {
		const char *Name = "drawColoredLines(V<I>V<I>V<I>V<I><uI>)";
		uInt nX1Points = x1.nelements(), nX2Points = x2.nelements();
		uInt nY1Points = y1.nelements(), nY2Points = y2.nelements();
		uInt nCPoints = colors.nelements();
		uInt nPoints;

		if((nX1Points != nY1Points) || (nX2Points != nY2Points) ||
		        (nX1Points != nX2Points) || (nX2Points != nCPoints)) {
			log_ << LogIO::WARN << LogOrigin( "GLPixelCanvas",
			                                  Name, WHERE)
			     << "Length of x, y and color vectors do not match ("
			     << nX1Points << "/" << nY1Points << ", "
			     << nX2Points << "/" << nY2Points << ", "
			     << "; " << nCPoints
			     << "). Using the smallest."
			     << LogIO::POST;
			nPoints = (nX1Points < nY1Points) ? nX1Points : nY1Points;
			nPoints = (nPoints < nX2Points) ? nPoints : nX2Points;
			nPoints = (nPoints < nY2Points) ? nPoints : nY2Points;
			nPoints = (nPoints < nCPoints) ? nPoints : nCPoints;
		} else
			nPoints = nX1Points;

		//# Might consider saving and restoring existing color value.
		GLPCColorValue cv;
		beginListEntry(Name);
		glPushAttrib(GL_CURRENT_BIT);	// Save current color.
		glBegin(GL_LINES);
		for(uInt i = 0; i < nPoints; i++) {
			uInt index = colors(i);
			//# This may need to be changed to setColorValue().
			storeColorIndex(cv, index);	//# Setup color info.
			setCurrentColorValue(cv);
			glVertex2i(x1(i), y1(i));
			glVertex2i(x2(i), y2(i));
		}
		glEnd();
		glPopAttrib();
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

	void GLPixelCanvas::drawColoredLines(const Vector<Float> &x1,
	                                     const Vector<Float> &y1,
	                                     const Vector<Float> &x2,
	                                     const Vector<Float> &y2,
	                                     const Vector<uInt> &colors) {
		const char *Name = "drawColoredLines(V<F>V<F>V<F>V<F><uI>)";
		uInt nX1Points = x1.nelements(), nX2Points = x2.nelements();
		uInt nY1Points = y1.nelements(), nY2Points = y2.nelements();
		uInt nCPoints = colors.nelements();
		uInt nPoints;

		if((nX1Points != nY1Points) || (nX2Points != nY2Points) ||
		        (nX1Points != nX2Points) || (nX2Points != nCPoints)) {
			log_ << LogIO::WARN << LogOrigin( "GLPixelCanvas",
			                                  Name, WHERE)
			     << "Length of x, y and color vectors do not match ("
			     << nX1Points << "/" << nY1Points << ", "
			     << nX2Points << "/" << nY2Points << ", "
			     << "; " << nCPoints
			     << "). Using the smallest."
			     << LogIO::POST;
			nPoints = (nX1Points < nY1Points) ? nX1Points : nY1Points;
			nPoints = (nPoints < nX2Points) ? nPoints : nX2Points;
			nPoints = (nPoints < nY2Points) ? nPoints : nY2Points;
			nPoints = (nPoints < nCPoints) ? nPoints : nCPoints;
		} else
			nPoints = nX1Points;

		// Might consider saving and restoring existing color value.
		GLPCColorValue cv;
		beginListEntry(Name);
		glPushAttrib(GL_CURRENT_BIT);	// Save current color.
		glBegin(GL_LINES);
		for(uInt i = 0; i < nPoints; i++) {
			uInt index = colors(i);
			//# This may need to be changed to setColorValue().
			storeColorIndex(cv, index);	//# Setup color info.
			setCurrentColorValue(cv);
			glVertex2f(x1(i), y1(i));
			glVertex2f(x2(i), y2(i));
		}
		glEnd();
		glPopAttrib();
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

	void GLPixelCanvas::drawColoredLines(const Vector<Double> &x1,
	                                     const Vector<Double> &y1,
	                                     const Vector<Double> &x2,
	                                     const Vector<Double> &y2,
	                                     const Vector<uInt> &colors) {
		const char *Name = "drawColoredLines(V<D>V<D>V<D>V<F><uI>)";
		uInt nX1Points = x1.nelements(), nX2Points = x2.nelements();
		uInt nY1Points = y1.nelements(), nY2Points = y2.nelements();
		uInt nCPoints = colors.nelements();
		uInt nPoints;

		if((nX1Points != nY1Points) || (nX2Points != nY2Points) ||
		        (nX1Points != nX2Points) || (nX2Points != nCPoints)) {
			log_ << LogIO::WARN << LogOrigin( "GLPixelCanvas",
			                                  Name, WHERE)
			     << "Length of x, y and color vectors do not match ("
			     << nX1Points << "/" << nY1Points << ", "
			     << nX2Points << "/" << nY2Points << ", "
			     << "; " << nCPoints
			     << "). Using the smallest."
			     << LogIO::POST;
			nPoints = (nX1Points < nY1Points) ? nX1Points : nY1Points;
			nPoints = (nPoints < nX2Points) ? nPoints : nX2Points;
			nPoints = (nPoints < nY2Points) ? nPoints : nY2Points;
			nPoints = (nPoints < nCPoints) ? nPoints : nCPoints;
		} else
			nPoints = nX1Points;

		// Might consider saving and restoring existing color value.
		GLPCColorValue cv;
		beginListEntry(Name);
		glPushAttrib(GL_CURRENT_BIT);	// Save current color.
		glBegin(GL_LINES);
		for(uInt i = 0; i < nPoints; i++) {
			uInt index = colors(i);
			//# This may need to be changed to setColorValue().
			storeColorIndex(cv, index);	//# Setup color info.
			setCurrentColorValue(cv);
			glVertex2d(x1(i), y1(i));
			glVertex2d(x2(i), y2(i));
		}
		glEnd();
		glPopAttrib();
		endListEntry();
		traceCheck(Name, GLTraceGraphics);
	}

// Set Graphics Attributes
// Options for functions with enum argument
// listed in <linkto class=Display>DisplayEnums</linkto>

// If DFCopy is requested, the op is set to copy, but disabled (the default).
// Otherwise, it is set to the requested function and enabled.
// RGB support requires OpenGL V1.1 or greater.
// This function has not been tested.
	void GLPixelCanvas::setDrawFunction(Display::DrawFunction function) {
		GLenum op = 0;
		const char *str=NULL;

		switch(function) {
		case Display::DFCopy:
			str = "DFCopy";
			op = GL_COPY;
			break;
		case Display::DFCopyInverted:
			str = "DFCopyInverted";
			op = GL_COPY_INVERTED;
			break;
		case Display::DFClear:
			str = "DFClear";
			op = GL_CLEAR;
			break;
		case Display::DFSet:
			str = "DFSet";
			op = GL_SET;
			break;
		case Display::DFInvert:
			str = "DFInvert";
			op = GL_INVERT;
			break;
		case Display::DFNoop:
			str = "DFNoop";
			op = GL_NOOP;
			break;
		case Display::DFXor:
			str = "DFXor";
			op = GL_XOR;
			break;
		case Display::DFEquiv:
			str = "DFEquiv";
			op = GL_EQUIV;
			break;
		case Display::DFAnd:
			str = "DFAnd";
			op = GL_AND;
			break;
		case Display::DFNand:
			str = "DFNand";
			op = GL_NAND;
			break;
		case Display::DFAndReverse:
			str = "DFAndReverse";
			op = GL_AND_REVERSE;
			break;
		case Display::DFAndInverted:
			str = "DFAndInverted";
			op = GL_AND_INVERTED;
			break;
		case Display::DFOr:
			str = "DFOr";
			op = GL_OR;
			break;
		case Display::DFNor:
			str = "DFNor";
			op = GL_NOR;
			break;
		case Display::DFOrReverse:
			str = "DFOrReverse";
			op = GL_OR_REVERSE;
			break;
		case Display::DFOrInverted:
			str = "DFOrInverted";
			op = GL_OR_INVERTED;
			break;
		default:
			str = "Unknown function";
			break;
		};

		char name[64];
		strcpy(name, "setDrawFunction(");
		strcat(name, str);
		strcat(name, ")");
		if(op != 0) {
			GLenum mode = isRGB_ ? GL_COLOR_LOGIC_OP : GL_INDEX_LOGIC_OP;
			beginListEntry(name);
			if(op == GL_COPY) {
				glDisable(mode);
				glLogicOp(op);
			} else {
				glEnable(mode);
				glLogicOp(op);
			}
			endListEntry();
		}
		traceCheck(name, GLTraceOther);
	}

//# Similar to setColor(), except color will be interpreted as packed RGB
//# values if in RGB mode.
	void GLPixelCanvas::setForeground(uLong color) {
		char Name[32];

		sprintf(Name, "setForeground(%ld)", color);
		setCurrentColorValue(color, Name);	//# Setup color info.
	}

//# Currently, background color is not supported.
	void GLPixelCanvas::setBackground(uLong color) {
		static Boolean print = True;

		if(print) {
			unimplemented("setBackground", WHERE);
			print = False;
		}
	}

	void GLPixelCanvas::setLineWidth(Float width) {
		char name[32];
		//# Display lib spec doesn't say what width means.
		sprintf(name, "setLineWidth(%.2f)", width);

		beginListEntry(name);
		glLineWidth(width);
		endListEntry();
		traceCheck(name, GLTraceGraphics);
	}

	void GLPixelCanvas::setLineStyle(Display::LineStyle style) {
		GLint factor = 1;
		GLushort pattern;
		char name[32];

		strcpy(name, "LineStyle(");

		lineStyle_ = style;
		switch(style) {
		default:
		case Display::LSSolid:
			strcat(name, "LSSolid)");
			break;
		case Display::LSDashed:
			pattern = 0x0F0F;
			strcat(name, "LSDashed)");
			break;
		case Display::LSDoubleDashed:
			pattern = 0x1C47;
			factor = 2;
			strcat(name, "LSDoubleDashed)");
			break;
		}

		beginListEntry(name);
		if(style == Display::LSSolid)
			glDisable(GL_LINE_STIPPLE);
		else {
			glEnable(GL_LINE_STIPPLE);
			glLineStipple(factor, pattern);
		}
		endListEntry();
		traceCheck(name, GLTraceGraphics);
	}

	void GLPixelCanvas::setCapStyle(Display::CapStyle style) {
		static Boolean print = True;

		if(print) {
			unimplemented("setCapStyle", WHERE);
			print = False;
		}
	}

	void GLPixelCanvas::setJoinStyle(Display::JoinStyle style) {
		static Boolean print = True;

		if(print) {
			unimplemented("setJoinStyle", WHERE);
			print = False;
		}
	}

// Patterns for polygon stippling.
	static GLubyte tiledstipple_[] = {
		0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF
	};
	static GLubyte halftonestipple_[] = {
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA
	};
#if 0
	0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
	      0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55
};
#endif
#define HTS2
#if defined (HTS2)
static GLubyte halftonestipple2_[] = {
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA
};
#else
static GLubyte teststipple0_[] = {
	0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0
};
#endif

void GLPixelCanvas::setFillStyle(Display::FillStyle style) {
	GLubyte *pattern;
	String name;
	const char *type;

	fillStyle_ = style;

	name = "setFillStyle(";

	switch(style) {
	case Display::FSSolid:
	default:
		type = "FSSolid";
		break;
	case Display::FSTiled:
		pattern = tiledstipple_;
		type = "FSTiled";
		break;
	case Display::FSStippled:
		pattern = halftonestipple_;
		type = "FSStippled";
		break;
	case Display::FSOpaqueStippled:
#if defined(HTS2)
		pattern = halftonestipple2_;
#else
		pattern = teststipple0_;
#endif
		type = "FSOpaqueStippled";
		break;
	}

	name += type;
	name += ")";

	beginListEntry(name.chars());
	if(style == Display::FSSolid)
		glDisable(GL_POLYGON_STIPPLE);
	else {
		glEnable(GL_POLYGON_STIPPLE);
		glPolygonStipple(pattern);
	}
	endListEntry();
	traceCheck(name, GLTraceGraphics);
}

void GLPixelCanvas::setFillRule(Display::FillRule rule) {
	static Boolean print = True;

	if(print) {
		unimplemented("setFillRule", WHERE);
		print = False;
	}
}

void GLPixelCanvas::setArcMode(Display::ArcMode mode) {
	static Boolean print = True;

	if(print) {
		unimplemented("setArcMode", WHERE);
		print = False;
	}
}

//# Get Graphics Attributes
Display::DrawFunction GLPixelCanvas::getDrawFunction() const {
	Display::DrawFunction func;
	GLint op;

	glGetIntegerv(GL_LOGIC_OP_MODE, &op);
	switch(op) {
	case GL_COPY:
		func = Display::DFCopy;
		break;
	case GL_COPY_INVERTED:
		func = Display::DFCopyInverted;
		break;
	case GL_CLEAR:
		func = Display::DFClear;
		break;
	case GL_SET:
		func = Display::DFSet;
		break;
	case GL_INVERT:
		func = Display::DFInvert;
		break;
	case GL_NOOP:
		func = Display::DFNoop;
		break;
	case GL_XOR:
		func = Display::DFXor;
		break;
	case GL_EQUIV:
		func = Display::DFEquiv;
		break;
	case GL_AND:
		func = Display::DFAnd;
		break;
	case GL_NAND:
		func = Display::DFNand;
		break;
	case GL_AND_REVERSE:
		func = Display::DFAndReverse;
		break;
	case GL_AND_INVERTED:
		func = Display::DFAndInverted;
		break;
	case GL_OR:
		func = Display::DFOr;
		break;
	case GL_NOR:
		func = Display::DFNor;
		break;
	case GL_OR_REVERSE:
		func = Display::DFOrReverse;
		break;
	case GL_OR_INVERTED:
		func = Display::DFOrInverted;
		break;
	};

	return func;
}

uLong GLPixelCanvas::getForeground() const {
	return currentColor_.index;
}

uLong GLPixelCanvas::getBackground() const {
	static Boolean print = True;

	if(print) {
		unimplemented("getBackground", WHERE);
		print = False;
	}
	return 0;
}

Float GLPixelCanvas::getLineWidth() const {
	GLfloat v[1];

	glGetFloatv(GL_LINE_WIDTH, v);
	return (Float)v[0];
}

Display::LineStyle GLPixelCanvas::getLineStyle() const {
	return lineStyle_;
}

Display::CapStyle  GLPixelCanvas::getCapStyle()  const {
	static Boolean print = True;

	if(print) {
		unimplemented("getCapStyle", WHERE);
		print = False;
	}
	return Display::CSNotLast;
}

Display::JoinStyle GLPixelCanvas::getJoinStyle() const {
	static Boolean print = True;

	if(print) {
		unimplemented("getJoinStyle", WHERE);
		print = False;
	}
	return Display::JSMiter;
}

Display::FillStyle GLPixelCanvas::getFillStyle() const {
	return fillStyle_;
}

Display::FillRule  GLPixelCanvas::getFillRule()  const {
	static Boolean print = True;

	if(print) {
		unimplemented("getFillRule", WHERE);
		print = False;
	}
	return Display::FREvenOdd;
}

Display::ArcMode   GLPixelCanvas::getArcMode()   const {
	static Boolean print = True;

	if(print) {
		unimplemented("getArcMode", WHERE);
		print = False;
	}
	return Display::AMChord;
}

//# (Cacheable) Option Control
//# Options listed in <linkto class=Display>DisplayEnums</linkto>
Bool GLPixelCanvas::enable(Display::Option option) {
	beginListEntry("enable");
	switch(option) {
	case Display::ClipWindow:
		doClipping_ = True;
		glEnable(GL_SCISSOR_TEST);
		break;
	default:
		;
	}
	endListEntry();
	traceCheck("enable(ClipWindow)", GLTraceOther);
	return True;
}

Bool GLPixelCanvas::disable(Display::Option option) {
	beginListEntry("disable");
	switch(option) {
	case Display::ClipWindow:
		doClipping_ = False;
		glDisable(GL_SCISSOR_TEST);
		break;
	default:
		;
	}
	endListEntry();
	traceCheck("disable(ClipWindow)", GLTraceOther);
	return True;
}

//# Control the image-caching strategy
void GLPixelCanvas::setImageCacheStrategy(Display::ImageCacheStrategy strategy) {
}

Display::ImageCacheStrategy GLPixelCanvas::imageCacheStrategy() const {
	return Display::ServerAlways;
}


//# (Cacheable) Setup the clip window.  The clip window, when enabled, allows
//# a user to clip all graphics output to a rectangular region on
//# the screen.
void GLPixelCanvas::setClipWindow(Int x1, Int y1, Int x2, Int y2) {
	clipX1_ = x1;
	clipY1_ = y1;
	clipX2_ = x2;
	clipY2_ = y2;
	GLsizei w = x2-x1;
	GLsizei h = y2-y1;
	beginListEntry("setClipWindow");
	glScissor(x1, y1, w, h);
	endListEntry();
	if(trace_) {
		char name[128];
		sprintf(name, "setClipWindow(%d,%d,%d,%d)", x1, y1, x2, y2);
		traceCheck(name, GLTraceOther);
	} else
		traceCheck("setClipWindow(IIII)", GLTraceOther);
}

void GLPixelCanvas::getClipWindow(Int &x1, Int &y1, Int &x2, Int &y2) {
	x1 = clipX1_;
	y1 = clipY1_;
	x2 = clipX2_;
	y2 = clipY2_;
}

#if 0
static const char *pr(Display::RefreshReason r) {
	switch(r) {
	case Display::UserCommand:
		return "UserCommand";
		break;
	case Display::ColorTableChange:
		return "ColorTableChange";
		break;
	case Display::ColormapChange:
		return "ColormapChange";
		break;
	case Display::PixelCoordinateChange:
		return "PixelCoordinateChange";
		break;
	case Display::LinearCoordinateChange:
		return "LinearCoordinateChange";
		break;
	case Display::WorldCoordinateChange:
		return "WorldCoordinateChange";
		break;
	case Display::BackCopiedToFront:
		return "BackCopiedToFront";
		break;
	case Display::ClearPriorToColorChange:
		return "ClearPriorToColorChange";
		break;
	}
	return "Unknown";
}
#endif

void GLPixelCanvas::refresh(const Display::RefreshReason &reason,
                            const Bool &explicitrequest) {
	if (explicitrequest && (reason != Display::BackCopiedToFront)) {
		setDrawBuffer(Display::BackBuffer);
		clear();
	}
	callRefreshEventHandlers(reason);
	callRefreshEventHandlers(reason);
	callRefreshEventHandlers(reason);
	callRefreshEventHandlers(reason);
	if (explicitrequest && (reason != Display::BackCopiedToFront)) {
#if 0
		copyBackBufferToFrontBuffer();
		setDrawBuffer(Display::FrontBuffer);
#endif
		callRefreshEventHandlers(Display::BackCopiedToFront);
	}
}

void GLPixelCanvas::repaint(const Bool redraw) {
	if(!(autoRefresh_ || redraw))
		return;
	if(dlists_[0] != NULL) {
		glClear(bufferMask_);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();		//# Initialize origin.
		traceCheck("Repaint", GLTraceOther, nspaces_++);
		dlists_[0]->call(True);
		swapBuffers();
		nspaces_--;
	}
}

//# Cause display to flush any graphics commands not yet drawn
void GLPixelCanvas::flush() {
	glFlush();
}

//# (Cacheable) Clear the window using the background color
void GLPixelCanvas::clear() {
	purgeLists();		//# Get rid of current repaint buffer.
	glClear(bufferMask_);
	traceCheck("clear()", GLTraceOther);
}

void GLPixelCanvas::clear(Int x1, Int y1, Int x2, Int y2) {
	if( x1 > x2) {
		Int t = x1;
		x1 = x2;
		x2 = t;
	}
	if( y1 > y2) {
		Int t = y1;
		y1 = y2;
		y2 = t;
	}

#if 0
	//# If this looks like it's clearing the entire window, assume it is.
	if((x1 <= 0) && (y1 <= 0) && (x2 >= (Int)(width_-1)) &&
	        (y2 >= (Int)(height_-1))) {
		traceCheck("clear(IIII)", GLTraceOther, nspaces_++);
		clear();
		nspaces--;
	} else
#endif
	{
		beginListEntry("clear_area");
		glRecti(x1, y1, x2, y2);
		endListEntry();
		traceCheck("clear(IIII)", GLTraceOther);
	}
}

//# (Cacheable) Set the color to use for clearing the display
void GLPixelCanvas::setClearColor(uInt colorIndex) {
	char name[32];

	sprintf(name, "setClearColor(%d)", colorIndex);
	setClearColorIndex(colorIndex, name);
}

//# Derived from X11PixelCanvas
void GLPixelCanvas::setClearColor(const String &colorname) {
	String lcolor(colorname.chars());
	String name = "setClearColor(";
	name += colorname;

	if (colorname == "foreground") {
		lcolor = deviceForegroundColor();
		name += " -> ";
		name += lcolor;
	} else if (colorname == "background") {
		lcolor = deviceBackgroundColor();
		name += " -> ";
		name += lcolor;
	}
	XColor c;
	if (XParseColor(display_, glpcctbl_->xcmap(), lcolor.chars(), &c)) {
		float r = (float) (c.red / 65535.0);
		float g = (float) (c.green / 65535.0);
		float b = (float) (c.blue / 65535.0);
		name += ",";
		name += r;
		name += ", ";
		name += g;
		name += ", ";
		name += b;
		name += ")";
		setClearColorValue(r, g, b, name.chars());
	} else {
		throw(AipsError("attempt to set clear color to undefined color"));
	}
}

void GLPixelCanvas::setClearColor(float r, float g, float b) {
	char name[48];

	sprintf(name, "setClearColor(%.4f, %.4f, %.4f)", r, g, b);
	setClearColorValue(r, g, b, name);
}

//# (Not Cacheable) Get the current color to use for clearing the display.
uInt GLPixelCanvas::clearColor() const {
	return (uInt)clearColor_.index;
}

void GLPixelCanvas::getClearColor(float &r, float &g, float &b) const {
	r = clearColor_.red;
	g = clearColor_.green;
	b = clearColor_.blue;
}

//# Get/set the current foreground/background colors.  These colors
//# should be used when the special Strings "foreground" and "background"
//# are given for a color.
void GLPixelCanvas::setDeviceForegroundColor(const String colorname) {
	itsDeviceForegroundColor_ = colorname;
}

String GLPixelCanvas::deviceForegroundColor() const {
	return itsDeviceForegroundColor_;
}

void GLPixelCanvas::setDeviceBackgroundColor(const String colorname) {
	itsDeviceBackgroundColor_ = colorname;
}

String GLPixelCanvas::deviceBackgroundColor() const {
	return itsDeviceBackgroundColor_;
}

//# Return the width of the GLPixelCanvas in pixels
uInt GLPixelCanvas::width() const {
	return width_;
}

//# Return the height of the GLPixelCanvas in pixels
uInt GLPixelCanvas::height() const {
	return height_;
}

//# Return the depth of the GLPixelCanvas in bits
uInt GLPixelCanvas::depth() const {
	return depth_;
}

//# Get the pixel density (in dots per inch [dpi]) of the GLPixelCanvas
void GLPixelCanvas::pixelDensity(Float &xdpi, Float &ydpi) const {
	Screen *screen = XtScreen(parent_);
	int height, width, widthmm, heightmm;
	static const float IPERMM = 1.0/25.4;	//# inches/mm.

	height = HeightOfScreen(screen);
	width = WidthOfScreen(screen);
	heightmm = HeightMMOfScreen(screen);
	widthmm = WidthMMOfScreen(screen);
	xdpi = height/(heightmm*IPERMM);
	ydpi = width/(widthmm*IPERMM);
}


//# Index is always treated as an index. (In RGB mode, the virtual
//# color table is used).
void GLPixelCanvas::storeColorIndex(GLPCColorValue &v, uInt index) {
	v.index = index;
#if 0
	if (glpcctbl_->colorModel() != Display::Index) {
		float r, g, b; //# Convert to RGB components then store them.
		glpcctbl_->indexToRGB(index, r, g, b);
		v.red = r;
		v.green = g;
		v.blue = b;
	}
#else
	float r, g, b; //# Get RGB components then store them.
	glpcctbl_->indexToRGB(index, r, g, b);
	v.red = r;
	v.green = g;
	v.blue = b;
#endif
}

//# Store the new value in the struct. If RGB mode, index is interpreted
//# as a packed RGB value.
void GLPixelCanvas::storeColorValue(GLPCColorValue &v, uLong index) {
	float r, g, b;
	v.index = index;
	if (glpcctbl_->colorModel() != Display::Index) {
		//# Convert to RGB components then store them.
		glpcctbl_->pixelToComponents(index, r, g, b);
	} else
		glpcctbl_->indexToRGB((uInt)index, r, g, b);
	v.red = r;
	v.green = g;
	v.blue = b;
}

void GLPixelCanvas::storeColorValue(GLPCColorValue &v,
                                    Float r, Float g, Float b) {
	v.red = r;
	v.green = g;
	v.blue = b;
}

void GLPixelCanvas::storeColorValue(GLPCColorValue &v,
                                    Float r, Float g, Float b, Float a) {
	v.red = r;
	v.green = g;
	v.blue = b;
	v.alpha = a;
}

//# Use the index value as the new current color using the virtual
//# colortable if necessary.
void GLPixelCanvas::setCurrentColorIndex(uInt index, const char *name) {
	storeColorIndex(currentColor_, index);
	setCurrentColorValue(currentColor_, name);
}

//# Similar to setCurrentColorIndex, but if in RGB mode, the value is
//# interpreted as packed RGB components.
void GLPixelCanvas::setCurrentColorValue(uLong value, const char *name) {
	storeColorValue(currentColor_, value);
	setCurrentColorValue(currentColor_, name);
}

//# Make the RGB values the currentColor.
void GLPixelCanvas::setCurrentColorValue(Float r, Float g, Float b,
        const char *name=NULL) {
	storeColorValue(currentColor_, r, g, b);
	setCurrentColorValue(currentColor_, name);
}

//# Make the RGBA values the currentColor.
void GLPixelCanvas::setCurrentColorValue(Float r, Float g, Float b, Float a,
        const char *name=NULL) {
	storeColorValue(currentColor_, r, g, b, a);
	setCurrentColorValue(currentColor_, name);
}

//# Tell OpenGL to use the given color index or RGB value depending
//# on whether we're in Index or RGB mode. If name is not NULL,
//# wrap the call in the usual begin/endListEntry, tracecheck.
void GLPixelCanvas::setCurrentColorValue(const GLPCColorValue &v,
        const char *name=NULL) {
	if(name != NULL)
		beginListEntry(name);
	if(glpcctbl_->colorModel() == Display::Index)
		glIndexi((GLint)v.index);
	else
		glColor4f(v.red, v.green, v.blue, v.alpha);
	if(name != NULL) {
		endListEntry();
		traceCheck(name, GLTraceGraphics);
	}
}

//# Use the index value as the new clear color using the virtual
//# colortable if necessary.
void GLPixelCanvas::setClearColorIndex(uInt index, const char *name) {
	storeColorIndex(clearColor_, index);
	setClearColorValue(clearColor_, name);
}

//# Similar to setClearColorIndex, but if in RGB mode, the value is
//# interpreted as packed RGB components.
void GLPixelCanvas::setClearColorValue(uLong value, const char *name) {
	storeColorValue(clearColor_, value);
	setClearColorValue(clearColor_, name);
}

//# Make the RGB values the clear Color.
void GLPixelCanvas::setClearColorValue( Float r, Float g, Float b,
                                        const char *name=NULL) {
	storeColorValue(clearColor_, r, g, b);
	setClearColorValue(clearColor_, name);
}

//# Make the RGBA values the clear Color.
void GLPixelCanvas::setClearColorValue( Float r, Float g, Float b, Float a,
                                        const char *name=NULL) {
	storeColorValue(clearColor_, r, g, b, a);
	setClearColorValue(clearColor_, name);
}

//# Tell OpenGL to use the given color index or RGB value depending
//# on whether we're in Index or RGB mode. If name is not NULL,
//# wrap the call in the usual begin/endListEntry, tracecheck.
void GLPixelCanvas::setClearColorValue( const GLPCColorValue &v,
                                        const char *name=NULL) {
	if(name != NULL)
		beginListEntry(name);
	if(glpcctbl_->colorModel() == Display::Index) {
		glClearIndex((GLfloat)v.index);
	} else {
		glClearColor(v.red, v.green, v.blue, v.alpha);
	}
	if(name != NULL) {
		endListEntry();
		traceCheck(name, GLTraceGraphics);
	}
}

//# (Cacheable) Set current color (works in RGB or colormap mode)
void GLPixelCanvas::setColor(uInt colorIndex) {
	char name[32];

	sprintf(name, "setColor(%d)", colorIndex);
	setCurrentColorIndex(colorIndex, name);
}


//# This currently does not work for indexed mode.
void GLPixelCanvas::setColor(const String &colorname) {
	char name[64];

	Float r, g, b;
	if (getColorComponents(colorname, r, g, b)) {
		sprintf(name,
		        "setColor(%s[%.2f,%.2f,%.2f])", colorname.chars(), r, g, b);
		setCurrentColorValue(r,g,b, name);
	} else {
		throw(AipsError("attempt to set color to an unknown name"));
	}
}

void GLPixelCanvas::setRGBColor(float r, float g, float b) {
	char name[32];
	sprintf(name, "setRGBColor(%.2f,%.2f,%.2f)", r, g, b);
	setCurrentColorValue(r, g, b, name);
}

//# Get color components in range 0 to 1 without actually
//# allocating the color.  This is needed to set up other
//# devices, for example PgPlot.
Bool GLPixelCanvas::getColorComponents( const String &colorname, Float &r,
                                        Float &g, Float &b) {
	//# Lifted from X11PixelCanvas.
	String lcolor(colorname.chars());
	if (colorname == "foreground") {
		lcolor = deviceForegroundColor();
	} else if (colorname == "background") {
		lcolor = deviceBackgroundColor();
	}
	XColor c;
	if (XParseColor(display_, glpcctbl_->xcmap(), lcolor.chars(), &c)) {
		r = (float) (c.red / 65535.0);
		g = (float) (c.green / 65535.0);
		b = (float) (c.blue / 65535.0);
		return True;
	}
	return False;
}

//# (Not Cacheable) Returns the current color as a color index
uInt GLPixelCanvas::color() const {
	return currentColor_.index;
}

//# (Not Cacheable) Retuns the current color as an RGB triple
void GLPixelCanvas::getColor(float &r, float &g, float &b) const {
	r = currentColor_.red;
	g = currentColor_.blue;
	b = currentColor_.green;
}

//# (Not Cacheable) Get color index value (works in RGB or colormap mode)
// (Not implemented).
Bool GLPixelCanvas::getColor(Int x, Int y, uInt &color) {
	return False;
}

// (Not implemented).
Bool GLPixelCanvas::getRGBColor(Int x, Int y, float &r, float &g, float &b) {
	return False;
}

//# (Not Cacheable) resize request.  returns true if window was resized.
//# Will refresh if doCallbacks is True.
//# This has not been tested!
Bool GLPixelCanvas::resize(uInt reqXSize, uInt reqYSize, Bool doCallbacks) {
	XtVaSetValues(form_, XmNwidth, reqXSize, XmNheight, reqYSize, NULL);

	uInt width, height;
	if (doCallbacks) refresh();

	getwidthheight(form_, width, height);
	if((reqXSize != width) || (reqYSize != height))
		return False;
	else
		return True;
}


//# (Not Cacheable) resize the colortable by requesting a new number of cells
Bool GLPixelCanvas::resizeColorTable(uInt newSize) {
	return glpcctbl_->resize(newSize);
}

//# (Not Cacheable) resize the colortable by requesting a new RGB/HSV cube
Bool GLPixelCanvas::resizeColorTable(uInt nReds, uInt nGreens, uInt nBlues) {
	return glpcctbl_->resize(nReds, nGreens, nBlues);
}

//# save/restore the current translation. This is called pushMatrix because
//# eventually we may want scaling or rotation to play a modest
//# role here.
void GLPixelCanvas::pushMatrix() {
	beginListEntry("pushMatrix");
	glPushMatrix();
	endListEntry();
	traceCheck("pushMatrix", GLTraceMath);
}

void GLPixelCanvas::popMatrix() {
	beginListEntry("popMatrix");
	glPopMatrix();
	endListEntry();
	traceCheck("popMatrix", GLTraceMath);
}

//# zero the current translation
void GLPixelCanvas::loadIdentity() {
	beginListEntry("loadIdentity");
	glLoadIdentity();
	endListEntry();
	traceCheck("loadIdentity", GLTraceMath);
}

//# translation functions
//# translate applies a relative translation to the current matrix and
//# can be used to position graphics.  Together with pushMatrix and
//# popMatrix it can be used to build heirarchical scenes.
void GLPixelCanvas::translate(Int xt, Int yt) {
	char name[32];
	sprintf(name, "translate(%d,%d)", xt, yt);
	beginListEntry(name);
	glTranslatef((GLfloat)xt, (GLfloat)yt, 0.0);
	endListEntry();
	traceCheck(name, GLTraceMath);
}

void GLPixelCanvas::getTranslation(Int &xt, Int &yt) const {
	GLfloat matrix[17];

	matrix[16] = -12345;
	glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	xt = (int)(matrix[12]+0.5);
	yt = (int)(matrix[13]+0.5);
}

Int GLPixelCanvas::xTranslation() const {
	Int x, y;

	getTranslation(x, y);
	return x;
}

Int GLPixelCanvas::yTranslation() const {
	Int x, y;

	getTranslation(x, y);
	return x;
}

void GLPixelCanvas::pushAttrib(const GLbitfield attrib) {
	String name = "pushAttrib(";

	if(trace_)
		GLAttribToString(attrib, name);
	else
		name += "mask";

	name += ")";

	beginListEntry(name);
	glPushAttrib(attrib);
	endListEntry();
	traceCheck(name, GLTraceOther);
}

void GLPixelCanvas::popAttrib() {
	const char *NAME = "popAttrib()";

	beginListEntry(NAME);
	glPopAttrib();
	endListEntry();
	traceCheck(NAME, GLTraceOther);
}


//# (Not cacheable) set the draw buffer
//# The user's request is overridden with GL_BACK if double buffered
//# or GL_FRONT if single buffered. The test programs don't seem to work
//# otherwise. Also, some vendor's OGL implementations don't seem to
//# work correctly.
void GLPixelCanvas::setDrawBuffer(Display::DrawBuffer buf) {
	//# static const char *name = "GLPixelCanvas::setDrawBuffer: ";
	GLenum modeused, mode;
	const char *name;

	switch(buf) {
	case Display::NoBuffer:
		mode = GL_NONE;
		name = "NoBuffer";
		modeused = doubleBuffered_ ? GL_BACK : GL_FRONT;
		break;
	case Display::FrontBuffer:
		mode = GL_FRONT;
		name = "FrontBuffer";
		modeused = doubleBuffered_ ? GL_BACK : GL_FRONT;
		break;
	case Display::DefaultBuffer:
		if(doubleBuffered_)
			mode = GL_BACK;
		else
			mode = GL_FRONT;
		name = "DefaultBuffer";
		modeused = mode;
		break;
	case Display::BackBuffer:
		if(doubleBuffered_)
			mode = GL_BACK;
		else
			mode = GL_FRONT;
		name = "BackBuffer";
		modeused = mode;
		break;
	case Display::FrontAndBackBuffer:
		mode = GL_FRONT_AND_BACK;
		name = "FrontAndBackBuffer";
		modeused = mode;
		break;
	default:
		modeused = mode = 0;
		name = "Unknown";
		break;
	}

//	modeused = mode;

	if(modeused != 0)
		glDrawBuffer(modeused);

	if(trace_) {
		if(mode == modeused)
			traceCheck("setDrawBuffer", GLTraceOther);
		else {
			char buf[128];

			sprintf(buf,
			        "setDrawBuffer: Overrode request for %s (%s) with %s\n",
			        GLbufferToString(mode), name,
			        GLbufferToString(modeused));
			traceCheck(buf, nspaces_+1);
		}
		pRWbuf("R/W Buffers: ", nspaces_ + 1);
	}
}

//# buffer memory exchanges
//# (Not cacheable)
void GLPixelCanvas::copyBackBufferToFrontBuffer() {
	//	copyBuffer(GL_BACK, GL_FRONT);
	traceCheck("Ignored copyBackBufferToFrontBuffer()", GLTraceOther);
}

void GLPixelCanvas::copyFrontBufferToBackBuffer() {
//	copyBuffer(GL_FRONT, GL_BACK);
	traceCheck("Ignored  copyFrontBufferToBackBuffer()", GLTraceOther);
}

void GLPixelCanvas::swapBuffers() {
	Window w = XtWindow(drawArea_);

	if(w == 0)
		return;

	if(doubleBuffered_) {
#if 0
		//# Don't swap if it doesn't make sense.
		GLint writeBuf;
		glGetIntegerv(GL_DRAW_BUFFER, &writeBuf);
		if(writeBuf == GL_BACK)
#endif
		{
			glXWaitX();
			glXSwapBuffers(display_, w);
			glXWaitGL();
			traceCheck("swapBuffers", GLTraceOther);
			if(tracing())
				pRWbuf();
		}
#if 0
		else if(trace_) {
			String msg = "swapBuffers ignored for ";
			msg += GLbufferToString(writeBuf);
			msg += " buffer.";
			traceCheck(msg.chars());
		}
#endif
	} else
		traceCheck("swapBuffers ignored for single buffer.",
		           nspaces_);
}

//# Copy entire buffer.
void GLPixelCanvas::copyBuffer( GLenum from, GLenum to) {
	copyBuffer(from, to, 0, 0, width_, height_);
}

//# Copy a buffer region.
void GLPixelCanvas::copyBuffer( GLenum from, GLenum to,
                                GLint x, GLint y,
                                GLsizei width, GLsizei height) {
	int writeBuf, readBuf;

	//# Change R/W buffers as necessary.
	glGetIntegerv(GL_DRAW_BUFFER, &writeBuf);
	glGetIntegerv(GL_READ_BUFFER, &readBuf);
	if((GLenum)readBuf != from)
		glReadBuffer(from);
	if((GLenum)writeBuf != to)
		glDrawBuffer(to);
	//# Copy
	glRasterPos2i(x, y);	//# ?? Should this be reset?
	glCopyPixels(x, y, width, height, GL_COLOR);
	//# Reset as necessary.
	if((GLenum)readBuf != from)
		glReadBuffer(readBuf);
	if((GLenum)writeBuf != to)
		glDrawBuffer(writeBuf);
}

//# partial buffer memory exchanges.  (x1,y1 are blc, x2,y2 are trc)
//# Not supported since it only confuses things.
void GLPixelCanvas::copyBackBufferToFrontBuffer(Int x1, Int y1,
        Int x2, Int y2) {
//	copyBuffer(GL_BACK, GL_FRONT, x1, y1, x2-x1, y2-y1);
	traceCheck("Ignored copyBackBufferToFrontBuffer(IIII)", GLTraceOther);
	if(trace_)
		pRWbuf(NULL, nspaces_+1);
}

void GLPixelCanvas::copyFrontBufferToBackBuffer(Int x1, Int y1,
        Int x2, Int y2) {
//	copyBuffer(GL_FRONT, GL_BACK, x1, y1, x2-x1, y2-y1);
	traceCheck("Ignored copyFrontBufferToBackBuffer(IIII)", GLTraceOther);
}

void GLPixelCanvas::swapBuffers(Int x1, Int y1, Int x2, Int y2) {
	unimplemented("swapBuffers(IIII)", WHERE);
	traceCheck("Ignored swapBuffers(IIII)", GLTraceOther);
}

//# return True if refresh is allowed right now...
Bool GLPixelCanvas::refreshAllowed() const {
	return True;
}

//# Make this context (window) the current OpenGL context.
// This needs to be called whenever drawing is switched between windows.
// (eg. Multiple OpenGL canvases).
void GLPixelCanvas::makeCurrent() {
	Window win = XtWindow(drawArea_);
	if(win == 0) {
		warn("makeCurrent", "No window available to be current",
		     WHERE);
	} else {
		glXMakeCurrent(display_, win, context_);
	}
	traceCheck("makeCurrent", GLTraceOther);
}

//# Wait for X commands to finish.
void GLPixelCanvas::waitX() {
//	makeCurrent();
	glXWaitX();
}

//# Wait for GL commands to finish.
void GLPixelCanvas::waitGL() {
//	makeCurrent();
	glXWaitGL();
}

//# Print a message and lists any current OpenGL errors if tracing is enabled.
//# Generally called early in a routine if some later call might also generate
//# a trace or after any OpenGL calls if there will be no other traceCheck
//# calls for the routine.
void GLPixelCanvas::traceCheck(const char *name, uLong tmask, uInt nspaces) {
	if(!trace_ || !(tmask & traceLevel_))
		return;

	log_.trace(name, nspaces, traceLevel_ == GLTraceErrors);
}

////////////////////////////////////////////////////////////////
// A couple of routines to make printing warnings easier.
void GLPixelCanvas::warn(const char *routine, const char *msg,
                         const SourceLocation *where)const {
	log_	<< LogIO::WARN
	        << LogOrigin( "GLPixelCanvas", routine, where)
	        << msg
	        << LogIO::POST;
}

void GLPixelCanvas::unimplemented(const char *routine,
                                  const SourceLocation *where)const {
	warn(routine, "unimplemented", where);
}

//-----------------------------------------------------
//#
//#  X Event Handling
//
//-----------------------------------------------------

void GLPixelCanvas::handleEventsCB(Widget w, GLPixelCanvas *glpc,
                                   XEvent *ev, Boolean *) {
	glpc->handleEvents(w,ev);
}

void GLPixelCanvas::handleEvents(Widget, XEvent *ev) {

	switch(ev->type) {
	case MotionNotify:
		callMotionEventHandlers(ev->xmotion.x,
		                        height_ - 1 - ev->xmotion.y,
		                        ev->xmotion.state);
		break;
	case ButtonPress:
	case ButtonRelease: {
		Display::KeySym ks = Display::K_Pointer_Button1;
		switch(ev->xbutton.button) {
		case Button1:
			ks = Display::K_Pointer_Button1;
			break;
		case Button2:
			ks = Display::K_Pointer_Button2;
			break;
		case Button3:
			ks = Display::K_Pointer_Button3;
			break;
		case Button4:
			ks = Display::K_Pointer_Button4;
			break;
		case Button5:
			ks = Display::K_Pointer_Button5;
			break;
		}

		callPositionEventHandlers(ks,
		                          (ev->type == ButtonPress ? True : False),
		                          ev->xbutton.x,
		                          height_ - 1 - ev->xbutton.y,
		                          ev->xbutton.state);
	}
	break;
	case KeyPress:
	case KeyRelease: {
		Bool keystate = (ev->type == KeyPress ? True : False);
		uInt state = ev->xkey.state;
		uInt keycode = ev->xkey.keycode;
		Int index = 0;
		if (state & ShiftMask) index = 1;
		else if (state & ControlMask) index = ControlMapIndex;
		else if (state & LockMask) index = LockMapIndex;
		else if (state & Mod1Mask) index = Mod1MapIndex;
		else if (state & Mod2Mask) index = Mod2MapIndex;
		else if (state & Mod3Mask) index = Mod3MapIndex;
		else if (state & Mod4Mask) index = Mod4MapIndex;
		else if (state & Mod5Mask) index = Mod5MapIndex;

		uLong keysym = XKeycodeToKeysym(display_, keycode, index);
		if (keysym == 0)
			keysym = XKeycodeToKeysym(display_, keycode, 0);

#ifdef XK_KP_Home
#ifdef XK_KP_Delete
		//# Handle numlock.  Some HP's may not have these keysyms defined and
		//# hence could not generate a keysym to trigger this numlock test.
		if ((state & 0x0010) && (keysym >= XK_KP_Home) && (keysym <= XK_KP_Delete)) {
			keysym = XKeycodeToKeysym(display_, keycode, 1);
		}
#endif
#endif

		callPositionEventHandlers((Display::KeySym) keysym,
		                          keystate,
		                          ev->xkey.x,
		                          height_ - 1 - ev->xkey.y,
		                          ev->xkey.state);
	}
	break;

	//# these next two case statements are used together
	//# to generate updates when the window resizes.

	case ConfigureNotify:
		//# Force configure notify to issue the refresh
		//# because expose will not if the window is made smaller.
		if ((uInt)ev->xconfigure.width < width_ ||
		        (uInt)ev->xconfigure.height < height_) {
			traceCheck("handleEvents::ConfigureNotify: Start", nspaces_++);
			exposeHandler();
			traceCheck("handleEvents::ConfigureNotify: End\n", --nspaces_);
		}
		break;

	case Expose:
		//# Use simple procedure of stripping all-but-last expose
		//# events and updating the whole window.
		if (ev->xexpose.count == 0) {
			traceCheck("handleEvents::Expose: Start", nspaces_++);
			exposeHandler();
			traceCheck("handleEvents::Expose: End\n", --nspaces_);
		}
		break;

	default:
		break;
	}
}

void GLPixelCanvas::exposeHandler() {

	traceCheck("exposeHandler Start", nspaces_++);
	makeCurrent();
	waitX();

	if (exposeHandlerFirstTime_) {
		//# install Colormap for this Window
		Widget top = X11TopLevelWidget(drawArea_);
		Widget wl[2];
		wl[0] = drawArea_;
		wl[1] = top;
		XtSetWMColormapWindows(top, wl, 2);

		BlackPixel_ = XBlackPixelOfScreen(XtScreen(top));
		glClear(bufferMask_);
		if(trace_) {
			char buf[128];
			::XDisplay *dpy = display_;
			int screen = DefaultScreen(dpy);
			const char *venstr = glXQueryServerString(dpy, screen, GLX_VENDOR);
			const char *verstr = glXQueryServerString(dpy, screen, GLX_VERSION);
			sprintf(buf, "GLX Server:\n\tVendor = %s\n\tVersion = %s\n",
			        venstr, verstr);
			log_.append(buf);
			venstr = glXGetClientString(dpy, GLX_VENDOR);
			verstr = glXGetClientString(dpy, GLX_VERSION);
			sprintf(buf, "GLX Client:\n\tVendor = %s\n\tVersion = %s\n",
			        venstr, verstr);
			log_.append(buf);
			sprintf(buf, "%s buffered, %s\n",
			        doubleBuffered_ ? "Double" : "Single",
			        isRGB_ ? "RGB" : "Indexed");
			log_.append(buf);
			log_.post();
		}
		exposeHandlerFirstTime_ = False;
	}

	Bool sizeChanged = handleResizeEvent(0, 0);
	if (sizeChanged) {
//#    glDrawBuffer(GL_FRONT_AND_BACK);
		clear();		//# Also deletes current repaint list.
		///////////# Reset defaults. (&place in DL 0)/////
		if(isRGB_)
			setClearColor(clearColor_.red, clearColor_.green, clearColor_.blue);
		else
			setClearColor(clearColor_.index);
		setPointSize(2.0);	//# 1 is a bit too small.
		/////////////////////////////////
		traceCheck("RefreshEventHandlers: Call", nspaces_++);
		callRefreshEventHandlers(Display::PixelCoordinateChange);
		swapBuffers();
		traceCheck("RefreshEventHandlers: Return", --nspaces_);
	} else
		repaint();

	traceCheck("exposeHandler End", GLTraceOther, --nspaces_);
}

//-----------------------------------------------------
//#
//#  GLPixelCanvas Event Handling
//
//-----------------------------------------------------

//# Reset the viewport and return True if the size has changed.
Boolean GLPixelCanvas::handleResizeEvent(uInt w, uInt h) {
	Boolean resized;

	if((w == 0) || (h == 0)) {
		uInt uw, uh;
		getwidthheight(drawArea_, uw, uh);
		w = uw;
		h = uh;
	}

	if((width_ != w) || (height_ != h)) {
		width_ = w;
		height_ = h;
		glViewport(0, 0, (GLsizei) w, (GLsizei) h);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.0,  (GLdouble)w, 0.0, (GLdouble)h, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		resized = True;
	} else
		resized = False;

	return resized;
}

//-----------------------------------------------------
//
//# Colortable handling
//
//-----------------------------------------------------

void GLPixelCanvas::colorTableResizeCB(PixelCanvasColorTable *, uInt,
                                       GLPixelCanvas * xpc,
                                       Display::RefreshReason reason) {
	if (!xpc->refreshAllowed()) {
		return;
	}

	if ((reason == Display::ColormapChange) &&
	        (xpc->visual_->c_class != PseudoColor) &&
	        (xpc->visual_->c_class != StaticColor)) {
		reason = Display::ColorTableChange;
	}

	/*
	if ((reason == Display::ClearPriorToColorChange) &&
	    ((xpc->visual_->c_class == PseudoColor) ||
	     (xpc->visual_->c_class == StaticColor))) {
	*/
	if (reason == Display::ClearPriorToColorChange) {
		Display::DrawBuffer buf = xpc->drawBuffer();
		if (buf == Display::FrontBuffer) {
			xpc->setDrawBuffer(Display::FrontAndBackBuffer);
		}
		//xpc->setDrawBuffer(Display::FrontAndBackBuffer);
		//xpc->clear();
		xpc->setDrawBuffer(buf);
	} else if (reason != Display::ColormapChange) {
		xpc->callRefreshEventHandlers(reason);
		if (xpc->drawBuffer() == Display::DefaultBuffer) {
			xpc->copyBackBufferToFrontBuffer();
		}
	}
}

} //# NAMESPACE CASA - END

#endif //# OGL
