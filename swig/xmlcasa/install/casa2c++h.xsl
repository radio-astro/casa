<?xml version="1.0"?>

<xsl:stylesheet version="2.0" 
          xmlns:aps="http://casa.nrao.edu/schema/psetTypes.html"
          xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"     
         xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
   
<xsl:param name="needscomma"></xsl:param>
<xsl:template match="*">
<xsl:apply-templates select="aps:interface"/>
<xsl:apply-templates select="aps:task"/>
}
#endif
</xsl:template>
<xsl:template match="aps:interface">  
#ifndef _<xsl:value-of select="@name"/>_cmpt_h_
#define _<xsl:value-of select="@name"/>_cmpt_h_
<xsl:text disable-output-escaping="yes">
#include &lt;vector&gt;
#include &lt;string&gt;
#include &lt;complex&gt;
</xsl:text>
<xsl:for-each select="aps:needs">
	<xsl:text disable-output-escaping="yes">#include &lt;casa</xsl:text><xsl:value-of select="."/><xsl:text disable-output-escaping="yes">.idl&gt;
</xsl:text>
</xsl:for-each>
<xsl:for-each select="aps:code">
<xsl:for-each select="aps:include">
	<xsl:text disable-output-escaping="yes">#include &lt;</xsl:text><xsl:value-of select="."/><xsl:text disable-output-escaping="yes">&gt;
</xsl:text>
</xsl:for-each>
</xsl:for-each>
<xsl:text disable-output-escaping="yes">
using namespace std;

typedef  std::vector&lt;std::string&gt; StringVec;
typedef  std::vector&lt;double&gt; DoubleVec;
typedef  std::vector&lt;int&gt; IntVec;
typedef  std::vector&lt;bool&gt; BoolVec;
typedef  std::vector&lt;complex&lt;double&gt; &gt; ComplexVec;
struct Quantity {
   DoubleVec value;
   string units;
};
typedef string MDirection;
typedef string MRadialVelocity;
typedef string MPosition;
typedef string Region;

namespace casac {
</xsl:text>           
   
   class  <xsl:value-of select="@name"/>  {
      public :
         <xsl:value-of select="@name"/>();
         ~<xsl:value-of select="@name"/>();
<xsl:for-each select="aps:method">
<xsl:if test="lower-case(@type)!='constructor'">
 <xsl:text>         </xsl:text><xsl:apply-templates select="aps:returns"/> <xsl:value-of select="@name"/>(<xsl:apply-templates select="aps:output"></xsl:apply-templates> <xsl:if test="aps:output and aps:input">, </xsl:if>
              <xsl:apply-templates select="aps:input">
</xsl:apply-templates>);
</xsl:if>
</xsl:for-each>
      private :
<xsl:for-each select="aps:code">
<xsl:for-each select="aps:private">
<xsl:for-each select="aps:include">
	<xsl:text disable-output-escaping="yes">#include &lt;</xsl:text><xsl:value-of select="."/><xsl:text disable-output-escaping="yes">&gt;</xsl:text>
</xsl:for-each>
</xsl:for-each>
</xsl:for-each>
   }; 
 </xsl:template>
  <xsl:template match="aps:task">
   <xsl:text>         </xsl:text><xsl:apply-templates select="aps:returns"/> <xsl:value-of select="@name"/>(<xsl:apply-templates select="aps:output"></xsl:apply-templates> <xsl:if test="aps:output and aps:input">, </xsl:if>
              <xsl:apply-templates select="aps:input">
</xsl:apply-templates>);
  </xsl:template>
  <xsl:template match="aps:input">  
  <xsl:call-template name="doargs">
   </xsl:call-template>
   </xsl:template>
 
  <xsl:template match="aps:inout">  
  <xsl:call-template name="doargs">
   </xsl:call-template>
   </xsl:template>
   
  <xsl:template match="aps:output">  
  <xsl:call-template name="doargs">
   </xsl:call-template>
   </xsl:template>

<xsl:template name="doargs">
     <xsl:for-each select="aps:param">
              <xsl:choose>           
                 <xsl:when test="lower-case(@xsi:type)='string'"> string <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='int'"> long <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='bool'"> boolean <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='float'"> float <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='double'"><xsl:choose>
                 <xsl:when test="@units">
                  Quantity <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if>
                 </xsl:when><xsl:otherwise>
                  double <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:otherwise></xsl:choose></xsl:when>   
                 <xsl:when test="lower-case(@xsi:type)='stringarray'"> StringVec <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='intarray'"> IntVec <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='boolarray'"> BoolVec <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='floatarray'"> FloatVec <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='complexarray'"> ComplexVec <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='doublearray'"><xsl:choose>
                 <xsl:when test="@units">
                  Quantity <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if>
                 </xsl:when><xsl:otherwise>
                  DoubleVec <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:otherwise></xsl:choose></xsl:when>   
                 <xsl:otherwise>
                 <xsl:text> </xsl:text><xsl:value-of select='@xsi:type'/><xsl:text> </xsl:text> <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if>
                 </xsl:otherwise>
              </xsl:choose>
     </xsl:for-each>   
</xsl:template>
     <xsl:template match="aps:returns">  
              <xsl:choose>
                <xsl:when test="lower-case(@xsi:type)='string'">string </xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='int'">long </xsl:when>
                  <xsl:when test="lower-case(@xsi:type)='bool'">boolean </xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='float'">float </xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='double'">double </xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='stringarray'">StringVec </xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='intarray'">IntVec </xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='boolarray'">BoolVec </xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='floatarray'">FloatVec </xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='doublearray'">DoubleVec </xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='complexarray'">DoubleVec </xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='record'">record </xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='void'">void </xsl:when>
                  <xsl:when test="@xsi:type=''">void </xsl:when>
                 <xsl:otherwise>
                   <xsl:choose>
                    <xsl:when test="string-length(@xsi:type)=0">void </xsl:when>
                    <xsl:otherwise>
                                      <xsl:value-of select='@xsi:type'/><xsl:text> </xsl:text>
                   </xsl:otherwise>
                    </xsl:choose>
                    </xsl:otherwise>
              </xsl:choose>

  
  </xsl:template>     
     
   
 
  <!-- templates go here -->
</xsl:stylesheet>
