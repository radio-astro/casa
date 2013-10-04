//# GLPixelClass.cc:	Defines a simple display list for use with
//#			GLPixelCanvas
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
#if defined(OGL)
#include <casa/iostream.h>
#include <stdio.h>
#include <string.h>
#include <display/Display/GLPCDisplayList.h>
#include <display/Display/GLSupport.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//# Each object created has a tag to help debugging.
	static uLong serial_ = 0;

//# One of these shared by everyone.
	static GLLogIO logIO_;

	GLPCDisplayListElement::GLPCDisplayListElement(const char *name) {
		char sbuf[32];

		usage_ = 0;
		enabled_ = True;
		trace_ = False;

		//# Copy name to buffer. Then append serial number.
		if(name != NULL)
			name_ = name;
		else
			name_ = "elem";

		sprintf(sbuf, ":%03ld", serial_++);
		name_ += sbuf;
	}

	GLPCDisplayListElement::~GLPCDisplayListElement() {
		usage_ = 0;
	}

	void GLPCDisplayListElement::disable() {
		enabled_ = False;
	}

	void GLPCDisplayListElement::enable() {
		enabled_ = True;
	}

	void GLPCDisplayListElement::ref() {
		usage_ += 1;
	}

	void GLPCDisplayListElement::unref() {
		if(usage_ > 0)
			usage_ -= 1;
		if(usage_ == 0) {
			delete this;
		}
	}

	void GLPCDisplayListElement::call(Bool /*force*/, uInt nspaces) {
		traceCheck(nspaces);
	}

	void GLPCDisplayListElement::traceCheck(uInt spaces, const char *str,
	                                        const char *namestr) {
		if(trace()) {
			const char *Name = (namestr == NULL) ? name() : namestr;

			logIO_.trace(Name, spaces);
		}
	}

// Recording is a one shot deal. After stop is called, recording
// can not be reenabled.
	void GLPCDisplayListElement::start() {}	//# Start recording.
// Stop display list recording. Ignored if not already recording.
	void GLPCDisplayListElement::stop() {}

	void GLPCDisplayListElement::name(const char *name) {
		name_ = name;
	}

//#//////////////////////////////////////////////////////////////
// GLPCDisplayListEntry adds GL display lists.
	GLPCDisplayListEntry::GLPCDisplayListEntry(const char *Name, GLenum mode) :
		GLPCDisplayListElement(Name) , mode_(mode), id_(0), recording_(INITED) {
	}

// Start recording.
	static GLuint maxid = 0;

	void GLPCDisplayListEntry::start() {
		if(recording_ == INITED) {
			recording_ = RECORDING;
			id_ = glGenLists(1);
			if(id_ > maxid)
				maxid = id_;
			glNewList(id_, mode_);
		}
	}

//# Stop recording. There is no way to restart.
	void GLPCDisplayListEntry::stop() {
		if(recording_ == RECORDING) {
			glEndList();
			recording_ = STOPPED;
		}
	}

//# Execute the display list.
	void GLPCDisplayListEntry::call(Bool force, uInt spaces) {
		if(force || enabled()) {	//# ??? Should recording_ be checked?
			if(id_ > maxid) {
				fprintf(stderr, "Bad ID: %u for %s\n", id_, name());
			} else
				glCallList(id_);
			traceCheck(spaces);
		}
	}

	GLPCDisplayListEntry::~GLPCDisplayListEntry() {
		stop();
		glDeleteLists(id_, 1);
	}


////////////////////////////////////////////////////////////////
// GLPCDisplayList is used to hold GLPCDisplayListEntry objects.
	GLPCDisplayList::GLPCDisplayList(const char *name, uInt sizeincr) :
		GLPCDisplayListElement(name), xt_(0.0), yt_(0.0), zt_(0.0),
		sizeincr_(0), numentries_(0), listSize_(0), list_(NULL) {
		sizeincr_ = (sizeincr > 0) ? sizeincr :
		            GLPCDisplayList::DefaultSizeIncrement;
	}

	GLPCDisplayList::GLPCDisplayList(const GLPCDisplayList &l) :
		GLPCDisplayListElement(l.name()) {
		xt_ = l.xt_;
		yt_ = l.yt_;
		zt_ = l.zt_;
		numentries_ = 0;
		// Make a list as long as l's.
		listSize_ = 0;
		sizeincr_ = l.listSize_;
		list_ = NULL;
		resize();
		sizeincr_ = l.sizeincr_;
		for(uInt i=0; i < l.numentries_; i++)
			add(l.list_[i]);
	}

// Increase size of list.
	void GLPCDisplayList::resize() {
		uInt newsize = listSize_ + sizeincr_;
		GLPCDisplayListElement **list = new GLPCDisplayListElement *[newsize];

		if(list_ != NULL) {
			memcpy(list, list_, sizeof(*list)*numentries_);;
			delete [] list_;
		}
		list_ = list;
		listSize_ = newsize;
	}

// Dec reference count for all entries.
// (Remove all entries from list).
	void GLPCDisplayList::unrefall() {
		if(list_ != NULL) {
			for(uInt i=0; i< numentries_; i++) {
				list_[i]->unref();
				list_[i] = NULL;
			}
		}
	}

	GLPCDisplayList::~GLPCDisplayList() {
		if(list_ != NULL) {
			unrefall();
			// Don't delete entries since another list might have them.
			delete [] list_;
			list_ = NULL;
			numentries_ = listSize_ = 0;
		}
	}

	void GLPCDisplayList::translate(Float xt, Float yt, Float zt) {
		xt_ += (GLfloat)xt;
		yt_ += (GLfloat)yt;
		zt_ += (GLfloat)zt;
	}

	void  GLPCDisplayList::translation(Float &xo, Float &yo)const {
		xo = (Float)xt_;
		yo = (Float)yt_;
	}

	void GLPCDisplayList::translation(Float &xo, Float &yo, Float &zo)const {
		xo = (Float)xt_;
		yo = (Float)yt_;
		zo = (Float)zt_;
	}

// If there are display lists to call, do the translation then call
// the lists.
	void GLPCDisplayList::call(Bool force, uInt spaces) {
		if(!(force || enabled()))
			return;
		if(numentries_ == 0)	// Nothing to do.
			return;

		if(trace()) {
			String str = "\nCalling ";
			char buf[64];

			str += name();

			sprintf(buf, " at %.2f,%.2f,%.2f", xt_, yt_, zt_);
			str += buf;
			traceCheck(spaces, NULL, str.chars());
		}
		glPushMatrix();
		glTranslatef(xt_, yt_, zt_);

		for(uInt i=0; i < numentries_; i++)
			list_[i]->call(force, spaces+1);
		glPopMatrix();
	}

// Append to list.
	void GLPCDisplayList::add(GLPCDisplayListElement *e) {
		if(e == NULL)
			return;
		if(numentries_ >= listSize_)
			resize();
		list_[numentries_++] = e;
		e->ref();
	}

} //# NAMESPACE CASA - END

#endif // OGL
