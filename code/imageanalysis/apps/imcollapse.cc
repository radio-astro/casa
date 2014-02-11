#include <casa/Inputs/Input.h>
#include <images/Images/ImageUtilities.h>
#include <imageanalysis/ImageAnalysis/ImageCollapser.h>
#include <casa/namespace.h>

Int main(Int argc, char *argv[]) {
	Input input(1);
	input.version("$ID:$");
	input.create("imagename");
	input.create("box", "");
	input.create("region", "");
	input.create("chans", CasacRegionManager::ALL);
	input.create("stokes", CasacRegionManager::ALL);
	input.create("mask", "");
	input.create("axis", "");
	input.create("function","");
	input.create("outname");
	input.create("overwrite", "F");

	input.readArguments(argc, argv);
	String imagename = input.getString("imagename");
	String box = input.getString("box");
	String region = input.getString("region");
	String chans = input.getString("chans");
	String stokes = input.getString("stokes");
	String mask = input.getString("mask");
	IPosition axes(1, input.getInt("axis"));
	String function = input.getString("function");
	String outname = input.getString("outname");
	Bool overwrite = input.getBool("overwrite");
	ImageInterface<Float> *myim = 0;
	LogIO mylog;
	ImageUtilities::openImage(myim, imagename);
	if (myim == 0) {
		mylog << "Unable to open image " << imagename << LogIO::EXCEPTION;
	}
	Record *regionPtr = 0;
	SPCIIF image(myim);
    ImageCollapser<Float> imCollapser(
		function, image, region, regionPtr, box,
		chans, stokes, mask, axes, outname,
        overwrite
    );

	imCollapser.collapse(False);

    return 0;
}



