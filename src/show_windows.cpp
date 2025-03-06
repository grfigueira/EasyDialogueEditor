/******************************************************************************
 * Copyright (c) 2025 Guilherme Figueira
 * MIT License - See LICENSE file in the project root for details
 * Contact: g.figueira.2002@gmail.com
 ******************************************************************************/

#include "show_windows.h"
#include <imgui.h>
#include <sstream>
#include "node_editor.h"
#include "imgui_markdown.h"
#include <nlohmann/json.hpp>
#include "Utils.h";

#include <string>
#include <format>
#include <iostream>

using json = nlohmann::json;

#define TEXT_BULLET(bullet, text) ImGui::TextWrapped(bullet); ImGui::SameLine(); ImGui::TextWrapped(text);

namespace ede {

//	void Markdown(const std::string& markdown_); // forward declared
//
//	void MarkdownAboutSection()
//	{
//		const char* markdownText = (const char*)u8R"(
//**EasyDialogEditor** is a lightweight dialog tree editor for games, designed to help you visually create dialog flows and export them as JSON. While it's _not_ a dialog system itself, it perfectly complements any existing system if you parse the generated data on your end. The result is a portable solution for building branching dialogues.
//
//**Features**
//- Lightweight and portable, no installation required.
//- Visual creation of dialog trees
//- Straightforward JSON export for easy integration
//- Free and open-source
//# Credits
//Created by [Guilherme Figueira](https://grfigueira.github.io/MyPortfolio/)
//GitHub repository: [github.com/grfigueira/EasyDialogEditor](https://github.com/grfigueira/EasyDialogEditor)
//## Built With
//- [Dear ImGui](https://github.com/ocornut/imgui)
//- [ImNodes](https://github.com/Nelarius/imnodes)
//- [SDL2](https://www.libsdl.org/))";
//		Markdown(markdownText);
//	}

	void ShowAboutWindow(bool* p_open) {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
		if (!ImGui::Begin("About", p_open, ImGuiWindowFlags_::ImGuiWindowFlags_None)) {
			ImGui::End();
			return;
		}
		ImGui::SetWindowSize(ImVec2(550, 400), ImGuiCond_::ImGuiCond_Once);
		ImGui::SeparatorText("About EasyDialogueEditor");
		ImGui::Indent();
		TEXT_BULLET("-", "EasyDialogueEditor is a FOSS and lightweight dialogue tree editor for games.");
		TEXT_BULLET("-", "It allows you to visually create dialog trees and export them as JSON files. It is not a dialog system, but rather a way of visually creating and editing dialog assets. As such, it will work on any dialog system and game engine as long as you deserialize the JSON files on your end.");
		ImGui::Unindent();
		ImGui::SeparatorText("Made possible by open-source");
		ImGui::TextWrapped("This project was created with the help of the following open-source projects");
		ImGui::Indent();
		TEXT_BULLET("-", "ImGui");
		TEXT_BULLET("-", "ImNodes");
		TEXT_BULLET("-", "SDL2");
		ImGui::Unindent();
		ImGui::Separator();
		ImGui::TextWrapped("Created by Guilherme Figueira");
		//MarkdownAboutSection();
		ImGui::End();
		ImGui::PopStyleVar();
	}

	void ShowMenuBar() {
		ImGui::BeginMainMenuBar();
		if (ImGui::BeginMenu("File"))
		{
			ImGui::BeginDisabled();
			if (ImGui::MenuItem("New")) {}
			ImGui::EndDisabled();
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			{
				ImGui::SetTooltip("Not yet implemented");
			}
			if (ImGui::BeginMenu("Export")) {
				if (ImGui::MenuItem("JSON", "Ctrl+X")) {
					ede::FileDialogs::ExportDialogueJsonFile();
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Window")) {
			ImGui::BeginDisabled();
			if (ImGui::MenuItem("Reset layout")) {}
			ImGui::EndDisabled();
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			{
				ImGui::SetTooltip("Not yet implemented");
			}
			if (ImGui::MenuItem("Toggle Demo Window"))
			{
				ede::ToggleDemoWindow();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help")) {
			ImGui::BeginDisabled();
			if (ImGui::MenuItem("How to use")) {
			}
			ImGui::EndDisabled();
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			{
				ImGui::SetTooltip("Not yet implemented");
			}
			if (ImGui::MenuItem("About")) {
				ede::ToggleAboutWindow();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	void ShowGraphInfoWindow()
	{
		float raw_text_block_height = 60.0f;
		ImGui::Begin("Story Graph Info");
		auto nodes = ede::GetNodesVec();

		ImGui::Text("Total number of nodes: %d", nodes.size());
		ImGui::Text("Number of Speech nodes: %d", ede::GetNumNodesOfType(NodeType::Speech));
		ImGui::Text("Number of Response nodes: %d", ede::GetNumNodesOfType(NodeType::Response));

		ImGui::Dummy(ImVec2(0.0f, 5.0f));

		int current_window_height = ImGui::GetContentRegionAvail().y - raw_text_block_height;

		ImGui::BeginChild("Node List", ImVec2(0, current_window_height-300), true, ImGuiWindowFlags_::ImGuiWindowFlags_HorizontalScrollbar |
			ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysVerticalScrollbar);
		ImGui::SeparatorText("Current state");
		ImGui::Dummy(ImVec2(0.0f, 5.0f));
		std::string raw_info;

		for (Node* node : nodes) {
			if (node) {
				if (SpeechNode* speech_node = node->AsSpeech()) {
					std::stringstream ss;
					ss << "{";
					for (int response : speech_node->responses) {
						ss << " " << response << " ";
					}
					ss << "}";
					raw_info += std::format(R"([{{ "id": "{}" ; "type": "Speech" ; "next_node_id": "{}" ; "expected_responses": "{}" }} ; )",
						node->id, node->nextNodeId, ss.str());
					ImGui::Text(std::format("Node {}: {{\n  \"id\": \"{}\",\n  \"type\": \"Speech\",\n \"text\": \"{}\",\n  \"next_node_id\": \"{}\",\n  \"expected_responses\": \"{}\"\n}}", node->id, node->id, node->text.c_str(), node->nextNodeId, ss.str().c_str()).c_str());

				}
				if (ResponseNode* response_node = node->AsResponse()) {

					raw_info += std::format(R"([{{ "id": "{}" ; "type": "Response" ; "next_node_id": "{}" }} ; )",
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

		ImGui::Dummy(ImVec2(0.0f, 5.0f));

		ImGui::BeginChild("CallbackEvents", ImVec2(0, 300), true);
		ImGui::SeparatorText("Callback events");
		ImGui::EndChild();

		ImGui::End();
	}

	/*******************************************************
	*                   Helper methods
	*******************************************************/

	void LinkCallback(ImGui::MarkdownLinkCallbackData data_);
	inline ImGui::MarkdownImageData ImageCallback(ImGui::MarkdownLinkCallbackData data_);

	static ImFont* H1 = NULL;
	static ImFont* H2 = NULL;
	static ImFont* H3 = NULL;

	static ImGui::MarkdownConfig mdConfig;


	void LinkCallback(ImGui::MarkdownLinkCallbackData data_)
	{
		std::string url(data_.link, data_.linkLength);
		if (!data_.isImage)
		{
			ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
		}
	}

}