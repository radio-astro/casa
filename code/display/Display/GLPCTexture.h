//# GLPCTexture.h:	Holds data for displaying images as textures.
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

#ifndef TRIALDISPLAY_GLPCTEXTURE_H
#define TRIALDISPLAY_GLPCTEXTURE_H

#include <graphics/X11/X_enter.h>
#include <GL/gl.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <graphics/X11/X_exit.h>
#include <casa/aips.h>
#include <display/Display/GLPCDisplayList.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Holds parameters for setting glTexParameteri variables.
// </summary>
// <visibility=local>
// <synopsis>
// Holds parameters for setting glTexParameteri variables. See the man
// page for glTexParameteri for description. This class is not needed
// unless the caller wishes to change the defaults for a textured image.
// maxtexturesize is used to specify the maximum texture size used for
// an image. The value used will be the smallest power of 2 that
// is >= maxtexturesize. By default, the display hosts' maximum texture
// size will be used. Again, this isn't normally changed.
// GLPCTextureParams may be deleted after GLPCTexturedImage is created.
	class GLPCTextureParams {
	public:
		GLPCTextureParams();
		GLPCTextureParams(const GLPCTextureParams &);
		GLPCTextureParams(const GLPCTextureParams *);
		void copy(const GLPCTextureParams &p);
		~GLPCTextureParams();
		// Set min/mag filters. If magFilter is 0, use min Filter.
		void filter(GLint minFilter, GLint magFilter=0);
		// Set wrap parameters. If wrapT is 0, use wrapS.
		void wrap(GLint wrapS, GLint wrapT=0);

		// Set value to use for max texture size. Value will be rounded
		// up to a power of 2 >= maxTextureSize.
		void maxTextureSize(GLint maxtexturesize);
		void format(GLenum fmt);

		// For glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode);
		void environMode(GLenum mode);
	public:
		// glTexParameteri(GL_TEXTURE_2D, x, y).
		GLint	min_filter_;	// GL_TEXTURE_MIN_FILTER
		GLint	mag_filter_;	// GL_TEXTURE_MAG_FILTER
		GLint	wrap_s_;	// GL_TEXTURE_WRAP_S
		GLint	wrap_t_;	// GL_TEXTURE_WRAP_T

		// Maximum texture size to use. Default is host's max texture size.
		GLuint	maxtexturesize_;
		GLenum	envmode_;	// GL_DECAL, GL_BLEND or GL_MODULATE.
		GLenum	format_;	// GL_RGB or GL_COLOR_INDEX.
	private:
		void init();
	};

	class GLPCTextureObject;

// <prerequisite>
// <li> <linkto class="GLPCDisplayListEntry">GLPCDisplayListEntry</linkto>
// </prerequisite>
// <synopsis>
// GLPCTexturedImage is used to convert an image to texture objects for
// display. The pixels array contains pixels stored as RGB triples.
// This class is typically only used by
// <linkto class="GLPixelCanvas">GLPixelCanvas</linkto>
// If the image is larger than the maximum texture size, it will be broken
// into pieces that fit.
// </synopsis>
//<thrown><li> AipsError </thrown>
	class GLPCTexturedImage : public GLPCDisplayListEntry {
	public:
		GLPCTexturedImage(GLsizei width, GLsizei height,
		                  const GLushort *pixels,
		                  const GLPCTextureParams* = NULL);
		~GLPCTexturedImage();
		void draw();	// Draw image using current values.
		// Draw image and save values for later.
		void draw(GLfloat x, GLfloat y, GLfloat z=0.0,
		          GLfloat xscale=1.0, GLfloat yscale=1.0);

		void envmode(GLenum mode);
		GLenum envmode()const {
			return params_->envmode_;
		}
		GLPCTextureParams *params()const {
			return params_;
		}
	private:
		void createTextures(GLsizei dWidth, GLsizei dHeight);
		void storeImage(GLsizei dWidth, GLsizei dHeight,
		                const GLushort *pixels);
		int	ntx_, nty_;
		int	numTextures_;		// ntx_*nty_;
		GLPCTextureObject **textures_;	// numtexures entries.
		GLfloat	x_, y_, z_;
		GLfloat	xscale_, yscale_;
		GLsizei	dWidth_, dHeight_;
		GLPCTextureParams *params_;
		GLint	maxTextureSize_;
	};


} //# NAMESPACE CASA - END

#endif
