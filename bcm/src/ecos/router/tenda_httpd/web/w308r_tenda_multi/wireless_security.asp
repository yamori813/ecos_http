<HTML> 
<HEAD>
<META http-equiv="Pragma" content="no-cache">
<META http-equiv="Content-Type" content="text/html; charset=UTF-8">
<TITLE>Wireless | Security Settings</TITLE>
<script type="text/javascript" src="lang/b28n.js"></script>
<SCRIPT language=JavaScript src="gozila.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu.js"></SCRIPT>
<SCRIPT language=JavaScript src="menu2.js"></SCRIPT>
<SCRIPT language=JavaScript src="table.js"></SCRIPT>
<script language="JavaScript" type="text/javascript">
var changed = 0;
var old_MBSSID;
var SSID = new Array();
var Secumode = new Array();
var AuthMode = new Array();
var EncrypType = new Array();
var DefaultKeyID = new Array();
var Key1Type = new Array();
var Key1Str = new Array();
var Key2Type = new Array();
var Key2Str = new Array();
var Key3Type = new Array();
var Key3Str = new Array();
var Key4Type = new Array();
var Key4Str = new Array();
var WPAPSK = new Array();
var Wepenable = new Array();
var RekeyInterval = new Array();
var Cur_ssid_index = new Array();
var wl0_mode = new Array();
var mode ;
var pin ;
var wpsmethod ;
var enablewireless ;
var isWPSConfiguredASP;
var OOB = 0;
var secs;
var timerID = null;
var timerRunning = false;
var timeout = 10;
var delay = 1000;
Butterlate.setTextDomain("wireless_set");
function initTranslate(){
	var e=document.getElementById("rstOOB");
	e.value=_("Reset OOB");
}
function trim(s){
    try{
        return s.replace(/^\s+|\s+$/g,"");
    }catch(e){
        return s;
    }
}
function InitializeTimer(){
    secs = timeout;
    StopTheClock();
    StartTheTimer();
}
function StopTheClock(){
    if(timerRunning)
        clearTimeout(timerID);
    timerRunning = false;
}
function StartTheTimer(){
    if (secs==0){
		StopTheClock();	
		secs = timeout;
		StartTheTimer();
    }else{
        secs = secs - 1;
        timerRunning = true;
        timerID = self.setTimeout("StartTheTimer()", delay);
    }
}
function checkRange(str, num, min, max){
    d = atoi(str,num);
    if(d > max || d < min)
        return false;
    return true;
}
function checkIpAddr(field){
    if(field.value == "")
        return false;
    if ( checkAllNum(field.value) == 0)
        return false;
    if( (!checkRange(field.value,1,0,255)) ||
        (!checkRange(field.value,2,0,255)) ||
        (!checkRange(field.value,3,0,255)) ||
        (!checkRange(field.value,4,1,254)) ){
        return false;
    }
   return true;
}
function atoi(str, num){
    i=1;
    if(num != 1 ){
        while (i != num && str.length != 0){
            if(str.charAt(0) == '.'){
                i++;
            }
            str = str.substring(1);
        }
        if(i != num )
            return -1;
    }
    for(i=0; i<str.length; i++){
        if(str.charAt(i) == '.'){
            str = str.substring(0, i);
            break;
        }
    }
    if(str.length == 0)
        return -1;
    return parseInt(str, 10);
}
function checkHex(str){
	var len = str.length;
	for (var i=0; i<str.length; i++) {
		if ((str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
			(str.charAt(i) >= 'a' && str.charAt(i) <= 'f') ||
			(str.charAt(i) >= 'A' && str.charAt(i) <= 'F') ){
				continue;
		}else
	        return false;
	}
    return true;
}


function checkInjection(str)
{
	var len = str.length;
	for (var i=0; i<str.length; i++) {
		if ( str.charAt(i) == ':' || str.charAt(i) == ',' ||
			  str.charAt(i) == '\r' || str.charAt(i) == '\n' ||
			  str.charAt(i) == '\"' || str.charAt(i) == '\'' ||
			  str.charAt(i) == '\\' || str.charAt(i) == '\/' ||
			  str.charAt(i) == '¡°' || str.charAt(i) == '¡®' ||
			  str.charAt(i) == '¡¯' || str.charAt(i) == '¡±' ||
			  str.charAt(i) == ' ' ){
				return false;
		}else
	        continue;
	}
    return true;
}

var illegal_user_pass = new Array("\\r","\\n","\\","&","%","!", "#","$","^","*","(",")","-","+","=","?",";",":","'","\"","<",">",",","~");
function checkAllNum(str){
    for (var i=0; i<str.length; i++){
        if((str.charAt(i) >= '0' && str.charAt(i) <= '9') || (str.charAt(i) == '.' ))
            continue;
        return false;
    }
    return true;
}
function style_display_on()
{
	if (window.ActiveXObject) {
		return "block";
	}
	else if (window.XMLHttpRequest) { 
		return "table-row";
	}
}
var http_request = false;
function makeRequest(url, content) {
	http_request = false;
	if (window.XMLHttpRequest) {
		http_request = new XMLHttpRequest();
		if (http_request.overrideMimeType) {
			http_request.overrideMimeType('text/xml');
		}
	} else if (window.ActiveXObject) {
		try {
			http_request = new ActiveXObject("Msxml2.XMLHTTP");
		} catch (e) {
			try {
			http_request = new ActiveXObject("Microsoft.XMLHTTP");
			} catch (e) {}
		}
	}
	if (!http_request) {
		alert(_("Warning: Can not create XMLHTTP example"));
		return false;
	}
	http_request.onreadystatechange = alertContents;
	http_request.open('POST', url, true);
	http_request.send(content);
}
function alertContents() {
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {
			parseAllData(http_request.responseText);
			UpdateMBSSIDList();
			LoadFields(0);
		if(mode == "disabled"){
			document.security_form.wifienable[0].checked = true;
			onwifiuse(0);
		}else if (mode == "enabled"){
			document.security_form.wifienable[1].checked = true;
			onwifiuse(1);
			if(wpsmethod == "pbc"){
			   document.security_form.wpsMode[0].checked = true;
			   document.getElementById("PIN").style.display = "none";
			}else if(wpsmethod == "pin"){
			   document.security_form.wpsMode[1].checked = true;
			   document.getElementById("PIN").style.display = "";
			   document.security_form.PIN.value = pin;
			}else{
			}
		}
		InitializeTimer();
			if(isWPSConfiguredASP==1 && wl0_mode[0] != "sta")
			{
				//alert(_("Information: the security setting is automatically set to WPS,but you can manually modify it."));
			}
		} else {
		}
	}
}
var http_request1 = false;
function makeRequest1(url, content) {
	http_request1 = false;
	if (window.XMLHttpRequest) {
		http_request1 = new XMLHttpRequest();
		if (http_request1.overrideMimeType) {
			http_request1.overrideMimeType('text/xml');
		}
	} else if (window.ActiveXObject) {
		try {
			http_request1 = new ActiveXObject("Msxml2.XMLHTTP");
		} catch (e) {
			try {
			http_request1 = new ActiveXObject("Microsoft.XMLHTTP");
			} catch (e) {}
		}
	}
	if (!http_request1) {
		alert(_("Giving up"));
		return false;
	}
	http_request1.onreadystatechange = alertContents1;
	http_request1.open('POST', url, true);
	http_request1.send(content);
}
function alertContents1() {
	if (http_request1.readyState == 4) {
		if (http_request1.status == 200) {
		    if(OOB == 1){
			    	window.document.body.style.cursor = "auto";
	            		document.getElementById("rstOOB").disabled = false;
				OOB = 0;
			}else{
			}	
		} else {
		}
	}
}
var http_request2 = false;
function makeRequest2(url, content) {
	http_request2 = false;
	if (window.XMLHttpRequest) {
		http_request2 = new XMLHttpRequest();
		if (http_request2.overrideMimeType) {
			http_request2.overrideMimeType('text/xml');
		}
	} else if (window.ActiveXObject) {
		try {
			http_request2 = new ActiveXObject("Msxml2.XMLHTTP");
		} catch (e) {
			try {
			http_request2 = new ActiveXObject("Microsoft.XMLHTTP");
			} catch (e) {}
		}
	}
	if (!http_request2) {
		alert(_("Giving up"));
		return false;
	}
	http_request2.onreadystatechange = alertContents2;
	http_request2.open('POST', url, true);
	http_request2.send(content);
}
function alertContents2() {
	if (http_request2.readyState == 4) {
		if (http_request2.status == 200) {
		 	var str=http_request2.responseText.split("\r");
		  	mode = str[0] ;
		 	pin =str[1];
			wpsmethod = str[2];
			enablewireless= str[3];
			isWPSConfiguredASP = str[4];
			initAll();
		} else {
		}
	}
}
function resetOOB()
{
   window.document.body.style.cursor = "wait";
   OOB = 1;
   makeRequest1("/goform/OOB", "something");
}
function parseAllData(str)
{
	var all_str = new Array();
	all_str = str.split("\n");
	for (var i=0; i<all_str.length; i++) {
		var fields_str = new Array();
		fields_str = all_str[i].split("\r");
		SSID[i] = trim(fields_str[0]);
		if(i == 1) break;
		Secumode[i] = trim(fields_str[1]);
		AuthMode[i] = trim(fields_str[2]);
		EncrypType[i] = trim(fields_str[3]);
		DefaultKeyID[i] = trim(fields_str[4]);
		Key1Type[i] = trim(fields_str[5]);
		Key1Str[i] = trim(fields_str[6]);
		Key2Type[i] = trim(fields_str[7]);
		Key2Str[i] = trim(fields_str[8]);
		Key3Type[i] = trim(fields_str[9]);
		Key3Str[i] = trim(fields_str[10]);
		Key4Type[i] = trim(fields_str[11]);
		Key4Str[i] = trim(fields_str[12]);
		WPAPSK[i] = trim(fields_str[13]);
		Wepenable[i] = trim(fields_str[14]);
		RekeyInterval[i] = trim(fields_str[15]);
		Cur_ssid_index[i] = trim(fields_str[16]);
		wl0_mode[i] = trim(fields_str[17]);
		
		if(AuthMode[i] == "0" && (Secumode[i]) == "" && Wepenable[i] == "disabled" )
			AuthMode[i] = "Disable";
		if(AuthMode[i] == "0" && Secumode[i] == "psk" &&  Wepenable[i] == "disabled"  )
			AuthMode[i] = "psk";
		if(AuthMode[i] == "0" && Secumode[i] == "psk2" &&  Wepenable[i] == "disabled" )
			AuthMode[i] = "psk2";
		if(AuthMode[i] == "0" && (Secumode[i] == "psk psk2") &&  Wepenable[i] == "disabled" )
			AuthMode[i] = "psk psk2";
		if(AuthMode[i] == "0" && Secumode[i] == "0" && Wepenable[i] == "enabled")//open
			AuthMode[i] = "open";
		if(AuthMode[i] == "1" && Secumode[i] == "1" && Wepenable[i] == "enabled")//share
			AuthMode[i] = "share";
			
	}
}

function checkData(){
	var securitymode;
	securitymode = document.security_form.security_mode.value;
	if (securitymode == "0" || securitymode == "1" ||securitymode == "WEPAUTO")
	{
		return check_Wep(securitymode);
	}else  if (securitymode == "psk" || securitymode == "psk2" || securitymode == "psk psk2" ){
		var keyvalue = document.security_form.passphrase.value;
		if (keyvalue.length == 0){
			alert(_("Please enter a key"));
			return false;
		}
		if ((keyvalue.length < 8) || (keyvalue.length > 63)){
			alert(_("Key range:8~63 characters"));
			return false;
		}
		if(!ill_check(document.security_form.passphrase.value,illegal_user_pass,_("Key"))) return false;
		if(checkInjection(document.security_form.passphrase.value) == false){
			alert(_("contains invalid characters"));
			return false;
		}
		var re =/^[0-9a-zA-Z_\-.,:]+$/;	
		if(document.security_form.passphrase.value == "" || !re.test(document.security_form.passphrase.value)){
				alert(_("contains invalid characters"));
				return false;
		}
		if(document.security_form.cipher[0].checked != true && 
		   document.security_form.cipher[1].checked != true &&
   		   document.security_form.cipher[2].checked != true){
   		   alert(_("Please select a WPA algorithm"));
   		   return false;
		}
		if(checkAllNum(document.security_form.keyRenewalInterval.value) == false){
			alert(_("Key renewal interval invalid"));
			return false;
		}
		if(document.security_form.keyRenewalInterval.value < 60){
			alert(_("Warning:Key renewal interval is too short,range ")+":60~9999");
			return false;
		}
		if(check_wpa() == false)
			return false;
	}
	return true;
}
function check_wpa(){
		if(document.security_form.cipher[0].checked != true && 
		   document.security_form.cipher[1].checked != true &&
   		   document.security_form.cipher[2].checked != true){
   		   alert(_("Please select a WPA algorithm"));
   		   return false;
		}
		if(checkAllNum(document.security_form.keyRenewalInterval.value) == false){
			alert(_("Please enter a valid key renewal interval"));
			return false;
		}
		if(document.security_form.keyRenewalInterval.value < 60){
			alert(_("Warning:Key renewal interval is too short,range ")+":60~9999");
			return false;
		}
		return true;
}
function securityMode(c_f,MBSSID){
	var security_mode;
	changed = c_f;
	
	hideWep();
	
	
	document.getElementById("div_security_shared_mode").style.visibility = "hidden";
	document.getElementById("div_security_shared_mode").style.display = "none";
	document.getElementById("div_wpa").style.visibility = "hidden";
	document.getElementById("div_wpa").style.display = "none";
	document.getElementById("div_wpa_algorithms").style.visibility = "hidden";
	document.getElementById("div_wpa_algorithms").style.display = "none";
	document.getElementById("wpa_passphrase").style.visibility = "hidden";
	document.getElementById("wpa_passphrase").style.display = "none";
	document.getElementById("wpa_key_renewal_interval").style.visibility = "hidden";
	document.getElementById("wpa_key_renewal_interval").style.display = "none";
	document.security_form.cipher[0].disabled = true;
	document.security_form.cipher[1].disabled = true;
	document.security_form.cipher[2].disabled = true;
	document.security_form.passphrase.disabled = true;
	document.security_form.keyRenewalInterval.disabled = true;
	security_mode = document.security_form.security_mode.value;
	
	
    if (security_mode == "0" || security_mode == "1" ||security_mode == "WEPAUTO"){
		showWep(security_mode);
	}else if (security_mode == "psk" || security_mode == "psk2" || security_mode == "psk psk2"){
		<!-- WPA -->
		document.getElementById("div_wpa").style.visibility = "visible";
		if (window.ActiveXObject) {
			document.getElementById("div_wpa").style.display = "block";
		}
		else if (window.XMLHttpRequest) {
			document.getElementById("div_wpa").style.display = "table";
		}
		document.getElementById("div_wpa_algorithms").style.visibility = "visible";
		document.getElementById("div_wpa_algorithms").style.display = style_display_on();
		document.security_form.cipher[0].disabled = false;
		document.security_form.cipher[1].disabled = false;
		if(security_mode == "psk" && document.security_form.cipher[2].checked)
			document.security_form.cipher[2].checked = false;
		 if(security_mode == "psk2" || security_mode == "psk psk2")
			document.security_form.cipher[2].disabled = false;
		document.getElementById("wpa_passphrase").style.visibility = "visible";
		document.getElementById("wpa_passphrase").style.display = style_display_on();
		document.security_form.passphrase.disabled = false;
		document.getElementById("wpa_key_renewal_interval").style.visibility = "visible";
		document.getElementById("wpa_key_renewal_interval").style.display = style_display_on();
		document.security_form.keyRenewalInterval.disabled = false;
	}
}


function hideWep()
{
	document.getElementById("div_wep").style.visibility = "hidden";
	document.getElementById("div_wep").style.display = "none";
}
function showWep(mode)
{
	<!-- WEP -->
	document.getElementById("div_wep").style.visibility = "visible";

	if (window.ActiveXObject) { // IE 
		document.getElementById("div_wep").style.display = "block";
	}
	else if (window.XMLHttpRequest) { // Mozilla, Safari...
		document.getElementById("div_wep").style.display = "table";
	}

	if(mode == "1"){
		document.getElementById("div_security_shared_mode").style.visibility = "visible";
		document.getElementById("div_security_shared_mode").style.display = style_display_on();
	}

}


function check_Wep(securitymode)
{
	var defaultid = document.security_form.wep_default_key.value;
	var key_input;

	if ( defaultid == 1 )
		var keyvalue = document.security_form.wep_key_1.value;
	else if (defaultid == 2)
		var keyvalue = document.security_form.wep_key_2.value;
	else if (defaultid == 3)
		var keyvalue = document.security_form.wep_key_3.value;
	else if (defaultid == 4)
		var keyvalue = document.security_form.wep_key_4.value;

	if(!ill_check(document.security_form.wep_key_1.value,illegal_user_pass,"key")) return false;
	if(!ill_check(document.security_form.wep_key_2.value,illegal_user_pass,"key")) return false;
	if(!ill_check(document.security_form.wep_key_3.value,illegal_user_pass,"key")) return false;
	if(!ill_check(document.security_form.wep_key_4.value,illegal_user_pass,"key")) return false;
	
	if (keyvalue.length == 0 &&  (securitymode == "1" || securitymode == "0")){ // shared wep  || md5
		alert('Please input WEP key'+defaultid+' !');
		return false;
	}

	var keylength = document.security_form.wep_key_1.value.length;
	if (keylength != 0){
		if (document.security_form.WEP1Select.options.selectedIndex == 0){
			if(keylength != 5 && keylength != 13) {
				alert('Wep key1 contains invalid characters,Please input 5 or 13 characters of wep key1 !');
				return false;
			}
			if(checkInjection(document.security_form.wep_key_1.value)== false){
				alert('Wep key1 contains invalid characters.');
				return false;
			}
		}
		if (document.security_form.WEP1Select.options.selectedIndex == 1){
			if(keylength != 10 && keylength != 26) {
				alert('Invalid Wep key1 format,Please input 10 or 26 characters of wep key1 !');
				return false;
			}
			if(checkHex(document.security_form.wep_key_1.value) == false){
				alert('Wep key1 contains invalid characters.');
				return false;
			}
		}
	}

	keylength = document.security_form.wep_key_2.value.length;
	if (keylength != 0){
		if (document.security_form.WEP2Select.options.selectedIndex == 0){
			if(keylength != 5 && keylength != 13) {
				alert('Wep key2 contains invalid characters,Please input 5 or 13 characters of wep key2 !');
				return false;
			}
			if(checkInjection(document.security_form.wep_key_1.value)== false){
				alert('Wep key2 contains invalid characters.');
				return false;
			}			
		}
		if (document.security_form.WEP2Select.options.selectedIndex == 1){
			if(keylength != 10 && keylength != 26) {
				alert('Invalid Wep key2 format,Please input 10 or 26 characters of wep key2 !');
				return false;
			}
			if(checkHex(document.security_form.wep_key_2.value) == false){
				alert('Wep key2 contains invalid characters.');
				return false;
			}
		}
	}

	keylength = document.security_form.wep_key_3.value.length;
	if (keylength != 0){
		if (document.security_form.WEP3Select.options.selectedIndex == 0){
			if(keylength != 5 && keylength != 13) {
				alert('Wep key3 contains invalid characters,Please input 5 or 13 characters of wep key3 !');
				return false;
			}
			if(checkInjection(document.security_form.wep_key_1.value)== false){
				alert('Wep key3 contains invalid characters.');
				return false;
			}
		}
		if (document.security_form.WEP3Select.options.selectedIndex == 1){
			if(keylength != 10 && keylength != 26) {
				alert('Invalid Wep key3 format,Please input 10 or 26 characters of wep key3 !');
				return false;
			}
			if(checkHex(document.security_form.wep_key_3.value) == false){
				alert('Wep key3 contains invalid characters.');
				return false;
			}			
		}
	}

	keylength = document.security_form.wep_key_4.value.length;
	if (keylength != 0){
		if (document.security_form.WEP4Select.options.selectedIndex == 0){
			if(keylength != 5 && keylength != 13) {
				alert('Wep key4 contains invalid characters,Please input 5 or 13 characters of wep key4 !');
				return false;
			}
			if(checkInjection(document.security_form.wep_key_1.value)== false){
				alert('Wep key4 contains invalid characters.');
				return false;
			}			
		}
		if (document.security_form.WEP4Select.options.selectedIndex == 1){
			if(keylength != 10 && keylength != 26) {
				alert('Invalid Wep key4 format,Please input 10 or 26 characters of wep key4 !');
				return false;
			}

			if(checkHex(document.security_form.wep_key_4.value) == false){
				alert('Wep key4 contains invalid characters.')
				return false;
			}			
		}
	}
	return true;
}

function preSubmit(){
    if(wl0_mode == "sta"){
		if(!window.confirm(_("You hava selected Wireless WAN access mode,please make sure current encryption settings match with that of the wireless WAN.")))
			return;
    }
    if(document.security_form.wifienable[1].checked == true){
	    var submitStr = "/goform/wirelessSetSecurity?GO=wireless_security.asp";
		var wifiEn, wpsmethod;
		wifiEn = "enabled";
        if(document.security_form.wpsMode[0].checked){
			wpsmethod ="pbc"
		   }
		else if(document.security_form.wpsMode[1].checked){   
			wpsmethod ="pin"  
		   }
		submitStr += "&wifiEn=" + wifiEn ;		
	    	submitStr += "&wpsmethod="+ wpsmethod;	
	    if(wifiEn == "enabled" && wpsmethod == "pin"){
		var t = /^[0-9]{8}$/;
		var v = document.forms[0].PIN.value;
		if(!t.test(v)){
				alert(_("PIN value invalid,please enter 8 characters."));
				return false;
		}	
		submitStr += "&PIN=" + v;
	}
	var code = 'location="' + submitStr + '"';
	eval(code);
	} else if(document.security_form.wifienable[0].checked == true){
	    document.security_form.wifiEn.value="disabled";
		document.security_form.wpsmethod.value="pbc";
		if (checkData() == true){
			changed = 0;
			document.security_form.submit();
		}
	}
}
function LoadFields(MBSSID){
	var result;
	sp_select = document.getElementById("security_mode");
	sp_select.options.length = 0;
	sp_select.options[sp_select.length] = new Option(_("Disable"),	"Disable",	false, AuthMode[MBSSID] == "Disable");
	sp_select.options[sp_select.length] = new Option("Open",	"0",		false, AuthMode[MBSSID] == "open");	//0:open
	sp_select.options[sp_select.length] = new Option("Shared",	"1", 	false, AuthMode[MBSSID] == "share");	//1:share
	sp_select.options[sp_select.length] = new Option("WPA - PSK", "psk",	false, AuthMode[MBSSID] == "psk");
	sp_select.options[sp_select.length] = new Option("WPA2 - PSK","psk2",	false, AuthMode[MBSSID] == "psk2");
	sp_select.options[sp_select.length] = new Option("Mixed WPA/WPA2 - PSK","psk psk2",	false, AuthMode[MBSSID] == "psk psk2");
	
	// WEP
	document.getElementById("WEP1").value = Key1Str[MBSSID];
	document.getElementById("WEP2").value = Key2Str[MBSSID];
	document.getElementById("WEP3").value = Key3Str[MBSSID];
	document.getElementById("WEP4").value = Key4Str[MBSSID];

	document.getElementById("WEP1Select").selectedIndex = (Key1Type[MBSSID] == "0" ? 1 : 0);
	document.getElementById("WEP2Select").selectedIndex = (Key2Type[MBSSID] == "0" ? 1 : 0);
	document.getElementById("WEP3Select").selectedIndex = (Key3Type[MBSSID] == "0" ? 1 : 0);
	document.getElementById("WEP4Select").selectedIndex = (Key4Type[MBSSID] == "0" ? 1 : 0);

	document.getElementById("wep_default_key").selectedIndex = parseInt(DefaultKeyID[MBSSID]) - 1 ;

	// SHARED && NONE
	if(AuthMode[MBSSID] == "1"/* && EncrypType[MBSSID] == "none"*/)
		document.getElementById("security_shared_mode").selectedIndex = 1;
	else
		document.getElementById("security_shared_mode").selectedIndex = 0;

	// WPA

	if(EncrypType[MBSSID] == "tkip")
		document.security_form.cipher[1].checked = true;
	else if(EncrypType[MBSSID] == "aes")
		document.security_form.cipher[0].checked = true;
	else if(EncrypType[MBSSID] == "tkip+aes")
		document.security_form.cipher[2].checked = true;
	document.getElementById("passphrase").value = WPAPSK[MBSSID];
	document.getElementById("keyRenewalInterval").value = RekeyInterval[MBSSID];		
	securityMode(0,MBSSID);
}
/*
 * When user select the different SSID, this function would be called.
 */ 
function selectMBSSIDChanged()
{
	var ssid_index;
	// check if any security settings changed
	if(changed){
		var ret = confirm(_("Are you sure to ignore the modification?"));
		if(!ret){
			document.security_form.ssidIndex.options.selectedIndex = old_MBSSID;
			return false;
		}
		else
			changed = 0;
	}
	ssid_index = document.security_form.ssidIndex.options.selectedIndex;
	
	var loc = "/goform/onSSIDChange?GO=wireless_security.asp";
	loc += "&ssid_index=" + ssid_index; 
	var code = 'location="' + loc + '"';
   	eval(code);
}
function changeSecurityPolicyTableTitle(t){
	var title = document.getElementById("sp_title");
	title.innerHTML = "SSID -- " + "\"" + t + "\"";
}
function init(){
	initTranslate();
	makeRequest2("/goform/wirelessInitSecurity", "something");
}
function initAll(){   
	if(enablewireless==1){
	    document.getElementById("rstOOB").disabled = true;
		makeRequest("/goform/wirelessGetSecurity", "something");
	}else{
		alert(_("This function can only be used after the wireless function is enabled"));
		top.mainFrame.location.href="wireless_basic.asp";
	}
}
function onwifiuse(x){
	if(x==0 || mode == "disabled"){
	 document.getElementById("wifiuse_wps").style.display="none";
	 document.getElementById("rstOOB").disabled = true;
	 document.getElementById("security_mode").disabled=false;
	 document.getElementById("passphrase").disabled=false;
	 document.getElementById("div_wpa_algorithms").disabled=false;
	 document.getElementById("keyRenewalInterval").disabled=false; 
	}else if(x==1){
	 document.getElementById("wifiuse_wps").style.display="";
	 document.getElementById("rstOOB").disabled = false;
	 document.getElementById("security_mode").disabled=true;
	 document.getElementById("passphrase").disabled=true;
	 document.getElementById("div_wpa_algorithms").disabled=false;
	 document.getElementById("keyRenewalInterval").disabled=true; 
	}
	if(wl0_mode[0] == "sta"){
		disableWpsSet();
	}
} 
function onSel(n){
	show_hide("PIN",( n == 1));
}
function UpdateMBSSIDList() {
	var defaultSelected = false,
		selected = false,
		optionStr = '&nbsp;&nbsp;&nbsp;&nbsp;<select name="ssidIndex" size="1" onChange="selectMBSSIDChanged()">';
	

	old_MBSSID = Cur_ssid_index[0];
	
	for(var i=0; i < SSID.length; i++){
		if(SSID[i] !== "") {
			if (old_MBSSID == i) {
				optionStr += '<option selected="selected">' + SSID[i] + '</option>';
			} else {
				optionStr += '<option>' + SSID[i] + '</option>';
			}
		}
	}
	optionStr += '</select>';	
	document.getElementById('ssid-select-content').innerHTML = optionStr;
	
	changeSecurityPolicyTableTitle(SSID[old_MBSSID]);
}
function setChange(c){
	changed = c;
}
var WPAAlgorithms;
function onWPAAlgorithmsClick(type)
{
	if(type == 0 && WPAAlgorithms == "tkip") return;
	if(type == 1 && WPAAlgorithms == "aes") return;
	if(type == 2 && WPAAlgorithms == "tkip+aes") return;
	setChange(1);
}

function disableWpsSet()
{
	document.security_form.wifienable[0].checked = true;
	document.security_form.wifienable[0].disabled = true;
	document.security_form.wifienable[1].disabled = true;
	document.security_form.wpsMode[0].disabled = true;
	document.security_form.wpsMode[0].disabled = true;
	document.security_form.rstOOB.disabled = true;
}

function enableWpsSet()
{
	document.security_form.wifienable[0].disabled = false;
	document.security_form.wifienable[1].disabled = false;
	document.security_form.wpsMode[0].disabled = false;
	document.security_form.wpsMode[0].disabled = false;
	if(mode == "disabled"){
		document.security_form.wifienable[0].checked = true;
		document.security_form.rstOOB.disabled = true;
	}else if (mode == "enabled"){
		document.security_form.rstOOB.disabled = false;
		document.security_form.wifienable[0].checked = true;
	}
}
</script>
<style type="text/css">
<!--
.style2 {
	color: #FF9900
}
-->
</style>
<link rel=stylesheet type=text/css href=style.css>
</HEAD>
<BODY leftMargin=0 topMargin=0 MARGINHEIGHT="0" MARGINWIDTH="0" onLoad="init();" class="bg">
	<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0">
      <tr>
        <td width="33">&nbsp;</td>
        <td width="679" valign="top">
		<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
          <tr>
            <td align="center" valign="top">
			<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0" height="100%">
                <tr>
                  <td align="center" valign="top">
				 <form method="post" name="security_form" action="/goform/wirelessSetSecurity">
				<!--<input type=hidden name="ssidIndex" value="0" >-->
				<input type="hidden" id="GO" name="GO" value="wireless_security.asp">
				<table cellpadding="0" cellspacing="0" class="content1" id="table4">
					<tbody>
					<tr>
					<td width="150" align="right" class="head"><script>document.write(_("Select SSID"));</script></td>
					<td id="ssid-select-content">&nbsp;&nbsp;&nbsp;&nbsp;</td>
					</tr>
					</tbody>
				</table>
				<table cellpadding="0" cellspacing="0" class="content3" id="table1">
				  <tbody>
				  <tr>
					<td colspan="2" align="left"><pre id="sp_title"><script>document.write(_("Security Policy"));</script></pre></td>
				  </tr>
				  <tr id="div_security_infra_mode" name="div_security_infra_mode"> 
					<td width="150" align="right" class="head"><script>document.write(_("Security Mode"));</script></td>
					<td>&nbsp;&nbsp;&nbsp;&nbsp;<select name="security_mode" id="security_mode" size="1" onChange="securityMode(1)">
						
					  </select>					</td>
				  </tr>
				  <tr id="div_security_shared_mode" name="div_security_shared_mode"> 
    <td width="100" align="right" class="head">Encryption type</td>
    <td>
       &nbsp;&nbsp;&nbsp;&nbsp;<select name="security_shared_mode" id="security_shared_mode" size="1" onChange="securityMode(1)">
		<option value="enable">WEP</option>
		<!--<option value=None>None</option> delete by stanley 2010/08/31-->
      </select>

    </td>
	</tr>
				</tbody></table>
				<br>
<!-- WEP -->
<!--<table id="div_wep" name="div_wep" width="90%" style="visibility:hidden; display:none">-->
<table cellpadding="0" cellspacing="0" class="content1" id="div_wep" style="margin-top:0px;" name="div_wep">
  <tbody>
  <tr> 
    <td colspan="1" align="right" class="head STYLE3"><script>document.write(_("Default Key"));</script></td>
	<td colspan="2">&nbsp;&nbsp;&nbsp;&nbsp;<select name="wep_default_key" id="wep_default_key" size="1" onChange="setChange(1)">
		<option value="1" selected="selected"><script>document.write(_("Key1"));</script></option>
		<option value="2"><script>document.write(_("Key2"));</script></option>
		<option value="3"><script>document.write(_("Key3"));</script></option>
		<option value="4"><script>document.write(_("Key4"));</script></option>
      </select></td>
  </tr>
  <tr> 
    <!--<td width="1%" rowspan="4" align="right" class="head1 STYLE3"></td>-->
    <td width="150" align="right" class="head2 STYLE3"><script>document.write(_("WEP key1"));</script></td>
    <td width="200">&nbsp;&nbsp;&nbsp;&nbsp;<input name="wep_key_1" id="WEP1" maxlength="26" value="" onKeyUp="setChange(1)"></td>
    <td ><select id="WEP1Select" name="WEP1Select" onChange="setChange(1)"> 
		<option value="1">ASCII</option>
		<option value="0">Hex</option>
		</select>
	</td>
  </tr>
  <tr> 
    <td align="right" class="head2 STYLE3"><script>document.write(_("WEP key2"));</script></td>
    <td>&nbsp;&nbsp;&nbsp;&nbsp;<input name="wep_key_2" id="WEP2" maxlength="26" value="" onKeyUp="setChange(1)"></td>
    <td><select id="WEP2Select" name="WEP2Select" onChange="setChange(1)">
		<option value="1">ASCII</option>
		<option value="0">Hex</option>
		</select></td>
  </tr>
  <tr> 
    <td align="right" class="head2 STYLE3"><script>document.write(_("WEP key3"));</script></td>
    <td>&nbsp;&nbsp;&nbsp;&nbsp;<input name="wep_key_3" id="WEP3" maxlength="26" value="" onKeyUp="setChange(1)"></td>
    <td><select id="WEP3Select" name="WEP3Select" onChange="setChange(1)">
		<option value="1">ASCII</option>
		<option value="0">Hex</option>
		</select></td>
  </tr>
  <tr> 
    <td align="right" class="head2 STYLE3"><script>document.write(_("WEP key4"));</script></td>
    <td>&nbsp;&nbsp;&nbsp;&nbsp;<input name="wep_key_4" id="WEP4" maxlength="26" value="" onKeyUp="setChange(1)"></td>
    <td><select id="WEP4Select" name="WEP4Select" onChange="setChange(1)">
		<option value="1">ASCII</option>
		<option value="0">Hex</option>
		</select></td>
  </tr>
</tbody></table>
<!-- <br /> -->
<!-- WPA -->
				<table cellpadding="0" cellspacing="0" class="content1" id="div_wpa" style="margin-top:0px;" name="div_wpa">
				  <tbody>
				  <tr id="div_wpa_algorithms" name="div_wpa_algorithms" style="visibility: visible;"> 
					<td width="150" align="right" class="head"><script>document.write(_("WPA Algorithms"));</script></td>
					<td>
					  &nbsp;&nbsp;&nbsp;&nbsp;<input name="cipher" id="cipher" value="aes" type="radio" onClick="onWPAAlgorithmsClick(1)"  checked="checked">AES &nbsp;
					  <input name="cipher" id="cipher" value="tkip" type="radio" onClick="onWPAAlgorithmsClick(0)">TKIP &nbsp;
					  <input name="cipher" id="cipher" value="tkip+aes" type="radio" onClick="onWPAAlgorithmsClick(2)">TKIP&AES &nbsp;
					</td>
				  </tr>
				  <tr id="wpa_passphrase" name="wpa_passphrase" style="visibility: visible;">
					<td width="150" align="right" class="head"><script>document.write(_("Key"));</script></td>
					<td>
					  &nbsp;&nbsp;&nbsp;&nbsp;<input name="passphrase" id="passphrase" size="28" maxlength="64" value="" onKeyUp="setChange(1)">
					</td>
				  </tr>
				  <tr id="wpa_key_renewal_interval" name="wpa_key_renewal_interval" style="visibility: visible;">
					<td width="150" align="right" class="head"><script>document.write(_("Key Renewal Interval"));</script></td>
					<td>
					  &nbsp;&nbsp;&nbsp;&nbsp;<input name="keyRenewalInterval" id="keyRenewalInterval" size="4" maxlength="4" value="" onKeyUp="setChange(1)"> <script>document.write(_("Second"));</script>
					</td>
				  </tr>
				</tbody></table>
				<hr style="width:80%">
				<table cellpadding="0" cellspacing="0" class="content1" id="table2" style="margin-top:0px;">
				<input type=hidden name=wifiEn>
				<input type=hidden name=wpsmethod>
				<tbody>
					<tr>
					  <td width="150" align="right" id="wpsWPS_text"><script>document.write(_("WPS Settings"));</script></td>
					  <td align="left">
						  &nbsp;&nbsp;&nbsp;&nbsp;<input type="radio" name="wifienable" value="disabled" checked="checked" onClick="onwifiuse(0)" ><script>document.write(_("Disable"));</script>
						  <input type="radio" name="wifienable" value="enabled" onClick="onwifiuse(1)" >
						<script>document.write(_("Enable"));</script></td>
					  <td width="13">&nbsp;</td>
					</tr>
				</tbody>
				</table>
				<table cellpadding="0" cellspacing="0" class="content1" id="table3" style="margin-top:0px;">
				   <tr id="wifiuse_wps" name="wifiuse_wps">
						<td width="150" align="right"><script>document.write(_("WPS Mode"));</script></td>
						<td width="25%" nowrap="nowrap">
						&nbsp;&nbsp;&nbsp;
						<input type="radio" name="wpsMode" value="pbc" onClick="onSel(2)">PBC
					    <input type="radio" name="wpsMode" value="pin" onClick="onSel(1)">PIN					    </td>
						<td width="25%"><input name="PIN" id="PIN" size="8" maxlength="8" class="text" type="text"></td>	
				        <td>&nbsp;</td>
				   </tr>
				</table>
				<table class="content1" style="margin-top:0px;">
					<tr>
					  <td align="right"><INPUT class=button2 type=button id="rstOOB" name="rstOOB" value='' onMouseOver="style.color='#FF9933'" onMouseOut="style.color='#000000'" onClick="resetOOB();"></td>
					</tr>
				</table>
				<table cellpadding="0" cellspacing="0" class="content1" style="margin-top:0px;">
				<tr><td>
				<span class="style2"><b><script>document.write(_("Note:"));</script></b>
				<script>document.write(_("Wireless Security Settings"));</script><br>
				<script>document.write(_("Wireless_secrity_note1"));</script>
				<br><script>document.write(_("Wireless_secrity_note2"));</script>
				</span><br>
				</td></tr>
				</table>
                <script>tbl_tail_save1("document.security_form");</script>
				</form>
				  </td>
                </tr>
            </table></td>
          </tr>
        </table></td>
        <td align="center" valign="top" height="100%">
		<script>helpInfo("WPA/WPA2:"+_("Wireless_secrity_helpinfo"));</script>		
		</td>
      </tr>
    </table>
	<script type="text/javascript">
	  table_onload1('table1');
	  table_onload('div_wpa');
	  table_onload('table2');
	  table_onload1('table3');
	  table_onload('table4');
	  table_onload('div_wep');
    </script>
</BODY>
</HTML>



