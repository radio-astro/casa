//# GLPXTexture.cc
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
#include <stdio.h>
#include <casa/Exceptions.h>
#include <casa/aips.h>
#include <casa/Logging/LogIO.h>
#include <display/Display/GLPCTexture.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Which of GL_NEAREST or GL_LINEAR to default to.
#if !defined(DEFAULT_FILTER)
#define DEFAULT_FILTER GL_NEAREST
#endif

// What to use as default.
	static const GLPCTextureParams DEFAULTPARAMS_;

// A single texture object.
	class GLPCTextureObject {
	public:
		// Create a texture object of the given size and format.
		GLPCTextureObject(GLsizei width, GLsizei height,
		                  GLPCTextureParams *p);
		~GLPCTextureObject();

		// Copy data to existing texture.
		// x/yoffset	- Where in the texture to begin copy.
		// dwidth	- Number of pixels per row to copy.
		// dHeight	- Number of rows to copy.
		// dStride	- width of data array in pixels. ( >= dwidth).
		// data		- Pixel array consisting of dStride*dHeight
		//		  pixels. Each pixel consists of 3 unsigned shorts.
		//		  ( R, G, B).
		void storeTexture(const GLint xoffset, const GLint yoffset,
		                  const GLsizei dwidth, const GLsizei dheight,
		                  const GLsizei dStride, const GLushort *data);

		// Use x, y and z as starting points for drawTexture().
		void locateTexture(const float x, const float y, const float z=0.0) {
			x_ = x;
			y_ = y;
			z_ = z;
		}

		// Set scaling for texture.
		void scaleTexture(const float xscl=1.0, const float yscl=1.0) {
			xscl_ = xscl;
			yscl_ = yscl;
		}

		GLuint textureID()const {
			return texid_;
		}
		// Draw texture. x, y and z become new x_, y_ and z_.
		void drawTexture(GLfloat x, GLfloat y, GLfloat z=0.0,
		                 GLfloat xscale=1.0, GLfloat yscale=1.0);
		// Draw texture at x_, y_, z_.
		void drawTexture();
		GLsizei tWidth()const {
			return twidth_;
		}
		GLsizei tHeight()const {
			return theight_;
		}
		GLsizei dWidth()const {
			return dwidth_;
		}
		GLsizei dHeight()const {
			return dheight_;
		}
		GLuint	textureid()const {
			return texid_;
		}
	public:
		// Return a power of two >= num.
		static unsigned int pow2(const unsigned int num);
		static GLint componentsPerPixel(GLint format);
		static GLint componentsFromFormat(GLint format);
	private:
		GLuint bindTexture();
	private:
		GLuint	texid_;
		GLfloat x_, y_, z_;	// Where to draw texture for drawTexture().
		GLfloat xscl_, yscl_;
		GLsizei	twidth_;	// Texture width/height
		GLsizei	theight_;
		GLsizei dwidth_;	// Data width/height.
		GLsizei dheight_;
		GLPCTextureParams *params_;
	};

// Return the number of textures in x & y needed to contain an image
// with a size of width x height.
	static void numTextures(const int width, const int height,
	                        int &nx, int &ny, GLint maxtexturesize) {
		maxtexturesize = GLPCTextureObject::pow2(maxtexturesize);

		// # of textures along X axis.
		if(width <= maxtexturesize)
			nx = 1;
		else {
			nx = width/maxtexturesize;
			if((width % maxtexturesize) > 0)
				nx += 1;
		}

		// # of textures along Y axis.
		if(height <= maxtexturesize)
			ny = 1;
		else {
			ny = height/maxtexturesize;
			if((height % maxtexturesize) > 0)
				ny += 1;
		}
	}

	GLPCTextureObject::GLPCTextureObject(GLsizei width, GLsizei height,
	                                     GLPCTextureParams *p) {
		twidth_ = pow2(width);
		theight_ = pow2(height);
		dwidth_ = dheight_ = 0;	// No data yet.
		params_ = p;

		//!!! There may be problems when going between SGI and
		// other systems! (SGI uses old GLX opcode).
		glGenTextures(1, &texid_);
		glBindTexture(GL_TEXTURE_2D, texid_);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, p->wrap_s_);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, p->wrap_t_);
		glTexParameteri(GL_TEXTURE_2D,
		                GL_TEXTURE_MAG_FILTER, p->mag_filter_);
		glTexParameteri(GL_TEXTURE_2D,
		                GL_TEXTURE_MIN_FILTER, p->min_filter_);

		GLint components = componentsFromFormat(p->format_);

		// Should use GL_PROXY_TEXTURE_2D to check first!
		// Creates (allocates) a texture, but w/o data.
		// Requires GL V1.1 or greater.
		if(components != 0)
			glTexImage2D(GL_TEXTURE_2D, 0, components, twidth_, theight_,
			             0, p->format_, GL_UNSIGNED_SHORT, NULL);
	}

	GLPCTextureObject::~GLPCTextureObject() {
		glDeleteTextures(1, &texid_);
	}

// Return a power of two >= num.
	unsigned int GLPCTextureObject::pow2(const unsigned int num) {
		register unsigned int mask = 1, val;

		if(num == 0)
			return 0;

		while(mask < num)
			mask = (mask<<1) + 1;

		val = mask+1;
		if( (val >> 1) == num)	// Num already a power of 2?
			return num;	// Yes.
		else
			return val;
	}

// Return the byte alignment for the value.
// val is an address that has been coerced to a long.
	static inline int getalignment(const long val) {
		int alignment, algncheck;
		algncheck = (int)(val & 0x7);
		switch (algncheck) {
		case 0:
			alignment = 8;
			break;
		case 2:
		case 6:
			alignment = 2;
			break;
		case 4:
			alignment = 4;
			break;
		default:		// 1,3,5,7
			alignment = 1;
			break;
		};
		return alignment;
	}

	GLint GLPCTextureObject::componentsPerPixel(GLint format) {
		GLint cpp=0;

		switch(format) {
		case GL_COLOR_INDEX:
			cpp = 1;
			break;
		case GL_RED:
		case GL_GREEN:
		case GL_BLUE:
			cpp = 1;
			break;
		case GL_LUMINANCE:
			cpp = 1;
			break;
		case GL_ALPHA:
			cpp = 1;
			break;
		case GL_RGB:
			cpp = 3;
			break;
		case GL_RGBA:
			cpp = 3;
			break;
		case GL_LUMINANCE_ALPHA:
			cpp = 2;
			break;
		default:
			cpp = 0;
		}
		return cpp;
	}

	GLint GLPCTextureObject::componentsFromFormat(GLint format) {
		GLint components;

		switch(format) {
		case GL_COLOR_INDEX:
			components = GL_RGB;
			break;
		case GL_RED:
		case GL_GREEN:
		case GL_BLUE:
			components = GL_RGB;
			break;
		case GL_LUMINANCE:
			components = GL_LUMINANCE;
			break;
		case GL_ALPHA:
			components = GL_ALPHA;
			break;
		case GL_RGB:
			components = GL_RGB;
			break;
		case GL_RGBA:
			components = GL_RGBA;
			break;
		case GL_LUMINANCE_ALPHA:
			components = GL_LUMINANCE_ALPHA;
			break;
		default:
			components = 0;
		}
		return components;
	}

// Copy a (sub)image from pixels to texture.
//  tXoffset, tYoffset	Offsets into texture.
//  dWidth, dHeight	Width & height of data image to be copied.
//  pixels		Pixel data stored as R, G, B triples.
//			(unsigned shorts).
// dStride		Difference, in pixels, between 1 row and the next.
//			( Actual stride is dStride*3).
//			If dStride is 0, it is assumed to be dWidth.
	void GLPCTextureObject::storeTexture(
	    const GLint tXoffset, const GLint tYoffset,
	    const GLsizei dWidth, const GLsizei dHeight,
	    const GLsizei dStride,
	    const GLushort *pixels) {
		GLsizei width=dWidth, height=dHeight;
		GLsizei stride = (dStride == 0) ? dWidth : dStride;

		// Sanity checks.
		if(tXoffset < 0) {
			LogIO os;
			os << LogIO::WARN
			   << LogOrigin("GLPCTextureObject", "storeTexture", WHERE)
			   << " tXoffset is negative (" << tXoffset << ")"
			   << LogIO::POST;
			return;
		} else if(tXoffset >= twidth_) {
			LogIO os;
			os << LogIO::WARN
			   << LogOrigin("GLPCTextureObject", "storeTexture", WHERE)
			   << " tXoffset (" << tXoffset << ") is > tWdith (" << twidth_
			   << ")"
			   << LogIO::POST;
			return;
		} else if(width + tXoffset > twidth_) {
			LogIO os;
			os << LogIO::WARN
			   << LogOrigin("GLPCTextureObject", "storeTexture", WHERE)
			   << " width (" << width << ") is > twidth (" << twidth_
			   << ")"
			   << LogIO::POST;
			width = twidth_ - tXoffset;
		}

		if(tYoffset < 0) {
			LogIO os;
			os << LogIO::WARN
			   << LogOrigin("GLPCTextureObject", "storeTexture", WHERE)
			   << " tYoffset is negative (" << tYoffset << ")"
			   << LogIO::POST;
			return;
		} else if(tYoffset >= theight_) {
			LogIO os;
			os << LogIO::WARN
			   << LogOrigin("GLPCTextureObject", "storeTexture", WHERE)
			   << " tYoffset (" << tYoffset << ") is > tWdith ("
			   << theight_ << ")"
			   << LogIO::POST;
			return;
		} else if(height + tYoffset > theight_) {
			LogIO os;
			os << LogIO::WARN
			   << LogOrigin("GLPCTextureObject", "storeTexture", WHERE)
			   << " height (" << height << ") is > theight (" << theight_
			   << ")"
			   << LogIO::POST;
			height = theight_ - tYoffset;
		}

		if(stride < dWidth) {	// Will be dWidth if dStride=0.
			LogIO os;
			os << LogIO::WARN
			   << LogOrigin("GLPCTextureObject", "storeTexture", WHERE)
			   << " stride (" << stride << ") is < dWidth (" << dWidth
			   << ")"
			   << LogIO::POST;
			return;
		}

		// Odd widths and odd heights (from the 1 big container buffer)
		// can cause alignment problems. If weird coloring occurs, one
		// could change this to make alignment always one.
		// Choose the minimum alignment.
		int alignment = getalignment((long)pixels);
		if(alignment > 1) {
			int widthalignment = getalignment(width);
			if(widthalignment < alignment)
				alignment = widthalignment;
		}

		glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
		bindTexture();

		GLint format = params_->format_;

		// If the width of the data matches the texture, just copy it.
		// Otherwise, copy row by row.
#if 0
		// This might not work.
		if((width == twidth_) && (width == dStride)) {
			// This might work if even if width < twidth_.
			glTexSubImage2D(GL_TEXTURE_2D, 0, tXoffset, tYoffset,
			                width, height, format, GL_UNSIGNED_SHORT,
			                pixels);
		} else
#endif
		{
			const GLushort *ptr = pixels;
			const GLenum CPP = componentsPerPixel(format);
			for(int r = 0; r < height; r++, ptr += dStride*CPP) {
				glTexSubImage2D(GL_TEXTURE_2D, 0,
				                tXoffset, tYoffset+r,
				                width, 1, format,
				                GL_UNSIGNED_SHORT, ptr);
			}
		}

		dwidth_ = width;
		dheight_ = height;
	}

// Draw the texture at the given location/scaling. Save
// the values for later.
	void GLPCTextureObject::drawTexture(GLfloat x, GLfloat y, GLfloat z,
	                                    GLfloat xscale, GLfloat yscale) {
		x_ = x;
		y_ = y;
		z_ = z;
		xscl_ = xscale;
		yscl_ = yscale;

		drawTexture();
	}

// Draw textured image by drawing a textured rectangle.
	void GLPCTextureObject::drawTexture() {
		GLfloat x0=x_, x1=x0+dwidth_*xscl_;
		GLfloat y0=y_, y1=y0+dheight_*yscl_;
		GLfloat tx0=0.0, tx1=(float)(dwidth_)/(float)(twidth_);
		GLfloat ty0=0.0, ty1=(float)(dheight_)/(float)(theight_);

		// These may be interesting when set differently,
		// but usually annoy if not set like this.
		// (drawRect & setFillStyle change).
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		// fillStyle needs to be persistent.
#if 0
		GLboolean pstipple = glIsEnabled(GL_POLYGON_STIPPLE);
#else
		glPushAttrib(GL_LINE_BIT);
#endif
		glDisable(GL_POLYGON_STIPPLE);

		bindTexture();
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, params_->envmode_);
		glBegin(GL_QUADS);
		glTexCoord2f(tx0, ty0);
		glVertex3f(x0, y0, z_);
		glTexCoord2f(tx0, ty1);
		glVertex3f(x0, y1, z_);
		glTexCoord2f(tx1, ty1);
		glVertex3f(x1, y1, z_);
		glTexCoord2f(tx1, ty0);
		glVertex3f(x1, y0, z_);
		glEnd();

		glDisable(GL_TEXTURE_2D);
#if 0
		if(pstipple)				// Restore state if necessary.
			glEnable(GL_POLYGON_STIPPLE);
#else
		glPopAttrib();
#endif
	}


// Bind a texture id.
	GLuint GLPCTextureObject::bindTexture() {
		glBindTexture(GL_TEXTURE_2D, texid_);
		return texid_;
	}

////////////////
//	A list of texture objects. Used for caching.
// Creates as many texture objects as are necessary to hold the image
// then copies the image to them.
	GLPCTexturedImage::GLPCTexturedImage(GLsizei dWidth, GLsizei dHeight,
	                                     const GLushort *pixels,
	                                     const GLPCTextureParams *params) :
		textures_(NULL), x_(0.0), y_(0.0), z_(0.0),
		xscale_(1.0), yscale_(0.0), dWidth_(dWidth), dHeight_(dHeight) {
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize_);

		params_ = new GLPCTextureParams(params);

		GLint maxtexturesize =
		    (params_->maxtexturesize_ == 0) ? maxTextureSize_
		    : params_->maxtexturesize_;

		// number of textures in x & y directions.
		numTextures(dWidth, dHeight, ntx_, nty_, maxtexturesize);
		numTextures_ = ntx_*nty_;
		textures_ = NULL;

		if(numTextures_ <= 0)
			return;

		textures_ =  new (GLPCTextureObject *[numTextures_]);

		createTextures(dWidth, dHeight);
		storeImage(dWidth, dHeight, pixels);
	}

// Create texture object(s) able to hold dWidth & dHeight pixels using the
// current parameter settings.
	void GLPCTexturedImage::createTextures(GLsizei dWidth, GLsizei dHeight) {
		int tindex=0;

		// Create texture objects.
		int remainingHeight = dHeight;
		while(remainingHeight > 0) {
			int height = (remainingHeight <= maxTextureSize_)
			             ? remainingHeight : maxTextureSize_;
			int remainingWidth = dWidth;
			while(remainingWidth > 0) {
				int width = (remainingWidth <= maxTextureSize_)
				            ? remainingWidth : maxTextureSize_;

				if(tindex >= numTextures_)
					throw(AipsError("Ran out of texture slots."));
				textures_[tindex++] = new GLPCTextureObject(width, height,
				        params_);
				remainingWidth -= width;
			}
			remainingHeight -= height;
		}
	}

// Store an image in the lower left corner of the texture.
	void GLPCTexturedImage::storeImage(GLsizei dWidth, GLsizei dHeight,
	                                   const GLushort *pixels) {
		int tindex=0;
		const GLint xoff=0, yoff=0;	// Always write to LL corner of texture.
		const GLint CPP = GLPCTextureObject::componentsPerPixel(params_->format_);
		int remainingHeight = dHeight;

		while(remainingHeight > 0) {
			int height, remainingWidth = dWidth;
			const GLushort *ptr = pixels;

			while(remainingWidth > 0) {
				if(tindex >= numTextures_)
					throw(AipsError("storeImage ran out of textures."));
				GLPCTextureObject *to = textures_[tindex++];
				int width  = to->tWidth();
				if(width > dWidth)
					width = dWidth;

				height = to->tHeight();
				if(height > dHeight)
					height = dHeight;
				to->storeTexture(xoff, yoff, width, height, dWidth, ptr);
				ptr += width*CPP;
				remainingWidth -= width;
			}
			remainingHeight -= height;	// Use last height retrieved.
			pixels += dWidth*height*CPP;	// since heights are constant
			// across a row.
		}
	}

	GLPCTexturedImage::~GLPCTexturedImage() {
		for(int i=0; i < numTextures_; i++)
			delete textures_[i];
		delete textures_;
		textures_ = NULL;
		numTextures_ = 0;
		delete params_;
		params_ = NULL;
	}

	void GLPCTexturedImage::draw() {
		for(int i=0; i< numTextures_; i++)
			textures_[i]->drawTexture();
	}

// Draw image by drawing the individual textures used to hold it.
	void GLPCTexturedImage::draw(GLfloat x, GLfloat y, GLfloat z,
	                             GLfloat xscale, GLfloat yscale) {
		int tindex=0;

		int remainingHeight = dHeight_;
		float y0 = y;

		while(remainingHeight > 0) {
			int height, remainingWidth = dWidth_;
			float x0 = x;

			while(remainingWidth > 0) {
				if(tindex >= numTextures_)
					throw(AipsError("draw ran out of textures."));
				GLPCTextureObject *to = textures_[tindex++];
				int width  = to->dWidth();
				height = to->dHeight();
				to->drawTexture(x0, y0, z, xscale, yscale);
				x0 += width*xscale;
				remainingWidth -= width;
			}
			remainingHeight -= height;	// Use last height retrieved.
			y0 += height*yscale;
		}
	}

	void GLPCTexturedImage::envmode(GLenum mode) {
		params_->envmode_ = mode;
	}

////////////////////////////////////////////////////////////////
	GLPCTextureParams::GLPCTextureParams() {
		init();
	}

	GLPCTextureParams::GLPCTextureParams(const GLPCTextureParams *p) {
		if(p != NULL) {
			copy(*p);
		} else {
			copy(DEFAULTPARAMS_);
		}
	}

	void GLPCTextureParams::copy(const GLPCTextureParams &p) {
		min_filter_ = p.min_filter_;
		mag_filter_ = p.mag_filter_;

		wrap_s_ = p.wrap_s_;
		wrap_t_ = p.wrap_t_;

		maxtexturesize_ = p.maxtexturesize_;
		envmode_ = p.envmode_;
		format_ = p.format_;
	}

	GLPCTextureParams::GLPCTextureParams(const GLPCTextureParams &p) {
		copy(p);
	}

	void GLPCTextureParams::init() {
		min_filter_ = DEFAULT_FILTER;
		mag_filter_ = DEFAULT_FILTER;;	// GL_TEXTURE_MAG_FILTER
		wrap_s_ = GL_REPEAT;		// GL_TEXTURE_WRAP_S
		wrap_t_ = GL_REPEAT;		// GL_TEXTURE_WRAP_T

		// Maximum texture size to use. Default is host's max texture size.
		maxtexturesize_ = 0;
		envmode_ = GL_DECAL;	// GL_DECAL, GL_BLEND or GL_MODULATE.
		format_ = GL_RGB;
	}

	GLPCTextureParams::~GLPCTextureParams() {
	}

	void GLPCTextureParams::filter(GLint minFilter, GLint magFilter) {
		min_filter_ = minFilter;
		mag_filter_ = (magFilter == 0) ? minFilter : magFilter;
	}

	void GLPCTextureParams::wrap(GLint wrapS, GLint wrapT) {
		wrap_s_ = wrapS;
		wrap_t_ = (wrapT == 0) ? wrapS : wrapT;
	}

	void GLPCTextureParams::maxTextureSize(GLint maxtexturesize) {
		maxtexturesize_ = GLPCTextureObject::pow2(maxtexturesize);
	}

	void GLPCTextureParams::format(GLenum fmt) {
		format_ = fmt;
	}

} //# NAMESPACE CASA - END

#endif // OGL
