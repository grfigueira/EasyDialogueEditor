/******************************************************************************
 * Copyright (c) 2025 Guilherme Figueira
 * MIT License - See LICENSE file in the project root for details
 * Contact: g.figueira.2002@gmail.com
 ******************************************************************************/

#include "node_editor.h"
#include "imnodes.h"
#include <imgui.h>
#include <string>
#include <vector>
#include "imgui_stdlib.h"
#include <iostream>
#include "Node.h"
#include "show_windows.h"
#include <unordered_map>
#include <imgui_internal.h>
#include <format>
#include <nlohmann/json.hpp>

#define LOG(x) std::cout << x << std::endl;

/******************************************************************************
 *                   EasyDialogEditor - Main file
 ******************************************************************************/

namespace ede
{
	// Anonymous namespace
	// makes the 'StoryTellerNodeEditor editor' instance global in this cpp file only
	namespace
	{

		class EasyDialogEditor
		{
		private:

			// Current state data
			std::unordered_map<int, Node*>                 nodes; // maybe these should be smart pointers?
			std::unordered_map<int, Link*>                 links;
			int                                next_node_id = -1;
			int                                next_link_id = -1;
			static const char* NodeTypeStrings[];
			bool bShowDemoWindow, bShowAboutSection, bShowCreateNodeTooltip;

		public:

			// runs every frame
			void show()
			{
				if (bShowDemoWindow) {
					ImGui::ShowDemoWindow();
				}

				if (bShowAboutSection) {
					ede::ShowAboutWindow(&bShowAboutSection);
				}

				if (bShowCreateNodeTooltip) {
					ImGui::SetTooltip("+ Add Node");
				}

				ImGuiIO& io = ImGui::GetIO();
				ImGuiViewport* viewport = ImGui::GetMainViewport();

				ImGui::SetNextWindowPos(viewport->Pos);
				ImGui::SetNextWindowSize(viewport->Size);
				ImGui::SetNextWindowViewport(viewport->ID);

				ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
					ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
					ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

				window_flags |= ImGuiWindowFlags_NoDocking;

				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

				ImGui::Begin("MainDockspace", nullptr, window_flags);
				ImGui::PopStyleVar(3);

				ImGuiID dockspace_id = ImGui::GetID("MainDockspace");
				ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

				ImGui::End();

				ImGui::Begin("Graph Editor", 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize |
					ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar |
					ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse |
					ImGuiWindowFlags_::ImGuiWindowFlags_NoScrollbar |
					ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);

				ede::ShowMenuBar();

				HandleNodeRemoval();

				ImNodes::BeginNodeEditor();

				/******************************************************************************
				 *             Draw every node and link from current state
				 ******************************************************************************/
				{
					for (const auto& pair : nodes)
					{
						Node* node = pair.second;
						if (node) {
							 std::string header_text = std::format("{} | id: {}", NodeTypeStrings[node->nodeType], node->id);
							DrawNode(node->id, header_text.c_str());
						}
					}

					for (const auto& pair : links)
					{
						Link* link = pair.second;
						if (link) {
							ImNodes::Link(link->id, link->start_attr, link->end_attr);
						}
					}
				}

				ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_::ImNodesMiniMapLocation_BottomRight);

				ImNodes::EndNodeEditor();

				/***************************************************
				 *                   Handle links
				 **************************************************/

				int start_attr, end_attr;
				if (ImNodes::IsLinkCreated(&start_attr, &end_attr))
				{
					HandleLinkManualCreation(start_attr, end_attr);
					bShowCreateNodeTooltip = false;
				}
				else {
					HandleLinkDropped();
				}

				if (ImNodes::IsLinkStarted(&start_attr)) {
					bShowCreateNodeTooltip = true;
				}

				ImGui::End();

				ede::ShowGraphInfoWindow();
			}

			// executed if user link two already existing nodes
			void HandleLinkManualCreation(int start_attr, int end_attr)
			{
				int start_node_id = start_attr >> NodePartShift::EndPin;
				int end_node_id = end_attr >> NodePartShift::InputPin;

				if (SpeechNode* start_speech_node = nodes[start_node_id]->AsSpeech()) {
					if (ResponseNode* end_response_node = nodes[end_node_id]->AsResponse()) {
						if (!start_speech_node->expectesResponse) {
							return;
						}
						else {
							start_speech_node->responses.push_back(end_node_id);
							end_response_node->prevNodeId = start_node_id;
						}
					}
					else {
						start_speech_node->nextNodeId = end_node_id;
					}
				}
				Link* link = new Link{ ++next_link_id, start_attr, end_attr };
				links[link->id] = link;

			}

			// create new node when dropping a link on empty space
			void HandleLinkDropped()
			{
				int started_attr;
				if (ImNodes::IsLinkDropped(&started_attr, /*including_detached_links=*/false))
				{
					bShowCreateNodeTooltip = false;
					LOG("linked dropped");
					Node* start_node = nodes[started_attr >> NodePartShift::EndPin];

					if (SpeechNode* speech_start_node = start_node->AsSpeech())
					{
						if (speech_start_node->expectesResponse)
						{
							Node* newNode = AddNode("Yes/No", ImGui::GetMousePos(), NodeType::Response);
							newNode->prevNodeId = started_attr >> NodePartShift::EndPin;

							Link* link = new Link{
								++next_link_id, started_attr, next_node_id << NodePartShift::InputPin };
							links[next_link_id] = link;
							speech_start_node->responses.push_back(next_node_id);
							return;
						}
					}
					if (start_node->nextNodeId == -1) // isn't connected to any node yet
					{
						Node* newNode = AddNode("This is interesting...", ImGui::GetMousePos(), NodeType::Speech);
						newNode->prevNodeId = started_attr >> NodePartShift::EndPin;

						Link* link = new Link{
							++next_link_id, started_attr, next_node_id << NodePartShift::InputPin };
						links[next_link_id] = link;
						start_node->nextNodeId = next_node_id;
					}
				}
			}
			
			[[deprecated("Creating nodes now works by dropping links")]]
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

			std::vector<Node> GetNodesData() {
				std::vector<Node> res;
				for (const auto& pair : nodes) {
					Node* node = pair.second;
					res.push_back(*node);
				}
				return res;
			}

			/******************************************************************************
			 *                   Node creation/removal logic
			 ******************************************************************************/

			 // addition of node to state data
			Node* AddNode(const char* text, ImVec2 pos, NodeType type)
			{
				pos.y -= 110.f;

				Node* node{};
				switch (type)
				{
				case NodeType::Speech:
					node = new SpeechNode(++next_node_id, text, pos);
					break;
				case NodeType::Response:
					node = new ResponseNode(++next_node_id, text, pos);
					break;
				}
				if (node) {
					ImNodes::SetNodeScreenSpacePos(node->id, node->position);
				}
				nodes[node->id] = node;
				return node;
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

							if (Node* node = nodes[node_id]) {

								if (Node* prev_node = nodes[node->prevNodeId]) {
									prev_node->nextNodeId = -1;

									if (node->nodeType == NodeType::Response) {
										if (SpeechNode* prev_speech_node = prev_node->AsSpeech()) {
											std::vector<int>& responses = prev_speech_node->responses;
											responses.erase(std::remove(responses.begin(), responses.end(), node_id), responses.end());
										}
									}
								}

								if (node->nextNodeId != -1) {
									if (Node* next_node = nodes[node->nextNodeId]) {
										next_node->prevNodeId = -1;
										std::cout << "next_node info: " << next_node->id << "\n";
									}
								}

							}

							nodes.erase(node_id);

						}
						delete[] selected_nodes;
					}
				}
			}

			// Renders a node on the grid
			void DrawNode(int node_id, const char* HeaderText)
			{
				Node* node = nodes[node_id];
				if (node)
				{

					ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(66, 150, 250, 255));
					ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(86, 170, 255, 255));

					ImNodes::BeginNode(node_id);

					// header
					ImNodes::BeginNodeTitleBar();
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 2.0f));
					ImGui::Dummy(ImVec2(0.0f, 0.6f));
					ImGui::TextUnformatted(HeaderText);
					ImGui::Dummy(ImVec2(0.0f, 0.6f));
					ImGui::PopStyleVar();
					ImNodes::EndNodeTitleBar();

					// spacing
					ImGui::Dummy(ImVec2(0.0f, 4.0f));

					// text input
					ImNodes::BeginStaticAttribute(node_id << 16);
					ImGui::PushItemWidth(200.0f);
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 4.0f));
					ImGui::InputText("Text", &nodes[node_id]->text);
					ImGui::PopStyleVar();
					ImGui::PopItemWidth();
					ImNodes::EndStaticAttribute();

					// checkbox
					if (nodes[node_id]->nodeType == NodeType::Speech)
					{
						SpeechNode* speech_node = nodes[node_id]->AsSpeech();
						if (speech_node->nextNodeId == -1 && speech_node->responses.empty())
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

					ImGui::Dummy(ImVec2(0.0f, 4.0f));

					// input pin
					if(node_id != 0)
					{
						ImNodes::BeginInputAttribute(node_id << 8);
						ImGui::TextUnformatted("input");
						ImNodes::EndInputAttribute();
					}
					
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
			}

			const std::unordered_map<int, Node*>& GetNodesMap() const {
				return nodes;
			}

			void ToggleDemoWindow() {
				bShowDemoWindow = !bShowDemoWindow;
			}

			void ToggleAboutWindow() {
				bShowAboutSection = !bShowAboutSection;
			}

			/******************************************************************************
			 ******************************************************************************/
		};

		const char* EasyDialogEditor::NodeTypeStrings[] = { "Speech", "Response" };

		static EasyDialogEditor editor;
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

		ede::LoadFonts(16.f);

		style.PinCircleRadius = 8.0f;
		style.PinQuadSideLength = 100.0f;

		style.PinLineThickness = 2.0f;
		style.PinHoverRadius = 10.0f;

		style.NodePadding = ImVec2(8.0f, 8.0f);
		style.NodeCornerRounding = 4.0f;
		style.NodeBorderThickness = 1.0f;
	}

	void NodeEditorShow() { editor.show(); }

	void NodeEditorShutdown() {}

	/*************************************
	*               Getters
	**************************************/

	std::vector<Node*> GetNodesVec() {
		const auto& nodesMap = editor.GetNodesMap();

		std::vector<Node*> nodes;
		nodes.reserve(nodesMap.size());

		for (auto& pair : nodesMap) {
			nodes.push_back(pair.second);
		}
		return nodes;
	}

	int GetNumNodesOfType(NodeType type) {
		const auto& nodesMap = editor.GetNodesMap();
		int res = 0;

		for (auto& pair : nodesMap) {
			if (pair.second) {
				if (pair.second->nodeType == type) {
					res++;
				}
			}
		}
		return res;
	}

	std::vector<Node> GetNodesData() {
		return editor.GetNodesData();
	}

	/*************************************
	*               Others
	**************************************/

	void ToggleDemoWindow() {
		editor.ToggleDemoWindow();
	}

	void ToggleAboutWindow() {
		editor.ToggleAboutWindow();
	}

} // namespace storyteller
