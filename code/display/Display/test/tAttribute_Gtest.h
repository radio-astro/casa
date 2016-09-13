#if ! defined (tAttribute_Test_H_201211118_1001)
#define tAttribute_Test_H_20121118_1001

#include <gtest/gtest.h>
#include <casa/aips.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Quanta/QLogical.h>
#include <display/Display/Attribute.h>
#include <casa/namespace.h>

namespace casa {
namespace viewer {
namespace test {

/**
 * Base class for Attribute Tests.
 */
class TestAttribute : public ::testing::Test {
public:
    TestAttribute (const casacore::String& name);
    virtual ~TestAttribute ();
protected:
    virtual void SetUp();
    virtual void TearDown();

    casacore::String m_testName;
    Attribute* m_att1;
    Attribute* m_att2;
    casacore::String m_att1name;
    casacore::String m_att2name;

};

/**
 * Test casacore::String Attributes.
 */
class TestAttributeString : public TestAttribute {
public:
	TestAttributeString();
	virtual ~TestAttributeString();
protected:
	virtual void SetUp();
	void checkConstruction();
	void checkConstructionVector();
	void checkCloning ();
	void checkOperators();
	casacore::String m_val1;
	casacore::String m_val2;
	casacore::Vector<casacore::String> m_vecVal;

};

/**
 * Test casacore::Quantity Attributes.
 */
class TestAttributeQuantity : public TestAttribute {
public:
	TestAttributeQuantity();
	virtual ~TestAttributeQuantity();
protected:
	virtual void SetUp();
	void checkConstruction();
	void checkConstructionPointer();
	void checkConstructionVector();
	void checkConstructionVectorPointer();
	void checkCloning();
	void checkCloningPointer();
	void checkOperators();
	void checkOperatorsPointer();
	void checkComparisonTolerance();
	casacore::Quantity m_val1;
	casacore::Quantity m_val2;
	casacore::Quantity m_val3;
	casacore::Quantity m_tol1;
	casacore::Quantity m_tol2;
	casacore::Vector<casacore::Quantity> m_vecVal;
};

/**
 * Class for testing numerically based attributes.
 */

template <class T>
class TestAttributeNumeric : public TestAttribute {
public:
    TestAttributeNumeric();
    virtual ~TestAttributeNumeric();
protected:
    virtual void SetUp();
    void checkConstruction();
    void checkConstructionVector();
    void checkConstructionPointer();

    void checkCloning ();
    void checkCloningPointer();

    void checkOperators();
    void checkOperatorsPointer();

    void checkComparisonTolerance();
    void checkComparisonToleranceVector();
    void checkComparisonTolerancePointer();

    void checkAddition();
    void checkAdditionVector();
    void checkAdditionPointer();
    void checkAdditionVectorPointer();

    casacore::Vector<T> m_vecVal1;
    casacore::Vector<T> m_vecVal2;
    T m_val1;
    T m_val2;
    T m_tol1;
    T m_tol2;
};

using testing::Types;
typedef Types<casacore::uInt, casacore::Int, casacore::Float, casacore::Double> Implementations;
TYPED_TEST_CASE( TestAttributeNumeric, Implementations );

} // end namespace test
} // end namespace viewer
} // end namespace casa

#endif // ! defined (tAttribute_Test_H_20121111_1001
