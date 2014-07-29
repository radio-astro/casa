<?xml version="1.0"?>

<xsl:stylesheet version="2.0" 
	xmlns:aps="http://casa.nrao.edu/schema/psetTypes.html"
          xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"     
         xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:param name="label"></xsl:param>
<xsl:param name="hasunits"></xsl:param>
<xsl:template match="*">
<xsl:text disable-output-escaping="yes">
&lt;?xml-stylesheet type=&quot;text/xsl&quot; ?&gt;
</xsl:text>
<xsl:element name="casacomponent">
<xsl:attribute name="exmlns"><xsl:text disable-output-escaping="yes">http://casa.nrao.edu/schema/psetTypes.html</xsl:text></xsl:attribute>
<xsl:attribute name="xsi:schemaLocation"><xsl:text disable-output-escaping="yes">http://casa.nrao.edu/schema/psetTypes.html</xsl:text></xsl:attribute>
<xsl:apply-templates select="aps:tool"/>
<xsl:apply-templates select="aps:task"/>
<xsl:text>
</xsl:text>
</xsl:element>
</xsl:template>

<xsl:template match="aps:any">
<xsl:text>
</xsl:text>
<xsl:element name="any"><xsl:attribute name="type"><xsl:value-of select="@type"/></xsl:attribute></xsl:element>
<xsl:text>
</xsl:text>
</xsl:template>

<xsl:template match="aps:defaultrecord">
	<xsl:element name="stuff"/>

</xsl:template>


<xsl:template match="aps:tool">
<xsl:text>
</xsl:text>
<xsl:element name="interface">
<xsl:attribute name="name">casa<xsl:value-of select="@name"/></xsl:attribute>
<xsl:attribute name="component"><xsl:value-of select="@name"/></xsl:attribute>
<xsl:apply-templates select="aps:shortdescription"/>
<xsl:for-each select="aps:needs">
	<xsl:element name="needs"><xsl:value-of select="."/></xsl:element>
</xsl:for-each>
<xsl:apply-templates select="aps:code"/>
<xsl:for-each select="aps:method">
   <xsl:call-template name="domethod"/>
</xsl:for-each>
<xsl:apply-templates select="aps:task"/>
</xsl:element>
</xsl:template>

<xsl:template match="aps:code">
<xsl:element name="code">
<xsl:text>
</xsl:text>
<xsl:for-each select="aps:include">
<xsl:element name="include"><xsl:value-of select="."/></xsl:element>
<xsl:text>
</xsl:text>
</xsl:for-each>
<xsl:for-each select="aps:private">
<xsl:element name="private"><xsl:value-of select="."/></xsl:element>
<xsl:text>
</xsl:text>
</xsl:for-each>
</xsl:element>
</xsl:template>

<xsl:template match="aps:shortdescription">
<xsl:text>
</xsl:text>
<xsl:element name="shortdescription">
<xsl:value-of select="replace(.,'\\','')"/>
</xsl:element>
<xsl:text>
</xsl:text>
</xsl:template>

<xsl:template match="aps:task">
<xsl:element name="task">
<xsl:attribute name="name"><xsl:value-of select="@name"/></xsl:attribute>
<xsl:attribute name="type"><xsl:value-of select="@type"/></xsl:attribute>
<xsl:apply-templates select="aps:shortdescription"/>
<xsl:apply-templates select="aps:output"/>
<xsl:apply-templates select="aps:inout"/>
<xsl:apply-templates select="aps:input"/>
<xsl:apply-templates select="aps:returns"/>
</xsl:element>
<xsl:text>
</xsl:text>
</xsl:template>

<xsl:template name="domethod">
<xsl:text>
</xsl:text>
<xsl:element name="method">
<xsl:attribute name="name"><xsl:value-of select="@name"/></xsl:attribute>
<xsl:attribute name="type"><xsl:value-of select="@type"/></xsl:attribute>
<xsl:if test="@template"> <xsl:attribute name="template"><xsl:value-of select="@template"/></xsl:attribute></xsl:if>
<xsl:apply-templates select="aps:shortdescription"/>
<xsl:apply-templates select="aps:output"/>
<xsl:apply-templates select="aps:inout"/>
<xsl:apply-templates select="aps:input"/>
<xsl:apply-templates select="aps:returns"/>
</xsl:element>
<xsl:text>
</xsl:text>
</xsl:template>

<xsl:template match="aps:output">
<xsl:element name="output">
<xsl:call-template name="doparam"/>
</xsl:element>
<xsl:text>
</xsl:text>
</xsl:template>

<xsl:template match="aps:inout">
<xsl:element name="inout">
<xsl:call-template name="doparam"/>
</xsl:element>
<xsl:text>
</xsl:text>
</xsl:template>

<xsl:template match="aps:input">
<xsl:element name="input">
<xsl:call-template name="doparam" />
</xsl:element>
<xsl:text>
</xsl:text>
</xsl:template>

<xsl:template match="aps:returns">
<xsl:element name="returns">
<xsl:attribute name="xsi:type"><xsl:value-of select="@type"/></xsl:attribute>
<xsl:if test="@template"> <xsl:attribute name="template"><xsl:value-of select="@template"/></xsl:attribute></xsl:if>
<xsl:if test="@units"> <xsl:attribute name="units"><xsl:value-of select="@units"/></xsl:attribute></xsl:if>
<xsl:choose>
<xsl:when test="@type='variant'">
<xsl:attribute name="xsi:type">any</xsl:attribute>
</xsl:when>
<xsl:when test="@type='record'">
<xsl:attribute name="xsi:type">any</xsl:attribute>
</xsl:when>
<xsl:otherwise>
<xsl:attribute name="xsi:type"><xsl:value-of select="@type"/></xsl:attribute>
</xsl:otherwise>
</xsl:choose>
<xsl:choose>
<xsl:when test="@type='variant'">
	<xsl:element name="any"><xsl:attribute name="type">variant</xsl:attribute></xsl:element>
</xsl:when>
<xsl:when test="@type='record'">
	<xsl:element name="any"><xsl:attribute name="type">record</xsl:attribute></xsl:element>
</xsl:when>
</xsl:choose>
<xsl:apply-templates select="aps:any"/>
</xsl:element>
</xsl:template>

<xsl:template name="doparam">  
<xsl:for-each select="aps:param">
<xsl:element name="param">
<xsl:choose>
<xsl:when test="@type='variant'">
<xsl:attribute name="xsi:type">any</xsl:attribute>
</xsl:when>
<xsl:when test="@type='record'">
<xsl:attribute name="xsi:type">any</xsl:attribute>
</xsl:when>
<xsl:otherwise>
<xsl:attribute name="xsi:type"><xsl:value-of select="@type"/></xsl:attribute>
</xsl:otherwise>
</xsl:choose>
<xsl:attribute name="name"> <xsl:value-of select="@name"/></xsl:attribute>
<xsl:if test="@units"> <xsl:attribute name="units"><xsl:value-of select="@units"/></xsl:attribute></xsl:if>
<xsl:if test="@direction"><xsl:attribute name="direction"> <xsl:value-of select="@direction"/></xsl:attribute></xsl:if>
<xsl:text>
</xsl:text>
 <xsl:element name="description">
  <xsl:value-of select="replace(.,'\s+',' ')"/>
 </xsl:element>
<xsl:choose>
<xsl:when test="@type='variant'">
	<xsl:element name="any"><xsl:attribute name="type">variant</xsl:attribute></xsl:element>
</xsl:when>
<xsl:when test="@type='record'">
	<xsl:element name="any"><xsl:attribute name="type">record</xsl:attribute></xsl:element>
</xsl:when>
</xsl:choose>

<xsl:apply-templates select="aps:any"/>
<xsl:apply-templates select="aps:defaultrecord"/>
<xsl:choose>
<xsl:when test="@units">
<xsl:variable name="theunits"><xsl:value-of select="@units"/></xsl:variable>
<xsl:for-each select="aps:value"><xsl:call-template name="dovalue"><xsl:with-param name="label">value</xsl:with-param><xsl:with-param name="hasunits"><xsl:value-of select="$theunits"/></xsl:with-param></xsl:call-template></xsl:for-each>
<xsl:for-each select="aps:choice"><xsl:call-template name="dovalue"><xsl:with-param name="label">choice</xsl:with-param><xsl:with-param name="hasunits"><xsl:value-of select="$theunits"/></xsl:with-param></xsl:call-template></xsl:for-each>
</xsl:when>
<xsl:otherwise>
<xsl:for-each select="aps:value"><xsl:call-template name="dovalue"><xsl:with-param name="label">value</xsl:with-param><xsl:with-param name="hasunits">F</xsl:with-param></xsl:call-template></xsl:for-each>
<xsl:for-each select="aps:choice"><xsl:call-template name="dovalue"><xsl:with-param name="label">choice</xsl:with-param><xsl:with-param name="hasunits">F</xsl:with-param></xsl:call-template></xsl:for-each>
</xsl:otherwise>

</xsl:choose>
</xsl:element>
<xsl:text>
</xsl:text>
</xsl:for-each>

</xsl:template>     

<!-- Need to handle records better -->
 
  <xsl:template name="dovalue">
  <xsl:param name="label"/>
  <xsl:param name="hasunits"/>  
<xsl:text>
</xsl:text>
  <xsl:choose>
  <xsl:when test="$hasunits='F'">
  <xsl:choose>
	  <xsl:when test="@type='vector'">
		  <xsl:element name="{$label}"><xsl:attribute name="type"><xsl:value-of select="@type"/></xsl:attribute>
		  <xsl:choose>
		      <xsl:when test="@name">
			<xsl:attribute name="name"><xsl:value-of select="@name"/></xsl:attribute>
	   	      </xsl:when>
		   </xsl:choose>
              <xsl:for-each select="aps:value"><xsl:call-template name="dovalue"><xsl:with-param name="label">value</xsl:with-param><xsl:with-param name="hasunits">F</xsl:with-param></xsl:call-template></xsl:for-each>
</xsl:element>
	  </xsl:when>
	  <xsl:otherwise>
		  <xsl:element name="{$label}">
			  <xsl:choose>
				  <xsl:when test="@name">
					  <xsl:attribute name="name"><xsl:value-of select="@name"/></xsl:attribute>
				  </xsl:when>
			  </xsl:choose>
			  <xsl:value-of select="."/>
		  </xsl:element>
	  </xsl:otherwise>
  </xsl:choose>
  </xsl:when> 
  <xsl:otherwise>
   <xsl:element name="{$label}"><xsl:attribute name="type">struct</xsl:attribute><xsl:text>
   </xsl:text> 
   <xsl:element name="value" ><xsl:attribute name="name">value</xsl:attribute>
	   <xsl:value-of select="."/></xsl:element><xsl:text>
   </xsl:text> 
  <xsl:element name="value" ><xsl:attribute name="name">units</xsl:attribute><xsl:value-of select="$hasunits"/></xsl:element><xsl:text>
   </xsl:text> 
 </xsl:element>
 
  </xsl:otherwise>
  </xsl:choose>
 
  </xsl:template>  
 
<!-- templates go here -->
</xsl:stylesheet>
