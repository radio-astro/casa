<link rel="stylesheet"  type="text/css" href="/common/nraostyle.css" > 
<style type="text/css">
body {margin-left: 5pt;}
</style>

<script language="Javascript" type="text/javascript">

var dayNames = new Array ();
dayNames[0]="Sunday";
dayNames[1]="Monday";
dayNames[2]="Tuesday";
dayNames[3]="Wednesday";
dayNames[4]="Thursday";
dayNames[5]="Friday";
dayNames[6]="Saturday";

var monthNames = new Array ();
monthNames[0]="January";
monthNames[1]="February";
monthNames[2]="March";
monthNames[3]="April";
monthNames[4]="May";
monthNames[5]="June";
monthNames[6]="July";
monthNames[7]="August";
monthNames[8]="September";
monthNames[9]="October";
monthNames[10]="November";
monthNames[11]="December";

var now = new Date();
var day = now.getDay();
var month = now.getMonth();
var year = now.getFullYear();
var date = now.getDate();

function MCollapse( number ) {
  var mID = 'lmenu' + number;
  var mDIV = document.getElementById( mID );
  mDIV.className = (mDIV.className == 'collapsed') ? 'open' : 'collapsed';
  mID = 'trig' + number;
  var tDIV = document.getElementById( mID );
  tDIV.className = (tDIV.className == 'tplus') ? 'tminus' : 'tplus';
}

</script>

