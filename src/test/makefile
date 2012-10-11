
CASACORE_INCD := /Users/drs/develop/casa/osx/include/casacore
CASACORE_LIBD := /Users/drs/develop/casa/osx/lib

INCD := ../include/graphpad

all: test_image_labeling test_image_affine test_image test_colormap

test_image_labeling: test_image_labeling.cc $(INCD)/Colormap.h $(INCD)/Functional.h $(INCD)/couplers/CasaColormapGen.h $(INCD)/couplers/CasaImageGen.h
	c++ -I../include -I$(CASACORE_INCD) -I/usr/include/cfitsio -Wall -g test_image_labeling.cc -o test_image_labeling -L$(CASACORE_LIBD) -lcasa_images -lcasa_casa -lcasa_tables -lcasa_lattices -lcasa_components -lcasa_coordinates -L/opt/local/lib -lagg

test_image_affine: test_image_affine.cc $(INCD)/Colormap.h $(INCD)/Functional.h $(INCD)/couplers/CasaColormapGen.h $(INCD)/couplers/CasaImageGen.h
	c++ -I../include -I$(CASACORE_INCD) -I/usr/include/cfitsio -Wall -g test_image_affine.cc -o test_image_affine -L$(CASACORE_LIBD) -lcasa_images -lcasa_casa -lcasa_tables -lcasa_lattices -lcasa_components -lcasa_coordinates -L/opt/local/lib -lagg

test_image: test_image.cc $(INCD)/Colormap.h $(INCD)/Functional.h $(INCD)/couplers/CasaColormapGen.h $(INCD)/couplers/CasaImageGen.h
	c++ -I../include -I$(CASACORE_INCD) -I/usr/include/cfitsio -Wall -g test_image.cc -o test_image -L$(CASACORE_LIBD) -lcasa_images -lcasa_casa -lcasa_tables -lcasa_lattices -lcasa_components -lcasa_coordinates

test_colormap: test_colormap.cc $(INCD)/Colormap.h $(INCD)/couplers/CasaColormapGen.h
	c++ -I../include -I$(CASACORE_INCD) -I/usr/include/cfitsio -Wall -g test_colormap.cc -o test_colormap -L$(CASACORE_LIBD) -lcasa_casa -lcasa_tables
