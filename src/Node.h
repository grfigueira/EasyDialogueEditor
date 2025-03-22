/******************************************************************************
    Created by Guilherme Figueira, 2025

    My contacts (feel free to reach out):
    - Github: https://github.com/grfigueira
    - LinkedIn: https://www.linkedin.com/in/grfigueira/
 ******************************************************************************/

#pragma once
#include <string>
#include <vector>
#include "imnodes.h"
#include <imgui.h>
#include <set>
#include <nlohmann/json.hpp>

#define NOT_CURRENTLY_IN_USE 0

/******************************************************************************
 *                   Every node-related data structure
 ******************************************************************************/

// used for converting node_ids to parameters and vice-versa using bit-shifting
enum NodePartShift
{
    InputPin = 8,
    EndPin = 24,
};

enum NodeType
{
    Speech,
    Response,
    NodeTypeCount // used to convert NodeType to string. do not remove, and keep it in last.
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
    std::vector<int> prevNodeIds{};
    std::vector<int> responses{};
    bool             expectesResponse = false;
    std::set<std::string> selected_callbacks{};

    // for runtime node creation
	Node(int _nodeId, NodeType _nodeType, const std::string& _text, ImVec2 _pos) 
    {
		id = _nodeId;
		nodeType = _nodeType;
		text = _text;
		position = _pos;
	}

    // for state loading
    Node(int _nodeId, NodeType _nodeType, const std::string& _text, ImVec2 _pos,
        int _nextNodeId, std::vector<int>& _prevNodeIds, std::vector<int>& _responses, bool _expectesResponse, std::set<std::string>& _selected_callbacks)
    {
		id = _nodeId;
		nodeType = _nodeType;
		text = _text;
		position = _pos;
        nextNodeId = _nextNodeId;
        prevNodeIds = _prevNodeIds;
        responses = _responses;
        expectesResponse = _expectesResponse;
        selected_callbacks = _selected_callbacks;
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

#if NOT_CURRENTLY_IN_USE
struct Conditional {
    std::string name;
    bool value;
};
#endif

struct State {
	std::unordered_map<int, std::shared_ptr<Node>>                 nodes{};
	std::unordered_map<int, std::shared_ptr<Link>>                 links{};
	int                                next_node_id = -1;
	int                                next_link_id = -1;
	std::set<std::string> callbacks{};
	//std::set<Conditional> conditionals{}; // pontential future feature, we'll see.
};
