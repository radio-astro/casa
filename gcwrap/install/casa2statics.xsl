<?xml version="1.0"?>

<xsl:stylesheet version="2.0" 
          xmlns:aps="http://casa.nrao.edu/schema/psetTypes.html"
          xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"     
         xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	 <xsl:output omit-xml-declaration="yes"/>   
<xsl:param name="needscomma"></xsl:param>
<xsl:template match="*">
<xsl:apply-templates select="aps:interface"/>
</xsl:template>
<xsl:template match="aps:interface">  
<xsl:text disable-output-escaping="yes">
#include &lt;vector&gt;
#include &lt;string&gt;
#include &lt;complex&gt;
#include &lt;stdcasa/record.h&gt;
#include &lt;tools/casaswig_types.h&gt;
#include &lt;</xsl:text><xsl:value-of select="@name"/><xsl:text disable-output-escaping="yes">_cmpt.h&gt;
</xsl:text>
<xsl:for-each select="aps:code">
</xsl:for-each>
<xsl:text disable-output-escaping="yes">
using namespace std;

namespace casac {
</xsl:text>           


<xsl:for-each select="aps:method">
	<xsl:variable name="mymethod"><xsl:value-of select="@name"/></xsl:variable>
<xsl:for-each select="aps:output">
	<xsl:for-each select="aps:param">
		<xsl:choose>
			<xsl:when test="@units">
				<xsl:choose>
					<xsl:when test="lower-case(@xsi:type)='doublearray'"><xsl:text disable-output-escaping="yes">Quantity </xsl:text><xsl:value-of select="../../../@name"/>::<xsl:value-of select="@name"/>_<xsl:value-of select="$mymethod"/>=Quantity();
</xsl:when>
<xsl:when test="lower-case(@xsi:type)='double'"><xsl:text disable-output-escaping="yes">Quantity </xsl:text><xsl:value-of select="../../../@name"/>::<xsl:value-of select="@name"/>_<xsl:value-of select="$mymethod"/>=Quantity();
</xsl:when>
                                 </xsl:choose>
			</xsl:when>
			<xsl:otherwise>
		<xsl:choose>
			<xsl:when test="lower-case(@xsi:type)='doublearray'"><xsl:text disable-output-escaping="yes">std::vector&lt;double&gt; </xsl:text><xsl:value-of select="../../../@name"/>::<xsl:value-of select="@name"/>_<xsl:value-of select="$mymethod"/><xsl:text disable-output-escaping="yes">= vector&lt;double&gt;(0)</xsl:text>;
</xsl:when>
<xsl:when test="lower-case(@xsi:type)='intarray'"><xsl:text disable-output-escaping="yes">std::vector&lt;int&gt; </xsl:text><xsl:value-of select="../../../@name"/>::<xsl:value-of select="@name"/>_<xsl:value-of select="$mymethod"/><xsl:text disable-output-escaping="yes">= vector&lt;int&gt;(0)</xsl:text>;
</xsl:when>
<xsl:when test="lower-case(@xsi:type)='boolarray'"><xsl:text disable-output-escaping="yes">std::vector&lt;bool&gt; </xsl:text><xsl:value-of select="../../../@name"/>::<xsl:value-of select="@name"/>_<xsl:value-of select="$mymethod"/><xsl:text disable-output-escaping="yes">= vector&lt;bool&gt;(0)</xsl:text>;
</xsl:when>
<xsl:when test="lower-case(@xsi:type)='any'"><xsl:text disable-output-escaping="yes">variant </xsl:text><xsl:value-of select="../../../@name"/>::<xsl:value-of select="@name"/>_<xsl:value-of select="$mymethod"/>=initialize_variant("");
</xsl:when>
<xsl:when test="lower-case(@xsi:type)='variant'"><xsl:text disable-output-escaping="yes">variant </xsl:text><xsl:value-of select="../../../@name"/>::<xsl:value-of select="@name"/>_<xsl:value-of select="$mymethod"/>=initialize_variant("");
</xsl:when>
<xsl:when test="lower-case(@xsi:type)='record'"><xsl:text disable-output-escaping="yes">record </xsl:text><xsl:value-of select="../../../@name"/>::<xsl:value-of select="@name"/>_<xsl:value-of select="$mymethod"/>=initialize_record("");
</xsl:when>
		<xsl:otherwise>
			<xsl:value-of select="@xsi:type"/><xsl:text> </xsl:text><xsl:value-of select="../../../@name"/>::<xsl:value-of select="@name"/>_<xsl:value-of select="$mymethod"/>= <xsl:value-of select="@xsi:type"/>();
		</xsl:otherwise>
	</xsl:choose>
</xsl:otherwise>
</xsl:choose>
	</xsl:for-each>
</xsl:for-each>	
</xsl:for-each>
}
 </xsl:template>
</xsl:stylesheet>
