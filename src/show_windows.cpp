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

namespace ede {

	void Markdown(const std::string& markdown_); // forward declared

	void MarkdownAboutSection()
	{
		const char* markdownText = (const char*)u8R"(
**EasyDialogEditor** is a lightweight dialog tree editor for games, designed to help you visually create dialog flows and export them as JSON. While it's _not_ a dialog system itself, it perfectly complements any existing system if you parse the generated data on your end. The result is a portable solution for building branching dialogues.

**Features**
- Lightweight and portable, no installation required.
- Visual creation of dialog trees
- Straightforward JSON export for easy integration
- Free and open-source
# Credits
Created by [Guilherme Figueira](https://grfigueira.github.io/MyPortfolio/)
GitHub repository: [github.com/grfigueira/EasyDialogEditor](https://github.com/grfigueira/EasyDialogEditor)
## Built With
- [Dear ImGui](https://github.com/ocornut/imgui)
- [ImNodes](https://github.com/Nelarius/imnodes)
- [SDL2](https://www.libsdl.org/))";
		Markdown(markdownText);
	}

	void ShowAboutWindow(bool* p_open) {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
		if (!ImGui::Begin("About", p_open, ImGuiWindowFlags_::ImGuiWindowFlags_None)) {
			ImGui::End();
			return;
		}
		ImGui::SetWindowSize(ImVec2(550, 400), ImGuiCond_::ImGuiCond_Once);
		MarkdownAboutSection();
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
					ede::FileDialogs::ExportJsonFile();
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
		float raw_text_block_height = 50.0f;
		ImGui::Begin("Story Graph Info");
		auto nodes = ede::GetNodesVec();

		ImGui::Text("Total number of nodes: %d", nodes.size());
		ImGui::Text("Number of Speech nodes: %d", ede::GetNumNodesOfType(NodeType::Speech));
		ImGui::Text("Number of Response nodes: %d", ede::GetNumNodesOfType(NodeType::Response));

		ImGui::Dummy(ImVec2(0.0f, 5.0f));

		int current_window_height = ImGui::GetContentRegionAvail().y - raw_text_block_height;

		ImGui::BeginChild("Node List", ImVec2(0, current_window_height), true, ImGuiWindowFlags_::ImGuiWindowFlags_HorizontalScrollbar |
			ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysVerticalScrollbar);

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

	inline ImGui::MarkdownImageData ImageCallback(ImGui::MarkdownLinkCallbackData data_)
	{
		// In your application you would load an image based on data_ input. Here we just use the imgui font texture.
		ImTextureID image = ImGui::GetIO().Fonts->TexID;
		// > C++14 can use ImGui::MarkdownImageData imageData{ true, false, image, ImVec2( 40.0f, 20.0f ) };
		ImGui::MarkdownImageData imageData;
		imageData.isValid = true;
		imageData.useLinkCallback = false;
		imageData.user_texture_id = image;
		imageData.size = ImVec2(40.0f, 20.0f);

		// For image resize when available size.x > image width, add
		ImVec2 const contentSize = ImGui::GetContentRegionAvail();
		if (imageData.size.x > contentSize.x)
		{
			float const ratio = imageData.size.y / imageData.size.x;
			imageData.size.x = contentSize.x;
			imageData.size.y = contentSize.x * ratio;
		}

		return imageData;
	}

	void LoadFonts(float fontSize_)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->Clear();
		// Base font
		io.Fonts->AddFontFromFileTTF("C:\\Users\\gfigu\\AppData\\Local\\Microsoft\\Windows\\Fonts\\Roboto-Medium.ttf", fontSize_);
		// Bold headings H2 and H3
		H2 = io.Fonts->AddFontFromFileTTF("C:\\Users\\gfigu\\AppData\\Local\\Microsoft\\Windows\\Fonts\\Roboto-Bold.ttf", fontSize_);
		H3 = mdConfig.headingFormats[1].font;
		// bold heading H1
		float fontSizeH1 = fontSize_ * 1.4f;
		H1 = io.Fonts->AddFontFromFileTTF("C:\\Users\\gfigu\\AppData\\Local\\Microsoft\\Windows\\Fonts\\Roboto-Bold.ttf", fontSizeH1);
	}

	void ExampleMarkdownFormatCallback(const ImGui::MarkdownFormatInfo& markdownFormatInfo_, bool start_)
	{
		// Call the default first so any settings can be overwritten by our implementation.
		// Alternatively could be called or not called in a switch statement on a case by case basis.
		// See defaultMarkdownFormatCallback definition for furhter examples of how to use it.
		ImGui::defaultMarkdownFormatCallback(markdownFormatInfo_, start_);

		switch (markdownFormatInfo_.type)
		{
			// example: change the colour of heading level 2
		case ImGui::MarkdownFormatType::HEADING:
		{
			if (markdownFormatInfo_.level == 2)
			{
				if (start_)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
				}
				else
				{
					ImGui::PopStyleColor();
				}
			}
			break;
		}
		default:
		{
			break;
		}
		}
	}

	void Markdown(const std::string& markdown_)
	{
		// You can make your own Markdown function with your prefered string container and markdown config.
		// > C++14 can use ImGui::MarkdownConfig mdConfig{ LinkCallback, NULL, ImageCallback, ICON_FA_LINK, { { H1, true }, { H2, true }, { H3, false } }, NULL };
		mdConfig.linkCallback = LinkCallback;
		mdConfig.tooltipCallback = NULL;
		mdConfig.imageCallback = ImageCallback;
		mdConfig.linkIcon = "";
		mdConfig.headingFormats[0] = { H1, true };
		mdConfig.headingFormats[1] = { H2, true };
		mdConfig.headingFormats[2] = { H3, false };
		mdConfig.userData = NULL;
		mdConfig.formatCallback = ExampleMarkdownFormatCallback;
		ImGui::Markdown(markdown_.c_str(), markdown_.length(), mdConfig);
	}

}