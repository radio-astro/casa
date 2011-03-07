#include <iostream>
#include <xmlcasa/xerces/asdmCasaSaxHandler.h>
#include <casa/BasicSL/String.h>
#include <casa/Arrays/Vector.h>
#include <xercesc/sax2/Attributes.hpp>


//
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Utilities/DataType.h>

#include <tables/Tables/Table.h>
#include <tables/Tables/TableProxy.h>
#include <tables/Tables/TableColumn.h>
#include <tables/Tables/TableParse.h>
#include <tables/Tables/TableLock.h>
#include <casa/Containers/ValueHolder.h>
#include <casa/Exceptions/Error.h>
#include <casa/Logging/LogIO.h>
#include <xmlcasa/utils/stdBaseInterface.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableIter.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/StandardStMan.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <ctype.h>
#include <fstream>



namespace casa {

//The value for nStringmember is totaly arbitrary
//I don't know how large the vector<string> will be
//I will implement a better way to do this
//
//tagList will grow big, so for now I am puting all the values in a file
//taglist.txt, while I find a better way


asdmCasaSaxHandler::asdmCasaSaxHandler(Table &aTab, const String &tabName):
	flagIn    (false),
	isRow     (false),
	itsEntityRef(false),
        nRow      (0),
	nElements (5),
	tName     (tabName),
	itsTable  (aTab),
	nMaxBuffer(100)
{ }
//
//
//

//setDescriptor will creates the row nodes for each
//ASDM table. The assumtion here is that itsRecord,
//coming from table_cmpt.cc fromASDM function, had
//defined the Record as variable.


int asdmCasaSaxHandler::countIt(char *str)
{
        char ch;
        int nWords = 0;
        bool wordStarted = false;

        do {
           ch = *str++;
           if (!isspace(ch)){
               wordStarted = true;
           }else if (wordStarted) {
               wordStarted = false;
               ++nWords;
           }
        } while(ch);

        return nWords;
}

Array<String> asdmCasaSaxHandler::sSplit(char *str)
{
        char ch;
        char buffer[nMaxBuffer];
        int nWords;
        int k      = 0;
        int i      = 0;
        bool wordStarted = false;


	nWords=countIt(str);
        Vector<String> wTag(nWords);
        

        do {
           ch = *str++;
           if (!isspace(ch)){
               wordStarted = true;
               buffer[k]=ch;
               ++k;
           }else if (wordStarted) {
               wordStarted = false;
               wTag[i]=String(buffer,k);
               k=0;
	       ++i;
           }
        } while(ch);

        return wTag;
}

Array<Double> asdmCasaSaxHandler::sSplit_d(char *str)
{
        char ch;
        //nMaxBuffer is maximum size for a word, so far I am using 100
        //characters
        char buffer[nMaxBuffer];
        int  nWords;
        int  k      = 0;
        int  i      = 0;
        bool wordStarted = false;


        nWords=countIt(str);
        Vector<Double> wTag(nWords);


        do {
           ch = *str++;
           if ((!isspace(ch))){
               wordStarted = true;
               buffer[k]=ch;
               ++k;
           }else if (wordStarted) {
               wordStarted = false;
               wTag[i]=atof(String(buffer,k).c_str());
               k=0;
               ++i;
           }
        } while(ch);

        return wTag;
}


Array<Int> asdmCasaSaxHandler::sSplit_i(char *str)
{
        char ch;
        char buffer[nMaxBuffer];
        int nWords;
        int k      = 0;
        int i      = 0;
        bool wordStarted = false;


        nWords=countIt(str);
        Vector<Int> wTag(nWords);


        do {
           ch = *str++;
           if (!isspace(ch)){
               wordStarted = true;
               buffer[k]=ch;
               ++k;
           }else if (wordStarted) {
               wordStarted = false;
               wTag[i]=atoi(String(buffer,k).c_str());
               k=0;
               ++i;
           }
        } while(ch);

        return wTag;
}
void asdmCasaSaxHandler::assignColumn(char *str, Array<Bool> &boolColumn)
{

        Vector<String> temp=sSplit(str);
        const int size=atoi(temp(0).c_str());
        int shape[size];
        for (int i=0; i<size; ++i)
                shape[i]=atoi(temp[i+1].c_str());
        if (size==1){
	   //Arbitrary decision, I am initializing the array as false
           Vector<Bool> bColumn(shape[0],false);
           for (int k=size+1; k<(shape[0]+size+1); ++k){
	     if (temp[k]=="true")
                 bColumn(k-(size+1))=true;
	     else
                 bColumn(k-(size+1))=false;
           }
           boolColumn=bColumn;

        }
        if (size==2){
           Matrix<Bool> bColumn(shape[0],shape[1],false);
           for (int k=size+1; k<(shape[0]*shape[1]+size+1); ++k){
             if (temp[k]=="true")
                 bColumn(int((k-(size+1))/shape[1]),(k-(size+1))%shape[1])=true;
             else
                 bColumn(int((k-(size+1))/shape[1]),(k-(size+1))%shape[1])=false;
           }
           boolColumn=bColumn;
        }
}

void asdmCasaSaxHandler::assignColumn(char *str, Array<String> &stringColumn)
{

        Vector<String> temp=sSplit(str);
        const int size=atoi(temp(0).c_str());
        int shape[size];
        for (int i=0; i<size; ++i)
                shape[i]=atoi(temp[i+1].c_str());
	if (size==1){
           Vector<String> sColumn(shape[0]," ");
           for (int k=size+1; k<(shape[0]+size+1); ++k){
             sColumn(k-(size+1))=temp[k];
           }
           stringColumn=sColumn;

        }
        if (size==2){
           Matrix<String> sColumn(shape[0],shape[1]," ");
           for (int k=size+1; k<(shape[0]*shape[1]+size+1); ++k){
             sColumn(int((k-(size+1))/shape[1]),(k-(size+1))%shape[1])=
                     temp[k];
           }
           stringColumn=sColumn;
        }
        //If a ever need Cube<String> I will have to do explicit instantiation
        /*else if (size==3){
           cout << "shape[0] :" << shape[0] <<
                   " shape[1]: "<< shape[1] <<
                   " shape[2]: "<< shape[2] << endl;
           Cube<String> sColumn(shape[0],shape[1],shape[2],0);
           for (int k=size+1;
                    k<((shape[0]*shape[1]*shape[2])+size+1);
                    ++k){
             sColumn(int((k-(size+1))/shape[1]),
                        ((k-(size+1))%shape[1]),
                     int((k-(size+1))/(shape[0]*shape[1])))=temp[k];
             cout << "k: " << k << endl;
             cout << "i: " << int((k-(size+1))/shape[1]) <<
                    " j: " << (k-(size+1))%shape[1] <<
                    " l: " << int((k-(size+1))/(shape[0]*shape[1])) << endl;
             cout << "icolumn1: " <<
                     sColumn(int((k-(size+1))/shape[1]),
                                ((k-(size+1))%shape[1]),
                             int((k-(size+1))/(shape[0]*shape[1]))) << endl;
           }
           stringColumn=sColumn;
        }*/
}



void asdmCasaSaxHandler::assignColumn(char *str, Array<Int> &intColumn)
{

        Vector<Int> temp=sSplit_i(str);
        const int size=temp(0);
        int shape[size];
        for (int i=0; i<size; ++i)
                shape[i]=temp[i+1];
        if (size==1){
           Vector<Int> iColumn(shape[0],0);
           for (int k=size+1; k<(shape[0]+size+1); ++k){
             iColumn(k-(size+1))=temp[k];
           }
           intColumn=iColumn;

        }

        if (size==2){
           Matrix<Int> iColumn(shape[0],shape[1],0);
           for (int k=size+1; k<(shape[0]*shape[1]+size+1); ++k){
             iColumn(int((k-(size+1))/shape[1]),(k-(size+1))%shape[1])=
                     temp[k];
           }
	   intColumn=iColumn;
        }
        else if (size==3){
           Cube<Int> iColumn(shape[0],shape[1],shape[2],0);
           for (int k=size+1; 
                    k<((shape[0]*shape[1]*shape[2])+size+1); 
                    ++k){
             iColumn(int((k-(size+1))/shape[1]),
                        ((k-(size+1))%shape[1]),
                     int((k-(size+1))/(shape[0]*shape[1])))=temp[k];
           }
	   intColumn=iColumn;
        }
}


void  asdmCasaSaxHandler::assignColumn(char *str, Array<Double> &doubleColumn)
{
	
	Vector<Double> temp=sSplit_d(str);
        const int size=int(temp(0));
        int shape[size];
	for (int i=0; i<size; ++i)
		shape[i]=int(temp[i+1]);
        if (size==1){
           Vector<Double> dColumn(shape[0],0.0);
           for (int k=size+1; k<(shape[0]+size+1); ++k){
             dColumn(k-(size+1))=temp[k];
           }
           doubleColumn=dColumn;
        }

        if (size==2){
           Matrix<Double> dColumn(shape[0],shape[1],0.0);
	   for (int k=size+1; k<(shape[0]*shape[1]+size+1); ++k){
	     dColumn(int((k-(size+1))/shape[1]),(k-(size+1))%shape[1])=
                     temp[k];
           }
	   doubleColumn=dColumn;
        }
	else if (size==3){
           Cube<Double> dColumn(shape[0],shape[1],shape[2],0.0);
           for (int k=size+1;
                    k<((shape[0]*shape[1]*shape[2])+size+1);
                    ++k){
             dColumn(int((k-(size+1))/shape[1]),
                        ((k-(size+1))%shape[1]),
                     int((k-(size+1))/(shape[0]*shape[1])))=temp[k];


           }
	   doubleColumn=dColumn;
	}
}

void  asdmCasaSaxHandler::assignColumn(char *str, Array<Complex> &complexColumn)
{

        Vector<Double> temp=sSplit_d(str);
        const int size=int(temp(0));
        int shape[size],l=0;
        for (int i=0; i<size; ++i)
                shape[i]=int(temp[i+1]);
        if (size==1){
           Vector<Complex> cColumn(shape[0]);
           for (int k=size+1; k<(shape[0]+size+1); k+=2){
	     l=int((k-(size+1))/2);
             cColumn(l)=Complex(temp[k],temp[k+1]);
           }
           complexColumn=cColumn;

        }
        if (size==2){
           Matrix<Complex> cColumn(shape[0],shape[1]);
           for (int k=size+1; k<(int(shape[0]*shape[1])+size+1); k+=2){
	     l=int((k-(size+1))/2);
             cColumn(int(l/shape[1]),(l)%shape[1])=Complex(temp[k],temp[k+1]);
           }
           complexColumn=cColumn;
        }
        else if (size==3){
           Cube<Complex> cColumn(shape[0],shape[1],shape[2]);
           for (int k=size+1;
                    k<(int(shape[0]*shape[1]*shape[2])+size+1);
                    k+=2){
	     l=int((k-(size+1))/2);
             cColumn(int(l/shape[1]),
                        (l%shape[1]),
                     int(l/(shape[0]*shape[1])))=temp[k];


           }
           complexColumn=cColumn;
        }
}


void asdmCasaSaxHandler::setDescriptor(String tabName)
{
        //
        //The table descriptor is set below
        // 
	if (tabName=="ScanTable")
	{
           TableDesc td("ScanTable","1",TableDesc::Scratch);
	   td.addColumn(ScalarColumnDesc<String>("execBlockId",""));
           td.addColumn(ScalarColumnDesc<Int>("scanNumber",""));
           td.addColumn(ScalarColumnDesc<String>("startTime",""));
           td.addColumn(ScalarColumnDesc<String>("endTime",""));
           td.addColumn(ScalarColumnDesc<Int>("numSubScan",""));
           td.addColumn(ScalarColumnDesc<Int>("numIntent",""));
           td.addColumn(ArrayColumnDesc<String >("scanIntent","",1,2));
           td.addColumn(ScalarColumnDesc<Bool> ("flagRow",""));
           td.addColumn(ScalarColumnDesc<Int> ("numField",""));
           td.addColumn(ScalarColumnDesc<String> ("sourceName",""));
           td.addColumn(ArrayColumnDesc<String >("fieldName",""));
 
           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           execBlockId.attach(itsTable,"execBlockId");
           scanNumber.attach(itsTable,"scanNumber");
           startTime.attach(itsTable,"startTime");
           endTime.attach(itsTable,"endTime");
           numSubScan.attach(itsTable,"numSubScan");
           numIntent.attach(itsTable,"numIntent");
           scanIntent.attach(itsTable,"scanIntent");
           flagRow.attach(itsTable,"flagRow");
           numField.attach(itsTable,"numField");
           sourceName.attach(itsTable,"sourceName");
           fieldName.attach(itsTable,"fieldName");

        } else if (tabName=="MainTable") {

           TableDesc td("MainTable","1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String>("configDesciptionId",""));
           td.addColumn(ScalarColumnDesc<String>("fieldId",""));
           td.addColumn(ScalarColumnDesc<String>("time",""));
           td.addColumn(ScalarColumnDesc<String>("execBlockId",""));
           td.addColumn(ArrayColumnDesc<String>("stateId",""));
           td.addColumn(ScalarColumnDesc<Int>("scanNumber",""));
           td.addColumn(ScalarColumnDesc<Int>("subscanNumber",""));
           td.addColumn(ScalarColumnDesc<Int> ("integrationNumber",""));
           td.addColumn(ArrayColumnDesc<Double> ("uvw",""));
           td.addColumn(ArrayColumnDesc<String> ("exposure",""));
           td.addColumn(ArrayColumnDesc<String> ("timeCentroid",""));
           td.addColumn(ScalarColumnDesc<String> ("dataOid",""));
           td.addColumn(ArrayColumnDesc<Int>("flagAnt",""));
           td.addColumn(ArrayColumnDesc<Int>("flagPol",""));
           td.addColumn(ArrayColumnDesc<Int>("flagBaseband",""));
           td.addColumn(ScalarColumnDesc<Bool> ("flagRow",""));
           td.addColumn(ScalarColumnDesc<String> ("interval",""));
           td.addColumn(ScalarColumnDesc<Int> ("subintegrationNumber",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;


           configDescriptionId.attach(itsTable,"configDesciptionId");
           fieldId.attach(itsTable,"fieldId");
           time.attach(itsTable,"time");
           execBlockId.attach(itsTable,"execBlockId");
           stateId.attach(itsTable,"stateId");
           scanNumber.attach(itsTable,"scanNumber");
           subscanNumber.attach(itsTable,"subscanNumber");
           integrationNumber.attach(itsTable,"integrationNumber");
           uvw.attach(itsTable,"uvw");
           exposure.attach(itsTable,"exposure");
           timeCentroid.attach(itsTable,"timeCentroid");
           dataOid.attach(itsTable,"dataOid");
           flagAnt.attach(itsTable,"flagAnt");
           flagPol.attach(itsTable,"flagPol");
           flagBaseband.attach(itsTable,"flagBaseband");
           flagRow.attach(itsTable,"flagRow");
           interval.attach(itsTable,"interval");
           subintegrationNumber.attach(itsTable,"subintegrationNumber");

        } else if (tabName=="AlmaCorrelatorModeTable") {
           
           TableDesc td("AlmaCorrelatorModeTable","1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String>("AlmaCorrelatorModeId",""));
           td.addColumn(ScalarColumnDesc<Int>("numBaseband",""));
           td.addColumn(ArrayColumnDesc<Int>("basebandIndex",""));
           td.addColumn(ScalarColumnDesc<String>("accumMode",""));
           td.addColumn(ArrayColumnDesc<Int> ("basebandConfig",""));
           td.addColumn(ScalarColumnDesc<Int> ("binMode",""));
           td.addColumn(ScalarColumnDesc<Bool>("quantization",""));
           td.addColumn(ScalarColumnDesc<String>("windowFunction",""));
           td.addColumn(ArrayColumnDesc<Int>("axesOrderArray",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           almaCorrelatorModeId.attach(itsTable,"AlmaCorrelatorModeId");
           numBaseband.attach(itsTable,"numBaseband");
           basebandIndex.attach(itsTable,"basebandIndex");
           accumMode.attach(itsTable,"accumMode");
           basebandConfig.attach(itsTable,"basebandConfig");
           binMode.attach(itsTable,"binMode");
           quantization.attach(itsTable,"quantization");
           windowFunction.attach(itsTable,"windowFunction");
           axesOrderArray.attach(itsTable,"axesOrderArray");

        //
        // Antenna Table Descriptor
        //
        } else if (tabName=="AntennaTable") {

           TableDesc td("Antenna","1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String>("antennaId",""));
           td.addColumn(ScalarColumnDesc<String>("stationId",""));
           td.addColumn(ScalarColumnDesc<String>("name",""));
           td.addColumn(ScalarColumnDesc<String>("type",""));
           td.addColumn(ScalarColumnDesc<Double>("xPosition",""));
           td.addColumn(ScalarColumnDesc<Double>("yPosition",""));
           td.addColumn(ScalarColumnDesc<Double>("zPosition",""));
           td.addColumn(ScalarColumnDesc<String>("time",""));
           td.addColumn(ScalarColumnDesc<Double>("xOffset",""));
           td.addColumn(ScalarColumnDesc<Double>("yOffset",""));
           td.addColumn(ScalarColumnDesc<Double>("zOffset",""));
           td.addColumn(ScalarColumnDesc<Double>("dishDiameter",""));
           td.addColumn(ScalarColumnDesc<Bool>("flagRow",""));
           td.addColumn(ScalarColumnDesc<String>("assocAntennaId",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           antennaId.attach(itsTable,"antennaId");
           stationId.attach(itsTable,"stationId");
           name.attach(itsTable,"name");
           type.attach(itsTable,"type");
           xPosition.attach(itsTable,"xPosition");
           yPosition.attach(itsTable,"yPosition");
           zPosition.attach(itsTable,"zPosition");
           time.attach(itsTable,"time");
           xOffset.attach(itsTable,"xOffset");
           yOffset.attach(itsTable,"yOffset");
           zOffset.attach(itsTable,"zOffset");
           dishDiameter.attach(itsTable,"dishDiameter");
           flagRow.attach(itsTable,"flagRow");
           assocAntennaId.attach(itsTable,"assocAntennaId");

        } else if (tabName=="ConfigDescriptionTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String>("configDescriptionId",""));
           //
           //AntennaId, in this table, is described as vector<string>
           //So in this case, I'll change the datatype instead of keeping
           //string
           // 
           td.addColumn(ArrayColumnDesc<String> ("antennaId",""));
           td.addColumn(ArrayColumnDesc<String> ("dataDescriptionId",""));
           td.addColumn(ArrayColumnDesc<Int>    ("feedId",""));
           td.addColumn(ScalarColumnDesc<String>("processorId",""));
           td.addColumn(ArrayColumnDesc<String> ("switchCycleId",""));
           td.addColumn(ScalarColumnDesc<Int>   ("numAntenna",""));
           td.addColumn(ScalarColumnDesc<Int>   ("numFeed",""));
           td.addColumn(ArrayColumnDesc<Int>    ("numSubBand",""));
           td.addColumn(ScalarColumnDesc<Int>   ("correlationMode",""));
           td.addColumn(ScalarColumnDesc<Int>   ("atmPhaseCode",""));
           td.addColumn(ArrayColumnDesc<Int>    ("phasedArrayList",""));
	   //
           //Certain tags such as flagAnt, have conflicting datatypes.
           //at MainTable, it is declared as vector<int>, here is 
           //vector<bool>. Is this a bug in the ASDM definition
           //
           td.addColumn(ArrayColumnDesc<Bool>    ("flagAnt",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;
           
           configDescriptionId.attach(itsTable,"configDescriptionId");
           antennaIdAUX.attach(itsTable,"antennaId");
           dataDescriptionId.attach(itsTable,"dataDescriptionId");
           feedId.attach(itsTable,"feedId");
           processorId.attach(itsTable,"processorId");
           switchCycleId.attach(itsTable,"switchCycleId");
           numAntenna.attach(itsTable,"numAntenna");
           numFeed.attach(itsTable,"numFeed");
           numSubBand.attach(itsTable,"numSubBand");
           correlationMode.attach(itsTable,"correlationMode");
           atmPhaseCode.attach(itsTable,"atmPhaseCode");
           phasedArrayList.attach(itsTable,"phasedArrayList");
           flagAntAUX.attach(itsTable,"flagAnt");
        
        } 
          //
	  // Now DataDescription Table 
          else if (tabName=="DataDescriptionTable") {
           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String>("dataDescriptionId",""));
           td.addColumn(ScalarColumnDesc<String>("polOrHoloId",""));
           td.addColumn(ScalarColumnDesc<String>("spectralWindowId",""));
           td.addColumn(ScalarColumnDesc<Bool>  ("flagRow",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           dataDescriptionIdDDT.attach(itsTable,"dataDescriptionId");
           polOrHoloId.attach(itsTable,"polOrHoloId");
           spectralWindowId.attach(itsTable,"spectralWindowId");
           flagRow.attach(itsTable,"flagRow");
        }
	   //
	   //ExecBlock Table
	   //

          else if (tabName=="ExecBlockTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("execBlockId",""));
           td.addColumn(ArrayColumnDesc<String>  ("antennaId",""));
           td.addColumn(ScalarColumnDesc<String> ("telescopeName",""));
           td.addColumn(ScalarColumnDesc<String> ("configName",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numAntenna",""));
           td.addColumn(ScalarColumnDesc<Double> ("baseRangeMin",""));
           td.addColumn(ScalarColumnDesc<Double> ("baseRangeMax",""));
           td.addColumn(ScalarColumnDesc<Double> ("baseRangeMinor",""));
           td.addColumn(ScalarColumnDesc<Double> ("baseRangeMajor",""));
           td.addColumn(ScalarColumnDesc<Double> ("basePa",""));
           td.addColumn(ScalarColumnDesc<String> ("timeInterval",""));
           td.addColumn(ScalarColumnDesc<String> ("observerName",""));
           td.addColumn(ArrayColumnDesc<String>  ("observingLog",""));
           td.addColumn(ArrayColumnDesc<String>  ("schedulerMode",""));
           td.addColumn(ScalarColumnDesc<String> ("projectId",""));
           td.addColumn(ScalarColumnDesc<Double> ("siteLongitude",""));
           td.addColumn(ScalarColumnDesc<Double> ("siteLatitude",""));
           td.addColumn(ScalarColumnDesc<Double> ("siteAltitude",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("flagRow",""));
           td.addColumn(ScalarColumnDesc<String> ("execBlockUID",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("aborted",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           execBlockId.attach(itsTable,"execBlockId");
           antennaIdAUX.attach(itsTable,"antennaId");
           telescopeName.attach(itsTable,"telescopeName");
           configName.attach(itsTable,"configName");
           numAntenna.attach(itsTable,"numAntenna");
           baseRangeMin.attach(itsTable,"baseRangeMin");
           baseRangeMax.attach(itsTable,"baseRangeMax");
           baseRangeMinor.attach(itsTable,"baseRangeMinor");
           baseRangeMajor.attach(itsTable,"baseRangeMajor");
           basePa.attach(itsTable,"basePa");
           timeInterval.attach(itsTable,"timeInterval");
           observerName.attach(itsTable,"observerName");
           observingLog.attach(itsTable,"observingLog");
           schedulerMode.attach(itsTable,"schedulerMode");
           projectId.attach(itsTable,"projectId");
           siteLongitude.attach(itsTable,"siteLongitude");
           siteLatitude.attach(itsTable,"siteLatitude");
           siteAltitude.attach(itsTable,"siteAltitude");
           execBlockUID.attach(itsTable,"execBlockUID");
           flagRow.attach(itsTable,"flagRow");
           aborted.attach(itsTable,"aborted");
        }
           //
           //Feed Tables
           //

          else if (tabName=="FeedTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<Int>    ("feedId",""));
           td.addColumn(ArrayColumnDesc<String>  ("timeInterval",""));
           td.addColumn(ScalarColumnDesc<String> ("spectralWindowId",""));
           td.addColumn(ScalarColumnDesc<String> ("antennaId",""));
           td.addColumn(ArrayColumnDesc<Int>     ("receiverId",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numReceptors",""));
           td.addColumn(ArrayColumnDesc<Double>  ("beamOffset",""));
           td.addColumn(ArrayColumnDesc<Double>  ("focusReference",""));
           td.addColumn(ArrayColumnDesc<String>  ("polarizationType",""));
           td.addColumn(ArrayColumnDesc<Complex> ("polResponse",""));
           td.addColumn(ArrayColumnDesc<Double>  ("receptorAngle",""));
           td.addColumn(ArrayColumnDesc<String>  ("beamId",""));
           td.addColumn(ScalarColumnDesc<Int>    ("feedNum",""));
           td.addColumn(ScalarColumnDesc<Float>  ("illumOffset",""));
           td.addColumn(ScalarColumnDesc<Float>  ("illumOffsetPa",""));
           td.addColumn(ScalarColumnDesc<Double> ("xPosition",""));
           td.addColumn(ScalarColumnDesc<Double> ("yPosition",""));
           td.addColumn(ScalarColumnDesc<Double> ("zPosition",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           feedIdFT.attach(itsTable,"feedId");
           timeInterval.attach(itsTable,"timeInterval");
           spectralWindowId.attach(itsTable,"spectralWindowId");
           antennaId.attach(itsTable,"antennaId");
           receiverId.attach(itsTable,"receiverId");
           numReceptors.attach(itsTable,"numReceptors");
           beamOffset.attach(itsTable,"beamOffset");
           focusReference.attach(itsTable,"focusReference");
           polarizationType.attach(itsTable,"polarizationType");
           polResponse.attach(itsTable,"polResponse");
           receptorAngle.attach(itsTable,"receptorAngle");
           beamId.attach(itsTable,"beamId");
           feedNum.attach(itsTable,"feedNum");
           illumOffset.attach(itsTable,"illumOffset");
           illumOffsetPa.attach(itsTable,"illumOffsetPa");
           xPosition.attach(itsTable,"xPosition");
           yPosition.attach(itsTable,"yPosition");
           zPosition.attach(itsTable,"zPosition");
        }
	  //
	  //Field Table
	  //
          else if (tabName=="FieldTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("fieldId",""));
           td.addColumn(ScalarColumnDesc<String> ("fieldName",""));
           td.addColumn(ScalarColumnDesc<String> ("code",""));
           td.addColumn(ScalarColumnDesc<String> ("time",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numPoly",""));
           td.addColumn(ArrayColumnDesc<Double>  ("delayDir",""));
           td.addColumn(ArrayColumnDesc<Double>  ("phaseDir",""));
           td.addColumn(ArrayColumnDesc<Double>  ("referenceDir",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("flagRow",""));
           td.addColumn(ArrayColumnDesc<String>  ("assocFieldId",""));
           td.addColumn(ScalarColumnDesc<String> ("ephemerisId",""));
           td.addColumn(ScalarColumnDesc<Int>    ("sourceId",""));
           td.addColumn(ScalarColumnDesc<String> ("assocNature",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           fieldId.attach(itsTable,"fieldId");
           fieldNameAUX.attach(itsTable,"fieldName");
           code.attach(itsTable,"code");
           time.attach(itsTable,"time");
           numPoly.attach(itsTable,"numPoly");
           delayDir.attach(itsTable,"delayDir");
           phaseDir.attach(itsTable,"phaseDir");
           referenceDir.attach(itsTable,"referenceDir");
           flagRow.attach(itsTable,"flagRow");
           assocFieldId.attach(itsTable,"assocFieldId");
           ephemerisId.attach(itsTable,"ephemerisId");
           sourceId.attach(itsTable,"sourceId");
           assocNature.attach(itsTable,"assocNature");
         }
          //
	  //Polarization Table
	  //
          else if (tabName=="PolarizationTable") {
           
           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("polarizationId",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numCorr",""));
           td.addColumn(ArrayColumnDesc<Int>     ("corrType",""));
           td.addColumn(ArrayColumnDesc<Int>     ("corrProduct",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("flagRow",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           polarizationId.attach(itsTable,"polarizationId");
           numCorr.attach(itsTable,"numCorr");
           corrType.attach(itsTable,"corrType");
           corrProduct.attach(itsTable,"corrProduct");
           flagRow.attach(itsTable,"flagRow");
         }
	   //
	   //Processor Table
	   //

          else if (tabName=="ProcessorTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("processorId",""));
           td.addColumn(ScalarColumnDesc<String> ("almaCorrelatorModeId",""));
           td.addColumn(ScalarColumnDesc<String> ("type",""));
           td.addColumn(ScalarColumnDesc<String> ("subType",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("flagRow",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           processorId.attach(itsTable,"processorId");
           almaCorrelatorModeId.attach(itsTable,"almaCorrelatorModeId");
           type.attach(itsTable,"type");
           subType.attach(itsTable,"subType");
           flagRow.attach(itsTable,"flagRow");
         }
          //
          //Receiver Table
          //
          else if (tabName=="ReceiverTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<Int>    ("receiverId",""));
           td.addColumn(ScalarColumnDesc<String> ("spectralWindowId",""));
           td.addColumn(ScalarColumnDesc<String> ("timeInterval",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numLo",""));
           td.addColumn(ScalarColumnDesc<String> ("name",""));
           td.addColumn(ScalarColumnDesc<String> ("frequencyBand",""));
           td.addColumn(ArrayColumnDesc<Double>  ("freqLo",""));
           td.addColumn(ScalarColumnDesc<Double> ("stability",""));
           td.addColumn(ArrayColumnDesc<Int>     ("sidebandLo",""));
           td.addColumn(ScalarColumnDesc<Double> ("tDewar",""));
           td.addColumn(ScalarColumnDesc<String> ("stabilityDuration",""));
           td.addColumn(ScalarColumnDesc<String> ("dewarName",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("stabilityflag",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           receiverIdAUX.attach(itsTable,"receiverId");
           spectralWindowId.attach(itsTable,"spectralWindowId");
           timeInterval.attach(itsTable,"timeInterval");
           numLo.attach(itsTable,"numLo");
           name.attach(itsTable,"name");
           frequencyBand.attach(itsTable,"frequencyBand");
           freqLo.attach(itsTable,"freqLo");
           stability.attach(itsTable,"stability");
           sidebandLo.attach(itsTable,"sidebandLo");
           tDewar.attach(itsTable,"tDewar");
           stabilityDuration.attach(itsTable,"stabilityDuration");
           dewarName.attach(itsTable,"dewarName");
           stabilityflag.attach(itsTable,"stabilityflag");
         }
	//
	//SBSummary Table
	//
	
	 else if (tabName=="SBSummaryTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("sbId",""));
           td.addColumn(ScalarColumnDesc<String> ("projectId",""));
           td.addColumn(ScalarColumnDesc<String> ("obsUnitSetId",""));
           td.addColumn(ScalarColumnDesc<String> ("sbIntent",""));
           td.addColumn(ScalarColumnDesc<String> ("sbType",""));
           td.addColumn(ScalarColumnDesc<String> ("sbDuration",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numScan",""));
           td.addColumn(ArrayColumnDesc<String>  ("scanIntent",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numberRepeats",""));
           td.addColumn(ArrayColumnDesc<String>  ("weatherConstraint",""));
           td.addColumn(ArrayColumnDesc<String>  ("scienceGoal",""));
           td.addColumn(ScalarColumnDesc<Double> ("raCenter",""));
           td.addColumn(ScalarColumnDesc<Double> ("decCenter",""));
           td.addColumn(ScalarColumnDesc<Double> ("frequency",""));
           td.addColumn(ScalarColumnDesc<String> ("frequencyBand",""));
           td.addColumn(ArrayColumnDesc<String>  ("observingMode",""));
 
           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           sbId.attach(itsTable,"sbId");
           projectId.attach(itsTable,"projectId");
           obsUnitSetId.attach(itsTable,"obsUnitSetId");
           sbIntent.attach(itsTable,"sbIntent");
           sbType.attach(itsTable,"sbType");
           sbDuration.attach(itsTable,"sbDuration");
           numScan.attach(itsTable,"numScan");
           scanIntent.attach(itsTable,"scanIntent");
           numberRepeats.attach(itsTable,"numberRepeats");
           weatherConstraint.attach(itsTable,"weatherConstraint");
           scienceGoal.attach(itsTable,"scienceGoal");
           raCenter.attach(itsTable,"raCenter");
           decCenter.attach(itsTable,"decCenter");
           frequency.attach(itsTable,"frequency");
           frequencyBand.attach(itsTable,"frequencyBand");
           observingMode.attach(itsTable,"observingMode");
          }
        //
        //Source Table
        //

         else if (tabName=="SourceTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<Int>    ("sourceId",""));
           td.addColumn(ScalarColumnDesc<String> ("spectralWindowId",""));
           td.addColumn(ScalarColumnDesc<String> ("timeInterval",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numLines",""));
           td.addColumn(ScalarColumnDesc<String> ("sourceName",""));
           td.addColumn(ScalarColumnDesc<String> ("code",""));
           td.addColumn(ArrayColumnDesc<Double>  ("direction",""));
           td.addColumn(ArrayColumnDesc<Double>  ("properMotion",""));
           td.addColumn(ScalarColumnDesc<Int>    ("sourceParameterId",""));
           td.addColumn(ScalarColumnDesc<String> ("catalog",""));
           td.addColumn(ScalarColumnDesc<Int>    ("calibrationGroup",""));
           td.addColumn(ArrayColumnDesc<Double>  ("position",""));
           td.addColumn(ArrayColumnDesc<String>  ("transition",""));
           td.addColumn(ArrayColumnDesc<Double>  ("restFrequency",""));
           td.addColumn(ArrayColumnDesc<Double>  ("sysVel",""));
           td.addColumn(ScalarColumnDesc<String> ("sourceModel",""));
           td.addColumn(ScalarColumnDesc<Double> ("deltaVel",""));
           td.addColumn(ArrayColumnDesc<Double>  ("rangeVel",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           sourceId.attach(itsTable,"sourceId");
           spectralWindowId.attach(itsTable,"spectralWindowId");
           timeInterval.attach(itsTable,"timeInterval");
           numLines.attach(itsTable,"numLines");
           sourceName.attach(itsTable,"sourceName");
           code.attach(itsTable,"code");
           direction.attach(itsTable,"direction");
           properMotion.attach(itsTable,"properMotion");
           sourceParameterId.attach(itsTable,"sourceParameterId");
           catalog.attach(itsTable,"catalog");
           calibrationGroup.attach(itsTable,"calibrationGroup");
           position.attach(itsTable,"position");
           transition.attach(itsTable,"transition");
           restFrequency.attach(itsTable,"restFrequency");
           sysVel.attach(itsTable,"sysVel");
           sourceModel.attach(itsTable,"sourceModel");
           deltaVel.attach(itsTable,"deltaVel");
           rangeVel.attach(itsTable,"rangeVel");
         }
        //
        //SpectralWindow Table
        //

         else if (tabName=="SpectralWindowTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("spectralWindowId",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numChan",""));
           td.addColumn(ScalarColumnDesc<Double> ("refFreq",""));
           td.addColumn(ArrayColumnDesc<Double>  ("chanFreq",""));
           td.addColumn(ArrayColumnDesc<Double>  ("chanWidth",""));
           td.addColumn(ArrayColumnDesc<Double>  ("effectiveBw",""));
           td.addColumn(ArrayColumnDesc<Double>  ("resolution",""));
           td.addColumn(ScalarColumnDesc<Double> ("totBandwidth",""));
           td.addColumn(ScalarColumnDesc<Int>    ("netSideband",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("flagRow",""));
           td.addColumn(ArrayColumnDesc<String>  ("assocSpectralWindowId",""));
           td.addColumn(ScalarColumnDesc<Int>    ("dopplerId",""));
           td.addColumn(ScalarColumnDesc<String> ("name",""));
           td.addColumn(ScalarColumnDesc<Int>    ("measFreqRef",""));
           td.addColumn(ScalarColumnDesc<Int>    ("bbcNo",""));
           td.addColumn(ScalarColumnDesc<Int>    ("bbcSideband",""));
           td.addColumn(ScalarColumnDesc<Int>    ("ifConvChain",""));
           td.addColumn(ScalarColumnDesc<Int>    ("freqGroup",""));
           td.addColumn(ScalarColumnDesc<String> ("freqGroupName",""));
           td.addColumn(ArrayColumnDesc<String>  ("assocNature",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           spectralWindowId.attach(itsTable,"spectralWindowId");
           numChan.attach(itsTable,"numChan");
           refFreq.attach(itsTable,"refFreq");
           chanFreq.attach(itsTable,"chanFreq");
           chanWidth.attach(itsTable,"chanWidth");
           effectiveBw.attach(itsTable,"effectiveBw");
           resolution.attach(itsTable,"resolution");
           totBandwidth.attach(itsTable,"totBandwidth");
           netSideband.attach(itsTable,"netSideband");
           flagRow.attach(itsTable,"flagRow");
           assocSpectralWindowId.attach(itsTable,"assocSpectralWindowId");
           dopplerId.attach(itsTable,"dopplerId");
           name.attach(itsTable,"name");
           measFreqRef.attach(itsTable,"measFreqRef");
           bbcNo.attach(itsTable,"bbcNo");
           bbcSideband.attach(itsTable,"bbcSideband");
           ifConvChain.attach(itsTable,"ifConvChain");
           freqGroup.attach(itsTable,"freqGroup");
           freqGroupName.attach(itsTable,"freqGroupName");
           assocNatureAUX.attach(itsTable,"assocNature");
         }
        //
        //State Table
        //

         else if (tabName=="StateTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("stateId",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("sig",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("ref",""));
           td.addColumn(ScalarColumnDesc<Int>    ("calloadNum",""));
           td.addColumn(ScalarColumnDesc<String> ("obsMode",""));
           td.addColumn(ScalarColumnDesc<String> ("obsIntent",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("flagRow",""));
           td.addColumn(ScalarColumnDesc<Float>  ("weight",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           stateIdAUX.attach(itsTable,"stateId");
           sig.attach(itsTable,"sig");
           ref.attach(itsTable,"ref");
           calloadNum.attach(itsTable,"calloadNum");
           obsMode.attach(itsTable,"obsMode");
           obsIntent.attach(itsTable,"obsIntent");
           flagRow.attach(itsTable,"flagRow");
           weight.attach(itsTable,"weight");

         }
        //
        //Station Table
        //

         else if (tabName=="StationTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("stationId",""));
           td.addColumn(ScalarColumnDesc<String> ("name",""));
           td.addColumn(ArrayColumnDesc<Double>  ("position",""));
           td.addColumn(ScalarColumnDesc<String> ("type",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           stationId.attach(itsTable,"stationId");
           name.attach(itsTable,"name");
           position.attach(itsTable,"position");
           type.attach(itsTable,"type");

         }
        //
        //Subscan Table
        //

         else if (tabName=="SubscanTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("execBlockId",""));
           td.addColumn(ScalarColumnDesc<Int>    ("scanNumber",""));
           td.addColumn(ScalarColumnDesc<Int>    ("subscanNumber",""));
           td.addColumn(ScalarColumnDesc<String> ("startTime",""));
           td.addColumn(ScalarColumnDesc<String> ("endTime",""));
           td.addColumn(ScalarColumnDesc<String> ("subscanIntent",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numberIntegration",""));
           td.addColumn(ArrayColumnDesc<Int>     ("numberSubintegration",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("flagRow",""));
           td.addColumn(ScalarColumnDesc<String> ("fieldName",""));
           td.addColumn(ScalarColumnDesc<String> ("subscanMode",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           execBlockId.attach(itsTable,"execBlockId");
           scanNumber.attach(itsTable,"scanNumber");
           subscanNumber.attach(itsTable,"subscanNumber");
           startTime.attach(itsTable,"startTime");
           endTime.attach(itsTable,"endTime");
           subscanIntent.attach(itsTable,"subscanIntent");
           numberIntegration.attach(itsTable,"numberIntegration");
           numberSubintegration.attach(itsTable,"numberSubintegration");
           flagRow.attach(itsTable,"flagRow");
           fieldNameAUX.attach(itsTable,"fieldName");
           subscanMode.attach(itsTable,"subscanMode");
         }
        //
        //SwitchCycle Table
        //

         else if (tabName=="SwitchCycleTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("switchCycleId",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numStep",""));
           td.addColumn(ArrayColumnDesc<Double>   ("weightArray",""));
           td.addColumn(ArrayColumnDesc<Double>  ("offsetArray",""));
           td.addColumn(ArrayColumnDesc<Double>  ("freqOffsetArray",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           switchCycleIdAUX.attach(itsTable,"switchCycleId");
           numStep.attach(itsTable,"numStep");
           weightArray.attach(itsTable,"weightArray");
           offsetArray.attach(itsTable,"offsetArray");
           freqOffsetArray.attach(itsTable,"freqOffsetArray");
        }
	//
	//CalCurve Table
	//

         else if (tabName=="CalCurveTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("calDataId",""));
           td.addColumn(ScalarColumnDesc<String> ("calReductionId",""));
           td.addColumn(ScalarColumnDesc<String> ("antennaName",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numPoly",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numReceptors",""));
           td.addColumn(ScalarColumnDesc<String> ("startValidTime",""));
           td.addColumn(ScalarColumnDesc<String> ("endValidTime",""));
           td.addColumn(ScalarColumnDesc<String> ("typeCurve",""));
           td.addColumn(ArrayColumnDesc<Double>  ("curve",""));
           td.addColumn(ArrayColumnDesc<Double>  ("frequencyRange",""));
           td.addColumn(ScalarColumnDesc<String> ("refAntennaName",""));
           td.addColumn(ArrayColumnDesc<String>  ("polarizationType",""));
           td.addColumn(ScalarColumnDesc<String> ("receiverBand",""));
           td.addColumn(ScalarColumnDesc<String> ("timeOrigin",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           calDataId.attach(itsTable,"calDataId");
           calReductionId.attach(itsTable,"calReductionId");
           antennaName.attach(itsTable,"antennaName");
           numReceptors.attach(itsTable,"numReceptors");
           numPoly.attach(itsTable,"numPoly");
           startValidTime.attach(itsTable,"startValidTime");
           endValidTime.attach(itsTable,"endValidTime");
           typeCurve.attach(itsTable,"typeCurve");
           curve.attach(itsTable,"curve");
           frequencyRange.attach(itsTable,"frequencyRange");
           refAntennaName.attach(itsTable,"refAntennaName");
           polarizationType.attach(itsTable,"polarizationType");
           receiverBand.attach(itsTable,"receiverBand");
           timeOrigin.attach(itsTable,"timeOrigin");

         }
	//
	//CalData Table
	//

         else if (tabName=="CalDataTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("calDataId",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numScan",""));
           td.addColumn(ArrayColumnDesc<Int>     ("scanSet",""));
           td.addColumn(ScalarColumnDesc<String> ("calType",""));
           td.addColumn(ScalarColumnDesc<String> ("startTimeObserved",""));
           td.addColumn(ScalarColumnDesc<String> ("endTimeObserved",""));
           td.addColumn(ScalarColumnDesc<String> ("calDataType",""));
           td.addColumn(ScalarColumnDesc<Int>    ("frequencyGroup",""));
           td.addColumn(ScalarColumnDesc<String> ("freqGroupName",""));
           td.addColumn(ScalarColumnDesc<String> ("fieldName",""));
           td.addColumn(ArrayColumnDesc<String>  ("fieldCode",""));
           td.addColumn(ArrayColumnDesc<String>  ("sourceName",""));
           td.addColumn(ArrayColumnDesc<String>  ("sourceCode",""));
           td.addColumn(ArrayColumnDesc<String>  ("scanIntent",""));
           td.addColumn(ScalarColumnDesc<String> ("assocCalDataId",""));
           td.addColumn(ScalarColumnDesc<String> ("assocCalNature",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           calDataId.attach(itsTable,"calDataId");
           numScan.attach(itsTable,"numScan");
           scanSet.attach(itsTable,"scanSet");
           calType.attach(itsTable,"calType");
           startTimeObserved.attach(itsTable,"startTimeObserved");
           endTimeObserved.attach(itsTable,"endTimeObserved");
           calDataType.attach(itsTable,"calDataType");
           frequencyGroup.attach(itsTable,"frequencyGroup");
           freqGroupName.attach(itsTable,"freqGroupName");
           fieldNameAUX.attach(itsTable,"fieldName");
           fieldCode.attach(itsTable,"fieldCode");
           sourceNameAUX.attach(itsTable,"sourceName");
           sourceCode.attach(itsTable,"sourceCode");
           scanIntent.attach(itsTable,"scanIntent");
           assocCalDataId.attach(itsTable,"assocCalDataId");
           assocCalNature.attach(itsTable,"assocCalNature");
         }

	//
	//CalPhase Table
	//
         else if (tabName=="CalPhaseTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("calDataId",""));
           td.addColumn(ScalarColumnDesc<String> ("calReductionId",""));
           td.addColumn(ScalarColumnDesc<String> ("basebandName",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numReceptors",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numBaseline",""));
           td.addColumn(ScalarColumnDesc<String> ("startValidTime",""));
           td.addColumn(ScalarColumnDesc<String> ("endValidTime",""));
           td.addColumn(ArrayColumnDesc<Double>  ("frequencyRange",""));
           td.addColumn(ArrayColumnDesc<String>  ("antennaNames",""));
           td.addColumn(ArrayColumnDesc<String>  ("polarizationType",""));
           td.addColumn(ArrayColumnDesc<Double>  ("decorrelationFactor",""));
           td.addColumn(ArrayColumnDesc<Double>  ("uncorrPhaseRms",""));
           td.addColumn(ArrayColumnDesc<Double>  ("corrPhaseRms",""));
           td.addColumn(ArrayColumnDesc<Double>  ("statPhaseRms",""));
           td.addColumn(ArrayColumnDesc<Bool>    ("corrValidity",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           calDataId.attach(itsTable,"calDataId");
           calReductionId.attach(itsTable,"calReductionId");
           basebandName.attach(itsTable,"basebandName");
           numReceptors.attach(itsTable,"numReceptors");
           numBaseline.attach(itsTable,"numBaseline");
           startValidTime.attach(itsTable,"startValidTime");
           endValidTime.attach(itsTable,"endValidTime");
           frequencyRange.attach(itsTable,"frequencyRange");
           antennaNames.attach(itsTable,"antennaNames");
           polarizationType.attach(itsTable,"polarizationType");
           decorrelationFactor.attach(itsTable,"decorrelationFactor");
           uncorrPhaseRms.attach(itsTable,"uncorrPhaseRms");
           corrPhaseRms.attach(itsTable,"corrPhaseRms");
           statPhaseRms.attach(itsTable,"statPhaseRms");
           corrValidity.attach(itsTable,"corrValidity");
         }
	//
	//CalReduction Table
	//
         else if (tabName=="CalReductionTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("calReductionId",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numApplied",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numParam",""));
           td.addColumn(ScalarColumnDesc<String> ("timeReduced",""));
           td.addColumn(ArrayColumnDesc<String>  ("calAppliedArray",""));
           td.addColumn(ArrayColumnDesc<String>  ("paramSet",""));
           td.addColumn(ScalarColumnDesc<String> ("messages",""));
           td.addColumn(ScalarColumnDesc<String> ("software",""));
           td.addColumn(ScalarColumnDesc<String> ("softwareVersion",""));
           td.addColumn(ScalarColumnDesc<String> ("invalidConditions",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           calReductionId.attach(itsTable,"calReductionId");
           numApplied.attach(itsTable,"numApplied");
           numParam.attach(itsTable,"numParam");
           timeReduced.attach(itsTable,"timeReduced");
           calAppliedArray.attach(itsTable,"calAppliedArray");
           paramSet.attach(itsTable,"paramSet");
           messages.attach(itsTable,"messages");
           software.attach(itsTable,"software");
           softwareVersion.attach(itsTable,"softwareVersion");
           invalidConditions.attach(itsTable,"invalidConditions");

         } 
        //
        //CalSeeing Table
        //
         else if (tabName=="CalSeeingTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("calDataId",""));
           td.addColumn(ScalarColumnDesc<String> ("calReductionId",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numBaseLength",""));
           td.addColumn(ScalarColumnDesc<String> ("startValidTime",""));
           td.addColumn(ScalarColumnDesc<String> ("endValidTime",""));
           td.addColumn(ArrayColumnDesc<Double>  ("frequencyRange",""));
           td.addColumn(ArrayColumnDesc<Double>  ("baseLength",""));
           td.addColumn(ArrayColumnDesc<Double>  ("uncorrPhaseRms",""));
           td.addColumn(ArrayColumnDesc<Double>  ("corrPhaseRms",""));
           td.addColumn(ScalarColumnDesc<Double> ("seeing",""));
           td.addColumn(ScalarColumnDesc<Double> ("seeingFrequency",""));
           td.addColumn(ScalarColumnDesc<Double> ("seeingFreqBandwidth",""));
           td.addColumn(ScalarColumnDesc<Double> ("exponent",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           calDataId.attach(itsTable,"calDataId");
           calReductionId.attach(itsTable,"calReductionId");
           numBaseLength.attach(itsTable,"numBaseLength");
           startValidTime.attach(itsTable,"startValidTime");
           endValidTime.attach(itsTable,"endValidTime");
           frequencyRange.attach(itsTable,"frequencyRange");
           baseLength.attach(itsTable,"baseLength");
           corrPhaseRms.attach(itsTable,"corrPhaseRms");
           uncorrPhaseRms.attach(itsTable,"uncorrPhaseRms");
           seeing.attach(itsTable,"seeing");
           seeingFrequency.attach(itsTable,"seeingFrequency");
           seeingFreqBandwidth.attach(itsTable,"seeingFreqBandwidth");
           exponent.attach(itsTable,"exponent");
         } 
        //
        //CalSeeing Table
        //
         else if (tabName=="BeamTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("beamId",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;
           
           beamId.attach(itsTable,"beamId");
          }
        //
        //CalAtmosphere Table
        //
         else if (tabName=="CalAtmosphereTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("calDataId",""));
           td.addColumn(ScalarColumnDesc<String> ("calReductionId",""));
           td.addColumn(ScalarColumnDesc<String> ("antennaName",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numReceptors",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numFreq",""));
           td.addColumn(ScalarColumnDesc<String> ("endValidTime",""));
           td.addColumn(ScalarColumnDesc<String> ("startValidTime",""));
           td.addColumn(ArrayColumnDesc<String>  ("polarizationType",""));
           td.addColumn(ArrayColumnDesc<Double>  ("frequencyRange",""));
           td.addColumn(ArrayColumnDesc<Double>  ("frequencySpectrum",""));
           td.addColumn(ScalarColumnDesc<String> ("syscalType",""));
           td.addColumn(ArrayColumnDesc<Double>  ("tSysSpectrum",""));
           td.addColumn(ArrayColumnDesc<Double>  ("tRecSpectrum",""));
           td.addColumn(ArrayColumnDesc<Double>  ("tAtmSpectrum",""));
           td.addColumn(ArrayColumnDesc<Double>  ("tauSpectrum",""));
           td.addColumn(ArrayColumnDesc<Double>  ("sbGainSpectrum",""));
           td.addColumn(ArrayColumnDesc<Double>  ("forwardEffSpectrum",""));
           td.addColumn(ScalarColumnDesc<Double> ("groundPressure",""));
           td.addColumn(ScalarColumnDesc<Double> ("groundTemperature",""));
           td.addColumn(ScalarColumnDesc<Double> ("groundRelHumidity",""));
           td.addColumn(ScalarColumnDesc<String> ("subType",""));
           td.addColumn(ArrayColumnDesc<Double>  ("tSys",""));
           td.addColumn(ArrayColumnDesc<Double>  ("tRec",""));
           td.addColumn(ArrayColumnDesc<Double>  ("tAtm",""));
           td.addColumn(ArrayColumnDesc<Double>  ("sbGain",""));
           td.addColumn(ArrayColumnDesc<Double>  ("water",""));
           td.addColumn(ArrayColumnDesc<Double>  ("forwardEfficiency",""));
           td.addColumn(ArrayColumnDesc<Double>  ("tau",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           calDataId.attach(itsTable,"calDataId");
           calReductionId.attach(itsTable,"calReductionId");
           antennaName.attach(itsTable,"antennaName");
           numReceptors.attach(itsTable,"numReceptors");
           numFreq.attach(itsTable,"numFreq");
           endValidTime.attach(itsTable,"endValidTime");
           startValidTime.attach(itsTable,"startValidTime");
           polarizationType.attach(itsTable,"polarizationType");
           frequencyRange.attach(itsTable,"frequencyRange");
           frequencySpectrum.attach(itsTable,"frequencySpectrum");
           syscalType.attach(itsTable,"syscalType");
           tSysSpectrum.attach(itsTable,"tSysSpectrum");
           tRecSpectrum.attach(itsTable,"tRecSpectrum");
           tAtmSpectrum.attach(itsTable,"tAtmSpectrum");
           tauSpectrum.attach(itsTable,"tauSpectrum");
           sbGainSpectrum.attach(itsTable,"sbGainSpectrum");
           forwardEffSpectrum.attach(itsTable,"forwardEffSpectrum");
           groundPressure.attach(itsTable,"groundPressure");
           groundTemperature.attach(itsTable,"groundTemperature");
           groundRelHumidity.attach(itsTable,"groundRelHumidity");
           subType.attach(itsTable,"subType");
           tSys.attach(itsTable,"tSys");
           tRec.attach(itsTable,"tRec");
           tAtm.attach(itsTable,"tAtm");
           sbGain.attach(itsTable,"sbGain");
           water.attach(itsTable,"water");
           forwardEfficiency.attach(itsTable,"forwardEfficiency");
           tau.attach(itsTable,"tau");
          }
        //
        //CalAmpli Table
        //
         else if (tabName=="CalAmpliTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("calDataId",""));
           td.addColumn(ScalarColumnDesc<String> ("calReductionId",""));
           td.addColumn(ScalarColumnDesc<String> ("antennaName",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numReceptors",""));
           td.addColumn(ScalarColumnDesc<String> ("endValidTime",""));
           td.addColumn(ScalarColumnDesc<String> ("startValidTime",""));
           td.addColumn(ArrayColumnDesc<String>  ("polarizationType",""));
           td.addColumn(ArrayColumnDesc<Double>  ("frequencyRange",""));
           td.addColumn(ArrayColumnDesc<Double>  ("apertureEfficiencyError",""));
           td.addColumn(ArrayColumnDesc<Double>  ("uncorrectedApertureEfficiency",""));
           td.addColumn(ArrayColumnDesc<Double>  ("correctedApertureEfficiency",""));
           td.addColumn(ArrayColumnDesc<Bool>    ("correctionValidity",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           calDataId.attach(itsTable,"calDataId");
           calReductionId.attach(itsTable,"calReductionId");
           antennaName.attach(itsTable,"antennaName");
           numReceptors.attach(itsTable,"numReceptors");
           numFreq.attach(itsTable,"numFreq");
           endValidTime.attach(itsTable,"endValidTime");
           startValidTime.attach(itsTable,"startValidTime");
           polarizationType.attach(itsTable,"polarizationType");
           frequencyRange.attach(itsTable,"frequencyRange");
           apertureEfficiencyError.attach(itsTable,"apertureEfficiencyError");
           uncorrectedApertureEfficiency.attach(itsTable,"uncorrectedApertureEfficiency");
           correctedApertureEfficiency.attach(itsTable,"correctedApertureEfficiency");
           correctionValidity.attach(itsTable,"correctionValidity");
         }
        //
        //CalBandpass Table
        //
         else if (tabName=="CalBandpassTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("calDataId",""));
           td.addColumn(ScalarColumnDesc<String> ("calReductionId",""));
           td.addColumn(ScalarColumnDesc<String> ("antennaName",""));
           td.addColumn(ScalarColumnDesc<String> ("basebandName",""));
           td.addColumn(ScalarColumnDesc<String> ("startValidTime",""));
           td.addColumn(ScalarColumnDesc<String> ("endValidTime",""));
           td.addColumn(ArrayColumnDesc<String>  ("polarizationType",""));
           td.addColumn(ArrayColumnDesc<Double>  ("freqLimits",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numReceptors",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numPhasePoly",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numAmpliPoly",""));
           td.addColumn(ScalarColumnDesc<String> ("receiverBand",""));
           td.addColumn(ScalarColumnDesc<String> ("refAntennaName",""));
           td.addColumn(ArrayColumnDesc<Double>  ("phaseCurve",""));
           td.addColumn(ArrayColumnDesc<Double>  ("ampliCurve",""));
      
           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           calDataId.attach(itsTable,"calDataId");
           calReductionId.attach(itsTable,"calReductionId");
           antennaName.attach(itsTable,"antennaName");
           basebandName.attach(itsTable,"basebandName");
           endValidTime.attach(itsTable,"endValidTime");
           startValidTime.attach(itsTable,"startValidTime");
           polarizationType.attach(itsTable,"polarizationType");
           freqLimits.attach(itsTable,"freqLimits");
           numReceptors.attach(itsTable,"numReceptors");
           numPhasePoly.attach(itsTable,"numPhasePoly");
           numAmpliPoly.attach(itsTable,"numAmpliPoly");
           receiverBand.attach(itsTable,"receiverBand");
           refAntennaName.attach(itsTable,"refAntennaName");
           phaseCurve.attach(itsTable,"phaseCurve");
           ampliCurve.attach(itsTable,"ampliCurve");
         }
        //
        //AlmaRadiometer Table
        //
         else if (tabName=="AlmaRadiometerTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("modeId",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numBand",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           modeId.attach(itsTable,"modeId");
           numBand.attach(itsTable,"numBand");
         }
        //
        //CalDelay Table
        //
         else if (tabName=="CalDelayTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("calDataId",""));
           td.addColumn(ScalarColumnDesc<String> ("calReductionId",""));
           td.addColumn(ScalarColumnDesc<String> ("antennaName",""));
           td.addColumn(ScalarColumnDesc<String> ("basebandName",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numReceptors",""));
           td.addColumn(ScalarColumnDesc<String> ("startValidTime",""));
           td.addColumn(ScalarColumnDesc<String> ("endValidTime",""));
           td.addColumn(ArrayColumnDesc<Double>  ("delayOffset",""));
           td.addColumn(ArrayColumnDesc<Double>  ("delayError",""));
           td.addColumn(ScalarColumnDesc<Double> ("crossDelayOffset",""));
           td.addColumn(ScalarColumnDesc<Double> ("crossDelayOffsetError",""));
           td.addColumn(ScalarColumnDesc<String> ("receiverBand",""));
           td.addColumn(ScalarColumnDesc<String> ("refAntennaName",""));
           td.addColumn(ArrayColumnDesc<String>  ("polarizationType",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           calDataId.attach(itsTable,"calDataId");
           calReductionId.attach(itsTable,"calReductionId");
           antennaName.attach(itsTable,"antennaName");
           basebandName.attach(itsTable,"basebandName");
           numReceptors.attach(itsTable,"numReceptors");
           startValidTime.attach(itsTable,"startValidTime");
           endValidTime.attach(itsTable,"endValidTime");
           delayOffset.attach(itsTable,"delayOffset");
           delayError.attach(itsTable,"delayError");
           crossDelayOffset.attach(itsTable,"crossDelayOffset");
           crossDelayOffsetError.attach(itsTable,"crossDelayOffsetError");
           receiverBand.attach(itsTable,"receiverBand");
           refAntennaName.attach(itsTable,"refAntennaName");
           polarizationType.attach(itsTable,"polarizationType");
         }
        //
        //CalDevice Table
        //
         else if (tabName=="CalDeviceTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("antennaId",""));
           td.addColumn(ScalarColumnDesc<Int>    ("feedId",""));
           td.addColumn(ScalarColumnDesc<String> ("spectralWindowId",""));
           td.addColumn(ScalarColumnDesc<String> ("timeInterval",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numCalload",""));
           td.addColumn(ArrayColumnDesc<Double>  ("noiseCal",""));
           td.addColumn(ArrayColumnDesc<Double>  ("temperatureLoad",""));
           td.addColumn(ArrayColumnDesc<Double>  ("calEff",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           antennaId.attach(itsTable,"antennaId");
           feedId.attach(itsTable,"feedId");
           spectralWindowId.attach(itsTable,"spectralWindowId");
           timeInterval.attach(itsTable,"timeInterval");
           numCalload.attach(itsTable,"numCalload");
           noiseCal.attach(itsTable,"noiseCal");
           temperatureLoad.attach(itsTable,"temperatureLoad");
           calEff.attach(itsTable,"calEff");
         }
        //
        //CalFlux Table
        //
         else if (tabName=="CalFluxTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("calDataId",""));
           td.addColumn(ScalarColumnDesc<String> ("calReductionId",""));
           td.addColumn(ScalarColumnDesc<String> ("sourceName",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numFreq",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numStokes",""));
           td.addColumn(ScalarColumnDesc<String> ("startValidTime",""));
           td.addColumn(ScalarColumnDesc<String> ("endValidTime",""));
           td.addColumn(ArrayColumnDesc<Int>     ("Stokes",""));
           td.addColumn(ArrayColumnDesc<Double>  ("frequency",""));
           td.addColumn(ArrayColumnDesc<Double>  ("frequencyWidth",""));
           td.addColumn(ArrayColumnDesc<Double>  ("flux",""));
           td.addColumn(ArrayColumnDesc<Double>  ("fluxError",""));
           td.addColumn(ScalarColumnDesc<String> ("fluxMethod",""));
           td.addColumn(ArrayColumnDesc<Double>  ("size",""));
           td.addColumn(ArrayColumnDesc<Double>  ("sizeError",""));
           td.addColumn(ArrayColumnDesc<Double>  ("PA",""));
           td.addColumn(ArrayColumnDesc<Double>  ("PAError",""));
           td.addColumn(ArrayColumnDesc<Double>  ("direction",""));
   
           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           calDataId.attach(itsTable,"calDataId");
           calReductionId.attach(itsTable,"calReductionId");
           sourceName.attach(itsTable,"sourceName");
           numFreq.attach(itsTable,"numFreq");
           numStokes.attach(itsTable,"numStokes");
           startValidTime.attach(itsTable,"startValidTime");
           endValidTime.attach(itsTable,"endValidTime");
           Stokes.attach(itsTable,"Stokes");
           frequencyAUX.attach(itsTable,"frequency");
           frequencyWidth.attach(itsTable,"frequencyWidth");
           flux.attach(itsTable,"flux");
           fluxError.attach(itsTable,"fluxError");
           fluxMethod.attach(itsTable,"fluxMethod");
           size.attach(itsTable,"size");
           sizeError.attach(itsTable,"sizeError");
           PA.attach(itsTable,"PA");
           PAError.attach(itsTable,"PAError");
           direction.attach(itsTable,"direction");
         }
        //
        //CalFocus Table
        //
         else if (tabName=="CalFocusTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("calDataId",""));
           td.addColumn(ScalarColumnDesc<String> ("calReductionId",""));
           td.addColumn(ScalarColumnDesc<String> ("antennaName",""));
           td.addColumn(ArrayColumnDesc<Double>  ("frequencyRange",""));
           td.addColumn(ScalarColumnDesc<String> ("startValidTime",""));
           td.addColumn(ScalarColumnDesc<String> ("endValidTime",""));
           td.addColumn(ArrayColumnDesc<Double>  ("offset",""));
           td.addColumn(ArrayColumnDesc<Double>  ("error",""));
           td.addColumn(ScalarColumnDesc<String> ("method",""));
           td.addColumn(ArrayColumnDesc<Double>  ("pointingDirection",""));
           td.addColumn(ArrayColumnDesc<Bool>    ("wasFixed",""));
           td.addColumn(ScalarColumnDesc<String> ("receiverBand",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           calDataId.attach(itsTable,"calDataId");
           calReductionId.attach(itsTable,"calReductionId");
           antennaName.attach(itsTable,"antennaName");
           frequencyRange.attach(itsTable,"frequencyRange");
           startValidTime.attach(itsTable,"startValidTime");
           endValidTime.attach(itsTable,"endValidTime");
           offset.attach(itsTable,"offset");
           error.attach(itsTable,"error");
           method.attach(itsTable,"method");
           pointingDirection.attach(itsTable,"pointingDirection");
           wasFixed.attach(itsTable,"wasFixed");
           receiverBand.attach(itsTable,"receiverBand");
         }
        //
        //CalFocusModel Table
        //
         else if (tabName=="CalFocusModelTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("calDataId",""));
           td.addColumn(ScalarColumnDesc<String> ("calReductionId",""));
           td.addColumn(ScalarColumnDesc<String> ("antennaName",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numCoeff",""));
           td.addColumn(ScalarColumnDesc<String> ("startValidTime",""));
           td.addColumn(ScalarColumnDesc<String> ("endValidTime",""));
           td.addColumn(ArrayColumnDesc<Double>  ("focusRMS",""));
           td.addColumn(ArrayColumnDesc<String>  ("coeffName",""));
           td.addColumn(ArrayColumnDesc<String>  ("coeffFormula",""));
           td.addColumn(ArrayColumnDesc<Double>  ("coeffValue",""));
           td.addColumn(ArrayColumnDesc<Double>  ("coeffError",""));
           td.addColumn(ArrayColumnDesc<Bool>    ("coeffFixed",""));
           td.addColumn(ScalarColumnDesc<String> ("focusModel",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numSourceObs",""));
           td.addColumn(ScalarColumnDesc<String> ("receiverBand",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           calDataId.attach(itsTable,"calDataId");
           calReductionId.attach(itsTable,"calReductionId");
           antennaName.attach(itsTable,"antennaName");
           numCoeff.attach(itsTable,"numCoeff");
           startValidTime.attach(itsTable,"startValidTime");
           endValidTime.attach(itsTable,"endValidTime");
           focusRMS.attach(itsTable,"focusRMS");
           coeffName.attach(itsTable,"coeffName");
           coeffFormula.attach(itsTable,"coeffFormula");
           coeffValue.attach(itsTable,"coeffValue");
           coeffError.attach(itsTable,"coeffError");
           coeffFixed.attach(itsTable,"coeffFixed");
           focusModel.attach(itsTable,"focusModel");
           numSourceObs.attach(itsTable,"numSourceObs");
           receiverBand.attach(itsTable,"receiverBand");
         }
        //
        //CalGain Table
        //
         else if (tabName=="CalGainTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("calDataId",""));
           td.addColumn(ScalarColumnDesc<String> ("calReductionId",""));
           td.addColumn(ScalarColumnDesc<String> ("startValidTime",""));
           td.addColumn(ScalarColumnDesc<String> ("endValidTime",""));
           td.addColumn(ArrayColumnDesc<Double>  ("gain",""));
           td.addColumn(ArrayColumnDesc<Bool>    ("gainValid",""));
           td.addColumn(ArrayColumnDesc<Double>  ("fit",""));
           td.addColumn(ArrayColumnDesc<Double>  ("fitWeight",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("totalGainValid",""));
           td.addColumn(ScalarColumnDesc<Double> ("totalFit",""));
           td.addColumn(ScalarColumnDesc<Double> ("totalFitWeight",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           calDataId.attach(itsTable,"calDataId");
           calReductionId.attach(itsTable,"calReductionId");
           startValidTime.attach(itsTable,"startValidTime");
           endValidTime.attach(itsTable,"endValidTime");
           gain.attach(itsTable,"gain");
           gainValid.attach(itsTable,"gainValid");
           fit.attach(itsTable,"fit");
           fitWeight.attach(itsTable,"fitWeight");
           totalGainValid.attach(itsTable,"totalGainValid");
           totalFit.attach(itsTable,"totalFit");
           totalFitWeight.attach(itsTable,"totalFitWeight");
         }
        //
        //CalHolography Table
        //
         else if (tabName=="CalHolographyTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("calDataId",""));
           td.addColumn(ScalarColumnDesc<String> ("calReductionId",""));
           td.addColumn(ScalarColumnDesc<String> ("antennaName",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numScrew",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numReceptors",""));
           td.addColumn(ScalarColumnDesc<String> ("startValidTime",""));
           td.addColumn(ScalarColumnDesc<String> ("endValidTime",""));
           td.addColumn(ArrayColumnDesc<Double>  ("focusPosition",""));
           td.addColumn(ScalarColumnDesc<Double> ("rawRms",""));
           td.addColumn(ScalarColumnDesc<Double> ("weightedRms",""));
           td.addColumn(ArrayColumnDesc<String>  ("screwName",""));
           td.addColumn(ArrayColumnDesc<Double>  ("screwMotion",""));
           td.addColumn(ArrayColumnDesc<Double>  ("screwMotionError",""));
           td.addColumn(ScalarColumnDesc<Int>    ("panelModes",""));
           td.addColumn(ScalarColumnDesc<Double> ("frequency",""));
           td.addColumn(ScalarColumnDesc<String> ("beamMapUID",""));
           td.addColumn(ScalarColumnDesc<String> ("surfaceMapUID",""));
           td.addColumn(ArrayColumnDesc<String>  ("polarizationType",""));
 
           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           calDataId.attach(itsTable,"calDataId");
           calReductionId.attach(itsTable,"calReductionId");
           antennaName.attach(itsTable,"antennaName");
           numScrew.attach(itsTable,"numScrew");
           numReceptors.attach(itsTable,"numReceptors");
           startValidTime.attach(itsTable,"startValidTime");
           endValidTime.attach(itsTable,"endValidTime");
           focusPosition.attach(itsTable,"focusPosition");
           rawRms.attach(itsTable,"rawRms");
           weightedRms.attach(itsTable,"weightedRms");
           screwName.attach(itsTable,"screwName");
           screwMotionError.attach(itsTable,"screwMotionError");
           panelModes.attach(itsTable,"panelModes");
           frequency.attach(itsTable,"frequency");
           beamMapUID.attach(itsTable,"beamMapUID");
           surfaceMapUID.attach(itsTable,"surfaceMapUID");
           polarizationType.attach(itsTable,"polarizationType");
         }
        //
        //CalPointing Table
        //
         else if (tabName=="CalPointingTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("calDataId",""));
           td.addColumn(ScalarColumnDesc<String> ("calReductionId",""));
           td.addColumn(ScalarColumnDesc<String> ("antennaName",""));
           td.addColumn(ScalarColumnDesc<String> ("startValidTime",""));
           td.addColumn(ScalarColumnDesc<String> ("endValidTime",""));
           td.addColumn(ScalarColumnDesc<String> ("receiverBand",""));
	   td.addColumn(ArrayColumnDesc<Double>  ("frequencyRange",""));
	   td.addColumn(ArrayColumnDesc<Double>  ("direction",""));
	   td.addColumn(ArrayColumnDesc<Double>  ("collOffset",""));
	   td.addColumn(ArrayColumnDesc<Double>  ("collError",""));
           td.addColumn(ScalarColumnDesc<String> ("pointingMethod",""));
           td.addColumn(ScalarColumnDesc<String> ("mode",""));
	   td.addColumn(ArrayColumnDesc<Double>  ("beamWidth",""));
	   td.addColumn(ArrayColumnDesc<Double>  ("beamWidthError",""));
           td.addColumn(ScalarColumnDesc<Double> ("beamPA",""));
           td.addColumn(ScalarColumnDesc<Double> ("beamPAError",""));
           td.addColumn(ScalarColumnDesc<Double> ("peakIntensity",""));
           td.addColumn(ScalarColumnDesc<Double> ("peakIntensityError",""));
   
           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           calDataId.attach(itsTable,"calDataId");
           calReductionId.attach(itsTable,"calReductionId");
           antennaName.attach(itsTable,"antennaName");
           startValidTime.attach(itsTable,"startValidTime");
           endValidTime.attach(itsTable,"endValidTime");
           receiverBand.attach(itsTable,"receiverBand");
           frequencyRange.attach(itsTable,"frequencyRange");
           direction.attach(itsTable,"direction");
           collOffset.attach(itsTable,"collOffset");
           collError.attach(itsTable,"collError");
           pointingMethod.attach(itsTable,"pointingMethod");
           mode.attach(itsTable,"mode");
           beamWidth.attach(itsTable,"beamWidth");
           beamWidthError.attach(itsTable,"beamWidthError");
           beamPA.attach(itsTable,"beamPA");
           beamPAError.attach(itsTable,"beamPAError");
           peakIntensity.attach(itsTable,"peakIntensity");
           peakIntensityError.attach(itsTable,"peakIntensityError");
         }
        //
        //CalPointingModel Table
        //
         else if (tabName=="CalPointingModelTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("calDataId",""));
           td.addColumn(ScalarColumnDesc<String> ("calReductionId",""));
           td.addColumn(ScalarColumnDesc<String> ("antennaName",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numObs",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numCoeff",""));
           td.addColumn(ScalarColumnDesc<String> ("startValidTime",""));
           td.addColumn(ScalarColumnDesc<String> ("endValidTime",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numFormula",""));
           td.addColumn(ScalarColumnDesc<Double> ("azimuthRms",""));
           td.addColumn(ScalarColumnDesc<Double> ("elevationRms",""));
           td.addColumn(ScalarColumnDesc<Double> ("skyRms",""));
           td.addColumn(ArrayColumnDesc<String>  ("coeffName",""));
           td.addColumn(ArrayColumnDesc<Double>  ("coeffVal",""));
           td.addColumn(ArrayColumnDesc<Double>  ("coeffError",""));
           td.addColumn(ArrayColumnDesc<Bool>    ("coeffFixed",""));
           td.addColumn(ArrayColumnDesc<String>  ("coeffFormula",""));
           td.addColumn(ScalarColumnDesc<String> ("pointingModel",""));
           td.addColumn(ScalarColumnDesc<String> ("receiverBand",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           calDataId.attach(itsTable,"calDataId");
           calReductionId.attach(itsTable,"calReductionId");
           antennaName.attach(itsTable,"antennaName");
           numObs.attach(itsTable,"numObs");
           numCoeff.attach(itsTable,"numCoeff");
           startValidTime.attach(itsTable,"startValidTime");
           endValidTime.attach(itsTable,"endValidTime");
           numFormula.attach(itsTable,"numFormula");
           azimuthRms.attach(itsTable,"azimuthRms");
           elevationRms.attach(itsTable,"elevationRms");
           skyRms.attach(itsTable,"skyRms");
           coeffName.attach(itsTable,"coeffName");
           coeffVal.attach(itsTable,"coeffVal");
           coeffError.attach(itsTable,"coeffError");
           coeffFixed.attach(itsTable,"coeffFixed");
           coeffFormula.attach(itsTable,"coeffFormula");
           pointingModel.attach(itsTable,"pointingModel");
           receiverBand.attach(itsTable,"receiverBand");
         } 

        //
        //CalPosition Table
        //
         else if (tabName=="CalPositionTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("calDataId",""));
           td.addColumn(ScalarColumnDesc<String> ("calReductionId",""));
           td.addColumn(ScalarColumnDesc<String> ("antennaName",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numAntenna",""));
           td.addColumn(ScalarColumnDesc<String> ("startValidTime",""));
           td.addColumn(ScalarColumnDesc<String> ("endValidTime",""));
           td.addColumn(ArrayColumnDesc<Double>  ("positionOffset",""));
           td.addColumn(ArrayColumnDesc<Double>  ("positionErr",""));
           td.addColumn(ScalarColumnDesc<String> ("delayRms",""));
           td.addColumn(ScalarColumnDesc<Double> ("phaseRms",""));
           td.addColumn(ScalarColumnDesc<Double> ("axesOffset",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("axesOffsetFixed",""));
           td.addColumn(ScalarColumnDesc<Double> ("axesOffsetErr",""));
           td.addColumn(ScalarColumnDesc<String> ("positionMethod",""));
           td.addColumn(ArrayColumnDesc<String>  ("refAntennaNames",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           calDataId.attach(itsTable,"calDataId");
           calReductionId.attach(itsTable,"calReductionId");
           antennaName.attach(itsTable,"antennaName");
           numAntenna.attach(itsTable,"numAntenna");
           startValidTime.attach(itsTable,"startValidTime");
           endValidTime.attach(itsTable,"endValidTime");
           positionOffset.attach(itsTable,"positionOffset");
           positionErr.attach(itsTable,"positionErr");
           delayRms.attach(itsTable,"delayRms");
           phaseRms.attach(itsTable,"phaseRms");
           axesOffset.attach(itsTable,"axesOffset");
           axesOffsetFixed.attach(itsTable,"axesOffsetFixed");
           axesOffsetErr.attach(itsTable,"axesOffsetErr");
           positionMethod.attach(itsTable,"positionMethod");
           refAntennaNames.attach(itsTable,"refAntennaNames");
         }
        //
        //CalPosition Table
        //
         else if (tabName=="CalPrimaryBeamTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("calDataId",""));
           td.addColumn(ScalarColumnDesc<String> ("calReductionId",""));
           td.addColumn(ScalarColumnDesc<String> ("antennaName",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numPixelX",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numPixelY",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numReceptors",""));
           td.addColumn(ArrayColumnDesc<String>  ("polarizationType",""));
           td.addColumn(ScalarColumnDesc<String> ("startValidTime",""));
           td.addColumn(ScalarColumnDesc<String> ("endValidTime",""));
           td.addColumn(ArrayColumnDesc<Double>  ("frequencyRange",""));
           td.addColumn(ScalarColumnDesc<Double> ("refX",""));
           td.addColumn(ScalarColumnDesc<Double> ("refY",""));
           td.addColumn(ScalarColumnDesc<Double> ("valX",""));
           td.addColumn(ScalarColumnDesc<Double> ("valY",""));
           td.addColumn(ScalarColumnDesc<Double> ("incX",""));
	   td.addColumn(ScalarColumnDesc<Double> ("incY",""));
           td.addColumn(ScalarColumnDesc<String> ("receiverBand",""));
           td.addColumn(ArrayColumnDesc<Double>  ("amplitude",""));
           td.addColumn(ArrayColumnDesc<Double>  ("phase",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           calDataId.attach(itsTable,"calDataId");
           calReductionId.attach(itsTable,"calReductionId");
           antennaName.attach(itsTable,"antennaName");
           numPixelX.attach(itsTable,"numPixelX");
           numPixelY.attach(itsTable,"numPixelY");
           numReceptors.attach(itsTable,"numReceptors");
           startValidTime.attach(itsTable,"startValidTime");
           endValidTime.attach(itsTable,"endValidTime");
           frequencyRange.attach(itsTable,"frequencyRange");
           refX.attach(itsTable,"refX");
           refY.attach(itsTable,"refY");
           valX.attach(itsTable,"valX");
           valY.attach(itsTable,"valY");
           incX.attach(itsTable,"incX");
           incY.attach(itsTable,"incY");
           receiverBand.attach(itsTable,"receiverBand");
           amplitude.attach(itsTable,"amplitude");
           phase.attach(itsTable,"phase");
        }
        //
        //CalWVR Table
        //
         else if (tabName=="CalWVRTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("calDataId",""));
           td.addColumn(ScalarColumnDesc<String> ("calReductionId",""));
           td.addColumn(ScalarColumnDesc<String> ("antennaName",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numPoly",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numChan",""));
           td.addColumn(ScalarColumnDesc<String> ("startValidTime",""));
           td.addColumn(ScalarColumnDesc<String> ("endValidTime",""));
           td.addColumn(ScalarColumnDesc<String> ("WVRMethod",""));
           td.addColumn(ArrayColumnDesc<Double>  ("freqLimits",""));
           td.addColumn(ArrayColumnDesc<Double>  ("pathCoeff",""));
           td.addColumn(ArrayColumnDesc<Double>  ("chanFreq",""));
           td.addColumn(ArrayColumnDesc<Double>  ("chanWidth",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           calDataId.attach(itsTable,"calDataId");
           calReductionId.attach(itsTable,"calReductionId");
           antennaName.attach(itsTable,"antennaName");
           numPoly.attach(itsTable,"numPoly");
           numChan.attach(itsTable,"numChan");
           startValidTime.attach(itsTable,"startValidTime");
           endValidTime.attach(itsTable,"endValidTime");
           WVRMethod.attach(itsTable,"WVRMethod");
           freqLimits.attach(itsTable,"freqLimits");
           pathCoeff.attach(itsTable,"pathCoeff");
           chanFreq.attach(itsTable,"chanFreq");
           chanWidth.attach(itsTable,"chanWidth");
         }
        //
        //Doppler Table
        //
         else if (tabName=="DopplerTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<Int>    ("dopplerId",""));
           td.addColumn(ScalarColumnDesc<Int>    ("sourceId",""));
           td.addColumn(ScalarColumnDesc<Int>    ("transitionIndex",""));
           td.addColumn(ScalarColumnDesc<Double> ("velDef",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           dopplerId.attach(itsTable,"dopplerId");
           sourceId.attach(itsTable,"sourceId");
           transitionIndex.attach(itsTable,"transitionIndex");
           velDef.attach(itsTable,"velDef");
         }
        //
        //Ephemeris Table
        //
         else if (tabName=="EphemerisTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("ephemerisId",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           ephemerisId.attach(itsTable,"ephemerisId");
         }
        //
        //Observation Table
        //
         else if (tabName=="ObservationTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("observationId",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           observationId.attach(itsTable,"observationId");
        }
        //
        //FlagCmd Table
        //
         else if (tabName=="FlagCmdTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("timeInterval",""));
           td.addColumn(ScalarColumnDesc<String> ("type",""));
           td.addColumn(ScalarColumnDesc<String> ("reason",""));
           td.addColumn(ScalarColumnDesc<Int>    ("level",""));
           td.addColumn(ScalarColumnDesc<Int>    ("severity",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("applied",""));
           td.addColumn(ScalarColumnDesc<String> ("command",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           timeInterval.attach(itsTable,"timeInterval");
           type.attach(itsTable,"type");
           reason.attach(itsTable,"reason");
           level.attach(itsTable,"level");
           severity.attach(itsTable,"severity");
           applied.attach(itsTable,"applied");
           command.attach(itsTable,"command");
         }
        //
        //FlagCmd Table
        //
         else if (tabName=="FocusTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("antennaId",""));
           td.addColumn(ScalarColumnDesc<Int>    ("feedId",""));
           td.addColumn(ScalarColumnDesc<String> ("timeInterval",""));
           td.addColumn(ScalarColumnDesc<String> ("focusModelId",""));
           td.addColumn(ScalarColumnDesc<Double> ("xFocusPosition",""));
           td.addColumn(ScalarColumnDesc<Double> ("yFocusPosition",""));
           td.addColumn(ScalarColumnDesc<Double> ("zFocusPosition",""));
           td.addColumn(ScalarColumnDesc<Double> ("focusTracking",""));
           td.addColumn(ScalarColumnDesc<Double> ("xFocusOffset",""));
           td.addColumn(ScalarColumnDesc<Double> ("yFocusOffset",""));
           td.addColumn(ScalarColumnDesc<Double> ("zFocusOffset",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           antennaId.attach(itsTable,"antennaId");
           feedId.attach(itsTable,"feedId");
           timeInterval.attach(itsTable,"timeInterval");
           focusModelId.attach(itsTable,"focusModelId");
           xFocusPosition.attach(itsTable,"xFocusPosition");
           yFocusPosition.attach(itsTable,"yFocusPosition");
           zFocusPosition.attach(itsTable,"zFocusPosition");
           focusTracking.attach(itsTable,"focusTracking");
           xFocusOffset.attach(itsTable,"xFocusOffset");
           yFocusOffset.attach(itsTable,"yFocusOffset");
           zFocusOffset.attach(itsTable,"zFocusOffset");
         }
        //
        //FocusModel Table
        //
         else if (tabName=="FocusModelTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("focusModelId",""));
 

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           focusModelId.attach(itsTable,"focusModelId");
         }
        //
        //FreqOffset Table
        //
         else if (tabName=="FreqOffsetTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("antennaId",""));
           td.addColumn(ScalarColumnDesc<Int>    ("feedId",""));
           td.addColumn(ScalarColumnDesc<String> ("spectralWindowId",""));
           td.addColumn(ScalarColumnDesc<String> ("timeInterval",""));
           td.addColumn(ScalarColumnDesc<Double> ("offset",""));


           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           focusModelId.attach(itsTable,"focusModelId");
           feedId.attach(itsTable,"feedId");
           spectralWindowId.attach(itsTable,"spectralWindowId");
           timeInterval.attach(itsTable,"timeInterval");
           offset.attach(itsTable,"offset");
         }
        //
        //GainTracking Table
        //
         else if (tabName=="GainTrackingTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("antennaId",""));
           td.addColumn(ScalarColumnDesc<Int>    ("feedId",""));
           td.addColumn(ScalarColumnDesc<String> ("spectralWindowId",""));
           td.addColumn(ScalarColumnDesc<String> ("timeInterval",""));
           td.addColumn(ScalarColumnDesc<Double> ("attenuator",""));
           td.addColumn(ScalarColumnDesc<String> ("delayoff1",""));
           td.addColumn(ScalarColumnDesc<String> ("delayoff2",""));
           td.addColumn(ScalarColumnDesc<Double> ("phaseoff1",""));
           td.addColumn(ScalarColumnDesc<Double> ("phaseoff2",""));
           td.addColumn(ScalarColumnDesc<Double> ("rateoff1",""));
           td.addColumn(ScalarColumnDesc<Double> ("rateoff2",""));
           td.addColumn(ScalarColumnDesc<Double> ("samplingLevel",""));
           td.addColumn(ScalarColumnDesc<Double> ("phaseRefOffset",""));


           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           focusModelId.attach(itsTable,"focusModelId");
           feedId.attach(itsTable,"feedId");
           spectralWindowId.attach(itsTable,"spectralWindowId");
           timeInterval.attach(itsTable,"timeInterval");
           attenuator.attach(itsTable,"attenuator");
           delayoff1.attach(itsTable,"delayoff1");
           delayoff2.attach(itsTable,"delayoff2");
           phaseoff1.attach(itsTable,"phaseoff1");
           phaseoff2.attach(itsTable,"phaseoff2");
           rateoff1.attach(itsTable,"rateoff1");
           rateoff2.attach(itsTable,"rateoff2");
           samplingLevel.attach(itsTable,"samplingLevel");
           phaseRefOffset.attach(itsTable,"phaseRefOffset");
         }
        //
        //History Table
        //
         else if (tabName=="HistoryTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("ExecBlockId",""));
           td.addColumn(ScalarColumnDesc<String> ("time",""));
           td.addColumn(ScalarColumnDesc<String> ("message",""));
           td.addColumn(ScalarColumnDesc<String> ("priority",""));
           td.addColumn(ScalarColumnDesc<String> ("origin",""));
           td.addColumn(ScalarColumnDesc<String> ("objectId",""));
           td.addColumn(ScalarColumnDesc<String> ("application",""));
           td.addColumn(ScalarColumnDesc<String> ("cliCommand",""));
           td.addColumn(ScalarColumnDesc<String> ("appParms",""));


           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           focusModelId.attach(itsTable,"focusModelId");
           feedId.attach(itsTable,"feedId");
           spectralWindowId.attach(itsTable,"spectralWindowId");
           timeInterval.attach(itsTable,"timeInterval");
           offset.attach(itsTable,"offset");
         }
        //
        //Holography Table
        //
         else if (tabName=="HolographyTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("holographyId",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numCorr",""));
           td.addColumn(ArrayColumnDesc<String>  ("type",""));
           td.addColumn(ScalarColumnDesc<Double> ("distance",""));
           td.addColumn(ScalarColumnDesc<Double> ("focus",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("flagRow",""));


           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           holographyId.attach(itsTable,"holographyId");
           numCorr.attach(itsTable,"numCorr");
           type.attach(itsTable,"type");
           distance.attach(itsTable,"distance");
           focus.attach(itsTable,"focus");
           flagRow.attach(itsTable,"flagRow");
         }
        //
        //Pointing Table
        //
         else if (tabName=="PointingTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("antennaId",""));
           td.addColumn(ScalarColumnDesc<String> ("timeInterval",""));
           td.addColumn(ScalarColumnDesc<Int>    ("pointingModelId",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numPoly",""));
           td.addColumn(ScalarColumnDesc<String> ("timeOrigin",""));
           td.addColumn(ArrayColumnDesc<Double>  ("pointingDirection",""));
           td.addColumn(ArrayColumnDesc<Double>  ("target",""));
           td.addColumn(ArrayColumnDesc<Double>  ("offset",""));
           td.addColumn(ArrayColumnDesc<Double>  ("encoder",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("pointingTracking",""));
           td.addColumn(ScalarColumnDesc<String> ("name",""));
           td.addColumn(ArrayColumnDesc<Double>  ("sourceOffset",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("phaseTracking",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("overTheTop",""));


           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           antennaId.attach(itsTable,"antennaId");
           timeInterval.attach(itsTable,"timeInterval");
           pointingModelId.attach(itsTable,"pointingModelId");
           numPoly.attach(itsTable,"numPoly");
           timeOrigin.attach(itsTable,"timeOrigin");
           pointingDirection.attach(itsTable,"pointingDirection");
           target.attach(itsTable,"target");
           offset.attach(itsTable,"offset");
           encoder.attach(itsTable,"encoder");
           pointingTracking.attach(itsTable,"pointingTracking");
           name.attach(itsTable,"name");
           sourceOffset.attach(itsTable,"sourceOffset");
           phaseTracking.attach(itsTable,"phaseTracking");
           overTheTop.attach(itsTable,"overTheTop");
         }
        //
        //PointingModel Table
        //
         else if (tabName=="PointingModelTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("antennaId",""));
           td.addColumn(ScalarColumnDesc<Int>    ("pointingModelId",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numCoeff",""));
           td.addColumn(ArrayColumnDesc<String>  ("coeffName",""));
           td.addColumn(ArrayColumnDesc<Double>  ("coeffVal",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numFormula",""));
           td.addColumn(ArrayColumnDesc<String>  ("coeffFormula",""));


           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           antennaId.attach(itsTable,"antennaId");
           pointingModelId.attach(itsTable,"pointingModelId");
           numCoeff.attach(itsTable,"numCoeff");
           coeffName.attach(itsTable,"coeffName");
           coeffVal.attach(itsTable,"coeffVal");
           numFormula.attach(itsTable,"numFormula");
           coeffFormula.attach(itsTable,"coeffFormula");
         }
        //
        //Seeing Table
        //
         else if (tabName=="SeeingTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("timeInterval",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numBaseLength",""));
           td.addColumn(ArrayColumnDesc<Int>     ("baseLength",""));
           td.addColumn(ArrayColumnDesc<Int>     ("phaseRms",""));
           td.addColumn(ScalarColumnDesc<Double> ("seeing",""));
           td.addColumn(ScalarColumnDesc<Double> ("exponent",""));


           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           timeInterval.attach(itsTable,"timeInterval");
           numBaseLength.attach(itsTable,"numBaseLength");
           baseLength.attach(itsTable,"baseLength");
           phaseRmsAUX.attach(itsTable,"phaseRms");
           seeing.attach(itsTable,"seeing");
           exponent.attach(itsTable,"exponent");
         }
        //
        //SourceParameter Table
        //
         else if (tabName=="SourceParameterTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<Int>    ("sourceParameterId",""));
           td.addColumn(ScalarColumnDesc<Int>    ("sourceId",""));
           td.addColumn(ScalarColumnDesc<String> ("timeInterval",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numFreq",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numStokes",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numDep",""));
           td.addColumn(ArrayColumnDesc<Int>     ("stokeParameter",""));
           td.addColumn(ArrayColumnDesc<Double>  ("flux",""));
           td.addColumn(ArrayColumnDesc<Double>  ("frequency",""));
           td.addColumn(ArrayColumnDesc<Double>  ("frequencyInterval",""));
           td.addColumn(ArrayColumnDesc<Double>  ("fluxErr",""));
           td.addColumn(ArrayColumnDesc<Double>  ("size",""));
           td.addColumn(ArrayColumnDesc<Double>  ("positionAngle",""));
           td.addColumn(ArrayColumnDesc<Double>  ("sizeErr",""));
           td.addColumn(ArrayColumnDesc<Double>  ("positionAngleErr",""));
           td.addColumn(ArrayColumnDesc<Int>     ("depSourceParameterId",""));


           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           sourceParameterId.attach(itsTable,"sourceParameterId");
           sourceId.attach(itsTable,"sourceId");
           timeInterval.attach(itsTable,"timeInterval");
           numFreq.attach(itsTable,"numFreq");
           numStokes.attach(itsTable,"numStokes");
           numDep.attach(itsTable,"numDep");
           stokeParameter.attach(itsTable,"stokeParameter");
           flux.attach(itsTable,"flux");
           frequency.attach(itsTable,"frequency");
           frequencyInterval.attach(itsTable,"frequencyInterval");
           fluxErr.attach(itsTable,"fluxErr");
           size.attach(itsTable,"size");
           positionAngle.attach(itsTable,"positionAngle");
           sizeErr.attach(itsTable,"sizeErr");
           positionAngleErr.attach(itsTable,"positionAngleErr");
           depSourceParameterId.attach(itsTable,"depSourceParameterId");
         }
        //
        //SquareLawDetector Table
        //
         else if (tabName=="SquareLawDetectorTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("squareLawDetectorId",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numBand",""));
           td.addColumn(ScalarColumnDesc<String> ("bandType",""));
   
           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           squareLawDetectorId.attach(itsTable,"squareLawDetectorId");
           numBand.attach(itsTable,"numBand");
           bandType.attach(itsTable,"bandType");
         }
        //
        //SysCal Table
        //
         else if (tabName=="SysCalTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("antennaId",""));
           td.addColumn(ScalarColumnDesc<Int>    ("feedId",""));
           td.addColumn(ScalarColumnDesc<String> ("spectralWindowId",""));
           td.addColumn(ScalarColumnDesc<String> ("timeInterval",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numLoad",""));
           td.addColumn(ArrayColumnDesc<Int>     ("calLoad",""));
           td.addColumn(ArrayColumnDesc<Double>  ("feff",""));
           td.addColumn(ArrayColumnDesc<Double>  ("aeff",""));
           td.addColumn(ScalarColumnDesc<Double> ("phaseDiff",""));
           td.addColumn(ScalarColumnDesc<Double> ("sbgain",""));
           td.addColumn(ArrayColumnDesc<Double>  ("tau",""));
           td.addColumn(ArrayColumnDesc<Double>  ("tcal",""));
           td.addColumn(ArrayColumnDesc<Double>  ("trx",""));
           td.addColumn(ArrayColumnDesc<Double>  ("tsky",""));
           td.addColumn(ArrayColumnDesc<Double>  ("tant",""));
           td.addColumn(ArrayColumnDesc<Double>  ("tantTsys",""));
           td.addColumn(ArrayColumnDesc<Double>  ("pwvPath",""));
           td.addColumn(ArrayColumnDesc<Double>  ("dpwvPath",""));
           td.addColumn(ArrayColumnDesc<Double>  ("feffSpectrum",""));
           td.addColumn(ArrayColumnDesc<Double>  ("sbgainSpectrum",""));
           td.addColumn(ArrayColumnDesc<Double>  ("tauSpectrum",""));
           td.addColumn(ArrayColumnDesc<Double>  ("tcalSpectrum",""));
           td.addColumn(ArrayColumnDesc<Double>  ("trxSpectrum",""));
           td.addColumn(ArrayColumnDesc<Double>  ("tskySpectrum",""));
           td.addColumn(ArrayColumnDesc<Double>  ("tsysSpectrum",""));
           td.addColumn(ArrayColumnDesc<Double>  ("tantSpectrum",""));
           td.addColumn(ArrayColumnDesc<Double>  ("tantTsysSpectrum",""));
           td.addColumn(ArrayColumnDesc<Double>  ("pwvPathSpectrum",""));
           td.addColumn(ArrayColumnDesc<Double>  ("dpwvPathSpectrum",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numPoly",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numPolyFreq",""));
           td.addColumn(ScalarColumnDesc<String> ("timeOrigin",""));
           td.addColumn(ScalarColumnDesc<Double> ("freqOrigin",""));
           td.addColumn(ArrayColumnDesc<Double>  ("phaseCurve",""));
           td.addColumn(ArrayColumnDesc<Double>  ("delayCurve",""));
           td.addColumn(ArrayColumnDesc<Double>  ("ampliCurve",""));
           td.addColumn(ArrayColumnDesc<Double>  ("bandpassCurve",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("phasediffFlag",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("sbgainFlag",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("tauFlag",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("tcalFlag",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("trxFlag",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("tskyFlag",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("tsysFlag",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("tantFlag",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("tantTsysFlag",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("pwvPathFlag",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           antennaId.attach(itsTable,"antennaId");
           feedId.attach(itsTable,"feedId");
           spectralWindowId.attach(itsTable,"spectralWindowId");
           timeInterval.attach(itsTable,"timeInterval");
           numLoad.attach(itsTable,"numLoad");
           calLoad.attach(itsTable,"calLoad");
           feff.attach(itsTable,"feff");
           aeff.attach(itsTable,"aeff");
           phaseDiff.attach(itsTable,"phaseDiff");
           sbgain.attach(itsTable,"sbgain");
           tau.attach(itsTable,"tau");
           trx.attach(itsTable,"trx");
           tsky.attach(itsTable,"tsky");
           tsys.attach(itsTable,"tsys");
           tant.attach(itsTable,"tant");
           tantTsys.attach(itsTable,"tantTsys");
           pwvPath.attach(itsTable,"pwvPath");
           dpwvPath.attach(itsTable,"dpwvPath");
           feffSpectrum.attach(itsTable,"feffSpectrum");
           sbgainSpectrum.attach(itsTable,"sbgainSpectrum");
           tauSpectrum.attach(itsTable,"tauSpectrum");
           tcalSpectrum.attach(itsTable,"tcalSpectrum");
           trxSpectrum.attach(itsTable,"trxSpectrum");
           tskySpectrum.attach(itsTable,"tskySpectrum");
           tsysSpectrum.attach(itsTable,"tsysSpectrum");
           tantSpectrum.attach(itsTable,"tantSpectrum");
           tantTsysSpectrum.attach(itsTable,"tantTsysSpectrum");
           pwvPathSpectrum.attach(itsTable,"pwvPathSpectrum");
           dpwvPathSpectrum.attach(itsTable,"dpwvPathSpectrum");
           numPoly.attach(itsTable,"numPoly");
           numPolyFreq.attach(itsTable,"numPolyFreq");
           timeOrigin.attach(itsTable,"timeOrigin");
           freqOrigin.attach(itsTable,"freqOrigin");
           phaseCurve.attach(itsTable,"phaseCurve");
           delayCurve.attach(itsTable,"delayCurve");
           ampliCurve.attach(itsTable,"ampliCurve");
           bandpassCurve.attach(itsTable,"bandpassCurve");
           phasediffFlag.attach(itsTable,"phasediffFlag");
           sbgainFlag.attach(itsTable,"sbgainFlag");
           tauFlag.attach(itsTable,"tauFlag");
           tcalFlag.attach(itsTable,"tcalFlag");
           trxFlag.attach(itsTable,"trxFlag");
           tskyFlag.attach(itsTable,"tskyFlag");
           tsysFlag.attach(itsTable,"tsysFlag");
           tantFlag.attach(itsTable,"tantFlag");
           tantTsysFlag.attach(itsTable,"tantTsysFlag");
           pwvPathFlag.attach(itsTable,"pwvPathFlag");
         }
        //
        //TotalPower Table
        //
         else if (tabName=="TotalPowerTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("configDescriptionId",""));
           td.addColumn(ScalarColumnDesc<String> ("fieldId",""));
           td.addColumn(ScalarColumnDesc<String> ("time",""));
           td.addColumn(ScalarColumnDesc<String> ("execBlockId",""));
           td.addColumn(ArrayColumnDesc<String>  ("stateId",""));
           td.addColumn(ScalarColumnDesc<Int>    ("scanNumber",""));
           td.addColumn(ScalarColumnDesc<Int>    ("subscanNumber",""));
           td.addColumn(ScalarColumnDesc<Int>    ("integrationNumber",""));
           td.addColumn(ArrayColumnDesc<Double>  ("uvw",""));
           td.addColumn(ArrayColumnDesc<Double>  ("exposure",""));
           td.addColumn(ArrayColumnDesc<String>  ("timeCentroid",""));
           td.addColumn(ArrayColumnDesc<Double>  ("floatData",""));
           td.addColumn(ArrayColumnDesc<Int>     ("flagAnt",""));
           td.addColumn(ArrayColumnDesc<Int>     ("flagPol",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("flagRow",""));
           td.addColumn(ScalarColumnDesc<String> ("interval",""));
           td.addColumn(ScalarColumnDesc<Int>    ("subintegrationNumber",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           configDescriptionId.attach(itsTable,"configDescriptionId");
           fieldId.attach(itsTable,"fieldId");
           time.attach(itsTable,"time");
           execBlockId.attach(itsTable,"execBlockId");
           stateId.attach(itsTable,"stateId");
           scanNumber.attach(itsTable,"scanNumber");
           subscanNumber.attach(itsTable,"subscanNumber");
           integrationNumber.attach(itsTable,"integrationNumber");
           uvw.attach(itsTable,"uvw");
           exposure.attach(itsTable,"exposure");
           timeCentroid.attach(itsTable,"timeCentroid");
           floatData.attach(itsTable,"floatData");
           flagAnt.attach(itsTable,"flagAnt");
           flagPol.attach(itsTable,"flagPol");
           flagRow.attach(itsTable,"flagRow");
           interval.attach(itsTable,"interval");
           subintegrationNumber.attach(itsTable,"subintegrationNumber");
         }
        //
        //WVMCal Table
        //
         else if (tabName=="WVMCalTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("antennaId",""));
           td.addColumn(ScalarColumnDesc<String> ("spectralWindowId",""));
           td.addColumn(ScalarColumnDesc<String> ("timeInterval",""));
           td.addColumn(ScalarColumnDesc<Int>    ("numPoly",""));
           td.addColumn(ScalarColumnDesc<Double> ("freqOrigin",""));
           td.addColumn(ArrayColumnDesc<Double>  ("pathCoeff",""));
           td.addColumn(ScalarColumnDesc<String> ("calibrationMode",""));
           td.addColumn(ScalarColumnDesc<String> ("operationMode",""));
           td.addColumn(ScalarColumnDesc<Double> ("wvrefModel",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           antennaId.attach(itsTable,"antennaId");
           spectralWindowId.attach(itsTable,"spectralWindowId");
           timeInterval.attach(itsTable,"timeInterval");
           numPoly.attach(itsTable,"numPoly");
           freqOrigin.attach(itsTable,"freqOrigin");
           pathCoeff.attach(itsTable,"pathCoeff");
           calibrationMode.attach(itsTable,"calibrationMode");
           operationMode.attach(itsTable,"operationMode");
           wvrefModel.attach(itsTable,"wvrefModel");
         }
        //
        //Weather Table
        //
         else if (tabName=="WeatherTable") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("stationId",""));
           td.addColumn(ScalarColumnDesc<String> ("timeInterval",""));
           td.addColumn(ScalarColumnDesc<Double> ("pressure",""));
           td.addColumn(ScalarColumnDesc<Double> ("relHumidity",""));
           td.addColumn(ScalarColumnDesc<Double> ("temperature",""));
           td.addColumn(ScalarColumnDesc<Double> ("windDirection",""));
           td.addColumn(ScalarColumnDesc<Double> ("windSpeed",""));
           td.addColumn(ScalarColumnDesc<Double> ("windMax",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("pressureFlag",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("relHumidityFlag",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("temperatureFlag",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("windDirectionFlag",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("windSpeedFlag",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("windMaxFlag",""));
           td.addColumn(ScalarColumnDesc<Double> ("dewPoint",""));
           td.addColumn(ScalarColumnDesc<Bool>   ("dewPointFlag",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           stationId.attach(itsTable,"stationId");
           timeInterval.attach(itsTable,"timeInterval");
           pressure.attach(itsTable,"pressure");
           relHumidity.attach(itsTable,"relHumidity");
           temperature.attach(itsTable,"temperature");
           windDirection.attach(itsTable,"windDirection");
           windSpeed.attach(itsTable,"windSpeed");
           windMax.attach(itsTable,"windMax");
           pressureFlag.attach(itsTable,"pressureFlag");
           relHumidityFlag.attach(itsTable,"relHumidityFlag");
           temperatureFlag.attach(itsTable,"temperatureFlag");
           windDirectionFlag.attach(itsTable,"windDirectionFlag");
           windSpeedFlag.attach(itsTable,"windSpeedFlag");
           windMaxFlag.attach(itsTable,"windMaxFlag");
           dewPoint.attach(itsTable,"dewPoint");
           dewPointFlag.attach(itsTable,"dewPointFlag");
         }
        //
        //ASDM Table
        //
         else if (tabName=="ASDM") {

           TableDesc td(tName,"1",TableDesc::Scratch);
           td.addColumn(ScalarColumnDesc<String> ("Name",""));
           td.addColumn(ScalarColumnDesc<Int>    ("NumberRows",""));

           SetupNewTable newtab(tName, td, Table::New);
           StandardStMan stmanStand_1;
           newtab.bindAll (stmanStand_1);

           Table tab(newtab);
           itsTable=tab;

           Name.attach(itsTable,"Name");
           NumberRows.attach(itsTable,"NumberRows");
         }
 

    
 
}

void asdmCasaSaxHandler::startElement(const XMLCh *const uri,
		                  const XMLCh * const localname,
				  const XMLCh * const qname,
				  const Attributes & attrs)
{
	char *tagName = XMLString::transcode(qname);
	String eName(tagName);
	//
	//The if is necessary because all the tables are not yet implemented
	//but maybe a try/catch exception statement may be used instead
	//
	if((eName=="ScanTable")||(eName == "MainTable")||
          (eName=="AlmaCorrelatorModeTable")||(eName=="AntennaTable")||
          (eName=="ConfigDescriptionTable")||(eName=="DataDescriptionTable")||
          (eName=="ExecBlockTable")||(eName=="FeedTable")||
          (eName=="FieldTable")||(eName=="PolarizationTable")||
          (eName=="ProcessorTable")||(eName=="ReceiverTable")||
          (eName=="SBSummaryTable")||(eName=="SourceTable")||
          (eName=="SpectralWindowTable")||(eName=="StateTable")||
          (eName=="StationTable")||(eName=="SubscanTable")||
	  (eName=="SwitchCycleTable")||(eName=="CalCurveTable")||
	  (eName=="CalDataTable")||(eName=="CalPhaseTable")||
	  (eName=="CalReductionTable")||(eName=="CalSeeingTable")||
          (eName=="BeamTable")||(eName=="CalAtmosphere")||
          (eName=="CalAmpiTable")||(eName=="CalBandpassTable")||
          (eName=="AlmaRadiometerTable")||(eName=="CalDelayTable")||
	  (eName=="CalDeviceTable")||(eName=="CalFluxTable")||
          (eName=="CalFocusTable")||(eName=="CalFocusModelTable")||
          (eName=="CalGainTable")||(eName=="CalHolographyTable")||
          (eName=="CalPointingTable")||(eName=="CalPointingModelTable")||
          (eName=="CalPositionTable")||(eName=="CalPrimaryBeam")||
	  (eName=="CalWVRTable")||(eName=="DopplerTable")||
          (eName=="EphemerisTable")||(eName=="ObservationTable")||
          (eName=="FlagCmdTable")||(eName=="FocusModelTable")||
          (eName=="FocusModelIdTable")||(eName=="FreqOffsetTable")||
          (eName=="GainTrackingTable")||(eName=="HistoryTable")||
          (eName=="HolographyTable")||(eName=="pointingTable")||
          (eName=="PointingModelTable")||(eName=="SourceParameterTable")||
          (eName=="SquareLawDetectorTable")||(eName=="SysCalTable")||
          (eName=="TotalPowerTable")||(eName=="WVMCalTable")||
          (eName=="WeatherTable")||(eName=="ASDM")){
                xmlTableName=eName;
		setDescriptor(eName);
                std::string str;
                std::ifstream in; 
                in.open("taglist.txt");  
                getline(in,str);
                tagList=str;

        //When encounters a <row> I'll keep counting the current row number
        //which can then be input to the root node

 
	} else if((eName == String("row")||(eName=="Table"))){
                isRow=true;
                itsTable.addRow();
        //In principle, when I find <tag>, e.g. <execBlockId>
        //I only have to fill itsElement, all the work is done
        //inside characters, so an else here should suffice. 
        //What about other tags?. I have to think about it.


	} else if( !(tagList.find(eName) == std::string::npos)){
                itsElement=eName;
		flagIn=true;
		cout << endl << "in: " << eName << endl;
	} else if (eName == "EntityRef") {

        //These xml arguments seem to be standard for all EntityRef tags

		XMLCh *xmlId = XMLString::transcode("entityId");
		XMLCh *xmlPartId = XMLString::transcode("partId");
		XMLCh *xmlTypeName = XMLString::transcode("entityTypeName");
		XMLCh *xmlDocumentVersion = XMLString::transcode("documentVersion");
		char  *pId=XMLString::transcode(attrs.getValue(xmlId));
		char  *pPartId=XMLString::transcode(attrs.getValue(xmlPartId));
		char  *pTypeName=XMLString::transcode(attrs.getValue(xmlTypeName));
		char  *pDocumentVersion=XMLString::transcode(attrs.getValue(xmlDocumentVersion));
		String value=String(pId)+"   "+String(pPartId)+"   "+
                             String(pTypeName)+"   "+String(pDocumentVersion);
                if (currentElement=="dataOid") 
		    dataOid.put(nRow,value);
		else if (currentElement=="projectId")
		    projectId.put(nRow,value);
		else if (currentElement=="execBlockUID")
		    execBlockUID.put(nRow,value);
		else if (currentElement=="sbId")
		    sbId.put(nRow,value);
		else if (currentElement=="obsUnitSetId")
		    obsUnitSetId.put(nRow,value);

                itsEntityRef=false;
	        XMLString::release(&xmlId);
	        XMLString::release(&xmlPartId);
	        XMLString::release(&xmlTypeName);
	        XMLString::release(&xmlDocumentVersion);
        } 
	XMLString::release(&tagName);
}
//
//
//
void asdmCasaSaxHandler::endElement(const XMLCh *const uri,
		                  const XMLCh * const localname,
				  const XMLCh * const qname)
{
	char *tagName = XMLString::transcode(localname);
	String eName(tagName);

        //Here I will look for the end of the row
        //is at the end of the row where I can add 
        //to the row number counter.


        //First if block look for the tableName
	//In order to build de descriptor. In principle
        //a descriptor is not necessary to build the
        //Record, but it is a good idea from a structural
        //point of view. Also, it might make the code
        //more readable and easier to maintain

	if((eName=="ScanTable")||(eName=="MainTable")||
          (eName=="AlmaCorrelatorModeTable")||(eName=="AntennaTable")||
          (eName=="ConfigDescriptionTable")||(eName=="DataDescriptionTable")||
          (eName=="ExecBlockTable")||(eName=="FeedTable")||
          (eName=="FieldTable")||(eName=="PolarizationTable")||
          (eName=="ProcessorTable")||(eName=="ReceiverTable")||
          (eName=="SBSummaryTable")||(eName=="SourceTable")||
          (eName=="SpectralWindowTable")||(eName=="StateTable")||
          (eName=="StationTable")||(eName=="SubscanTable")||
	  (eName=="SwitchCycleTable")||(eName=="CalCurveTable")||
	  (eName=="CalDataTable")||(eName=="CalPhaseTable")||
	  (eName=="CalReductionTable")||(eName=="CalSeeingTable")||
          (eName=="BeamTable")||(eName=="CalAtmosphere")||
          (eName=="CalAmpiTable")||(eName=="CalBandpassTable")||
          (eName=="AlmaRadiometerTable")||(eName=="CalDelayTable")||
	  (eName=="CalDeviceTable")||(eName=="CalFluxTable")||
          (eName=="CalFocusTable")||(eName=="CalFocusModelTable")||
          (eName=="CalGainTable")||(eName=="CalHolographyTable")||
          (eName=="CalPointingTable")||(eName=="CalPointingModelTable")||
          (eName=="CalPositionTable")||(eName=="CalPrimaryBeam")||
	  (eName=="CalWVRTable")||(eName=="DopplerTable")||
          (eName=="EphemerisTable")||(eName=="ObservationTable")||
          (eName=="FlagCmdTable")||(eName=="FocusModelTable")||
          (eName=="FocusModelIdTable")||(eName=="FreqOffsetTable")||
          (eName=="GainTrackingTable")||(eName=="HistoryTable")||
          (eName=="HolographyTable")||(eName=="pointingTable")||
          (eName=="PointingModelTable")||(eName=="SourceParameterTable")||
          (eName=="SquareLawDetectorTable")||(eName=="SysCalTable")||
          (eName=="TotalPowerTable")||(eName=="WVMCalTable")||
          (eName=="WeatherTable")||(eName=="ASDM")){
		flagIn=false;
		cout << "Done with XML" << endl;
        } else if((eName == "row")||(eName=="Table")){
                ++nRow;
		isRow=false;
        }else if( !(tagList.find(eName) == std::string::npos)){
                flagIn=false;
        }
 
	XMLString::release(&tagName);

}


//This function makes most of the parsing. Because the tagnames are unique
//and have to be defined uniquely as tables columns, they have to be
//process independetly => a bif if-else block
void asdmCasaSaxHandler::characters (const XMLCh *const chars, 
		                 const unsigned int length){
	char *xmlValue = XMLString::transcode(chars);
	String value(xmlValue);
	if (flagIn) {

	  //Remember that the events where triggered more than once
	  //see old code

	  //
	  // The following block managed the values from the Scan table
	  //

	  //
	  //Scan Table
	  //
	  if (itsElement == "execBlockId")
  	  {
		execBlockId.put(nRow,value);
	  } else if (itsElement == String("scanNumber")) {
		scanNumber.put(nRow,atoi(xmlValue));
	  } else if (itsElement == "startTime") {
		startTime.put(nRow,value);
	  } else if (itsElement == "endTime") {
		endTime.put(nRow,value);
	  } else if (itsElement == "numSubScan") {
		numSubScan.put(nRow,atoi(xmlValue));
	  } else if (itsElement == "numIntent") {
		numIntent.put(nRow,atoi(xmlValue));
	  } else if (itsElement == "numField") {
		numField.put(nRow,atoi(xmlValue));
	  } else if (itsElement == "scanIntent") {
		Array<String> temp;
		assignColumn(xmlValue,temp);
		scanIntent.put(nRow,temp);
	  } else if (itsElement == "flagRow") {
		if (value == "true") 
		    flagRow.put(nRow,true);
		else
		    flagRow.put(nRow,false);
	  } else if (itsElement == "sourceName") {
		 if (xmlTableName=="CalDataTable"){
		   Array<String> temp;
		   assignColumn(xmlValue,temp);
		   sourceNameAUX.put(nRow,temp);
		 }
                 else if ((xmlTableName=="CalFluxTable")||
                          (xmlTableName=="ScanTable")||
                          (xmlTableName=="SourceTable")){
		   sourceName.put(nRow,value);
                 }
	  } else if (itsElement == "fieldName") {
                 if (xmlTableName=="ScanTable") {
		   Array<String> temp;
		   assignColumn(xmlValue,temp);
		   fieldName.put(nRow,temp);
                 }
                 else if ((xmlTableName=="FieldTable")||
			  (xmlTableName=="SubscanTable")||
			  (xmlTableName=="CalDataTable"))
		   fieldNameAUX.put(nRow,value);
	  //
          //
          } else if(itsElement=="configDescriptionId"){
		configDescriptionId.put(nRow,value);
          } else if(itsElement=="fieldId"){
                fieldId.put(nRow,value);
          } else if (itsElement == "time") {
		time.put(nRow,value);
          } else if (itsElement == "stateId") {
		  if (xmlTableName=="StateTable")
			stateIdAUX.put(nRow,value);
		  else{
			Array<String> temp;
			assignColumn(xmlValue,temp);
		        stateId.put(nRow,temp);
		  }
          } else if(itsElement=="subscanNumber"){
		subscanNumber.put(nRow,atoi(xmlValue));
          } else if(itsElement=="integrationNumber"){
		integrationNumber.put(nRow,atoi(xmlValue));
          } else if(itsElement=="subintegrationNumber"){
		subintegrationNumber.put(nRow,atoi(xmlValue));
          } else if (itsElement == "uvw") {
                Array<Double> temp;
		assignColumn(xmlValue,temp);
                uvw.put(nRow,temp);
          } else if (itsElement == "exposure") {
                Array<String> temp;
		assignColumn(xmlValue,temp);
                exposure.put(nRow,temp);
          } else if (itsElement == "timeCentroid") {
                Array<String> temp;
		assignColumn(xmlValue,temp);
                timeCentroid.put(nRow,temp);
          } else if (itsElement == "dataOid") {
		currentElement=itsElement;
                itsEntityRef=true;
          } else if (itsElement == "flagAnt") {
		if (xmlTableName=="MainTable"){
		    Array<Int> temp;
		    assignColumn(xmlValue,temp);
		    flagAnt.put(nRow,temp);
                }
		else if (xmlTableName=="ConfigDescriptionTable"){
		    Array<Bool> temp;
		    assignColumn(xmlValue,temp);
		    flagAntAUX.put(nRow,temp);
                }
          } else if (itsElement == "flagPol") {
		Array<Int> temp;
		assignColumn(xmlValue,temp);
		flagPol.put(nRow,temp);
          } else if (itsElement == "flagBaseband") {
                Array<Int> temp;
		assignColumn(xmlValue,temp);
		flagBaseband.put(nRow,temp);
          } else if (itsElement == "interval") {
		interval.put(nRow,value);
          } else if(itsElement=="subintegrationNumber"){
		subintegrationNumber.put(nRow,atoi(xmlValue));
          }
          //
	  //AlmaCorrelatorMode Table tags
	  //
	    else if(itsElement=="almaCorrelatorModeId") {
		almaCorrelatorModeId.put(nRow,value);
          } else if(itsElement=="numBaseband"){
		numBaseband.put(nRow,atoi(xmlValue));
 	  } else if (itsElement == "basebandIndex") {
		Array<Int> temp;
		assignColumn(xmlValue,temp);
                basebandIndex.put(nRow,temp);
	  } else if(itsElement=="accumMode") {
		accumMode.put(nRow,value);
 	  } else if (itsElement == "basebandConfig") {
		Array<Int> temp;
		assignColumn(xmlValue,temp);
                basebandConfig.put(nRow,temp);
          } else if(itsElement=="binMode"){
		binMode.put(nRow,atoi(xmlValue));
          } else if(itsElement=="quantization"){
		if (value == "true") 
		    quantization.put(nRow,true);
		else
		    quantization.put(nRow,false);
          } else if(itsElement=="windowFunction"){
		windowFunction.put(nRow,value);
 	  } else if (itsElement == "axesOrderArray") {
		Array<Int> temp;
		assignColumn(xmlValue,temp);
                axesOrderArray.put(nRow,temp);
	  }
          //
	  //Antenna Table tags
	  //
            else if(itsElement=="antennaId"){
		if ((xmlTableName=="AntennaTable")||
                    (xmlTableName=="FeedTable")){
		    antennaId.put(nRow,value);
                } 
                else if ((xmlTableName=="ConfigDescriptionTable")||
                         (xmlTableName=="ExecBlockTable")){
		    Array<String> temp;
		    assignColumn(xmlValue,temp);
                    antennaIdAUX.put(nRow,temp);
                }

          } else if(itsElement=="stationId"){
		stationId.put(nRow,value);
          } else if(itsElement=="name"){
		name.put(nRow,value);
          } else if(itsElement=="type"){
		type.put(nRow,value);
          } else if(itsElement=="xPosition"){
		xPosition.put(nRow,atof(xmlValue));
          } else if(itsElement=="yPosition"){
		yPosition.put(nRow,atof(xmlValue));
          } else if(itsElement=="zPosition"){
		zPosition.put(nRow,atof(xmlValue));
          } else if(itsElement=="xOffset"){
		xOffset.put(nRow,atof(xmlValue));
          } else if(itsElement=="yOffset"){
		yOffset.put(nRow,atof(xmlValue));
          } else if(itsElement=="zOffset"){
		zOffset.put(nRow,atof(xmlValue));
          } else if(itsElement=="dishDiameter"){
		dishDiameter.put(nRow,atof(xmlValue));
          } else if(itsElement=="assocAntennaId"){
		assocAntennaId.put(nRow,value);
          } 
	  //
	  //ConfigDescription Table
	  //
            else if(itsElement=="dataDescriptionId"){
              if (xmlTableName=="ConfigDescriptionTable"){
		Array<String> temp;
		assignColumn(xmlValue,temp);
                dataDescriptionId.put(nRow,temp);
              } else if(xmlTableName=="DataDescriptionTable"){
		dataDescriptionIdDDT.put(nRow,value);
              }
          } else if(itsElement=="feedId"){
              if (xmlTableName=="ConfigDescriptionTable"){
		Array<Int> temp;
		assignColumn(xmlValue,temp);
                feedId.put(nRow,temp);
	      } else if(xmlTableName=="FeedIdTable"){
                feedIdFT.put(nRow,atoi(xmlValue));
              }

          } else if(itsElement=="processorId"){
		processorId.put(nRow,value);
          } else if(itsElement=="switchCycleId"){
		   if(xmlTableName=="SwitchCycleTable")
                      switchCycleIdAUX.put(nRow,value);
		   else if (xmlTableName=="ConfigDescriptionTable"){
		      Array<String> temp;
		      assignColumn(xmlValue,temp);
                      switchCycleId.put(nRow,temp);
                   }
          } else if(itsElement=="numAntenna"){
		numAntenna.put(nRow,atoi(xmlValue));
          } else if(itsElement=="numFeed"){
		numFeed.put(nRow,atoi(xmlValue));
          } else if(itsElement=="numSubBand"){
		Array<Int> temp;
		assignColumn(xmlValue,temp);
                numSubBand.put(nRow,temp);
          } else if(itsElement=="correlationMode"){
		correlationMode.put(nRow,atoi(xmlValue));
          } else if(itsElement=="atmPhaseCode"){
		atmPhaseCode.put(nRow,atoi(xmlValue));
          } else if(itsElement=="phasedArrayList"){
		Array<Int> temp;
		assignColumn(xmlValue,temp);
                phasedArrayList.put(nRow,temp);
          }
	  //
	  //DataDescription Table
          //
            else if(itsElement=="polOrHoloId"){
		polOrHoloId.put(nRow,value);
          } else if(itsElement=="spectralWindowId"){
		spectralWindowId.put(nRow,value);
          }
	  //
          //ExecBlock Table
          //
            else if(itsElement=="telescopeName"){
		telescopeName.put(nRow,value);
          } else if(itsElement=="configName"){
		configName.put(nRow,value);
          } else if(itsElement=="baseRangeMin"){
		baseRangeMin.put(nRow,atof(xmlValue));
          } else if(itsElement=="baseRangeMin"){
		baseRangeMin.put(nRow,atof(xmlValue));
          } else if(itsElement=="baseRangeMax"){
		baseRangeMax.put(nRow,atof(xmlValue));
          } else if(itsElement=="baseRangeMinor"){
		baseRangeMinor.put(nRow,atof(xmlValue));
          } else if(itsElement=="baseRangeMajor"){
		baseRangeMajor.put(nRow,atof(xmlValue));
          } else if(itsElement=="basePa"){
		basePa.put(nRow,atof(xmlValue));
          } else if(itsElement=="timeInterval"){
                timeInterval.put(nRow,value);
          } else if(itsElement=="observerName"){
		observerName.put(nRow,value);
          } else if(itsElement=="observingLog"){
		Array<String> temp;
		assignColumn(xmlValue,temp);
		observingLog.put(nRow,temp);
          } else if(itsElement=="schedulerMode"){
		Array<String> temp;
		assignColumn(xmlValue,temp);
                schedulerMode.put(nRow,temp);
          } else if(itsElement=="projectId"){
                currentElement=itsElement;
                itsEntityRef=true;
          } else if(itsElement=="siteLongitude"){
		siteLongitude.put(nRow,atof(xmlValue));
          } else if(itsElement=="siteLatitude"){
		siteLatitude.put(nRow,atof(xmlValue));
          } else if(itsElement=="siteAltitude"){
		siteAltitude.put(nRow,atof(xmlValue));
          } else if(itsElement=="execBlockUID"){
                currentElement=itsElement;
                itsEntityRef=true;
          } else if(itsElement=="aborted"){
		if (value == "true") 
		    aborted.put(nRow,true);
		else
		    aborted.put(nRow,false);
          }
	  //
	  //Feed Table
	  //
	    else if (itsElement=="receiverId"){
		    if (xmlTableName=="ReceiverTable")
			receiverIdAUX.put(nRow,atoi(xmlValue));
		    else if(xmlTableName=="FeedTable"){
			Array<Int> temp;
			assignColumn(xmlValue,temp);
		        receiverId.put(nRow,temp);
		    }
          } else if (itsElement=="numReceptors"){
		numReceptors.put(nRow,atoi(xmlValue));
		cout << "Done: " << itsElement << endl;
          } else if (itsElement=="beamOffset"){
		Array<Double> temp;
		assignColumn(xmlValue,temp);
		beamOffset.put(nRow,temp);
          } else if (itsElement=="focusReference"){
		Array<Double> temp;
		assignColumn(xmlValue,temp);
		focusReference.put(nRow,temp);
          } else if (itsElement=="polarizationType"){
		Array<String> temp;
		assignColumn(xmlValue,temp);
		polarizationType.put(nRow,temp);
          //
	  //It is not clear to me how to store the complex numbers
	  //from the table, it will be (a+ib), (a,b), or (aexp(ib))?
          //You need to number to construct the complex datatype, often
          //there is an odd number fo values in the table, so I'll use
          //double from now.
          } else if (itsElement=="polResponse"){
		Array<Complex> temp;
		assignColumn(xmlValue,temp);
		polResponse.put(nRow,temp);
          } else if (itsElement=="receptorAngle"){
		Array<Double> temp;
		assignColumn(xmlValue,temp);
		receptorAngle.put(nRow,temp);
          } else if (itsElement=="beamId"){
		Array<String> temp;
		assignColumn(xmlValue,temp);
		beamId.put(nRow,temp);
          } else if (itsElement=="feedNum"){
		feedNum.put(nRow,atoi(xmlValue));
          } else if (itsElement=="illumOffset"){
		illumOffset.put(nRow,atof(xmlValue));
          } else if (itsElement=="illumOffsetPa"){
		illumOffsetPa.put(nRow,atof(xmlValue));
          } else if (itsElement=="xPosition"){
		xPosition.put(nRow,atof(xmlValue));
          } else if (itsElement=="yPosition"){
		yPosition.put(nRow,atof(xmlValue));
          } else if (itsElement=="zPosition"){
		zPosition.put(nRow,atof(xmlValue));
	  } 
	  //
	  //Field Table
	  //
            else if (itsElement=="code"){
		code.put(nRow,value);
          } else if (itsElement=="numPoly"){
		numPoly.put(nRow,atoi(xmlValue));
		cout << "Done: " << itsElement << endl;
          } else if (itsElement=="delayDir"){
		Array<Double> temp;
		assignColumn(xmlValue,temp);
		delayDir.put(nRow,temp);
          } else if (itsElement=="phaseDir"){
		Array<Double> temp;
		assignColumn(xmlValue,temp);
		phaseDir.put(nRow,temp);
          } else if (itsElement=="referenceDir"){
		Array<Double> temp;
		assignColumn(xmlValue,temp);
		referenceDir.put(nRow,temp);
          } else if (itsElement=="assocFieldId"){
		Array<String> temp;
		assignColumn(xmlValue,temp);
		assocFieldId.put(nRow,temp);
          } else if (itsElement=="ephemerisId"){
		ephemerisId.put(nRow,value);
          } else if (itsElement=="sourceId"){
		sourceId.put(nRow,atoi(xmlValue));
          } else if (itsElement=="assocNature"){
                if (xmlTableName=="SpectralWindow"){
		    Array<String> temp;
	            assignColumn(xmlValue,temp);
		    assocNatureAUX.put(nRow,temp);
                }
                else
		    assocNature.put(nRow,value);
          }
          //
	  //Polarization Table tags
	  //
            else if (itsElement=="polarizationId"){
		polarizationId.put(nRow,value);
          } else if (itsElement=="numCorr"){
		numCorr.put(nRow,atoi(xmlValue));
          } else if (itsElement=="corrType"){
		Array<Int> temp;
		assignColumn(xmlValue,temp);
		corrType.put(nRow,temp);
          } else if (itsElement=="corrProduct"){
		Array<Int> temp;
		assignColumn(xmlValue,temp);
		corrProduct.put(nRow,temp);
          }
	  //
	  //Processor Table
          //
            else if (itsElement=="subType"){
		subType.put(nRow,value);
          }
	  //
	  //Receiver Table
	  //
            else if (itsElement=="numLo"){
		numLo.put(nRow,atoi(xmlValue));
          } else if (itsElement=="frequencyBand"){
		frequencyBand.put(nRow,value);
          } else if (itsElement=="freqLo"){
		Array<Double> temp;
		assignColumn(xmlValue,temp);
		freqLo.put(nRow,temp);
          } else if (itsElement=="sidebandLo"){
		Array<Int> temp;
		assignColumn(xmlValue,temp);
		sidebandLo.put(nRow,temp);
          } else if (itsElement=="tDewar"){
		tDewar.put(nRow,atof(xmlValue));
          } else if (itsElement=="stabilityDuration"){
		stabilityDuration.put(nRow,value);
          } else if (itsElement=="stability"){
		stability.put(nRow,atof(xmlValue));
          } else if (itsElement=="dewarName"){
		dewarName.put(nRow,value);
	  } else if (itsElement == "stabilityflag") {
		if (value == "true") 
		    stabilityflag.put(nRow,true);
		else
		    stabilityflag.put(nRow,false);
          }
	//
	//SBSummary Table
	//
            else if (itsElement=="sbId"){
                currentElement=itsElement;
                itsEntityRef=true;
          } else if (itsElement == "obsUnitSetId") {
                currentElement=itsElement;
                itsEntityRef=true;
          } else if (itsElement == "sbIntent") {
                sbIntent.put(nRow,value); 
          } else if (itsElement == "sbType") {
                sbType.put(nRow,value); 
          } else if (itsElement == "sbDuration") {
                sbDuration.put(nRow,value); 
          } else if (itsElement == "numScan") {
                numScan.put(nRow,atoi(xmlValue)); 
          } else if (itsElement == "numberRepeats") {
                numberRepeats.put(nRow,atoi(xmlValue)); 
          } else if (itsElement == "weatherConstraint") {
		Array<String> temp;
		assignColumn(xmlValue,temp);
                weatherConstraint.put(nRow,temp); 
          } else if (itsElement == "scienceGoal") {
		Array<String> temp;
		assignColumn(xmlValue,temp);
                scienceGoal.put(nRow,temp); 
          } else if (itsElement == "raCenter") {
                raCenter.put(nRow,atof(xmlValue)); 
          } else if (itsElement == "decCenter") {
                decCenter.put(nRow,atof(xmlValue)); 
          } else if (itsElement == "frequency") {
	          if (xmlTableName=="CalFluxTable"){
		     Array<Double> temp;
		     assignColumn(xmlValue,temp);
                     frequencyAUX.put(nRow,temp); 
                  } else if (xmlTableName=="SBSummaryTable"){
                     frequency.put(nRow,atof(xmlValue)); 
                  }
          } else if (itsElement == "frequencyBand") {
                frequencyBand.put(nRow,value); 
          } else if (itsElement == "observingMode") {
		Array<String> temp;
		assignColumn(xmlValue,temp);
                observingMode.put(nRow,temp); 
          }
	//
	//Source Table
	//
            else if (itsElement == "numLines") {
                numLines.put(nRow,atoi(xmlValue)); 
          } else if (itsElement == "direction") {
		Array<Double> temp;
		assignColumn(xmlValue,temp);
                direction.put(nRow,temp); 
          } else if (itsElement == "properMotion") {
		Array<Double> temp;
		assignColumn(xmlValue,temp);
                properMotion.put(nRow,temp); 
          } else if (itsElement == "sourceParameterId") {
                sourceParameterId.put(nRow,atoi(xmlValue)); 
          } else if (itsElement == "catalog") {
                catalog.put(nRow,value); 
          } else if (itsElement == "calibrationGroup") {
                calibrationGroup.put(nRow,atoi(xmlValue)); 
          } else if (itsElement == "position") {
		Array<Double> temp;
		assignColumn(xmlValue,temp);
                position.put(nRow,temp); 
          } else if (itsElement == "transition") {
		Array<String> temp;
		assignColumn(xmlValue,temp);
                transition.put(nRow,temp); 
          } else if (itsElement == "restFrequency") {
		Array<Double> temp;
		assignColumn(xmlValue,temp);
                restFrequency.put(nRow,temp); 
          } else if (itsElement == "sysVel") {
                sysVel.put(nRow,sSplit_d(xmlValue)); 
          } else if (itsElement == "sourceModel") {
                sourceModel.put(nRow,value); 
          } else if (itsElement == "deltaVel") {
                deltaVel.put(nRow,atof(xmlValue)); 
          } else if (itsElement == "rangeVel") {
		Array<Double> temp;
		assignColumn(xmlValue,temp);
                rangeVel.put(nRow,temp); 
          }
	//
	//SpectralWindow Table
	//
            else if (itsElement == "numChan") {
                numChan.put(nRow,atoi(xmlValue)); 
          } else if (itsElement == "refFreq") {
                refFreq.put(nRow,atof(xmlValue)); 
          } else if (itsElement == "chanFreq") {
		Array<Double> temp;
		assignColumn(xmlValue,temp);
                chanFreq.put(nRow,temp); 
          } else if (itsElement == "chanWidth") {
		Array<Double> temp;
		assignColumn(xmlValue,temp);
                chanWidth.put(nRow,temp); 
          } else if (itsElement == "effectiveBw") {
		Array<Double> temp;
		assignColumn(xmlValue,temp);
                effectiveBw.put(nRow,temp); 
          } else if (itsElement == "resolution") {
		Array<Double> temp;
		assignColumn(xmlValue,temp);
                resolution.put(nRow,temp); 
          } else if (itsElement == "totBandwidth") {
                totBandwidth.put(nRow,atof(xmlValue)); 
          } else if (itsElement == "netSideband") {
                totBandwidth.put(nRow,atoi(xmlValue)); 
          } else if (itsElement == "assocSpectralWindowId") {
		Array<String> temp;
		assignColumn(xmlValue,temp);
                assocSpectralWindowId.put(nRow,temp); 
          } else if (itsElement == "dopplerId") {
                dopplerId.put(nRow,atoi(xmlValue)); 
          } else if (itsElement == "measFreqRef") {
                measFreqRef.put(nRow,atoi(xmlValue)); 
          } else if (itsElement == "bbcNo") {
                bbcNo.put(nRow,atoi(xmlValue)); 
          } else if (itsElement == "bbcSideband") {
                bbcSideband.put(nRow,atoi(xmlValue)); 
          } else if (itsElement == "ifConvChain") {
                ifConvChain.put(nRow,atoi(xmlValue)); 
          } else if (itsElement == "freqGroup") {
                freqGroup.put(nRow,atoi(xmlValue)); 
          } else if (itsElement == "freqGroupName") {
                freqGroupName.put(nRow,value); 
          }
	//
	//State Table
	//
            else if (itsElement == "sig") {
		if (value == "true") 
		    sig.put(nRow,true);
		else
		    sig.put(nRow,false);
          } else if (itsElement == "ref") {
		if (value == "true") 
		    ref.put(nRow,true);
		else
		    ref.put(nRow,false);
          }  else if (itsElement == "calloadNum") {
                calloadNum.put(nRow,atoi(xmlValue)); 
          }  else if (itsElement == "obsMode") {
                obsMode.put(nRow,value); 
          }  else if (itsElement == "obsIntent") {
                obsIntent.put(nRow,value); 
          }  else if (itsElement == "weight") {
                weight.put(nRow,atof(xmlValue)); 
          }
	//
	//Station Table
	//All already defined
	//

	//
	//Subscan Table
	//
             else if (itsElement == "subscanIntent") {
                subscanIntent.put(nRow,value);
          }  else if (itsElement == "numberIntegration") {
                numberIntegration.put(nRow,atoi(xmlValue));
          }  else if (itsElement == "numberSubintegration") {
		Array<Int> temp;
		assignColumn(xmlValue,temp);
                numberSubintegration.put(nRow,temp);
          }  else if (itsElement == "subscanMode") {
                subscanMode.put(nRow,value);
          }
	//
	//SwitchCycle Table
	//
             else if (itsElement == "numStep") {
                numStep.put(nRow,atoi(xmlValue));
          }  else if (itsElement == "weightArray") {
		Array<Double> temp;
		assignColumn(xmlValue,temp);
                weightArray.put(nRow,temp);
          }  else if (itsElement == "offsetArray") {
		Array<Double> temp;
		assignColumn(xmlValue,temp);
                offsetArray.put(nRow,temp);
          }  else if (itsElement == "freqOffsetArray") {
		Array<Double> temp;
		assignColumn(xmlValue,temp);
                freqOffsetArray.put(nRow,temp);
          }
	//
	//CalCurve Table
	//
             else if (itsElement == "calDataId") {
                calDataId.put(nRow,value);
          }  else if (itsElement == "calReductionId") {
                calReductionId.put(nRow,value);
          }  else if (itsElement == "antennaName") {
                antennaName.put(nRow,value);
          }  else if (itsElement == "startValidTime") {
                startValidTime.put(nRow,value);
          }  else if (itsElement == "endValidTime") {
                endValidTime.put(nRow,value);
          }  else if (itsElement == "typeCurve") {
                typeCurve.put(nRow,value);
          }  else if (itsElement == "curve") {
                Array<Double>  temp;
                assignColumn(xmlValue,temp);
                curve.put(nRow,temp);
          }  else if (itsElement == "frequencyRange") {
		Array<Double> temp;
		assignColumn(xmlValue,temp);
                frequencyRange.put(nRow,temp);
          }  else if (itsElement == "refAntennaName") {
                refAntennaName.put(nRow,value);
          }  else if (itsElement == "receiverBand") {
                receiverBand.put(nRow,value);
          }  else if (itsElement == "timeOrigin") {
                timeOrigin.put(nRow,value);
          }
	  
	//
	//CalData Table
	//

             else if (itsElement == "scanSet") {
		Array<Int> temp;
		assignColumn(xmlValue,temp);
                scanSet.put(nRow,temp);
          }  else if (itsElement == "calType") {
                calType.put(nRow,value);
          }  else if (itsElement == "startTimeObserved") {
                startTimeObserved.put(nRow,value);
          }  else if (itsElement == "endTimeObserved") {
                endTimeObserved.put(nRow,value);
          }  else if (itsElement == "calDataType") {
                calDataType.put(nRow,value);
          }  else if (itsElement == "frequencyGroup") {
                frequencyGroup.put(nRow,atoi(xmlValue));
          }  else if (itsElement == "freqGroupName") {
                freqGroupName.put(nRow,value);
          }  else if (itsElement == "fieldCode") {
		Array<String> temp;
		assignColumn(xmlValue,temp);
                fieldCode.put(nRow,temp);
          }  else if (itsElement == "sourceCode") {
		Array<String> temp;
		assignColumn(xmlValue,temp);
                sourceCode.put(nRow,temp);
          }  else if (itsElement == "assocCalDataId") {
                assocCalDataId.put(nRow,value);
          }  else if (itsElement == "assocCalNature") {
                assocCalNature.put(nRow,value);
	  }
	//
	//CalPhase Table
	//
             else if (itsElement == "basebandName") {
                basebandName.put(nRow,value);
          }  else if (itsElement == "basebandName") {
                basebandName.put(nRow,value);
          }  else if (itsElement == "numBaseline") {
                numBaseline.put(nRow,atoi(xmlValue));
          }  else if (itsElement == "antennaNames") {
		Array<String> temp;
		assignColumn(xmlValue,temp);
                antennaNames.put(nRow,temp);
          }  else if (itsElement == "decorrelationFactor") {
		Array<Double> temp;
		assignColumn(xmlValue,temp);
                decorrelationFactor.put(nRow,temp);
          }  else if (itsElement == "uncorrPhaseRms") {
		Array<Double> temp;
		assignColumn(xmlValue,temp);
                uncorrPhaseRms.put(nRow,temp);
          }  else if (itsElement == "corrPhaseRms") {
		Array<Double> temp;
		assignColumn(xmlValue,temp);
                corrPhaseRms.put(nRow,temp);
          }  else if (itsElement == "statPhaseRms") {
		Array<Double> temp;
		assignColumn(xmlValue,temp);
                statPhaseRms.put(nRow,temp);
          }  else if (itsElement == "corrValidity") {
		Array<Bool> temp;
		assignColumn(xmlValue,temp);
                corrValidity.put(nRow,temp);
          }
	//
	//CalReduction Table
	     else if (itsElement == "numApplied") {
	      numApplied.put(nRow,atoi(xmlValue));
	   } else if (itsElement == "numParam") {
	      numParam.put(nRow,atoi(xmlValue));
	   } else if (itsElement == "timeReduced") {
	      timeReduced.put(nRow,value);
	   } else if (itsElement == "calAppliedArray") {
		Array<String> temp;	      
		assignColumn(xmlValue,temp);
	        calAppliedArray.put(nRow,temp);
	   } else if (itsElement == "paramSet") {
		Array<String> temp;	      
		assignColumn(xmlValue,temp);
	        paramSet.put(nRow,temp);
	   } else if (itsElement == "messages") {
	      messages.put(nRow,value);
	   } else if (itsElement == "software") {
	      software.put(nRow,value);
	   } else if (itsElement == "softwareVersion") {
	      softwareVersion.put(nRow,value);
	   } else if (itsElement == "invalidConditions") {
	      invalidConditions.put(nRow,value);
           }
	//
	//CalSeeing Table
	//
	     else if (itsElement == "numBaseLength") {
	      numBaseLength.put(nRow,atoi(xmlValue));
	   } else if (itsElement == "baseLength") {
	      Array<Double> temp;
	      assignColumn(xmlValue,temp);
	      baseLength.put(nRow,temp);
	   } else if (itsElement == "seeing") {
	      seeing.put(nRow,atof(xmlValue));
	   } else if (itsElement == "seeingFrequency") {
	      seeingFrequency.put(nRow,atof(xmlValue));
	   } else if (itsElement == "seeingFreqBandwidth") {
	      seeingFreqBandwidth.put(nRow,atof(xmlValue));
	   } else if (itsElement == "exponent") {
	      exponent.put(nRow,atof(xmlValue));
           }
	//
	//CalAtmosphere Table
	//
	     else if (itsElement == "numFreq") {
	      numFreq.put(nRow,atoi(xmlValue));
	   } else if (itsElement == "frequencySpectrum") {
	      Array<Double> temp;
              assignColumn(xmlValue,temp);
	      frequencySpectrum.put(nRow,temp);
	   } else if (itsElement == "syscalType") {
	      syscalType.put(nRow,value);
	   } else if (itsElement == "tSysSpectrum") {
	      Array<Double> temp;
              assignColumn(xmlValue,temp);
	      tSysSpectrum.put(nRow,temp);
	   } else if (itsElement == "tRecSpectrum") {
	      Array<Double> temp;
              assignColumn(xmlValue,temp);
	      tRecSpectrum.put(nRow,temp);
	   } else if (itsElement == "tAtmSpectrum") {
	      Array<Double> temp;
              assignColumn(xmlValue,temp);
	      tAtmSpectrum.put(nRow,temp);
	   } else if (itsElement == "tauSpectrum") {
	      Array<Double> temp;
              assignColumn(xmlValue,temp);
	      tauSpectrum.put(nRow,temp);
	   } else if (itsElement == "sbGainSpectrum") {
	      Array<Double> temp;
              assignColumn(xmlValue,temp);
	      sbGainSpectrum.put(nRow,temp);
	   } else if (itsElement == "forwardEffSpectrum") {
	      Array<Double> temp;
              assignColumn(xmlValue,temp);
	      forwardEffSpectrum.put(nRow,temp);
	   } else if (itsElement == "groundPressure") {
	      groundPressure.put(nRow,atof(xmlValue));
	   } else if (itsElement == "groundTemperature") {
	      groundTemperature.put(nRow,atof(xmlValue));
	   } else if (itsElement == "groundRelHumidity") {
	      groundRelHumidity.put(nRow,atof(xmlValue));
	   } else if (itsElement == "tSys") {
	      Array<Double> temp;
              assignColumn(xmlValue,temp);
	      tSys.put(nRow,temp);
	   } else if (itsElement == "tRec") {
	      Array<Double> temp;
              assignColumn(xmlValue,temp);
	      tRec.put(nRow,temp);
	   } else if (itsElement == "tAtm") {
	      Array<Double> temp;
              assignColumn(xmlValue,temp);
	      tAtm.put(nRow,temp);
	   } else if (itsElement == "sbGain") {
	      Array<Double> temp;
              assignColumn(xmlValue,temp);
	      sbGain.put(nRow,temp);
	   } else if (itsElement == "forwardEfficiency") {
	      Array<Double> temp;
              assignColumn(xmlValue,temp);
	      forwardEfficiency.put(nRow,temp);
	   } else if (itsElement == "tau") {
	      Array<Double> temp;
              assignColumn(xmlValue,temp);
	      tau.put(nRow,temp);
           }
	//
	//CalAmpli Table
	//
	     else if (itsElement == "apertureEfficiencyError") {
	      Array<Double> temp;
              assignColumn(xmlValue,temp);
	      apertureEfficiencyError.put(nRow,temp);
	   } else if (itsElement == "uncorrectedApertureEfficiency") {
	      Array<Double> temp;
              assignColumn(xmlValue,temp);
	      uncorrectedApertureEfficiency.put(nRow,temp);
	   } else if (itsElement == "correctedApertureEfficiency") {
	      Array<Double> temp;
              assignColumn(xmlValue,temp);
	      correctedApertureEfficiency.put(nRow,temp);
	   } else if (itsElement == "correctionValidity") {
	      Array<Bool> temp;
              assignColumn(xmlValue,temp);
	      correctionValidity.put(nRow,temp);
           }
        //
        //CalBandpass Table
        //
             else if (itsElement == "freqLimits") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              freqLimits.put(nRow,temp);
           } else if (itsElement == "numPhasePoly") {
              numPhasePoly.put(nRow,atoi(xmlValue));
           } else if (itsElement == "numAmpliPoly") {
              numAmpliPoly.put(nRow,atoi(xmlValue));
           } else if (itsElement == "phaseCurve") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              phaseCurve.put(nRow,temp);
           } else if (itsElement == "ampliCurve") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              ampliCurve.put(nRow,temp);
           }
        //
        //AlmaRadiometer Table
        //
             else if (itsElement == "modeId") {
	      modeId.put(nRow,value);
           } else if (itsElement == "numBand") {
	      numBand.put(nRow,atoi(xmlValue));
           }
        //
        //CalDelay Table
        //
             else if (itsElement == "delayOffset") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              delayOffset.put(nRow,temp);
           } else if (itsElement == "delayError") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              delayError.put(nRow,temp);
           } else if (itsElement == "crossDelayOffset") {
	      crossDelayOffset.put(nRow,atof(xmlValue));
           } else if (itsElement == "crossDelayOffsetError") {
	      crossDelayOffsetError.put(nRow,atof(xmlValue));
           }
        //
        //CalDevice Table
        //
             else if (itsElement == "numCalload") {
	      numCalload.put(nRow,atoi(xmlValue));
           } else if (itsElement == "noiseCal") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              noiseCal.put(nRow,temp);
           } else if (itsElement == "temperatureLoad") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              temperatureLoad.put(nRow,temp);
           } else if (itsElement == "calEff") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              calEff.put(nRow,temp);
           }
        //
        //CalFlux Table
        //
             else if (itsElement == "numStokes") {
	      numStokes.put(nRow,atoi(xmlValue));
           } else if (itsElement == "Stokes") {
              Array<Int> temp;
              assignColumn(xmlValue,temp);
              Stokes.put(nRow,temp);
           } else if (itsElement == "frequencyWidth") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              frequencyWidth.put(nRow,temp);
           } else if (itsElement == "flux") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              flux.put(nRow,temp);
           } else if (itsElement == "fluxError") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              fluxError.put(nRow,temp);
           } else if (itsElement == "fluxMethod") {
	      fluxMethod.put(nRow,value);
           } else if (itsElement == "size") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              size.put(nRow,temp);
           } else if (itsElement == "sizeError") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              sizeError.put(nRow,temp);
           } else if (itsElement == "PA") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              PA.put(nRow,temp);
           } else if (itsElement == "PAError") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              PAError.put(nRow,temp);
           }
        //
        //CalFocus Table
        //
             else if (itsElement == "offset") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              offset.put(nRow,temp);
           } else if (itsElement == "error") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              error.put(nRow,temp);
           } else if (itsElement == "method") {
	      method.put(nRow,value);
           } else if (itsElement == "pointingDirection") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              pointingDirection.put(nRow,temp);
           } else if (itsElement == "wasFixed") {
              Array<Bool> temp;
              assignColumn(xmlValue,temp);
              wasFixed.put(nRow,temp);
           }
        //
        //CalFocus Table
        //
             else if (itsElement == "numCoeff") {
	      numCoeff.put(nRow,atoi(xmlValue));
           } else if (itsElement == "focusRMS") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              focusRMS.put(nRow,temp);
           } else if (itsElement == "coeffName") {
              Array<String> temp;
              assignColumn(xmlValue,temp);
              coeffName.put(nRow,temp);
           } else if (itsElement == "coeffFormula") {
              Array<String> temp;
              assignColumn(xmlValue,temp);
              coeffFormula.put(nRow,temp);
           } else if (itsElement == "coeffValue") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              coeffValue.put(nRow,temp);
           } else if (itsElement == "coeffError") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              coeffError.put(nRow,temp);
           } else if (itsElement == "coeffFixed") {
              Array<Bool> temp;
              assignColumn(xmlValue,temp);
              coeffFixed.put(nRow,temp);
           } else if (itsElement == "focusModel") {
	      focusModel.put(nRow,value);
           } else if (itsElement == "numSourceObs") {
	      numSourceObs.put(nRow,atoi(xmlValue));
           }
        //
        //CalGain Table
        //
             else if (itsElement == "gain") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              gain.put(nRow,temp);
           } else if (itsElement == "gainValid") {
              Array<Bool> temp;
              assignColumn(xmlValue,temp);
              gainValid.put(nRow,temp);
           } else if (itsElement == "fit") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              fit.put(nRow,temp);
           } else if (itsElement == "fitWeight") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              fitWeight.put(nRow,temp);
           } else if (itsElement == "totalGainValid") {
	      if (value=="true")
                 numSourceObs.put(nRow,true);
	      else
                 numSourceObs.put(nRow,false);
           } else if (itsElement == "totalFit") {
	      totalFit.put(nRow,atof(xmlValue));
           } else if (itsElement == "totalFitWeight") {
	      totalFitWeight.put(nRow,atof(xmlValue));
           }
        //
        //CalHolography Table
        //
             else if (itsElement == "numScrew") {
	      numScrew.put(nRow,atoi(xmlValue));
           } else if (itsElement == "focusPosition") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              focusPosition.put(nRow,temp);
           } else if (itsElement == "rawRms") {
	      rawRms.put(nRow,atof(xmlValue));
           } else if (itsElement == "screwName") {
              Array<String> temp;
              assignColumn(xmlValue,temp);
              screwName.put(nRow,temp);
           } else if (itsElement == "screwMotion") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              screwMotion.put(nRow,temp);
           } else if (itsElement == "screwMotionError") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              screwMotionError.put(nRow,temp);
           } else if (itsElement == "panelModes") {
	      panelModes.put(nRow,atoi(xmlValue));
           } else if (itsElement == "beamMapUID") {
	      beamMapUID.put(nRow,value);
           } else if (itsElement == "surfaceMapUID") {
	      surfaceMapUID.put(nRow,value);
           }
        //
        //CalPointing Table
        //
             else if (itsElement == "collError") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              collError.put(nRow,temp);
           } else if (itsElement == "collOffset") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              collOffset.put(nRow,temp);
           } else if (itsElement == "pointingMethod") {
              pointingMethod.put(nRow,value);
           } else if (itsElement == "mode") {
              mode.put(nRow,value);
           } else if (itsElement == "beamWidth") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              beamWidth.put(nRow,temp);
           } else if (itsElement == "beamWidthError") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              beamWidthError.put(nRow,temp);
           } else if (itsElement == "beamPA") {
              beamPA.put(nRow,atof(xmlValue));
           } else if (itsElement == "beamPAError") {
              beamPAError.put(nRow,atof(xmlValue));
           } else if (itsElement == "peakIntensity") {
              peakIntensity.put(nRow,atof(xmlValue));
           } else if (itsElement == "peakIntensityError") {
              peakIntensityError.put(nRow,atof(xmlValue));
           }
        //
        //CalPointingModel Table
        //
             else if (itsElement == "numObs") {
              numObs.put(nRow,atoi(xmlValue));
           } else if (itsElement == "numFormula") {
              numFormula.put(nRow,atoi(xmlValue));
           } else if (itsElement == "azimuthRms") {
              azimuthRms.put(nRow,atof(xmlValue));
           } else if (itsElement == "elevationRms") {
              elevationRms.put(nRow,atof(xmlValue));
           } else if (itsElement == "skyRms") {
              skyRms.put(nRow,atof(xmlValue));
           } else if (itsElement == "coeffVal") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              coeffVal.put(nRow,temp);
           } else if (itsElement == "pointingModel") {
              pointingModel.put(nRow,value);
           }
        //
        //CalPosition Table
        //
             else if (itsElement == "positionOffset") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              positionOffset.put(nRow,temp);
           } else if (itsElement == "positionErr") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              positionErr.put(nRow,temp);
           } else if (itsElement == "delayRms") {
              delayRms.put(nRow,value);
           } else if (itsElement == "phaseRms") {
		     if (xmlTableName=="CalPositionTable")
                        phaseRms.put(nRow,atof(xmlValue));
                     else if (xmlTableName=="SeeingTable"){
			Array<Int> temp;
			assignColumn(xmlValue,temp);
			phaseRmsAUX.put(nRow,temp);
                     }
           } else if (itsElement == "axesOffset") {
              axesOffset.put(nRow,atof(xmlValue));
           } else if (itsElement == "axesOffsetFixed") {
	      if (value=="true")
                 axesOffsetFixed.put(nRow,true);
	      else
                 axesOffsetFixed.put(nRow,false);
           } else if (itsElement == "axesOffsetErr") {
              axesOffsetErr.put(nRow,atof(xmlValue));
           } else if (itsElement == "positionMethod") {
              positionMethod.put(nRow,value);
           } else if (itsElement == "refAntennaNames") {
              Array<String> temp;
              assignColumn(xmlValue,temp);
              refAntennaNames.put(nRow,temp);
           }
        //
        //CalPrimaryBeam Table
        //
             else if (itsElement == "numPixelX") {
              numPixelX.put(nRow,atoi(xmlValue));
           } else if (itsElement == "numPixelY") {
              numPixelY.put(nRow,atoi(xmlValue));
           } else if (itsElement == "refX") {
              refX.put(nRow,atof(xmlValue));
           } else if (itsElement == "refY") {
              refY.put(nRow,atof(xmlValue));
           } else if (itsElement == "valX") {
              valX.put(nRow,atof(xmlValue));
           } else if (itsElement == "valY") {
              valY.put(nRow,atof(xmlValue));
           } else if (itsElement == "incX") {
              incX.put(nRow,atof(xmlValue));
           } else if (itsElement == "incY") {
              incY.put(nRow,atof(xmlValue));
           } else if (itsElement == "amplitude") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              amplitude.put(nRow,temp);
           } else if (itsElement == "phase") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              phase.put(nRow,temp);
           }
        //
        //CalWVR Table
        //
             else if (itsElement == "WVRMethod") {
              WVRMethod.put(nRow,value);
           } else if (itsElement == "pathCoeff") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              pathCoeff.put(nRow,temp);
           }
        //
        //Doppler Table
        //
             else if (itsElement == "transitionIndex") {
              transitionIndex.put(nRow,atoi(xmlValue));
           } else if (itsElement == "velDef") {
              velDef.put(nRow,atof(xmlValue));
           }
        //
        //Doppler Table
        //
             else if (itsElement == "ephemerisId") {
              ephemerisId.put(nRow,value);
           }
        //
        //Observation Table
        //
             else if (itsElement == "observationId") {
              observationId.put(nRow,value);
           }
        //
        //FlagCmd Table
        //
             else if (itsElement == "reason") {
              reason.put(nRow,value);
           } else if (itsElement == "level") {
              level.put(nRow,atoi(xmlValue));
           } else if (itsElement == "severity") {
              severity.put(nRow,atoi(xmlValue));
           } else if (itsElement == "applied") {
	      if (value=="true")
                 applied.put(nRow,true);
	      else
                 applied.put(nRow,false);
           } else if (itsElement == "command") {
              command.put(nRow,value);
           }
        //
        //FocusModel Table
        //
             else if (itsElement == "focusModelId") {
              focusModelId.put(nRow,value);
           } else if (itsElement == "xFocusPosition") {
              xFocusPosition.put(nRow,atof(xmlValue));
           } else if (itsElement == "yFocusPosition") {
              yFocusPosition.put(nRow,atof(xmlValue));
           } else if (itsElement == "zFocusPosition") {
              zFocusPosition.put(nRow,atof(xmlValue));
           } else if (itsElement == "focusTracking") {
              focusTracking.put(nRow,atof(xmlValue));
           } else if (itsElement == "xFocusOffset") {
              xFocusOffset.put(nRow,atof(xmlValue));
           } else if (itsElement == "yFocusOffset") {
              yFocusOffset.put(nRow,atof(xmlValue));
           } else if (itsElement == "zFocusOffset") {
              zFocusOffset.put(nRow,atof(xmlValue));
           }
        //
        //GainTracking Table
        //
             else if (itsElement == "attenuator") {
              attenuator.put(nRow,atof(xmlValue));
           } else if (itsElement == "delayoff1") {
              delayoff1.put(nRow,value);
           } else if (itsElement == "delayoff2") {
              delayoff2.put(nRow,value);
           } else if (itsElement == "phaseoff1") {
              phaseoff1.put(nRow,atof(xmlValue));
           } else if (itsElement == "phaseoff2") {
              phaseoff2.put(nRow,atof(xmlValue));
           } else if (itsElement == "rateoff1") {
              rateoff1.put(nRow,atof(xmlValue));
           } else if (itsElement == "rateoff2") {
              rateoff2.put(nRow,atof(xmlValue));
           } else if (itsElement == "samplingLevel") {
              samplingLevel.put(nRow,atof(xmlValue));
           } else if (itsElement == "phaseRefOffset") {
              phaseRefOffset.put(nRow,atof(xmlValue));
           }
        //
        //History Table
        //
             else if (itsElement == "message") {
              message.put(nRow,value);
           } else if (itsElement == "priority") {
              priority.put(nRow,value);
           } else if (itsElement == "origin") {
              origin.put(nRow,value);
           } else if (itsElement == "objectId") {
              objectId.put(nRow,value);
           } else if (itsElement == "application") {
              application.put(nRow,value);
           } else if (itsElement == "cliCommand") {
              cliCommand.put(nRow,value);
           } else if (itsElement == "appParms") {
              appParms.put(nRow,value);
           }
        //
        //Holography Table
        //
             else if (itsElement == "holographyId") {
              holographyId.put(nRow,value);
           } else if (itsElement == "distance") {
              distance.put(nRow,atof(xmlValue));
           } else if (itsElement == "focus") {
              focus.put(nRow,atof(xmlValue));
           }
        //
        //Pointing Table
        //
             else if (itsElement == "pointingModelId") {
              pointingModelId.put(nRow,atoi(xmlValue));
           } else if (itsElement == "target") {
	      Array<Double> temp;
	      assignColumn(xmlValue,temp);
              target.put(nRow,temp);
           } else if (itsElement == "encoder") {
	      Array<Double> temp;
	      assignColumn(xmlValue,temp);
              encoder.put(nRow,temp);
           } else if (itsElement == "pointingTracking") {
	      if (value=="true")
                 pointingTracking.put(nRow,true);
	      else
                 pointingTracking.put(nRow,false);
           } else if (itsElement == "sourceOffset") {
	      Array<Double> temp;
	      assignColumn(xmlValue,temp);
              sourceOffset.put(nRow,temp);
           } else if (itsElement == "phaseTracking") {
	      if (value=="true")
                 phaseTracking.put(nRow,true);
	      else
                 phaseTracking.put(nRow,false);
           } else if (itsElement == "overTheTop") {
	      if (value=="true")
                 overTheTop.put(nRow,true);
	      else
                 overTheTop.put(nRow,false);
           }
        //
        //SourceParameter Table
        //
             else if (itsElement == "numDep") {
              numDep.put(nRow,atoi(xmlValue));
           } else if (itsElement == "stokeParameter") {
              Array<Int> temp;
              assignColumn(xmlValue,temp);
              stokeParameter.put(nRow,temp);
           } else if (itsElement == "frequencyInterval") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              frequencyInterval.put(nRow,temp);
           } else if (itsElement == "fluxErr") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              fluxErr.put(nRow,temp);
           } else if (itsElement == "positionAngle") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              positionAngle.put(nRow,temp);
           } else if (itsElement == "sizeErr") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              sizeErr.put(nRow,temp);
           } else if (itsElement == "positionAngleErr") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              positionAngleErr.put(nRow,temp);
           } else if (itsElement == "depSourceParameterId") {
              Array<Int> temp;
              assignColumn(xmlValue,temp);
              depSourceParameterId.put(nRow,temp);
           }
        //
        //SourceParameter Table
        //
             else if (itsElement == "squareLawDetectorId") {
              squareLawDetectorId.put(nRow,value);
           } else if (itsElement == "bandType") {
              bandType.put(nRow,value);
           }
        //
        //SourceParameter Table
        //
             else if (itsElement == "numLoad") {
              numLoad.put(nRow,atoi(xmlValue));
           } else if (itsElement == "calLoad") {
              Array<Int> temp;
              assignColumn(xmlValue,temp);
              calLoad.put(nRow,temp);
           } else if (itsElement == "feff") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              feff.put(nRow,temp);
           } else if (itsElement == "aeff") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              aeff.put(nRow,temp);
           } else if (itsElement == "phaseDiff") {
              phaseDiff.put(nRow,atof(xmlValue));
           } else if (itsElement == "sbgain") {
              sbgain.put(nRow,atof(xmlValue));
           } else if (itsElement == "tcal") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              tcal.put(nRow,temp);
           } else if (itsElement == "trx") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              trx.put(nRow,temp);
           } else if (itsElement == "tsky") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              tsky.put(nRow,temp);
           } else if (itsElement == "tant") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              tant.put(nRow,temp);
           } else if (itsElement == "tantTsys") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              tantTsys.put(nRow,temp);
           } else if (itsElement == "pwvPath") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              pwvPath.put(nRow,temp);
           } else if (itsElement == "dpwvPath") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              dpwvPath.put(nRow,temp);
           } else if (itsElement == "feffSpectrum") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              feffSpectrum.put(nRow,temp);
           } else if (itsElement == "sbgainSpectrum") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              sbgainSpectrum.put(nRow,temp);
           } else if (itsElement == "tauSpectrum") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              tauSpectrum.put(nRow,temp);
           } else if (itsElement == "tcalSpectrum") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              tcalSpectrum.put(nRow,temp);
           } else if (itsElement == "trxSpectrum") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              trxSpectrum.put(nRow,temp);
           } else if (itsElement == "tskySpectrum") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              tskySpectrum.put(nRow,temp);
           } else if (itsElement == "tsysSpectrum") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              tsysSpectrum.put(nRow,temp);
           } else if (itsElement == "tantSpectrum") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              tantSpectrum.put(nRow,temp);
           } else if (itsElement == "tantTsysSpectrum") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              tantTsysSpectrum.put(nRow,temp);
           } else if (itsElement == "pwvPathSpectrum") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              pwvPathSpectrum.put(nRow,temp);
           } else if (itsElement == "dpwvPathSpectrum") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              dpwvPathSpectrum.put(nRow,temp);
           } else if (itsElement == "numPolyFreq") {
              numPolyFreq.put(nRow,atoi(xmlValue));
           } else if (itsElement == "freqOrigin") {
              freqOrigin.put(nRow,atof(xmlValue));
           } else if (itsElement == "delayCurve") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              delayCurve.put(nRow,temp);
           } else if (itsElement == "bandpassCurve") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              bandpassCurve.put(nRow,temp);
           } else if (itsElement == "phasediffFlag") {
	      if (value=="true")
                 phasediffFlag.put(nRow,true);
	      else
                 phasediffFlag.put(nRow,false);
           } else if (itsElement == "sbgainFlag") {
	      if (value=="true")
                 sbgainFlag.put(nRow,true);
	      else
                 sbgainFlag.put(nRow,false);
           } else if (itsElement == "tauFlag") {
	      if (value=="true")
                 tauFlag.put(nRow,true);
	      else
                 tauFlag.put(nRow,false);
           } else if (itsElement == "tcalFlag") {
	      if (value=="true")
                 tcalFlag.put(nRow,true);
	      else
                 tcalFlag.put(nRow,false);
           } else if (itsElement == "trxFlag") {
	      if (value=="true")
                 trxFlag.put(nRow,true);
	      else
                 trxFlag.put(nRow,false);
           } else if (itsElement == "tskyFlag") {
	      if (value=="true")
                 tskyFlag.put(nRow,true);
	      else
                 tskyFlag.put(nRow,false);
           } else if (itsElement == "tsysFlag") {
	      if (value=="true")
                 tsysFlag.put(nRow,true);
	      else
                 tsysFlag.put(nRow,false);
           } else if (itsElement == "tantFlag") {
	      if (value=="true")
                 tantFlag.put(nRow,true);
	      else
                 tantFlag.put(nRow,false);
           } else if (itsElement == "tantTsysFlag") {
	      if (value=="true")
                 tantTsysFlag.put(nRow,true);
	      else
                 tantTsysFlag.put(nRow,false);
           } else if (itsElement == "pwvPathFlag") {
	      if (value=="true")
                 pwvPathFlag.put(nRow,true);
	      else
                 pwvPathFlag.put(nRow,false);
           }
	//
	//TotalPower Table
	//
             else if (itsElement == "floatData") {
              Array<Double> temp;
              assignColumn(xmlValue,temp);
              floatData.put(nRow,temp);
           }
	//
	//WVMCal Table
	//
             else if (itsElement == "calibrationMode") {
              calibrationMode.put(nRow,value);
           } else if (itsElement == "operationMode") {
              operationMode.put(nRow,value);
           } else if (itsElement == "wvrefModel") {
              wvrefModel.put(nRow,atof(xmlValue));
           }
        //
        //Weather Table
        //
             else if (itsElement == "pressure") {
              pressure.put(nRow,atof(xmlValue));
           } else if (itsElement == "relHumidity") {
              relHumidity.put(nRow,atof(xmlValue));
           } else if (itsElement == "windDirection") {
              windDirection.put(nRow,atof(xmlValue));
           } else if (itsElement == "windSpeed") {
              windSpeed.put(nRow,atof(xmlValue));
           } else if (itsElement == "windMax") {
              windMax.put(nRow,atof(xmlValue));
           } else if (itsElement == "pressureFlag") {
	      if (value=="true")
                 pressureFlag.put(nRow,true);
	      else
                 pressureFlag.put(nRow,false);
           } else if (itsElement == "relHumidityFlag") {
	      if (value=="true")
                 relHumidityFlag.put(nRow,true);
	      else
                 relHumidityFlag.put(nRow,false);
           } else if (itsElement == "temperatureFlag") {
	      if (value=="true")
                 temperatureFlag.put(nRow,true);
	      else
                 temperatureFlag.put(nRow,false);
           } else if (itsElement == "windDirectionFlag") {
	      if (value=="true")
                 windDirectionFlag.put(nRow,true);
	      else
                 windDirectionFlag.put(nRow,false);
           } else if (itsElement == "windSpeedFlag") {
	      if (value=="true")
                 windSpeedFlag.put(nRow,true);
	      else
                 windSpeedFlag.put(nRow,false);
           } else if (itsElement == "windMaxFlag") {
	      if (value=="true")
                 windMaxFlag.put(nRow,true);
	      else
                 windMaxFlag.put(nRow,false);
           } else if (itsElement == "dewPoint") {
              dewPoint.put(nRow,atof(xmlValue));
           } else if (itsElement == "dewPointFlag") {
	      if (value=="true")
                 dewPointFlag.put(nRow,true);
	      else
                 dewPointFlag.put(nRow,false);
           }
	//
	//ASDM Table
	//
             else if (itsElement == "Name") {
              Name.put(nRow,value);
	      cout << "value: " <<  value <<endl;
           } else if (itsElement == "NumberRows") {
              NumberRows.put(nRow,atoi(xmlValue));
	      cout << "value: " <<  value <<endl;
           }



  
        }
	XMLString::release(&xmlValue);
}

void asdmCasaSaxHandler::fatalError(const SAXParseException& exception) 
{ 
	char* message = XMLString::transcode(exception.getMessage()); 
	cerr << "Fatal Error: " << message 
		<< " at line: " << exception.getLineNumber() 
		<< endl; 
} 


}
