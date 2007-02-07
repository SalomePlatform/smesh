//	WebHelp 5.10.001
var gTEA = new Array();
function aTE(sTopicTitle, sTopicURL)
{
	var len = gTEA.length;
	gTEA[len] = new topicEntry(sTopicTitle, sTopicURL);
}

function topicEntry(sTopicTitle, sTopicURL)
{
	this.sTopicTitle = sTopicTitle;
	this.sTopicURL = sTopicURL;
}

function window_OnLoad()
{
	if (parent && parent != this) {
		if (parent.putFtsTData) 
		{
			parent.putFtsTData(gTEA);
		}
	}
}

window.onload = window_OnLoad;