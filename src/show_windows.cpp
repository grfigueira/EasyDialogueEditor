/******************************************************************************
 * Copyright (c) 2025 Guilherme Figueira
 * MIT License - See LICENSE file in the project root for details
 * Contact: g.figueira.2002@gmail.com
 ******************************************************************************/

#include "show_windows.h"
#include <imgui.h>
#include <sstream>
#include "node_editor.h"
#include <format>

namespace otherwindows {
	void ShowMenuBar() {
		ImGui::BeginMainMenuBar();
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New")) {}
			if (ImGui::BeginMenu("Save/Export")) {
				if (ImGui::MenuItem("JSON")) {}
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Window")) {
			if (ImGui::MenuItem("Reset layout")) {}
			if (ImGui::MenuItem("Toggle Demo Window")) 
			{
				storyteller::ToggleDemoWindow();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help")) {
			if (ImGui::MenuItem("How to use")) {
				
			}
			if (ImGui::MenuItem("About")) {

			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	void ShowGraphInfoWindow()
	{
		float raw_text_block_height = 50.0f;
		ImGui::Begin("Story Graph Info");
		auto nodes = storyteller::GetNodesVec();

		ImGui::Text("Total number of nodes: %d", nodes.size());
		ImGui::Text("Number of Speech nodes: %d", storyteller::GetNumNodesOfType(NodeType::Speech));
		ImGui::Text("Number of Response nodes: %d", storyteller::GetNumNodesOfType(NodeType::Response));

		ImGui::Dummy(ImVec2(0.0f, 5.0f));

		int current_window_height = ImGui::GetContentRegionAvail().y - raw_text_block_height;

		ImGui::BeginChild("Node List", ImVec2(0, current_window_height), true, ImGuiWindowFlags_::ImGuiWindowFlags_HorizontalScrollbar |
			ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysVerticalScrollbar);

		std::string raw_info = "";

		for (Node* node : nodes) {
			if (node) {
				if (SpeechNode* speech_node = node->AsSpeech()) {
					std::stringstream ss;
					ss << "{";
					for (int response : speech_node->responses) {
						ss << " " << response << " ";
					}
					ss << "}";
					//char* info;
					//std::sprintf(&info, "{ id: %d ; type: Speech ; next_node_id: %d ; expected_responses: %s }", node->id, node->nextNodeId, ss.str().c_str());
					raw_info += std::format("[{{ \"id\": \"{}\" ; \"type\": \"Speech\" ; \"next_node_id\": \"{}\" ; \"expected_responses\": \"{}\" }} ; ",	
						node->id, node->nextNodeId, ss.str());
					ImGui::Text(std::format("Node {}: {{\n  \"id\": \"{}\",\n  \"type\": \"Speech\",\n \"text\": \"{}\",\n  \"next_node_id\": \"{}\",\n  \"expected_responses\": \"{}\"\n}}", node->id, node->id, node->text.c_str(), node->nextNodeId, ss.str().c_str()).c_str());

				}
				if (ResponseNode* response_node = node->AsResponse()) {

					raw_info += std::format("[{{ \"id\": \"{}\" ; \"type\": \"Response\" ; \"next_node_id\": \"{}\" }} ; ",
						node->id, node->nextNodeId);

					ImGui::Text("Node %d: {\n  \"id\": \"%d\",\n  \"type\": \"Response\",\n  \"next_node_id\": \"%d\"\n}", node->id, node->id, node->nextNodeId);

				}
				ImGui::Dummy(ImVec2(0.0f, 2.0f));
			}
		}
		raw_info += "]";

		ImGui::EndChild();


		ImGui::Dummy(ImVec2(0.0f, 5.0f));

		ImGui::BeginChild("ScrollableText", ImVec2(0, raw_text_block_height), true);
		ImGui::InputTextMultiline("##rawdata",
			raw_info.data(),
			raw_info.size() + 1,
			ImVec2(-1.0f, -1.0f),
			ImGuiInputTextFlags_ReadOnly);
		ImGui::EndChild();

		ImGui::End();
	}

}