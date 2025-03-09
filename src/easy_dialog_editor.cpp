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
#include <set>

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
			State current_state;
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
					for (const auto& pair : current_state.nodes)
					{
						std::shared_ptr<Node> node = pair.second;
						if (node) {
							 std::string header_text = std::format("{} | id: {}", NodeTypeStrings[node->nodeType], node->id);
							DrawNode(node->id, header_text.c_str());
						}
					}

					for (const auto& pair : current_state.links)
					{
						std::shared_ptr<Link> link = pair.second;
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
				std::shared_ptr<Node> start_node = current_state.nodes[start_node_id];
				std::shared_ptr<Node> end_node = current_state.nodes[end_node_id];

				if (start_node->nodeType == NodeType::Speech) {
					if (end_node->nodeType == NodeType::Response) {
						if (!start_node->expectesResponse) {
							return;
						}
						else {
							start_node->responses.push_back(end_node_id);
							end_node->prevNodeId = start_node_id;
						}
					}
					else {
						start_node->nextNodeId = end_node_id;
					}
				}

				if (start_node->nodeType == NodeType::Response && end_node->nodeType == NodeType::Response) {
					return;
				}
				
				std::shared_ptr<Link> link = std::make_shared<Link>(++current_state.next_link_id, start_attr, end_attr);
				current_state.links[link->id] = link;

			}

			// create new node when dropping a link on empty space
			void HandleLinkDropped()
			{
				int started_attr;
				if (ImNodes::IsLinkDropped(&started_attr, /*including_detached_links=*/false))
				{
					bShowCreateNodeTooltip = false;
					LOG("linked dropped");
					std::shared_ptr<Node> start_node = current_state.nodes[started_attr >> NodePartShift::EndPin];

					if (start_node)
					{
						if (start_node->expectesResponse)
						{
							std::shared_ptr<Node> newNode = AddNode("Yes/No", ImGui::GetMousePos(), NodeType::Response);
							newNode->prevNodeId = started_attr >> NodePartShift::EndPin;

							std::shared_ptr<Link> link = std::make_shared<Link>(
								++current_state.next_link_id, started_attr, current_state.next_node_id << NodePartShift::InputPin);

							current_state.links[current_state.next_link_id] = link;

							start_node->responses.push_back(current_state.next_node_id);

							return;
						}
					}
					if (start_node->nextNodeId == -1) // isn't connected to any node yet
					{
						std::shared_ptr<Node> newNode = AddNode("This is interesting...", ImGui::GetMousePos(), NodeType::Speech);
						newNode->prevNodeId = started_attr >> NodePartShift::EndPin;

						std::shared_ptr<Link> link = std::make_shared<Link>(
							++current_state.next_link_id, started_attr, current_state.next_node_id << NodePartShift::InputPin);

						current_state.links[current_state.next_link_id] = link;

						start_node->nextNodeId = current_state.next_node_id;
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
				for (const auto& pair : current_state.links) {
					std::shared_ptr<Link> link = pair.second;
					if (link->EndsWithNode(node_id) || link->StartsWithNode(node_id)) {
						resIds.push_back(link->id);
					}
				}
				return resIds;
			}

			std::vector<Node> GetNodesData() {
				std::vector<Node> res;
				for (const auto& pair : current_state.nodes) {
					std::shared_ptr<Node> node = pair.second;
					res.push_back(*node);
				}
				return res;
			}

			const State& GetCurrentState() const {
				return current_state;
			}

			/******************************************************************************
			 *                   Node creation/removal logic
			 ******************************************************************************/

			 // addition of node to state data
			std::shared_ptr<Node> AddNode(const char* text, ImVec2 pos, NodeType type)
			{
				pos.y -= 110.f;

				std::shared_ptr<Node> node = std::make_shared<Node>(++current_state.next_node_id, type, text, pos);

				if (node) {
					ImNodes::SetNodeScreenSpacePos(node->id, node->position);
				}
				current_state.nodes[node->id] = node;
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
								current_state.links.erase(link_id);
							}

							if (std::shared_ptr<Node> node = current_state.nodes[node_id]) {

								if (std::shared_ptr<Node> prev_node = current_state.nodes[node->prevNodeId]) {
									prev_node->nextNodeId = -1;
									if (node->nodeType == NodeType::Response) {
											std::vector<int>& responses = prev_node->responses;
											responses.erase(std::remove(responses.begin(), responses.end(), node_id), responses.end());
									}
								}

								if (node->nextNodeId != -1) {
									if (std::shared_ptr<Node> next_node = current_state.nodes[node->nextNodeId]) {
										next_node->prevNodeId = -1;
										std::cout << "next_node info: " << next_node->id << "\n";
									}
								}

							}

							current_state.nodes.erase(node_id);

						}
						delete[] selected_nodes;
					}
				}
			}

			// Renders a node on the grid
			void DrawNode(int node_id, const char* HeaderText)
			{
				std::shared_ptr<Node> node = current_state.nodes[node_id];
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
					ImGui::InputText("Text", &current_state.nodes[node_id]->text);
					ImGui::PopStyleVar();
					ImGui::PopItemWidth();
					ImNodes::EndStaticAttribute();

					// checkbox
					if (node->nodeType == NodeType::Speech)
					{
						if (node->nextNodeId == -1 && node->responses.empty())
						{
							ImGui::Checkbox("Expects response", &node->expectesResponse);
						}
						else
						{
							ImGui::BeginDisabled();
							ImGui::Checkbox("Expects response", &node->expectesResponse);
							ImGui::EndDisabled();
						}
					}

					ImGui::Dummy(ImVec2(0.0f, 4.0f));

					// callback selection
					const char* combo_preview_value = "Select callback";

					// Pass in the preview value visible before opening the combo (it could technically be different contents or not pulled from items[])
					if (!node->selected_callbacks.empty()) {
						const char* combo_preview_value = "Select callback";
					}

					if (ImGui::BeginCombo("Callback tags", combo_preview_value, ImGuiComboFlags_::ImGuiComboFlags_WidthFitPreview))
					{
						ImGui::PushItemFlag(ImGuiItemFlags_::ImGuiItemFlags_SelectableDontClosePopup, true);
						for (auto& callback : current_state.callbacks)
						{
							const bool is_selected = node->selected_callbacks.contains(callback);
							if (ImGui::Selectable(callback.c_str(), is_selected)) {

								// callback tag was already selected
								if(!node->selected_callbacks.insert(callback).second) {

									auto it = node->selected_callbacks.find(callback);

									if (it != node->selected_callbacks.end()) {
										node->selected_callbacks.erase(it);
									}
								}
							}

							// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::PopItemFlag();
						ImGui::EndCombo();
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

			const std::unordered_map<int, std::shared_ptr<Node>>& GetNodesMap() const {
				return current_state.nodes;
			}

			std::set<std::string>& GetCallbacks() {
				return current_state.callbacks;
			}

			void ToggleDemoWindow() {
				bShowDemoWindow = !bShowDemoWindow;
			}

			void ToggleAboutWindow() {
				bShowAboutSection = !bShowAboutSection;
			}
			void NotifyCallbackDeletion(const std::string& deleted_callback) {
				for (auto& node_pair : current_state.nodes) {
					std::shared_ptr<Node> node = node_pair.second;
					if (node) {
						node->selected_callbacks.erase(deleted_callback);
					}
				}
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

		style.PinCircleRadius = 8.0f;
		style.PinQuadSideLength = 100.0f;

		style.PinLineThickness = 2.0f;
		style.PinHoverRadius = 10.0f;

		style.NodePadding = ImVec2(8.0f, 8.0f);
		style.NodeCornerRounding = 6.0f;
		style.NodeBorderThickness = 1.0f;
	}

	void NodeEditorShow() { editor.show(); }

	void NodeEditorShutdown() {}

	/*************************************
	*               Getters
	**************************************/

	std::vector<std::shared_ptr<Node>> GetNodesVec() {
		const auto& nodesMap = editor.GetNodesMap();

		std::vector<std::shared_ptr<Node>> nodes;
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

	std::set<std::string>& GetCallbacksMutable() {
		return editor.GetCallbacks();
	}

	const State& GetCurrentState() {
		return editor.GetCurrentState();
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

	void NotifyCallbackDeletion(const std::string& deleted_callback) {
		editor.NotifyCallbackDeletion(deleted_callback);
	}

} // namespace storyteller
