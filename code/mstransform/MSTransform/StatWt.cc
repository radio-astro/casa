//#
//#  CASA - Common Astronomy Software Applications (http://casa.nrao.edu/)
//#  Copyright (C) Associated Universities, Inc. Washington DC, USA 2011, All rights reserved.
//#  Copyright (C) European Southern Observatory, 2011, All rights reserved.
//#
//#  This library is free software; you can redistribute it and/or
//#  modify it under the terms of the GNU Lesser General Public
//#  License as published by the Free software Foundation; either
//#  version 2.1 of the License, or (at your option) any later version.
//#
//#  This library is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY, without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//#  Lesser General Public License for more details.
//#
//#  You should have received a copy of the GNU Lesser General Public
//#  License along with this library; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//#  MA 02111-1307  USA

#include <casacore/casa/Containers/ValueHolder.h>
#include <casacore/tables/Tables/TableProxy.h>

#include <mstransform/MSTransform/StatWt.h>
#include <mstransform/TVI/StatWtTVI.h>
#include <mstransform/TVI/StatWtTVILayerFactory.h>
#include <msvis/MSVis/ViImplementation2.h>
#include <msvis/MSVis/IteratingParameters.h>
#include <msvis/MSVis/LayeredVi2Factory.h>

using namespace casacore;

namespace casa { 

StatWt::StatWt(MeasurementSet* ms) : _ms(ms) {
    ThrowIf(! _ms, "Input MS pointer cannot be NULL");
}

StatWt::~StatWt() {}

void StatWt::setOutputMS(const casacore::String& outname) {
    _outname = outname;
}

void StatWt::writeWeights() const {
    Record config;
    vi::IteratingParameters ipar(200); // seconds
    vi::VisIterImpl2LayerFactory data(_ms, ipar, True/*,useMSIter2*/);
    vi::StatWtTVILayerFactory statWtLayerFactory(config);
    Vector<vi::ViiLayerFactory*> facts(2);
    facts[0] = &data;
    facts[1] = &statWtLayerFactory;
    vi::VisibilityIterator2 vi(facts);

    vi::VisBuffer2 *vb = vi.getVisBuffer();
    TableProxy tp(*_ms);
    Slice defaultSlice;
    Vector<Int> vr(1);
    static const String WEIGHT = "WEIGHT";
    static const String WEIGHT_SPECTRUM = "WEIGHT_SPECTRUM";
    static const String FLAG = "FLAG";
    static const String FLAG_ROW = "FLAG_ROW";
    for (vi.originChunks(); vi.moreChunks(); vi.nextChunk()) {
        auto doWtSp = vi.weightSpectrumExists();
        for (vi.origin(); vi.more(); vi.next()) {
            const auto rownr = vb->rowIds();
            const auto rend = rownr.end();
            uInt i = 0;
            if (doWtSp) {
                Cube<Float> wtsp = vb->weightSpectrum();
                for (auto riter=rownr.begin(); riter!=rend; ++riter, ++i) {
                    *vr.begin() = *riter;
                    auto wtSpSlice = wtsp(defaultSlice, defaultSlice, i);
                    auto sliceShape = wtSpSlice.shape();
                    auto newCell = wtSpSlice.reform(IPosition(2, sliceShape[0], sliceShape[1]));
                    tp.putCell(WEIGHT_SPECTRUM, vr, ValueHolder(newCell));
                }
            }
            Matrix<Float> wt = vb->weight();
            Cube<Bool> flag;
            Vector<Bool> flagRow;
            auto wtv = wt.tovector();
            Bool updateFlags = std::find(wtv.begin(), wtv.end(), 0) != wtv.end();
            if (updateFlags) {
                flag = vb->flagCube();
                flagRow = vb->flagRow();
            }
            i = 0;
            for (auto riter=rownr.begin(); riter!=rend; ++riter, ++i) {
                *vr.begin() = *riter;
                auto wtSlice = wt(defaultSlice, i);
                auto newCell = wtSlice.reform(IPosition(1, wtSlice.size()));
                tp.putCell(WEIGHT, vr, ValueHolder(newCell));
                if (updateFlags) {
                    auto flagSlice = flag(defaultSlice, defaultSlice, i);
                    auto sliceShape = flagSlice.shape();
                    Array<Bool> flagCell = flagSlice.reform(IPosition(2, sliceShape[0], sliceShape[1]));
                    tp.putCell(FLAG, vr, ValueHolder(flagCell));
                    tp.putCell(FLAG_ROW, vr, ValueHolder(flagRow[i]));
                }
            }
        }
    }
}

}

