/*!
	@version:	0.1.1
	@date:		2012-06-06
	@coder:		jaffrykee@hotmail.com
	@note:		For code view.
	@ads:		贤狼浏览器，棒棒棒！
*/
var colorArr = new Array("#F3F3F3","#DDDDDD");
var colorLeftFrame = "#DDEEFF";
var s = 0;

function defInit()
{
	ccInit();
	codeListInit();
	getTitleForCatalogue();
}

function ccInit()
{
	var ccList = document.getElementsByClassName("cc1");
	for(var i=0; i<ccList.length; i++)
	{
		ccList[i].innerHTML = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"+ccList[i].innerHTML
	}
	return;
}

function getTitleForCatalogue()
{
	var leftFrame = document.getElementsByClassName("leftfr");
	var hd1List = document.getElementsByClassName("hc1fr");
	for(var i=0; i<hd1List.length; i++)
	{
		var hc1List = hd1List[i].getElementsByClassName("hc1");
		var hh1 = document.createElement("p");
		hh1.innerHTML = "<a " + 'href="#hc1_' + (i+1) + '">' + hc1List[0].innerHTML + "<\/a>";
		hh1.class = "hh1";
		hh1.id = "hh1_" + (i+1);
		//hh1.style="font-size:12px; background-color:" + colorLeftFrame + ";";
		hc1List[0].innerHTML = '<a id="hc1_' + (i+1) + '"><\/a>' + hc1List[0].innerHTML;
		leftFrame[0].appendChild(hh1);
		
		var hd2List = hd1List[i].getElementsByClassName("hc2fr");
		for(var j=0; j<hd2List.length; j++)
		{
			var hc2List = hd2List[j].getElementsByClassName("hc2");
			var hh2 = document.createElement("p");
			hh2.innerHTML = "&nbsp;&nbsp;" + "<a " + 'href="#hc2_' + (i+1) + '_' + (j+1) + '">' + hc2List[0].innerHTML + "<\/a>";
			hh2.class = "hh2";
			hh2.id = "hh2_" + (j+1);
			//hh2.style="font-size:12px; background-color:" + colorLeftFrame + ";";
			hc2List[0].innerHTML = '<a id="hc2_' + (i+1) + '_' + (j+1) + '"><\/a>' + hc2List[0].innerHTML;
			leftFrame[0].appendChild(hh2);
			
			var hd3List = hd2List[j].getElementsByClassName("hc3fr");
			for(var k=0; k<hd3List.length; k++)
			{
				var hc3List = hd3List[k].getElementsByClassName("hc3");
				var hh3 = document.createElement("p");
				hh3.innerHTML = "&nbsp;&nbsp;&nbsp;&nbsp;" + "<a " + 'href="#hc3_' + (i+1) + '_' + (j+1) + '_' + (k+1) + '">' + hc3List[0].innerHTML + "<\/a>";
				hh3.class = "hh3";
				hh3.id = "hh3_" + (k+1);
				//hh3.style="font-size:12px; background-color:" + colorLeftFrame + ";";
				hc3List[0].innerHTML = '<a id="hc3_' + (i+1) + '_' + (j+1) + '_' + (k+1) + '"><\/a>' + hc3List[0].innerHTML;
				leftFrame[0].appendChild(hh3);
				
				var hd4List = hd3List[k].getElementsByClassName("hc4fr");
				for(var m=0; m<hd4List.length; m++)
				{
					var hc4List = hd4List[m].getElementsByClassName("hc4");
					var hh4 = document.createElement("p");
					hh4.innerHTML = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;" + "<a " + 'href="#hc4_' + (i+1) + '_' + (j+1) + '_' + (k+1) + '_' + (m+1) + '">' + hc4List[0].innerHTML + "<\/a>";
					hh4.class = "hh4";
					hh4.id = "hh4_" + (m+1);
					//hh4.style="font-size:12px; background-color:" + colorLeftFrame + ";";
					hc4List[0].innerHTML = '<a id="hc4_' + (i+1) + '_' + (j+1) + '_' + (k+1) + '_' + (m+1) + '"><\/a>' + hc4List[0].innerHTML;
					leftFrame[0].appendChild(hh4);
				}
			}
		}
	}
}

function codeListInit()
{
	var j=0;
	
	var cnFr = document.getElementsByClassName("codeNumFr");
	for(var i=0; i<cnFr.length; i++)
	{
		var codeFr = cnFr[i].getElementsByClassName("code");
		for(j=0,s=0; j<codeFr.length; j++)
		{
			codeFr[j].style.backgroundColor = colorArr[s++];
			codeFr[j].innerHTML = replaceSpecialChar(codeFr[j].innerHTML);
			if(s == colorArr.length)
			{
				s = 0;
			}
		}
		cnFr[i].innerHTML = '<div class="numfr"></div>'+cnFr[i].innerHTML;
		var frNum = cnFr[i].getElementsByClassName("numfr");
		if(cnFr[i].id.charAt(0) == 0)
		{
			for(var k=0; k<j; k++)
			{
				frNum[0].innerHTML = frNum[0].innerHTML + '<p class="num">' + (k+1) + '&nbsp;</p>';
			}
		}
		else
		{
			for(var k=0; k<j; k++)
			{
				//太慢了
//				frNum[0].innerHTML = frNum[0].innerHTML + '<p class="num">' + '<a id="_ac__' + cnFr[i].id + '_' + (k+1) + '" ></a>' + (k+1) + '&nbsp;</p>';
				frNum[0].innerHTML = frNum[0].innerHTML + '<p class="num">' + (k+1) + '&nbsp;</p>';
			}
		}
	}
	return;
}

//放弃使用code标签，无法达到预期效果，不好控制，转用js强制改动
function replaceSpecialChar(str)
{
	str = str.replace(/ /g, "&nbsp;");
	if(str.charAt(0) == "\t")
	{
		str = str.replace(/\t/g, "&nbsp;&nbsp;&nbsp;&nbsp;");
	}
	else
	{
		if(str.charAt(0) == 0)
		{
			str = "&nbsp;";
		}
	}
	
	return str;
}