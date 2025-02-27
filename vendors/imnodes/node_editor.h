#pragma once
#include "Node.h"

namespace storyteller
{
void NodeEditorInitialize();
void NodeEditorShow();
void NodeEditorShutdown();
void InitializeConversation();
std::vector<Node*> GetNodesVec();
int GetNumNodesOfType(NodeType type);
} // namespace example