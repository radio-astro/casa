<?xml version="1.0"?>

<xsl:stylesheet version="2.0" 
          xmlns:aps="http://casa.nrao.edu/schema/psetTypes.html"
          xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"     
         xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output omit-xml-declaration="yes"/>   
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
#include &lt;casa/Exceptions/Error.h&gt;
</xsl:text>
<xsl:for-each select="aps:needs">
	<xsl:text disable-output-escaping="yes">#include &lt;</xsl:text><xsl:value-of select="."/><xsl:text disable-output-escaping="yes">_cmpt.h&gt;
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

namespace casac {
</xsl:text>           


   class  <xsl:value-of select="@name"/>  {
       public :
<xsl:for-each select="aps:method">
	<xsl:variable name="mymethod"><xsl:value-of select="@name"/></xsl:variable>
<xsl:for-each select="aps:output">
	<xsl:for-each select="aps:param">
		<xsl:choose>
			<xsl:when test="@units">
				<xsl:choose>
<xsl:when test="lower-case(@xsi:type)='doublearray'"><xsl:text disable-output-escaping="yes">static Quantity </xsl:text><xsl:value-of select="@name"/>_<xsl:value-of select="$mymethod"/>;
</xsl:when>
<xsl:when test="lower-case(@xsi:type)='double'"><xsl:text disable-output-escaping="yes">static Quantity </xsl:text><xsl:value-of select="@name"/>_<xsl:value-of select="$mymethod"/>;
</xsl:when>
                                 </xsl:choose>
			</xsl:when>
			<xsl:otherwise>
		<xsl:choose>
<xsl:when test="lower-case(@xsi:type)='doublearray'"><xsl:text disable-output-escaping="yes">static std::vector&lt;double&gt; </xsl:text><xsl:value-of select="@name"/>_<xsl:value-of select="$mymethod"/>;
</xsl:when>
<xsl:when test="lower-case(@xsi:type)='intarray'"><xsl:text disable-output-escaping="yes">static std::vector&lt;int&gt; </xsl:text><xsl:value-of select="@name"/>_<xsl:value-of select="$mymethod"/>;
</xsl:when>
<xsl:when test="lower-case(@xsi:type)='boolarray'"><xsl:text disable-output-escaping="yes">static std::vector&lt;bool&gt; </xsl:text><xsl:value-of select="@name"/>_<xsl:value-of select="$mymethod"/>;
</xsl:when>
<xsl:when test="lower-case(@xsi:type)='any'"><xsl:text disable-output-escaping="yes">static variant </xsl:text><xsl:value-of select="@name"/>_<xsl:value-of select="$mymethod"/>;
</xsl:when>
		<xsl:otherwise>
static <xsl:value-of select="@xsi:type"/><xsl:text> </xsl:text><xsl:value-of select="@name"/>_<xsl:value-of select="$mymethod"/>;
		</xsl:otherwise>
	</xsl:choose>
</xsl:otherwise>
</xsl:choose>
	</xsl:for-each>
</xsl:for-each>	
</xsl:for-each>

         <xsl:value-of select="@name"/>();
         ~<xsl:value-of select="@name"/>();
<xsl:for-each select="aps:method">
<xsl:if test="lower-case(@type)!='constructor'">
 <xsl:text>         </xsl:text><xsl:apply-templates select="aps:returns"/> <xsl:value-of select="@name"/>(<xsl:apply-templates select="aps:input"></xsl:apply-templates> <xsl:if test="aps:output and aps:input">, </xsl:if>
              <xsl:apply-templates select="aps:output">
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
	  <xsl:with-param name="defdirection"><xsl:value-of>in</xsl:value-of></xsl:with-param>
	  <xsl:with-param name="myname"><xsl:value-of select="../@name"/></xsl:with-param>
   </xsl:call-template>
   </xsl:template>
 
  <xsl:template match="aps:inout">  
  <xsl:call-template name="doargs">
	  <xsl:with-param name="defdirection"><xsl:value-of>in</xsl:value-of></xsl:with-param>
	  <xsl:with-param name="myname"><xsl:value-of select="../@name"/></xsl:with-param>
   </xsl:call-template>
   </xsl:template>
   
  <xsl:template match="aps:output">  
  <xsl:call-template name="doargs">
	  <xsl:with-param name="defdirection"><xsl:value-of>out</xsl:value-of></xsl:with-param>
	  <xsl:with-param name="myname"><xsl:value-of select="../@name"/></xsl:with-param>
   </xsl:call-template>
   </xsl:template>

   <xsl:template name="doquant">
           <xsl:text disable-output-escaping="yes">=Quantity(std::vector&lt;double&gt;(1, </xsl:text>
	   <xsl:for-each select="aps:value/aps:value">
		   <xsl:choose>
			   <xsl:when test="@name='value'">
				   <xsl:value-of select="normalize-space(.)"/><xsl:text>),"</xsl:text>
			   </xsl:when>
			   <xsl:when test="@name='units'">
				   <xsl:value-of select="normalize-space(.)"/><xsl:text>")</xsl:text>
			   </xsl:when>
		   </xsl:choose>
	   </xsl:for-each>
   </xsl:template>


   <xsl:template name="dovec">
	<xsl:param name="mytype"/>
	   <xsl:choose>
		   <!-- need to fix this so it will at least assign one element properly -->
		   <xsl:when test="$mytype='string'">
		                  <xsl:text disable-output-escaping="yes">=std::vector&lt;string&gt;(1, "")</xsl:text>
		   </xsl:when>
		   <xsl:when test="aps:value/aps:value">
		      <xsl:text disable-output-escaping="yes">=initialize_vector(</xsl:text>
		      <xsl:for-each select="aps:value/aps:value">
			 <xsl:if test="position()=1"><xsl:value-of select="last()"/>,</xsl:if>
			 <xsl:if test="$mytype!='string'">(<xsl:value-of select="$mytype"/>)</xsl:if><xsl:if test="$mytype='string'">string("</xsl:if>
                         <xsl:value-of select="normalize-space(.)"/>
			 <xsl:if test="$mytype='string'">")</xsl:if><xsl:if test="position()&lt;last()">, </xsl:if>
	              </xsl:for-each>
                      <xsl:text>)</xsl:text>
              </xsl:when>
              <xsl:otherwise>
		      <xsl:choose>
			      <xsl:when test="aps:value != ''">
		                  <xsl:text disable-output-escaping="yes">=initialize_vector(</xsl:text>
				  <xsl:text>1, </xsl:text>
			          <xsl:if test="$mytype='string'">string("</xsl:if>(<xsl:value-of select="$mytype"/>)<xsl:value-of select="normalize-space(aps:value)"/>
			          <xsl:if test="$mytype='string'">")</xsl:if>
			          <xsl:text>)</xsl:text>
		              </xsl:when>
		              <xsl:otherwise>
		                  <xsl:text disable-output-escaping="yes">=std::vector&lt;</xsl:text>
				  <xsl:value-of select="$mytype"/>
		                  <xsl:text disable-output-escaping="yes">&gt;() </xsl:text>
		              </xsl:otherwise>
	             </xsl:choose>
              </xsl:otherwise>
          </xsl:choose>
   </xsl:template>

   <xsl:template name="dorec">
	   <xsl:choose>
	      <xsl:when test="aps:value/aps:value">
		      <xsl:text disable-output-escaping="yes">=initialize_record("</xsl:text>
		      <xsl:for-each select="aps:value/aps:value">
			      <xsl:value-of select="normalize-space(.)"/> <xsl:if test="position()&lt;last()">, </xsl:if><xsl:if test="position()=last()"><xsl:text>")</xsl:text></xsl:if>
	              </xsl:for-each>
              </xsl:when>
              <xsl:otherwise>
		  <xsl:text disable-output-escaping="yes">=initialize_record("</xsl:text>
		  <xsl:choose>
		      <xsl:when test="aps:value != ''">
			 <xsl:value-of select="normalize-space(aps:value)"/> <xsl:text>")</xsl:text>
		      </xsl:when>
		      <xsl:otherwise>
		         <xsl:text disable-output-escaping="yes">") </xsl:text>
		      </xsl:otherwise>
	          </xsl:choose>
              </xsl:otherwise>
          </xsl:choose>
   </xsl:template>

   <xsl:template name="setdef">
	<xsl:param name="mytype"/>
            <xsl:for-each select="aps:value">
		<xsl:if test="position()=last()">
		      <xsl:text disable-output-escaping="yes">=</xsl:text>
	              <xsl:if test="$mytype='string'">string("</xsl:if>
	              <xsl:value-of select="normalize-space(.)"/>
	              <xsl:if test="$mytype='string'">")</xsl:if>
	        </xsl:if>
	</xsl:for-each>
   </xsl:template>

<xsl:template name="dummyargs">
</xsl:template>

<xsl:template name="doargs">
	<xsl:param name="defdirection"/>
	<xsl:param name="myname"/>
     <xsl:for-each select="aps:param">
              <xsl:choose>           
		      <xsl:when test="lower-case(@xsi:type)='string' or lower-case(@xsi:type)='mdirection'">
			  <xsl:choose>
			      <xsl:when test="@direction">
				      <xsl:if test="@direction='in'"> const</xsl:if><xsl:text disable-output-escaping="yes"> string&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="aps:value"><xsl:call-template name="setdef"><xsl:with-param name="mytype"><xsl:value-of>string</xsl:value-of></xsl:with-param></xsl:call-template></xsl:if><xsl:if test="position()&lt;last()">, </xsl:if>
			      </xsl:when>
			      <xsl:otherwise>
				   <xsl:choose>
			           <xsl:when test="$defdirection='in'">
					   <xsl:text disable-output-escaping="yes">const  string&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="aps:value"><xsl:call-template name="setdef"><xsl:with-param name="mytype"><xsl:value-of>string</xsl:value-of></xsl:with-param></xsl:call-template></xsl:if><xsl:if test="position()&lt;last()">, </xsl:if>
			           </xsl:when>
				   <xsl:otherwise>
					   <xsl:text disable-output-escaping="yes">string&amp; </xsl:text><xsl:value-of select="@name"/>
					   <xsl:if test="aps:value">=string("<xsl:value-of select="aps:value"/>")</xsl:if>
			                   <xsl:if test="$defdirection='out'">=<xsl:value-of select="../../../@name"/>::<xsl:value-of select="@name"/>_<xsl:value-of select="$myname"/></xsl:if>
					   <xsl:if test="position()&lt;last()">, </xsl:if>
				   </xsl:otherwise>
			           </xsl:choose>
			      </xsl:otherwise>
		          </xsl:choose>
		      </xsl:when>
		      <xsl:when test="lower-case(@xsi:type)='variant'">
			  <xsl:choose>
			      <xsl:when test="@direction">
				      <xsl:if test="@direction='in'"> const</xsl:if><xsl:text disable-output-escaping="yes"> variant&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="aps:value"> = initialize_variant("<xsl:value-of select="aps:value"/>")</xsl:if><xsl:if test="position()&lt;last()">, </xsl:if>
		              </xsl:when>
			      <xsl:otherwise>
				   <xsl:choose>
			           <xsl:when test="$defdirection='in'">
					   <xsl:text disable-output-escaping="yes"> const variant&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="aps:value"> = initialize_variant("<xsl:value-of select="aps:value"/>")</xsl:if><xsl:if test="position()&lt;last()">, </xsl:if>
			           </xsl:when>
				   <xsl:when test="$defdirection='out'">
					   <xsl:text disable-output-escaping="yes"> variant&amp; </xsl:text><xsl:value-of select="@name"/>=<xsl:value-of select="../../../@name"/>::<xsl:value-of select="@name"/>_<xsl:value-of select="$myname"/><xsl:if test="position()&lt;last()">, </xsl:if>
				   </xsl:when>
				   <xsl:otherwise>
			         <xsl:text disable-output-escaping="yes"> variant&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if>
				   </xsl:otherwise>
			           </xsl:choose>
			      </xsl:otherwise>
		          </xsl:choose>
		      </xsl:when>
		      <xsl:when test="lower-case(@xsi:type)='any'">
			  <xsl:choose>
			      <xsl:when test="@direction">
				      <xsl:if test="@direction='in'"> const</xsl:if><xsl:text disable-output-escaping="yes"> variant&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="aps:value"> = initialize_variant("<xsl:value-of select="aps:value"/>")</xsl:if><xsl:if test="position()&lt;last()">, </xsl:if>
		              </xsl:when>
			      <xsl:otherwise>
				   <xsl:choose>
			           <xsl:when test="$defdirection='in'">
					   <xsl:text disable-output-escaping="yes"> const variant&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="aps:value">= initialize_variant("<xsl:value-of select="aps:value"/>")</xsl:if><xsl:if test="position()&lt;last()">, </xsl:if>
			           </xsl:when>
				   <xsl:when test="$defdirection='out'">
					   <xsl:text disable-output-escaping="yes"> variant&amp; </xsl:text><xsl:value-of select="@name"/>=<xsl:value-of select="../../../@name"/>::<xsl:value-of select="@name"/>_<xsl:value-of select="$myname"/><xsl:if test="position()&lt;last()">, </xsl:if>
				   </xsl:when>
				   <xsl:otherwise>
			         <xsl:text disable-output-escaping="yes"> variant&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if>
				   </xsl:otherwise>
			           </xsl:choose>
			      </xsl:otherwise>
		          </xsl:choose>
		      </xsl:when>
		      <xsl:when test="lower-case(@xsi:type)='record'">
			  <xsl:choose>
			      <xsl:when test="@direction">
				      <xsl:if test="@direction='in'"> const</xsl:if><xsl:text disable-output-escaping="yes"> record&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="aps:value"><xsl:call-template name="dorec"/></xsl:if><xsl:if test="position()&lt;last()">, </xsl:if>
		              </xsl:when>
			      <xsl:otherwise>
				   <xsl:choose>
			           <xsl:when test="$defdirection='in'">
					   <xsl:text disable-output-escaping="yes"> const record&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="aps:value"><xsl:call-template name="dorec"/></xsl:if><xsl:if test="position()&lt;last()">, </xsl:if>
			           </xsl:when>
				   <xsl:when test="$defdirection='out'">
					   <xsl:text disable-output-escaping="yes"> record&amp; </xsl:text><xsl:value-of select="@name"/>=<xsl:value-of select="../../../@name"/>::<xsl:value-of select="@name"/>_<xsl:value-of select="$myname"/><xsl:if test="position()&lt;last()">, </xsl:if>
				   </xsl:when>
				   <xsl:otherwise>
			         <xsl:text disable-output-escaping="yes"> record&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if>
				   </xsl:otherwise>
			           </xsl:choose>
			      </xsl:otherwise>
		          </xsl:choose>
		      </xsl:when>
		      <xsl:when test="lower-case(@xsi:type)='char'"> char <xsl:if test="$defdirection='out' or @defdirecton='inout' "><xsl:text disable-output-escaping="yes">&amp;</xsl:text></xsl:if><xsl:text> </xsl:text><xsl:value-of select="@name"/><xsl:if test="aps:value">=<xsl:value-of select="aps:value"/></xsl:if><xsl:if test="position()&lt;last()">, </xsl:if>
		      </xsl:when>
		      <xsl:when test="lower-case(@xsi:type)='int'"> int <xsl:if test="$defdirection='out' or @defdirecton='inout' "><xsl:text disable-output-escaping="yes">&amp;</xsl:text></xsl:if>
			      <xsl:text> </xsl:text><xsl:value-of select="@name"/>
			      <xsl:if test="aps:value">=<xsl:value-of select="aps:value"/></xsl:if>
			      <xsl:if test="$defdirection='out'">=<xsl:value-of select="../../../@name"/>::<xsl:value-of select="@name"/>_<xsl:value-of select="$myname"/></xsl:if>
			      <xsl:if test="position()&lt;last()">, </xsl:if>
		      </xsl:when>
		      <xsl:when test="lower-case(@xsi:type)='boolean'"> bool <xsl:if test="$defdirection='out' or @defdirecton='inout'"><xsl:text disable-output-escaping="yes">&amp;</xsl:text></xsl:if><xsl:text> </xsl:text><xsl:value-of select="@name"/><xsl:if test="aps:value">=<xsl:value-of select="aps:value"/></xsl:if><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
		      <xsl:when test="lower-case(@xsi:type)='bool'"> bool <xsl:if test="$defdirection='out' or @defdirecton='inout'"><xsl:text disable-output-escaping="yes">&amp;</xsl:text></xsl:if><xsl:text> </xsl:text><xsl:value-of select="@name"/><xsl:if test="aps:value">=<xsl:value-of select="aps:value"/></xsl:if><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
		      <xsl:when test="lower-case(@xsi:type)='float'"> float <xsl:if test="$defdirection='out' or @defdirecton='inout'"><xsl:text disable-output-escaping="yes">&amp;</xsl:text></xsl:if><xsl:text> </xsl:text><xsl:value-of select="@name"/><xsl:if test="aps:value">=<xsl:value-of select="aps:value"/></xsl:if><xsl:if test="position()&lt;last()">, </xsl:if></xsl:when>
		      <xsl:when test="lower-case(@xsi:type)='double'">
			      <xsl:choose>
                 <xsl:when test="@units">
			  <xsl:choose>
			      <xsl:when test="@direction">
				      <xsl:if test="@direction='in'"> const</xsl:if><xsl:text disable-output-escaping="yes"> Quantity&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="aps:value"><xsl:call-template name="doquant" /></xsl:if><xsl:if test="position()&lt;last()">, </xsl:if>
		              </xsl:when>
			      <xsl:otherwise>
				   <xsl:choose>
			           <xsl:when test="$defdirection='in'">
					   <xsl:text disable-output-escaping="yes"> const Quantity&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="aps:value"><xsl:text disable-output-escaping="yes">=Quantity(std::vector&lt;double&gt;(1, </xsl:text><xsl:value-of select="aps:value"/>),<xsl:value-of select="@units"/>)</xsl:if><xsl:if test="position()&lt;last()">, </xsl:if>
			           </xsl:when>
				   <xsl:otherwise>
					   <xsl:text disable-output-escaping="yes"> Quantity&amp; </xsl:text><xsl:value-of select="@name"/>
			                   <xsl:if test="$defdirection='out'">=<xsl:value-of select="../../../@name"/>::<xsl:value-of select="@name"/>_<xsl:value-of select="$myname"/></xsl:if>
					   <xsl:if test="position()&lt;last()">, </xsl:if>
				   </xsl:otherwise>
			           </xsl:choose>
			      </xsl:otherwise>
		          </xsl:choose>
		  </xsl:when>
		  <xsl:otherwise>
			  double<xsl:if test="$defdirection='out' or $defdirection='inout'"><xsl:text disable-output-escaping="yes">&amp;</xsl:text></xsl:if><xsl:text> </xsl:text><xsl:value-of select="@name"/>
<xsl:if test="aps:value">=<xsl:value-of select="aps:value"/></xsl:if>
<xsl:if test="$defdirection='out'">=<xsl:value-of select="../../../@name"/>::<xsl:value-of select="@name"/>_<xsl:value-of select="$myname"/></xsl:if>
<xsl:if test="position()&lt;last()">, </xsl:if>
		  </xsl:otherwise>
  </xsl:choose>
  </xsl:when>   
  <xsl:when test="lower-case(@xsi:type)='stringarray'"> 
	  <xsl:choose>
		  <xsl:when test="@direction">
			  <xsl:if test="@direction='in'"> const </xsl:if><xsl:text disable-output-escaping="yes"> std::vector&lt;std::string&gt;&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="aps:value"><xsl:call-template name="dovec"><xsl:with-param name="mytype"><xsl:value-of>string</xsl:value-of></xsl:with-param></xsl:call-template></xsl:if><xsl:if test="position()&lt;last()">, </xsl:if>
		  </xsl:when>
		  <xsl:otherwise>
			  <xsl:choose>
				  <xsl:when test="$defdirection='in'">
					  <xsl:text disable-output-escaping="yes"> const std::vector&lt;std::string&gt;&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="aps:value"><xsl:call-template name="dovec"><xsl:with-param name="mytype"><xsl:value-of>string</xsl:value-of></xsl:with-param></xsl:call-template></xsl:if><xsl:if test="position()&lt;last()">, </xsl:if>
				  </xsl:when>
				  <xsl:otherwise>
			   <xsl:text disable-output-escaping="yes"> std::vector&lt;std::string&gt;&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if>
				  </xsl:otherwise>
			  </xsl:choose>
		  </xsl:otherwise>
	  </xsl:choose>
  </xsl:when>
  <xsl:when test="lower-case(@xsi:type)='recordarray'">
	  <xsl:choose>
		  <xsl:when test="@direction">
			  <xsl:if test="@direction='in'"> const </xsl:if><xsl:text disable-output-escaping="yes"> RecordVec&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if>
		  </xsl:when>
		  <xsl:otherwise>
			  <xsl:choose>
				  <xsl:when test="$defdirection='in'">
			  <xsl:text disable-output-escaping="yes"> const RecordVec&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if>
				  </xsl:when>
				  <xsl:otherwise>
			  <xsl:text disable-output-escaping="yes"> RecordVec&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if>
				  </xsl:otherwise>
			  </xsl:choose>
		  </xsl:otherwise>
	  </xsl:choose>
  </xsl:when>
 <xsl:when test="lower-case(@xsi:type)='intarray'">
	  <xsl:choose>
		  <xsl:when test="@direction">
			  <xsl:if test="@direction='in'"> const </xsl:if><xsl:text disable-output-escaping="yes"> std::vector&lt;int&gt;&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="aps:value"><xsl:call-template name="dovec"><xsl:with-param name="mytype"><xsl:value-of>int</xsl:value-of></xsl:with-param></xsl:call-template></xsl:if><xsl:if test="position()&lt;last()">, </xsl:if>
		  </xsl:when>
		  <xsl:otherwise>
			  <xsl:choose>
				  <xsl:when test="$defdirection='in'">
					  <xsl:text disable-output-escaping="yes"> const std::vector&lt;int&gt;&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="aps:value"><xsl:call-template name="dovec"><xsl:with-param name="mytype"><xsl:value-of>int</xsl:value-of></xsl:with-param></xsl:call-template></xsl:if><xsl:if test="position()&lt;last()">, </xsl:if>
				  </xsl:when>
				  <xsl:otherwise>
					  <xsl:text disable-output-escaping="yes"> std::vector&lt;int&gt;&amp; </xsl:text><xsl:value-of select="@name"/>
			                   <xsl:if test="$defdirection='out'">=<xsl:value-of select="../../../@name"/>::<xsl:value-of select="@name"/>_<xsl:value-of select="$myname"/></xsl:if>
					  <xsl:if test="position()&lt;last()">, </xsl:if>
				  </xsl:otherwise>
			  </xsl:choose>
		  </xsl:otherwise>
	  </xsl:choose>
  </xsl:when>
 <xsl:when test="lower-case(@xsi:type)='boolarray'">
	  <xsl:choose>
		  <xsl:when test="@direction">
			  <xsl:if test="@direction='in'"> const </xsl:if><xsl:text disable-output-escaping="yes"> std::vector&lt;bool&gt;&amp;  </xsl:text><xsl:value-of select="@name"/><xsl:if test="aps:value"><xsl:call-template name="dovec"><xsl:with-param name="mytype"><xsl:value-of>bool</xsl:value-of></xsl:with-param></xsl:call-template></xsl:if><xsl:if test="position()&lt;last()">, </xsl:if>
		  </xsl:when>
		  <xsl:otherwise>
			  <xsl:choose>
				  <xsl:when test="$defdirection='in'">
					  <xsl:text disable-output-escaping="yes"> const std::vector&lt;bool&gt;&amp;  </xsl:text><xsl:value-of select="@name"/><xsl:if test="aps:value"><xsl:call-template name="dovec"><xsl:with-param name="mytype"><xsl:value-of>bool</xsl:value-of></xsl:with-param></xsl:call-template></xsl:if><xsl:if test="position()&lt;last()">, </xsl:if>
				  </xsl:when>
				  <xsl:otherwise>
					  <xsl:text disable-output-escaping="yes"> std::vector&lt;bool&gt;&amp;  </xsl:text><xsl:value-of select="@name"/>
			                   <xsl:if test="$defdirection='out'">=<xsl:value-of select="../../../@name"/>::<xsl:value-of select="@name"/>_<xsl:value-of select="$myname"/></xsl:if>
					  <xsl:if test="position()&lt;last()">, </xsl:if>
				  </xsl:otherwise>
			  </xsl:choose>
		  </xsl:otherwise>
	  </xsl:choose>
  </xsl:when>
<xsl:when test="lower-case(@xsi:type)='floatarray'">
	  <xsl:choose>
		  <xsl:when test="@direction">
       	<xsl:if test="@direction='in' "> const </xsl:if><xsl:text disable-output-escaping="yes"> std::vector&lt;float&gt;&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if>
		  </xsl:when>
		  <xsl:otherwise>
			  <xsl:choose>
				  <xsl:when test="$defdirection='in'">
       	<xsl:text disable-output-escaping="yes"> const std::vector&lt;float&gt;&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if>
				  </xsl:when>
				  <xsl:otherwise>
       	<xsl:text disable-output-escaping="yes"> std::vector&lt;float&gt;&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if>
				  </xsl:otherwise>
			  </xsl:choose>
		  </xsl:otherwise>
	  </xsl:choose>
  </xsl:when>
<xsl:when test="lower-case(@xsi:type)='complexarray'">
	  <xsl:choose>
		  <xsl:when test="@direction">
       	<xsl:if test="@direction='in' "> const </xsl:if><xsl:text disable-output-escaping="yes"> std::vector&lt;casac::complex&gt;&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if>
		  </xsl:when>
		  <xsl:otherwise>
			  <xsl:choose>
				  <xsl:when test="$defdirection='in'">
       	<xsl:text disable-output-escaping="yes"> const std::vector&lt;casac::complex&gt;&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if>
				  </xsl:when>
				  <xsl:otherwise>
       	<xsl:text disable-output-escaping="yes"> std::vector&lt;casac::complex&gt;&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if>
				  </xsl:otherwise>
			  </xsl:choose>
		  </xsl:otherwise>
	  </xsl:choose>
  </xsl:when>
<xsl:when test="lower-case(@xsi:type)='doublearray'">
	<xsl:choose>
         <xsl:when test="@units">
	  <xsl:choose>
		  <xsl:when test="@direction">
			  <xsl:if test="@direction='in'"> const</xsl:if><xsl:text disable-output-escaping="yes"> Quantity&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="aps:value"><xsl:call-template name="dovec"><xsl:with-param name="mytype"><xsl:value-of>double</xsl:value-of></xsl:with-param></xsl:call-template></xsl:if><xsl:if test="position()&lt;last()">, </xsl:if>
		  </xsl:when>
		  <xsl:otherwise>
			  <xsl:choose>
				  <xsl:when test="$defdirection='in'">
					  <xsl:text disable-output-escaping="yes"> const Quantity&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="aps:value"><xsl:call-template name="dovec"><xsl:with-param name="mytype"><xsl:value-of>double</xsl:value-of></xsl:with-param></xsl:call-template></xsl:if><xsl:if test="position()&lt;last()">, </xsl:if>
				  </xsl:when>
				  <xsl:otherwise>
			 <xsl:text disable-output-escaping="yes"> Quantity&amp; </xsl:text><xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if>
				  </xsl:otherwise>
			  </xsl:choose>
		  </xsl:otherwise>
	  </xsl:choose>
	</xsl:when>
	<xsl:otherwise>
	  <xsl:choose>
		  <xsl:when test="@direction">
			  <xsl:if test="@direction='in' "> const</xsl:if><xsl:text disable-output-escaping="yes"> std::vector&lt;double&gt;&amp; </xsl:text> <xsl:value-of select="@name"/><xsl:if test="aps:value"><xsl:call-template name="dovec"><xsl:with-param name="mytype"><xsl:value-of>double</xsl:value-of></xsl:with-param></xsl:call-template></xsl:if><xsl:if test="position()&lt;last()">, </xsl:if>
		  </xsl:when>
		  <xsl:otherwise>
			  <xsl:choose>
				  <xsl:when test="$defdirection='in'">
					  <xsl:text disable-output-escaping="yes"> const std::vector&lt;double&gt;&amp; </xsl:text> <xsl:value-of select="@name"/>
					  <xsl:if test="aps:value"><xsl:call-template name="dovec"><xsl:with-param name="mytype"><xsl:value-of>double</xsl:value-of></xsl:with-param></xsl:call-template></xsl:if>
					  <xsl:if test="position()&lt;last()">, </xsl:if>
				  </xsl:when>
				  <xsl:otherwise>
					  <xsl:text disable-output-escaping="yes"> std::vector&lt;double&gt;&amp; </xsl:text> <xsl:value-of select="@name"/>
			                  <xsl:if test="$defdirection='out'">=<xsl:value-of select="../../../@name"/>::<xsl:value-of select="@name"/>_<xsl:value-of select="$myname"/></xsl:if>
					  <xsl:if test="position()&lt;last()">, </xsl:if>
				  </xsl:otherwise>
			  </xsl:choose>
		  </xsl:otherwise>
	  </xsl:choose>
      </xsl:otherwise>
    </xsl:choose>
</xsl:when>   
   <xsl:otherwise>
	  <xsl:choose>
		  <xsl:when test="@direction">
			 <xsl:text> </xsl:text><xsl:value-of select='@xsi:type'/><xsl:if test="@direction='in' "> const</xsl:if><xsl:text> </xsl:text> <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if>
		  </xsl:when>
		  <xsl:otherwise>
			  <xsl:choose>
				  <xsl:when test="$defdirection='in'">
			 <xsl:text> const casac::</xsl:text><xsl:value-of select='@xsi:type'/> <xsl:text> *</xsl:text> <xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if>
				  </xsl:when>
				  <xsl:otherwise>
			 <xsl:text> casac::</xsl:text><xsl:value-of select='@xsi:type'/><xsl:text> </xsl:text> *<xsl:value-of select="@name"/><xsl:if test="position()&lt;last()">, </xsl:if>
				  </xsl:otherwise>
			  </xsl:choose>
		  </xsl:otherwise>
	  </xsl:choose>
    </xsl:otherwise>
</xsl:choose>
     </xsl:for-each>   
</xsl:template>
     <xsl:template match="aps:returns">  
              <xsl:choose>
                <xsl:when test="lower-case(@xsi:type)='string'">string </xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='int'">int </xsl:when>
                  <xsl:when test="lower-case(@xsi:type)='bool'">bool </xsl:when>
                  <xsl:when test="lower-case(@xsi:type)='boolean'">bool </xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='float'">float </xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='double'">double </xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='quantity'">Quantity </xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='any'">variant* </xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='variant'">variant* </xsl:when>

		 <xsl:when test="lower-case(@xsi:type)='stringarray'"><xsl:text disable-output-escaping="yes">std::vector&lt;std::string&gt; </xsl:text></xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='recordarray'">RecordVec </xsl:when>
		 <xsl:when test="lower-case(@xsi:type)='intarray'"><xsl:text disable-output-escaping="yes">std::vector&lt;int&gt; </xsl:text></xsl:when>
			 <xsl:when test="lower-case(@xsi:type)='boolarray'"><xsl:text disable-output-escaping="yes">std::vector&lt;bool&gt; </xsl:text> </xsl:when>
				 <xsl:when test="lower-case(@xsi:type)='floatarray'"><xsl:text disable-output-escaping="yes">std::vector&lt;float&gt;</xsl:text> </xsl:when>
					 <xsl:when test="lower-case(@xsi:type)='doublearray'"><xsl:text disable-output-escaping="yes">std::vector&lt;double&gt;</xsl:text> </xsl:when>
						 <xsl:when test="lower-case(@xsi:type)='complexarray'"><xsl:text disable-output-escaping="yes">std::vector&lt;casac::complex&gt;</xsl:text> </xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='record'">record* </xsl:when>
                 <xsl:when test="lower-case(@xsi:type)='void'">void </xsl:when>
                  <xsl:when test="@xsi:type=''">void </xsl:when>
                 <xsl:otherwise>
                   <xsl:choose>
                    <xsl:when test="string-length(@xsi:type)=0">void </xsl:when>
                    <xsl:otherwise>
                                      casac::<xsl:value-of select='@xsi:type'/><xsl:text>* </xsl:text>
                   </xsl:otherwise>
                    </xsl:choose>
                    </xsl:otherwise>
              </xsl:choose>

  
  </xsl:template>     
     
   
 
  <!-- templates go here -->
</xsl:stylesheet>
