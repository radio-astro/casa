<xsl:stylesheet version="2.0" 
          xmlns:aps="http://casa.nrao.edu/schema/psetTypes.html"
          xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" 
         xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:param name="label"></xsl:param>
<xsl:param name="hasunits"></xsl:param>
<xsl:variable name="percent" select="'%'"></xsl:variable>
<xsl:variable name="escapedpercent" select="'\\%'"></xsl:variable>
<xsl:variable name="tilde" select="'~'"></xsl:variable>
<xsl:variable name="escapedtilde" select="'$~$'"></xsl:variable>
<xsl:variable name="carrot" select="'^'"></xsl:variable>
<xsl:variable name="escapedcarrot" select="'$^$'"></xsl:variable>
<xsl:variable name="amp" select="'\amp;'"></xsl:variable>
<xsl:variable name="escapedamp" select="'\\amp;'"></xsl:variable>

<xsl:template match="*">

<xsl:apply-templates select="aps:tool"/>
<xsl:apply-templates select="aps:task"/>
</xsl:template>

<xsl:template match="aps:link">
	\htmladdnormallink{<xsl:value-of select="." disable-output-escaping="yes"/>}{<xsl:value-of select="@anchor" disable-output-escaping="yes"/>}
</xsl:template>

<xsl:template match="aps:any"> <xsl:value-of select="@type"/> </xsl:template>

<xsl:template match="aps:tool">
<xsl:param name="toolname"><xsl:value-of select="@name"/></xsl:param>
<xsl:param name="modulename"><xsl:value-of select="@module"/></xsl:param>
\newpage
\ahtool{<xsl:value-of select="@name"/>}{<xsl:apply-templates select="aps:shortdescription"/>}
Requires:
<xsl:for-each select="aps:needs">
	<xsl:value-of select="."/>
</xsl:for-each>
Synopsis
<xsl:apply-templates select="aps:description"/>
Methods
\begin{longtable}{ll}
<xsl:for-each select="aps:method">
	\ahlink{<xsl:value-of select="@name"/>}{<xsl:value-of select="$modulename"/>:<xsl:value-of select="$toolname"/>.<xsl:value-of select="@name"/>} <xsl:text disable-output-escaping="yes">&amp;</xsl:text><xsl:apply-templates select="aps:shortdescription"/> \\
</xsl:for-each>
<xsl:apply-templates select="aps:task"/>
\end{longtable}

<xsl:apply-templates select="aps:description"/>

<xsl:for-each select="aps:method">
      <xsl:call-template name="domethod">
      <xsl:with-param name="toolname"><xsl:value-of select="$toolname"/>.</xsl:with-param>
      </xsl:call-template>
</xsl:for-each>

</xsl:template>

<xsl:template match="aps:example">
<xsl:text disable-output-escaping="yes">
\flushleft\vspace{.20in}\textbf {Example}\\
\hfill \\
</xsl:text>
<xsl:value-of select="." disable-output-escaping="yes"/>
</xsl:template>

<xsl:template match="aps:description">
<xsl:text disable-output-escaping="yes">
\flushleft\vspace{.20in}\textbf {Description}\\
   \hfill \\</xsl:text>
<xsl:value-of select="." disable-output-escaping="yes"/>
</xsl:template>



<xsl:template match="aps:shortdescription">
	<xsl:value-of select="." disable-output-escaping="yes"/>
</xsl:template>

<xsl:template match="aps:task">
\newpage
\subsection{<xsl:value-of select="@name"/>}
Requires:
<xsl:for-each select="aps:needs">
	<xsl:value-of select="."/>
</xsl:for-each>
Synopsis
<xsl:apply-templates select="aps:shortdescription"/>
<xsl:apply-templates select="aps:description"/>
\flushleft\vspace{.20in}\textbf {Arguments}\\
\hfill \\
\begin{tabular}{|l p{0.25in}p{0.75in}p{2.38in}|}
\hline
<xsl:apply-templates select="aps:output"/>
<xsl:apply-templates select="aps:inout"/>
<xsl:apply-templates select="aps:input"/>
\end{tabular}
<xsl:apply-templates select="aps:returns"/>
<xsl:apply-templates select="aps:example"/>
</xsl:template>

<xsl:template name="methodsummary">
</xsl:template>
<xsl:template name="domethod">
<xsl:param name="toolname"/>
\newpage
\ahfunction{<xsl:value-of select="$toolname"/><xsl:value-of select="@name"/>}{<xsl:apply-templates select="aps:shortdescription"/>}
<xsl:apply-templates select="aps:description"/>
\flushleft\vspace{.20in}\textbf {Arguments}\\
\hfill \\
\begin{tabular}{|l p{0.25in}p{0.75in}p{2.38in}|}
\hline
<xsl:apply-templates select="aps:output"/>
<xsl:apply-templates select="aps:inout"/>
<xsl:apply-templates select="aps:input"/>
\end{tabular}
<xsl:apply-templates select="aps:returns"/>
<xsl:apply-templates select="aps:example"/>
</xsl:template>

<xsl:template match="aps:output">
<xsl:text disable-output-escaping="yes">Outputs &amp;&amp; \multicolumn{2}{p{3.36in}|}{}\\
</xsl:text>
<xsl:call-template name="doparam"/>
<xsl:text>
</xsl:text>
</xsl:template>

<xsl:template match="aps:inout">
<xsl:text disable-output-escaping="yes">Inputs/Outputs &amp;&amp; \multicolumn{2}{p{3.36in}|}{}\\
</xsl:text>
<xsl:call-template name="doparam"/>
<xsl:text>
</xsl:text>
</xsl:template>

<xsl:template match="aps:input">
<xsl:text disable-output-escaping="yes">Inputs &amp;&amp; \multicolumn{2}{p{3.36in}|}{}\\
</xsl:text>
<xsl:call-template name="doparam" />
<xsl:text>
</xsl:text>
</xsl:template>

<xsl:template match="aps:returns">
\flushleft\vspace{.20in}\textbf {Returns}\newline
<xsl:value-of select="@type"/> <xsl:if test="@units"> <xsl:value-of select="@units"/></xsl:if>
<xsl:apply-templates select="aps:any"/>
\newline
</xsl:template>

<xsl:template name="doparam">  
<xsl:for-each select="aps:param">
<xsl:value-of select="@name"/> <xsl:text disable-output-escaping="yes">&amp;</xsl:text> <xsl:if test="@units"> <xsl:value-of select="@units"/></xsl:if><xsl:text disable-output-escaping="yes">&amp;</xsl:text> \multicolumn{2}{p{3.36in}|}{<xsl:value-of select="aps:description" disable-output-escaping="yes"/>}\\
<xsl:text disable-output-escaping="yes">&amp; &amp;allowed: &amp; </xsl:text><xsl:value-of select="@type"/> <xsl:if test="@units"> <xsl:value-of select="@units"/></xsl:if>\\
<xsl:text disable-output-escaping="yes">&amp; &amp;Default: &amp; </xsl:text>
<xsl:apply-templates select="aps:any"/>
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
</xsl:choose>\\
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
              <xsl:for-each select="aps:value"><xsl:call-template name="dovalue"><xsl:with-param name="label">value</xsl:with-param><xsl:with-param name="hasunits">F</xsl:with-param></xsl:call-template></xsl:for-each>
	  </xsl:when>
	  <xsl:otherwise>
			  <xsl:value-of select="replace(.,$percent,$escapedpercent)"/>
			  </xsl:otherwise>
  </xsl:choose>
  </xsl:when> 
  <xsl:otherwise>
	<xsl:value-of select="replace(.,$percent,$escapedpercent)"/><xsl:text>
   </xsl:text> 

 
  </xsl:otherwise>
  </xsl:choose>
 
  </xsl:template>  
 
<!-- templates go here -->
</xsl:stylesheet>
