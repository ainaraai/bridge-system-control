t <html><head><title>Time Control</title>
t <script language=JavaScript type="text/javascript" src="xml_http.js"></script>
t <script language=JavaScript type="text/javascript">
# Define URL and refresh timeout
t var formUpdate = new periodicObj("time.cgx", 500);
t function plotTimeGraph() {
t  document.getElementById("time_value").value;
t  document.getElementById("date_value").value;
t }
t function periodicUpdateTime() {
t  if(document.getElementById("adChkBox").checked == true) {
t   updateMultiple(formUpdate,plotTimeGraph);
t   ad_elTime = setTimeout(periodicUpdateTime, formUpdate.period);
t  }
t  else
t   clearTimeout(ad_elTime);
t }
t </script></head>
i pg_header.inc
t <h2 align=center><img src=watch.gif><br>Time Module Control</h2>
t <p><font size="2">This page allows you to visualize the date and hour of the system provided by the RTC.<br><br>
t  This Form uses a <b>POST</b> method to send data to a Web server.</font></p>
t <form action=time.cgi method=post name=cgi>
t <input type=hidden value="time" name=pg>
t <table border=0 width=99%><font size="3">
t <tr bgcolor=#aaccff>
t  <th width=40%>Item</th> 
t  <th width=60%>Setting</th></tr>
# Here begin data setting which is formatted in HTTP_CGI.C module
t <tr><td><img src=pabb.gif>System Hour</td>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 5px"
c j 1  size="14" id="time_value" value="%s"></td></tr>
t <tr><td><img src=pabb.gif>System Date</TD>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 5px"
c j 2  size="14" id="date_value" value="%s"></td></tr>
t </font></table>
# Here begin button definitions
t <p align=center>
t Periodic:<input type="checkbox" id="adChkBox" onclick="periodicUpdateTime()">
t </p></form>
i pg_footer.inc
. End of script must be closed with period.