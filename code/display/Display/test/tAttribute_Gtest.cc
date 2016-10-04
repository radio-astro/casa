#include "tAttribute_Gtest.h"

#include <casa/BasicSL.h>
#include <casa/Arrays/ArrayLogical.h>
#include <display/Display/AttVal.h>

using namespace std;
using namespace casa;
using namespace casacore;

int
main (int nArgs, char * args []){
    ::testing::InitGoogleTest(& nArgs, args);
    return RUN_ALL_TESTS();
}

using namespace casacore;
namespace casa {
namespace viewer {
namespace test {

TestAttribute::TestAttribute (const String& name):
		m_testName( name ){
}

void TestAttribute::SetUp(){
	m_att1 = NULL;
	m_att2 = NULL;
	m_att1name = "frogs";
	m_att2name = "another name with spaces in it";
}

void TestAttribute::TearDown(){
	delete m_att1;
	delete m_att2;
	m_att1 = NULL;
	m_att2 = NULL;
}


TestAttribute::~TestAttribute (){
	TearDown();
}

TestAttributeString::TestAttributeString ():
		TestAttribute( "Attribute String Test" ),
		m_vecVal(3){
}

void TestAttributeString::SetUp(){
	TestAttribute::SetUp();
	m_val1 = "true";
	m_val2 = "false";
	m_vecVal(0) = "Maybe";
	m_vecVal(1) = "true";
	m_vecVal(2) = "false";
}

void TestAttributeString::checkConstruction(){
	m_att1 = new Attribute(m_att1name, m_val1);
	ASSERT_TRUE(m_att1 != NULL );
	ASSERT_TRUE(m_att1->getName() == m_att1name );
	ASSERT_TRUE(m_att1->getType() == AttValue::AtString);
	AttributeValue<String>* t1 = dynamic_cast<AttributeValue<String>* >(m_att1->getAttributeValue());
	ASSERT_TRUE(t1->getValue()(0)==m_val1);
	ASSERT_TRUE(*m_att1 == *m_att1);
}

void TestAttributeString::checkConstructionVector(){
	m_att1 = new Attribute(m_att1name, m_val1); // no tolerance, non-strict
	m_att2 = new Attribute(m_att1name, m_vecVal); // ditto
	ASSERT_TRUE(*m_att1 == *m_att2);
	delete m_att2;
	m_att2 = new Attribute(m_att1name, m_vecVal, true); // strict
	ASSERT_TRUE(*m_att1 != *m_att2);
	delete m_att1;
	m_att1 = new Attribute(m_att1name, m_val1, true); // strict
	ASSERT_TRUE(*m_att1 != *m_att2);
	delete m_att2;
	m_att2 = new Attribute(m_att1name, m_vecVal); // non-strict again
	ASSERT_TRUE(*m_att1 != *m_att2);
}

void TestAttributeString::checkCloning (){
	m_att1 = new Attribute(m_att1name, m_val1);
	m_att2 = m_att1->clone();
	ASSERT_TRUE(m_att2 != NULL);
	ASSERT_TRUE(m_att2->getName() == m_att1name);
	ASSERT_TRUE(m_att2->getType() == AttValue::AtString);
	ASSERT_TRUE(!(*m_att1 != *m_att2));
	AttributeValue<String>* t1 = dynamic_cast<AttributeValue<String>* >(m_att1->getAttributeValue());
	AttributeValue<String>* t2 = dynamic_cast<AttributeValue<String>* >(m_att2->getAttributeValue());
	ASSERT_TRUE(t1->getValue()(0)==t2->getValue()(0));
}

void TestAttributeString::checkOperators(){
	m_att1 = new Attribute(m_att1name, m_val1);
	m_att2 = new Attribute(m_att1name, m_val2);
	ASSERT_TRUE(*m_att1 != *m_att2);
	m_att2->setValue(*m_att1);
	ASSERT_TRUE(*m_att1 == *m_att2);
}

TestAttributeString::~TestAttributeString(){

}

TestAttributeQuantity::TestAttributeQuantity():
		TestAttribute( "Attribute Quantity Test" ),
		m_vecVal(3){
}

void TestAttributeQuantity::SetUp(){
	TestAttribute::SetUp();
	m_val1 = Quantity( 4.0, "deg");
	m_val2 = Quantity( 6.0, "m");
	m_val3 = Quantity( 4.2, "deg");
	m_tol1 = Quantity( 3.0, "deg");
	m_tol2 = Quantity( 0.1, "deg");
	m_vecVal(0) = Quantity( -2.0, "deg");
	m_vecVal(1) = Quantity( 4.0, "deg");
	m_vecVal(2) = Quantity( 6.0, "m");
}

void TestAttributeQuantity::checkConstruction(){
	m_att1 = new Attribute(m_att1name, m_val1);
	ASSERT_TRUE(m_att1 != NULL);
	ASSERT_TRUE(m_att1->getName() == m_att1name);
	ASSERT_TRUE(m_att1->getType() == AttValue::AtQuantity);
	AttributeValue<Quantity>* t1 = dynamic_cast<AttributeValue<Quantity>* >(m_att1->getAttributeValue());
	ASSERT_TRUE(t1->getValue()(0)==m_val1);
	ASSERT_TRUE(*m_att1 == *m_att1);
}

void TestAttributeQuantity::checkConstructionPointer(){
	m_att1 = new Attribute(m_att1name, &m_val1);
	ASSERT_TRUE(m_att1 != NULL);
	ASSERT_TRUE(m_att1->getName() == m_att1name);
	ASSERT_TRUE(m_att1->getType() == AttValue::AtQuantity);
	AttributeValue<Quantity>* t1 = dynamic_cast<AttributeValue<Quantity>* >(m_att1->getAttributeValue());
	ASSERT_TRUE(t1->getValue()(0)==m_val1);
	ASSERT_TRUE(*m_att1 == *m_att1);
}

void TestAttributeQuantity::checkConstructionVector(){
	m_att1 = new Attribute(m_att1name, m_val1); // no tolerance, non-strict
	m_att2 = new Attribute(m_att1name, m_vecVal); // ditto
	ASSERT_TRUE(*m_att1 == *m_att2);
	delete m_att2;
	m_att2 = new Attribute(m_att1name, m_vecVal, true); // strict
	ASSERT_TRUE(*m_att1 != *m_att2);
	delete m_att1;
	m_att1 = new Attribute(m_att1name, m_val1, true); // strict
	ASSERT_TRUE(*m_att1 != *m_att2);
	delete m_att2;
	m_att2 = new Attribute(m_att1name, m_vecVal); // non-strict again
	ASSERT_TRUE(*m_att1 != *m_att2);
}

void TestAttributeQuantity::checkConstructionVectorPointer(){
	m_att1 = new Attribute(m_att1name, &m_val1); // non-strict
	m_att2 = new Attribute(m_att1name, &m_vecVal); // ditto
	ASSERT_TRUE(*m_att1 == *m_att2);
	delete m_att2;
	m_att2 = new Attribute(m_att1name, &m_vecVal, true); // strict
	ASSERT_TRUE(*m_att1 != *m_att2);
	delete m_att1;
	m_att1 = new Attribute(m_att1name, &m_val1, true); // strict
	ASSERT_TRUE(*m_att1 != *m_att2);
	delete m_att2;
	m_att2 = new Attribute(m_att1name, &m_vecVal); // non-strict again
	ASSERT_TRUE(*m_att1 != *m_att2);
}

void TestAttributeQuantity::checkCloning(){
	m_att1 = new Attribute(m_att1name, m_val1);
	m_att2 = m_att1->clone();
	ASSERT_TRUE(m_att2 != NULL);
	ASSERT_TRUE(m_att2->getName() == m_att1name);
	ASSERT_TRUE(m_att2->getType() == AttValue::AtQuantity);
	ASSERT_TRUE(!(*m_att1 != *m_att2));
	AttributeValue<Quantity>* t1 = dynamic_cast<AttributeValue<Quantity>* >(m_att1->getAttributeValue());
	AttributeValue<Quantity>* t2 = dynamic_cast<AttributeValue<Quantity>* >(m_att2->getAttributeValue());
	ASSERT_TRUE(t1->getValue()(0)==t2->getValue()(0));
}

void TestAttributeQuantity::checkCloningPointer(){
	m_att1 = new Attribute(m_att1name, &m_val1);
	m_att2 = m_att1->clone();
	ASSERT_TRUE(m_att2 != NULL);
	ASSERT_TRUE(m_att2->getName() == m_att1name);
	ASSERT_TRUE(m_att2->getType() == AttValue::AtQuantity);
	ASSERT_TRUE(!(*m_att1 != *m_att2));
	AttributeValue<Quantity>* t1 = dynamic_cast<AttributeValue<Quantity>* >(m_att1->getAttributeValue());
	AttributeValue<Quantity>* t2 = dynamic_cast<AttributeValue<Quantity>* >(m_att2->getAttributeValue());
	ASSERT_TRUE(t1->getValue()(0)==t2->getValue()(0));
}

void TestAttributeQuantity:: checkOperators(){
	m_att1 = new Attribute(m_att1name, m_val1);
	m_att2 = new Attribute(m_att1name, m_val2);
	ASSERT_TRUE(*m_att1 != *m_att2);
	m_att2->setValue(*m_att1);
	ASSERT_TRUE(*m_att1 == *m_att2);
}

void TestAttributeQuantity::checkOperatorsPointer(){
	m_att1 = new Attribute(m_att1name, &m_val1);
	m_att2 = new Attribute(m_att1name, &m_val2);
	Quantity val2Original = m_val2;
	ASSERT_TRUE(*m_att1 != *m_att2);
	m_att2->setValue(*m_att1);
	ASSERT_TRUE(*m_att1 == *m_att2);
	// QuantityVal2 has changed because of aliasing.
	ASSERT_TRUE(m_val2 != val2Original);
}

void TestAttributeQuantity::checkComparisonTolerance(){
	m_att1 = new Attribute(m_att1name, m_val1, m_tol1);
	m_att2 = new Attribute(m_att1name, m_vecVal);
	ASSERT_TRUE(*m_att1 == *m_att2);
	delete m_att1;
	m_att1 = new Attribute(m_att1name, m_val3, m_tol2);
	try {
		// this should throw, because it will ultimately try to compare
		// 4.2 +/- 0.1 degree with 6.0 m.
		*m_att1 != *m_att2;
	}
	catch (const AipsError &x) {
		ASSERT_TRUE(&x != NULL);
	}
}

TestAttributeQuantity::~TestAttributeQuantity(){

}

TEST_F ( TestAttributeString, checkConstruction ){
    ASSERT_NO_FATAL_FAILURE (checkConstruction ());
}

TEST_F ( TestAttributeString, checkConstructionVector ){
    ASSERT_NO_FATAL_FAILURE (checkConstructionVector());
}

TEST_F ( TestAttributeString, checkCloning ){
    ASSERT_NO_FATAL_FAILURE (checkCloning());
}



TEST_F ( TestAttributeString, checkOperators ){
    ASSERT_NO_FATAL_FAILURE (checkOperators());
}

TEST_F ( TestAttributeQuantity, checkConstruction ){
    ASSERT_NO_FATAL_FAILURE (checkConstruction());
}

TEST_F ( TestAttributeQuantity, checkConstructionPointer ){
    ASSERT_NO_FATAL_FAILURE (checkConstructionPointer());
}

TEST_F ( TestAttributeQuantity, checkConstructionVector ){
    ASSERT_NO_FATAL_FAILURE (checkConstructionVector());
}

TEST_F ( TestAttributeQuantity, checkConstructionVectorPointer ){
    ASSERT_NO_FATAL_FAILURE (checkConstructionVectorPointer());
}

TEST_F ( TestAttributeQuantity, checkCloning ){
    ASSERT_NO_FATAL_FAILURE (checkCloning());
}

TEST_F ( TestAttributeQuantity, checkCloningPointer ){
    ASSERT_NO_FATAL_FAILURE (checkCloningPointer());
}

TEST_F ( TestAttributeQuantity, checkOperators ){
    ASSERT_NO_FATAL_FAILURE (checkOperators());
}

TEST_F ( TestAttributeQuantity, checkOperatorsPointer ){
    ASSERT_NO_FATAL_FAILURE (checkOperatorsPointer());
}

TEST_F ( TestAttributeQuantity, checkComparisonTolerance ){
	ASSERT_NO_FATAL_FAILURE( checkComparisonTolerance());
}

//-------------------------------------------------------------------------
//                        Numeric
//-------------------------------------------------------------------------

template <class T>
TestAttributeNumeric<T>::TestAttributeNumeric():
		TestAttribute( "Attribute Test" ),
		m_vecVal1(3),
		m_vecVal2(3){
}

template <class T>
void TestAttributeNumeric<T>::SetUp(){
	TestAttribute::SetUp();

	m_vecVal1(0) = 2;
	m_vecVal1(1) = 6;
	m_vecVal1(2) = 9;
	m_vecVal2 = 0;
	m_val1 = 2;
	m_val2 = 8;
	m_tol1 = 5;
	m_tol2 = 7;
}

template <class T>
TestAttributeNumeric<T>::~TestAttributeNumeric(){

}


//------------------------------------------------------------------------
//                    Construction Tests
//------------------------------------------------------------------------
template <class T>
void TestAttributeNumeric<T>::checkConstruction(){
	m_att1 = new Attribute(m_att1name, m_val1);
	ASSERT_TRUE( m_att1 != NULL );
	EXPECT_EQ(m_att1->getName(), m_att1name);
	EXPECT_EQ(m_att1->getType(), AttValue::whatType( &m_val1) );
	AttributeValue<T>* t1 = dynamic_cast<AttributeValue<T>* >(m_att1->getAttributeValue());
	EXPECT_EQ(t1->getValue()(0), m_val1 );
	EXPECT_EQ(*m_att1, *m_att1 );
}

template <class T>
void TestAttributeNumeric<T>::checkConstructionVector(){
	m_att1 = new Attribute(m_att1name, m_val1);
	m_att2 = new Attribute(m_att1name, m_vecVal1);
	ASSERT_TRUE(*m_att1 == *m_att2);
	delete m_att2;
	m_att2 = new Attribute(m_att1name, m_vecVal1, T(0), true); // notol, strict
	ASSERT_TRUE(*m_att1 != *m_att2);
	delete m_att1;
	m_att1 = new Attribute(m_att1name, m_val1, T(0), true); // no tol, strict
	ASSERT_TRUE(*m_att1 != *m_att2);
	delete m_att2;
	m_att2 = new Attribute(m_att1name, m_vecVal1); // no tol, non-strict again
	ASSERT_TRUE(*m_att1 != *m_att2);
}

template <class T>
void TestAttributeNumeric<T>::checkConstructionPointer(){
	m_att1 = new Attribute(m_att1name, &m_val1);
	ASSERT_TRUE(m_att1!=NULL);
	ASSERT_TRUE(m_att1->getName() == m_att1name);
	ASSERT_TRUE(m_att1->getType() == AttValue::whatType( &m_val1));
	AttributeValue<T>* t1 = dynamic_cast<AttributeValue<T>* >(m_att1->getAttributeValue());
	ASSERT_TRUE(t1->getValue()(0)==m_val1);
	ASSERT_TRUE(*m_att1 == *m_att1);
}

//---------------------------------------------------------------------
//                       Cloning Tests
//---------------------------------------------------------------------

template <class T>
void TestAttributeNumeric<T>::checkCloning(){
	m_att1 = new Attribute(m_att1name, m_val1);
	m_att2 = m_att1->clone();
	ASSERT_TRUE(m_att2 != NULL );
	EXPECT_EQ(m_att2->getName(), m_att1name );
	EXPECT_EQ(m_att2->getType(), AttValue::whatType( &m_val1));
	ASSERT_TRUE(!(*m_att1 != *m_att2) );
	AttributeValue<T>* t1 = dynamic_cast<AttributeValue<T>* >(m_att1->getAttributeValue());
	AttributeValue<T>* t2 = dynamic_cast<AttributeValue<T>* >(m_att2->getAttributeValue());
	EXPECT_EQ(t1->getValue()(0), t2->getValue()(0));
}

template <class T>
void TestAttributeNumeric<T>::checkCloningPointer(){
	m_att1 = new Attribute(m_att1name, &m_val1);
	m_att2 = m_att1->clone();
	ASSERT_TRUE(m_att2 != NULL );
	EXPECT_EQ(m_att2->getName(), m_att1name);
	EXPECT_EQ(m_att2->getType(), AttValue::whatType( &m_val1));
	ASSERT_TRUE(!(*m_att1 != *m_att2));
	AttributeValue<T>* t1 = dynamic_cast<AttributeValue<T>* >(m_att1->getAttributeValue());
	AttributeValue<T>* t2 = dynamic_cast<AttributeValue<T>* >(m_att2->getAttributeValue());
	EXPECT_EQ(t1->getValue()(0), t2->getValue()(0));
}

//---------------------------------------------------------------------
//                       Operator Tests
//---------------------------------------------------------------------

template <class T>
void TestAttributeNumeric<T>::checkOperators(){
	m_att1 = new Attribute(m_att1name, m_val1);
	m_att2 = new Attribute(m_att1name, m_val2);
	ASSERT_TRUE(*m_att1 != *m_att2);
	m_att2->setValue(*m_att1);
	ASSERT_TRUE(*m_att1 == *m_att2);
}

template <class T>
void TestAttributeNumeric<T>::checkOperatorsPointer(){
	m_att1 = new Attribute(m_att1name, &m_val1);
	m_att2 = new Attribute(m_att1name, &m_val2);
	ASSERT_TRUE(*m_att1 != *m_att2);
	m_att2->setValue(*m_att1);
	ASSERT_TRUE(*m_att1 == *m_att2);
	delete m_att2;

	// above setValue has modified att2, and therefore uIntVal2, so now
	// the next test as in above section would fail, eg.:
	m_att2 = new Attribute(m_att1name, &m_val2, m_tol2);
	ASSERT_TRUE(*m_att1 == *m_att2);

}

//---------------------------------------------------------------------
//                       Comparison Tolerance Tests
//---------------------------------------------------------------------

template <class T>
void TestAttributeNumeric<T>::checkComparisonTolerance(){
	m_att1 = new Attribute(m_att1name, m_val1);
	m_att2 = new Attribute(m_att1name, m_val2);
	ASSERT_TRUE(*m_att1 != *m_att2);
	m_att2->setValue(*m_att1);
	ASSERT_TRUE(*m_att1 == *m_att2);
}

template <class T>
void TestAttributeNumeric<T>::checkComparisonTolerancePointer(){
	m_att1 = new Attribute(m_att1name, &m_val1);
	m_att2 = new Attribute(m_att1name, &m_val2, m_tol2);
	ASSERT_TRUE(*m_att1 != *m_att2);
	delete m_att1;
	m_att1 = new Attribute(m_att1name, &m_val1, m_tol1);
	ASSERT_TRUE(*m_att1 != *m_att2);
	delete m_att1;
	m_att1 = new Attribute(m_att2name, &m_val1, m_tol2);
	ASSERT_TRUE(*m_att1 == *m_att2);
}

template <class T>
void TestAttributeNumeric<T>::checkComparisonToleranceVector(){
	m_att1 = new Attribute(m_att1name, &m_val1); // no tolerance, non-strict
	m_att2 = new Attribute(m_att1name, &m_vecVal1); // ditto
	ASSERT_TRUE(*m_att1 == *m_att2);
	delete m_att2;
	m_att2 = new Attribute(m_att1name, &m_vecVal1, T(0),
			                     true); // notol, strict
	ASSERT_TRUE(*m_att1 != *m_att2);
	delete m_att1;
	m_att1 = new Attribute(m_att1name, &m_val1, T(0), true); // no tol, strict
	ASSERT_TRUE(*m_att1 != *m_att2);
	delete m_att2;
	m_att2 = new Attribute(m_att1name, &m_vecVal1); // no tol, non-strict again
	ASSERT_TRUE(*m_att1 != *m_att2 );
}

//---------------------------------------------------------------------
//                       Arithmetic Tests
//---------------------------------------------------------------------

template <class T>
void TestAttributeNumeric<T>::checkAddition(){
	m_att1 = new Attribute(m_att1name, m_val1);
	m_att2 = new Attribute(m_att1name, m_val2);
	*m_att2 += *m_att1;
	AttributeValue<T>* t1 = dynamic_cast<AttributeValue<T>* >(m_att2->getAttributeValue());
	ASSERT_EQ(t1->getValue()(0), (m_val1 + m_val2));
}

template <class T>
void TestAttributeNumeric<T>::checkAdditionPointer(){
	m_att1 = new Attribute(m_att1name, &m_val1);
	m_att2 = new Attribute(m_att1name, &m_val2);
	// store old value because it will be over-written!
	T val2original = m_val2;
	*m_att2 += *m_att1;
	AttributeValue<T>* t2 = dynamic_cast<AttributeValue<T>* >(m_att2->getAttributeValue());
	ASSERT_EQ(t2->getValue()(0),(m_val1 + val2original));
}

template <class T>
void TestAttributeNumeric<T>::checkAdditionVector(){
	m_att1 = new Attribute(m_att1name, m_vecVal1);
	m_att2 = new Attribute(m_att1name, m_vecVal1);
	*m_att2 += *m_att1;
	AttributeValue<T>* t1 = dynamic_cast<AttributeValue<T>* >(m_att2->getAttributeValue());
	Vector<T> vals = t1->getValue();
	ASSERT_TRUE( allEQ(t1->getValue(), m_vecVal1 + m_vecVal1));
}

template <class T>
void TestAttributeNumeric<T>::checkAdditionVectorPointer(){
	m_att1 = new Attribute(m_att1name, &m_vecVal1); // ditto
	m_att2 = new Attribute(m_att1name, &m_vecVal2); // ditto
	Vector<T> vecVal2original = m_vecVal2;
	*m_att2 += *m_att1;
	AttributeValue<T>* t2 = dynamic_cast<AttributeValue<T>* >(m_att2->getAttributeValue());
	ASSERT_TRUE(allEQ(t2->getValue(), (m_vecVal1 + vecVal2original)) );
}




TYPED_TEST( TestAttributeNumeric, checkConstruction ){
    ASSERT_NO_FATAL_FAILURE (this->checkConstruction());
}

TYPED_TEST( TestAttributeNumeric, checkConstructionVector ){
    ASSERT_NO_FATAL_FAILURE (this->checkConstructionVector());
}

TYPED_TEST( TestAttributeNumeric, checkConstructionPointer ){
    ASSERT_NO_FATAL_FAILURE (this->checkConstructionPointer());
}

TYPED_TEST( TestAttributeNumeric, checkCloning ){
    ASSERT_NO_FATAL_FAILURE (this->checkCloning());
}

TYPED_TEST( TestAttributeNumeric, checkCloningPointer ){
    ASSERT_NO_FATAL_FAILURE (this->checkCloningPointer());
}

TYPED_TEST( TestAttributeNumeric, checkOperators ){
    ASSERT_NO_FATAL_FAILURE (this->checkOperators ());
}

TYPED_TEST( TestAttributeNumeric, checkOperatorsPointer ){
    ASSERT_NO_FATAL_FAILURE (this->checkOperatorsPointer());
}

TYPED_TEST( TestAttributeNumeric, checkComparisonTolerance ){
    ASSERT_NO_FATAL_FAILURE (this->checkComparisonTolerance ());
}

TYPED_TEST( TestAttributeNumeric, checkComparisonToleranceVector ){
    ASSERT_NO_FATAL_FAILURE (this->checkComparisonToleranceVector());
}

TYPED_TEST( TestAttributeNumeric, checkComparisonTolerancePointer ){
    ASSERT_NO_FATAL_FAILURE (this->checkComparisonTolerancePointer());
}

TYPED_TEST( TestAttributeNumeric, checkAddition ){
    ASSERT_NO_FATAL_FAILURE (this->checkAddition());
}

TYPED_TEST( TestAttributeNumeric, checkAdditionVector ){
    ASSERT_NO_FATAL_FAILURE (this->checkAdditionVector());
}

TYPED_TEST( TestAttributeNumeric, checkAdditionPointer ){
    ASSERT_NO_FATAL_FAILURE (this->checkAdditionPointer());
}

TYPED_TEST( TestAttributeNumeric, checkAdditionVectorPointer ){
    ASSERT_NO_FATAL_FAILURE (this->checkAdditionVectorPointer());
}


} // end namespace test
} // end namespace display
using namespace casacore;
} // end namespace casa

