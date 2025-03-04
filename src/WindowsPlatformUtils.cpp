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

	void FileDialogs::ExportJsonFile()
	{
		std::vector<Node> nodes = ede::GetNodesData();
		json j;
		for (const Node& node : nodes) {
			j.push_back(node);
		}
		std::cout << j << std::endl;
		FileDialogs::SaveFile(j);
	}

}