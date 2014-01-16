//# GLSupport.h: GLLogIO and a couple support routines.
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
//	Support functions for GLPixelCanvas.
// </summary>
// <synopsis>
// The GLLogIO class extends the LogIO class to add tracing support.
// Also, a couple of miscellaneous support routines are defined.
// </synopsis>
// <prerequisite>
// <li> <linkto class="LogIO">LogIO</linkto>
// </prerequisite>
// <use visibility=local>
//
// <thrown>
//  None
// </thrown>
//

#ifndef TRIALDISPLAY_GLSUPPORT_H
#define TRIALDISPLAY_GLSUPPORT_H

#include <casa/aips.h>
#include <casa/Logging/LogIO.h>
#include <casa/BasicSL/String.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// GLLogIO adds tracing support to the AIPS++ LogIO object.
	class GLLogIO : public LogIO {
	public:
		GLLogIO();
		virtual ~GLLogIO();
		// Print tracing information. Also calls glcheck.
		// This is the main use for GLLogIO.
		// nspaces is the number of spaces to indent name and any errors.
		// If errorsonly is True, don't print anything if there are no
		// errors.
		void trace(const char *name, int nspaces, Bool errorsonly=False);

		// Append a line to buffer then add a newline.
		// msg is indented by nspaces spaces. msg may contain newline
		// characters.
		void append(const char *msg, uInt nspaces=0);
		// Adds nspaces to buffer.
		void indent(uInt nspaces=0);
		// Deletes current buffer contents.
		void clear();
		// Copies any current OpenGL error messages to internal buffer.
		int glcheck(uInt nspaces);

		// Logging to standard AIPS++ disrupts trace formatting. These
		// routines allow redirecting or delaying output.
		// NOTE: There is only 1 instance of the use iostream flag or String
		// for all instances of GLLogIO.

		// Post messages to an iostream instead of LogIO object.
		static void postToStream(Bool useStream=False);

		// Post messages to str for later use. Disables if NULL.
		// If str is non NULL, no output is done until postString is called.
		// Replacing an existing string does not cause the existing string
		// to be posted.
		static void postToString(String *str=NULL);

		// If an output string exists, copy it to an output stream if
		// it exists. If the output stream doesn't exist, copy it to
		// the LogIO object. The string is then cleared.
		// Ignore if the string doesn't exist.
		void postString();
		// Copy string to either output stream or LogIO object. The string
		// is not cleared.
		void postString(const String &str);
		// Copy internal buffer to output. Then clear internal buffer.
		void post();
	private:
	private:
		String	buffer_;
		static Bool postToStream_;
		static String	*ostring_;
	};

	enum GLTraceLevel {GLTraceNone, GLTraceErrors=1, GLTraceInfo=2,
	                   GLTraceCalls=4, GLTraceImages=8,
	                   GLTraceGraphics=0x10, GLTraceMath=0x20,
	                   GLTraceOther=0x40,
	                   GLTraceNormal= 0x3F, GLTraceAll= ~0
	                  };

// Convert various OpenGL constants to their string representations.
	const char *GLbufferToString(int b);
	const char *GLformatToString(GLenum format);
// Given a glPushAttrib argument, append the string with the arg's
// string representation.
	void GLAttribToString(GLbitfield bits, String &str);

//const char *GLtypeToString(GLenum type);

} //# NAMESPACE CASA - END

#endif
