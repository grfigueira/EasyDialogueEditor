/******************************************************************************
 * Copyright (c) 2025 Guilherme Figueira
 * MIT License - See LICENSE file in the project root for details
 * Contact: g.figueira.2002@gmail.com
 ******************************************************************************/

#pragma once
#include "Node.h"

namespace ede
{
	void NodeEditorInitialize();
	void NodeEditorShow();
	void NodeEditorShutdown();
	void InitializeConversation();
	std::vector<Node*> GetNodesVec();
	int GetNumNodesOfType(NodeType type);
	void ToggleDemoWindow();
	void ToggleAboutWindow();
	std::vector<Node> GetNodesData();
} // namespace storyteller