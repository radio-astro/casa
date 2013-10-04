//# tAttValPoi.cc: test program for AttVaPoi class
//# Copyright (C) 1999,2000,2001
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

#include <casa/aips.h>
#include <casa/Exceptions/Error.h>
#include <casa/Utilities/Assert.h>
#include <display/Display/AttValPoi.h>


#include <casa/namespace.h>
int main(int, char **) {
	try {


// Int

		{
			Bool strict = True;
			Int val0 = 10;
			AttributeValuePoi<Int> av0(&val0, strict);
			AlwaysAssert(av0.getStrictness()==strict, AipsError);
			AlwaysAssert(av0.getPointerType()==TpInt, AipsError);
			AlwaysAssert(av0.getType()==AttValue::AtInt, AipsError);
//
			av0.setStrictness(!strict);
			AlwaysAssert(av0.getStrictness()==!strict, AipsError);
			av0.setStrictness(strict);
//
			Vector<Int> vval0 = av0.getValue();
			AlwaysAssert(vval0.nelements()==1,AipsError);
			AlwaysAssert(vval0(0)==val0, AipsError);
//
			AttributeValuePoi<Int> av1(av0);
			AlwaysAssert(av1.getStrictness()==strict, AipsError);
			AlwaysAssert(av1.getPointerType()==TpInt, AipsError);
			AlwaysAssert(av1.getType()==AttValue::AtInt, AipsError);
			Vector<Int> vval1 = av1.getValue();
			AlwaysAssert(vval1.nelements()==1,AipsError);
			AlwaysAssert(vval1(0)==val0,AipsError);
			AlwaysAssert(av1==av0, AipsError);
			AlwaysAssert(!(av1!=av0), AipsError);
//
			Int val2 = 20;
			AttributeValuePoi<Int> av2(&val2, strict);
			av2 = av0;
			AlwaysAssert(av2==av0, AipsError);
//
			Int val2Copy = val2;
			av2.setValue(val2);
			av2 += av0;          // changes val2
			Vector<Int> vval2 = av2.getValue();
			AlwaysAssert(vval2.nelements()==1,AipsError);
			AlwaysAssert(vval2(0)==val2Copy+av0.getValue()(0),AipsError);
//
			AttributeValueBase* p = av0.clone();
			AttributeValuePoi<Int>* p2 = dynamic_cast<AttributeValuePoi<Int>* >(p);
			AlwaysAssert(*(p2)==av0, AipsError);
			delete p;
//
			Vector<Int> val3(2);
			val3(0) = 20;
			val3(1) = 100;
			av0.setValue(val3);                     // Discards second element
			Vector<Int> vval3 = av0.getValue();
			AlwaysAssert(vval3.nelements()==1,AipsError);
			AlwaysAssert(vval3(0)==val3(0), AipsError);
			Int val4 = 30;
			av0.setValue(val4);
			vval3.resize(0);
			vval3 = av0.getValue();
			AlwaysAssert(vval3.nelements()==1,AipsError);
			AlwaysAssert(vval3(0)==val4, AipsError);
//
			Vector<Int> val5(2);
			val5(0) = 2;
			val5(1) = 10;
			AttributeValuePoi<Int> av5(&val5, strict);
			AlwaysAssert(av5.getStrictness()==strict, AipsError);
			AlwaysAssert(av5.getPointerType()==TpArrayInt, AipsError);
			AlwaysAssert(av5.getType()==AttValue::AtInt, AipsError);
			Vector<Int> vval5 = av5.getValue();
			AlwaysAssert(vval5.nelements()==2,AipsError);
			AlwaysAssert(vval5(0)==val5(0) && vval5(1)==val5(1), AipsError);
//
			Vector<Int> val6(3);
			val6 = 45;
			av5.setValue(val6);
			Vector<Int> vval6 = av5.getValue();
			AlwaysAssert(vval6.nelements()==3,AipsError);
			AlwaysAssert(vval6(0)==val6(0) && vval6(1)==val6(1) && vval6(2)==val6(2), AipsError);
		}

// Float

		{
			Bool strict = True;
			Float val0 = 10;
			AttributeValuePoi<Float> av0(&val0, strict);
			AlwaysAssert(av0.getStrictness()==strict, AipsError);
			AlwaysAssert(av0.getPointerType()==TpFloat, AipsError);
			AlwaysAssert(av0.getType()==AttValue::AtFloat, AipsError);
//
			av0.setStrictness(!strict);
			AlwaysAssert(av0.getStrictness()==!strict, AipsError);
			av0.setStrictness(strict);
//
			Vector<Float> vval0 = av0.getValue();
			AlwaysAssert(vval0.nelements()==1,AipsError);
			AlwaysAssert(vval0(0)==val0, AipsError);
//
			AttributeValuePoi<Float> av1(av0);
			AlwaysAssert(av1.getStrictness()==strict, AipsError);
			AlwaysAssert(av1.getPointerType()==TpFloat, AipsError);
			AlwaysAssert(av1.getType()==AttValue::AtFloat, AipsError);
			Vector<Float> vval1 = av1.getValue();
			AlwaysAssert(vval1.nelements()==1,AipsError);
			AlwaysAssert(vval1(0)==val0,AipsError);
			AlwaysAssert(av1==av0, AipsError);
			AlwaysAssert(!(av1!=av0), AipsError);
//
			Float val2 = 20;
			AttributeValuePoi<Float> av2(&val2, strict);
			av2 = av0;
			AlwaysAssert(av2==av0, AipsError);
//
			Float val2Copy = val2;
			av2.setValue(val2);
			av2 += av0;          // changes val2
			Vector<Float> vval2 = av2.getValue();
			AlwaysAssert(vval2.nelements()==1,AipsError);
			AlwaysAssert(vval2(0)==val2Copy+av0.getValue()(0),AipsError);
//
			AttributeValueBase* p = av0.clone();
			AttributeValuePoi<Float>* p2 = dynamic_cast<AttributeValuePoi<Float>* >(p);
			AlwaysAssert(*(p2)==av0, AipsError);
			delete p;
//
			Vector<Float> val3(2);
			val3(0) = 20;
			val3(1) = 100;
			av0.setValue(val3);                     // Discards second element
			Vector<Float> vval3 = av0.getValue();
			AlwaysAssert(vval3.nelements()==1,AipsError);
			AlwaysAssert(vval3(0)==val3(0), AipsError);
			Float val4 = 30;
			av0.setValue(val4);
			vval3.resize(0);
			vval3 = av0.getValue();
			AlwaysAssert(vval3.nelements()==1,AipsError);
			AlwaysAssert(vval3(0)==val4, AipsError);
//
			Vector<Float> val5(2);
			val5(0) = 2;
			val5(1) = 10;
			AttributeValuePoi<Float> av5(&val5, strict);
			AlwaysAssert(av5.getStrictness()==strict, AipsError);
			AlwaysAssert(av5.getPointerType()==TpArrayFloat, AipsError);
			AlwaysAssert(av5.getType()==AttValue::AtFloat, AipsError);
			Vector<Float> vval5 = av5.getValue();
			AlwaysAssert(vval5.nelements()==2,AipsError);
			AlwaysAssert(vval5(0)==val5(0) && vval5(1)==val5(1), AipsError);
//
			Vector<Float> val6(3);
			val6 = 45;
			av5.setValue(val6);
			Vector<Float> vval6 = av5.getValue();
			AlwaysAssert(vval6.nelements()==3,AipsError);
			AlwaysAssert(vval6(0)==val6(0) && vval6(1)==val6(1) && vval6(2)==val6(2), AipsError);
		}


		cout << "OK" << endl;
		exit(0);
	} catch (const AipsError &x) {
		cerr << "Failed with " << x.getMesg() << endl;
		exit(1);
	}
}

