RegisterListener2(this, WH_MSG_GETSTARTFRAME);
RegisterListener2(this, WH_MSG_GETDEFAULTTOPIC);
RegisterListener2(this, WH_MSG_MINIBARORDER);
RegisterListener2(this, WH_MSG_TOOLBARORDER);
RegisterListener2(this, WH_MSG_ISSEARCHSUPPORT);
RegisterListener2(this, WH_MSG_ISSYNCSSUPPORT);
RegisterListener2(this, WH_MSG_ISAVENUESUPPORT);
RegisterListener2(this, WH_MSG_GETPANETYPE);
RegisterListener2(this, WH_MSG_GETPANES);
RegisterListener2(this, WH_MSG_RELOADNS6);

if (gbNav6)
{
	var gnReload=0;
	setTimeout("delayReload();",5000);
}

function delayReload()
{
	if(!gnReload)
	{
		if(nViewFrameType&&nViewFrameType==1)
			document.location=document.location;
	}
}

var gsToolbarOrder = "toc|fts|glo|blankblock|banner";
var gsMinibarOrder = "blankblock|hide2|";

var gsTopic = "files/introduction_to_smesh.htm";

if (location.hash.length > 1)
	gsTopic = location.hash.substring(1);

function setToolbarOrder(sOrder)
{
	gsToolbarOrder = sOrder;
}

function setMinibarOrder(sOrder)
{
	gsMinibarOrder = sOrder;
}

function onSendMessageX(oMsg)
{
	var nMsgId = oMsg.nMessageId;
	if (nMsgId == WH_MSG_GETSTARTFRAME)
	{
		oMsg.oParam.oFrame = this;
		return false;
	}
	else if (nMsgId == WH_MSG_GETDEFAULTTOPIC)
	{
		if (this.cMRServer && cMRServer.m_strVersion)
		{
			if (cMRServer.m_strURLTopic);
			{
				oMsg.oParam.sTopic = cMRServer.m_strURLTopic;
				return false;
			}

		}
		oMsg.oParam.sTopic = gsTopic;
		return false;
	}
	else if (nMsgId == WH_MSG_TOOLBARORDER)
	{
		var oMsg1 = new whMessage(WH_MSG_GETPANES, this, 1, null);
		if (SendMessage(oMsg1))
		{
			if (oMsg1.oParam)
			{
				if (this.cMRServer && cMRServer.m_strVersion)
				{
					var aToolbarOrder=cMRServer.m_strAgentList.split(";");
					var i=0;
					for (i=0;i<aToolbarOrder.length;i++)
						aToolbarOrder[i]=transferANToPN2(aToolbarOrder[i]);
					aToolbarOrder[aToolbarOrder.length] = "blankblock";
					if (cMRServer.m_bShowSearchInput)
					{
						aToolbarOrder[aToolbarOrder.length] = "searchform";
					}
					aToolbarOrder[aToolbarOrder.length] = "banner";
					var aToolbarOrderNew = new Array();
					for (i=0;i<aToolbarOrder.length;i++)
					{
						if (isAPane(aToolbarOrder[i]))
						{
							if (oMsg1.oParam.aPanes && oMsg1.oParam.aPanes.length)
							{
								for (var j=0;j<oMsg1.oParam.aPanes.length;j++)
								{
									if (aToolbarOrder[i] == oMsg1.oParam.aPanes[j].sPaneName)
									{
										aToolbarOrderNew[aToolbarOrderNew.length] = aToolbarOrder[i];
										break;
									}
								}
							}
						}
						else
							aToolbarOrderNew[aToolbarOrderNew.length] = aToolbarOrder[i];
					}
					oMsg.oParam = aToolbarOrderNew.join("|");
					return false;
				}
			}
		}
		oMsg.oParam = gsToolbarOrder;
		return false;
	}
	else if (nMsgId == WH_MSG_MINIBARORDER)
	{
		oMsg.oParam = gsMinibarOrder;
		return false;
	}
	else if (nMsgId == WH_MSG_ISSYNCSSUPPORT)
	{
		if (this.cMRServer && cMRServer.m_strVersion)
		{
			if (cMRServer.m_bShowSync)
				oMsg.oParam=true;
			else
				oMsg.oParam=false;
			return false;
		}
		else
		{
			if(nViewFrameType)
			{
				oMsg.oParam= (nViewFrameType < 3);
				return false;
			}
		}
	}
	else if (nMsgId == WH_MSG_ISAVENUESUPPORT)
	{
		if (this.cMRServer && cMRServer.m_strVersion)
		{
			if (cMRServer.m_bShowBrowseSequences)
				oMsg.oParam=true;
			else
				oMsg.oParam=false;
		}
		else
		{
			oMsg.oParam=true;
		}
		return false;
	}
	else if (nMsgId == WH_MSG_ISSEARCHSUPPORT)
	{
		if(nViewFrameType)
		{
			oMsg.oParam= (nViewFrameType < 3);
			return false;
		}
	}
	else if (nMsgId == WH_MSG_GETPANETYPE)
	{
		if(nViewFrameType)
		{
			var oPaneInfo = new Object();
			oPaneInfo.nType = nViewFrameType;
			oPaneInfo.sPaneURL = strPane;
			oMsg.oParam = oPaneInfo;
			return false;
		}
	}
	else if (nMsgId == WH_MSG_GETPANES)
	{
		if (this.cMRServer && cMRServer.m_strVersion)
		{
			var oPanes = new Object();
			var aAgentNames = null;
			if (cMRServer.m_strDefAgent)
				oPanes.sDefault = transferAgentNameToPaneName(cMRServer.m_strDefAgent);

			aPanes = new Array();
			for(var i=0; i< cMRServer.m_cAgents.length; i++)
			{
				var nCur = aPanes.length;
				aPanes[nCur] = new Object();
				aPanes[nCur].sPaneName = transferAgentNameToPaneName(cMRServer.m_cAgents[i].m_strID);
				aPanes[nCur].sPaneURL = cMRServer.m_cAgents[i].m_strURL;
			}
			oPanes.aPanes = aPanes;
			oMsg.oParam = oPanes;
			return false;
		}
		else
		{
			oMsg.oParam = null;
			return false;
		}
	}
	else if(nMsgId==WH_MSG_RELOADNS6)
	{
		if(gbNav6)
			gnReload=1;
		return false;
	}
	return true;
}

function transferANToPN2(sAN)
{
	if (sAN =="toc")
		return "toc";
	else if	(sAN =="ndx")
		return "idx";
	else if	(sAN =="nls")
		return "fts";
	else if	(sAN =="Glossary")
		return "glo";
	else if	(sAN =="WebSearch")
		return "websearch";
	else if (sAN.indexOf("custom_")==0);
		return "custom" + sAN.substring(7);
	return sAN;
}

function transferAgentNameToPaneName(sAgentName)
{
	if (sAgentName =="toc")
		return "toc";
	else if	(sAgentName =="ndx")
		return "idx";
	else if	(sAgentName =="nls")
		return "fts";
	else if	(sAgentName =="gls")
		return "glo";
	return "";
}

function isAPane(sPaneName)
{
	if (sPaneName == "toc" || sPaneName == "idx" || sPaneName == "fts" || sPaneName == "glo")
		return true;
	else
		return false;
}