//# tAttVal.cc: test program for AttVal class
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
#include <display/Display/AttVal.h>


#include <casa/namespace.h>
int main(int, char **) {
	try {


// Int

		{
			Int val0 = 10;
			Bool strict = True;
			AttributeValue<Int> av0(val0, strict);
			AlwaysAssert(av0.getType()==AttValue::AtInt, AipsError);
			AlwaysAssert(av0.getStrictness()==strict, AipsError);
			av0.setStrictness(!strict);
			AlwaysAssert(av0.getStrictness()==!strict, AipsError);
			av0.setStrictness(strict);
			Vector<Int> vval0 = av0.getValue();
			AlwaysAssert(vval0.nelements()==1,AipsError);
			AlwaysAssert(vval0(0)==val0, AipsError);
			Vector<Int> v = av0.getValue();
			AlwaysAssert(v.nelements()==1,AipsError);
			AlwaysAssert(v(0)==val0, AipsError);
//
			AttributeValue<Int> av1(av0);
			AlwaysAssert(av1.getStrictness()==strict, AipsError);
			Vector<Int> vval1 = av1.getValue();
			AlwaysAssert(vval1.nelements()==1,AipsError);
			AlwaysAssert(vval1(0)==val0,AipsError);
			AlwaysAssert(av1==av0, AipsError);
			AlwaysAssert(!(av1!=av0), AipsError);
//
			Int val2 = 20;
			AttributeValue<Int> av2(val2, strict);
			av2 = av0;
			Vector<Int> vval2 = av2.getValue();
			AlwaysAssert(vval2.nelements()==1,AipsError);
			AlwaysAssert(vval2(0)==val0,AipsError);
			AlwaysAssert(av2==av0, AipsError);
			AlwaysAssert(!(av2!=av0), AipsError);
//
			av2.setValue(val2);
			av2 += av0;
			vval2.resize(0);
			vval2 = av2.getValue();
			AlwaysAssert(vval2.nelements()==1,AipsError);
			AlwaysAssert(vval2(0)==val2+val0,AipsError);
//
			av0.setValue(val2);
			vval0 = av0.getValue();
			AlwaysAssert(vval0.nelements()==1,AipsError);
			AlwaysAssert(vval0(0)==val2, AipsError);
//
			AttributeValueBase* p = av0.clone();
			AlwaysAssert(p->getStrictness()==strict,AipsError);
			AttributeValue<Int>* p2 = dynamic_cast<AttributeValue<Int>*>(p);
			Vector<Int> vval2b = p2->getValue();
			AlwaysAssert(vval2b.nelements()==1,AipsError);
			AlwaysAssert(vval2b(0)==val2, AipsError);
			AlwaysAssert(*p==av0, AipsError);
			AlwaysAssert(!(*p!=av0), AipsError);
			delete p;
		}

// uInt

		{
			uInt val0 = 10;
			Bool strict = True;
			AttributeValue<uInt> av0(val0, strict);
			AlwaysAssert(av0.getType()==AttValue::AtuInt, AipsError);
			AlwaysAssert(av0.getStrictness()==strict, AipsError);
			av0.setStrictness(!strict);
			AlwaysAssert(av0.getStrictness()==!strict, AipsError);
			av0.setStrictness(strict);
			Vector<uInt> vval0 = av0.getValue();
			AlwaysAssert(vval0.nelements()==1,AipsError);
			AlwaysAssert(vval0(0)==val0, AipsError);
			Vector<uInt> v = av0.getValue();
			AlwaysAssert(v.nelements()==1,AipsError);
			AlwaysAssert(v(0)==val0, AipsError);
//
			AttributeValue<uInt> av1(av0);
			AlwaysAssert(av1.getStrictness()==strict, AipsError);
			Vector<uInt> vval1 = av1.getValue();
			AlwaysAssert(vval1.nelements()==1,AipsError);
			AlwaysAssert(vval1(0)==val0,AipsError);
			AlwaysAssert(av1==av0, AipsError);
			AlwaysAssert(!(av1!=av0), AipsError);
//
			uInt val2 = 20;
			AttributeValue<uInt> av2(val2, strict);
			av2 = av0;
			Vector<uInt> vval2 = av2.getValue();
			AlwaysAssert(vval2.nelements()==1,AipsError);
			AlwaysAssert(vval2(0)==val0,AipsError);
			AlwaysAssert(av2==av0, AipsError);
			AlwaysAssert(!(av2!=av0), AipsError);
//
			av2.setValue(val2);
			av2 += av0;
			vval2.resize(0);
			vval2 = av2.getValue();
			AlwaysAssert(vval2.nelements()==1,AipsError);
			AlwaysAssert(vval2(0)==val2+val0,AipsError);
//
			av0.setValue(val2);
			vval0 = av0.getValue();
			AlwaysAssert(vval0.nelements()==1,AipsError);
			AlwaysAssert(vval0(0)==val2, AipsError);
//
			AttributeValueBase* p = av0.clone();
			AlwaysAssert(p->getStrictness()==strict,AipsError);
			AttributeValue<uInt>* p2 = dynamic_cast<AttributeValue<uInt>*>(p);
			Vector<uInt> vval2b = p2->getValue();
			AlwaysAssert(vval2b.nelements()==1,AipsError);
			AlwaysAssert(vval2b(0)==val2, AipsError);
			AlwaysAssert(*p==av0, AipsError);
			AlwaysAssert(!(*p!=av0), AipsError);
			delete p;
		}

// Float

		{
			Float val0 = 10;
			Bool strict = True;
			AttributeValue<Float> av0(val0, strict);
			AlwaysAssert(av0.getType()==AttValue::AtFloat, AipsError);
			AlwaysAssert(av0.getStrictness()==strict, AipsError);
			av0.setStrictness(!strict);
			AlwaysAssert(av0.getStrictness()==!strict, AipsError);
			av0.setStrictness(strict);
			Vector<Float> vval0 = av0.getValue();
			AlwaysAssert(vval0.nelements()==1,AipsError);
			AlwaysAssert(vval0(0)==val0, AipsError);
			Vector<Float> v = av0.getValue();
			AlwaysAssert(v.nelements()==1,AipsError);
			AlwaysAssert(v(0)==val0, AipsError);
//
			AttributeValue<Float> av1(av0);
			AlwaysAssert(av1.getStrictness()==strict, AipsError);
			Vector<Float> vval1 = av1.getValue();
			AlwaysAssert(vval1.nelements()==1,AipsError);
			AlwaysAssert(vval1(0)==val0,AipsError);
			AlwaysAssert(av1==av0, AipsError);
			AlwaysAssert(!(av1!=av0), AipsError);
//
			Float val2 = 20;
			AttributeValue<Float> av2(val2, strict);
			av2 = av0;
			Vector<Float> vval2 = av2.getValue();
			AlwaysAssert(vval2.nelements()==1,AipsError);
			AlwaysAssert(vval2(0)==val0,AipsError);
			AlwaysAssert(av2==av0, AipsError);
			AlwaysAssert(!(av2!=av0), AipsError);
//
			av2.setValue(val2);
			av2 += av0;
			vval2.resize(0);
			vval2 = av2.getValue();
			AlwaysAssert(vval2.nelements()==1,AipsError);
			AlwaysAssert(vval2(0)==val2+val0,AipsError);
//
			av0.setValue(val2);
			vval0 = av0.getValue();
			AlwaysAssert(vval0.nelements()==1,AipsError);
			AlwaysAssert(vval0(0)==val2, AipsError);
//
			AttributeValueBase* p = av0.clone();
			AlwaysAssert(p->getStrictness()==strict,AipsError);
			AttributeValue<Float>* p2 = dynamic_cast<AttributeValue<Float>*>(p);
			Vector<Float> vval2b = p2->getValue();
			AlwaysAssert(vval2b.nelements()==1,AipsError);
			AlwaysAssert(vval2b(0)==val2, AipsError);
			AlwaysAssert(*p==av0, AipsError);
			AlwaysAssert(!(*p!=av0), AipsError);
			delete p;
		}

// Double

		{
			Double val0 = 10;
			Bool strict = True;
			AttributeValue<Double> av0(val0, strict);
			AlwaysAssert(av0.getType()==AttValue::AtDouble, AipsError);
			AlwaysAssert(av0.getStrictness()==strict, AipsError);
			av0.setStrictness(!strict);
			AlwaysAssert(av0.getStrictness()==!strict, AipsError);
			av0.setStrictness(strict);
			Vector<Double> vval0 = av0.getValue();
			AlwaysAssert(vval0.nelements()==1,AipsError);
			AlwaysAssert(vval0(0)==val0, AipsError);
			Vector<Double> v = av0.getValue();
			AlwaysAssert(v.nelements()==1,AipsError);
			AlwaysAssert(v(0)==val0, AipsError);
//
			AttributeValue<Double> av1(av0);
			AlwaysAssert(av1.getStrictness()==strict, AipsError);
			Vector<Double> vval1 = av1.getValue();
			AlwaysAssert(vval1.nelements()==1,AipsError);
			AlwaysAssert(vval1(0)==val0,AipsError);
			AlwaysAssert(av1==av0, AipsError);
			AlwaysAssert(!(av1!=av0), AipsError);
//
			Double val2 = 20;
			AttributeValue<Double> av2(val2, strict);
			av2 = av0;
			Vector<Double> vval2 = av2.getValue();
			AlwaysAssert(vval2.nelements()==1,AipsError);
			AlwaysAssert(vval2(0)==val0,AipsError);
			AlwaysAssert(av2==av0, AipsError);
			AlwaysAssert(!(av2!=av0), AipsError);
//
			av2.setValue(val2);
			av2 += av0;
			vval2.resize(0);
			vval2 = av2.getValue();
			AlwaysAssert(vval2.nelements()==1,AipsError);
			AlwaysAssert(vval2(0)==val2+val0,AipsError);
//
			av0.setValue(val2);
			vval0 = av0.getValue();
			AlwaysAssert(vval0.nelements()==1,AipsError);
			AlwaysAssert(vval0(0)==val2, AipsError);
//
			AttributeValueBase* p = av0.clone();
			AlwaysAssert(p->getStrictness()==strict,AipsError);
			AttributeValue<Double>* p2 = dynamic_cast<AttributeValue<Double>*>(p);
			Vector<Double> vval2b = p2->getValue();
			AlwaysAssert(vval2b.nelements()==1,AipsError);
			AlwaysAssert(vval2b(0)==val2, AipsError);
			AlwaysAssert(*p==av0, AipsError);
			AlwaysAssert(!(*p!=av0), AipsError);
			delete p;
		}

// String

		{
			String val0 = "10";
			Bool strict = True;
			AttributeValue<String> av0(val0, strict);
			AlwaysAssert(av0.getType()==AttValue::AtString, AipsError);
			AlwaysAssert(av0.getStrictness()==strict, AipsError);
			av0.setStrictness(!strict);
			AlwaysAssert(av0.getStrictness()==!strict, AipsError);
			av0.setStrictness(strict);
			Vector<String> vval0 = av0.getValue();
			AlwaysAssert(vval0.nelements()==1,AipsError);
			AlwaysAssert(vval0(0)==val0, AipsError);
			Vector<String> v = av0.getValue();
			AlwaysAssert(v.nelements()==1,AipsError);
			AlwaysAssert(v(0)==val0, AipsError);
//
			AttributeValue<String> av1(av0);
			AlwaysAssert(av1.getStrictness()==strict, AipsError);
			Vector<String> vval1 = av1.getValue();
			AlwaysAssert(vval1.nelements()==1,AipsError);
			AlwaysAssert(vval1(0)==val0,AipsError);
			AlwaysAssert(av1==av0, AipsError);
			AlwaysAssert(!(av1!=av0), AipsError);
//
			String val2 = "20";
			AttributeValue<String> av2(val2, strict);
			av2 = av0;
			Vector<String> vval2 = av2.getValue();
			AlwaysAssert(vval2.nelements()==1,AipsError);
			AlwaysAssert(vval2(0)==val0,AipsError);
			AlwaysAssert(av2==av0, AipsError);
			AlwaysAssert(!(av2!=av0), AipsError);
//
			av2.setValue(val2);
			av2 += av0;
			vval2.resize(0);
			vval2 = av2.getValue();
			AlwaysAssert(vval2.nelements()==1,AipsError);
			AlwaysAssert(vval2(0)==val2+val0,AipsError);
//
			av0.setValue(val2);
			vval0 = av0.getValue();
			AlwaysAssert(vval0.nelements()==1,AipsError);
			AlwaysAssert(vval0(0)==val2, AipsError);
//
			AttributeValueBase* p = av0.clone();
			AlwaysAssert(p->getStrictness()==strict,AipsError);
			AttributeValue<String>* p2 = dynamic_cast<AttributeValue<String>*>(p);
			Vector<String> vval2b = p2->getValue();
			AlwaysAssert(vval2b.nelements()==1,AipsError);
			AlwaysAssert(vval2b(0)==val2, AipsError);
			AlwaysAssert(*p==av0, AipsError);
			AlwaysAssert(!(*p!=av0), AipsError);
			delete p;
		}

// Bool

		{
			Bool val0 = True;
			Bool strict = True;
			AttributeValue<Bool> av0(val0, strict);
			AlwaysAssert(av0.getType()==AttValue::AtBool, AipsError);
			AlwaysAssert(av0.getStrictness()==strict, AipsError);
			av0.setStrictness(!strict);
			AlwaysAssert(av0.getStrictness()==!strict, AipsError);
			av0.setStrictness(strict);
			Vector<Bool> vval0 = av0.getValue();
			AlwaysAssert(vval0.nelements()==1,AipsError);
			AlwaysAssert(vval0(0)==val0, AipsError);
			Vector<Bool> v = av0.getValue();
			AlwaysAssert(v.nelements()==1,AipsError);
			AlwaysAssert(v(0)==val0, AipsError);
//
			AttributeValue<Bool> av1(av0);
			AlwaysAssert(av1.getStrictness()==strict, AipsError);
			Vector<Bool> vval1 = av1.getValue();
			AlwaysAssert(vval1.nelements()==1,AipsError);
			AlwaysAssert(vval1(0)==val0,AipsError);
			AlwaysAssert(av1==av0, AipsError);
			AlwaysAssert(!(av1!=av0), AipsError);
//
			Bool val2 = False;
			AttributeValue<Bool> av2(val2, strict);
			av2 = av0;
			Vector<Bool> vval2 = av2.getValue();
			AlwaysAssert(vval2.nelements()==1,AipsError);
			AlwaysAssert(vval2(0)==val0,AipsError);
			AlwaysAssert(av2==av0, AipsError);
			AlwaysAssert(!(av2!=av0), AipsError);
//
			av2.setValue(val2);
			av2 += av0;
			vval2.resize(0);
			vval2 = av2.getValue();
			AlwaysAssert(vval2.nelements()==1,AipsError);
			AlwaysAssert(vval2(0)==val2+val0,AipsError);
//
			av0.setValue(val2);
			vval0 = av0.getValue();
			AlwaysAssert(vval0.nelements()==1,AipsError);
			AlwaysAssert(vval0(0)==val2, AipsError);
//
			AttributeValueBase* p = av0.clone();
			AlwaysAssert(p->getStrictness()==strict,AipsError);
			AttributeValue<Bool>* p2 = dynamic_cast<AttributeValue<Bool>*>(p);
			Vector<Bool> vval2b = p2->getValue();
			AlwaysAssert(vval2b.nelements()==1,AipsError);
			AlwaysAssert(vval2b(0)==val2, AipsError);
			AlwaysAssert(*p==av0, AipsError);
			AlwaysAssert(!(*p!=av0), AipsError);
			delete p;
		}

//
		cout << "OK" << endl;
		exit(0);
	} catch (const AipsError &x) {
		cerr << "Failed with " << x.getMesg() << endl;
		exit(1);
	}
}

