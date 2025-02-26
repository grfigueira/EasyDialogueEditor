#include "node_editor.h"
#include "imnodes.h"
#include <imgui.h>
#include <string>
#include <vector>
#include "imgui_stdlib.h"
#include <iostream>
#include <cassert>
#include "Node.h"
#include <map>

/******************************************************************************
 *                   StoryTeller - Main File
 ******************************************************************************/

namespace storyteller
{
    namespace
    {

        enum ImNodesMiniMapLocation_
        {
            ImNodesMiniMapLocation_BottomLeft,
            ImNodesMiniMapLocation_BottomRight,
            ImNodesMiniMapLocation_TopLeft,
            ImNodesMiniMapLocation_TopRight,
        };

        class StoryTellerNodeEditor
        {
        private:

            // Current state data
            std::map<int, Node*>                 nodes; // these don't need to be pointers at all
            std::map<int, Link*>                 links;
            int                                next_node_id = -1;
            int                                next_link_id = -1;
            static const char* NodeTypeStrings[];

        public:

            // runs every frame
            void show()
            {
                // Get the screen size from ImGui
                ImGuiIO& io = ImGui::GetIO();

                // Set the next window to fullscreen
                ImGui::SetNextWindowPos(ImVec2(0, 0));
                ImGui::SetNextWindowSize(io.DisplaySize);

                ImGui::Begin("StoryTeller");
                HandleNodeRemoval();

                ImNodes::BeginNodeEditor();


                /******************************************************************************
                 *             Draw every node and link from current state
                 ******************************************************************************/
                {
                    for (const auto& pair : nodes)
                    {
                        Node* node = pair.second;
                        CreateNode(node->id, NodeTypeStrings[node->nodeType]);
                    }

                    for (const auto& pair : links)
                    {
                        Link* link = pair.second;
                        ImNodes::Link(link->id, link->start_attr, link->end_attr);
                    }
                }

                ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_::ImNodesMiniMapLocation_BottomRight);

                ImNodes::EndNodeEditor();

                /******************************************************************************
                 *                   What to do when a link is dropped
                 ******************************************************************************/
                {
                    int started_attr;
                    if (ImNodes::IsLinkDropped(&started_attr, /*including_detached_links=*/false))
                    {
                        std::cout << "it happened" << std::endl;
                        Node* start_node = nodes[started_attr >> NodePartShift::EndPin];

                        if (SpeechNode* speech_start_node = start_node->AsSpeech())
                        {
                            if (speech_start_node->expectesResponse)
                            {
                                AddNode("Yes/No", ImGui::GetMousePos(), NodeType::Response);
                                Link* link = new Link{
                                    ++next_link_id, started_attr, next_node_id << NodePartShift::InputPin };
                                links[next_link_id] = link;
                                speech_start_node->responses.push_back(next_node_id);
                            }
                            else if (speech_start_node->nextNodeId == -1) // isn't connected to any node yet
                            {
                                AddNode("This is interesting...", ImGui::GetMousePos(), NodeType::Speech);
                                Link* link = new Link{
                                    ++next_link_id, started_attr, next_node_id << NodePartShift::InputPin };
                                links[next_link_id] = link;
                                speech_start_node->nextNodeId = next_node_id;
                            }
                        }
                    }
                }

                // Handle link creation between two already existing nodes
                {
                    Link link;
                    if (ImNodes::IsLinkCreated(&link.start_attr, &link.end_attr))
                    {
                        link.id = ++next_link_id;
                        links[link.id] = &link;
                        //links.push_back(link);

                        int startNode = link.start_attr >> NodePartShift::EndPin;
                        int endNode = link.end_attr >> NodePartShift::InputPin;
                        assert(startNode < nodes.size());
                        assert(endNode < nodes.size());
                        if (nodes[startNode]->nodeType == NodeType::Speech)
                        {
                            nodes[startNode]->AsSpeech()->nextNodeId = endNode;
                        }
                    }
                }

                ImGui::Text("info: next_node_id: %d | window width: %f | window height: %f", next_node_id, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
                ImGui::End();
            }

            void ShowNodeCreationPopup()
            {
                if (ImGui::BeginPopup("Add node"))
                {
                    const ImVec2 click_pos = ImGui::GetMousePosOnOpeningCurrentPopup();

                    if (ImGui::MenuItem("Speech"))
                    {
                        AddNode("This is interesting.", click_pos, NodeType::Speech);
                    }
                    if (ImGui::MenuItem("Response"))
                    {
                        AddNode("Yes/No", click_pos, NodeType::Response);
                    }
                    ImGui::EndPopup();
                }
            }

            std::vector<int> GetConnectedLinks(int node_id) {
                std::vector<int> resIds;
                for (const auto& pair : links) {
                    Link* link = pair.second;
                    if (link->EndsWithNode(node_id) || link->StartsWithNode(node_id)) {
                        resIds.push_back(link->id);
                    }
                }
                return resIds;
            }

            /******************************************************************************
             *                   Node creation/removal logic
             ******************************************************************************/

            void AddNode(const char* text, ImVec2 pos, NodeType type)
            {
                Node* node;
                switch (type)
                {
                case NodeType::Speech:
                    node = new SpeechNode(++next_node_id, text, pos);
                    break;
                case NodeType::Response:
                    node = new ResponseNode(++next_node_id, text, pos);
                    break;
                }

                ImNodes::SetNodeScreenSpacePos(node->id, node->position);
                nodes[node->id] = node;
            }

            void HandleNodeRemoval() {
                const int num_nodes_selected = ImNodes::NumSelectedNodes();
                if (num_nodes_selected > 0 && (ImGui::IsKeyReleased(ImGuiKey_Delete)))
                {
                    int* selected_nodes = new int[num_nodes_selected];
                    ImNodes::GetSelectedNodes(selected_nodes);
                    if (selected_nodes) {
                        for (int i = 0; i < num_nodes_selected; ++i) {

                            int node_id = selected_nodes[i];

                            // user shouldn't remove the root node
                            if (node_id == 0) {
                                continue;
                            }

                            // delete every related link
                            for (int link_id : GetConnectedLinks(node_id)) {
                                links.erase(link_id);
                            }

                            nodes.erase(node_id);

                        }
                        delete[] selected_nodes;
                    }
                }
            }

            // Visual creation and insertion in UI grid
            void CreateNode(int node_id, const char* HeaderText)
            {
                ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(66, 150, 250, 255));
                ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(86, 170, 255, 255));

                ImNodes::BeginNode(node_id);

                // Header
                ImNodes::BeginNodeTitleBar();
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 2.0f));
                ImGui::Dummy(ImVec2(0.0f, 0.6f));
                ImGui::TextUnformatted(HeaderText);
                ImGui::Dummy(ImVec2(0.0f, 0.6f));
                ImGui::PopStyleVar();
                ImNodes::EndNodeTitleBar();

                // Add some spacing between title and content
                ImGui::Dummy(ImVec2(0.0f, 4.0f));

                // Text input
                ImNodes::BeginStaticAttribute(node_id << 16);
                ImGui::PushItemWidth(200.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 4.0f));
                if (nodes.find(node_id) != nodes.end() && nodes[node_id] != nullptr) {
                    ImGui::InputText("Text", &nodes[node_id]->text);
                }
                else {
                    // Handle the case where the node is not found or is not initialized
                    std::cerr << "Node with id " << node_id << " is not initialized." << std::endl;
                }
                ImGui::PopStyleVar();
                ImGui::PopItemWidth();
                ImNodes::EndStaticAttribute();

                // checkbox
                if (nodes[node_id]->nodeType == NodeType::Speech)
                {
                    if (nodes[node_id]->AsSpeech()->nextNodeId == -1) 
                    {
                        ImGui::Checkbox("Expects response", &nodes[node_id]->AsSpeech()->expectesResponse);
                    }
                    else 
                    {
                        ImGui::BeginDisabled();
                        ImGui::Checkbox("Expects response", &nodes[node_id]->AsSpeech()->expectesResponse);
                        ImGui::EndDisabled();
                    }
                }

                // Create a row for input and output pins
                ImGui::Dummy(ImVec2(0.0f, 4.0f));

                // input pin
                ImNodes::BeginInputAttribute(node_id << 8);
                ImGui::TextUnformatted("input");
                ImNodes::EndInputAttribute();

                // output pin
                ImGui::SameLine(200);
                ImNodes::BeginOutputAttribute(node_id << 24);
                ImGui::TextUnformatted("output");
                ImNodes::EndOutputAttribute();

                ImGui::Dummy(ImVec2(0.0f, 4.0f));

                ImNodes::EndNode();

                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();

            }

            /******************************************************************************
             ******************************************************************************/
        };

        const char* StoryTellerNodeEditor::NodeTypeStrings[] = { "Speech", "Response" };

        static StoryTellerNodeEditor editor;
    } // namespace

    void InitializeConversation()
    {
        // spawn root node of conversation
        editor.AddNode("Conversation starter", ImVec2(150, ImGui::GetWindowSize().y / 1.2), NodeType::Speech);
    }

    void NodeEditorInitialize()
    {
        ImNodes::SetNodeGridSpacePos(1, ImVec2(200.0f, 200.0f));

        ImNodesStyle& style = ImNodes::GetStyle();

        style.PinCircleRadius = 8.0f;

        style.PinLineThickness = 2.0f;
        style.PinHoverRadius = 10.0f;

        style.NodePadding = ImVec2(8.0f, 8.0f);
        style.NodeCornerRounding = 4.0f;
        style.NodeBorderThickness = 1.0f;   
    }

    void NodeEditorShow() { editor.show(); }

    void NodeEditorShutdown() {}

} // namespace example
