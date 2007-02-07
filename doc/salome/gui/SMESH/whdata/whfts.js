//	WebHelp 5.10.001
var gaFileMapping = new Array();
var gaFileTopicMapping = new Array();

function fileMapping(sStartKey, sEndKey, sFileName)
{
	this.sStartKey = sStartKey;
	this.sEndKey = sEndKey;
	this.sFileName = sFileName;
	this.aFtsKeys = null;
}

function fileTopicMapping(nIdBegin, nIdEnd, sFileName)
{
	this.nBegin = nIdBegin;
	this.nEnd = nIdEnd;
	this.sFileName = sFileName;
	this.aTopics = null;
}


function iWM(sStartKey, sEndKey, sFileName)
{
	gaFileMapping[gaFileMapping.length] = new fileMapping(sStartKey, sEndKey, sFileName);	
}

function window_OnLoad()
{
	if (parent && parent != this && parent.ftsReady)
	{
		parent.ftsReady(gaFileMapping, gaFileTopicMapping);
	}		
}

function iTM(nIdBegin, nIdEnd, sFileName)
{
	gaFileTopicMapping[gaFileTopicMapping.length] = new fileTopicMapping(nIdBegin, nIdEnd, sFileName);	
}

window.onload = window_OnLoad;
