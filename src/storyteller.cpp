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

#define LOG(x) std::cout << x << std::endl;

/******************************************************************************
 *                   StoryTeller - Main File
 ******************************************************************************/

namespace storyteller
{
	// Anonymous namespace
	// makes the 'StoryTellerNodeEditor editor' instance global in this cpp file only
	namespace 
	{

		class StoryTellerNodeEditor
		{
		private:

			// Current state data
			std::unordered_map<int, Node*>                 nodes;
			std::unordered_map<int, Link*>                 links;
			int                                next_node_id = -1;
			int                                next_link_id = -1;
			static const char* NodeTypeStrings[];

		public:

			// runs every frame
			void show()
			{
				//ImGui::ShowDemoWindow();
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

				otherwindows::ShowMenuBar();

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
							CreateNode(node->id, NodeTypeStrings[node->nodeType]);
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

				/******************************************************************************
				 *                   Create node when link is dropped
				 ******************************************************************************/

				HandleLinkDropped();

				// Handle link creation between two already existing nodes
				// TODO: not working right now
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

				ImGui::End();

				otherwindows::ShowGraphInfoWindow();
				otherwindows::ShowSelectedNodeInfoWindow();
			}	

			void HandleLinkDropped()
			{
				int started_attr;
				if (ImNodes::IsLinkDropped(&started_attr, /*including_detached_links=*/false))
				{
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

			// addition of node to data structure
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

				ImNodes::SetNodeScreenSpacePos(node->id, node->position);
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

							std::cout << "Deleting node " << node_id << " with prevNodeId " << nodes[node_id]->prevNodeId << "\n";
							
							// TODO i should change all these pointers to shared pointers
							if (Node* node = nodes[node_id]) {
								if (Node* prev_node = nodes[node->prevNodeId]) {
									prev_node->nextNodeId = -1;
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

			// Visual creation and insertion in grid
			void CreateNode(int node_id, const char* HeaderText)
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
					if (nodes[node_id]->nextNodeId == -1)
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

			const std::unordered_map<int, Node*>& GetNodesMap() const {
				return nodes;
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

} // namespace storyteller
