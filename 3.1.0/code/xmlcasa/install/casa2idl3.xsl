<?xml version="1.0"?>

<xsl:stylesheet version="2.0" 
          xmlns:aps="http://casa.nrao.edu/schema/psetTypes.html"
          xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"     
         xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
   
<xsl:param name="needscomma"></xsl:param>
<xsl:template match="*">
<xsl:apply-templates select="aps:interface"/>
<xsl:apply-templates select="aps:task"/>
};
#endif
</xsl:template>
<xsl:template match="aps:interface">  
#ifndef _<xsl:value-of select="@name"/>_idl_
#define _<xsl:value-of select="@name"/>_idl_
<xsl:text disable-output-escaping="yes">
#include &lt;basicVecs.idl&gt;
#include &lt;casatypes.idl&gt;
</xsl:text>
<xsl:for-each select="aps:needs">
	<xsl:text disable-output-escaping="yes">#include &lt;casa</xsl:text><xsl:value-of select="."/><xsl:text disable-output-escaping="yes">.idl&gt;
</xsl:text>
</xsl:for-each>
<xsl:text disable-output-escaping="yes">
module casac
{
   typedef string MDirection;
   typedef string MRadialVelocity;
   typedef string MPosition;
   typedef string Region;
</xsl:text>           
   
   interface <xsl:value-of select="@name"/>  {
<xsl:for-each select="aps:method">
<xsl:if test="@type!='constructor'">
 <xsl:text>         </xsl:text><xsl:apply-templates select="aps:returns"/> <xsl:value-of select="@name"/>(<xsl:apply-templates select="aps:output"></xsl:apply-templates> <xsl:if test="aps:output and aps:input">, </xsl:if>
              <xsl:apply-templates select="aps:input">
</xsl:apply-templates>);
</xsl:if>
</xsl:for-each>
         }; 
         component <xsl:value-of select="@component"/> supports <xsl:value-of select="@name"/>
         { };
         home <xsl:value-of select="@component"/>Home manages <xsl:value-of select="@component"/>
         { };
 </xsl:template>
  <xsl:template match="aps:task">
   <xsl:text>         </xsl:text><xsl:apply-templates select="aps:returns"/> <xsl:value-of select="@name"/>(<xsl:apply-templates select="aps:output"></xsl:apply-templates> <xsl:if test="aps:output and aps:input">, </xsl:if>
              <xsl:apply-templates select="aps:input">
</xsl:apply-templates>);
  </xsl:template>
  <xsl:template match="aps:input">  
  <xsl:call-template name="doargs">
   <xsl:with-param name="ioflag">in</xsl:with-param>
   </xsl:call-template>
   </xsl:template>
 
  <xsl:template match="aps:inout">  
  <xsl:call-template name="doargs">
   <xsl:with-param name="ioflag">inout</xsl:with-param>
   </xsl:call-template>
   </xsl:template>
   
  <xsl:template match="aps:output">  
  <xsl:call-template name="doargs">
   <xsl:with-param name="ioflag">out</xsl:with-param>  
   </xsl:call-template>
   </xsl:template>

<xsl:template name="doargs">
    <xsl:param name="ioflag"/>
     <xsl:for-each select="aps:param">
              <xsl:choose>           
                 <xsl:when test="@xsi:type='string'"><xsl:value-of select="$ioflag"/> string <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
                 <xsl:when test="@xsi:type='int'"><xsl:value-of select="$ioflag"/> long <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
                 <xsl:when test="@xsi:type='bool'"><xsl:value-of select="$ioflag"/> boolean <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
                 <xsl:when test="@xsi:type='float'"><xsl:value-of select="$ioflag"/> float <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
                 <xsl:when test="@xsi:type='double'"><xsl:choose>
                 <xsl:when test="@units">
                 <xsl:value-of select="$ioflag"/> casac::Quantity <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if>
                 </xsl:when><xsl:otherwise>
                 <xsl:value-of select="$ioflag"/> double <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:otherwise></xsl:choose></xsl:when>   
                 <xsl:when test="@xsi:type='stringArray'"><xsl:value-of select="$ioflag"/> casac::StringVec <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
                 <xsl:when test="@xsi:type='intArray'"><xsl:value-of select="$ioflag"/> casac::IntVec <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
                 <xsl:when test="@xsi:type='boolArray'"><xsl:value-of select="$ioflag"/> casac::BoolVec <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
                 <xsl:when test="@xsi:type='floatArray'"><xsl:value-of select="$ioflag"/> casac::FloatVec <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
                 <xsl:when test="@xsi:type='complexArray'"><xsl:value-of select="$ioflag"/> casac::ComplexVec <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
                 <xsl:when test="@xsi:type='doubleArray'"><xsl:choose>
                 <xsl:when test="@units">
                 <xsl:value-of select="$ioflag"/> casac::Quantity <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if>
                 </xsl:when><xsl:otherwise>
                 <xsl:value-of select="$ioflag"/> casac::DoubleVec <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if></xsl:otherwise></xsl:choose></xsl:when>   
                 <xsl:otherwise>
                 <xsl:value-of select="$ioflag"/><xsl:text> </xsl:text><xsl:value-of select='@xsi:type'/><xsl:text> </xsl:text> <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if>
                 </xsl:otherwise>
              </xsl:choose>
     </xsl:for-each>   
</xsl:template>
     <xsl:template match="aps:returns">  
              <xsl:choose>
                <xsl:when test="@xsi:type='string'">string </xsl:when>
                 <xsl:when test="@xsi:type='int'">long </xsl:when>
                  <xsl:when test="@xsi:type='bool'">boolean </xsl:when>
                 <xsl:when test="@xsi:type='float'">float </xsl:when>
                 <xsl:when test="@xsi:type='double'">double </xsl:when>
                 <xsl:when test="@xsi:type='stringArray'">casac::StringVec </xsl:when>
                 <xsl:when test="@xsi:type='intArray'">casac::IntVec </xsl:when>
                 <xsl:when test="@xsi:type='boolArray'">casac::BoolVec </xsl:when>
                 <xsl:when test="@xsi:type='floatArray'">casac::FloatVec </xsl:when>
                 <xsl:when test="@xsi:type='doubleArray'">casac::DoubleVec </xsl:when>
                 <xsl:when test="@xsi:type='complexArray'">casac::DoubleVec </xsl:when>
                 <xsl:when test="@xsi:type='record'">casac::record </xsl:when>
                 <xsl:when test="@xsi:type='void'">void </xsl:when>
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
