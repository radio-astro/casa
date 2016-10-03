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

		virtual casacore::Bool staticSize() {
			return true;
		}

		virtual casacore::Bool resize(casacore::uInt /*nReds*/, casacore::uInt /*nGreens*/, casacore::uInt /*nBlues*/) {
			return false;
		}

		virtual void nColors(casacore::uInt &n1, casacore::uInt &n2, casacore::uInt &n3) const {
			n1 = 256;
			n2 = 256;
			n3 = 256;
		}

		virtual casacore::uInt depth() const {
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

		virtual casacore::uInt nColors() const {
			return size_;
		}
		// Current number of slots usable by colormaps.
		// can be expanded at runtime up to MAX_QTPCCT_SIZE.

		virtual casacore::uInt nSpareColors() const {
			return MAX_QTPCCT_SIZE - size_;
		}

		virtual casacore::Bool resize(casacore::uInt newSize);

		// Called by corresp. ColormapManager to actually fill color slots
		// for the Colormaps.  r,g,b ahould be in range [0,1].
		virtual casacore::Bool installRGBColors(const casacore::Vector<casacore::Float> & r,
		                              const casacore::Vector<casacore::Float> & g,
		                              const casacore::Vector<casacore::Float> & b,
		                              const casacore::Vector<casacore::Float> & alpha,
		                              casacore::uInt offset = 0);

		// Translate casacore::Array of colormap indices into graphics platform's colors.

		//# The version actually used...
		virtual void mapToColor(const Colormap * map,
		                        casacore::Array<casacore::uInt> & outArray,
		                        const casacore::Array<casacore::uInt> & inArray,
		                        casacore::Bool rangeCheck = true) const;


		//# The one of these really needed when/if non-Index (3-channel)
		//  ColorModels are implemented for QtPixelCanvas.
		virtual void mapToColor3(casacore::Array<casacore::uLong> & /*out*/,
		                         const casacore::Array<casacore::uInt> & /*chan1in*/,
		                         const casacore::Array<casacore::uInt> & /*chan2in*/,
		                         const casacore::Array<casacore::uInt> & /*chan3in*/) {  }


		//# irrelevant, unused versions:
		virtual void mapToColor(const Colormap * /*map*/,
		                        casacore::Array<casacore::uChar> & /*outArray*/,
		                        const casacore::Array<casacore::uChar> & /*inArray*/,
		                        casacore::Bool /*rangeCheck*/ = true) const {  }

		virtual void mapToColor(const Colormap * /*map*/,
		                        casacore::Array<casacore::uShort> & /*outArray*/,
		                        const casacore::Array<casacore::uShort> & /*inArray*/,
		                        casacore::Bool /*rangeCheck*/ = true) const {  }

		virtual void mapToColor(const Colormap * /*map*/,
		                        casacore::Array<casacore::uLong> & /*outArray*/,
		                        const casacore::Array<casacore::uLong> & /*inArray*/,
		                        casacore::Bool /*rangeCheck*/ = true) const {  }

		virtual void mapToColor(const Colormap * /*map*/,
		                        casacore::Array<casacore::uChar> & /*inOutArray*/,
		                        casacore::Bool /*rangeCheck*/ = true) const {  }

		virtual void mapToColor(const Colormap * /*map*/,
		                        casacore::Array<casacore::uShort> & /*inOutArray*/,
		                        casacore::Bool /*rangeCheck*/ = true) const {  }

		virtual void mapToColor(const Colormap * /*map*/,
		                        casacore::Array<casacore::uInt> & /*inOutArray*/,
		                        casacore::Bool /*rangeCheck*/ = true) const {  }

		virtual void mapToColor(const Colormap * /*map*/,
		                        casacore::Array<casacore::uLong> & /*inOutArray*/,
		                        casacore::Bool /*rangeCheck*/ = true) const {  }

		virtual void mapToColor3(casacore::Array<casacore::uLong> & /*out*/,
		                         const casacore::Array<casacore::Float> & /*chan1in*/,
		                         const casacore::Array<casacore::Float> & /*chan2in*/,
		                         const casacore::Array<casacore::Float> & /*chan3in*/) {  }
		virtual void mapToColor3(casacore::Array<casacore::uLong> & /*out*/,
		                         const casacore::Array<casacore::Double> & /*chan1in*/,
		                         const casacore::Array<casacore::Double> & /*chan2in*/,
		                         const casacore::Array<casacore::Double> & /*chan3in*/) {  }

		virtual void mapToColor3(casacore::Array<casacore::uLong> & /*out*/,
		                         const casacore::Array<casacore::uShort> & /*chan1in*/,
		                         const casacore::Array<casacore::uShort> & /*chan2in*/,
		                         const casacore::Array<casacore::uShort> & /*chan3in*/) {  }

		virtual casacore::Bool colorSpaceMap(Display::ColorModel,
		                           const casacore::Array<casacore::Float> & /*chan1in*/,
		                           const casacore::Array<casacore::Float> & /*chan2in*/,
		                           const casacore::Array<casacore::Float> & /*chan3in*/,
		                           casacore::Array<casacore::Float> & /*chan1out*/,
		                           casacore::Array<casacore::Float> & /*chan2out*/,
		                           casacore::Array<casacore::Float> & /*chan3out*/) {
			return false;
		}
		// (could move X11 version down to base class and use
		// that -- it has no X11 dependencies...)

	protected:

		casacore::Int clrByte(casacore::Float f) {
			return casacore::Int(casacore::min( casacore::max(0.f,f)*256.f, 255.f ));
		}
		// translate color component in the range [0., 1.]
		// to clrByte, with 0 <= clrByte < 256

	private:

		casacore::Vector<casacore::uInt> maps_;
		// actual storage for the color slots.
		casacore::uInt size_;
		// Current size of the colortable; number of slots usable by colormaps.
		// can be expanded at runtime, up to MAX_QTPCCT_SIZE.


	};


}
#endif
