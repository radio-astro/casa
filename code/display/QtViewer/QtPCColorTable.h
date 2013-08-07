#if !defined(QTPCCOLORTABLE_H)
#define QTPCCOLORTABLE_H

#include <vector>

#include <casa/aips.h>

#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <display/Display/DisplayEnums.h>
#include <display/Display/PixelCanvasColorTable.h>
#include <casa/BasicMath/Math.h>

#include <graphics/X11/X_enter.h>
#include <QtGui/QImage>
#include <QtGui/QColor>
#include <graphics/X11/X_exit.h>

namespace casa {

	class QtPCColorTable : public PixelCanvasColorTable {

	public:

		QtPCColorTable();
		virtual ~QtPCColorTable() {  }


		//# only-vaguely-informative boilerplate...

		virtual Bool staticSize() {
			return True;
		}

		virtual Bool resize(uInt /*nReds*/, uInt /*nGreens*/, uInt /*nBlues*/) {
			return False;
		}

		virtual void nColors(uInt &n1, uInt &n2, uInt &n3) const {
			n1 = 256;
			n2 = 256;
			n3 = 256;
		}

		virtual uInt depth() const {
			return 32;
		}
		// The non-indexed 'true' color depth that Qt supports, and
		// the number of bits in a Qrgb.  This includes room for the
		// alpha channel (yay).  FYI, component byte order in Qrgb
		// (most- to least-significant) is A-R-G-B.

		virtual Display::ColorModel colorModel() const {
			return Display::Index;
		}


# define MAX_QTPCCT_SIZE 5000u
		// maximum color slots this colortable can hold (distributed
		// among its colormaps).  Could be expanded a lot, but it's
		// probably unnecessary.

# define INITIAL_QTPCCT_SIZE 256u
		// Initial size of the colortable.

		virtual uInt nColors() const {
			return size_;
		}
		// Current number of slots usable by colormaps.
		// can be expanded at runtime up to MAX_QTPCCT_SIZE.

		virtual uInt nSpareColors() const {
			return MAX_QTPCCT_SIZE - size_;
		}

		virtual Bool resize(uInt newSize);

		// Called by corresp. ColormapManager to actually fill color slots
		// for the Colormaps.  r,g,b ahould be in range [0,1].
		virtual Bool installRGBColors(const Vector<Float> & r,
		                              const Vector<Float> & g,
		                              const Vector<Float> & b,
		                              const Vector<Float> & alpha,
		                              uInt offset = 0);

		// Translate Array of colormap indices into graphics platform's colors.

		//# The version actually used...
		virtual void mapToColor(const Colormap * map,
		                        Array<uInt> & outArray,
		                        const Array<uInt> & inArray,
		                        Bool rangeCheck = True) const;


		//# The one of these really needed when/if non-Index (3-channel)
		//  ColorModels are implemented for QtPixelCanvas.
		virtual void mapToColor3(Array<uLong> & /*out*/,
		                         const Array<uInt> & /*chan1in*/,
		                         const Array<uInt> & /*chan2in*/,
		                         const Array<uInt> & /*chan3in*/) {  }


		//# irrelevant, unused versions:
		virtual void mapToColor(const Colormap * /*map*/,
		                        Array<uChar> & /*outArray*/,
		                        const Array<uChar> & /*inArray*/,
		                        Bool /*rangeCheck*/ = True) const {  }

		virtual void mapToColor(const Colormap * /*map*/,
		                        Array<uShort> & /*outArray*/,
		                        const Array<uShort> & /*inArray*/,
		                        Bool /*rangeCheck*/ = True) const {  }

		virtual void mapToColor(const Colormap * /*map*/,
		                        Array<uLong> & /*outArray*/,
		                        const Array<uLong> & /*inArray*/,
		                        Bool /*rangeCheck*/ = True) const {  }

		virtual void mapToColor(const Colormap * /*map*/,
		                        Array<uChar> & /*inOutArray*/,
		                        Bool /*rangeCheck*/ = True) const {  }

		virtual void mapToColor(const Colormap * /*map*/,
		                        Array<uShort> & /*inOutArray*/,
		                        Bool /*rangeCheck*/ = True) const {  }

		virtual void mapToColor(const Colormap * /*map*/,
		                        Array<uInt> & /*inOutArray*/,
		                        Bool /*rangeCheck*/ = True) const {  }

		virtual void mapToColor(const Colormap * /*map*/,
		                        Array<uLong> & /*inOutArray*/,
		                        Bool /*rangeCheck*/ = True) const {  }

		virtual void mapToColor3(Array<uLong> & /*out*/,
		                         const Array<Float> & /*chan1in*/,
		                         const Array<Float> & /*chan2in*/,
		                         const Array<Float> & /*chan3in*/) {  }
		virtual void mapToColor3(Array<uLong> & /*out*/,
		                         const Array<Double> & /*chan1in*/,
		                         const Array<Double> & /*chan2in*/,
		                         const Array<Double> & /*chan3in*/) {  }

		virtual void mapToColor3(Array<uLong> & /*out*/,
		                         const Array<uShort> & /*chan1in*/,
		                         const Array<uShort> & /*chan2in*/,
		                         const Array<uShort> & /*chan3in*/) {  }

		virtual Bool colorSpaceMap(Display::ColorModel,
		                           const Array<Float> & /*chan1in*/,
		                           const Array<Float> & /*chan2in*/,
		                           const Array<Float> & /*chan3in*/,
		                           Array<Float> & /*chan1out*/,
		                           Array<Float> & /*chan2out*/,
		                           Array<Float> & /*chan3out*/) {
			return False;
		}
		// (could move X11 version down to base class and use
		// that -- it has no X11 dependencies...)

	protected:

		Int clrByte(Float f) {
			return Int(min( max(0.f,f)*256.f, 255.f ));
		}
		// translate color component in the range [0., 1.]
		// to clrByte, with 0 <= clrByte < 256

	private:

		Vector<uInt> maps_;
		// actual storage for the color slots.
		uInt size_;
		// Current size of the colortable; number of slots usable by colormaps.
		// can be expanded at runtime, up to MAX_QTPCCT_SIZE.


	};


}
#endif
