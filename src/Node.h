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
    NodeType    nodeType;
    std::string text;
    ImVec2      position;
    int nextNodeId = -1;
    int prevNodeId = -1;
    std::vector<int> responses;
    bool             expectesResponse;
    std::set<std::string> selected_callbacks;

    SpeechNode* AsSpeech();

    ResponseNode* AsResponse();

    virtual ~Node() = default;
};

struct SpeechNode : public Node
{
    SpeechNode(int nodeId, const std::string& speechText, ImVec2 pos)
    {
        id = nodeId;
        nodeType = NodeType::Speech;
        text = speechText;
        position = pos;
        responses = {};
        nextNodeId = -1;
        prevNodeId = -1;
        expectesResponse = false;
    }
};

struct ResponseNode : public Node
{

    ResponseNode(int nodeId, const std::string& speechText, ImVec2 pos)
    {
        id = nodeId;
        nodeType = NodeType::Response;
        text = speechText;
        position = pos;
        nextNodeId = -1;
        prevNodeId = -1;
    }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Node, id, nodeType, text, nextNodeId, responses, selected_callbacks);
// TODO create a wrapper that exports the current state of the program including links

inline SpeechNode* Node::AsSpeech()
{
    return (nodeType == NodeType::Speech) ? static_cast<SpeechNode*>(this) : nullptr;
}

inline ResponseNode* Node::AsResponse()
{
    return (nodeType == NodeType::Response) ? static_cast<ResponseNode*>(this) : nullptr;
}

struct Link
{
    int id;
    int start_attr, end_attr;

    bool StartsWithNode(int startNodeId) {
        return start_attr == startNodeId << NodePartShift::EndPin;
    }

    bool EndsWithNode(int endNodeId) {
        return end_attr == endNodeId << NodePartShift::InputPin;
    }
};

struct Conditional {
    std::string name;
    bool value;
};

struct State {
	std::unordered_map<int, Node*>                 nodes{}; // maybe these should be smart pointers?
	std::unordered_map<int, Link*>                 links{};
	int                                next_node_id = -1;
	int                                next_link_id = -1;
	std::set<std::string> callbacks{};
	std::set<Conditional> conditionals{};
};

// JSON friendly wrapper
struct StateWrapper {
    std::vector<Node> nodes{};
    std::vector<Link> nodes{};

};

NLOHMANN_DEFINE_TYPE_INTRUSIVE(State, nodes, links, next_node_id, next_link_id, callbacks);
