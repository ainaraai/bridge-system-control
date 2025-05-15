/* Main function, that handles request and responses in background.
 * Response functions are handled if response code equals to OK 200. */
function updateMultiple(formUpd, callBack, userName, userPassword) {
 let xmlHttp = GetXmlHttpObject();
 if(xmlHttp == null) {
  alert("XmlHttp not initialized!");
  return 0;
 }
 xmlHttp.onreadystatechange = responseHandler; 
 xmlHttp.open("GET", formUpd.url, true, userName, userPassword);
 xmlHttp.send(null);

 function responseHandler(){
  if(xmlHttp.readyState == 4) { //response ready 
   if(xmlHttp.status == 200) { //handle received data
    var xmlDoc = xmlHttp.responseXML;
    if(xmlDoc == null)
     return 0;
    try {  //catching IE bug
     processResponse(xmlDoc);
    }
    catch(e) {
     return 0;
    }
    /* Callback function for custom update. */
    if (callBack != undefined)
     callBack();
   }
   else if(xmlHttp.status == 401)
    alert("Error code 401: Unauthorized");
   else if(xmlHttp.status == 403)
    alert("Error code 403: Forbidden");
   else if(xmlHttp.status == 404)
    alert("Error code 404: URL not found!");
  }
 }
}

// function processResponse(xmlDoc) {
//  textElementArr = xmlDoc.getElementsByTagName("text");
//  for(var i = 0; i < textElementArr.length; i++) {
//   try {
//    elId = textElementArr[i].childNodes[0].childNodes[0].nodeValue;
//    elValue = textElementArr[i].childNodes[1].childNodes[0].nodeValue;
//    document.getElementById(elId).value = elValue;
//   }
//   catch(error) {
//    if(elId == undefined){
//     continue;
//    }
//    else if(elValue == undefined) {
//     elValue = "";
//     document.getElementById(elId).value = elValue;
//    }
//   }
//  }
// data = xmlDoc.getElementsByTagName("chart");
//  try {
//    bridgeVars.openBridge = parseInt(data[0].childNodes[0].nodeValue);
//  } catch (error) {
//    for (var mesasure in bridgeVars) {
//      if (mesasure == undefined)
//        bridgeVars[mesasure] = "";
//    }
//  }
// }
// textElementArr = xmlDoc.getElementsByTagName("data");
// for(var i = 0; i < textElementArr.length; i++) {
//  try {
//   elId = textElementArr[i].childNodes[0].childNodes[0].nodeValue;
//   elValue = textElementArr[i].childNodes[1].childNodes[0].nodeValue;
////    document.getElementById(elId).value = elValue;
//    chartObjData.new_value = elValue;
//    
//  }
//  catch(error) {
//   if(elId == undefined){
//    continue;
//   }
//   else if(elValue == undefined) {
//    elValue = "";
//    // document.getElementById(elId).value = elValue;
//        chartObjData.new_value = elValue;
//   }
//  }
// }
// textElementArr = xmlDoc.getElementsByTagName("time");
// for(var i = 0; i < textElementArr.length; i++) {
//  try {
//   elId = textElementArr[i].childNodes[0].childNodes[0].nodeValue;
//   elValue = textElementArr[i].childNodes[1].childNodes[0].nodeValue;
////    document.getElementById(elId).value = elValue;
//    chartObjData.new_time = elValue;
//  }
//  catch(error) {
//   if(elId == undefined){
//    continue;
//   }
//   else if(elValue == undefined) {
//    elValue = "";
//    // document.getElementById(elId).value = elValue;
//        chartObjData.new_time = elValue;
//   }
//  }
// }
// }

function processResponse(xmlDoc) {
  data = xmlDoc.getElementsByTagName("control");
	if (data.length > 0) {
  try {
    bridgeVars.bridgeStatus  = parseInt(data[0].childNodes[0].textContent);
    bridgeVars.barrier       = parseInt(data[0].childNodes[1].textContent);
    bridgeVars.time          = data[0].childNodes[2].textContent;
    bridgeVars.date          = data[0].childNodes[3].textContent;
    bridgeVars.emergencyStop = parseInt(data[0].childNodes[4].textContent);
    bridgeVars.boatWarn      = parseInt(data[0].childNodes[5].textContent);
    
  } catch (error) {
    for (var mesasure in bridgeVars) {
      if (mesasure == undefined)
        bridgeVars[mesasure] = "";
    }
  }
}
		textElementArr = xmlDoc.getElementsByTagName("text");
	if (textElementArr.length > 0) {
  for(var i = 0; i < textElementArr.length; i++) {
   try {
    elId = textElementArr[i].childNodes[0].childNodes[0].nodeValue;
    elValue = textElementArr[i].childNodes[1].childNodes[0].nodeValue;
    document.getElementById(elId).value = elValue;
   }
   catch(error) {
    if(elId == undefined){
     continue;
    }
    else if(elValue == undefined) {
     elValue = "";
     document.getElementById(elId).value = elValue;
    }
   }
  }
}
}

/* XMLHttpRequest object specific functions */
function GetXmlHttpObject() { //init XMLHttp object
 var xmlHttp=null;
 try {
  xmlHttp=new XMLHttpRequest(); // Firefox, Opera 8.0+, Safari
 }
 catch (e) {
  try {   // Internet Explorer
   xmlHttp=new ActiveXObject("Msxml2.XMLHTTP");
  }
  catch (e) {
   xmlHttp=new ActiveXObject("Microsoft.XMLHTTP");
  }
 }
 return xmlHttp;
}

/* Objects templates */
function periodicObj(url, period) {
 this.url = url;
 this.period = (typeof period == "undefined") ? 0 : period;
}
