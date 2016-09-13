#include <display/QtViewer/QtPCColorTable.h>
#include <casa/namespace.h>
#include <QDebug>


QtPCColorTable::QtPCColorTable(): maps_(MAX_QTPCCT_SIZE, qRgba(0,0,0,1)),
	size_(INITIAL_QTPCCT_SIZE) {
	resize(size_);
}


Bool QtPCColorTable::resize(uInt newSize) {
//cerr<<"QtPCColorTable::resize" << endl;	//#diag
	if (newSize>MAX_QTPCCT_SIZE) return False;

	size_ = newSize;
	colormapManager().redistributeColormaps();
	return True;
}


Bool QtPCColorTable::installRGBColors(const Vector<Float> & r,
                                      const Vector<Float> & g,
                                      const Vector<Float> & b,
                                      const Vector<Float> & alpha,
                                      uInt offset) {
	// Called by corresp. ColormapManager to actually fill color slots
	// for the Colormaps.  r,g,b should be in range [0,1].

//cout << "QtPCColorTable::installRGBColors" << endl;	//#diag
//cerr<<"clr ofst:"<<offset<<" rsz:"<<r.nelements()<<endl;	//#diag
//cerr<<"szuI:"<<sizeof(uInt)<<" szQRgb:"<<sizeof(QRgb)<<endl;	//#diag
//std::hex(cerr);	//#diag
//std::dec(cerr);	//#diag
	if ( r.nelements() + offset > nColors() ||
	        r.nelements() > g.nelements() ||
	        r.nelements() > b.nelements() ) return False;
	for (uInt i = 0; i < r.nelements(); i++) {
		maps_[i+offset] = qRgba( clrByte(r(i)), clrByte(g(i)), clrByte(b(i)), clrByte(alpha(i)) );
	}
//QColor qc(maps_[i+offset]);	//#diag
//cout<<i+offset<<" "<<qc.red()<<" "<<qc.green()<<" "<<qc.blue()<<
//" rgb:"<<r[i]<<" "<<g[i]<<" "<<b[i]<<endl;	//#diag

	return True;
}




void QtPCColorTable::mapToColor(const Colormap* map, Array<uInt>& outArray,
                                const Array<uInt>& inArray,
                                Bool rangeCheck) const {
	// Translate Array of colormap indices into an Array of
	// graphics-platform-specific colors (Qrgb's, in this case).

	if(outArray.nelements() < inArray.nelements()) {
		outArray.resize(inArray.shape());
	}

	Bool inDel, outDel;

	const uInt* in  =  inArray.getStorage(inDel);
	uInt* out = outArray.getStorage(outDel);

	const uInt* end = in + inArray.nelements();

	uInt offset   = getColormapOffset(map);
	uInt cmapsize = getColormapSize(map);
	uInt maxc     = offset + cmapsize - 1;

	const uInt* inp  = in;
	uInt* outp = out;

	if (rangeCheck)  {
		while(inp < end) {
			uInt ind = (*inp >= cmapsize) ? maxc : offset + *inp;
			*outp++ = (uInt)maps_[ind];
			inp++;
		}
	}
	else {
		while(inp < end){
			int index = offset + *inp++;
			*outp++ = (uInt)maps_[index];
		}
	}
	inArray.freeStorage(in, inDel);
	outArray.putStorage(out, outDel);
}


//#move this to templates...
//#include <qwindowdefs.h>
//template class Vector<QRgb>;


