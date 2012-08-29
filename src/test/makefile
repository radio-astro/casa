
CASACORE_INCD := /home/warp/casa/dschieb/casa/active/linux/include/casacore
CASACORE_LIBD := /home/warp/casa/dschieb/casa/active/linux/lib

INCD := ../include/graphpad

all: test_image test_colormap

test_image: test_image.cc $(INCD)/Colormap.h $(INCD)/Functional.h $(INCD)/couplers/CasaColormapGen.h $(INCD)/couplers/CasaImageGen.h
	echo $@
	echo $<
	c++ -I../include -I$(CASACORE_INCD) -I/usr/include/cfitsio -Wall -g test_image.cc -o test_image -L$(CASACORE_LIBD) -lcasa_images -lcasa_casa -lcasa_tables

test_colormap: test_colormap.cc $(INCD)/Colormap.h $(INCD)/couplers/CasaColormapGen.h
	c++ -I../include -I$(CASACORE_INCD) -I/usr/include/cfitsio -Wall -g test_colormap.cc -o test_colormap -L$(CASACORE_LIBD) -lcasa_casa -lcasa_tables
