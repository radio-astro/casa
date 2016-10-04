#include <gtest/gtest.h>

#include <imageanalysis/ImageAnalysis/test/ImageMaskHandlerTest.h>
#include <imageanalysis/ImageAnalysis/ImageMaskHandler.h>
#include <imageanalysis/Annotations/AnnCircle.h>

#include <images/Images/TempImage.h>
#include <lattices/Lattices/ArrayLattice.h>

using namespace std;

using namespace casacore;
using namespace casa;

namespace test {

TEST_F(ImageMaskHandlerTest, copy) {
    auto csys = CoordinateUtil::defaultCoords3D();
    SPIIF tmp(
        new TempImage<Float>(
            IPosition(3, 20, 20, 20), csys
        )
    );
    ImageMaskHandler<Float> imh(tmp);
    {
        TempImage<Float> x(IPosition(3, 20, 20, 19), csys);
        EXPECT_THROW(imh.copy(x), AipsError);
    }
    TempImage<Float> y(IPosition(3, 20, 20, 20), csys);
    imh.copy(y);

    EXPECT_FALSE(tmp->hasPixelMask());
    Array<Bool> mask(tmp->shape(), true);
    mask(IPosition(3, 0, 0, 0)) = false;
    y.attachMask(ArrayLattice<Bool>(mask));
    imh.copy(y);
    mask = tmp->pixelMask().get().copy();
    EXPECT_FALSE(mask(IPosition(3, 0, 0, 0)));
    EXPECT_TRUE(nfalse(mask) == 1);

    UnitMap::putUser("pix", UnitVal(1.0), "pixel units");
    AnnCircle circle(
        Quantity(9, "pix"), Quantity(9, "pix"),
        Quantity(10, "pix"), csys,
        tmp->shape(), Vector<Stokes::StokesTypes>()
    );
    auto subim = SubImageFactory<Float>::createSubImageRO(
        *tmp, circle.getRegion2()->toRecord(""), "", nullptr
    );
    imh.copy(*subim);
    mask = tmp->pixelMask().get().copy();
    EXPECT_TRUE(ntrue(mask) == 6300);
    EXPECT_TRUE(nfalse(mask) == 1700);

    casacore::String name = "xyz.im";
    SPIIF paged(new PagedImage<Float>(tmp->shape(), csys, name));
    paged->flush();
    auto data = paged->get();
    data(IPosition(3, 9, 9, 9)) = 1;
    paged->put(data);
    {
        // enclose in a block so we have no remaining references when
        // we try to delete the image
        ImageMaskHandler<Float> imh2(paged);
        auto subim2 = SubImageFactory<Float>::createSubImageRO(
            *paged, circle.getRegion2()->toRecord(""), name + "<0.5", nullptr
        );
        imh2.copy(*subim2);
        mask = paged->pixelMask().get().copy();
        EXPECT_TRUE(ntrue(mask) == 6299);
        EXPECT_TRUE(nfalse(mask) == 1701);
    }
    ImageFactory::remove(paged, false);
}
}

int main (int nArgs, char * args []) {
    ::testing::InitGoogleTest(& nArgs, args);
    cout << "MSChecker test " << endl;
    return RUN_ALL_TESTS();
}
