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
#include &lt;stdcasa/record.h&gt;
#include &lt;tools/casaswig_types.h&gt;
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
//using namespace casac;

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
		      <xsl:when test="lower-case(@xsi:type)='string'"><xsl:if test="@direction='in'"> const</xsl:if><xsl:text disable-output-escaping="yes"> string&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
		      <xsl:when test="lower-case(@xsi:type)='any'"><xsl:if test="@direction='in'"> const</xsl:if><xsl:text disable-output-escaping="yes"> variant&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
		      <xsl:when test="lower-case(@xsi:type)='int'"> int <xsl:if test="@direction!='in'"><xsl:text disable-output-escaping="yes">&amp;</xsl:text></xsl:if><xsl:text> </xsl:text><xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
		      <xsl:when test="lower-case(@xsi:type)='bool'"> bool <xsl:if test="@direction!='in'"><xsl:text disable-output-escaping="yes">&amp;</xsl:text></xsl:if><xsl:text> </xsl:text><xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
		      <xsl:when test="lower-case(@xsi:type)='float'"> float <xsl:if test="@direction!='in'"><xsl:text disable-output-escaping="yes">&amp;</xsl:text></xsl:if><xsl:text> </xsl:text><xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='double'"><xsl:choose>
                 <xsl:when test="@units">
			 <xsl:if test="@direction='in'"> const</xsl:if><xsl:text disable-output-escaping="yes"> Quantity&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if>
                 </xsl:when><xsl:otherwise>
		 double<xsl:if test="@direction!='in'"><xsl:text disable-output-escaping="yes">&amp;</xsl:text></xsl:if><xsl:text> </xsl:text><xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:otherwise></xsl:choose></xsl:when>   
		 <xsl:when test="lower-case(@xsi:type)='stringarray'"> <xsl:if test="@direction='in'"> const</xsl:if><xsl:text disable-output-escaping="yes">std::vector&lt;std::string&gt;&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='recordarray'"> RecordVec&amp; <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
		 <xsl:when test="lower-case(@xsi:type)='intarray'"> <xsl:if test="@direction='in'"> const</xsl:if><xsl:text disable-output-escaping="yes"> std::vector&lt;int&gt;&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
		 <xsl:when test="lower-case(@xsi:type)='boolarray'"> <xsl:if test="@direction='in'"> const</xsl:if><xsl:text disable-output-escaping="yes"> std::vector&lt;bool&gt;&amp;  </xsl:text><xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
		 <xsl:when test="lower-case(@xsi:type)='floatarray'"> <xsl:if test="@direction='in'"> const</xsl:if><xsl:text disable-output-escaping="yes"> std::vector&lt;float&gt;&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
		 <xsl:when test="lower-case(@xsi:type)='complexarray'"> <xsl:if test="@direction='in'"> const</xsl:if><xsl:text disable-output-escaping="yes"> std::vector&lt;std::complex&gt;&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='doublearray'"><xsl:choose>
                 <xsl:when test="@units">
			 <xsl:if test="@direction='in'"> const</xsl:if><xsl:text disable-output-escaping="yes"> Quantity&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if>
                 </xsl:when><xsl:otherwise>
		 <xsl:if test="@direction='in'"> const</xsl:if><xsl:text disable-output-escaping="yes"> std::vector&lt;double&gt;&amp; </xsl:text> <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:otherwise></xsl:choose></xsl:when>   
                 <xsl:otherwise>
			 <xsl:text> </xsl:text><xsl:value-of select='@xsi:type'/><xsl:if test="@direction='in'"> const</xsl:if><xsl:text> </xsl:text> <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if>
                 </xsl:otherwise>
              </xsl:choose>
     </xsl:for-each>   
</xsl:template>
     <xsl:template match="aps:returns">  
              <xsl:choose>
                <xsl:when test="lower-case(@xsi:type)='string'">string </xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='int'">int </xsl:when>
                  <xsl:when test="lower-case(@xsi:type)='bool'">bool </xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='float'">float </xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='double'">double </xsl:when>

		 <xsl:when test="lower-case(@xsi:type)='stringarray'"><xsl:text disable-output-escaping="yes">std::vector&lt;std::string&gt; </xsl:text></xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='recordarray'">RecordVec </xsl:when>
		 <xsl:when test="lower-case(@xsi:type)='intarray'"><xsl:text disable-output-escaping="yes">std::vector&lt;int&gt; </xsl:text></xsl:when>
			 <xsl:when test="lower-case(@xsi:type)='boolarray'"><xsl:text disable-output-escaping="yes">std::vector&lt;bool&gt; </xsl:text> </xsl:when>
				 <xsl:when test="lower-case(@xsi:type)='floatarray'"><xsl:text disable-output-escaping="yes">std::vector&lt;float&gt;</xsl:text> </xsl:when>
					 <xsl:when test="lower-case(@xsi:type)='doublearray'"><xsl:text disable-output-escaping="yes">std::vector&lt;double&gt;</xsl:text> </xsl:when>
						 <xsl:when test="lower-case(@xsi:type)='complexarray'"><xsl:text disable-output-escaping="yes">std::vector&lt;std::complex&gt;</xsl:text> </xsl:when>
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
