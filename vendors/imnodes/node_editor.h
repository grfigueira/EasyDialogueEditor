/******************************************************************************
 * Copyright (c) 2025 Guilherme Figueira
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Contact: g.figueira.2002@gmail.com
 ******************************************************************************/

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
void ToggleDemoWindow();
} // namespace storyteller