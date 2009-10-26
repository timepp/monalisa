var fso = new ActiveXObject("Scripting.FileSystemObject");
var shell = new ActiveXObject("WScript.Shell");
var wmi = null;
var ForReading = 1, ForWriting = 2;

function WMI()
{
	if (!wmi)
	{
		wmi = GetObject("winmgmts:\\\\.\\root\\cimv2");
	}
	return wmi;
}

function ResizeWindow(cx, cy, center)
{
	window.resizeTo(cx, cy);
	if (center)
	{
		var items = WMI().ExecQuery("Select * From Win32_DesktopMonitor");
		var item = new Enumerator(items).item();
		var w = item.ScreenWidth;
		var h = item.ScreenHeight;
		window.moveTo((w-cx)/2, (h-cy)/2);
	}
}

function RunCommand(cmd, a, b)
{
	shell.Run('"' + cmd + '"', a, b);
}
function GetErrDesc(e)
{
	if (e instanceof Error) return e.description;
	return e;
}
function GetCurTime()
{
	var ct = new Date();
	var D2 = function(n) { if (n < 10) return "0" + n.toString(); return n.toString(); };
	return D2(ct.getHours()) + ":" + D2(ct.getMinutes()) + ":" + D2(ct.getSeconds());
}
function SetText(obj, text, color)
{
	if (color == undefined) color = "black";
	obj.innerHTML = text.fontcolor(color);
}

// 取文本文件信息, \r\n会被替换为\n， 文件中不能有控制字符
function GetTextFileContent(filename)
{
	var content = "";
	try
	{
		var ofile = fso.OpenTextFile(filename, ForReading);
		content = ofile.ReadAll();
		ofile.Close();
	}
	catch(e)
	{
		alert(e.message);
	}
	return content;
}

function PlainTextToHTML(text)
{
	var html = "";
	for (var i = 0; i < text.length; i++)
	{
		var code = text.charCodeAt(i);
		switch (code)
		{
			case 38: html += "&amp;";   break;
			case 60: html += "&lt;";    break;
			case 62: html += "&gt;";    break;
			case 10: html += "<br />";  break;
			default: html += String.fromCharCode(code);
		}
	}
	return html;
}

// 取二进制文件信息，文件内容按原貌读取
function GetBinaryFileContent(filename, pos, len)
{
	var stream = new ActiveXObject("ADODB.Stream");
//	stream.Type = 1;
	stream.Open();
	stream.LoadFromFile(filename);
	stream.Position = pos;
	var str = "";
//	str = stream.Read(len);
	str = stream.ReadText(len);
	debugger;
	return DumpBin(str);
}

function DumpBin(str, npl){
	var codeMap = "0123456789ABCDEF";
	function hex(str, pos)
	{
		var n = str.charCodeAt(pos);
		return codeMap[n/16] + codeMap[n%16];
	}
	
	var outstr;
	for (var i = 0; i < str.length; i += npl)
	{
		for (var j = 0; j < npl; j++)
		{
			var pos = i*npl + j;
			if (pos >= str.length)
			{
				outstr += "   ";
			}
			else
			{
				outstr += hex(str, pos) + " ";
			}
		}
		
		outstr += " ";
		for (var j = 0; j < npl; j++)
		{
			var pos = i*npl + j;
			if (pos > str.length)
			{
				outstr += " ";
			}
			else
			{
				var n = str.charCodeAt(pos);
				outstr += (n >= 0x20 && n < 0x80)? String.fromCharCode(n) : ".";
			}
		}
		
		outstr += "\n";
	}
}

