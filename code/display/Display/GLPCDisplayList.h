//# GLPCDisplayList.h: this defines GLPCDisplayList, which acts as a wrapper
//# around OpenGL display lists for the GLPixelCanvas.
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

// <summary>
//	Display list support for GLPixelCanvas.
// </summary>
// <use visibility=local>
//
// <synopsis>
// These classes are used to implement display list support for GLPixelCanvas.
// Execution tracing is supported. The variable 'nspaces' gives the number
// of spaces to indent the printout. All of these classes are used internally
// by GLPixelCanvas and wouldn't be useful elsewhere.
// </synopsis>
//
// <prerequisite>
//   <li> None
// </prerequisite>
//
// <thrown>
//  None
// </thrown>
//
// <todo asof="2001/10/9">
//
// </todo>

#ifndef TRIALDISPLAY_GLPCDISPLAYLIST_H
#define TRIALDISPLAY_GLPCDISPLAYLIST_H

#include <string.h>
#include <GL/gl.h>
#include <casa/aips.h>
#include <casa/BasicSL/String.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Base class for the various display list subclasses.
// </summary>
	class GLPCDisplayListElement {
	public:
		// Draw element unless disabled or force is True.
		virtual void call(Bool force=False, const uInt nspaces=0);

		// enable/disable
		// <group>
		Bool enabled()const {
			return enabled_;
		}
		Bool disabled()const {
			return !enabled_;
		}
		virtual void disable();
		virtual void enable();
		// </group>

		// Each element has a name which is printed out when tracing.
		// <group>
		const char *name()const {
			return name_.chars();
		}
		void name(const char *);
		// </group>
		// Enable/disable tracing.
		// <group>
		Bool trace()const {
			return trace_;
		}
		void trace(const Bool t) {
			trace_ = t;
		}
		// </group>
		// Begin recording commands.
		// Recording is a one shot deal. After stop is called, recording
		// can not be reenabled.
		virtual void start();	//# Start recording.
		// Stop display list recording. Ignored if not already recording.
		virtual void stop();
		// Each element is reference counted.
		uLong useCount()const {
			return usage_;
		}
		void ref();
		void unref();
	protected:
		GLPCDisplayListElement(const char *name=NULL);
		// Elements self delete when the reference count goes to 0.
		virtual ~GLPCDisplayListElement();
		void traceCheck(uInt spaces=0, const char *str=NULL,
		                const char *name=NULL);
	private:
		uLong	usage_;
		Bool	enabled_;
		Bool	trace_;
		String	name_;
	};

// <summary>
// Returns a Display List Element for recording GL commands.
// </summary>
// <synopsis>
// Returns a Display List Element for recording GL commands.
// Commands are recorded until stop is called by creating an OpenGL
// display list and letting OpenGL do the actual recording. Calling
// stop ends the list. Typically, these OpenGL lists are very short,
// containing just one or two commands. eg. draw a rectangle.
// </synopsis>
	class GLPCDisplayListEntry : public GLPCDisplayListElement {
	public:
		enum RECORDSTATE { INITED, RECORDING, STOPPED};

		GLPCDisplayListEntry::GLPCDisplayListEntry(
		    const char *name=NULL,
		    GLenum mode=GL_COMPILE_AND_EXECUTE);
		virtual ~GLPCDisplayListEntry();
		//# Draw list.
		virtual void call(Bool force, const uInt spaces);
		//# Recording is a one shot deal. After stop is called, recording
		//# can not be reenabled.
		virtual void start();	//# Start recording.
		//# Stop display list recording. Ignored if not already recording.
		virtual void stop();
	protected:
	private:
		GLenum		mode_;
		GLuint		id_;
		RECORDSTATE	recording_;
	};

//#////////////////////////////////////////////////////////////////

// <summary>
// DisplayListElement that can contain lists of other DisplayListElements.
// </summary>
// <synopsis>
// When GLPixelCanvas::newList() is called, a GLPCDisplayList is created
// to hold all the GLPCDisplayListEntrys that are created. A GLPCDisplayList
// can also hold other GLPCDisplayLists (drawList called inside a list).
// </synopsis>
	class GLPCDisplayList : public GLPCDisplayListElement {
	public:
		// Amount by which to increment the list of display lists when
		// it fills up.
		enum {DefaultSizeIncrement=16};

		GLPCDisplayList(const char *name=NULL,
		                uInt sizeincr=GLPCDisplayList::DefaultSizeIncrement);

		// Copy a display list's list.
		GLPCDisplayList(const GLPCDisplayList &list);
		~GLPCDisplayList();

		// Append another element to list.
		void add(GLPCDisplayListElement *);

		// Run the current list.
		virtual void call(Bool force=False, uInt spaces=0);

		// Translate the list.
		// Set translation values. New values are added to current.
		void translate(Float xt, Float yt, Float zt=0.0);

		//# Miscellaneous

		// Return current translation
		void  translation(Float &xo, Float &yo)const;
		void  translation(Float &xo, Float &yo, Float &zo)const;
		// Return/Set amount to increase id list by.
		uInt sizeincrement()const {
			return sizeincr_;
		}
		void sizeincrement(const uInt sizeincr) {
			sizeincr_ = sizeincr;
		}
	protected:
	private:
		void resize();
		void unrefall();

		GLfloat	xt_, yt_, zt_;	//# Amount to translate display list.
		uInt	sizeincr_;	//# Amount to increase listids_ when it's full.
		uInt	numentries_;	//# # of entries & index to next free.
		uInt	listSize_;	//# # of slots in list.
		GLPCDisplayListElement **list_;
	};


} //# NAMESPACE CASA - END

#endif
