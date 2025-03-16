/******************************************************************************
	Created by Guilherme Figueira, 2025

	My contacts (feel free to reach out):
	- Github: https://github.com/grfigueira
	- LinkedIn: https://www.linkedin.com/in/grfigueira/
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

	std::shared_ptr<Node> node_from_json(const json& j);
	std::shared_ptr<Link> link_from_json(const json& j);

	void FileDialogs::SaveFile(const json& j, const wchar_t* title) {
		IFileSaveDialog* pFileSave;
		HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileSave));

		if (SUCCEEDED(hr)) {

			pFileSave->SetTitle(title);

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

	json FileDialogs::LoadFile(const wchar_t* title) {
		json j; // Default empty JSON
		IFileOpenDialog* pFileOpen;
		HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileOpen));

		if (SUCCEEDED(hr)) {

			pFileOpen->SetTitle(title);

			// Set file type filters
			COMDLG_FILTERSPEC fileTypes[] = { {L"JSON Files", L"*.json"} };
			pFileOpen->SetFileTypes(1, fileTypes);
			pFileOpen->SetDefaultExtension(L"json");

			// Show dialog
			hr = pFileOpen->Show(NULL);
			if (SUCCEEDED(hr)) {
				// Get the result
				IShellItem* pItem;
				hr = pFileOpen->GetResult(&pItem);
				if (SUCCEEDED(hr)) {
					PWSTR filePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
					if (SUCCEEDED(hr)) {
						// Convert PWSTR to std::wstring
						std::wstring ws(filePath);
						std::string fileName(ws.begin(), ws.end());

						// Read JSON from file
						std::ifstream inFile(fileName);
						if (inFile.is_open()) {
							try {
								inFile >> j; // Parse JSON directly from the file stream
								inFile.close();
							}
							catch (const json::exception& e) {
								std::cerr << "Error parsing JSON: " << e.what() << std::endl;
							}
						}
						else {
							std::cerr << "Could not open file: " << fileName << std::endl;
						}

						CoTaskMemFree(filePath);
					}
					pItem->Release();
				}
			}
			pFileOpen->Release();
		}

		return j;
	}

	void FileDialogs::ExportDialogueJsonFile()
	{
		std::vector<Node> nodes = ede::GetNodesData();
		json j;
		for (const Node& node : nodes) {
			j.push_back(node);
		}
		std::cout << j << std::endl;
		SaveFile(j, L"Export Dialogue");
		ede::RequestNotification("Success", "Your dialogue was successfully exported!");
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
					{"position", {{"x", ImNodes::GetNodeScreenSpacePos(node->id).x}, {"y", ImNodes::GetNodeScreenSpacePos(node->id).y}}},
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
		SaveFile(j, L"Save Current State");
	}

	void FileDialogs::LoadStateJson()
	{
		json j = LoadFile(L"Load a previous state");
		if (!j.empty()) {

			if (!j.contains("nodes") || !j.contains("links") ||
				!j.contains("next_node_id") || !j.contains("next_link_id") ||
				!j.contains("callbacks"))
			{
				std::cout << "Invalid JSON\n";
				ede::RequestNotification("Invalid JSON", "Could not parse the data from the file. \nMaybe you chose the wrong file or it's corrupted.");
				return;
			}
			State new_state;

			// Load nodes
			std::vector<json> nodes_json = j.at("nodes").get<std::vector<json>>();
			for (const json& jn : nodes_json) {
				std::shared_ptr<Node> node = node_from_json(jn);
				new_state.nodes[node->id] = node;
			}

			// Load links
			std::vector<json> links_json = j.at("links").get<std::vector<json>>();
			for (const json& jl : links_json) {
				std::shared_ptr<Link> link = link_from_json(jl);
				new_state.links[link->id] = link;
			}

			new_state.next_node_id = j.at("next_node_id").get<int>();

			new_state.next_link_id = j.at("next_link_id").get<int>();

			new_state.callbacks = j.at("callbacks").get<std::set<std::string>>();

			ede::SetState(new_state);
		}

	}

	// --- helper methods ---

	std::shared_ptr<Node> node_from_json(const json& j) {

		int nodeId = j.at("nodeId").get<int>();
		NodeType nodeType = static_cast<NodeType>(j.at("nodeType").get<int>());
		std::string text = j.at("text").get<std::string>();
		ImVec2 position(j.at("position").at("x").get<float>(), j.at("position").at("y").get<float>());
		int nextNodeId = j.at("nextNodeId").get<int>();
		int prevNodeId = j.at("prevNodeId").get<int>();
		std::vector<int> responses = j.at("responses").get<std::vector<int>>();
		bool expectsResponse = j.at("expectsResponse").get<bool>();

		std::set<std::string> selected_callbacks;
		for (auto& callback : j.at("selected_callbacks").get<std::vector<std::string>>()) {
			selected_callbacks.insert(callback);
		}

		return std::make_shared<Node>(nodeId, nodeType, text, position, nextNodeId, prevNodeId,
			responses, expectsResponse, selected_callbacks);
	}

	std::shared_ptr<Link> link_from_json(const json& j) {
		int id = j.at("id").get<int>();
		int start_attr = j.at("start_attr").get<int>();
		int end_attr = j.at("end_attr").get<int>();

		return std::make_shared<Link>(id, start_attr, end_attr);
	}


}