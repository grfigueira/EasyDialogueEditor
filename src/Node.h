/******************************************************************************
 * Copyright (c) 2025 Guilherme Figueira
 * MIT License - See LICENSE file in the project root for details
 * Contact: g.figueira.2002@gmail.com
 ******************************************************************************/

#pragma once
#include <string>
#include <vector>
#include "imnodes.h"
#include <imgui.h>
#include <set>
#include <nlohmann/json.hpp>

/******************************************************************************
 *                   Every node-related data structure
 ******************************************************************************/

// used for converting node_ids to parameters using bit-shifting
enum NodePartShift
{
    InputPin = 8,
    EndPin = 24,
};

enum NodeType
{
    Speech,
    Response,
    NodeTypeCount // used to convert NodeType to string. do not remove, and keep it in last
};

struct SpeechNode;
struct ResponseNode;

struct Node
{
    int         id;
    NodeType    nodeType = NodeType::Speech;
    std::string text;
    ImVec2      position;
    int nextNodeId = -1;
    int prevNodeId = -1;
    std::vector<int> responses{};
    bool             expectesResponse = false;
    std::set<std::string> selected_callbacks{};

	Node(int _nodeId, NodeType _nodeType, const std::string& _text, ImVec2 _pos) {
		id = _nodeId;
		nodeType = _nodeType;
		text = _text;
		position = _pos;
	}

    ~Node() = default;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Node, id, nodeType, text, nextNodeId, responses, selected_callbacks);

struct Link
{
    int id;
    int start_attr, end_attr;

    Link(int _id, int _start_attr, int _end_attr) {
        id = _id;
        start_attr = _start_attr;
        end_attr = _end_attr;
    }

    bool StartsWithNode(int startNodeId) {
        return start_attr == startNodeId << NodePartShift::EndPin;
    }

    bool EndsWithNode(int endNodeId) {
        return end_attr == endNodeId << NodePartShift::InputPin;
    }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Link, id, start_attr, end_attr);

struct Conditional {
    std::string name;
    bool value;
};

struct State {
	std::unordered_map<int, std::shared_ptr<Node>>                 nodes{}; // maybe these should be smart pointers?
	std::unordered_map<int, std::shared_ptr<Link>>                 links{};
	int                                next_node_id = -1;
	int                                next_link_id = -1;
	std::set<std::string> callbacks{};
	std::set<Conditional> conditionals{};
};
