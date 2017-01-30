#include <imageanalysis/ImageAnalysis/StatImageCreator.h>

#include <casacore/images/Images/ImageStatistics.h>

#include <imageanalysis/Annotations/AnnCenterBox.h>
#include <imageanalysis/Annotations/AnnCircle.h>

// debug
#include <components/ComponentModels/C11Timer.h>
#include <casacore/casa/Arrays/ArrayIO.h>
#include <casacore/casa/BasicSL/STLIO.h>
namespace casa {

StatImageCreator::StatImageCreator(
    const SPCIIF image, const Record *const region,
	const String& mask, const String& outname, Bool overwrite
) : ImageStatsConfigurator(image, region, mask, outname, overwrite) {
    this->_construct();
	auto da = _getImage()->coordinates().directionAxesNumbers();
    _dirAxes[0] = da[0];
    _dirAxes[1] = da[1];
	setAnchorPosition(0, 0);
}

void StatImageCreator::setRadius(const Quantity& radius) {
    _xlen = radius;
    _ylen = Quantity(0,"");
}

void StatImageCreator::setRectangle(
    const Quantity& xLength, const Quantity& yLength
) {
    _xlen = xLength;
    _ylen = yLength;
}

void StatImageCreator::setAnchorPosition(Int x, Int y) {
    Vector<Double> anchorPixel(_getImage()->ndim(), 0);
    anchorPixel[_dirAxes[0]] = x;
    anchorPixel[_dirAxes[1]] = y;
    _anchor = _getImage()->coordinates().toWorld(anchorPixel);
}

void StatImageCreator::setGridSpacing(uInt x, uInt y) {
    _grid.first = x;
    _grid.second = y;
}

SPIIF StatImageCreator::compute() {
    static const AxesSpecifier dummyAxesSpec;
    *_getLog() << LogOrigin(getClass(), __func__);
    auto mylog = _getLog().get();
    auto subImage = SubImageFactory<Float>::createSubImageRO(
        *_getImage(), *_getRegion(), _getMask(),
        mylog, dummyAxesSpec, _getStretch()
    );
    _doMask = ! ImageMask::isAllMaskTrue(*subImage);
    const auto imshape = subImage->shape();
    const auto xshape = imshape[_dirAxes[0]];
    const auto yshape = imshape[_dirAxes[1]];
    const auto& csys = subImage->coordinates();
    auto anchorPixel = csys.toPixel(_anchor);
    Int xanchor = rint(anchorPixel[_dirAxes[0]]);
    Int yanchor = rint(anchorPixel[_dirAxes[1]]);
    String rStr;
    TempImage<Float> output(imshape, csys);
    output.set(0);
    if (_doMask) {
        output.attachMask(ArrayLattice<Bool>(imshape));
        output.pixelMask().set(True);
    }
    // xstart and ystart are the pixel location in the
    // subimage of the lower left corner of the grid,
    // ie they are the pixel location of the grid point
    // with the smallest non-negative x and y values
    Int xstart = xanchor % _grid.first;
    Int ystart = yanchor % _grid.second;
    if (xstart < 0) {
        xstart += _grid.first;
    }
    if (ystart < 0) {
        ystart += _grid.second;
    }
    C11Timer t0, t1, t2, t3, t4, t5;
    Array<Float> tmp;
    // integer division
    auto nxpts = (xshape - xstart)/_grid.first;
    if ((xshape - xstart) % _grid.first != 0) {
        ++nxpts;
    }
    auto nypts = (yshape - ystart)/ _grid.second;
    if ((yshape - ystart) % _grid.second != 0) {
        ++nypts;
    }
    IPosition storeShape = imshape;
    storeShape[_dirAxes[0]] = nxpts;
    storeShape[_dirAxes[1]] = nypts;
    auto interpolate = _grid.first > 1 || _grid.second > 1;
    TempImage<Float>* writeTo = &output;
    std::unique_ptr<TempImage<Float>> store;
    if (interpolate) {
        store.reset(new TempImage<Float>(storeShape, csys));
        store->set(0.0);
        if (_doMask) {
            store->attachMask(ArrayLattice<Bool>(storeShape));
            store->pixelMask().set(True);
        }
        writeTo = store.get();
    }
    _computeStorage(
        writeTo, subImage, nxpts, nypts,
        xstart, ystart
    );
    if (interpolate) {
        _doInterpolation(
            output, *store,
            subImage,  nxpts,  nypts,
            xstart,  ystart
        );
    }
    return _prepareOutputImage(output);
}

void StatImageCreator::_computeStorage(
    TempImage<Float> *const& writeTo, SPCIIF subImage,
    uInt nxpts, uInt nypts, Int xstart, Int ystart
) {
    const auto imshape = subImage->shape();
    const auto xshape = imshape[_dirAxes[0]];
    const auto yshape = imshape[_dirAxes[1]];
    const auto& csys = subImage->coordinates();
    const auto ngrid = nxpts * nypts;
    ProgressMeter pm(0, ngrid, "Processing stats at grid points");
    Quantity xq(0, "pix");
    Quantity yq(0, "pix");
    const auto doCircle = _ylen.getValue() == 0;
    *_getLog() << LogIO::NORMAL << "Using a ";
    uInt ptsCount = 0;
    static const AxesSpecifier dummyAxesSpec;
    static const Vector<Stokes::StokesTypes> dummyStokes;
    Record reg;
    auto ndim = subImage->ndim();
    IPosition impos(ndim, 0);
    auto storePos = impos;
    auto arrShape = imshape;
    arrShape[_dirAxes[0]] = 1;
    arrShape[_dirAxes[1]] = 1;
    Array<Float> stat(arrShape);
    const Array<Float> zeroArr(arrShape, 0.0);
    const Array<Bool> maskArr(arrShape, False);
    _resetStats(
        new ImageStatistics<Float>(
            TempImage<Float>(IPosition(ndim, 1), csys), False
        )
    );
    auto& statsCalc = _getImageStats();
    auto algString = _configureAlgorithm();
    statsCalc->setAxes(_dirAxes.asVector());
    if (doCircle) {
        *_getLog() << "circular region of radius " << _xlen;
    }
    else {
        *_getLog() << "rectangular region of dimensions " << _xlen
            << " x " << _ylen;
    }
    *_getLog() << " to choose pixels for computing " << _statName
        << " using the " << algString << " algorithm around each of "
        << ngrid << " grid points." << LogIO::POST;
    for (uInt y=ystart; y<yshape; y+=_grid.second, ++storePos[_dirAxes[1]]) {
        impos[_dirAxes[1]] = y;
        yq.setValue(y);
        storePos[_dirAxes[0]] = 0;
        for (uInt x=xstart; x < xshape; x+=_grid.first, ++storePos[_dirAxes[0]]) {
            xq.setValue(x);
            impos[_dirAxes[0]] = x;
            if (doCircle) {
                AnnCircle circle(
                    xq, yq, _xlen, csys, imshape, dummyStokes
                );
                reg = circle.getRegion()->toRecord("");
            }
            else {
                AnnCenterBox box(
                    xq, yq, _xlen, _ylen, csys, imshape, dummyStokes
                );
                reg = box.getRegion()->toRecord("");
            }
            auto chunkImage = SubImageFactory<Float>::createSubImageRO(
                *subImage, reg, "", nullptr, dummyAxesSpec, False
            );
            statsCalc->setNewImage(*chunkImage, False);
            statsCalc->getConvertedStatistic(stat, _statType);
            if (stat.empty()) {
                // no stats, pixels were masked
                writeTo->putSlice(zeroArr, storePos);
                writeTo->pixelMask().putSlice(maskArr, storePos);
            }
            else {
                writeTo->putSlice(stat.reform(arrShape), storePos);
            }
        }
        ptsCount += nxpts;
        pm.update(ptsCount);
    }
}

void StatImageCreator::_doInterpolation(
    TempImage<Float>& output, TempImage<Float>& store,
    SPCIIF subImage, uInt nxpts, uInt nypts,
    Int xstart, Int ystart
) const {
    const auto imshape = subImage->shape();
    const auto xshape = imshape[_dirAxes[0]];
    const auto yshape = imshape[_dirAxes[1]];
    const auto ndim = subImage->ndim();
    *_getLog() << LogIO::NORMAL << "Interpolate using "
        << _interpName << " algorithm." << LogIO::POST;
    Matrix<Float> result(xshape, yshape);
    Matrix<Bool> resultMask;
    if (_doMask) {
        resultMask.resize(IPosition(2, xshape, yshape));
        resultMask.set(True);
    }
    Matrix<Float> storage(nxpts, nypts);
    Matrix<Bool> storeMask(nxpts, nypts);
    std::pair<uInt, uInt> start;
    start.first = xstart;
    start.second = ystart;
    if (ndim == 2) {
        store.get(storage);
        store.getMask(storeMask);
        _interpolate(result, resultMask, storage, storeMask, start);
        output.put(result);
        if (_doMask) {
            output.pixelMask().put(resultMask);
        }
    }
    else {
        // get each direction plane in the storage lattice at each chan/stokes
        IPosition cursorShape(ndim, 1);
        cursorShape[_dirAxes[0]] = nxpts;
        cursorShape[_dirAxes[1]] = nypts;
        auto axisPath = _dirAxes;
        axisPath.append((IPosition::otherAxes(ndim,_dirAxes)));
        LatticeStepper stepper(store.shape(), cursorShape, axisPath);
        Slicer slicer(stepper.position(), stepper.endPosition(), Slicer::endIsLast);
        for (stepper.reset(); ! stepper.atEnd(); stepper++) {
            auto pos = stepper.position();
            slicer.setStart(pos);
            slicer.setEnd(stepper.endPosition());
            storage = store.getSlice(slicer, True);
            storeMask = store.getMaskSlice(slicer, True);
            _interpolate(result, resultMask, storage, storeMask, start);
            output.putSlice(result, pos);
            if (_doMask) {
                output.pixelMask().putSlice(resultMask, pos);
            }
        }
    }
}

void StatImageCreator::setInterpAlgorithm(Interpolate2D::Method alg) {
    switch (alg) {
    case Interpolate2D::CUBIC:
        _interpName = "CUBIC";
        return;
    case Interpolate2D::LANCZOS:
        _interpName = "LANCZOS";
        return;
    case Interpolate2D::LINEAR:
        _interpName = "LINEAR";
        return;
    case Interpolate2D::NEAREST:
        _interpName = "NEAREST";
        return;
    default:
        ThrowCc("Unhandled interpolation method " + String::toString(alg));
    }
    _interpolater = Interpolate2D(alg);
}

void StatImageCreator::setStatType(LatticeStatsBase::StatisticsTypes s) {
    ThrowIf(
        s == LatticeStatsBase::FLUX || s == LatticeStatsBase::NSTATS,
        "This application does not support the specified statistic "
            + LatticeStatsBase::toStatisticName(s)
    );
    if (s == LatticeStatsBase::Q1) {
        _statName = "FIRST QUARTILE";
    }
    else if (s == LatticeStatsBase::Q3) {
        _statName = "THIRD QUARTILE";
    }
    else if (s == LatticeStatsBase::QUARTILE) {
        _statName = "INNER QUARTILE RANGE";
    }
    else {
        _statName = LatticeStatsBase::toStatisticName(s);
    }
    ThrowIf(
        _statName.empty(),
        "Logic error: No name found for statistic "
            + String::toString(s)
    );
    _statType = s;
}

void StatImageCreator::setStatType(const String& s) {
    String m = s;
    LatticeStatsBase::StatisticsTypes stat;
    m.downcase();
    if (m.startsWith("i")) {
        stat = LatticeStatsBase::QUARTILE;
    }
    else if (m.startsWith("max")) {
        stat = LatticeStatsBase::MAX;
    }
    else if (m.startsWith("mea")) {
        stat = LatticeStatsBase::MEAN;
    }
    else if (m.startsWith("meda") || m.startsWith("mad")) {
        stat = LatticeStatsBase::MEDABSDEVMED;
    }
    else if (m.startsWith("medi")) {
        stat = LatticeStatsBase::MEDIAN;
    }
    else if (m.startsWith("mi")) {
        stat = LatticeStatsBase::MIN;
    }
    else if (m.startsWith("n")) {
        stat = LatticeStatsBase::NPTS;
    }
    else if (m.startsWith("q1")) {
        stat = LatticeStatsBase::Q1;
    }
    else if (m.startsWith("q3")) {
        stat = LatticeStatsBase::Q3;
    }
    else if (m.startsWith("r")) {
        stat = LatticeStatsBase::RMS;
    }
    else if (m.startsWith("si") || m.startsWith("st")) {
        stat = LatticeStatsBase::SIGMA;
    }
    else if (m.startsWith("sums")) {
        stat = LatticeStatsBase::SUMSQ;
    }
    else if (m.startsWith("sum")) {
        stat = LatticeStatsBase::SUM;
    }
    else if (m.startsWith("v")) {
        stat = LatticeStatsBase::VARIANCE;
    }
    else {
        ThrowCc("Statistic " + s + " not supported.");
    }
    setStatType(stat);
}

void StatImageCreator::_interpolate(
    Matrix<Float>& result, Matrix<Bool>& resultMask,
    const Matrix<Float>& storage,
    const Matrix<Bool>& storeMask,
    const std::pair<uInt, uInt>& start
) const {
    auto shape = result.shape();
    auto imax = shape[0];
    auto jmax = shape[1];
    // x values change fastest in the iterator
    auto iter = result.begin();
    auto miter = resultMask.begin();
    // xcell and ycell are the current positions within the current
    // grid cell represented by an integer modulo pointsPerCell
    auto xCell = start.first == 0 ? 0 : _grid.first - start.first;
    auto yCell = start.second == 0 ? 0 : _grid.second - start.second;
    Int xStoreInt = start.first == 0 ? 0 : -1;
    Int yStoreInt = start.second == 0 ? 0 : -1;
    Double xStoreFrac = (Double)xCell/(Double)_grid.first;
    Double yStoreFrac = (Double)yCell/(Double)_grid.second;
    Vector<Double> storeLoc(2);
    storeLoc[0] = xStoreInt + xStoreFrac;
    storeLoc[1] = yStoreInt + yStoreFrac;
    for (uInt j=0; j<jmax; ++j, ++yCell) {
        Bool onYGrid = yCell == 0;
        if (yCell == _grid.second) {
            yCell = 0;
            ++yStoreInt;
            yStoreFrac = 0;
            onYGrid = True;
        }
        else {
            yStoreFrac = (Double)yCell/(Double)_grid.second;
        }
        storeLoc[1] = yStoreInt + yStoreFrac;
        xCell = start.first == 0 ? 0 : _grid.first - start.first;
        xStoreInt = start.first == 0 ? 0 : -1;
        xStoreFrac = (Double)xCell/(Double)_grid.first;
        for (uInt i=0; i<imax; ++i, ++xCell) {
            Bool onXGrid = xCell == 0;
            if (xCell == _grid.first) {
                xCell = 0;
                ++xStoreInt;
                onXGrid = True;
            }
            if (onXGrid && onYGrid) {
                // exactly on a grid point, no interpolation needed
                // just copy value directly from storage matrix
                *iter = storage(xStoreInt, yStoreInt);
                if (_doMask) {
                    *miter = storeMask(xStoreInt, yStoreInt);
                }
            }
            else {
                xStoreFrac = (Double)xCell/(Double)_grid.first;
                storeLoc[0] = xStoreInt + xStoreFrac;
                _interpolater.interp(*iter, storeLoc, storage, storeMask);
            }
            ++iter;
            if (_doMask) {
                ++miter;
            }
        }
    }
}

}
