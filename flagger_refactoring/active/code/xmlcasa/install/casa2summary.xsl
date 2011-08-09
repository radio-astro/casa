<?xml version="1.0"?>

<xsl:stylesheet version="2.0" 
          xmlns:aps="http://casa.nrao.edu/schema/psetTypes.html"
          xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"     
         xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:param name="label"></xsl:param>
<xsl:param name="hasunits"></xsl:param>
<xsl:template match="*">
<xsl:apply-templates select="aps:tool"/>
<xsl:apply-templates select="aps:task"/>
<xsl:text>
</xsl:text>
</xsl:template>

<xsl:template match="aps:any">
<xsl:text>
</xsl:text>
<xsl:element name="any"><xsl:attribute name="type"><xsl:value-of select="@type"/></xsl:attribute></xsl:element>
<xsl:text>
</xsl:text>
</xsl:template>

<xsl:template match="aps:tool">
<xsl:text>
</xsl:text>
|Tool: <xsl:value-of select="@name"/>|<xsl:apply-templates select="aps:shortdescription"/>|||
||||
<xsl:for-each select="aps:method">
   <xsl:call-template name="domethod"/>||||
||||
</xsl:for-each>
<xsl:apply-templates select="aps:task"/>
</xsl:template>
<xsl:template match="aps:shortdescription">
<xsl:value-of select="."/>
</xsl:template>
<xsl:template match="aps:task">|Task: <xsl:value-of select="@name"/>|<xsl:apply-templates select="aps:shortdescription"/>||
<xsl:apply-templates select="aps:output"/>
<xsl:apply-templates select="aps:inout"/>
<xsl:apply-templates select="aps:input"/>
<xsl:text>
</xsl:text>
</xsl:template>

<xsl:template name="domethod">|Method: <xsl:value-of select="@name"/>|<xsl:apply-templates select="aps:shortdescription"/>||
<xsl:apply-templates select="aps:output"/>
<xsl:apply-templates select="aps:inout"/>
<xsl:apply-templates select="aps:input"/>
</xsl:template>

<xsl:template match="aps:output"><xsl:call-template name="doparam"/></xsl:template>

<xsl:template match="aps:inout"><xsl:call-template name="doparam"/></xsl:template>

<xsl:template match="aps:input"><xsl:call-template name="doparam" /></xsl:template>

<xsl:template name="doparam">  
<xsl:for-each select="aps:param">|<xsl:value-of select="@name"/>|<xsl:value-of select="@type"/>|<xsl:value-of select="aps:description"/>|
</xsl:for-each>
</xsl:template>     
 
  <xsl:template name="dovalue">
  <xsl:param name="label"/>
  <xsl:param name="hasunits"/>  
<xsl:text>
</xsl:text>
  <xsl:choose>
  <xsl:when test="$hasunits='F'">
  <xsl:choose>
	  <xsl:when test="@type='vector'">
		  <xsl:element name="{$label}"><xsl:attribute name="type">vector</xsl:attribute>
              <xsl:for-each select="aps:value"><xsl:call-template name="dovalue"><xsl:with-param name="label">value</xsl:with-param><xsl:with-param name="hasunits">F</xsl:with-param></xsl:call-template></xsl:for-each>
</xsl:element>
	  </xsl:when>
	  <xsl:otherwise>
		  <xsl:element name="{$label}">
			  <xsl:value-of select="."/></xsl:element>
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
