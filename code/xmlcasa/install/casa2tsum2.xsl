<?xml version="1.0"?>
<xsl:stylesheet version="2.0" 
          xmlns:aps="http://casa.nrao.edu/schema/psetTypes.html"
          xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"     
         xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output omit-xml-declaration="yes"></xsl:output>   
<xsl:param name="needscomma"/>
<xsl:param name="taskname"/>
<xsl:param name="taskdescription"/>
<xsl:template match="*">
<xsl:apply-templates select="aps:task"/>
</xsl:template>
<xsl:template match="aps:task">
<xsl:param name="taskname"><xsl:value-of select="@name"/></xsl:param>
<xsl:param name="taskdescription"><xsl:value-of select="aps:shortdescription"/></xsl:param>
<xsl:choose>
<xsl:when test="@startup='false'">
#from <xsl:value-of select="$taskname"/>_cli import <xsl:value-of select="$taskname"/>_cli as <xsl:value-of select="$taskname"/>
</xsl:when>
<xsl:otherwise>
from <xsl:value-of select="$taskname"/>_cli import  <xsl:value-of select="$taskname"/>_cli as <xsl:value-of select="$taskname"/>
</xsl:otherwise>
</xsl:choose>
tasksum['<xsl:value-of select="$taskname"/>'] = '<xsl:value-of select="$taskdescription"/>'</xsl:template>
</xsl:stylesheet>
