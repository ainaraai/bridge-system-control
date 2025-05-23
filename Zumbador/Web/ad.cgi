t <html><head><title>AD Input</title>
t <script language=JavaScript type="text/javascript" src="xml_http.js"></script>
t <script language=JavaScript type="text/javascript">
# Define URL and refresh timeout
t var formUpdate = new periodicObj("ad.cgx", 500);
t var formUpdate_2 = new periodicObj("ad_2.cgx", 500);
t function plotADGraph() {
t  adVal = document.getElementById("ad_value").value;
t  numVal = parseInt(adVal, 16);
t  voltsVal = (3.3*numVal)/4096;
t  tableSize = (numVal*100/4096);
t  document.getElementById("ad_table").style.width = (tableSize + '%');
t  document.getElementById("ad_volts").value = (voltsVal.toFixed(3) + ' V');
t }
t function plotADGraph2() {
t  adVal = document.getElementById("ad_value_2").value;
t  numVal = parseInt(adVal, 16);
t  voltsVal = (3.3*numVal)/4096;
t  tableSize = (numVal*100/4096);
t  document.getElementById("ad_table_2").style.width = (tableSize + '%');
t  document.getElementById("ad_volts_2").value = (voltsVal.toFixed(3) + ' V');
t }
t function periodicUpdateAd() {
t  if(document.getElementById("adChkBox").checked == true) {
t   updateMultiple(formUpdate,plotADGraph);
t   ad_elTime = setTimeout(periodicUpdateAd, formUpdate.period);
t  }
t  else
t   clearTimeout(ad_elTime);
t }
t function periodicUpdateAd2() {
t  if(document.getElementById("adChkBox_2").checked == true) {
t   updateMultiple(formUpdate_2,plotADGraph2);
t   ad_elTime = setTimeout(periodicUpdateAd2, formUpdate_2.period);
t  }
t  else
t   clearTimeout(ad_elTime);
t }
t </script></head>
i pg_header.inc
t <h2 align="center"><br>AD Converter Input</h2>
t <p><font size="2">This page allows you to monitor AD input value in numeric
t  and graphics form. Periodic screen update is based on <b>xml</b> technology.
t  This results in smooth flicker-free screen update.<br><br>
t  Turn potentiometer on an evaluation board clockwise or counterclockwise
t  and observe the change of AD value on the screen.</font></p>
t <form action="ad.cgi" method="post" name="ad">
t <input type="hidden" value="ad" name="pg">
t <table border=0 width=99%><font size="3">
t <tr style="background-color: #aaccff">
t  <th width=15%>Item</th>
t  <th width=15%>Value</th>
t  <th width=15%>Volts</th>
t  <th width=55%>Bargraph</th></tr>
t <tr><td><img src="pabb.gif">POT1:</td>
t   <td align="center">
t <input type="text" readonly style="background-color: transparent; border: 0px"
c g 1  size="10" id="ad_value" value="0x%03X"></td>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 0px"
c g 2  size="10" id="ad_volts" value="%5.3f V"></td>
t <td height=50><table bgcolor="#FFFFFF" border="2" cellpadding="0" cellspacing="0" width="100%"><tr>
c g 3 <td><table id="ad_table" style="width: %d%%" border="0" cellpadding="0" cellspacing="0">
t <tr><td bgcolor="#0000FF">&nbsp;</td></tr></table></td></tr></table></td></tr>
t <tr><td><img src="pabb.gif">POT2:</td>
t   <td align="center">
t <input type="text" readonly style="background-color: transparent; border: 0px"
c g 4  size="10" id="ad_value_2" value="0x%03X"></td>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 0px"
c g 5  size="10" id="ad_volts_2" value="%5.3f V"></td>
t <td height=50><table bgcolor="#FFFFFF" border="2" cellpadding="0" cellspacing="0" width="100%"><tr>
c g 6 <td><table id="ad_table_2" style="width: %d%%" border="0" cellpadding="0" cellspacing="0">
t <tr><td bgcolor="#0000FF">&nbsp;</td></tr></table></td></tr></table></td></tr>
t </font></table>
t <p align=center>
t <input type=button value="Refresh" onclick="updateMultiple(formUpdate,plotADGraph)">
t <input type=button value="Refresh 2" onclick="updateMultiple(formUpdate_2,plotADGraph2)">
t Periodic:<input type="checkbox" id="adChkBox" onclick="periodicUpdateAd()">
t Periodic 2:<input type="checkbox" id="adChkBox_2" onclick="periodicUpdateAd2()">
t </p></form>
i pg_footer.inc
. End of script must be closed with period
