#include "component/controller/ActivationController.h"

#include "data/access/StorageAccess.h"
#include "settings/ApplicationSettings.h"

#include "utility/messaging/type/MessageActivateAll.h"
#include "utility/messaging/type/MessageActivateTokens.h"
#include "utility/messaging/type/MessageChangeFileView.h"
#include "utility/messaging/type/MessageFlushUpdates.h"
#include "utility/messaging/type/MessageRefresh.h"
#include "utility/messaging/type/MessageShowErrors.h"
#include "utility/messaging/type/MessageStatus.h"
#include "utility/messaging/type/MessageScrollToLine.h"

ActivationController::ActivationController(StorageAccess* storageAccess)
	: m_storageAccess(storageAccess)
{
}

ActivationController::~ActivationController()
{
}

void ActivationController::clear()
{
}

void ActivationController::handleMessage(MessageActivateEdge* message)
{
	if (message->isAggregation())
	{
		MessageActivateTokens m(message);
		m.tokenIds = message->aggregationIds;
		m.setKeepContent(false);
		m.isAggregation = true;
		m.dispatchImmediately();
	}
	else
	{
		MessageActivateTokens m(message);
		m.tokenIds.push_back(message->tokenId);
		m.isEdge = true;
		m.dispatchImmediately();
	}
}

void ActivationController::handleMessage(MessageActivateFile* message)
{
	Id fileId = m_storageAccess->getNodeIdForFileNode(message->filePath);

	if (fileId)
	{
		MessageActivateTokens messageActivateTokens(message);
		messageActivateTokens.tokenIds.push_back(fileId);
		messageActivateTokens.tokenNames.push_back(NameHierarchy(message->filePath.wstr(), NAME_DELIMITER_FILE));
		messageActivateTokens.searchMatches = m_storageAccess->getSearchMatchesForTokenIds({ fileId });
		messageActivateTokens.dispatchImmediately();
	}
	else
	{
		MessageChangeFileView msg(
			message->filePath,
			MessageChangeFileView::FILE_MAXIMIZED,
			MessageChangeFileView::VIEW_CURRENT,
			true,
			true
		);
		msg.dispatchImmediately();
	}

	if (message->line > 0)
	{
		MessageScrollToLine(message->filePath, message->line).dispatch();
	}
}

void ActivationController::handleMessage(MessageActivateNodes* message)
{
	MessageActivateTokens m(message);
	for (const MessageActivateNodes::ActiveNode& node : message->nodes)
	{
		Id nodeId = node.nodeId;
		NameHierarchy name = node.nameHierarchy;
		if (!nodeId)
		{
			nodeId = m_storageAccess->getNodeIdForNameHierarchy(name);
		}
		else if (!name.size())
		{
			name = m_storageAccess->getNameHierarchyForNodeId(nodeId);
		}

		if (nodeId > 0)
		{
			m.tokenIds.push_back(nodeId);
		}
		m.tokenNames.push_back(name);
	}
	m.searchMatches = m_storageAccess->getSearchMatchesForTokenIds(m.tokenIds);
	m.dispatchImmediately();
}

void ActivationController::handleMessage(MessageActivateTokenIds* message)
{
	MessageActivateTokens m(message);
	m.tokenIds = message->tokenIds;
	m.searchMatches = m_storageAccess->getSearchMatchesForTokenIds(message->tokenIds);
	m.tokenNames = m_storageAccess->getNameHierarchiesForNodeIds(m.tokenIds);
	m.dispatchImmediately();
}

void ActivationController::handleMessage(MessageActivateSourceLocations* message)
{
	MessageActivateNodes m;
	for (Id nodeId : m_storageAccess->getNodeIdsForLocationIds(message->locationIds))
	{
		m.addNode(nodeId);
	}
	m.dispatchImmediately();
}

void ActivationController::handleMessage(MessageResetZoom* message)
{
	ApplicationSettings* settings = ApplicationSettings::getInstance().get();
	int fontSizeStd = settings->getFontSizeStd();

	if (settings->getFontSize() != fontSizeStd)
	{
		settings->setFontSize(fontSizeStd);
		settings->save();

		MessageRefresh().refreshUiOnly().dispatch();
	}


	MessageStatus(L"Font size: " + std::to_wstring(fontSizeStd)).dispatch();
}

void ActivationController::handleMessage(MessageSearch* message)
{
	const std::vector<SearchMatch>& matches = message->getMatches();

	if (matches.size() && matches.back().searchType == SearchMatch::SEARCH_COMMAND)
	{
		switch (matches.back().getCommandType())
		{
			case SearchMatch::COMMAND_ALL:
			case SearchMatch::COMMAND_NODE_FILTER:
			{
				MessageActivateAll(message->acceptedNodeTypes).dispatchImmediately();
				return;
			}

			case SearchMatch::COMMAND_ERROR:
			{
				MessageShowErrors(m_storageAccess->getErrorCount()).dispatch();
				MessageFlushUpdates().dispatch();
				return;
			}
		}
	}

	MessageActivateTokens m(message);
	m.tokenIds = message->getTokenIdsOfMatches();
	m.searchMatches = matches;
	m.tokenNames = m_storageAccess->getNameHierarchiesForNodeIds(m.tokenIds);
	m.isFromSearch = message->isFromSearch;
	m.dispatchImmediately();
}

void ActivationController::handleMessage(MessageShowErrorsForFile* message)
{
	MessageShowErrors(m_storageAccess->getErrorsForFileLimited(message->filePath)).dispatch();
}

void ActivationController::handleMessage(MessageZoom* message)
{
	bool zoomIn = message->zoomIn;

	ApplicationSettings* settings = ApplicationSettings::getInstance().get();

	int fontSize = settings->getFontSize();
	int maxSize = settings->getFontSizeMax();
	int minSize = settings->getFontSizeMin();

	if ((fontSize >= maxSize && zoomIn)
		|| (fontSize <= minSize && !zoomIn))
	{
		return;
	}

	settings->setFontSize(fontSize + (message->zoomIn ? 1 : -1));
	settings->save();

	MessageStatus(L"Font size: " + std::to_wstring(settings->getFontSize())).dispatch();
	MessageRefresh().refreshUiOnly().dispatch();
}

