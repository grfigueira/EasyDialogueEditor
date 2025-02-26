#pragma once
#include <string>
#include <vector>
#include "imnodes.h"
#include <imgui.h>

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

    SpeechNode* AsSpeech();

    ResponseNode* AsResponse();

    virtual ~Node() = default;
};

struct SpeechNode : public Node
{
    std::vector<int> responses;
    int              nextNodeId;
    bool             expectesResponse;

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

