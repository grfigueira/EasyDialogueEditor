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

#include <string>
#include <format>

namespace otherwindows {

    void Markdown(const std::string& markdown_); // forward declared

	void MarkdownAboutSection()
	{
		const char* markdownText = (const char*)u8R"(
# H1 Header: Text and Links
You can add [links like this one to enkisoftware](https://www.enkisoftware.com/) and lines will wrap well.
You can also insert images ![image alt text](image identifier e.g. filename)
Horizontal rules:
***
___
*Emphasis* and **strong emphasis** change the appearance of the text.
## H2 Header: indented text.
  This text has an indent (two leading spaces).
    This one has two.
### H3 Header: Lists
  * Unordered lists
    * Lists can be indented with two extra spaces.
  * Lists can have [links like this one to Avoyd](https://www.avoyd.com/) and *emphasized text*)";
		Markdown(markdownText);
	}

	void ShowAboutWindow(bool* p_open) {
		if (!ImGui::Begin("About", p_open, ImGuiWindowFlags_::ImGuiWindowFlags_None)) {
            ImGui::End();
            return;
        }
		ImGui::SetWindowSize(ImVec2(550, 400), ImGuiCond_::ImGuiCond_Once);
		MarkdownAboutSection();
		//ImGui::Markdown(reinterpret_cast<const char*>(markdownText));
		//ImGui::TextWrapped("EasyDialogEditor is an open-source dialog tree editor. It is written in C++ for Windows using ImGui and ImNodes.");
		ImGui::End();
	}

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
				ede::ToggleDemoWindow();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help")) {
			if (ImGui::MenuItem("How to use")) {
				
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

    /*******************************************************
    *                   Mardown methods
    *             (taken from imgui_markdown)
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
        io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Roboto-Medium.ttf", fontSize_);
        // Bold headings H2 and H3
        H2 = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Roboto-Medium.ttf", fontSize_);
        H3 = mdConfig.headingFormats[1].font;
        // bold heading H1
        float fontSizeH1 = fontSize_ * 2.f;
        H1 = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Roboto-Medium.ttf", fontSizeH1);
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