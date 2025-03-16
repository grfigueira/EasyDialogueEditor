/******************************************************************************
	Created by Guilherme Figueira, 2025

	My contacts (feel free to reach out):
	- Github: https://github.com/grfigueira
	- LinkedIn: https://www.linkedin.com/in/grfigueira/
 ******************************************************************************/

#pragma once

#include <string>
#include <nlohmann/json_fwd.hpp>

using json = nlohmann::json;

static const char* DEFAULT_INI = R"INI(
[Window][MainDockspace]
Pos=0,0
Size=1440,900
Collapsed=0

[Window][Debug##Default]
Pos=60,60
Size=400,400
Collapsed=0

[Window][Graph Editor]
Pos=0,0
Size=1057,900
Collapsed=0
DockId=0x00000001,0

[Window][Story Graph Info]
Pos=1059,0
Size=381,900
Collapsed=0
DockId=0x00000002,0

[Window][Dear ImGui Demo]
Pos=425,72
Size=550,680
Collapsed=0

[Docking][Data]
DockSpace     ID=0x5F4274ED Window=0x1F1D7494 Pos=0,0 Size=1440,900 Split=X
  DockNode    ID=0x00000003 Parent=0x5F4274ED SizeRef=703,900 Selected=0xEEAF2C9D
  DockNode    ID=0x00000004 Parent=0x5F4274ED SizeRef=735,900 Split=X
    DockNode  ID=0x00000001 Parent=0x00000004 SizeRef=1057,900 CentralNode=1 Selected=0xEEAF2C9D
    DockNode  ID=0x00000002 Parent=0x00000004 SizeRef=381,900 Selected=0xBCDEDAD9
)INI";

namespace ede {
	class FileDialogs 
	{
	public:
		static void SaveFile(const json& j, const wchar_t* title = L"Save File");
		static json LoadFile(const wchar_t* title = L"Open File");
		static void ExportDialogueJsonFile();
		static void SaveStateJson();
		static void LoadStateJson();
	};

}
