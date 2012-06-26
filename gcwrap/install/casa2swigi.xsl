<?xml version="1.0"?>

<xsl:stylesheet version="2.0" 
          xmlns:aps="http://casa.nrao.edu/schema/psetTypes.html"
          xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"     
         xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	 <xsl:output omit-xml-declaration="yes"/>   
<xsl:template match="*">
<xsl:apply-templates select="aps:interface"/>
</xsl:template>

<xsl:template match="aps:interface">  
%module <xsl:value-of select="@name"/>
<xsl:text disable-output-escaping="yes">
%include &lt;tools/casa_typemaps.i&gt;
</xsl:text>
<xsl:for-each select="aps:method">
   <xsl:if test="lower-case(@type)!='constructor'">
      <xsl:apply-templates select="aps:output"/>
   </xsl:if>
</xsl:for-each>
<xsl:text disable-output-escaping="yes">
%feature("kwargs");
%feature("autodoc");
%exception {
   try {
      $action
      } catch (const casa::AipsError &amp;ae) {
         PyErr_SetString(PyExc_RuntimeError, ae.what());
	 //PyErr_Print();
         return NULL;
      }
}
%include "</xsl:text><xsl:value-of select="@name"/><xsl:text disable-output-escaping="yes">_cmpt.h"
%{
#include &lt;exception&gt;
#include &lt;</xsl:text><xsl:value-of select="@name"/><xsl:text disable-output-escaping="yes">_cmpt.h&gt;
%}
</xsl:text>
</xsl:template>

<xsl:template match="aps:output">  
	  <xsl:call-template name="dooutargs"/>
</xsl:template>

<xsl:template name="dooutargs">
     <xsl:for-each select="aps:param">
              <xsl:choose>           
		      <xsl:when test="lower-case(@xsi:type)='record'">
<xsl:text disable-output-escaping="yes">%apply record &amp;OUTARGREC {record &amp;</xsl:text><xsl:value-of select="@name"/><xsl:text>}
</xsl:text>
		      </xsl:when>
		      <xsl:when test="lower-case(@xsi:type)='intarray'">
			      <xsl:text disable-output-escaping="yes">%apply std::vector&lt;int&gt;  &amp;OUTARGVEC {std::vector&lt;int&gt; &amp;</xsl:text><xsl:value-of select="@name"/><xsl:text>}
</xsl:text>
		      </xsl:when>
		      <xsl:when test="lower-case(@xsi:type)='doublearray'">
			      <xsl:text disable-output-escaping="yes">%apply std::vector&lt;double&gt;  &amp;OUTARGVEC {std::vector&lt;double&gt; &amp;</xsl:text><xsl:value-of select="@name"/><xsl:text>}
</xsl:text>
		      </xsl:when>
		      <xsl:when test="lower-case(@xsi:type)='stringarray'">
			      <xsl:text disable-output-escaping="yes">%apply std::vector&lt;string&gt;  &amp;OUTARGVEC {std::vector&lt;string&gt; &amp;</xsl:text><xsl:value-of select="@name"/><xsl:text>}
</xsl:text>
		      </xsl:when>
		      <xsl:when test="lower-case(@xsi:type)='int'">
			      <xsl:text disable-output-escaping="yes">%apply int  &amp;OUTARGINT {int &amp;</xsl:text><xsl:value-of select="@name"/><xsl:text>}
</xsl:text>
		      </xsl:when>
		      <xsl:when test="lower-case(@xsi:type)='double'">
			      <xsl:text disable-output-escaping="yes">%apply double  &amp;OUTARGDBL {double &amp;</xsl:text><xsl:value-of select="@name"/><xsl:text>}
</xsl:text>
		      </xsl:when>
		      <xsl:when test="lower-case(@xsi:type)='string'">
			      <xsl:text disable-output-escaping="yes">%apply std::string  &amp;OUTARGSTR {std::string &amp;</xsl:text><xsl:value-of select="@name"/><xsl:text>}
</xsl:text>
		      </xsl:when>
	      </xsl:choose>
     </xsl:for-each>
</xsl:template>
     
</xsl:stylesheet>
