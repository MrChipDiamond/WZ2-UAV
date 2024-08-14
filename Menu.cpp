#include "stdafx.h"
#include "Menu.h"
#include "imgui/imgui.h"
# include "globals.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"
#include "obfuscator.hpp"
//#include "xor.hpp"
#include"memory.h"
#include "style.h"
#include "utility.h"
#include "sdk.h"
#include "xorstr.hpp"
int i_MenuTab = 0;


#define INRANGE(x,a,b)    (x >= a && x <= b) 
#define getBits( x )    (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))
#define getByte( x )    (getBits(x[0]) << 4 | getBits(x[1]))
bool b_menu_open = true;
bool b_debug_open = false;
bool boxcheck;


using offsets = std::vector<uint64_t>;

void MISC()
{
	ImGui::Dummy(ImVec2(0.0f, 3.0f));
	ImGui::Checkbox(xorstr_("UAV"), &globals::b_UAV);
	

}

bool init_once = true;
namespace g_menu
{
	void menu()
	{
		if (GetAsyncKeyState(VK_INSERT) & 0x1)
		{
			b_menu_open = !b_menu_open;
			
		}
		if (init_once)
		{
			
			ImGui::SetNextWindowPos(ImVec2(200, 200));
			init_once = false;
		}
		EditorColorScheme::ApplyTheme();
		if (b_menu_open && screenshot::visuals)
		{
			EditorColorScheme::ApplyTheme();

			ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_Always);

			ImGui::Begin(xorstr_(" MW II UAV"), &b_menu_open, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize);
			

			int dwWidth = GetSystemMetrics(SM_CXSCREEN) / 3;
			int dwHeight = GetSystemMetrics(SM_CYSCREEN) / 2;
			ImGui::Dummy(ImVec2(0.0f, 1.0f));
			for (int i = 0; i < 25; i++)
			{
				ImGui::Spacing();
				ImGui::SameLine();
			}
			//ImGui::Checkbox(xorstr_("Enable Cheat"), &globals::b_cheat);

			ImGui::Dummy(ImVec2(0.0f, 3.0f));
			ImGui::SetWindowPos(ImVec2(dwWidth * 2.0f, dwHeight * 0.2f), ImGuiCond_Once);
			{
				

				ImGui::BeginChild(xorstr_("##FEATURESCHILD"), ImVec2(-1, -1), false);
				{
					
					MISC();
				}
				ImGui::EndChild();
			}

		}
	}
}