//# tAttribute.cc: test program for Attribute class
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
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Exceptions/Error.h>
#include <casa/Utilities/Assert.h>
#include <display/Display/Attribute.h>
#include <display/Display/AttVal.h>
#include <casa/Quanta/QLogical.h>

#include <casa/namespace.h>
void localAbort(String message) {
	cout << message << endl;
	exit(1);
}

int main(int, char **) {
	try {

		Attribute *att1(0), *att2(0);
		String att1name, att2name;

		att1name = "frogs";
		att2name = "another name with spaces in it";

		// ------------------------------------------------------------
		// Test uInt Attributes

		uInt uIntVal1(2), uIntVal2(8), uIntTol1(5), uIntTol2(7);
		uInt uIntVal2original(uIntVal2);
		Vector<uInt> uIntVecVal1(3);
		uIntVecVal1(0) = 2;
		uIntVecVal1(1) = 6;
		uIntVecVal1(2) = 9;
		Vector<uInt> uIntVecVal2(3);
		uIntVecVal2 = 0;
		Vector<uInt> uIntVecVal2original;

		// test basic construction and Attribute validity
		att1 = new Attribute(att1name, uIntVal1);
		AlwaysAssert(att1, AipsError);
		AlwaysAssert(att1->getName() == att1name, AipsError);
		AlwaysAssert(att1->getType() == AttValue::AtuInt, AipsError);
		AttributeValue<uInt>* t1 = dynamic_cast<AttributeValue<uInt>* >(att1->getAttributeValue());
		AlwaysAssert(t1->getValue()(0)==uIntVal1, AipsError);
		AlwaysAssert(*att1 == *att1, AipsError);

		// test cloning
		att2 = att1->clone();
		AlwaysAssert(att2, AipsError);
		AlwaysAssert(att2->getName() == att1name, AipsError);
		AlwaysAssert(att2->getType() == AttValue::AtuInt, AipsError);
		AlwaysAssert(!(*att1 != *att2), AipsError);
		{
			t1 = dynamic_cast<AttributeValue<uInt>* >(att1->getAttributeValue());
			AttributeValue<uInt>* t2 = dynamic_cast<AttributeValue<uInt>* >(att2->getAttributeValue());
			AlwaysAssert(t1->getValue()(0)==t2->getValue()(0), AipsError);
		}

		// test operators
		delete att2;
		att2 = new Attribute(att1name, uIntVal2);
		AlwaysAssert(*att1 != *att2, AipsError);
		att2->setValue(*att1);
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att2;

		// test tolerance in comparisons
		att2 = new Attribute(att1name, uIntVal2, uIntTol2);
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att1name, uIntVal1, uIntTol1);
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att2name, uIntVal1, uIntTol2);
		AlwaysAssert(*att1 == *att2, AipsError);

		// test Attribute arithmetic: addition
		delete att1;
		att1 = new Attribute(att1name, uIntVal1);
		delete att2;
		att2 = new Attribute(att1name, uIntVal2);
		*att2 += *att1;
		{
			AttributeValue<uInt>* t1 = dynamic_cast<AttributeValue<uInt>* >(att2->getAttributeValue());
			AlwaysAssert(t1->getValue()(0) == (uIntVal1 + uIntVal2),  AipsError);
		}
//
		delete att1;
		delete att2;


		// test Vector<uInt>
		att1 = new Attribute(att1name, uIntVal1); // no tolerance, non-strict
		att2 = new Attribute(att1name, uIntVecVal1); // ditto
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att2;
		att2 = new Attribute(att1name, uIntVecVal1, uInt(0), True); // notol, strict
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att1name, uIntVal1, uInt(0), True); // no tol, strict
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att2;
		att2 = new Attribute(att1name, uIntVecVal1); // no tol, non-strict again
		AlwaysAssert(*att1 != *att2, AipsError);

		// test Vector<uInt> arithmetic: addition
		delete att1;
		delete att2;
		att1 = new Attribute(att1name, uIntVecVal1); // ditto
		att2 = new Attribute(att1name, uIntVecVal1); // ditto
		*att2 += *att1;
		{
			AttributeValue<uInt>* t1 = dynamic_cast<AttributeValue<uInt>* >(att2->getAttributeValue());
			AlwaysAssert(allEQ(t1->getValue(), uIntVecVal1 + uIntVecVal1), AipsError);
		}

		// ... all again, but with ptr constructors...
		// test basic construction and Attribute validity
		att1 = new Attribute(att1name, &uIntVal1);
		AlwaysAssert(att1, AipsError);
		AlwaysAssert(att1->getName() == att1name, AipsError);
		AlwaysAssert(att1->getType() == AttValue::AtuInt, AipsError);
		{
			AttributeValue<uInt>* t1 = dynamic_cast<AttributeValue<uInt>* >(att1->getAttributeValue());
			AlwaysAssert(t1->getValue()(0)==uIntVal1, AipsError);
		}
		AlwaysAssert(*att1 == *att1, AipsError);

		// test cloning
		att2 = att1->clone();
		AlwaysAssert(att2, AipsError);
		AlwaysAssert(att2->getName() == att1name, AipsError);
		AlwaysAssert(att2->getType() == AttValue::AtuInt, AipsError);
		AlwaysAssert(!(*att1 != *att2), AipsError);
		{
			AttributeValue<uInt>* t1 = dynamic_cast<AttributeValue<uInt>* >(att1->getAttributeValue());
			AttributeValue<uInt>* t2 = dynamic_cast<AttributeValue<uInt>* >(att2->getAttributeValue());
			AlwaysAssert(t1->getValue()(0)==t2->getValue()(0), AipsError);
		}

		// test operators
		delete att2;
		att2 = new Attribute(att1name, &uIntVal2);
		AlwaysAssert(*att1 != *att2, AipsError);
		att2->setValue(*att1);
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att2;

		// above setValue has modified att2, and therefore uIntVal2, so now
		// the next test as in above section would fail, eg.:
		att2 = new Attribute(att1name, &uIntVal2, uIntTol2);
		AlwaysAssert(*att1 == *att2, AipsError);
		// and set uIntVal2 back to original:
		uIntVal2 = uIntVal2original;

		// test tolerance in comparisons
		att2 = new Attribute(att1name, &uIntVal2, uIntTol2);
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att1name, &uIntVal1, uIntTol1);
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att2name, &uIntVal1, uIntTol2);
		AlwaysAssert(*att1 == *att2, AipsError);

		// test Attribute arithmetic: addition
		delete att1;
		att1 = new Attribute(att1name, &uIntVal1);
		delete att2;
		att2 = new Attribute(att1name, &uIntVal2);
		// store old value because it will be over-written!
		uIntVal2original = uIntVal2;
		*att2 += *att1;
		{
			AttributeValue<uInt>* t2 = dynamic_cast<AttributeValue<uInt>* >(att2->getAttributeValue());
			AlwaysAssert(t2->getValue()(0)==(uIntVal1 + uIntVal2original), AipsError);
		}

		// test Vector<uInt>
		delete att1;
		delete att2;
		att1 = new Attribute(att1name, &uIntVal1); // no tolerance, non-strict
		att2 = new Attribute(att1name, &uIntVecVal1); // ditto
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att2;
		att2 = new Attribute(att1name, &uIntVecVal1, uInt(0),
		                     True); // notol, strict
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att1name, &uIntVal1, uInt(0), True); // no tol, strict
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att2;
		att2 = new Attribute(att1name, &uIntVecVal1); // no tol, non-strict again
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		delete att2;

		// test Vector<uInt> arithmetic: addition
		att1 = new Attribute(att1name, &uIntVecVal1); // ditto
		att2 = new Attribute(att1name, &uIntVecVal2); // ditto
		uIntVecVal2original = uIntVecVal2;
		*att2 += *att1;
		{
			AttributeValue<uInt>* t2 = dynamic_cast<AttributeValue<uInt>* >(att2->getAttributeValue());
			AlwaysAssert(allEQ(t2->getValue(), (uIntVecVal1 + uIntVecVal2original)), AipsError);
		}
		delete att1;
		delete att2;

		// ------------------------------------------------------------
		// Test Int Attributes

		Int IntVal1(2), IntVal2(8), IntTol1(5), IntTol2(7);
		Int IntVal2original(IntVal2);
		Vector<Int> IntVecVal(3);
		IntVecVal(0) = 2;
		IntVecVal(1) = 6;
		IntVecVal(2) = 9;

		// test basic construction and Attribute validity
		att1 = new Attribute(att1name, IntVal1);
		AlwaysAssert(att1, AipsError);
		AlwaysAssert(att1->getName() == att1name, AipsError);
		AlwaysAssert(att1->getType() == AttValue::AtInt, AipsError);
		{
			AttributeValue<Int>* t1 = dynamic_cast<AttributeValue<Int>* >(att1->getAttributeValue());
			AlwaysAssert(t1->getValue()(0)==IntVal1, AipsError);
		}
		AlwaysAssert(*att1 == *att1, AipsError);

		// test cloning
		att2 = att1->clone();
		AlwaysAssert(att2, AipsError);
		AlwaysAssert(att2->getName() == att1name, AipsError);
		AlwaysAssert(att2->getType() == AttValue::AtInt, AipsError);
		AlwaysAssert(!(*att1 != *att2), AipsError);
		{
			AttributeValue<Int>* t1 = dynamic_cast<AttributeValue<Int>* >(att1->getAttributeValue());
			AttributeValue<Int>* t2 = dynamic_cast<AttributeValue<Int>* >(att2->getAttributeValue());
			AlwaysAssert(t1->getValue()(0) == t2->getValue()(0), AipsError);
		}

		// test operators
		delete att2;
		att2 = new Attribute(att1name, IntVal2);
		AlwaysAssert(*att1 != *att2, AipsError);
		att2->setValue(*att1);
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att2;

		// test tolerance in comparisons
		att2 = new Attribute(att1name, IntVal2, IntTol2);
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att1name, IntVal1, IntTol1);
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att2name, IntVal1, IntTol2);
		AlwaysAssert(*att1 == *att2, AipsError);

		// test Attribute arithmetic: addition
		delete att1;
		att1 = new Attribute(att1name, IntVal1);
		delete att2;
		att2 = new Attribute(att1name, IntVal2);
		*att2 += *att1;
		{
			AttributeValue<Int>* t2 = dynamic_cast<AttributeValue<Int>* >(att2->getAttributeValue());
			AlwaysAssert(t2->getValue()(0)==(IntVal1 + IntVal2), AipsError);
		}

		// test Vector<Int>
		delete att1;
		delete att2;
		att1 = new Attribute(att1name, IntVal1); // no tolerance, non-strict
		att2 = new Attribute(att1name, IntVecVal); // ditto
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att2;
		att2 = new Attribute(att1name, IntVecVal, Int(0), True); // notol, strict
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att1name, IntVal1, Int(0), True); // no tol, strict
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att2;
		att2 = new Attribute(att1name, IntVecVal); // no tol, non-strict again
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		delete att2;

		// ... all again, but with ptr constructors...
		// test basic construction and Attribute validity
		att1 = new Attribute(att1name, &IntVal1);
		AlwaysAssert(att1, AipsError);
		AlwaysAssert(att1->getName() == att1name, AipsError);
		AlwaysAssert(att1->getType() == AttValue::AtInt, AipsError);
		{
			AttributeValue<Int>* t1 = dynamic_cast<AttributeValue<Int>* >(att1->getAttributeValue());
			AlwaysAssert(t1->getValue()(0)==IntVal1, AipsError);
		}
		AlwaysAssert(*att1 == *att1, AipsError);

		// test cloning
		att2 = att1->clone();
		AlwaysAssert(att2, AipsError);
		AlwaysAssert(att2->getName() == att1name, AipsError);
		AlwaysAssert(att2->getType() == AttValue::AtInt, AipsError);
		AlwaysAssert(!(*att1 != *att2), AipsError);
		{
			AttributeValue<Int>* t1 = dynamic_cast<AttributeValue<Int>* >(att1->getAttributeValue());
			AttributeValue<Int>* t2 = dynamic_cast<AttributeValue<Int>* >(att2->getAttributeValue());
			AlwaysAssert(t1->getValue()(0)==t2->getValue()(0), AipsError);
		}

		// test operators
		delete att2;
		att2 = new Attribute(att1name, &IntVal2);
		AlwaysAssert(*att1 != *att2, AipsError);
		att2->setValue(*att1);
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att2;

		// above setValue has modified att2, and therefore IntVal2, so now
		// the next test as in above section would fail, eg.:
		att2 = new Attribute(att1name, &IntVal2, IntTol2);
		AlwaysAssert(*att1 == *att2, AipsError);
		// and set IntVal2 back to original:
		IntVal2 = IntVal2original;

		// test tolerance in comparisons
		att2 = new Attribute(att1name, &IntVal2, IntTol2);
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att1name, &IntVal1, IntTol1);
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att2name, &IntVal1, IntTol2);
		AlwaysAssert(*att1 == *att2, AipsError);

		// test Attribute arithmetic: addition
		delete att1;
		att1 = new Attribute(att1name, &IntVal1);
		delete att2;
		att2 = new Attribute(att1name, &IntVal2);
		// store old value because it will be over-written!
		IntVal2original = IntVal2;
		*att2 += *att1;
		{
			AttributeValue<Int>* t2 = dynamic_cast<AttributeValue<Int>* >(att2->getAttributeValue());
			AlwaysAssert(t2->getValue()(0)==(IntVal1 + IntVal2original),AipsError);
		}

		// test Vector<Int>
		delete att1;
		delete att2;
		att1 = new Attribute(att1name, &IntVal1); // no tolerance, non-strict
		att2 = new Attribute(att1name, &IntVecVal); // ditto
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att2;
		att2 = new Attribute(att1name, &IntVecVal, Int(0),
		                     True); // notol, strict
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att1name, &IntVal1, Int(0), True); // no tol, strict
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att2;
		att2 = new Attribute(att1name, &IntVecVal); // no tol, non-strict again
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		delete att2;


		// ------------------------------------------------------------
		// Test Float Attributes

		Float FloatVal1(2), FloatVal2(8), FloatTol1(5), FloatTol2(7);
		Float FloatVal2original(FloatVal2);
		Vector<Float> FloatVecVal(3);
		FloatVecVal(0) = 2;
		FloatVecVal(1) = 6;
		FloatVecVal(2) = 9;

		// test basic construction and Attribute validity
		att1 = new Attribute(att1name, FloatVal1);
		AlwaysAssert(att1, AipsError);
		AlwaysAssert(att1->getName() == att1name, AipsError);
		AlwaysAssert(att1->getType() == AttValue::AtFloat, AipsError);
		{
			AttributeValue<Float>* t1 = dynamic_cast<AttributeValue<Float>* >(att1->getAttributeValue());
			AlwaysAssert(t1->getValue()(0)==FloatVal1,AipsError);
		}
		AlwaysAssert(*att1 == *att1, AipsError);

		// test cloning
		att2 = att1->clone();
		AlwaysAssert(att2, AipsError);
		AlwaysAssert(att2->getName() == att1name, AipsError);
		AlwaysAssert(att2->getType() == AttValue::AtFloat, AipsError);
		AlwaysAssert(!(*att1 != *att2), AipsError);
		{
			AttributeValue<Float>* t1 = dynamic_cast<AttributeValue<Float>* >(att1->getAttributeValue());
			AttributeValue<Float>* t2 = dynamic_cast<AttributeValue<Float>* >(att2->getAttributeValue());
			AlwaysAssert(t1->getValue()(0)==t2->getValue()(0), AipsError);
		}

		// test operators
		delete att2;
		att2 = new Attribute(att1name, FloatVal2);
		AlwaysAssert(*att1 != *att2, AipsError);
		att2->setValue(*att1);
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att2;

		// test tolerance in comparisons
		att2 = new Attribute(att1name, FloatVal2, FloatTol2);
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att1name, FloatVal1, FloatTol1);
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att2name, FloatVal1, FloatTol2);
		AlwaysAssert(*att1 == *att2, AipsError);

		// test Vector<Float>
		delete att1;
		delete att2;
		att1 = new Attribute(att1name, FloatVal1); // no tolerance, non-strict
		att2 = new Attribute(att1name, FloatVecVal); // ditto
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att2;
		att2 = new Attribute(att1name, FloatVecVal, Float(0), True);
		// notol, strict
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att1name, FloatVal1, Float(0), True);
		// no tol, strict
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att2;
		att2 = new Attribute(att1name, FloatVecVal); // no tol, non-strict again
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		delete att2;

		// ... all again, but with ptr constructors...
		// test basic construction and Attribute validity
		att1 = new Attribute(att1name, &FloatVal1);
		AlwaysAssert(att1, AipsError);
		AlwaysAssert(att1->getName() == att1name, AipsError);
		AlwaysAssert(att1->getType() == AttValue::AtFloat, AipsError);
		{
			AttributeValue<Float>* t1 = dynamic_cast<AttributeValue<Float>* >(att1->getAttributeValue());
			AlwaysAssert(t1->getValue()(0)==FloatVal1, AipsError);
		}
		AlwaysAssert(*att1 == *att1, AipsError);

		// test cloning
		att2 = att1->clone();
		AlwaysAssert(att2, AipsError);
		AlwaysAssert(att2->getName() == att1name, AipsError);
		AlwaysAssert(att2->getType() == AttValue::AtFloat, AipsError);
		AlwaysAssert(!(*att1 != *att2), AipsError);
		{
			AttributeValue<Float>* t1 = dynamic_cast<AttributeValue<Float>* >(att1->getAttributeValue());
			AttributeValue<Float>* t2 = dynamic_cast<AttributeValue<Float>* >(att2->getAttributeValue());
			AlwaysAssert(t1->getValue()(0)==t2->getValue()(0), AipsError);
		}

		// test operators
		delete att2;
		att2 = new Attribute(att1name, &FloatVal2);
		AlwaysAssert(*att1 != *att2, AipsError);
		att2->setValue(*att1);
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att2;

		// above setValue has modified att2, and therefore FloatVal2, so now
		// the next test as in above section would fail, eg.:
		att2 = new Attribute(att1name, &FloatVal2, FloatTol2);
		AlwaysAssert(*att1 == *att2, AipsError);
		// and set FloatVal2 back to original:
		FloatVal2 = FloatVal2original;

		// test tolerance in comparisons
		att2 = new Attribute(att1name, &FloatVal2, FloatTol2);
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att1name, &FloatVal1, FloatTol1);
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att2name, &FloatVal1, FloatTol2);
		AlwaysAssert(*att1 == *att2, AipsError);

		// test Vector<Float>
		delete att1;
		delete att2;
		att1 = new Attribute(att1name, &FloatVal1); // no tolerance, non-strict
		att2 = new Attribute(att1name, &FloatVecVal); // ditto
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att2;
		att2 = new Attribute(att1name, &FloatVecVal, Float(0),
		                     True); // notol, strict
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att1name, &FloatVal1, Float(0), True);
		// no tol, strict
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att2;
		att2 = new Attribute(att1name, &FloatVecVal); // no tol, non-strict again
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		delete att2;


		// ------------------------------------------------------------
		// Test Double Attributes

		Double DoubleVal1(2), DoubleVal2(8), DoubleTol1(5), DoubleTol2(7);
		Double DoubleVal2original(DoubleVal2);
		Vector<Double> DoubleVecVal(3);
		DoubleVecVal(0) = 2;
		DoubleVecVal(1) = 6;
		DoubleVecVal(2) = 9;

		// test basic construction and Attribute validity
		att1 = new Attribute(att1name, DoubleVal1);
		AlwaysAssert(att1, AipsError);
		AlwaysAssert(att1->getName() == att1name, AipsError);
		AlwaysAssert(att1->getType() == AttValue::AtDouble, AipsError);
		{
			AttributeValue<Double>* t1 = dynamic_cast<AttributeValue<Double>* >(att1->getAttributeValue());
			AlwaysAssert(t1->getValue()(0)==DoubleVal1, AipsError);
		}
		AlwaysAssert(*att1 == *att1, AipsError);

		// test cloning
		att2 = att1->clone();
		AlwaysAssert(att2, AipsError);
		AlwaysAssert(att2->getName() == att1name, AipsError);
		AlwaysAssert(att2->getType() == AttValue::AtDouble, AipsError);
		AlwaysAssert(!(*att1 != *att2), AipsError);
		{
			AttributeValue<Double>* t1 = dynamic_cast<AttributeValue<Double>* >(att1->getAttributeValue());
			AttributeValue<Double>* t2 = dynamic_cast<AttributeValue<Double>* >(att2->getAttributeValue());
			AlwaysAssert(t1->getValue()(0)==t2->getValue()(0), AipsError);
		}

		// test operators
		delete att2;
		att2 = new Attribute(att1name, DoubleVal2);
		AlwaysAssert(*att1 != *att2, AipsError);
		att2->setValue(*att1);
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att2;

		// test tolerance in comparisons
		att2 = new Attribute(att1name, DoubleVal2, DoubleTol2);
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att1name, DoubleVal1, DoubleTol1);
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att2name, DoubleVal1, DoubleTol2);
		AlwaysAssert(*att1 == *att2, AipsError);

		// test Vector<Double>
		delete att1;
		delete att2;
		att1 = new Attribute(att1name, DoubleVal1); // no tolerance, non-strict
		att2 = new Attribute(att1name, DoubleVecVal); // ditto
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att2;
		att2 = new Attribute(att1name, DoubleVecVal, Double(0), True);
		// notol, strict
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att1name, DoubleVal1, Double(0), True);
		// no tol, strict
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att2;
		att2 = new Attribute(att1name, DoubleVecVal); // no tol, non-strict again
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		delete att2;

		// ... all again, but with ptr constructors...
		// test basic construction and Attribute validity
		att1 = new Attribute(att1name, &DoubleVal1);
		AlwaysAssert(att1, AipsError);
		AlwaysAssert(att1->getName() == att1name, AipsError);
		AlwaysAssert(att1->getType() == AttValue::AtDouble, AipsError);
		{
			AttributeValue<Double>* t1 = dynamic_cast<AttributeValue<Double>* >(att1->getAttributeValue());
			AlwaysAssert(t1->getValue()(0)==DoubleVal1, AipsError);
		}
		AlwaysAssert(*att1 == *att1, AipsError);

		// test cloning
		att2 = att1->clone();
		AlwaysAssert(att2, AipsError);
		AlwaysAssert(att2->getName() == att1name, AipsError);
		AlwaysAssert(att2->getType() == AttValue::AtDouble, AipsError);
		AlwaysAssert(!(*att1 != *att2), AipsError);
		{
			AttributeValue<Double>* t1 = dynamic_cast<AttributeValue<Double>* >(att1->getAttributeValue());
			AttributeValue<Double>* t2 = dynamic_cast<AttributeValue<Double>* >(att2->getAttributeValue());
			AlwaysAssert(t1->getValue()(0)==t2->getValue()(0), AipsError);
		}

		// test operators
		delete att2;
		att2 = new Attribute(att1name, &DoubleVal2);
		AlwaysAssert(*att1 != *att2, AipsError);
		att2->setValue(*att1);
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att2;

		// above setValue has modified att2, and therefore DoubleVal2, so now
		// the next test as in above section would fail, eg.:
		att2 = new Attribute(att1name, &DoubleVal2, DoubleTol2);
		AlwaysAssert(*att1 == *att2, AipsError);
		// and set DoubleVal2 back to original:
		DoubleVal2 = DoubleVal2original;

		// test tolerance in comparisons
		att2 = new Attribute(att1name, &DoubleVal2, DoubleTol2);
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att1name, &DoubleVal1, DoubleTol1);
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att2name, &DoubleVal1, DoubleTol2);
		AlwaysAssert(*att1 == *att2, AipsError);

		// test Vector<Double>
		delete att1;
		delete att2;
		att1 = new Attribute(att1name, &DoubleVal1); // no tolerance, non-strict
		att2 = new Attribute(att1name, &DoubleVecVal); // ditto
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att2;
		att2 = new Attribute(att1name, &DoubleVecVal, Double(0),
		                     True); // notol, strict
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att1name, &DoubleVal1, Double(0), True);
		// no tol, strict
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att2;
		att2 = new Attribute(att1name, &DoubleVecVal); // no tol, non-strict again
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		delete att2;


		// ------------------------------------------------------------
		// Test Bool Attributes

		Bool BoolVal1(True), BoolVal2(False);
		Bool BoolVal2original(BoolVal2);
		Vector<Bool> BoolVecVal(3);
		BoolVecVal(0) = False;
		BoolVecVal(1) = True;
		BoolVecVal(2) = False;

		// test basic construction and Attribute validity
		att1 = new Attribute(att1name, BoolVal1);
		AlwaysAssert(att1, AipsError);
		AlwaysAssert(att1->getName() == att1name, AipsError);
		AlwaysAssert(att1->getType() == AttValue::AtBool, AipsError);
		{
			AttributeValue<Bool>* t1 = dynamic_cast<AttributeValue<Bool>* >(att1->getAttributeValue());
			AlwaysAssert(t1->getValue()(0)==BoolVal1, AipsError);
		}
		AlwaysAssert(*att1 == *att1, AipsError);

		// test cloning
		att2 = att1->clone();
		AlwaysAssert(att2, AipsError);
		AlwaysAssert(att2->getName() == att1name, AipsError);
		AlwaysAssert(att2->getType() == AttValue::AtBool, AipsError);
		AlwaysAssert(!(*att1 != *att2), AipsError);
		{
			AttributeValue<Bool>* t1 = dynamic_cast<AttributeValue<Bool>* >(att1->getAttributeValue());
			AttributeValue<Bool>* t2 = dynamic_cast<AttributeValue<Bool>* >(att2->getAttributeValue());
			AlwaysAssert(t1->getValue()(0)==t2->getValue()(0), AipsError);
		}

		// test operators
		delete att2;
		att2 = new Attribute(att1name, BoolVal2);
		AlwaysAssert(*att1 != *att2, AipsError);
		att2->setValue(*att1);
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att2;

		// test Vector<Bool>
		delete att1;
		att1 = new Attribute(att1name, BoolVal1); // no tolerance, non-strict
		att2 = new Attribute(att1name, BoolVecVal); // ditto
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att2;
		att2 = new Attribute(att1name, BoolVecVal, True); // strict
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att1name, BoolVal1, True); // strict
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att2;
		att2 = new Attribute(att1name, BoolVecVal); // non-strict again
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		delete att2;

		// ... all again, but with ptr constructors...
		// test basic construction and Attribute validity
		att1 = new Attribute(att1name, &BoolVal1);
		AlwaysAssert(att1, AipsError);
		AlwaysAssert(att1->getName() == att1name, AipsError);
		AlwaysAssert(att1->getType() == AttValue::AtBool, AipsError);
		{
			AttributeValue<Bool>* t1 = dynamic_cast<AttributeValue<Bool>* >(att1->getAttributeValue());
			AlwaysAssert(t1->getValue()(0)==BoolVal1, AipsError);
		}
		AlwaysAssert(*att1 == *att1, AipsError);

		// test cloning
		att2 = att1->clone();
		AlwaysAssert(att2, AipsError);
		AlwaysAssert(att2->getName() == att1name, AipsError);
		AlwaysAssert(att2->getType() == AttValue::AtBool, AipsError);
		AlwaysAssert(!(*att1 != *att2), AipsError);
		{
			AttributeValue<Bool>* t1 = dynamic_cast<AttributeValue<Bool>* >(att1->getAttributeValue());
			AttributeValue<Bool>* t2 = dynamic_cast<AttributeValue<Bool>* >(att2->getAttributeValue());
			AlwaysAssert(t1->getValue()(0)==t2->getValue()(0), AipsError);
		}

		// test operators
		delete att2;
		att2 = new Attribute(att1name, &BoolVal2);
		AlwaysAssert(*att1 != *att2, AipsError);
		att2->setValue(*att1);
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att2;

		// BoolVal2 has changed because of aliasing.
		AlwaysAssert(BoolVal2 != BoolVal2original, AipsError);
		// set BoolVal2 back to original:
		BoolVal2 = BoolVal2original;

		// test Vector<Bool>
		delete att1;
		att1 = new Attribute(att1name, &BoolVal1); // non-strict
		att2 = new Attribute(att1name, &BoolVecVal); // ditto
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att2;
		att2 = new Attribute(att1name, &BoolVecVal, True); // strict
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att1name, &BoolVal1, True); // strict
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att2;
		att2 = new Attribute(att1name, &BoolVecVal); // non-strict again
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		delete att2;


		// ------------------------------------------------------------
		// Test String Attributes

		String StringVal1("True"), StringVal2("False");
		String StringVal2original(StringVal2);
		Vector<String> StringVecVal(3);
		StringVecVal(0) = "Maybe";
		StringVecVal(1) = "True";
		StringVecVal(2) = "False";

		// test basic construction and Attribute validity
		att1 = new Attribute(att1name, StringVal1);
		AlwaysAssert(att1, AipsError);
		AlwaysAssert(att1->getName() == att1name, AipsError);
		AlwaysAssert(att1->getType() == AttValue::AtString, AipsError);
		{
			AttributeValue<String>* t1 = dynamic_cast<AttributeValue<String>* >(att1->getAttributeValue());
			AlwaysAssert(t1->getValue()(0)==StringVal1, AipsError);
		}
		AlwaysAssert(*att1 == *att1, AipsError);

		// test cloning
		att2 = att1->clone();
		AlwaysAssert(att2, AipsError);
		AlwaysAssert(att2->getName() == att1name, AipsError);
		AlwaysAssert(att2->getType() == AttValue::AtString, AipsError);
		AlwaysAssert(!(*att1 != *att2), AipsError);
		{
			AttributeValue<String>* t1 = dynamic_cast<AttributeValue<String>* >(att1->getAttributeValue());
			AttributeValue<String>* t2 = dynamic_cast<AttributeValue<String>* >(att2->getAttributeValue());
			AlwaysAssert(t1->getValue()(0)==t2->getValue()(0), AipsError);
		}

		// test operators
		delete att2;
		att2 = new Attribute(att1name, StringVal2);
		AlwaysAssert(*att1 != *att2, AipsError);
		att2->setValue(*att1);
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att2;

		// test Vector<String>
		delete att1;
		att1 = new Attribute(att1name, StringVal1); // no tolerance, non-strict
		att2 = new Attribute(att1name, StringVecVal); // ditto
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att2;
		att2 = new Attribute(att1name, StringVecVal, True); // strict
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att1name, StringVal1, True); // strict
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att2;
		att2 = new Attribute(att1name, StringVecVal); // non-strict again
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		delete att2;

		// ... all again, but with ptr constructors...
		// test basic construction and Attribute validity
		att1 = new Attribute(att1name, &StringVal1);
		AlwaysAssert(att1, AipsError);
		AlwaysAssert(att1->getName() == att1name, AipsError);
		AlwaysAssert(att1->getType() == AttValue::AtString, AipsError);
		{
			AttributeValue<String>* t1 = dynamic_cast<AttributeValue<String>* >(att1->getAttributeValue());
			AlwaysAssert(t1->getValue()(0)==StringVal1, AipsError);
		}
		AlwaysAssert(*att1 == *att1, AipsError);

		// test cloning
		att2 = att1->clone();
		AlwaysAssert(att2, AipsError);
		AlwaysAssert(att2->getName() == att1name, AipsError);
		AlwaysAssert(att2->getType() == AttValue::AtString, AipsError);
		AlwaysAssert(!(*att1 != *att2), AipsError);
		{
			AttributeValue<String>* t1 = dynamic_cast<AttributeValue<String>* >(att1->getAttributeValue());
			AttributeValue<String>* t2 = dynamic_cast<AttributeValue<String>* >(att2->getAttributeValue());
			AlwaysAssert(t1->getValue()(0)==t2->getValue()(0), AipsError);
		}

		// test operators
		delete att2;
		att2 = new Attribute(att1name, &StringVal2);
		AlwaysAssert(*att1 != *att2, AipsError);
		att2->setValue(*att1);
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att2;

		// StringVal2 has changed because of aliasing.
		AlwaysAssert(StringVal2 != StringVal2original, AipsError);
		// set StringVal2 back to original:
		StringVal2 = StringVal2original;

		// test Vector<String>
		delete att1;
		att1 = new Attribute(att1name, &StringVal1); // non-strict
		att2 = new Attribute(att1name, &StringVecVal); // ditto
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att2;
		att2 = new Attribute(att1name, &StringVecVal, True); // strict
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att1name, &StringVal1, True); // strict
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att2;
		att2 = new Attribute(att1name, &StringVecVal); // non-strict again
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		delete att2;


		// ------------------------------------------------------------
		// Test Quantity Attributes


		Quantity QuantityVal1(4.0, "deg"), QuantityVal2(6.0, "m");
		Quantity QuantityVal3(4.2, "deg");
		Quantity QuantityTol1(3.0, "deg"), QuantityTol2(0.1, "deg");
		Quantity QuantityVal2original(QuantityVal2);
		Vector<Quantity> QuantityVecVal(3);
		QuantityVecVal(0) = Quantity(-2.0, "deg");
		QuantityVecVal(1) = Quantity(4.0, "deg");
		QuantityVecVal(2) = Quantity(6.0, "m");

		// test basic construction and Attribute validity
		att1 = new Attribute(att1name, QuantityVal1);
		AlwaysAssert(att1, AipsError);
		AlwaysAssert(att1->getName() == att1name, AipsError);
		AlwaysAssert(att1->getType() == AttValue::AtQuantity, AipsError);
		{
			AttributeValue<Quantity>* t1 = dynamic_cast<AttributeValue<Quantity>* >(att1->getAttributeValue());
			AlwaysAssert(t1->getValue()(0)==QuantityVal1, AipsError);
		}
		AlwaysAssert(*att1 == *att1, AipsError);

		// test cloning
		att2 = att1->clone();
		AlwaysAssert(att2, AipsError);
		AlwaysAssert(att2->getName() == att1name, AipsError);
		AlwaysAssert(att2->getType() == AttValue::AtQuantity, AipsError);
		AlwaysAssert(!(*att1 != *att2), AipsError);
		{
			AttributeValue<Quantity>* t1 = dynamic_cast<AttributeValue<Quantity>* >(att1->getAttributeValue());
			AttributeValue<Quantity>* t2 = dynamic_cast<AttributeValue<Quantity>* >(att2->getAttributeValue());
			AlwaysAssert(t1->getValue()(0)==t2->getValue()(0), AipsError);
		}

		// test operators
		delete att2;
		att2 = new Attribute(att1name, QuantityVal2);
		AlwaysAssert(*att1 != *att2, AipsError);
		att2->setValue(*att1);
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att2;

		// test Vector<Quantity>
		delete att1;
		att1 = new Attribute(att1name, QuantityVal1); // no tolerance, non-strict
		att2 = new Attribute(att1name, QuantityVecVal); // ditto
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att2;
		att2 = new Attribute(att1name, QuantityVecVal, True); // strict
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att1name, QuantityVal1, True); // strict
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att2;
		att2 = new Attribute(att1name, QuantityVecVal); // non-strict again
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		delete att2;

		// test tolerance in comparison of V<Q> to Q.
		att1 = new Attribute(att1name, QuantityVal1, QuantityTol1);
		att2 = new Attribute(att1name, QuantityVecVal);
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att1;
		att1 = new Attribute(att1name, QuantityVal3, QuantityTol2);
		try {
			// this should throw, because it will ultimately try to compare
			// 4.2 +/- 0.1 degree with 6.0 m.
			*att1 != *att2;
		} catch (const AipsError &x) {
			AlwaysAssert(&x, AipsError);
		}
		delete att1;
		delete att2;

		// ... all again, but with ptr constructors...
		// test basic construction and Attribute validity
		att1 = new Attribute(att1name, &QuantityVal1);
		AlwaysAssert(att1, AipsError);
		AlwaysAssert(att1->getName() == att1name, AipsError);
		AlwaysAssert(att1->getType() == AttValue::AtQuantity, AipsError);
		{
			AttributeValue<Quantity>* t1 = dynamic_cast<AttributeValue<Quantity>* >(att1->getAttributeValue());
			AlwaysAssert(t1->getValue()(0)==QuantityVal1, AipsError);
		}
		AlwaysAssert(*att1 == *att1, AipsError);

		// test cloning
		att2 = att1->clone();
		AlwaysAssert(att2, AipsError);
		AlwaysAssert(att2->getName() == att1name, AipsError);
		AlwaysAssert(att2->getType() == AttValue::AtQuantity, AipsError);
		AlwaysAssert(!(*att1 != *att2), AipsError);
		{
			AttributeValue<Quantity>* t1 = dynamic_cast<AttributeValue<Quantity>* >(att1->getAttributeValue());
			AttributeValue<Quantity>* t2 = dynamic_cast<AttributeValue<Quantity>* >(att2->getAttributeValue());
			AlwaysAssert(t1->getValue()(0)==t2->getValue()(0), AipsError);
		}

		// test operators
		delete att2;
		att2 = new Attribute(att1name, &QuantityVal2);
		AlwaysAssert(*att1 != *att2, AipsError);
		att2->setValue(*att1);
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att2;

		// QuantityVal2 has changed because of aliasing.
		AlwaysAssert(QuantityVal2 != QuantityVal2original, AipsError);
		// set QuantityVal2 back to original:
		QuantityVal2 = QuantityVal2original;

		// test Vector<Quantity>
		delete att1;
		att1 = new Attribute(att1name, &QuantityVal1); // non-strict
		att2 = new Attribute(att1name, &QuantityVecVal); // ditto
		AlwaysAssert(*att1 == *att2, AipsError);
		delete att2;
		att2 = new Attribute(att1name, &QuantityVecVal, True); // strict
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		att1 = new Attribute(att1name, &QuantityVal1, True); // strict
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att2;
		att2 = new Attribute(att1name, &QuantityVecVal); // non-strict again
		AlwaysAssert(*att1 != *att2, AipsError);
		delete att1;
		delete att2;

		cout << "OK" << endl;
		return 0;
	} catch (const AipsError &x) {
		localAbort(x.getMesg());
	}
}

