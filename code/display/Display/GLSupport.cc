//# GLSupport.cc:	GLPixelCanvas support functions.
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
#include <graphics/X11/X_enter.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <graphics/X11/X_exit.h>
#include <display/Display/GLSupport.h>
#include <casa/iostream.h>

#include <stdio.h>		// sprintf.

namespace casa { //# NAMESPACE CASA - BEGIN

	Bool GLLogIO::postToStream_ = False;
	String *GLLogIO::ostring_ = NULL;

	GLLogIO::GLLogIO() {
		clear();
	}

	GLLogIO::~GLLogIO() {
	}

// Print name followed by any errors.
// If errorsonly is True, don't print anything if there aren't any errors.
	void GLLogIO::trace(const char *name, int nspaces, Bool errorsonly) {
		append(name, nspaces);
		int nerrs = glcheck(nspaces+2);
		if((errorsonly && (nerrs > 0)) || !errorsonly)
			post();
		else
			clear();
	}

// Add message to internal buffer.
	void GLLogIO::append(const char *msg, uInt nspaces) {
		char c;

		if((msg == NULL) || (*msg == '\0'))
			return;
		if(*msg != '\n')
			indent(nspaces);

		// Append character to internal string. If a newline is seen,
		// indent the next line unless it's the last character.
		while((c = *msg++) != '\0') {
			buffer_ += c;
			if((c == '\n') && (*msg != '\0'))
				indent(nspaces);
		}

		// Append a newline. If the last character of the string was also
		// a newline, there will be a blank line printed.
		buffer_ += '\n';
	}

// Append any OpenGL error messages to buffer and return the error count.
// (Normally, there will be at most one error).
	int GLLogIO::glcheck(uInt nspaces) {
		int numerrs = 0;
		GLenum err;
		const char *MSG ="glGetError caused an error. \n\
Called between glBegin/glEnd?";

		while( (err = glGetError()) != GL_NO_ERROR) {
			char buf[64];
			const unsigned char *errstr = gluErrorString(err);
			numerrs += 1;
			if(errstr != NULL)
				sprintf(buf, "%s(%d)", errstr, err);
			else
				sprintf(buf, "GLError: %d", err);
			append(buf, nspaces);
			if(err == 0) {	// glGetError caused the error.
				append(MSG, nspaces);
				break;
			}
		}
		return numerrs;
	}

	void GLLogIO::clear() {
//	buffer_.clear();
		buffer_ = "";
	}

	void GLLogIO::indent(uInt nspaces) {
		while(nspaces-- > 0)
			buffer_ += ' ';
	}

// Copy internal buffer to one of:
//	output string.
//	output stream.
//	LogIO object.
// The internal buffer is then cleared.
	void GLLogIO::post() {
		if(ostring_ != NULL)
			*ostring_ += buffer_;
		else
			postString(buffer_);
		clear();
	}

	void GLLogIO::postToStream(Bool useStream) {
		postToStream_ = useStream;
	}

// Post messages to String. Disables if NULL.
	void GLLogIO::postToString(String *str) {
		ostring_ = str;
	}

// If posting to a string, copies it to the logging object, then
// clears the string. Ignored if there is no string.
	void GLLogIO::postString() {
		if(ostring_ != NULL) {
			postString(*ostring_);
			*ostring_ = "";
		}
	}

// Copy string to either ostream or LogIO object. Doesn't clear string.
	void GLLogIO::postString(const String &str) {
		if(str.length() == 0)
			return;

		if(postToStream_)
			cout << str;
		else
			*this << str << LogIO::POST;
	}

////////////////////////////////////////////////////////////////
// Convert various OpenGL constants to their string representations.

// Print the name of the GL buffer variable
	const char *GLbufferToString(int b) {
		switch(b) {
		case GL_FRONT:
			return "FRONT";
			break;
		case GL_BACK:
			return "BACK";
			break;
		case GL_FRONT_AND_BACK:
			return "FRONT_AND_BACK";
			break;
		default:
			return "UNKNOWN";
		}
	}

////////////////////////////////////////////////////////////////
// Convert a GL format to its string representation.
	const char *GLformatToString(GLenum format) {
		const char *fmt;

		switch(format) {
		case GL_COLOR_INDEX:
			fmt = "INDEX";
			break;
		case GL_RGB:
			fmt = "RGB";
			break;
		case GL_STENCIL_INDEX:
			fmt = "GL_STENCIL_INDEX";
			break;
		case GL_DEPTH_COMPONENT:
			fmt = "GL_DEPTH_COMPONENT";
			break;
		case GL_RGBA:
			fmt = "GL_RGBA";
			break;
		case GL_RED:
			fmt = "GL_RED";
			break;
		case GL_GREEN:
			fmt = "GL_GREEN";
			break;
		case GL_BLUE:
			fmt = "GL_BLUE";
			break;
		case GL_ALPHA:
			fmt = "GL_ALPHA";
			break;
		case GL_LUMINANCE:
			fmt = "GL_LUMINANCE";
			break;
		case GL_LUMINANCE_ALPHA:
			fmt = "GL_LUMINANCE_ALPHA";
			break;
		default:
			fmt = "Unknown";
		}
		return fmt;
	}

// The bit values are defined and the table stored so that successive
// bits are one bit position offset from the previous.
// (eg. GL_FOG_BIT is 2*GL_LIGHTING_BIT).
	static struct {
		GLuint value;
		char *name;
	} GL_ATTRIB_BITS[] = {
		{GL_CURRENT_BIT, "GL_CURRENT_BIT"},
		{GL_POINT_BIT, "GL_POINT_BIT"},
		{GL_LINE_BIT, "GL_LINE_BIT"},
		{GL_POLYGON_BIT, "GL_POLYGON_BIT"},
		{GL_POLYGON_STIPPLE_BIT, "GL_POLYGON_STIPPLE_BIT"},
		{GL_PIXEL_MODE_BIT, "GL_PIXEL_MODE_BIT"},
		{GL_LIGHTING_BIT, "GL_LIGHTING_BIT"},
		{GL_FOG_BIT, "GL_FOG_BIT"},
		{GL_DEPTH_BUFFER_BIT, "GL_DEPTH_BUFFER_BIT"},
		{GL_ACCUM_BUFFER_BIT, "GL_ACCUM_BUFFER_BIT"},
		{GL_STENCIL_BUFFER_BIT, "GL_STENCIL_BUFFER_BIT"},
		{GL_VIEWPORT_BIT, "GL_VIEWPORT_BIT"},
		{GL_TRANSFORM_BIT, "GL_TRANSFORM_BIT"},
		{GL_ENABLE_BIT, "GL_ENABLE_BIT"},
		{GL_COLOR_BUFFER_BIT, "GL_COLOR_BUFFER_BIT"},
		{GL_HINT_BIT, "GL_HINT_BIT"},
		{GL_EVAL_BIT, "GL_EVAL_BIT"},
		{GL_LIST_BIT, "GL_LIST_BIT"},
		{GL_TEXTURE_BIT, "GL_TEXTURE_BIT"},
		{GL_SCISSOR_BIT, "GL_SCISSOR_BIT"}
		// SGI also has GL_MULTISAMPLE_BIT_EXT which we don't handle.
	};
	static const unsigned int NBITS =
	    sizeof(GL_ATTRIB_BITS)/sizeof(*GL_ATTRIB_BITS);

// Convert glPushAttrib arg mask to a string.
	void GLAttribToString(GLbitfield bits, String &str) {
		GLuint mask = 1;
		GLuint index = 0;
		Bool haveEntry = False;

		bits &= GL_ALL_ATTRIB_BITS;		// Ignore any invalid bits.
		if(bits == GL_ALL_ATTRIB_BITS) {
			str += "GL_ALL_ATTRIB_BITS";
			return;
		}
		while(bits) {
			if(bits & mask) {
				bits &= ~mask;		// Remove bit.
				if(index >= NBITS)	// Shouldn't happen.
					break;
				if(haveEntry)
					str += " | ";
				str += GL_ATTRIB_BITS[index].name;
				haveEntry = True;
#if 0
				if(mask != GL_ATTRIB_BITS[index].value) {
					printf("Value is 0x%4x!!\n",
					       GL_ATTRIB_BITS[index].value);
				}
#endif
			}
			mask += mask;
			index++;
		}
	}

#if 0
	const char *GLtypeToString(GLenum type) {
		const char *fmt;

		switch(type) {
		case GL_UNSIGNED_BYTE:
			fmt = "GL_UNSIGNED_BYTE";
			break;
		case GL_BYTE:
			fmt = "GL_BYTE";
			break;
		case GL_BITMAP:
			fmt = "GL_BITMAP";
			break;
		case GL_UNSIGNED_SHORT:
			fmt = "GL_UNSIGNED_SHORT";
			break;
		case GL_SHORT:
			fmt = "GL_SHORT";
			break;
		case GL_UNSIGNED_INT:
			fmt = "GL_UNSIGNED_INT";
			break;
		case GL_INT:
			fmt = "GL_INT";
			break;
		case GL_FLOAT:
			fmt = "GL_FLOAT";
			break;
		default:
			fmt = "Unknown";
		}
		return fmt;
	}
#endif

} //# NAMESPACE CASA - END

#endif	// OGL
