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
    TestAttribute (const String& name);
    virtual ~TestAttribute ();
protected:
    virtual void SetUp();
    virtual void TearDown();

    String m_testName;
    Attribute* m_att1;
    Attribute* m_att2;
    String m_att1name;
    String m_att2name;

};

/**
 * Test String Attributes.
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
	String m_val1;
	String m_val2;
	Vector<String> m_vecVal;

};

/**
 * Test Quantity Attributes.
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
	Quantity m_val1;
	Quantity m_val2;
	Quantity m_val3;
	Quantity m_tol1;
	Quantity m_tol2;
	Vector<Quantity> m_vecVal;
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

    Vector<T> m_vecVal1;
    Vector<T> m_vecVal2;
    T m_val1;
    T m_val2;
    T m_tol1;
    T m_tol2;
};

using testing::Types;
typedef Types<uInt, Int, Float, Double> Implementations;
TYPED_TEST_CASE( TestAttributeNumeric, Implementations );

} // end namespace test
} // end namespace viewer
} // end namespace casa

#endif // ! defined (tAttribute_Test_H_20121111_1001
