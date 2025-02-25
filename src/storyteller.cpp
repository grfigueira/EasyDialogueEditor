#include "node_editor.h"
#include "imnodes.h"
#include <imgui.h>
#include <string>
#include <vector>
#include "imgui_stdlib.h"
#include <iostream>
#include <cassert>
#include "Node.h"

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
            std::vector<std::shared_ptr<Node>> nodes;
            std::vector<Link>                  links;
            int                                next_node_id = -1;
            int                                next_link_id = -1;
            static const char* NodeTypeStrings[];

        public:

            /******************************************************************************
             *                   show(): Runs every frame
             ******************************************************************************/

            void show()
            {
                // Get the screen size from ImGui
                ImGuiIO& io = ImGui::GetIO();

                // Set the next window to fullscreen
                ImGui::SetNextWindowPos(ImVec2(0, 0));
                ImGui::SetNextWindowSize(io.DisplaySize);

                ImGui::Begin("StoryTeller");

                ImNodes::BeginNodeEditor();

                /******************************************************************************
                 *             Draw every node and link from current state
                 ******************************************************************************/
                {
                    for (const std::shared_ptr<Node> node : nodes)
                    {
                        CreateNode(node->id, NodeTypeStrings[node->nodeType]);
                    }

                    for (const Link& link : links)
                    {
                        ImNodes::Link(link.id, link.start_attr, link.end_attr);
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
                        std::shared_ptr<Node> start_node = nodes[started_attr >> NodePartShift::EndPin];

                        if (SpeechNode* speech_start_node = start_node->AsSpeech())
                        {
                            if (speech_start_node->expectesResponse)
                            {
                                AddNode("Yes/No", ImGui::GetMousePos(), NodeType::Response);
                                Link link = {
                                    ++next_link_id, started_attr, next_node_id << NodePartShift::InputPin };
                                links.push_back(link);
                                speech_start_node->responses.push_back(next_node_id);
                            }
                            else if (speech_start_node->nextNodeId == -1) // isn't connected to any node yet
                            {
                                AddNode("This is interesting...", ImGui::GetMousePos(), NodeType::Speech);
                                Link link = {
                                    ++next_link_id, started_attr, next_node_id << NodePartShift::InputPin };
                                links.push_back(link);
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
                        links.push_back(link);

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

                ImGui::Text("next_node_id: %d", next_node_id);
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

            /******************************************************************************
             *                   Node creation/removal logic
             ******************************************************************************/

            void AddNode(const char* text, ImVec2 pos, NodeType type)
            {
                std::shared_ptr<Node> node;
                switch (type)
                {
                case NodeType::Speech:
                    node = std::make_shared<SpeechNode>(++next_node_id, text, pos);
                    break;
                case NodeType::Response:
                    node = std::make_shared<ResponseNode>(++next_node_id, text, pos);
                    break;
                }

                ImNodes::SetNodeScreenSpacePos(node->id, node->position);
                nodes.push_back(node);
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
                assert(node_id < nodes.size());
                ImGui::InputText("Text", &nodes[node_id]->text);
                ImGui::PopStyleVar();
                ImGui::PopItemWidth();
                ImNodes::EndStaticAttribute();

                // checkbox
                assert(node_id < nodes.size());
                if (nodes[node_id]->nodeType == NodeType::Speech)
                {
                    ImGui::Checkbox("Expects response", &nodes[node_id]->AsSpeech()->expectesResponse);
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

        editor.AddNode("Conversation starter", ImVec2(0, 0), NodeType::Speech);
    }

    void NodeEditorShow() { editor.show(); }

    void NodeEditorShutdown() {}

} // namespace example
