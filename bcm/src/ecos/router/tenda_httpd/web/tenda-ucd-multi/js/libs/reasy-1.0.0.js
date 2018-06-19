!/*UCD - 0.2.0
 *Easy Web Team of Tenda
 *2014-06-12 04:39:03 */
!function(){"use strict";function a(a){return b.test(String(a))}var b=/^[^{]+\{\s*\[native code/;Function.prototype.method=function(b,c){return this.prototype[b]||a(this[b])?void 0:(this.prototype[b]=c,this)},Object.method("create",function(a){function b(){}return b.prototype=a,new b}),String.method("trim",function(){for(var a=this.replace(/^\s+/,""),b=a.length-1,c=/\s/;c.test(a.charAt(b));)b--;return a.slice(0,b+1)})}(),function(a){"use strict";function b(a){var b=a.length,c=s.type(a);return s.isWindow(a)?!1:1===a.nodeType&&b?!0:"array"===c||"function"!==c&&(0===b||"number"==typeof b&&b>0&&b-1 in a)}function c(a,b){do a=a[b];while(a&&1!==a.nodeType);return a}function d(a){return s.isWindow(a)?a:9===a.nodeType?a.defaultView||a.parentWindow:!1}function e(a,b,c){if(b=b||0,s.isFunction(b))return s.grep(a,function(a,d){var e=!!b.call(a,d,a);return e===c});if(b.nodeType)return s.grep(a,function(a){return a===b===c});if("string"==typeof b){var d=s.grep(a,function(a){return 1===a.nodeType});if(B.test(b))return s.filter(b,d,!c);b=s.filter(b,d)}return s.grep(a,function(a){return s.inArray(a,b)>=0===c})}function f(){var a=null;try{a=new XMLHttpRequest,a.overrideMimeType&&a.overrideMimeType("text/xml")}catch(b){try{a=new ActiveXObject("MSXML2.XMLHTTP")}catch(c){try{a=new ActiveXObject("Microsoft.XMLHTTP")}catch(d){a=!1}}}if(!a)throw new Error("No XHR object available.");return a}var g,h,i="1.0.0",j=(a.location,a.document),k=a.REasy,l=a.$,m=/^<(\w+)\s*\/?>(?:<\/\1>|)$/,n=/^#([\w-]*)$/,o=(Array.prototype.concat,Array.prototype.push,Array.prototype.slice),p=Array.prototype.indexOf,q=Object.prototype.toString,r=(Object.prototype.hasOwnProperty,String.prototype.trim),s=function(a,b){return s.inst(a,b)},t=function(a){return a.Parent===s},u=/^[\],:{}\s]*$/,v=/(?:^|:|,)(?:\s*\[)+/g,w=/\\(?:["\\\/bfnrt]|u[\da-fA-F]{4})/g,x=/"[^"\\\r\n]*"|true|false|null|-?(?:\d+\.|)\d+(?:[eE][+-]?\d+|)/g,y={};s.extend=function(){var a,b,c,d=arguments[0]||{},e=1,f=arguments.length;for("object"==typeof d||s.isFunction(d)||(d={}),f===e&&(d=this,--e),e;f>e;e++)if(null!=(c=arguments[e]))for(b in c)a=c[b],d!==a&&void 0!==a&&(d[b]=a);return d},h={init:function(){},prototype:{init:function(){}},create:function(){var a=Object.create(this);return a.Parent=this,a.init.apply(a,arguments),a},inst:function(){var a=Object.create(this.prototype);return a.Parent=this,a.init.apply(a,arguments),a},proxy:function(a){var b=this;return function(){return a.apply(b,arguments)}},include:function(a){var b,c=a.included||a.setup;for(b in a)this.fn[b]=a[b];c&&c(this)}},h.fn=h.prototype,s.extend(h),s.include({REasy:i,init:function(a,b){var c;if(!a)return this;if(a.nodeType)return this.context=this[0]=a,this.length=1,this;if("string"==typeof a){if(n.test(a))return c=j.getElementById(RegExp.$1),c&&c.parentNode&&(c.id!==RegExp.$1&&(c=s.find(a,j)[0]),this.length=1,this[0]=c),this.context=j,this.selector=a,this;!b||t(b)?(this.context=b||g,this.selector=(this.context.selector?this.context.selector+" ":"")+a,c=this.context.find(a).toArray()):(this.context=b,this.selector=a,c=$(this.context).find(a).toArray())}else if(s.isFunction(a))return s.ready(a);return t(a)&&(this.selector=a.selector,this.context=a.context,c=a),s.likeArray(a)?$.isElement(a[0])&&(c=a):c=[a],s.merge(this,c)},selector:"",length:0,modules:["core"],toArray:function(){return o.call(this)},pushStack:function(a){var b=s.merge(this.Parent(),a);return b.prevObject=this,b.context=this.context,b},slice:function(){return this.pushStack(o.apply(this,arguments))},first:function(){return this.eq(0)},last:function(){return this.eq(-1)},get:function(a){return this[a]},eq:function(a){var b=this.length,c=+a+(0>a?b:0);return this.pushStack(c>=0&&b>c?[this[c]]:[])},map:function(a){return s.map(this,function(b,c){a.call(b,c)})},each:function(a){return this.map(a),this},end:function(){return this.prevObject||this.constructor(null)}}),s.extend({modules:["base"],noConflict:function(b){return a.$===s&&(a.$=l),b&&a.REasy===s&&(a.REasy=k),s},isReady:!1,ready:function(){function b(a){if(!s.isReady&&("readystatechange"!==a.type||"complete"===j.readyState))try{for(d=0,c=e.length;c>d;d++)e[d].call(j)}finally{s.isReady=!0,e=null}}var c,d,e=[];return j.addEventListener?(j.addEventListener("DOMContentLoaded",b,!1),a.addEventListener("load",b,!1)):(j.attachEvent("onreadystatechange",b),a.attachEvent("onload",b)),function(a){s.isReady?a.call(j):e.push(a)}}(),isFunction:function(a){return"function"===s.type(a)},isArray:Array.isArray||function(a){return"array"===s.type(a)},likeArray:function(a){var b=s.type(a);return"number"!=typeof a.length&&"string"!==b&&"function"!==b&&"regexp"!==b||!s.isWindow(a)},isWindow:function(a){return null!=a&&a==a.window},isNumeric:function(a){return!isNaN(parseFloat(a))&&isFinite(a)},type:function(a){return null==a?String(a):y[q.call(a)]||"object"},error:function(a){throw new Error(a)},parseHTML:function(a,b,c){if(!a||"string"!=typeof a)return null;"boolean"==typeof b&&(c=b,b=!1),b=b||j;var d=m.exec(a),e=!c&&[];return d?[b.createElement(d[1])]:(d=s.buildFragment([a],b,e),e&&s(e).remove(),s.merge([],d.childNodes))},parseJSON:function(b){return a.JSON&&a.JSON.parse?a.JSON.parse(b):null===b?b:"string"==typeof b&&(b=s.trim(b),b&&u.test(b.replace(w,"@").replace(x,"]").replace(v,"")))?new Function("return "+b)():void s.error("Invalid JSON: "+b)},parseXML:function(b){var c,d;if(!b||"string"!=typeof b)return null;try{a.DOMParser?(d=new DOMParser,c=d.parseFromString(b,"text/xml")):(c=new ActiveXObject("Microsoft.XMLDOM"),c.async="false",c.loadXML(b))}catch(e){c=void 0}return c&&c.documentElement&&!c.getElementsByTagName("parsererror").length||s.error("Invalid XML: "+b),c},isElement:function(a){var b;return a?(b=q.call(a),-1!==b.indexOf("HTML")||"[object Object]"===b&&1===a.nodeType&&!(a instanceof Object)):!1},nodeName:function(a,b){return a.nodeName&&a.nodeName.toLowerCase()===b.toLowerCase()},each:function(a,c,d){var e,f,g,h=a.length,i=b(a);if(d){if(i)for(g=0;h>g&&(f=c.apply(a[g],d),f!==!1);g++);else for(e in a)if(f=c.apply(a[e],d),f===!1)break}else if(i)for(g=0;h>g&&(f=c.call(a[g],g,a[g]),f!==!1);g++);else for(e in a)if(f=c.call(a[e],e,a[e]),f===!1)break;return a},trim:function(a){return null==a?"":r.call(a)},inArray:function(a,b,c){var d;if(b){if(p)return p.call(b,a,c);for(d=b.length,c=c?0>c?Math.max(0,d+c):c:0;d>c;c++)if(c in b&&b[c]===a)return c}return-1},merge:function(a,b){var c=b.length,d=a.length,e=0;if("number"==typeof c)for(e;c>e;e++)a[d++]=b[e];else for(;void 0!==b[e];)a[d++]=b[e++];return a.length=d,a},grep:function(a,b,c){var d,e=[],f=a.length,g=0;for(c=!!c,g=0;f>g;g++)d=!!b(a[g],g),c!==d&&e.push(a[g]);return e},map:function(a,b){var c,d,e,f=[],g=a.length;if(s.likeArray(a))for(d=0;g>d;d++)c=b(a[d],d),null!=c&&f.push(c);else for(e in a)c=b(a[e],e),null!=c&&f.push(c);return f.concat.apply([],f)}}),s.each("Boolean Number String Function Array Date RegExp Object".split(" "),function(a,b){y["[object "+b+"]"]=b.toLowerCase()}),s.include({included:function(a){a.modules.push("event")},on:function(a,b){return this.each(function(){var c=this;return c?void(j.attachEvent?c.attachEvent("on"+a,function(){s.isFunction(b)&&b.apply(c,arguments)}):j.addEventListener?c.addEventListener(a,function(){s.isFunction(b)&&b.apply(c,arguments)},!1):s.isFunction(b)&&(c["on"+a]=b)):!1})},off:function(a,b){return this.each(function(){var c=this;return c?void(j.attachEvent?c.detachEvent("on"+a,b):j.addEventListener?c.removeEventListener(a,b,!1):c["on"+a]=null):!1})}});var z,A,B=/^.[^:#\[\.,]*$/,C=/^[^{]+\{\s*\[native code/,D=(j.documentElement,{letterSpacing:0,fontWeight:400}),E=null;g=s(j),s.extend({cssHooks:{opacity:{get:function(a,b){if(b){var c=A(a,"opacity");return""===c?"1":c}}}},cssNumber:{columnCount:!0,fillOpacity:!0,fontWeight:!0,lineHeight:!0,opacity:!0,orphans:!0,widows:!0,zIndex:!0,zoom:!0},cssProps:{},style:function(a,b,c,d){if(a&&3!==a.nodeType&&8!==a.nodeType&&a.style){var e,f,g,h=a.style;if(b=s.cssProps[b]||b,g=s.cssHooks[b],void 0===c)return g&&"get"in g&&void 0!==(e=g.get(a,!1,d))?e:h[b];if(f=typeof c,!(g&&"set"in g&&void 0===(c=g.set(a,c,d))))try{h[b]=c}catch(i){}}},css:function(a,b,c,d){var e,f,g;return b=s.cssProps[b]||b,g=s.cssHooks[b],g&&"get"in g&&(f=g.get(a,!0,c)),void 0===f&&(f=A(a,b,d)),"normal"===f&&b in D&&(f=D[b]),""===c||c?(e=parseFloat(f),c===!0||s.isNumeric(e)?e||0:f):f}}),a.getComputedStyle?(z=function(b){return a.getComputedStyle(b,null)},A=function(a,b,c){{var d=c||z(a),e=d?d.getPropertyValue(b)||d[b]:void 0;a.style}return d&&(""!==e||s.contains(a.ownerDocument,a)||(e=s.style(a,b))),e}):j.documentElement.currentStyle&&(z=function(a){return a.currentStyle},A=function(a,b,c){var d=c||z(a),e=d?d[b]:void 0,f=a.style;return null==e&&f&&f[b]&&(e=f[b]),""===e?"auto":e}),E=function(a,b){function c(a){return C.test(String(a))}function d(a,c){var d,e=[],f=(c||b).getElementsByTagName("*"),g=f.length,h=0;for(h;g>h;h++)d=" "+f[h].className+" ",-1!==d.indexOf(" "+a+" ")&&e.push(f[h]);return e}function e(a,c){c=c||b;var e={TAG:/^((?:\\.|[\w*-]|[^\x00-\xa0])+)/,CLASS:/^\.((?:\\.|[\w-]|[^\x00-\xa0])+)/,ATTR:/^\[[\x20\t\r\n\f]*((?:\\.|[\w-]|[^\x00-\xa0])+)[\x20\t\r\n\f]*(?:([*^$|~]?=)[\x20\t\r\n\f]*(?:(['"])((?:\\.|[^\\])*?)\3|((?:\\.|[\w#-]|[^\x00-\xa0])+)|)|)[\x20\t\r\n\f]*\]/,ID:/^#((?:\\.|[\w-]|[^\x00-\xa0])+)/},f=/^[\x20\t\r\n\f]*([\x20\t\r\n\f>+~])[\x20\t\r\n\f]*/,g=/^[\x20\t\r\n\f]+|((?:^|[^\\])(?:\\.)*)[\x20\t\r\n\f]+$/g,i={">":{dir:"parentNode",first:!0}," ":{dir:"parentNode"},"+":{dir:"previousSibling",first:!0},"~":{dir:"previousSibling"}},j={tabindex:"tabIndex",readonly:"readOnly","for":"htmlFor","class":"className",maxlength:"maxLength",cellspacing:"cellSpacing",cellpadding:"cellPadding",rowspan:"rowSpan",colspan:"colSpan",usemap:"useMap",frameborder:"frameBorder",contenteditable:"contentEditable"},k={ID:function(a){var b=[],d=c.getElementById(a);return null==d||d&&d.id!=a?!1:(b[0]=d,b)},TAG:function(a){var b=c.getElementsByTagName(a);return 0===b.length?!1:b},CLASS:function(a){var b=h.byClass?c.getElementsByClassName(a):d(a,c);return 0===b.length?!1:b},ATTR:function(a){var b=[],d=a[0],e=void 0===a[1]?"":a[1],f=a[2],g=c.getElementsByTagName("*"),h=g.length,i=0,k="";for(i;h>i;i++)switch(k=q[d]?q[d](g[i]):g[i].getAttribute(d)||g[i][j[d]],k="string"==typeof k?k.replace(/^\s+|\s+$/g,""):"",e){case"":""!=k&&b.push(g[i]);break;case"=":k==f&&b.push(g[i]);break;case"~=":if(k=" "+k+" ",-1!=f.indexOf(" "))break;var l=" "+f+" ";-1!=k.indexOf(l)&&b.push(g[i]);break;case"^=":k=k.slice(0,f.length),k==f&&b.push(g[i]);break;case"$=":k=k.slice(-f.length),k==f&&b.push(g[i]);break;case"*=":-1!=k.indexOf(f)&&b.push(g[i]);break;case"|=":k=k.split("-")[0],k==f&&b.push(g[i])}return b}},l={TAG:function(a,b){return 1==b.nodeType?b.getAttribute("tagName"):null},CLASS:function(a,b){return 1==b.nodeType?b.getAttribute("className"):null},ID:function(a,b){return 1==b.nodeType?b.getAttribute("id"):null},ATTR:function(a,b){var c;return 1==b.nodeType?(c=q[a[0]]?q[a[0]](b):b.getAttribute(a[0])||b[j[a[0]]],"string"==typeof c?c:""):null}},m=/\\([\da-fA-F]{1,6}[\x20\t\r\n\f]?|.)/g,n=function(a,b){var c="0x"+b-65536;return c!==c?b:0>c?String.fromCharCode(c+65536):String.fromCharCode(c>>10|55296,1023&c|56320)},o=function(a){for(var b,c="",d="",e=0,f="",g=[],h=[],j=!1,m={},n=[],o=[],q=[];m=a.pop();){if(c=m.type,d="ATTR"==c?m.matches:m.matches?m.matches[0]:m.value,m&&i[c]){for(f=i[c].dir,j=i[c].first,e=h.length,m=a.pop(),c=m.type,d="ATTR"==c?m.matches[2]:m.matches?m.matches[0]:m.value,q=[],t="",b=0;e>b;b++)if(q=[],"parentNode"==f&&j){var r=[];n[b]&&n[b][0]?q=n[b]:q[0]=h[b];for(var s=[];s[0]=q.pop();)t=l[c](m.matches,s[0][f]),t=t?t.toLowerCase().replace(/^\s+|\s+$/g,""):t,t&&("ATTR"==c&&p(t,m.matches)||t==d||0===(" "+t+" ").indexOf(d))&&(r.push(s[0][f]),g.length-1!=h[b]&&g.push(h[b]));0!==r.length&&o.push(r)}else if("parentNode"!=f||j)if("previousSibling"!=f||j){if("previousSibling"==f&&j){var r=[];n[b]&&n[b][0]?q=n[b]:q[0]=h[b];for(var s=[];s[0]=q.pop();)for(;s[0][f];){if(t=l[c](m.matches,s[0][f]),t=t?t.toLowerCase().replace(/^\s+|\s+$/g,""):t,3!=s[0][f].nodeType||/\S+/.test(s[0][f].nodeValue)){if(t&&("ATTR"==c&&p(t,m.matches)||t==d||0===(" "+t+" ").indexOf(d))){r.push(s[0][f]),g[g.length-1]!=h[b]&&g.push(h[b]);break}break}s[0]=s[0][f]}0!==r.length&&o.push(r)}}else{var r=[];n[b]&&n[b][0]?q=n[b]:q[0]=h[b];for(var s=[];s[0]=q.pop();)for(;s[0][f];)t=l[c](m.matches,s[0][f]),t=t?t.toLowerCase().replace(/^\s+|\s+$/g,""):t,t&&("ATTR"==c&&p(t,m.matches)||t==d||0===(" "+t+" ").indexOf(d))&&(r.push(s[0][f]),g[g.length-1]!=h[b]&&g.push(h[b])),s[0]=s[0][f];0!==r.length&&o.push(r)}else{var r=[];n[b]&&n[b][0]?q=n[b]:q[0]=h[b];for(var s=[];s[0]=q.pop();)for(;s[0][f];)t=l[c](m.matches,s[0][f]),t=t?t.toLowerCase().replace(/^\s+|\s+$/g,""):t,t&&("ATTR"==c&&p(t,m.matches)||t==d||0==(" "+t+" ").indexOf(d))&&(r.push(s[0][f]),g[g.length-1]!=h[b]&&g.push(h[b])),s[0]=s[0][f];0!=r.length&&o.push(r)}h=g,n=o,o=[],g=[]}else if(0===h.length)h=k[c](d);else{e=h.length;var t="";for(b=0;e>b;b++)t=l[c](m.matches,h[b]),t=t?t.toLowerCase().replace(/^\s+|\s+$/g,""):t,t&&("ATTR"==c&&p(t,m.matches)||t==d||0===(" "+t+" ").indexOf(d))&&g.push(h[b]);h=g,g=[]}if(0===h.length)break}return h},p=function(a,b){var c=b[2],d=b[1],e=-1;switch(a="string"==typeof a?a.replace(/^\s+|\s+$/g,""):"",d){case"":""!==a&&(e=0);break;case"=":a==c&&(e=0);break;case"~=":if(a=" "+a+" ",-1!=c.index(" "))break;var f=" "+c+" ";-1!=a.indexOf(f)&&(e=0);break;case"^=":a=a.slice(0,c.length),a==c&&(e=0);break;case"$=":a=a.slice(-c.length),a==c&&(e=0);break;case"*=":-1!=a.indexOf(c)&&(e=0);break;case"|=":a=a.split("-")[0],a==c&&(e=0)}return 0===e?!0:!1},q={};s.each(["href","src","width","height"],function(a,b){q[b]=function(a){var c=a.getAttribute(b,2);return null==c?void 0:c}}),q.style=function(a){var b=a.style.cssText.toLowerCase();return null==b?void 0:b};for(var r,t=a,u=[],v=[],w=!1;""!=t;){(u=f.exec(t))&&(w=u.shift(),v.push({value:w,type:u[0].replace(g," ")}),t=t.slice(w.length));for(r in e)if(e.hasOwnProperty(r)&&(u=e[r].exec(t),null!=u)){"ATTR"==r&&(u[1]=u[1].replace(m,n),u[3]=(u[4]||u[5]||"").replace(m,n),u=u.slice(0,4)),w=u.shift(),v.push({matches:u,type:r,value:w}),t=t.slice(w.length);break}if(!w)break}return o(v)}var f,g,h={};return f=b.documentElement,h.qs=c(b.querySelectorAll),h.qsa=c(b.querySelectorAll),h.byClass=c(b.getElementsByClassName),h.byClass||(b.getElementsByClassName=d),h.qsa||(b.querySelectorAll=e),h.qs||(b.querySelector=function(a){return e(a)[0]}),g=c(f.contains)||f.compareDocumentPosition?function(a,b){var c=9===a.nodeType?a.documentElement:a,d=b&&b.parentNode;return a===d||!(!d||1!==d.nodeType||!(c.contains?c.contains(d):a.compareDocumentPosition&&16&a.compareDocumentPosition(d)))}:function(a,b){if(b)for(;b=b.parentNode;)if(b===a)return!0;return!1},{selectAll:c(b.querySelectorAll)?function(a,c){return(c||b).querySelectorAll(a)}:function(a,c){return e(a,c||b)},contains:g}}(a,j),s.extend({find:function(a,b,c){var d,e,f=b.length;if(c=c||[],!a||"string"!=typeof a)return c;if(d=b.nodeType||(b[0]?b[0].nodeType:-1),1!==d&&9!==d)return c;if(f)for(e=0;f>e;e++)s.merge(c,E.selectAll(a,b[e]));else s.merge(c,E.selectAll(a,b));return c},contains:E.contains,getPosition:function(a){var b,c=a.offsetTop,d=a.offsetLeft;return null!=a.offsetParent&&(b=s.getPosition(a.offsetParent),c+=b.top+s.css(a,"borderTopWidth",!0),d+=b.left+s.css(a,"borderLeftWidth",!0)),{top:c,left:d}},getValue:function(a){return"undefined"!=typeof a.value?a.value:s.isFunction(a.val)?a.val():void 0},setValue:function(a,b){return"undefined"!=typeof b?!1:void("undefined"!=typeof a.value?a.value=b:s.isFunction(a.val)&&a.val(b))}}),s.include({included:function(a){a.modules.push("dom")},find:function(a){var b,c,d,e=this.length;return"string"!=typeof a?(d=this,this.pushStack(s(a).filter(function(){for(c=0;e>c;c++)if(s.contains(d[c],this))return!0}))):(b=[],s.find(a,this,b),b=this.pushStack(b),b.selector=(this.selector?this.selector+" ":"")+a,b)},not:function(a){return this.pushStack(e(this,a,!1))},filter:function(a){return this.pushStack(e(this,a,!0))},html:function(a){return"string"==typeof a?this.each(function(){this.innerHTML=a}):a?void 0:this[0].innerHTML},append:function(a){return $.isElement(a)?this.each(function(){this.appendChild(a)}):this},prepend:function(a){return $.isElement(a)?this.each(function(){(1===this.nodeType||11===this.nodeType||9===this.nodeType)&&this.insertBefore(elem,this.firstChild)}):this},before:function(a){return $.isElement(a)?this.each(function(){this.parentNode.insertBefore(a,this)}):this},after:function(a){return $.isElement(a)?this.each(function(){var b=this.parentNode;b.lastChild==this?b.appendChild(a):b.insertBefore(a,this.nextSibling)}):this},remove:function(){return this.each(function(){var a=this&&this.parentNode;$.isElement(a)&&a.removeChild(this)})},css:function(a,b){if("object"===s.type(a)&&void 0===b)for(var c in a)a.hasOwnProperty(c)&&this.css(c,a[c]);else this.each(function(){if(void 0!==b)try{this.style[a]=b}catch(c){}});return this},val:function(a){if("undefined"!=typeof a)return this.each(function(){"undefined"!==this.value?this.value=a:s.isFunction(this.val)&&this.val(a)});if("string"==typeof a){if("undefined"!==this[0].value)return this[0].value;if(s.isFunction(this[0].val))return this[0].val()}},offset:function(){if(arguments.length)return void 0===options?this:this.each(function(a){s.offset.setOffset(this,options,a)});var a,b,c={top:0,left:0},e=this[0],f=e&&e.ownerDocument;if(f)return a=f.documentElement,s.contains(a,e)?(void 0!==typeof e.getBoundingClientRect&&(c=e.getBoundingClientRect()),b=d(f),{top:c.top+(b.pageYOffset||a.scrollTop)-(a.clientTop||0),left:c.left+(b.pageXOffset||a.scrollLeft)-(a.clientLeft||0)}):c},show:function(){return this.each(function(){s(this).css("display","")})},hide:function(){return this.each(function(){s(this).css("display","none")})},hasClass:function(a){var b,c=this[0];return null===c?!1:(a=a||"",b=" "+c.className+" ",-1!==b.indexOf(" "+a.trim()+" "))},addClass:function(a){return"string"===s.type(a)?this.each(function(){$(this).hasClass(a)||(this.className+=""===this.className?a:" "+a)}):this},removeClass:function(a){return"string"===s.type(a)?this.each(function(){var b,c;return $(this).hasClass(a)?(c=" "+this.className+" ",b=c.replace(" "+a+" "," ").trim(),this.className=b,this):void 0}):void 0}}),s.extend({filter:function(a,b){return 1===b.length?s.find(a,b[0])?b[0]:[]:s.find(a,b)},dir:function(a,b,c){for(var d=[],e=a[b];e&&9!==e.nodeType&&(void 0===c||1!==e.nodeType||e!==c);)1===e.nodeType&&d.push(e),e=e[b];return d},sibling:function(a,b){for(var c=[];a;a=a.nextSibling)1===a.nodeType&&a!==b&&c.push(a);return c},isHidden:function(a){return"none"===s.css(a,"display")||"hidden"===s.css(a,"visibility")||0==a.offsetHeight&&0==a.offsetWidth}}),s.each({parent:function(a){var b=a.parentNode;return b&&11!==b.nodeType?b:null},parents:function(a){return s.dir(a,"parentNode")},next:function(a){return c(a,"nextSibling")},prev:function(a){return c(a,"previousSibling")},nextAll:function(a){return s.dir(a,"nextSibling")},prevAll:function(a){return s.dir(a,"previousSibling")},siblings:function(a){return s.sibling((a.parentNode||{}).firstChild,a)},children:function(a){return s.sibling(a.firstChild)},contents:function(a){return s.nodeName(a,"iframe")?a.contentDocument||a.contentWindow.document:s.merge([],a.childNodes)}},function(a,b){s.fn[a]=function(){var a=s.map(this,b);return this.pushStack(a)}});var F,G=/^(?:GET|HEAD)$/;try{F=a.location.href}catch(H){F=j.createElement("a"),F.href="",F=F.href}s.extend({encodeFormData:function(a){var b,c,d=[];if(!a)return"";for(b in a)a.hasOwnProperty(b)&&!s.isFunction(a[b])&&(c=a[b].toString(),b=encodeURIComponent(b.replace("%20","+")),c=encodeURIComponent(c.replace("%20","+")),d.push(b+"="+c));return d.join("&")},queryToObj:function(a){for(var b,c=a.split("&"),d=0,e=c.length;e>d;d++)b=c[d]},serializeByClass:function(a,b){var c=[],d={};return s("."+a).each(function(){var a=this.getAttribute("name");a&&(c.push(encodeURIComponent(a)+"="+encodeURIComponent($.getValue(this))),d[a]=$.getValue(this))}),b?d:c.join("&")},serialize:function(a,b){var c,d,e,f,g,h,i=[],j=null;for(c=0,d=a.elements.length;d>c;c++)switch(j=a.elements[c],j.type){case"select-one":case"select-multiple":if(j.name.length)for(e=0,f=j.options.length;f>e;e++)g=j.options[e],h=g.hasAttribute?g.hasAttribute("value")?g.value:g.text:g.attributes.value.specified?g.value:g.text,i.push(encodeURIComponent(j.name)+"="+encodeURIComponent(h));break;case void 0:case"file":case"submit":case"reset":case"button":break;case"radio":case"checkbox":if(!j.checked)break;default:j.name.length&&i.push(encodeURIComponent(j.name)+"="+encodeURIComponent(j.value))}return b?i.join("&")+"&"+s.serializeByClass(b):i.join("&")},ajaxSettings:{url:F,type:"GET",global:!0,processData:!0,async:!0,contentType:"application/x-www-form-urlencoded;"},ajax:function(a){var b=s.extend(s.ajaxSettings,a),c=f();return b.hasContent=!G.test(b.type),b.type=b.type.toUpperCase(),c.open(b.type,b.url,b.async),(b.data&&b.hasContent&&b.contentType!==!1||a.contentType)&&c.setRequestHeader("Content-Type",b.contentType),c.onreadystatechange=function(){4===c.readyState&&200===c.status&&b.success&&(b.successed||"function"!==s.type(b.success)||(b.success.call(c,c),b.successed=!0))},"object"===s.type(b.data)&&(b.data=$.encodeFormData(b.data)),c.send(b.data),b.successed=!1,c},getJSON:function(a,b){s.get(a,function(a){var c=a.responseText,d={};c=c.replace(/[\x00-\x1F\x7F]/g,"");try{d=$.parseJSON(c)}catch(e){c=c.replace(/[^"],"/,'","');try{d=$.parseJSON(c)}catch(e){console.log("Json parse failed! Check the input string.")}}finally{"function"==typeof b&&b.call(a,d)}})}}),s.each(["get","post"],function(a,b){s[b]=function(a,c,d,e){return s.isFunction(c)&&(e=e||d,d=c,c=void 0),s.ajax({url:a,type:b,dataType:e,data:c,success:d})}}),a.REasy=a.R=a.$=s}(window);