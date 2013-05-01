////# PanelParams.cc: Plotter class for the TablePlot (tableplot) tool
//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000,2001,2002,2003-2008
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

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>

#include <tools/tables/TablePlot/PanelParams.h>

#define LOG0 0

namespace casa { //# NAMESPACE CASA - BEGIN

PlotOptions::PlotOptions()
{
   /* The long list of standard pre-defined colours */
   /* Some colours that do not contrast well with white, 
           have been left out */
   pylabcolourstring = String("black,default,blue,green,cyan,red,yellow,darkorange,");
   pylabcolourstring += String("aqua,blueviolet,brown,burlywood,");
   pylabcolourstring += String("chartreuse,chocolate,coral,cornflowerblue,");
   pylabcolourstring += String("crimson,darkblue,darkcyan,darkgoldenrod,");
   pylabcolourstring += String("darkgray,darkgreen,darkkhaki,darkmagenta,");
   pylabcolourstring += String("darkolivegreen,darkorange,darkorchid,darkred,");
   pylabcolourstring += String("darksalmon,darkseagreen,darkslateblue,darkslategray,");
   pylabcolourstring += String("darkturquoise,darkviolet,deeppink,deepskyblue,");
   pylabcolourstring += String("dimgray,dodgerblue,firebrick,forestgreen,");
   pylabcolourstring += String("fuchsia,gainsboro,gold,goldenrod,gray,");
   pylabcolourstring += String("greenyellow,hotpink,indianred,indigo,khaki,");
   pylabcolourstring += string("lavender,lawngreen,lightblue,");
   pylabcolourstring += String("lightcoral,lightgoldenrodyellow,lightgreen,");
   pylabcolourstring += String("lightgrey,lightpink,lightsalmon,lightseagreen,");
   pylabcolourstring += String("lightskyblue,lightslategray,lightsteelblue,");
   pylabcolourstring += String("lime,limegreen,maroon,mediumaquamarine,");
   pylabcolourstring += String("mediumblue,mediumorchid,mediumpurple,mediumseagreen,");
   pylabcolourstring += String("mediumslateblue,mediumspringgreen,mediumturquoise,");
   pylabcolourstring += String("mediumvioletred,midnightblue,");
   pylabcolourstring += String("moccasin,navy,olive,olivedrab,");
   pylabcolourstring += String("orange,orchid,orangered,palegoldenrod,palegreen,");
   pylabcolourstring += String("palevioletred,peru,pink,plum,");
   pylabcolourstring += String("powderblue,purple,rosybrown,royalblue,saddlebrown,");
   pylabcolourstring += String("salmon,sandybrown,seagreen,sienna,silver,");
   pylabcolourstring += String("skyblue,slateblue,slategray,springgreen,cornsilk,");
   pylabcolourstring += String("steelblue,tan,teal,thistle,tomato,turquoise,violet,");
   pylabcolourstring += String("wheat,yellowgreen,aquamarine,azure,bisque,cadetblue");
   // no magenta. reserved for flags.

   reset();
}

PlotOptions::~PlotOptions()
{
   if(ConvertIsLocal_p && Convert != NULL)
      delete Convert;
}
   
/* Copy semantics */
PlotOptions& PlotOptions::operator= (PlotOptions &inplop)
{
   
   PanelMap.resize( inplop.PanelMap.nelements() );
   PanelMap = inplop.PanelMap;
   PlotRange.resize( inplop.PlotRange.nelements() );
   PlotRange = inplop.PlotRange;
   PlotRangesSet.resize( inplop.PlotRangesSet.nelements() );
   PlotRangesSet = inplop.PlotRangesSet;


   MultiColour = inplop.MultiColour;
   TableMultiColour = inplop.TableMultiColour;
   ColumnsXaxis = inplop.ColumnsXaxis;
   OverPlot = inplop.OverPlot;
   ReplaceTopPlot = inplop.ReplaceTopPlot;
   RemoveOldPanels = inplop.RemoveOldPanels;
   FontSize = inplop.FontSize;
   
   XLabel = inplop.XLabel;
   YLabel = inplop.YLabel;
   Title = inplop.Title;

   DoScalingCorrection = inplop.DoScalingCorrection;
   SeparateIter = inplop.SeparateIter;
   HonourXFlags = inplop.HonourXFlags;
   
   PlotSymbol = inplop.PlotSymbol;
   ColourString = inplop.ColourString ;
   
   PointLabels.resize( inplop.PointLabels.nelements() );
   PointLabels = inplop.PointLabels;
   LocateColumns.resize( inplop.LocateColumns.nelements() );
   LocateColumns = inplop.LocateColumns;
   
   MarkerSize = inplop.MarkerSize;
   LineWidth =inplop.LineWidth;
   ShowFlags = inplop.ShowFlags;
   FlagExt = inplop.FlagExt;
   FlagVersion = inplop.FlagVersion;
   SkipNRows = inplop.SkipNRows;
   AverageNRows = inplop.AverageNRows;
   Connect = inplop.Connect;

   WindowSize = inplop.WindowSize;
   AspectRatio = inplop.AspectRatio;

   TimePlot_p=inplop.TimePlot_p;
   PlotColour_p = inplop.PlotColour_p;
   PyPlotSymbol_p = inplop.PyPlotSymbol_p;

   Convert = inplop.Convert;
   ConvertIsLocal_p = inplop.ConvertIsLocal_p;
   CallBackHooks = inplop.CallBackHooks;

   TimePlotChar = inplop.TimePlotChar;
   
   NColours = inplop.NColours;
   useLayerColor = inplop.useLayerColor;
   ColourList.resize( inplop.ColourList.nelements() );
   ColourList = inplop.ColourList;

   doAverage = inplop.doAverage;
   ChanMap.resize(inplop.ChanMap.shape());
   RowMap.resize(inplop.RowMap.shape());
   ChanMap = inplop.ChanMap;
   RowMap = inplop.RowMap;
   MSName = inplop.MSName;
   spwExpr = inplop.spwExpr;

   return *this;
}


/* Reset params to defaults */
Bool PlotOptions::reset()
{
   PanelMap.resize(3,True); 
   for(Int i=0;i<3;i++) 
      PanelMap[i]=1;
   PlotRange.resize(4); 
      PlotRange.set(0);
   PlotRangesSet.resize(4); 
      PlotRangesSet.set(False);
   
   MultiColour=String("none");
   TableMultiColour=True;
   ColumnsXaxis=True;
   OverPlot=False;
   ReplaceTopPlot=False;
   RemoveOldPanels=True;
   FontSize = 11.0;
   
   XLabel = "";
   YLabel = "";
   Title = "";

   DoScalingCorrection = False;
   SeparateIter = String("none");
   HonourXFlags = False;
   
   PlotSymbol = String("");
   ColourString = String("");
   PointLabels.resize(0);
   LocateColumns.resize(0);
   MarkerSize = 10.0;
   LineWidth = 2.0;
   ShowFlags = False;
   FlagExt = "";
   FlagVersion = String("main");
   SkipNRows = 1;
   AverageNRows = 1;
   Connect = String("none");

   WindowSize = 8.0;
   AspectRatio = 1.0;

   TimePlot_p=0;
   PlotColour_p = -1;
   PyPlotSymbol_p = ',';

   Convert = NULL;
   ConvertIsLocal_p = False;
   CallBackHooks = NULL;

   TimePlotChar = String("o");
   
   /* Initialize the ColourList */
   /* Parse the list of colours, set the default colour,
      and add a custom colour holder that will be filled
      if the user specifies the colour via (rgb) or html hex */
   Char delim = ',';
   string res[200];
   NColours = split( pylabcolourstring, res, 200, delim ) + 1;
   useLayerColor = True;
   ColourList.resize(NColours);
   for( Int i=0;i<NColours-1;i++ ) {
      ColourList[i] = String("'") + String(res[i]) + String("'");
      if( ColourList[i].contains("default") ) 
          ColourList[i] = String("(1.0,0.4,0.2)");
   }
   ColourList[NColours-1] = String("(1.0,1.0,1.0)");
   
   doAverage = False;
   ChanMap.resize(0, 0);
   RowMap.resize(0, 0);
   MSName = "";
   spwExpr = "";

   return True;
}

/* Return String to print */
String PlotOptions::print()
{
   ostringstream oss;
      
   oss << "Panel : " << PanelMap << endl;
   oss << "PlotRange : " << PlotRange << endl;
   oss << "PlotRangesSet : " << PlotRangesSet << endl;

   oss << "MultiColour : " << MultiColour << endl;
   oss << "TableMultiColour : " << TableMultiColour << endl;
   oss << "ColumnXaxis : " << ColumnsXaxis << endl;
   oss << "OverPlot : " << OverPlot << endl;
   oss << "ReplaceTopPlot : " << ReplaceTopPlot << endl;
   oss << "RemoveOldPanels : " << RemoveOldPanels << endl;
   oss << "FontSize : " << FontSize << endl;
   
   oss << "XLabel : " << XLabel << endl;
   oss << "YLabel : " << YLabel << endl;
   oss << "Title : " << Title << endl;

   oss << "DoScalingCorrection : " << DoScalingCorrection << endl;
   oss << "SeparateIter : " << SeparateIter << endl;
   oss << "HonourXFlags : " << HonourXFlags << endl;
   
   oss << "PlotSymbol : " << PlotSymbol << endl;
   oss << "ColourString : " << ColourString << endl;
   oss << "PointLabels : " << PointLabels << endl;
   oss << "LocateColumns : " << LocateColumns << endl;
   oss << "MarkerSize : " << MarkerSize << endl;
   oss << "LineWidth : " << LineWidth << endl;
   oss << "ShowFlags : " << ShowFlags << endl;
   oss << "FlagExt : " << FlagExt << endl;
   oss << "FlagVersion : " << FlagVersion << endl;
   oss << "SkipNRows : " << SkipNRows << endl;
   oss << "AverageNRows : " << AverageNRows << endl;
   oss << "Connect : " << Connect << endl;

   oss << "WindowSize : " << WindowSize << endl;
   oss << "AspectRatio : " << AspectRatio << endl;
   oss << "doAverage : " << doAverage << endl;
#if LOG0
   //oss << "ChanMap : " << std::setprecision(8) << ChanMap << endl;
   //oss << "RowMap : " << std::setprecision(8) << RowMap << endl;
#endif 
   oss << "MSName : " << MSName << endl;
   oss << "spwExpr : " << spwExpr << endl;

   return String( oss.str() );
}

/* Check for parameter validity. */
Vector<String> PlotOptions::validateParams()
{
   Vector<String> ErrorString(2);
   
   ErrorString[0] = String("");
   ErrorString[1] = String("");
   
   if(PanelMap[0] < 1) 
      ErrorString[0] += String("\n") + String::toString(PanelMap[0]) 
                                     + String(" : 'nrows' must be > 0");
   if(PanelMap[1] < 1) 
      ErrorString[0] += String("\n") + String::toString(PanelMap[2]) 
                                     + String(" : 'ncols' must be > 0");
   if(PanelMap[2] < 1 || PanelMap[2]>PanelMap[0]*PanelMap[1]) 
      ErrorString[0] += String("\n") + String::toString(PanelMap[2]) + 
              String(" : 'panel' must be within [ 1 , ") + 
              String::toString(PanelMap[0]*PanelMap[1]) + String(" ]");

   
   if(FontSize < 0.0) ErrorString[0] += String("\n") +  
           String(" : 'fontsize' must be positive");
   if(FontSize < 5.0) ErrorString[1] += String("\n") +  
           String(" : 'fontsize' is too small. Try 12.0");
   if(FontSize > 30.0) ErrorString[1] += String("\n") +  
           String(" : 'fontsize' is too large. Try 12.0");
   
   if(MarkerSize < 0.0) ErrorString[0] += String("\n") + 
           String(" : 'markersize' must be positive");
   if(MarkerSize < 1.0) ErrorString[1] += String("\n") + 
           String(" : 'markersize' is too small. Try 10.0");
   if(MarkerSize > 20.0) ErrorString[1] += String("\n") + 
           String(" : 'markersize' is too large. Try 10.0");

   if(LineWidth < 0.0) 
      ErrorString[0] += String("\n") + 
           String(" : 'linewidth' must be positive");
   if(LineWidth < 1.0) 
      ErrorString[1] += String("\n") + 
           String(" : 'linewidth' is too small. Try 2.0");
   if(LineWidth > 20.0) 
      ErrorString[1] += String("\n") + 
           String(" : 'linewidth' is too large. Try 2.0");
   
   if(WindowSize < 1.0) 
      ErrorString[0] += String("\n") + 
         String(" : 'WindowSize' is too small. Try 8.0 ");
   if(WindowSize > 30.0) 
      ErrorString[1] += String("\n") + 
         String(" : 'WindowSize' is too large. Try 8.0 ");
         
   if(AspectRatio < 0.0) 
      ErrorString[0] += String("\n") + 
         String(" : 'AspectRatio' must be a positive number. ");
   if(AspectRatio > 30.0) 
      ErrorString[1] += String("\n") + 
         String(" : 'AspectRatio' is very large. Try 1.0 ");
         
   if(SkipNRows < 1) 
      ErrorString[0] += String("\n") + String::toString(SkipNRows) 
                      + String(" : 'skipnrows' must be >= 1 ");
   if(AverageNRows < 1) 
      ErrorString[0] += String("\n") + String::toString(AverageNRows) 
                           + String(" : 'averagenrows' must be >= 1 ");
   
   /* Get the first char as colour, if it is specified */
   //cout << "plotsymbol : " << PlotSymbol << endl;
   String symstr;
   Int len = PlotSymbol.length();
   if(len>0)
   {
   switch(PlotSymbol[0])
   {
      case 'k': PlotColour_p=0;len--;symstr=PlotSymbol.substr(1,len);break;
      case 'r': PlotColour_p=5;len--;symstr=PlotSymbol.substr(1,len);break;
      case 'g': PlotColour_p=3;len--;symstr=PlotSymbol.substr(1,len);break;
      case 'b': PlotColour_p=2;len--;symstr=PlotSymbol.substr(1,len);break;
      case 'c': PlotColour_p=4;len--;symstr=PlotSymbol.substr(1,len);break;
      case 'y': PlotColour_p=6;len--;symstr=PlotSymbol.substr(1,len);break;
      //case 'm': PlotColour_p=-1;len--;symstr=PlotSymbol.substr(1,len);break;
      //case 'w': PlotColour_p=WHITE;len--;symstr=PlotSymbol.substr(1,len);break;
      default: PlotColour_p=-1;symstr=PlotSymbol;
   }

   if(len==0) 
       PyPlotSymbol_p = ',';
   else {
      /* If the rest of the string contains more colour values, complain */
      if(symstr.contains('r') || symstr.contains('g') || symstr.contains('b')
         || symstr.contains('c') || symstr.contains('y')
         || symstr.contains('m') || symstr.contains('k')) {
         ErrorString[0] += String("\n") + PyPlotSymbol_p +  
                 String(" : plotsymbol must contain only one colour");
      }
      else {
         /* Check for validity of remaining symbol string */
         Bool valid=True;
         String mplib("-.:.,o^v<>s+xDd1234hH|_");
         for(Int s=0;s<len;s++)
         {
            if(!mplib.contains(symstr[s]))
            {
               valid=False;
               break;
            }
            if (symstr[s]=='-')
               symstr[s] = '_';
            if (symstr[s]=='1')
               symstr[s] = '|';
         }
         if(!valid)
            ErrorString[0] += String("\n") + PlotSymbol + 
               String(" : has invalid matplotlib symbols") + symstr;
         else PyPlotSymbol_p = symstr;
      }
   }
   }

   
   /* Check the ColourString */
   /* If valid, this over-rides the plotcolour specified through plotsymbol */
   if( ColourString.length() > 0 )
   {
      /* Check if the string is a valid (rgb) tuple */
      if( (ColourString.chars())[0] == '(' &&
          (ColourString.chars())[ColourString.length()-1] == ')' &&
          ColourString.freq(",")==2 )
      {
         Bool ok = True;
         Char delim = ',';
         string res[50];
         Double col=0.0;
         String subs = ColourString.after("(");
         subs = subs.before(")");
         Int nvals = split( subs, res, 50, delim );
         //cout << subs << endl;
         for( Int i=0;i<nvals;i++ ) 
         {
            col = atof((res[i]).data());
            if(col<0.0 || col>1.0) ok = False;
            //cout << col << endl;
         }

         if(ok)
         {
            ColourList[NColours-1] = ColourString;
            PlotColour_p = NColours-1;
         }
         else
         {
            ErrorString[1] += String("\n ") + ColourString + 
            String(" must be a valid (r,g,b) tuple with r,g,b between 0 and 1");
         }
      }
      /* Check if the string is a valid Html hex string */
      else if( (ColourString.chars())[0] == '#' && ColourString.length()==7 )
      {
         Bool ok = True;
         String hex("0123456789AaBbCcDdEeFf");
         for( Int i=1; i<(Int)ColourString.length(); i++ )
         {
            if( ! hex.contains((ColourString.chars())[i]) )
               ok = False;   
         }
         if(ok)
         {
            ColourList[NColours-1] = String("'") + ColourString + String("'");
            PlotColour_p = NColours-1;
         }
         else
         {
            ErrorString[1] += String("\n ") + ColourString + 
               String(" must be a valid html hex colour string");
         }

      }
      else /* Check if it's a valid pre-defined colour */
      {
         if( !pylabcolourstring.contains(ColourString) ) 
            ErrorString[1] += String("\n Color : ") + ColourString + 
              String(" : is not one of the pre-defined colors.");
         else // Find and assign the corresponding PlotColour
         {
            for( Int i=0; i<NColours; i++) 
               if( ColourList[i].contains(ColourString) )
               {
                  PlotColour_p = i;
                  break;
               }
         }
      }
   }
   
   /* Check the TimePlot characters */
   if(TimePlotChar.length() != 1) ErrorString[0] += String(
       "\n TimePlotChar must be a single character. One of 'o','x','y','b'");
   //   TimePlotChar = String("o");
   else
   switch(TimePlotChar[0])
   {
      case 'o': TimePlot_p = 0; break;
      case 'x': TimePlot_p = 1; break;
      case 'y': TimePlot_p = 2; break;
      case 'b': TimePlot_p = 3; break;
      default : TimePlot_p = 0; ErrorString[0] += 
        String("\n Invalid timeplot option. Need one of 'o','x','y','b'");
   }

   /* Add a check for the Convert pointer.
      Do a dynamic cast to see if it's a derived version of
      TPConvertBase */

      if(Convert == NULL) 
      {
         Convert = new TPConvertBase();
         ConvertIsLocal_p = True;
      }
      else ConvertIsLocal_p = False;

   /* Check the shape of PlotRange and PlotRangesSet*/
    if( PlotRange.nelements() != 4 )
       ErrorString[0] += String(
           "\n plotrange must have [xmin,xmax,ymin,ymax]");
    if( PlotRangesSet.nelements() != 4 )
       ErrorString[0] += String(
          "\n plotrangesset must have [xminset,xmaxset,yminset,ymaxset]");

   /* Check the Connect param */
    if( !Connect.matches(String("none")) && 
        !Connect.matches(String("tablerow")) &&
        !Connect.matches(String("cellrow")) &&
        !Connect.matches(String("cellcol")) )
    {
       ErrorString[1] += String("\n plotfirst : ") + Connect + 
           String(" : Must be one of 'none','tablerow','cellrow'") + 
           String(",or 'cellcol'. Using default of 'none'.");
       Connect = String("none");
    }
    /* If points are to be connected, update the PyPlotSymbol_p to do so */
    //cout << "Connect=" << Connect << endl;
    if( Connect.matches("tablerow") || Connect.matches("cellrow") || 
        Connect.matches("cellcol") )
      if( !PyPlotSymbol_p.contains('-') ) PyPlotSymbol_p += String("-");
    
   /* Check the MultiColour param */
    if( !MultiColour.matches(String("none")) && 
        !MultiColour.matches(String("both")) &&
        !MultiColour.matches(String("cellrow")) &&
        !MultiColour.matches(String("cellcol")) )
    {
       ErrorString[1] += String("\n multicolour : ") + MultiColour + 
         String(" : Must be either boolean, or one of 'none',") +
         String("'both','cellrow',or 'cellcol'. Using default of 'none'.");
       MultiColour = String("none");
    }

    if( !SeparateIter.matches(String("none")) &&
        !SeparateIter.matches(String("col")) &&
        !SeparateIter.matches(String("row")) )
    {
       ErrorString[1] += String("\n separateiterplot : ") + 
               SeparateIter + 
       String(" : Must be one of 'none','col','row'. Using default of 'none'.");
       SeparateIter = String("none");
    }

     /* If a CallBackHooks class is defined, use the Vector<String> 
            from there. */
    if(CallBackHooks != NULL) 
    {
        LocateColumns.resize(CallBackHooks->getLocateColumns().nelements());
        LocateColumns = CallBackHooks->getLocateColumns();
    }
         
   return ErrorString;
}

/* Check input data types, when a record is used */
String PlotOptions::fillFromRecord(Record &rpop)
{

   /* First reset all params to defaults */
   reset();

   DataType type;
   String ErrorString("");
   
   /* Parse the input record .*/
   if(rpop.isDefined("nrows"))
   {
      RecordFieldId ridny("nrows");
      if(rpop.dataType(ridny) != TpInt) 
         ErrorString += String("\nnrows must be an 'int'");
      else 
         rpop.get(ridny,PanelMap[0]);
   }

   if(rpop.isDefined("ncols"))
   {
      RecordFieldId ridnx("ncols");
      if(rpop.dataType(ridnx) != TpInt) 
         ErrorString += String("\nncols must be an 'int'");
      else
         rpop.get(ridnx,PanelMap[1]);
   }

   if(rpop.isDefined("panel"))
   {
      RecordFieldId ridny("panel");
      if(rpop.dataType(ridny) != TpInt) 
         ErrorString += String("\npanel must be an 'int'");
      else
         rpop.get(ridny,PanelMap[2]);
   }
   
   if(rpop.isDefined("windowsize"))
   {
      RecordFieldId ridwin("windowsize");
      if(rpop.dataType(ridwin) != TpDouble) 
         ErrorString += String("\nwindowsize must be a 'double'");
      else
         rpop.get(ridwin,WindowSize);
   } 

   if(rpop.isDefined("aspectratio"))
   {
      RecordFieldId ridasp("aspectratio");
      if(rpop.dataType(ridasp) != TpDouble) 
         ErrorString += String("\naspectratio must be a 'double'");
      else
         rpop.get(ridasp,AspectRatio);
   }

   if(rpop.isDefined("fontsize"))
   {
      RecordFieldId ridfont("fontsize");
      if(rpop.dataType(ridfont) == TpDouble)
      {
         rpop.get(ridfont,FontSize);
      }
      else 
      {
         if(rpop.dataType(ridfont) == TpInt)
         {
            Int val;
            rpop.get(ridfont,val);
            FontSize = val * 1.0;
         }
         else 
            ErrorString += String("\nfontsize must be an 'int' or 'double'");
      }
   }
   
   if(rpop.isDefined("linewidth"))
   {
      RecordFieldId ridfont("linewidth");
      if(rpop.dataType(ridfont) != TpDouble) 
         ErrorString += String("\nlinewidth must be an 'double'");
      else
         rpop.get(ridfont,LineWidth);
   }

   if(rpop.isDefined("markersize"))
   {
      RecordFieldId ridfont("markersize");
      if(rpop.dataType(ridfont) != TpDouble) 
         ErrorString += String("\nmarkersize must be a 'Float'");
      else
         rpop.get(ridfont,MarkerSize);
   }

   if(rpop.isDefined("timeplot"))
   {
      RecordFieldId ridfont("timeplot");
      if(rpop.dataType(ridfont) != TpString) 
         ErrorString += String("\ntimeplot must be a single character");
      else
      {
         rpop.get(ridfont,TimePlotChar);
      }
   }

   if(rpop.isDefined("overplot"))
   {
      RecordFieldId ridfont("overplot");
      if(rpop.dataType(ridfont) != TpBool) 
         ErrorString += String("\noverplot must be a 'bool'");
      else
         rpop.get(ridfont,OverPlot);
   }
   
   if(rpop.isDefined("replacetopplot"))
   {
      RecordFieldId ridfont("replacetopplot");
      if(rpop.dataType(ridfont) != TpBool) 
         ErrorString += String("\nreplot must be a 'bool'");
      else
         rpop.get(ridfont,ReplaceTopPlot);
   }
   
   if(rpop.isDefined("removeoldpanels"))
   {
      RecordFieldId ridfont("removeoldpanels");
      if(rpop.dataType(ridfont) != TpBool) 
         ErrorString += String("\nremoveoldpanels must be a 'bool'");
      else
         rpop.get(ridfont,RemoveOldPanels);
   }
   
   if(rpop.isDefined("xlabel"))
   {
      RecordFieldId ridlab("xlabel");
      if(rpop.dataType(ridlab) != TpString) 
         ErrorString += String("\nxlabel must be a 'string'");
      else
         rpop.get(ridlab,XLabel);
   }
   
   if(rpop.isDefined("ylabel"))
   {
      RecordFieldId ridlab("ylabel");
      if(rpop.dataType(ridlab) != TpString) 
         ErrorString += String("\nylabel must be a 'string'");
      else
         rpop.get(ridlab,YLabel);
   }
   
   
   if(rpop.isDefined("title"))
   {
      RecordFieldId ridlab("title");
      if(rpop.dataType(ridlab) != TpString) 
         ErrorString += String("\ntitle must be a 'string'");
      else
         rpop.get(ridlab,Title);
   }
   
   if(rpop.isDefined("doscalingcorrection"))
   {
      RecordFieldId ridfont("doscalingcorrection");
      if(rpop.dataType(ridfont) != TpBool) 
         ErrorString += String("\ndoscalingcorrection must be a 'bool'");
      else
         rpop.get(ridfont,DoScalingCorrection);
   }
   
   if(rpop.isDefined("separateiterplot"))
   {
      RecordFieldId ridfont("separateiterplot");
      if(rpop.dataType(ridfont) != TpString) 
         ErrorString += String("\nseparateiterplot must be a 'string'");
      else
         rpop.get(ridfont,SeparateIter);
   }
        
   if(rpop.isDefined("honourxflags"))
   {
      RecordFieldId ridfont("honourxflags");
      if(rpop.dataType(ridfont) != TpBool) 
         ErrorString += String("\nhonourxflags must be a 'bool'");
      else
         rpop.get(ridfont,HonourXFlags);
   }
        
   if(rpop.isDefined("plotsymbol"))
   {
      RecordFieldId ridfont("plotsymbol");
      if(rpop.dataType(ridfont) != TpString) 
         ErrorString += String("\nplotsymbol must be a 'string'");
      else
         rpop.get(ridfont,PlotSymbol);
   }

   if(rpop.isDefined("color"))
   {
      RecordFieldId ridfont("color");
      if(rpop.dataType(ridfont) != TpString) 
         ErrorString += String("\ncolor must be a 'string'");
      else
         rpop.get(ridfont,ColourString);
   }

   if(rpop.isDefined("connect"))
   {
      RecordFieldId ridfont("connect");
      if(rpop.dataType(ridfont) != TpString) 
         ErrorString += String("\nconnect must be a 'string'");
      else
         rpop.get(ridfont,Connect);
   }

   if(rpop.isDefined("plotrange"))
   {
      RecordFieldId ridfont("plotrange");
      type = rpop.dataType(ridfont);
      if(type != TpArrayDouble) 
         ErrorString += String("\nplotrange must be a list of 4 'doubles'");
      else
      {
         Vector<Double> prange(4);
         rpop.get(ridfont,prange);
         if(prange.shape() != IPosition(1,4)) 
            ErrorString += String("\nplotrange must be a list of 4 'doubles'");
         else 
         {
            PlotRange = prange;
         }
      }
   }

   if(rpop.isDefined("plotrangesset"))
   {
      RecordFieldId ridprs("plotrangesset");
      type = rpop.dataType(ridprs);
      if(type != TpArrayBool) 
         ErrorString += 
            String("\nplotrangesset must be a list of 4 'true/false' values");
      else
      {
         Vector<Bool> prangesSet(4);
         rpop.get(ridprs,prangesSet);
         if(prangesSet.shape() != IPosition(1,4)) 
            ErrorString += 
             String("\nplotrangesSet must be a list of 4 'true/false' values");
         else 
         {
            PlotRangesSet = prangesSet;
         }
      }
   }

   if(rpop.isDefined("showflags"))
   {
      RecordFieldId ridfont("showflags");
      if(rpop.dataType(ridfont) != TpBool) 
         ErrorString += String("\nshowflags must be a 'boolean'");
      else
         rpop.get(ridfont,ShowFlags);
   }
   
   if(rpop.isDefined("useflagversion"))
   {
      RecordFieldId ridfont("useflagversion");
      if(rpop.dataType(ridfont) != TpString) 
         ErrorString += String("\nuseflagversion must be a 'string'");
      else
         rpop.get(ridfont,FlagVersion);
   }
   
   if(rpop.isDefined("columnsxaxis"))
   {
      RecordFieldId ridfont("columnsxaxis");
      if(rpop.dataType(ridfont) != TpBool) 
         ErrorString += String("\ncolumnsxaxis must be a 'bool'");
      else
         rpop.get(ridfont,ColumnsXaxis);
   }

   if(rpop.isDefined("pointlabels"))
   {
      RecordFieldId ridfont("pointlabels");
      type = rpop.dataType(ridfont);
      if(type != TpArrayString) 
         ErrorString += String("\npointlabels must be a list of 'strings'");
      else
      {
         rpop.get(ridfont,PointLabels);
      }
   }

   if(rpop.isDefined("locatecolumns"))
   {
      RecordFieldId ridfont("locatecolumns");
      type = rpop.dataType(ridfont);
      if(type != TpArrayString) 
         ErrorString += String("\nlocatecolumns must be a list of 'strings'");
      else
      {
         rpop.get(ridfont,LocateColumns);
      }
   }

   if(rpop.isDefined("multicolour"))
   {
      RecordFieldId ridfont("multicolour");
      if(rpop.dataType(ridfont) != TpBool)
      {
         if(rpop.dataType(ridfont) != TpString)
           ErrorString += String("\nmulticolour must be a 'bool' or 'string'");
         else
           rpop.get(ridfont,MultiColour);
      }
      else
      {
         Bool tmp;
         rpop.get(ridfont,tmp);
         if(tmp) MultiColour = String("both");
         else MultiColour = String("none");
      }
   }

   if(rpop.isDefined("tablemulticolour"))
   {
      RecordFieldId ridfont("tablemulticolour");
      if(rpop.dataType(ridfont) != TpBool) 
         ErrorString += String("\ntablemulticolour must be a 'bool'");
      else
         rpop.get(ridfont,TableMultiColour);
   }

   if(rpop.isDefined("skipnrows"))
   {
      RecordFieldId ridfont("skipnrows");
      if(rpop.dataType(ridfont) != TpInt) 
         ErrorString += String("\nskipnrows must be an 'Int'");
      else
         rpop.get(ridfont,SkipNRows);
   }

   if(rpop.isDefined("extendflag"))
   {
      RecordFieldId ridfont("extendflag");
      if(rpop.dataType(ridfont) != TpString) 
         ErrorString += String("\nextendflag must be an 'String'");
      else
         rpop.get(ridfont,FlagExt);
   }

   if(rpop.isDefined("averagenrows"))
   {
      RecordFieldId ridfont("averagenrows");
      if(rpop.dataType(ridfont) != TpInt) 
         ErrorString += String("\naveragenrows must be an 'Int'");
      else
         rpop.get(ridfont,AverageNRows);
   }

   if(rpop.isDefined("doaverage"))
   {
      RecordFieldId ridfont("doaverage");
      type = rpop.dataType(ridfont);
      if(type != TpBool) 
         ErrorString += String("\ndoaverage must be a 'bool'");
      else 
         rpop.get(ridfont, doAverage);
   }

   if(rpop.isDefined("chanmap"))
   {
      RecordFieldId ridfont("chanmap");
      type = rpop.dataType(ridfont);
      if(type != TpArrayInt) 
         ErrorString += String("\nchanmap must be a nx3 matrix of 'int'");
      else
      {
         Matrix<Int> cmap(0, 0);
         rpop.get(ridfont, cmap);
         ChanMap = cmap;
      }
   }

   if(rpop.isDefined("rowmap"))
   {
      RecordFieldId ridfont("rowmap");
      type = rpop.dataType(ridfont);
      if(type != TpArrayInt) 
         ErrorString += String("\nrowmap must be a nx2 matrix of 'int'");
      else
      {
         Matrix<Int> rmap(0, 0);
         rpop.get(ridfont, rmap);
         ChanMap = rmap;
      }
   }

   //if(rpop.isDefined("msname"))
   //{
   //   RecordFieldId ridfont("msname");
   //   if(rpop.dataType(ridfont) != TpString) 
   //      ErrorString += String("\nmsname must be a 'string'");
   //   else
   //      rpop.get(ridfont, MSName);
   //}

   //if(rpop.isDefined("spwexpr"))
   //{
   //   RecordFieldId ridfont("spwexpr");
   //   if(rpop.dataType(ridfont) != TpString) 
   //      ErrorString += String("\nspwexpr must be a 'string'");
   //   else
   //      rpop.get(ridfont, spwExpr);
   //}


   return ErrorString;
}


/****************************************************************/
   
PanelParams::PanelParams():Plop()
{
   reset();
}

PanelParams::~PanelParams()
{}

Bool PanelParams::reset()
{
   Plop.reset();

   PanelZrange.resize(4,True);
   for(Int i=0;i<4;i++) PanelZrange[i]=0;
   
   FlagList.resize(0);
   MaxLayer = 1;
   TimePlot = 0;
   PlotColour = -1;
   PyPlotSymbol = ',';
   
   nBP = 0;
   LayerNumbers.resize(nBP);
   LayerColours.resize(nBP);
   //LayerColourStrings.resize(nBP);
   LayerSymbols.resize(nBP);
   LayerPointLabels.resize(nBP);
   LayerLocateColumns.resize(nBP);
   LayerXYTaqls.resize(nBP);
   LayerMarkerSizes.resize(nBP);
   LayerLineWidths.resize(nBP);
   LayerShowFlags.resize(nBP);
   LayerFlagExt.resize(nBP);
   LayerFlagVersions.resize(nBP);
   LayerSkipNRows.resize(nBP);
   LayerAverageNRows.resize(nBP);
   LayerConnects.resize(nBP);
   LayerMultiColours.resize(nBP);


   return True;
}

Int PanelParams::changeNlayers(Int nlayers)
{
   nBP = nlayers;
   LayerNumbers.resize(nBP,True);
   LayerColours.resize(nBP,True);
   //LayerColourStrings.resize(nBP,True);
   LayerSymbols.resize(nBP,True);
   LayerPointLabels.resize(nBP,True);
   LayerLocateColumns.resize(nBP,True);
   LayerXYTaqls.resize(nBP,True);
   LayerMarkerSizes.resize(nBP,True);
   LayerLineWidths.resize(nBP,True);
   LayerShowFlags.resize(nBP,True);
   LayerFlagExt.resize(nBP,True);
   LayerFlagVersions.resize(nBP,True);
   LayerSkipNRows.resize(nBP,True);
   LayerAverageNRows.resize(nBP,True);
   LayerConnects.resize(nBP,True);
   LayerMultiColours.resize(nBP,True);
   
   return 0;
}


/* LayerNumbers and LayerXYTaqls are set by TablePlot and not here */
/* All other layer params are filled in. */
Int PanelParams::updateLayerParams()
{
   /* Read in some private variables from Plop */
   Plop.getParsedParams(TimePlot, PlotColour, PyPlotSymbol);
   
   /* Find the top-most layer */
   /* The number of plot layers can be different from PBP.nelements() 
      - eg. a single layer can be made from multiple tables... */
   /* Use LayerNumbers as the template, because this is the one that
      gets set in TP. All other layer params must follow this shape */
   
   Int maxlayer=1;
   for(Int i=0;i<nBP;i++)
   {
      if(LayerNumbers[i] > maxlayer) maxlayer = LayerNumbers[i];
   }

   MaxLayer = maxlayer;
   
   /* Set up plot colour */
   Int pcol = 2;
   Int prev=0;
   /* Find the index of colour of the last plot of the previous layer */
   for(prev=nBP-1;prev>=0;prev--)
      if(LayerNumbers[prev] == maxlayer-1) break;

   if(PlotColour < 0) /* decide colours automatically */
   {
      if(maxlayer==1) PlotColour=1; /* The first layer the "default" */
      else /* increment from previous layer colour */
      {
         PlotColour = LayerColours[prev] + 1 ;
         PlotColour = PlotColour%Plop.NColours;
      }
   }

   pcol = PlotColour%Plop.NColours;

   //if(nBP>1 && pcol == LayerColours[prev] && Plop.OverPlot==True)
   //   cout << "WARNING : Plotting new layer in the same colour as the previous plot" << endl;
   
   Int ccc=0;
   /* Each PBP entry has a separate Layer entry */
   for(Int i=0;i<nBP;i++)
   {
      /* There can be multiple plots per "layer"... */
      /* LayerNumbers is set in setPlotOptions for new layers */
      if(LayerNumbers[i]==maxlayer)
      {
         /* layer DEPENDANT parameters */
         if(Plop.TableMultiColour) 
             LayerColours[i] = (pcol+ccc) % Plop.NColours;
         else LayerColours[i] = (pcol) % Plop.NColours;
         LayerSymbols[i] = PyPlotSymbol;
         LayerMarkerSizes[i] = Plop.MarkerSize;
         LayerLineWidths[i] = Plop.LineWidth;
         LayerShowFlags[i] = Plop.ShowFlags;
         LayerFlagExt[i] = Plop.FlagExt;
         LayerFlagVersions[i] = Plop.FlagVersion;
         LayerSkipNRows[i] = Plop.SkipNRows;
         LayerAverageNRows[i] = Plop.AverageNRows;
         LayerConnects[i] = Plop.Connect;
         LayerMultiColours[i] = Plop.MultiColour;
         
         LayerPointLabels[i].resize(Plop.PointLabels.nelements() );
         LayerPointLabels[i] = Plop.PointLabels;
         LayerLocateColumns[i].resize(Plop.LocateColumns.nelements() );
         LayerLocateColumns[i] = Plop.LocateColumns;
         
         ccc++;
      }
   }

   return 0;
}
   
} //# NAMESPACE CASA - END 


