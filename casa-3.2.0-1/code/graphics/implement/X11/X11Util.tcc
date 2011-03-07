//# X11Util.cc: X utilities to assist the X11 classes
//# Copyright (C) 1999,2001
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

#include <casa/stdlib.h>
#include <casa/iostream.h>
#include <graphics/X11/X11Util.h>
#include <casa/Exceptions/Error.h>

namespace casa { //# NAMESPACE CASA - BEGIN

Bool X11TestColormapAlloc(::XDisplay * display, XColormap colormap, 
			  Bool contig, uInt nColors)
{
  Bool retval = False;
  uLong * colors = new uLong[nColors];
  uLong planeMask[1];
  if (XAllocColorCells(display, colormap, contig?1:0, 
		       planeMask, 0, colors, nColors))
    {
      XFreeColors(display, colormap, colors, nColors, 0);
      retval = True;
    } 
  delete [] colors;
  return retval;
}

uInt X11QueryColorsAvailable(::XDisplay * display,
			     XColormap colormap,
			     Bool contig)
{
  // Initialize interval to reasonable guess
  uInt a = 1;
  uInt b = 256;

  // expand b quadratically until it fails
  while (X11TestColormapAlloc(display, colormap, contig, b)) b *= 2;

  // cout << "bisecting ...\n";

  // Use binary search to get the precise number of colors
  while(a < b-1)
    {
      uInt m = (a+b)/2;

      //cout << "a,m,b = <" << a << "," << m << "," << b << ">\n";  cout.flush();

      Bool fm = X11TestColormapAlloc(display, colormap, contig, m);
      if (fm)
	a = m;
      else
	b = m;
    }

  return a;
}

const char * X11DefaultDisplayName()
{
  static char * defaultDisplay = ":0";
  char * dpyname = getenv("DISPLAY");
  if (!dpyname) dpyname = defaultDisplay;

  return (const char *) dpyname;
}

::XDisplay * X11DefaultDisplay()
{
  return XOpenDisplay(X11DefaultDisplayName());
}

Screen * X11DefaultScreen()
{
  ::XDisplay * display = X11DefaultDisplay();
  return DefaultScreenOfDisplay(display);
}

uInt X11DepthOfVisual(::XDisplay * display, Visual * visual)
{
  XVisualInfo vtemplate;

  if (!visual) return 0;

  int nVis;
  XVisualInfo * vlist = XGetVisualInfo(display,
				       VisualNoMask,
				       &vtemplate, &nVis);
  uint depth = 0;
  if (vlist)
    {
      for (XVisualInfo * v = vlist; v < vlist + nVis; v++)
	{
	  if (v->visual == visual)
	    { depth = v->depth; break; }
	}
      
      XFree(vlist);
    }
  return depth;
}


XVisualInfo X11VisualInfoFromVisual(::XDisplay *display, Visual *visual) {
  XVisualInfo vtemplate;
  XVisualInfo *v = 0;
  if (!visual) {
    throw(AipsError("X11VisualInfoFromVisual given null visual"));
  }
  int nVis;
  XVisualInfo *vlist = XGetVisualInfo(display, VisualNoMask,
				      &vtemplate, &nVis);
  if (vlist) {
    for (v = vlist; v < vlist + nVis; v++) {
      if (v->visual == visual) {
	return *v;
      }
    }
    XFree(vlist);
  }
  throw(AipsError("X11VisualInfoFromVisual given non-existent visual"));
  return vtemplate;
}

// Returns best visual in class given
Visual * X11BestVisual(Screen * screen, uInt vclass)
{
  XVisualInfo vtemplate;
  vtemplate.c_class = vclass;
  vtemplate.screen = XScreenNumberOfScreen(screen);

  int nVis;
  XVisualInfo * vlist = XGetVisualInfo(DisplayOfScreen(screen),
				       VisualClassMask | VisualScreenMask,
				       &vtemplate, &nVis);

  uInt matchDepth = 0;
  Visual * matchVisual = 0;

  if (vlist)
    {
      for (XVisualInfo * v = vlist; v < vlist + nVis; v++)
	{
	  if (v->depth > Int(matchDepth))
	    { matchDepth = v->depth; matchVisual = v->visual; }
	}
      
      XFree(vlist);
    }

  return matchVisual;
}

// map vis id to visual
Visual * X11VisualIdToVisual(::XDisplay * display, uInt visId)
{
  XVisualInfo vtemplate;
  int nVis;
  XVisualInfo * vlist = XGetVisualInfo(display,
				       VisualNoMask,
				       &vtemplate, &nVis);

  Visual * matchVisual = 0;
  for (XVisualInfo * v = vlist; v < vlist + nVis; v++)
    {
      if (v->visualid == visId)
	{
	  matchVisual = v->visual;
	  break;
	}
    }

  XFree(vlist);

  return matchVisual;
}

ostream & operator << (ostream & os, const XStandardColormap & mapInfo)
{
  os << "[ colormap=" << mapInfo.colormap
    << ", red_max=" << mapInfo.red_max
    << ", green_max=" << mapInfo.green_max
    << ", blue_max=" << mapInfo.blue_max
    << ", red_mult=" << mapInfo.red_mult
    << ", green_mult=" << mapInfo.green_mult
    << ", blue_mult=" << mapInfo.blue_mult
    << ", base_pixel=" << mapInfo.base_pixel
    << ", visualid=" << mapInfo.visualid
    << ", killid=" << mapInfo.killid << "]";
  return os;
}

ostream & operator << (ostream & os, Visual * visual)
{
  if (visual == NULL) { os << "(null)"; return os; }
  os << "----------------- Visual Information -------------------\n";
  os << "Visual Id : " << XVisualIDFromVisual(visual) << endl;
  os << "Visual Class : ";
  switch(visual->c_class)
    {
    case StaticGray:  os << "StaticGray";  break;
    case GrayScale:   os << "GrayScale";   break;
    case StaticColor: os << "StaticColor"; break;
    case PseudoColor: os << "PseudoColor"; break;
    case TrueColor:   os << "TrueColor";   break;
    case DirectColor: os << "DirectColor"; break;
    }
  os << endl;
  //  os << "Depth        : " << X11DepthOfVisual(visual) << endl;
  os << "red_mask     : " << visual->red_mask << endl;
  os << "green_mask   : " << visual->green_mask << endl;
  os << "blue_mask    : " << visual->blue_mask << endl;
  os << "Colormap Size: " << visual->map_entries << endl;
  os << "Bits per RGB : " << visual->bits_per_rgb << endl;
  os << "-------------------- END VISUAL ------------------------\n";
  return os;
}

ostream & operator << (ostream & os, Screen * screen)
{
  Int i;
  if (screen == NULL) { os << "(null)"; return os; }
  os << "----------------- Screen Information -------------------\n";
  os << "This screen number        : " << XScreenNumberOfScreen(screen) << endl;
  os << "Black pixel               : " << BlackPixelOfScreen(screen) << endl;
  os << "White pixel               : " << WhitePixelOfScreen(screen) << endl;
  os << "Cells of Screen           : " << CellsOfScreen(screen) << endl;
  os << "Backing store             : ";
  switch(DoesBackingStore(screen))
    {
    case WhenMapped:  os << "When Mapped\n"; break;
    case NotUseful: os << "Not Useful\n"; break;
    case Always: os << "Always\n"; break;
    }
  os << "Save Unders               : ";
  switch(DoesSaveUnders(screen))
    {
    case True: os << "Supported\n"; break;
    case False: os << "Not supported\n"; break;
    }
os << "Screen Width (pixels)     : " << XWidthOfScreen(screen) << endl;
  os << "Screen Height (pixels)    : " << XHeightOfScreen(screen) << endl;
  os << "Screen Width (mm)         : " << XWidthMMOfScreen(screen) << endl;
  os << "Screen Height (mm)        : " << XHeightMMOfScreen(screen) << endl;
 
  os << "Depths available          : ";
  int ndepths = 0;
  int * depths = XListDepths(DisplayOfScreen(screen), XScreenNumberOfScreen(screen), &ndepths);
  if (depths)
    {
      for (i = 0; i < ndepths; i++)
        os << depths[i] << " ";
      os << endl;
      XFree(depths);
    }
  else
    os << "Unknown\n";
  os << "Maximum # of installed colormaps : " << XMaxCmapsOfScreen(screen) << endl;
  os << "Installed Colormaps       : ";
  int nColormaps;
  Window w = RootWindowOfScreen(screen);
  XColormap * maps = XListInstalledColormaps(DisplayOfScreen(screen), w, &nColormaps);
  if (maps)
    {
      os << "[" << nColormaps << "] - id's: ";
      for (i = 0; i < nColormaps; i++)
        os << maps[i] << " ";
      os << endl;
      XFree(maps);
    }
  else
    os << "Unknown\n";
  os << "Depth of root window (planes)   : " << XPlanesOfScreen(screen) << endl;

  XVisualInfo vTemplate;
  Int nVisualInfos;
  vTemplate.screen = XScreenNumberOfScreen(screen);
  XVisualInfo * visualInfoList = XGetVisualInfo(DisplayOfScreen(screen), VisualScreenMask, 
						&vTemplate, &nVisualInfos);
 
  for (i = 0; i < nVisualInfos; i++)
    {
      os << "\nVisual ID #" << i+1 << ":\n";
      os << visualInfoList[i].visual;
    }
  os << "------------------- END SCREEN -------------------------\n";
  return os;
}

Widget X11TopLevelWidget(Widget wp)
{
  Widget w = wp;
  while (XtParent(w) != NULL) w = XtParent(w);
  return w;
}

Widget X11ContainingShell(Widget wp)
{
  Widget w = wp;
  while (XtParent(w) != NULL && !XtIsShell(w)) w = XtParent(w);
  return w;
}

Bool X11IsBuiltinMap(Atom property)
{
  return (property == XA_RGB_DEFAULT_MAP ||
		property == XA_RGB_BEST_MAP ||
		property == XA_RGB_RED_MAP ||
		property == XA_RGB_GREEN_MAP ||
		property == XA_RGB_BLUE_MAP ||
		property == XA_RGB_GRAY_MAP);
}

Bool X11InitializeStandardColormap(Screen * screen,
				   Visual * visual,
				   Atom property,
				   XStandardColormap ** mapInfo)
{
  ::XDisplay * display = DisplayOfScreen(screen);

  if (X11IsBuiltinMap(property))
    {
      // 
      // Call utility library to create any standard
      // colormaps not already created for this visual
      XmuVisualStandardColormaps(display, 
				 XScreenNumberOfScreen(screen),
				 visual->visualid,
				 X11DepthOfVisual(display, visual),
				 False,   // replace existing SCM?
				 False);  // retain SCM permanently?
      
      Int count;
      if (XGetRGBColormaps(display, 
			   RootWindowOfScreen(screen),
			   mapInfo,
			   &count,
			   property) == 0)
	{
	  // FAIL1 property is not set
	  return False;
	}
      XStandardColormap * mip = *mapInfo;
      cout << "mip is " << (void *)mip << endl;
      cout << "map from XGetRGBColormaps: " << (*mip) << endl << flush;

      if (mapInfo[0]->colormap != 0)
	{
	  // Map created by another client
	  if (mapInfo[0]->red_max == 0)
	    {
	      // FAIL2 data missing ==> not created properly
	      return False;
	    }
	  else
	    {
	      // Created and usable
	      cout << "inside X11ISC: mapInfo is " << (*mapInfo[0]) << endl << flush;
	      return True;
	    }
	}
      else
	{
	  cerr << "Shouln't get here!!";
	}
    }
  else
    {
      // Placeholder for constructing custom "StandardColorMaps".
      // For example, a high-resolution Hue-Intensity map could
      // be generated using DirectColor visual.
      return False;
    }
  return False;
}


} //# NAMESPACE CASA - END

