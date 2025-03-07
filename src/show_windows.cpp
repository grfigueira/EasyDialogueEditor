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
#include <algorithm>

#include <string>
#include <format>
#include <iostream>
#include <set>
#include <imgui_internal.h>

using json = nlohmann::json;

#define TEXT_BULLET(bullet, text) ImGui::TextWrapped(bullet); ImGui::SameLine(); ImGui::TextWrapped(text);

namespace ede {

	static void HelpMarker(const char* desc)
	{
		ImGui::TextDisabled("(?)");
		if (ImGui::BeginItemTooltip())
		{
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	void ShowAboutWindow(bool* p_open) {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
		if (!ImGui::Begin("About", p_open, ImGuiWindowFlags_::ImGuiWindowFlags_None)) {
			ImGui::End();
			return;
		}
		ImGui::SetWindowSize(ImVec2(550, 335), ImGuiCond_::ImGuiCond_Once);
		ImGui::SeparatorText("About EasyDialogueEditor");
		ImGui::Indent();
		TEXT_BULLET(">", "EasyDialogueEditor is a FOSS and lightweight dialogue tree editor for games.");
		TEXT_BULLET(">", "It allows you to visually create dialog trees and export them as JSON files. It is not a dialog system, but rather a way of visually creating and editing dialog assets. As such, it will work on any dialog system and game engine as long as you deserialize the JSON files on your end.");
		ImGui::Unindent();
		ImGui::Dummy(ImVec2(0.0f, 5.f));
		ImGui::SeparatorText("Made possible by open-source");
		ImGui::Dummy(ImVec2(0.0f, 5.f));
		ImGui::TextWrapped("This project was created with the help of the following open-source libraries:");
		ImGui::Indent();
		TEXT_BULLET(">", "ocornut/imgui");
		TEXT_BULLET(">", "Nelarius/imnodes");
		TEXT_BULLET(">", "nlohmann/json");
		TEXT_BULLET(">", "SDL2");
		ImGui::Unindent();
		ImGui::Dummy(ImVec2(0.0f, 5.f));
		ImGui::Separator();
		ImGui::Dummy(ImVec2(0.0f, 5.f));
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

		ImGui::BeginChild("Node List", ImVec2(0, current_window_height-415), true, ImGuiWindowFlags_::ImGuiWindowFlags_HorizontalScrollbar |
			ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysVerticalScrollbar);
		ImGui::SeparatorTextEx(0, "Current state", NULL, ImGui::CalcTextSize(" (?)").x);
		ImGui::SameLine();
		HelpMarker("Displays how the current state will be exported");
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
					if (speech_node->responses.empty())
						ss << " ";
					ss << "}";
					raw_info += std::format(R"([{{ "id": "{}" ; "type": "Speech" ; "next_node_id": "{}" ; "expected_responses": "{}" }} ; )",
						node->id, node->nextNodeId, ss.str());
					std::set<std::string>& current_callbacks = node->selected_callbacks;
					std::ostringstream stream;
					std::copy(current_callbacks.begin(), current_callbacks.end(), std::ostream_iterator<std::string>(stream, " "));
					std::string result_str = stream.str();
					ImGui::Text(std::format("Node {}: {{\n  \"id\": \"{}\",\n  \"type\": \"Speech\",\n \"text\": \"{}\",\n  \"next_node_id\": \"{}\",\n  \"expected_responses\": \"{}\",\n \"callbacks\": \"{{ {}}}\"\n}}", node->id, node->id, node->text, node->nextNodeId, ss.str(), result_str).c_str());

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

		ImGui::BeginChild("ScrollableText", ImVec2(0, raw_text_block_height + 15), true);
		ImGui::SeparatorTextEx(0, "Raw string", NULL, ImGui::CalcTextSize(" (?)").x);
		ImGui::SameLine();
		HelpMarker("Raw JSON output. In case you want to quickly copy and paste somewhere");
		ImGui::InputTextMultiline("##rawdata",
			raw_info.data(),
			raw_info.size() + 1,
			ImVec2(-1.0f, -1.0f),
			ImGuiInputTextFlags_ReadOnly);
		ImGui::EndChild();

		ImGui::Dummy(ImVec2(0.0f, 5.0f));

		ImGui::BeginChild("CallbackEvents", ImVec2(0, ImGui::GetContentRegionAvail().y), true);
		ImGui::SeparatorTextEx(0, "Callback events tags", NULL, ImGui::CalcTextSize(" (?)").x);
		ImGui::SameLine();
		ImGui::TextDisabled("(?)");
		if (ImGui::BeginItemTooltip())
		{
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted("Adding \"callback tags\" to nodes can be useful to trigger callback events in your dialog system when a node is displayed");
			ImGui::TextUnformatted("For example:");
			ImGui::Indent();
			TEXT_BULLET(">", "You may have a riddle speech node connected to multiple response nodes");
			TEXT_BULLET(">", "One of the response nodes can have a \"pass_riddle\" callback tag that triggers an event in-game that unlocks a door or something");
			TEXT_BULLET(">", "Other reponses can have a \"trigger_battle\" callback tag that triggers a battle if the player chooses that response");
			ImGui::Unindent();
			ImGui::TextUnformatted("Just remember to check for these tags in your game's system");
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
		std::set<std::string>& current_callbacks = ede::GetCallbacksMutable();
		ImGui::Indent();
		for (auto& callback : current_callbacks) {
			TEXT_BULLET(">", callback.c_str());
			ImGui::SameLine();
			if (ImGui::Button("X")) {
				auto it = std::find(current_callbacks.begin(), current_callbacks.end(), callback);
				if (it != current_callbacks.end()) {
					current_callbacks.erase(it);
				}
			}
		}
		ImGui::Unindent();

		static char new_callback[128] = "";

		// user requests addition of new callback tag
		if (ImGui::InputTextWithHint("##callback_input", "New callback tag...", new_callback, IM_ARRAYSIZE(new_callback), ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue) 
			|| ImGui::Button("Add")) {
			std::string callback_str = new_callback;
			
			// is callback tag valid, i.e. not empty or blank
			if(!callback_str.empty() 
				&& !std::all_of(callback_str.begin(), callback_str.end(), [](unsigned char c) {return std::isspace(c);})) 
			{
				current_callbacks.insert(new_callback);
				strcpy(new_callback, "");
				ImGui::SetKeyboardFocusHere(-1);
			}			
		}
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