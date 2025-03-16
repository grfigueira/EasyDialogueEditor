/******************************************************************************
	Created by Guilherme Figueira, 2025

	My contacts (feel free to reach out):
	- Github: https://github.com/grfigueira
	- LinkedIn: https://www.linkedin.com/in/grfigueira/
 ******************************************************************************/

#pragma once
#include "Node.h"
#include <set>


namespace ede
{
	void NodeEditorInitialize();
	void NodeEditorShow();
	void NodeEditorShutdown();
	void InitializeConversation();
	std::vector<std::shared_ptr<Node>> GetNodesVec();
	std::set<std::string>& GetCallbacksMutable();
	const State& GetCurrentState();
	int GetNumNodesOfType(NodeType type);
	void ToggleDemoWindow();
	void ToggleAboutWindow();
	void ToggleHowToWindow();
	std::vector<Node> GetNodesData();
	void NotifyCallbackDeletion(const std::string& deleted_callback);
	void SetState(const State& new_state);
	void RequestNotification(const char* title, const char* description);
} // namespace storyteller