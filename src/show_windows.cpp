/******************************************************************************
	Created by Guilherme Figueira, 2025

	My contacts (feel free to reach out):
	- Github: https://github.com/grfigueira
	- LinkedIn: https://www.linkedin.com/in/grfigueira/
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

	void ShowHowToUseGuide(bool* p_open)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
		if (!ImGui::Begin("How to Use", p_open, ImGuiWindowFlags_::ImGuiWindowFlags_None)) {
			ImGui::End();
			return;
		}

		ImGui::SetWindowSize(ImVec2(550, 500), ImGuiCond_::ImGuiCond_Once);
		ImGui::SeparatorText("GETTING STARTED");

		ImGui::TextWrapped("EasyDialogueEditor allows you to create interactive dialogue trees visually. Here's how to get started:");

		ImGui::Dummy(ImVec2(0.0f, 10.0f));
		ImGui::SeparatorText("NAVIGATING THE GRAPH EDITOR");

		ImGui::BulletText("Right-click and drag to pan around the node graph editor");
		ImGui::BulletText("Use the mouse wheel to zoom in and out of the node graph editor");
		ImGui::BulletText("The minimap in the bottom-right corner helps you navigate larger graphs");

		ImGui::Dummy(ImVec2(0.0f, 10.0f));
		ImGui::SeparatorText("CREATING & CONNECTING NODES");

		ImGui::BulletText("Drag from an output pin (right side) of a node and release to create a new node");
		ImGui::BulletText("Connect nodes by dragging from an output pin to an input pin (left side) of another node");
		ImGui::BulletText("Press Delete key to remove selected nodes (except root node)");

		ImGui::Dummy(ImVec2(0.0f, 10.0f));
		ImGui::SeparatorText("CALLBACK TAGS");

		ImGui::BulletText("Create callback tags in the sidebar to trigger events in your game");
		ImGui::BulletText("Assign callback tags to nodes to indicate that a game-event should happen when the conversation hits that node");
		ImGui::TextWrapped("Example: Adding a \"trigger_fight\" tag to a response node that insults a certain NPC");
		ImGui::BulletText("Delete callback tags by clicking the X button next to them");

		ImGui::Dummy(ImVec2(0.0f, 10.0f));
		ImGui::SeparatorText("SAVING & EXPORTING");

		ImGui::BulletText("Save your work in progress with Ctrl+S or File -> Save As");
		ImGui::TextWrapped("This saves the entire dialogue graph and editor state for later editing");
		ImGui::Dummy(ImVec2(0.0f, 5.0f));

		ImGui::BulletText("Export your finished dialogue with Ctrl+X or File -> Export Dialogue");
		ImGui::TextWrapped("This creates a streamlined JSON file containing only the dialogue data needed for your game");
		ImGui::Dummy(ImVec2(0.0f, 5.0f));

		ImGui::BulletText("Load a previously saved dialogue graph with Ctrl+O or File -> Load");

		ImGui::Dummy(ImVec2(0.0f, 10.0f));
		ImGui::SeparatorText("USING IN YOUR GAME");

		ImGui::TextWrapped("To use the exported dialogue in your game:");
		ImGui::Indent();
		ImGui::BulletText("Write a deserializer for the JSON format in your game engine");
		ImGui::BulletText("You can visit https://app.quicktype.io/ which helps by generating default code to parse any given JSON");
		ImGui::BulletText("In your dialogue system, check for callback tags and trigger your desired game events accordingly");
		ImGui::Unindent();

		ImGui::Dummy(ImVec2(0.0f, 10.0f));
		ImGui::SeparatorText("TIPS");

		ImGui::BulletText("The sidebar shows information about all nodes in your dialogue graph");
		ImGui::BulletText("Check the JSON preview in the sidebar to understand the export format");

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
			ImGui::Separator();
			if (ImGui::MenuItem("Load", "Ctrl+O")) {
				ede::FileDialogs::LoadStateJson();
			}
			if (ImGui::MenuItem("Save As", "Ctrl+S")) {
				ede::FileDialogs::SaveStateJson();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Export Dialogue", "Ctrl+X")) {
				ede::FileDialogs::ExportDialogueJsonFile();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Window")) {
			if (ImGui::MenuItem("Reset layout", "Ctrl+R")) {
				ImGui::LoadIniSettingsFromMemory(DEFAULT_INI);
				ImGui::MarkIniSettingsDirty();
			}
#ifdef _DEBUG
			if (ImGui::MenuItem("Toggle Demo Window"))
			{
				ede::ToggleDemoWindow();
			}
#endif
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help")) {
			if (ImGui::MenuItem("How to use")) {
				ede::ToggleHowToWindow();
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
		float raw_text_block_height = 35.0f;
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

		for (const std::shared_ptr<Node>& node : nodes) {
			if (node) {
				std::set<std::string>& current_callbacks = node->selected_callbacks;
				std::ostringstream stream;
				std::copy(current_callbacks.begin(), current_callbacks.end(), std::ostream_iterator<std::string>(stream, " "));
				std::string result_str = stream.str();
				if (node->nodeType == NodeType::Speech) {
					std::stringstream ss;
					ss << "{";
					for (int response : node->responses) {
						ss << " " << response << " ";
					}
					if (node->responses.empty())
						ss << " ";
					ss << "}";
					raw_info += std::format(R"([{{ "id": "{}" ; "type": "Speech" ; "next_node_id": "{}" ; "expected_responses": "{}" }} ; "callbacks": "{}")",
						node->id, node->nextNodeId, ss.str(), result_str);
					
					ImGui::Text(std::format("Node {}: {{\n  \"id\": \"{}\",\n  \"type\": \"Speech\",\n \"text\": \"{}\",\n  \"next_node_id\": \"{}\",\n  \"expected_responses\": \"{}\",\n \"callbacks\": \"{{ {}}}\"\n}}", node->id, node->id, node->text, node->nextNodeId, ss.str(), result_str).c_str());

				}
				if (node->nodeType == NodeType::Response) {

					raw_info += std::format(R"([{{ "id": "{}" ; "type": "Response" ; "next_node_id": "{}" ; "callbacks": "{}" }} ; )",
						node->id, node->nextNodeId, result_str);
					ImGui::Text(std::format("Node {}: {{\n  \"id\": \"{}\",\n  \"type\": \"Response\",\n  \"next_node_id\": \"{}\",\n \"callbacks\": \"{{ {}}}\"\n}}", node->id, node->id, node->nextNodeId, result_str).c_str());
				}
				ImGui::Dummy(ImVec2(0.0f, 2.0f));
			}
		}
		raw_info += "]";

		ImGui::EndChild();


		/*
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
		*/
		ImGui::Dummy(ImVec2(0.0f, 5.0f));

		ImGui::BeginChild("CallbackEvents", ImVec2(0, ImGui::GetContentRegionAvail().y - 80), true);
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

		// display current callback tags and delete buttons
		for (auto it = current_callbacks.begin(); it != current_callbacks.end();) {

			const auto& callback = *it;
			TEXT_BULLET(">", callback.c_str());
			ImGui::SameLine();
			std::string button_label = "X##" + callback;
			if (ImGui::Button(button_label.c_str())) {
				ede::NotifyCallbackDeletion(callback);
				it = current_callbacks.erase(it);
			}
			else {
				++it;
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

		ImGui::Dummy(ImVec2(0.0f, 5.0f));

		int current_window_width = ImGui::GetContentRegionAvail().x;
		if(ImGui::Button("Export Dialogue", ImVec2(current_window_width, 60))) {
			FileDialogs::ExportDialogueJsonFile();
		}

		ImGui::End();
	}

}