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
		static void ExportJsonFile();

	};
}
