/******************************************************************************
 * Copyright (c) 2025 Guilherme Figueira
 * MIT License - See LICENSE file in the project root for details
 * Contact: g.figueira.2002@gmail.com
 ******************************************************************************/

#pragma once

#include <string>
#include <nlohmann/json_fwd.hpp>

using json = nlohmann::json;

namespace ede {
	class FileDialogs 
	{
	public:
		//static std::string OpenFile(const char* filter);
		static void SaveFile(const json& j);
		static void ExportDialogueJsonFile();
		static void ExportStateJsonFile();

	};
}
