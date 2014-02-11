<?xml version="1.0"?>

<xsl:stylesheet version="2.0" 
          xmlns:aps="http://casa.nrao.edu/schema/psetTypes.html"
          xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"     
         xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	 <xsl:output omit-xml-declaration="yes"/>   

	 <xsl:variable name="singlequote"><xsl:text>'</xsl:text></xsl:variable>
<xsl:param name="methodname"/>

<xsl:template match="*">
<xsl:apply-templates select="aps:interface"/>
</xsl:template>

<xsl:template match="aps:interface">  
%module <xsl:value-of select="@name"/>
<xsl:text disable-output-escaping="yes">
%include &lt;tools/casa_typemaps.i&gt;
%feature("kwargs");
%feature("autodoc", "0");
</xsl:text>
<xsl:for-each select="aps:method">
   <xsl:if test="lower-case(@type)!='constructor'">
      <xsl:apply-templates select="aps:output"/>
      <xsl:text disable-output-escaping="yes">%feature("docstring", " </xsl:text>
      <xsl:apply-templates select="aps:shortdescription"/>
      <xsl:apply-templates select="aps:description"/>
      <xsl:if test="count(aps:input/aps:param)">
         <xsl:call-template name="iparaminfo"/>
      </xsl:if>
      <xsl:if test="count(aps:output/aps:param)">
         <xsl:call-template name="oparaminfo"/>
      </xsl:if>
      <xsl:apply-templates select="aps:example"/>
      <xsl:text>
--------------------------------------------------------------------------------
	      ") </xsl:text><xsl:value-of select="@name"/>;
   </xsl:if>
</xsl:for-each>
<xsl:text disable-output-escaping="yes">
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
</xsl:text>
<xsl:if test="aps:instantiate">
	<xsl:call-template name="instantiate"/>
</xsl:if>
<xsl:text disable-output-escaping="yes">
%{
#include &lt;exception&gt;
#include &lt;</xsl:text><xsl:value-of select="@name"/><xsl:text disable-output-escaping="yes">_cmpt.h&gt;
%}
</xsl:text>
</xsl:template>

<xsl:template match="aps:shortdescription">  
<xsl:text>

Summary
	</xsl:text> <xsl:value-of select="."/>
</xsl:template>

<xsl:template match="aps:description">  
<xsl:text>

Description
	</xsl:text> <xsl:value-of disable-output-escaping="yes" select="replace(., '&quot;', $singlequote)"/>
</xsl:template>


<xsl:template match="aps:output">  
	  <xsl:call-template name="dooutargs"/>
</xsl:template>

<xsl:template match="aps:example">
	<xsl:text>
Example:
	</xsl:text> <xsl:value-of disable-output-escaping="yes" select="replace(., '&quot;', $singlequote)"/>
</xsl:template>

<xsl:template name="iparaminfo">
<xsl:text>

Input Parameters:
	</xsl:text>
	<xsl:for-each select="aps:input/aps:param">
	   <xsl:choose>
	<xsl:when test="string-length(@name) &lt; 8">
	<xsl:value-of select="@name"/><xsl:text>		</xsl:text><xsl:value-of disable-output-escaping="yes" select="replace(aps:description, '&quot;', $singlequote)"/><xsl:text>
	</xsl:text>
              </xsl:when>
	      <xsl:otherwise>
	<xsl:value-of select="@name"/><xsl:text>	</xsl:text><xsl:value-of disable-output-escaping="yes" select="replace(aps:description, '&quot;', $singlequote)"/><xsl:text>
	</xsl:text>
	      </xsl:otherwise>
	</xsl:choose>
   </xsl:for-each>
</xsl:template>

<xsl:template name="oparaminfo">
<xsl:text>

Output Parameters:
	</xsl:text>
	<xsl:for-each select="aps:output/aps:param">
	   <xsl:choose>
	      <xsl:when test="string-length(@name) &lt; 8">
	<xsl:value-of select="@name"/><xsl:text>		</xsl:text><xsl:value-of disable-output-escaping="yes" select="replace(aps:description, '&quot;', $singlequote)"/><xsl:text>
	</xsl:text>
              </xsl:when>
	      <xsl:otherwise>
	<xsl:value-of select="@name"/><xsl:text>	</xsl:text><xsl:value-of disable-output-escaping="yes" select="replace(aps:description, '&quot;', $singlequote)"/><xsl:text>
	</xsl:text>
	      </xsl:otherwise>
	</xsl:choose>
   </xsl:for-each>
</xsl:template>

<xsl:template name="dooutargs">
     <xsl:for-each select="aps:param">
              <xsl:choose>           
		      <xsl:when test="lower-case(@xsi:type)='record'">
<xsl:text disable-output-escaping="yes">%apply record &amp;OUTARGREC {record &amp;</xsl:text><xsl:value-of select="@name"/><xsl:text>}
</xsl:text>
		      </xsl:when>
		      <xsl:when test="lower-case(@xsi:type)='mdirection'">
<xsl:text disable-output-escaping="yes">%apply record &amp;OUTARGREC {record &amp;</xsl:text><xsl:value-of select="@name"/><xsl:text>}
</xsl:text>
		      </xsl:when>
		      <xsl:when test="lower-case(@xsi:type)='intarray'">
			      <xsl:text disable-output-escaping="yes">%apply std::vector&lt;int&gt;  &amp;OUTARGVEC {std::vector&lt;int&gt; &amp;</xsl:text><xsl:value-of select="@name"/><xsl:text>}
</xsl:text>
		      </xsl:when>
		      <xsl:when test="lower-case(@xsi:type)='doublearray'">
			      <xsl:choose>
			      <xsl:when test="@units">
			         <xsl:text disable-output-escaping="yes">%apply Quantity  &amp;OUTARGQUANTITY {Quantity &amp;</xsl:text><xsl:value-of select="@name"/><xsl:text>}
</xsl:text>
		              </xsl:when>
			      <xsl:otherwise>
			         <xsl:text disable-output-escaping="yes">%apply std::vector&lt;double&gt;  &amp;OUTARGVEC {std::vector&lt;double&gt; &amp;</xsl:text><xsl:value-of select="@name"/><xsl:text>}
</xsl:text>
			      </xsl:otherwise>
		      </xsl:choose>
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
		      <xsl:choose>
			      <xsl:when test="@units">
			         <xsl:text disable-output-escaping="yes">%apply Quantity  &amp;OUTARGQUANTITY {Quantity &amp;</xsl:text><xsl:value-of select="@name"/><xsl:text>}
</xsl:text>
		              </xsl:when>
			      <xsl:otherwise>
			         <xsl:text disable-output-escaping="yes">%apply double  &amp;OUTARGDBL {double &amp;</xsl:text><xsl:value-of select="@name"/><xsl:text>}
</xsl:text>
			      </xsl:otherwise>
		      </xsl:choose>
		      </xsl:when>
		      <xsl:when test="lower-case(@xsi:type)='string'">
			      <xsl:text disable-output-escaping="yes">%apply string  &amp;OUTARGSTR {string &amp;</xsl:text><xsl:value-of select="@name"/><xsl:text>}
</xsl:text>
		      </xsl:when>
	      </xsl:choose>
     </xsl:for-each>
</xsl:template>
<xsl:template name="instantiate">
	<xsl:for-each select="aps:instantiate">
	<xsl:choose>
		<xsl:when test="@name">
%template(<xsl:value-of select="@template"/><xsl:value-of select="@name"/>) casac::<xsl:value-of select="@name"/><xsl:text disable-output-escaping="yes">&lt;</xsl:text><xsl:value-of select="@template"/><xsl:text disable-output-escaping="yes">&gt;;</xsl:text>
		</xsl:when>
		<xsl:when test="@file">
			<xsl:text disable-output-escaping="yes">#include &lt;</xsl:text><xsl:value-of select="@file"/><xsl:text disable-output-escaping="yes">&gt;</xsl:text>
		</xsl:when>
	</xsl:choose>
</xsl:for-each>
</xsl:template> 
</xsl:stylesheet>
