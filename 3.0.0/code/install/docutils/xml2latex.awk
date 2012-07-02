#
# Simple script to finish up the translation from XML to latex
# Basically it figures out when to use math mode and when not to use math mode
#
BEGIN { verbatimFlag = 0
        mathFlag = 0
        mathEnv = 0}
/begin.*verbatim/{verbatimFlag = 1}
/end.*verbatim/{verbatimFlag = 0}
/begin.*equation/{mathFlag = 1}
/begin.*displaymath/{mathFlag = 1}
/end.*displaymath/{mathFlag = 0}
/end.*equation/{mathFlag = 0}
/\$.*\$/{mathEnv = 1}
//{if(verbatimFlag || mathFlag || mathEnv){
    theText = gsub("&gt;",">", $0)
    theText = gsub("&lt;","<", $0)
    mathEnv = 0
   }else{
    theText = gsub("$&gt;$","$>$", $0)
    theText = gsub("&gt;","$>$", $0)
    theText = gsub("$&lt;$","$<$", $0)
    theText = gsub("&lt;","$<$", $0)
    #Escape those _'s that haven't been escaped, hoping to miss the math environs though!
    if(match($0,"[^\\\\]\\\^")!= 0) gsub("\\\^","\\^",$0)
    if(match($0,"[^\\\\]_")!= 0) gsub("_","\\_",$0)
    if(match($0,"[^\\\\]%")!= 0)gsub("%","\\%",$0)
   }
   print $0
  }
