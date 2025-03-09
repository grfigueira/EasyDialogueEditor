/******************************************************************************
 * Copyright (c) 2025 Guilherme Figueira
 * MIT License - See LICENSE file in the project root for details
 * Contact: g.figueira.2002@gmail.com
 ******************************************************************************/

#include "Utils.h"
#include <windows.h>
#include <shobjidl.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>
#include "Node.h"
#include <node_editor.h>

namespace ede {

	/*std::string FileDialogs::OpenFile(const char* filter) {

	}*/

	void FileDialogs::SaveFile(const json& j) {
		IFileSaveDialog* pFileSave;
		HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileSave));

		if (SUCCEEDED(hr)) {
			// Set default file extension
			COMDLG_FILTERSPEC fileTypes[] = { {L"JSON Files", L"*.json"} };
			pFileSave->SetFileTypes(1, fileTypes);
			pFileSave->SetDefaultExtension(L"json");

			// Show dialog
			hr = pFileSave->Show(NULL);
			if (SUCCEEDED(hr)) {
				// Get the result
				IShellItem* pItem;
				hr = pFileSave->GetResult(&pItem);
				if (SUCCEEDED(hr)) {
					PWSTR filePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
					if (SUCCEEDED(hr)) {
						// Convert PWSTR to std::wstring
						std::wstring ws(filePath);
						std::string fileName(ws.begin(), ws.end());

						// Write JSON to file
						std::ofstream outFile(fileName);
						if (outFile.is_open()) {
							outFile << j.dump(4); // Pretty-print JSON
							outFile.close();
						}

						CoTaskMemFree(filePath);
					}
					pItem->Release();
				}
			}
			pFileSave->Release();
		}

	}

	void FileDialogs::ExportDialogueJsonFile()
	{
		std::vector<Node> nodes = ede::GetNodesData();
		json j;
		for (const Node& node : nodes) {
			j.push_back(node);
		}
		std::cout << j << std::endl;
		SaveFile(j);
	}

	// Converts state to json
	void FileDialogs::SaveStateJson() {

		State state = ede::GetCurrentState();
		
		json nodes;
		for (const auto& pair : state.nodes) {
			std::shared_ptr<Node> node = pair.second;
			if (node) {
				nodes.push_back({
					{"nodeId", node->id},
					{"nodeType", static_cast<int>(node->nodeType)},
					{"text", node->text},
					{"position", {{"x", node->position.x}, {"y", node->position.y}}},
					{"nextNodeId", node->nextNodeId},
					{"prevNodeId", node->prevNodeId},
					{"responses", node->responses},
					{"expectsResponse", node->expectesResponse},
					{"selected_callbacks", node->selected_callbacks}
					});
			}
		}

		json links;
		for (const auto& pair : state.links) {
			std::shared_ptr<Link> link = pair.second;
			if (link) {
				links.push_back(*link);
			}
		}

		json j = json{
			{"nodes", nodes},
			{"links", links},
			{"next_node_id", state.next_node_id},
			{"next_link_id", state.next_link_id},
			{"callbacks", state.callbacks},
			/* TODO: place 'conditionals' here, once its implemented */
		};
		SaveFile(j);
	}

	void FileDialogs::LoadStateJson()
	{
		std::cout << "Not yet implemented\n";
	}
}