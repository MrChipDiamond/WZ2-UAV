#include "stdafx.h"
#include "sdk.h"
#include "xor.hpp"
#include "lazyimporter.h"
#include "memory.h"
#include <map>
#include "defs.h"
#include "globals.h"
#include "weapon.h"
//#include "utility.h"

namespace process
{
	HWND hwnd;

	BOOL CALLBACK EnumWindowCallBack(HWND hWnd, LPARAM lParam)
	{
		DWORD dwPid = 0;
		GetWindowThreadProcessId(hWnd, &dwPid);
		if (dwPid == lParam)
		{
			hwnd = hWnd;
			return FALSE;
		}
		return TRUE;
	}

	HWND get_process_window()
	{
		if (hwnd)
			return hwnd;

		EnumWindows(EnumWindowCallBack, GetCurrentProcessId());

		if (hwnd == NULL)
			Exit();

		return hwnd;
	}
}
DWORD GetProcId(const char* procName)
{
	DWORD procId = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 procEntry;
		procEntry.dwSize = sizeof(procEntry);

		if (Process32First(hSnap, &procEntry))
		{
			do
			{
				if (!_stricmp(procEntry.szExeFile, procName))
				{
					procId = procEntry.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnap, &procEntry));
		}
	}
	CloseHandle(hSnap);
	return procId;
}

namespace g_data
{
	uintptr_t base, baseModule;
	//bool outlines_bones = true;
	uintptr_t Peb;
	HWND hWind;
	uintptr_t visible_base;
	HANDLE hProc;
	DWORD procID, bnet, steam;
	void init()
	{
		base = (uintptr_t)(iat(GetModuleHandleA).get()("cod.exe"));
		baseModule = (QWORD)(iat(GetModuleHandleA).get()("cod.exe"));
		//procID = GetProcId("cod.exe");
		/*steam = GetProcId("steam.exe");
		if (steam)
		{
			globals::uavbase = 0x0C698240;
		}
		else
			globals::uavbase = 0x0C939640;*/

		hWind = process::get_process_window();
		//hProc = OpenProcess(PROCESS_ALL_ACCESS, NULL, procID);
		Peb = __readgsqword(0x60);
	

	}
}

template<typename T> inline auto readMemory(uintptr_t ptr) noexcept -> T {
	if (is_bad_ptr(ptr)) {
		//DEBUG_INFO("Attempted to read invalid memory at {:#x}", ptr);
		return {};
	}
	return *reinterpret_cast<T*>(ptr);
}
template<typename T> inline auto writeMemory(uintptr_t ptr, T value) noexcept -> T {
	if (is_bad_ptr(ptr)) {
		//DEBUG_INFO("Attempted to read invalid memory at {:#x}", ptr);
		return {};
	}
	return *reinterpret_cast<T*>(ptr) = value;
}
dvar_s* Dvar_FindVarByName(const char* dvarName)
{
	//[48 83 EC 48 49 8B C8 E8 ?? ?? ?? ?? + 0x7] resolve call.
	return reinterpret_cast<dvar_s * (__fastcall*)(const char* dvarName)>(g_data::base + globals::Dvar_FindVarByName)(dvarName);
}

uintptr_t Dvar_SetBool_Internal(dvar_s* a1, bool a2)
{
	//E8 ? ? ? ? 80 3D ? ? ? ? ? 4C 8D 35 ? ? ? ? 74 43 33 D2 F7 05 ? ? ? ? ? ? ? ? 76 2E
	return reinterpret_cast<std::ptrdiff_t(__fastcall*)(dvar_s * a1, bool a2)>(g_data::base + globals::Dvar_SetBoolInternal)(a1, a2);
}

uintptr_t Dvar_SetInt_Internal(dvar_s* a1, unsigned int a2)
{
	//40 53 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 0F B6 41 09 48 8B D9
	return reinterpret_cast<std::ptrdiff_t(__fastcall*)(dvar_s * a1, unsigned int a2)>(g_data::base + globals::Dvar_SetInt_Internal)(a1, a2);
}

uintptr_t Dvar_SetBoolByName(const char* dvarName, bool value)
{
	//"48 89 ? ? ? 57 48 81 EC ? ? ? ? 0F B6 ? 48 8B"
	int64(__fastcall * Dvar_SetBoolByName_t)(const char* dvarName, bool value); //48 89 5C 24 ? 57 48 81 EC ? ? ? ? 0F B6 DA
	return reinterpret_cast<decltype(Dvar_SetBoolByName_t)>(globals::Dvar_SetBoolByName)(dvarName, value);
}

uintptr_t Dvar_SetFloat_Internal(dvar_s* a1, float a2)
{
	//E8 ? ? ? ? 45 0F 2E C8 RESOLVE CALL
	return reinterpret_cast<std::ptrdiff_t(__fastcall*)(dvar_s * a1, float a2)>(g_data::base + globals::Dvar_SetFloat_Internal)(a1, a2);
}

uintptr_t Dvar_SetIntByName(const char* dvarname, int value)
{
	uintptr_t(__fastcall * Dvar_SetIntByName_t)(const char* dvarname, int value); //48 89 5C 24 ? 48 89 74 24 ? 57 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 8B DA 48 8B F9
	return reinterpret_cast<decltype(Dvar_SetIntByName_t)>(globals::Dvar_SetIntByName)(dvarname, value);
}


__int64 Com_DDL_LoadAsset(__int64 file) {
	uintptr_t address = g_data::baseModule + globals::ddl_loadasset;
	return ((__int64 (*)(__int64))address)(file);
}

__int64 DDL_GetRootState(__int64 state, __int64 file) {
	uintptr_t address = g_data::baseModule + globals::ddl_getrootstate;
	return ((__int64 (*)(__int64, __int64))address)(state, file);
}

bool CL_PlayerData_GetDDLBuffer(__int64 context, int controllerindex, int stats_source, unsigned int statsgroup) {
	uintptr_t address = g_data::baseModule + globals::ddl_getdllbuffer;
	return ((bool (*)(__int64, int, int, unsigned int))address)(context, controllerindex, stats_source, statsgroup);
}
bool ParseShit(const char* a, const char** b, const int c, int* d)
{
	uintptr_t address = g_data::baseModule + globals::a_parse;
	return ((bool (*)(const char* a, const char** b, const int c, int* d))address)(a, b, c, d);

}
char DDL_MoveToPath(__int64* fromState, __int64* toState, int depth, const char** path) {
	uintptr_t address = g_data::baseModule + globals::ddl_movetopath;
	return ((char (*)(__int64* fromState, __int64* toState, int depth, const char** path))address)(fromState, toState, depth, path);

}
__int64 DDL_MoveToName(__int64 fstate, __int64 tstate, __int64 path) {
	uintptr_t address = g_data::baseModule + globals::ddl_movetoname;
	return ((__int64 (*)(__int64, __int64, __int64))address)(fstate, tstate, path);
}

char DDL_SetInt(__int64 fstate, __int64 context, unsigned int value) {
	uintptr_t address = g_data::baseModule + globals::ddl_setint;
	return ((char (*)(__int64, __int64, unsigned int))address)(fstate, context, value);
}
int DDL_GetInt(__int64* fstate, __int64* context) {
	uintptr_t address = g_data::baseModule + globals::ddl_getint;
	return ((int (*)(__int64*, __int64*))address)(fstate, context);
}
char DDL_SetString(__int64 fstate, __int64 context, const char* value) {
	uintptr_t address = g_data::baseModule + globals::ddl_setstring;
	return ((char (*)(__int64, __int64, const char*))address)(fstate, context, value);
}
char DDL_SetInt2(__int64* fstate, __int64* context, int value) {
	uintptr_t address = g_data::baseModule + globals::ddl_setint;
	return ((char (*)(__int64*, __int64*, unsigned int))address)(fstate, context, value);
}
void ShowToastNotificationAfterUserJoinedParty(const char* message)
{
	uintptr_t address = g_data::baseModule + globals::ShowToastNotificationAfterUserJoinedParty; // CC 48 89 74 24 ? 57 48 83 EC 20 4C 8B 05 ? ? ? ? 33 + 1 
	((void(*)(int, int, int, const char*, int))address)(0, 0, 0, message, 0);
}

namespace sdk
{
	const DWORD nTickTime = 1;//64 ms
	bool bUpdateTick = false;
	std::map<DWORD, velocityInfo_t> velocityMap;
	uint64_t current_visible_offset;
	uint64_t last_visible_offset;
	uint64_t cached_visible_base;
	uintptr_t uavsteam = 0x12BD7A70 + 0x130;
	uintptr_t uavbnet = 0x12E9F340 + 0x130;
	void enable_uav()
	{

		const auto uav = 0x418;
		const auto auav = 0x306 - 0x5;
		auto uavptr = *(uint64_t*)(g_data::base + uavbnet);
		if (uavptr != 0)
		{
			
			*(BYTE*)(uavptr + 0x430) = 2;
			
		}
		
	}
	void unlockallnew()
	{
		uintptr_t codeCaveAddress = (g_data::base + 0x130A0010);
		uintptr_t codeCaveAddressAdjusted = (codeCaveAddress + 0x60);

		uintptr_t originalBytesAddress = (uintptr_t)(g_data::base + globals::unlockallnew);
		memcpy((BYTE*)codeCaveAddress, (BYTE*)originalBytesAddress, 96);

		uintptr_t* LEAPointer = &codeCaveAddressAdjusted;
		memcpy((BYTE*)(codeCaveAddress + 0x8), (BYTE*)LEAPointer, 8);

		memcpy((BYTE*)codeCaveAddress + 0x0F, (BYTE*)originalBytesAddress + 0x0F, 12);

		*(uintptr_t*)(codeCaveAddressAdjusted) = { 0x48 };
		*(uintptr_t*)(codeCaveAddressAdjusted + 0x01) = { 0x83 };
		*(uintptr_t*)(codeCaveAddressAdjusted + 0x02) = { 0xC4 };
		*(uintptr_t*)(codeCaveAddressAdjusted + 0x03) = { 0x08 };
		*(uintptr_t*)(codeCaveAddressAdjusted + 0x04) = { 0x48 };
		*(uintptr_t*)(codeCaveAddressAdjusted + 0x05) = { 0x8B };
		*(uintptr_t*)(codeCaveAddressAdjusted + 0x06) = { 0x5C };

		*(uintptr_t*)(codeCaveAddressAdjusted + 0x07) = { 0x24 };
		*(uintptr_t*)(codeCaveAddressAdjusted + 0x08) = { 0x30 };
		*(uintptr_t*)(codeCaveAddressAdjusted + 0x09) = { 0x48 };
		*(uintptr_t*)(codeCaveAddressAdjusted + 0x0A) = { 0x8B };
		*(uintptr_t*)(codeCaveAddressAdjusted + 0x0B) = { 0x74 };
		*(uintptr_t*)(codeCaveAddressAdjusted + 0x0C) = { 0x24 };
		*(uintptr_t*)(codeCaveAddressAdjusted + 0x0D) = { 0x38 };

		*(uintptr_t*)(codeCaveAddressAdjusted + 0x0E) = { 0x48 };
		*(uintptr_t*)(codeCaveAddressAdjusted + 0x0F) = { 0x83 };
		*(uintptr_t*)(codeCaveAddressAdjusted + 0x10) = { 0xC4 };
		*(uintptr_t*)(codeCaveAddressAdjusted + 0x11) = { 0x20 };
		*(uintptr_t*)(codeCaveAddressAdjusted + 0x12) = { 0x5F };
		*(uintptr_t*)(codeCaveAddressAdjusted + 0x13) = { 0x48 };
		*(uintptr_t*)(codeCaveAddressAdjusted + 0x14) = { 0xC7 };

		*(uintptr_t*)(codeCaveAddressAdjusted + 0x15) = { 0xC0 };
		*(uintptr_t*)(codeCaveAddressAdjusted + 0x16) = { 0x01 };
		*(uintptr_t*)(codeCaveAddressAdjusted + 0x17) = { 0x00 };
		*(uintptr_t*)(codeCaveAddressAdjusted + 0x18) = { 0x00 };
		*(uintptr_t*)(codeCaveAddressAdjusted + 0x19) = { 0x00 };
		*(uintptr_t*)(codeCaveAddressAdjusted + 0x1A) = { 0xC3 };

		*(uintptr_t*)(g_data::base + globals::unlockallnew) = codeCaveAddress;
	}	
	void testgr()
	{
		uintptr_t num = (g_data::base + globals::unlockall + 0xC);
		int num2 = readMemory<int>(num);

		uintptr_t unlock_base = num + num2 + 4 - g_data::base;
		uintptr_t numP = (g_data::base + unlock_base + 0x60);

		memcpy((BYTE*)numP, (BYTE*)"\x48\x83\xC4\x08\x48\x8B\x5C\x24\x30\x48\x8B\x74\x24\x38\x48\x83\xC4\x20\x5F\x48\xC7\xC0\x01\x00\x00\x00\xC3", 28);
		writeMemory<uintptr_t>(g_data::base + unlock_base + 8, numP);
		writeMemory<uintptr_t>(g_data::base + unlock_base, g_data::base + unlock_base);
	}
	uintptr_t _get_player(int i)
	{
		auto cl_info_base = get_client_info_base();

		if (is_bad_ptr(cl_info_base))return 0;
		
		
			auto base_address = *(uintptr_t*)(cl_info_base);
			if (is_bad_ptr(base_address))return 0;

				return sdk::get_client_info_base() + (i * player_info::size);

	}
	bool in_game()
	{
		auto gameMode = *(int*)(g_data::base + game_mode);
		return  gameMode > 4;
	}

	int get_game_mode()
	{
		return *(int*)(g_data::base + game_mode + 0x4);
	}

	/*int GetCurrentPlayerCount()
	{
		return *(int*)(g_data::base + Lobby::currentPlayerCountPtr);
	}*/
	int get_current_player_count()
	{
		return *(int*)(g_data::base + Lobby::currentPlayerCountPtr);
	}
	int get_max_player_count()
	{
		return *(int*)(g_data::base + game_mode);
	}
	Vector3 _get_pos(uintptr_t address)
	{
		auto local_pos_ptr = *(uintptr_t*)((uintptr_t)address + player_info::position_ptr);

		if (local_pos_ptr)
		{
			return *(Vector3*)(local_pos_ptr + 0x40);
		}
		return Vector3{};
	}

	uint32_t _get_index(uintptr_t address)
	{
		auto cl_info_base = get_client_info_base();

		if (is_bad_ptr(cl_info_base))return 0;

		return ((uintptr_t)address - cl_info_base) / player_info::size;
	
		
	}

	int _team_id(uintptr_t address)    {

		return *(int*)((uintptr_t)address + player_info::team_id);
	}

	int decrypt_visible_flag(int i, QWORD valid_list)
	{
		auto ptr = valid_list + ((i + i * 8) * 8) + 0xA83; //80 BF ? ? ? ? ? 74 20 80 BF ? ? ? ? ? 74 17
		DWORD dw1 = (*(DWORD*)(ptr + 4) ^ (DWORD)ptr);
		DWORD dw2 = ((dw1 + 2) * dw1);
		BYTE dec_visible_flag = *(BYTE*)(ptr) ^ BYTE1(dw2) ^ (BYTE)dw2;

		return (int)dec_visible_flag;
	}


	bool _is_visible(uintptr_t address)
	{
		if (IsValidPtr<uintptr_t>(&g_data::visible_base))
		{
			uint64_t VisibleList = *(uint64_t*)(g_data::visible_base + 0x108);
			if (is_bad_ptr( VisibleList))
				return false;

			uint64_t rdx = VisibleList + (_get_index(address) * 9 + 0x14E) * 8;
			if (is_bad_ptr(rdx))
				return false;

			DWORD VisibleFlags = (rdx + 0x10) ^ (*(DWORD*)(rdx + 0x14));
			if (is_bad_ptr(VisibleFlags))
				return false;

			DWORD v511 = VisibleFlags * (VisibleFlags + 2);
			if (!v511)
				return false;

			BYTE VisibleFlags1 = *(DWORD*)(rdx + 0x10) ^ v511 ^ BYTE1(v511);
			if (VisibleFlags1 == 3) {
				return true;
			}
		}
		return false;
	}





	uint64_t get_client_info()
	{
		//auto baseModuleAddr = g_data::base;
		//auto Peb = __readgsqword(0x60);

		//uint64_t rax = baseModuleAddr, rbx = baseModuleAddr, rcx = baseModuleAddr, rdx = baseModuleAddr, rdi = baseModuleAddr, rsi = baseModuleAddr, r8 = baseModuleAddr, r9 = baseModuleAddr, r10 = baseModuleAddr, r11 = baseModuleAddr, r12 = baseModuleAddr, r13 = baseModuleAddr, r14 = baseModuleAddr, r15 = baseModuleAddr;
		//rbx =  *(uintptr_t*)(baseModuleAddr + 0x11C27F00);
		//if (!rbx)
		//	return rbx;
		//rdx = ~Peb;              //mov rdx, gs:[rax]
		//rax = rbx;              //mov rax, rbx
		//rax >>= 0x22;           //shr rax, 0x22
		//rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//rbx ^= rax;             //xor rbx, rax
		//rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//rcx ^=  *(uintptr_t*)(baseModuleAddr + 0x8D7A0E3);             //xor rcx, [0x0000000000E22846]
		//rax = baseModuleAddr + 0x1343C359;              //lea rax, [0x000000000B4E4AB5]
		//rbx += rdx;             //add rbx, rdx
		//rcx = ~rcx;             //not rcx
		//rbx += rax;             //add rbx, rax
		//rax = 0xD63E4A83CB9A620B;               //mov rax, 0xD63E4A83CB9A620B
		//rbx *=  *(uintptr_t*)(rcx + 0x11);             //imul rbx, [rcx+0x11]
		//rbx -= rdx;             //sub rbx, rdx
		//rbx *= rax;             //imul rbx, rax
		//rax = 0x57242547CAD98C71;               //mov rax, 0x57242547CAD98C71
		//rbx -= rax;             //sub rbx, rax
		//return rbx;
	}
	uint64_t get_client_info_base()
	{
		//auto baseModuleAddr = g_data::base;
		//auto Peb = __readgsqword(0x60);


		//uint64_t rax = baseModuleAddr, rbx = baseModuleAddr, rcx = baseModuleAddr, rdx = baseModuleAddr, rdi = baseModuleAddr, rsi = baseModuleAddr, r8 = baseModuleAddr, r9 = baseModuleAddr, r10 = baseModuleAddr, r11 = baseModuleAddr, r12 = baseModuleAddr, r13 = baseModuleAddr, r14 = baseModuleAddr, r15 = baseModuleAddr;
		//rax =  *(uintptr_t*)(get_client_info() + 0x10b770);
		//if (!rax)
		//	return rax;
		//r11 = ~Peb;              //mov r11, gs:[rcx]
		//rcx = r11;              //mov rcx, r11
		////failed to translate: mov [rsp+0x3E0], r13
		//rcx = _rotl64(rcx, 0x23);               //rol rcx, 0x23
		//rcx &= 0xF;
		//auto clientSwitch = rcx;
		//switch (rcx) {
		//case 0:
		//{
		//	rbx = baseModuleAddr;           //lea rbx, [0xFFFFFFFFFD6C2EDE]
		//	r10 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A129);              //mov r10, [0x000000000643CF9A]
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x7;            //shr rcx, 0x07
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0xE;            //shr rcx, 0x0E
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x1C;           //shr rcx, 0x1C
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x38;           //shr rcx, 0x38
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0xA;            //shr rcx, 0x0A
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x14;           //shr rcx, 0x14
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x28;           //shr rcx, 0x28
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//	rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//	rcx ^= r10;             //xor rcx, r10
		//	rcx = (_byteswap_uint64)(rcx);                 //bswap rcx
		//	rax *=  *(uintptr_t*)(rcx + 0x15);             //imul rax, [rcx+0x15]
		//	rax += rbx;             //add rax, rbx
		//	rcx = 0x6A51BC9BC4AA6767;               //mov rcx, 0x6A51BC9BC4AA6767
		//	rax *= rcx;             //imul rax, rcx
		//	rcx = 0x5447EBF1221B83E6;               //mov rcx, 0x5447EBF1221B83E6
		//	rax -= rcx;             //sub rax, rcx
		//	rax ^= r11;             //xor rax, r11
		//	rax ^= rbx;             //xor rax, rbx
		//	return rax;
		//}
		//case 1:
		//{
		//	r10 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A129);              //mov r10, [0x000000000643CB49]
		//	rbx = baseModuleAddr;           //lea rbx, [0xFFFFFFFFFD6C2A19]
		//	rdx = baseModuleAddr + 0x6B3C0100;              //lea rdx, [0x0000000068A82ABF]
		//	rax -= r11;             //sub rax, r11
		//	rcx = rdx;              //mov rcx, rdx
		//	rcx = ~rcx;             //not rcx
		//	rax ^= rcx;             //xor rax, rcx
		//	rax ^= r11;             //xor rax, r11
		//	rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//	rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//	rcx ^= r10;             //xor rcx, r10
		//	rcx = (_byteswap_uint64)(rcx);                 //bswap rcx
		//	rax *=  *(uintptr_t*)(rcx + 0x15);             //imul rax, [rcx+0x15]
		//	rax ^= rbx;             //xor rax, rbx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x24;           //shr rcx, 0x24
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = 0x4A2A83616AD92661;               //mov rcx, 0x4A2A83616AD92661
		//	rax *= rcx;             //imul rax, rcx
		//	rcx = 0xECFC5B4C57C54F28;               //mov rcx, 0xECFC5B4C57C54F28
		//	rax += rcx;             //add rax, rcx
		//	rcx = 0xF0FDCE631F7BA29F;               //mov rcx, 0xF0FDCE631F7BA29F
		//	rax ^= rcx;             //xor rax, rcx
		//	return rax;
		//}
		//case 2:
		//{
		//	rbx = baseModuleAddr;           //lea rbx, [0xFFFFFFFFFD6C2571]
		//	rdx =  *(uintptr_t*)(baseModuleAddr + 0x8D7A129);              //mov rdx, [0x000000000643C616]
		//	rcx = 0xD511FD9CF85D2C07;               //mov rcx, 0xD511FD9CF85D2C07
		//	rax *= rcx;             //imul rax, rcx
		//	rax ^= r11;             //xor rax, r11
		//	rcx = r11;              //mov rcx, r11
		//	uintptr_t RSP_0x60;
		//	RSP_0x60 = baseModuleAddr + 0x2E433015;                 //lea rcx, [0x000000002BAF553F] : RSP+0x60
		//	rcx *= RSP_0x60;                //imul rcx, [rsp+0x60]
		//	rax += rcx;             //add rax, rcx
		//	rax -= rbx;             //sub rax, rbx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x12;           //shr rcx, 0x12
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x24;           //shr rcx, 0x24
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = 0x3EDAD65FDC1034FF;               //mov rcx, 0x3EDAD65FDC1034FF
		//	rax *= rcx;             //imul rax, rcx
		//	rcx = 0x2AE3002A8E8BF08B;               //mov rcx, 0x2AE3002A8E8BF08B
		//	rax -= rcx;             //sub rax, rcx
		//	rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//	rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//	rcx ^= rdx;             //xor rcx, rdx
		//	rcx = (_byteswap_uint64)(rcx);                 //bswap rcx
		//	rax *=  *(uintptr_t*)(rcx + 0x15);             //imul rax, [rcx+0x15]
		//	return rax;
		//}
		//case 3:
		//{
		//	r10 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A129);              //mov r10, [0x000000000643C25C]
		//	rbx = baseModuleAddr;           //lea rbx, [0xFFFFFFFFFD6C212C]
		//	rcx = rbx + 0x1771cb1b;                 //lea rcx, [rbx+0x1771CB1B]
		//	rcx += r11;             //add rcx, r11
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0xB;            //shr rcx, 0x0B
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x16;           //shr rcx, 0x16
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x2C;           //shr rcx, 0x2C
		//	rax ^= rcx;             //xor rax, rcx
		//	rax -= r11;             //sub rax, r11
		//	rcx = 0xFD500870540625B;                //mov rcx, 0xFD500870540625B
		//	rax *= rcx;             //imul rax, rcx
		//	rcx = 0x1BC06434489E44B5;               //mov rcx, 0x1BC06434489E44B5
		//	rax += rcx;             //add rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x1F;           //shr rcx, 0x1F
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x3E;           //shr rcx, 0x3E
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//	rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//	rcx ^= r10;             //xor rcx, r10
		//	rcx = (_byteswap_uint64)(rcx);                 //bswap rcx
		//	rax *=  *(uintptr_t*)(rcx + 0x15);             //imul rax, [rcx+0x15]
		//	rcx = 0x600D6B3C699E6524;               //mov rcx, 0x600D6B3C699E6524
		//	rax += rcx;             //add rax, rcx
		//	return rax;
		//}
		//case 4:
		//{
		//	rbx = baseModuleAddr;           //lea rbx, [0xFFFFFFFFFD6C1B34]
		//	r13 = baseModuleAddr + 0x27B9;          //lea r13, [0xFFFFFFFFFD6C42D5]
		//	r9 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A129);               //mov r9, [0x000000000643BBE9]
		//	rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//	rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//	rcx ^= r9;              //xor rcx, r9
		//	rcx = (_byteswap_uint64)(rcx);                 //bswap rcx
		//	rax *=  *(uintptr_t*)(rcx + 0x15);             //imul rax, [rcx+0x15]
		//	rcx = 0x44DF33AE79D34CE7;               //mov rcx, 0x44DF33AE79D34CE7
		//	rax *= rcx;             //imul rax, rcx
		//	rax -= r11;             //sub rax, r11
		//	rcx = r11;              //mov rcx, r11
		//	rcx = ~rcx;             //not rcx
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = baseModuleAddr + 0x3B36;          //lea rcx, [0xFFFFFFFFFD6C513B]
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x27;           //shr rcx, 0x27
		//	rax ^= rcx;             //xor rax, rcx
		//	rax -= r11;             //sub rax, r11
		//	rcx = r11;              //mov rcx, r11
		//	rcx *= r13;             //imul rcx, r13
		//	rax += rcx;             //add rax, rcx
		//	rcx = r11;              //mov rcx, r11
		//	rcx -= rbx;             //sub rcx, rbx
		//	rcx += 0xFFFFFFFFD6AD7A46;              //add rcx, 0xFFFFFFFFD6AD7A46
		//	rax += rcx;             //add rax, rcx
		//	return rax;
		//}
		//case 5:
		//{
		//	r10 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A129);              //mov r10, [0x000000000643B705]
		//	rbx = baseModuleAddr;           //lea rbx, [0xFFFFFFFFFD6C15D5]
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x17;           //shr rcx, 0x17
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x2E;           //shr rcx, 0x2E
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rbx + 0xb7ef;             //lea rcx, [rbx+0xB7EF]
		//	rcx += r11;             //add rcx, r11
		//	rax += rcx;             //add rax, rcx
		//	rax -= r11;             //sub rax, r11
		//	uintptr_t RSP_0x30;
		//	RSP_0x30 = 0x20E3F69C982B8265;          //mov rcx, 0x20E3F69C982B8265 : RSP+0x30
		//	rax ^= RSP_0x30;                //xor rax, [rsp+0x30]
		//	rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//	rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//	rcx ^= r10;             //xor rcx, r10
		//	rcx = (_byteswap_uint64)(rcx);                 //bswap rcx
		//	rax *=  *(uintptr_t*)(rcx + 0x15);             //imul rax, [rcx+0x15]
		//	rcx = 0xAEE1A029315E3D4F;               //mov rcx, 0xAEE1A029315E3D4F
		//	rax *= rcx;             //imul rax, rcx
		//	rcx = rbx + 0x618b;             //lea rcx, [rbx+0x618B]
		//	rcx += r11;             //add rcx, r11
		//	rax += rcx;             //add rax, rcx
		//	rcx = 0x4F576A9DC4CD39EE;               //mov rcx, 0x4F576A9DC4CD39EE
		//	rax += rcx;             //add rax, rcx
		//	return rax;
		//}
		//case 6:
		//{
		//	r9 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A129);               //mov r9, [0x000000000643B253]
		//	rax -= r11;             //sub rax, r11
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x12;           //shr rcx, 0x12
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x24;           //shr rcx, 0x24
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x21;           //shr rcx, 0x21
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x1A;           //shr rcx, 0x1A
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x34;           //shr rcx, 0x34
		//	rcx ^= rax;             //xor rcx, rax
		//	rax = 0x17CF0497F2D22203;               //mov rax, 0x17CF0497F2D22203
		//	rcx *= rax;             //imul rcx, rax
		//	rax = rcx;              //mov rax, rcx
		//	rax >>= 0x25;           //shr rax, 0x25
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = 0xBE5CC72B0AEE64FD;               //mov rcx, 0xBE5CC72B0AEE64FD
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//	rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//	rcx ^= r9;              //xor rcx, r9
		//	rcx = (_byteswap_uint64)(rcx);                 //bswap rcx
		//	rax *=  *(uintptr_t*)(rcx + 0x15);             //imul rax, [rcx+0x15]
		//	return rax;
		//}
		//case 7:
		//{
		//	rbx = baseModuleAddr;           //lea rbx, [0xFFFFFFFFFD6C0C3A]
		//	r10 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A129);              //mov r10, [0x000000000643ACD6]
		//	rdx = r11;              //mov rdx, r11
		//	rcx = baseModuleAddr + 0xDEF0;          //lea rcx, [0xFFFFFFFFFD6CE820]
		//	rdx ^= rcx;             //xor rdx, rcx
		//	rcx = 0xF875422C3B24C08F;               //mov rcx, 0xF875422C3B24C08F
		//	rcx -= rdx;             //sub rcx, rdx
		//	rax += rcx;             //add rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x16;           //shr rcx, 0x16
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x2C;           //shr rcx, 0x2C
		//	rax ^= rcx;             //xor rax, rcx
		//	rax ^= rbx;             //xor rax, rbx
		//	rcx = 0x65FDE940447DEE2B;               //mov rcx, 0x65FDE940447DEE2B
		//	rax *= rcx;             //imul rax, rcx
		//	rcx = 0x39A26EAD2B76265B;               //mov rcx, 0x39A26EAD2B76265B
		//	rax += rcx;             //add rax, rcx
		//	rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//	rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//	rcx ^= r10;             //xor rcx, r10
		//	rcx = (_byteswap_uint64)(rcx);                 //bswap rcx
		//	rax *=  *(uintptr_t*)(rcx + 0x15);             //imul rax, [rcx+0x15]
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x1B;           //shr rcx, 0x1B
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x36;           //shr rcx, 0x36
		//	rax ^= rcx;             //xor rax, rcx
		//	return rax;
		//}
		//case 8:
		//{
		//	r10 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A129);              //mov r10, [0x000000000643A810]
		//	rbx = baseModuleAddr;           //lea rbx, [0xFFFFFFFFFD6C06E0]
		//	rcx = 0xFFFFFFFFFFFF597D;               //mov rcx, 0xFFFFFFFFFFFF597D
		//	rcx -= r11;             //sub rcx, r11
		//	rcx -= rbx;             //sub rcx, rbx
		//	rax += rcx;             //add rax, rcx
		//	rax ^= r11;             //xor rax, r11
		//	rcx = 0xD0F09E7A8C7613B3;               //mov rcx, 0xD0F09E7A8C7613B3
		//	rax *= rcx;             //imul rax, rcx
		//	rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//	rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//	rcx ^= r10;             //xor rcx, r10
		//	rcx = (_byteswap_uint64)(rcx);                 //bswap rcx
		//	rax *=  *(uintptr_t*)(rcx + 0x15);             //imul rax, [rcx+0x15]
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x1F;           //shr rcx, 0x1F
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x3E;           //shr rcx, 0x3E
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = 0x256B3436B62B89E5;               //mov rcx, 0x256B3436B62B89E5
		//	rax -= rcx;             //sub rax, rcx
		//	rax += rbx;             //add rax, rbx
		//	rdx = r11;              //mov rdx, r11
		//	rcx = baseModuleAddr + 0x7B64B958;              //lea rcx, [0x0000000078D0BD47]
		//	rcx = ~rcx;             //not rcx
		//	rdx = ~rdx;             //not rdx
		//	rdx += rcx;             //add rdx, rcx
		//	rax ^= rdx;             //xor rax, rdx
		//	return rax;
		//}
		//case 9:
		//{
		//	rbx = baseModuleAddr;           //lea rbx, [0xFFFFFFFFFD6C025E]
		//	r10 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A129);              //mov r10, [0x000000000643A305]
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x4;            //shr rcx, 0x04
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x8;            //shr rcx, 0x08
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x10;           //shr rcx, 0x10
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x20;           //shr rcx, 0x20
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = 0x54E648D07B6D0B80;               //mov rcx, 0x54E648D07B6D0B80
		//	rax += rcx;             //add rax, rcx
		//	rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//	rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//	rcx ^= r10;             //xor rcx, r10
		//	rcx = (_byteswap_uint64)(rcx);                 //bswap rcx
		//	rax *=  *(uintptr_t*)(rcx + 0x15);             //imul rax, [rcx+0x15]
		//	rax ^= rbx;             //xor rax, rbx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x16;           //shr rcx, 0x16
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x2C;           //shr rcx, 0x2C
		//	rcx ^= r11;             //xor rcx, r11
		//	rax ^= rcx;             //xor rax, rcx
		//	rax -= r11;             //sub rax, r11
		//	rcx = 0xB0386AF6C89E01ED;               //mov rcx, 0xB0386AF6C89E01ED
		//	rax *= rcx;             //imul rax, rcx
		//	return rax;
		//}
		//case 10:
		//{
		//	r10 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A129);              //mov r10, [0x0000000006439E0A]
		//	rbx = baseModuleAddr;           //lea rbx, [0xFFFFFFFFFD6BFCCE]
		//	rcx = 0x9332D19135BB918F;               //mov rcx, 0x9332D19135BB918F
		//	rax *= rcx;             //imul rax, rcx
		//	rcx = 0xFFFFFFFF8DA4B362;               //mov rcx, 0xFFFFFFFF8DA4B362
		//	rcx -= r11;             //sub rcx, r11
		//	rcx -= rbx;             //sub rcx, rbx
		//	rax += rcx;             //add rax, rcx
		//	rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//	rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//	rcx ^= r10;             //xor rcx, r10
		//	rcx = (_byteswap_uint64)(rcx);                 //bswap rcx
		//	rax *=  *(uintptr_t*)(rcx + 0x15);             //imul rax, [rcx+0x15]
		//	rax ^= r11;             //xor rax, r11
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x8;            //shr rcx, 0x08
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x10;           //shr rcx, 0x10
		//	rax ^= rcx;             //xor rax, rcx
		//	rdx = r11;              //mov rdx, r11
		//	rdx = ~rdx;             //not rdx
		//	rcx = baseModuleAddr + 0xD1ED;          //lea rcx, [0xFFFFFFFFFD6CCD1C]
		//	rdx *= rcx;             //imul rdx, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x20;           //shr rcx, 0x20
		//	rdx ^= rcx;             //xor rdx, rcx
		//	rax ^= rdx;             //xor rax, rdx
		//	rdx = r11;              //mov rdx, r11
		//	rdx = ~rdx;             //not rdx
		//	rcx = baseModuleAddr + 0x868;           //lea rcx, [0xFFFFFFFFFD6C0409]
		//	rax += rcx;             //add rax, rcx
		//	rax += rdx;             //add rax, rdx
		//	rcx = r11;              //mov rcx, r11
		//	rcx = ~rcx;             //not rcx
		//	rcx -= rbx;             //sub rcx, rbx
		//	rcx -= 0x7CCC6306;              //sub rcx, 0x7CCC6306
		//	rax ^= rcx;             //xor rax, rcx
		//	return rax;
		//}
		//case 11:
		//{
		//	r9 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A129);               //mov r9, [0x000000000643988F]
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x12;           //shr rcx, 0x12
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x24;           //shr rcx, 0x24
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0xA;            //shr rcx, 0x0A
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x14;           //shr rcx, 0x14
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x28;           //shr rcx, 0x28
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = 0x8CABBD467C0219D3;               //mov rcx, 0x8CABBD467C0219D3
		//	rax *= rcx;             //imul rax, rcx
		//	rcx = 0xAB98E88DE9C18818;               //mov rcx, 0xAB98E88DE9C18818
		//	rax ^= rcx;             //xor rax, rcx
		//	rax -= r11;             //sub rax, r11
		//	rcx = r11;              //mov rcx, r11
		//	uintptr_t RSP_0x78;
		//	RSP_0x78 = baseModuleAddr + 0x8516;             //lea rcx, [0xFFFFFFFFFD6C7D20] : RSP+0x78
		//	rcx *= RSP_0x78;                //imul rcx, [rsp+0x78]
		//	rax += rcx;             //add rax, rcx
		//	rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//	rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//	rcx ^= r9;              //xor rcx, r9
		//	rcx = (_byteswap_uint64)(rcx);                 //bswap rcx
		//	rax *=  *(uintptr_t*)(rcx + 0x15);             //imul rax, [rcx+0x15]
		//	rcx = 0x596E42B1953FE5C1;               //mov rcx, 0x596E42B1953FE5C1
		//	rax += rcx;             //add rax, rcx
		//	return rax;
		//}
		//case 12:
		//{
		//	r10 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A129);              //mov r10, [0x000000000643932C]
		//	rcx = baseModuleAddr + 0x1BAF;          //lea rcx, [0xFFFFFFFFFD6C0A16]
		//	rcx ^= r11;             //xor rcx, r11
		//	rax ^= rcx;             //xor rax, rcx
		//	rdx = 0;                //and rdx, 0xFFFFFFFFC0000000
		//	rdx = _rotl64(rdx, 0x10);               //rol rdx, 0x10
		//	rcx = baseModuleAddr + 0x259F56F6;              //lea rcx, [0x00000000230B47EF]
		//	rdx ^= r10;             //xor rdx, r10
		//	rcx -= r11;             //sub rcx, r11
		//	rdx = (_byteswap_uint64)(rdx);                 //bswap rdx
		//	rax += rcx;             //add rax, rcx
		//	rax *=  *(uintptr_t*)(rdx + 0x15);             //imul rax, [rdx+0x15]
		//	rcx = 0x294D76F27D0F6D85;               //mov rcx, 0x294D76F27D0F6D85
		//	rax -= rcx;             //sub rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x8;            //shr rcx, 0x08
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x10;           //shr rcx, 0x10
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x20;           //shr rcx, 0x20
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = 0x40AE2552A77DAFE6;               //mov rcx, 0x40AE2552A77DAFE6
		//	rax -= r11;             //sub rax, r11
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = 0x6425FC1CEAFDBD3B;               //mov rcx, 0x6425FC1CEAFDBD3B
		//	rax *= rcx;             //imul rax, rcx
		//	return rax;
		//}
		//case 13:
		//{
		//	r10 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A129);              //mov r10, [0x0000000006438F0F]
		//	rbx = baseModuleAddr;           //lea rbx, [0xFFFFFFFFFD6BEDDF]
		//	rax += rbx;             //add rax, rbx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x10;           //shr rcx, 0x10
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x20;           //shr rcx, 0x20
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//	rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//	rcx ^= r10;             //xor rcx, r10
		//	rcx = (_byteswap_uint64)(rcx);                 //bswap rcx
		//	rax *=  *(uintptr_t*)(rcx + 0x15);             //imul rax, [rcx+0x15]
		//	rcx = r11;              //mov rcx, r11
		//	rcx -= rbx;             //sub rcx, rbx
		//	rax += rcx;             //add rax, rcx
		//	rcx = 0x7AAF0F372FD53CD5;               //mov rcx, 0x7AAF0F372FD53CD5
		//	rax *= rcx;             //imul rax, rcx
		//	rcx = 0x4BE188FD7D45B824;               //mov rcx, 0x4BE188FD7D45B824
		//	rax -= rcx;             //sub rax, rcx
		//	rax ^= r11;             //xor rax, r11
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x25;           //shr rcx, 0x25
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = 0x2F94247E3E6CDFF6;               //mov rcx, 0x2F94247E3E6CDFF6
		//	rax -= rcx;             //sub rax, rcx
		//	return rax;
		//}
		//case 14:
		//{
		//	r10 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A129);              //mov r10, [0x0000000006438A64]
		//	rbx = baseModuleAddr;           //lea rbx, [0xFFFFFFFFFD6BE934]
		//	rdx = baseModuleAddr + 0x9DA7;          //lea rdx, [0xFFFFFFFFFD6C8673]
		//	rcx = 0x59FC5D34C1D95075;               //mov rcx, 0x59FC5D34C1D95075
		//	rax += rcx;             //add rax, rcx
		//	rcx = 0x113F93E895C764EB;               //mov rcx, 0x113F93E895C764EB
		//	rax *= rcx;             //imul rax, rcx
		//	rcx = baseModuleAddr + 0x3BFCF952;              //lea rcx, [0x000000003968DF65]
		//	rcx = ~rcx;             //not rcx
		//	rcx ^= r11;             //xor rcx, r11
		//	rax -= rcx;             //sub rax, rcx
		//	rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//	rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//	rcx ^= r10;             //xor rcx, r10
		//	rcx = (_byteswap_uint64)(rcx);                 //bswap rcx
		//	rax *=  *(uintptr_t*)(rcx + 0x15);             //imul rax, [rcx+0x15]
		//	rax -= rbx;             //sub rax, rbx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x23;           //shr rcx, 0x23
		//	rax ^= rcx;             //xor rax, rcx
		//	uintptr_t RSP_0x28;
		//	RSP_0x28 = 0x3D4F5BB3C70BE95B;          //mov rcx, 0x3D4F5BB3C70BE95B : RSP+0x28
		//	rax *= RSP_0x28;                //imul rax, [rsp+0x28]
		//	rcx = rdx;              //mov rcx, rdx
		//	rcx ^= r11;             //xor rcx, r11
		//	rax -= rcx;             //sub rax, rcx
		//	return rax;
		//}
		//case 15:
		//{
		//	rbx = baseModuleAddr;           //lea rbx, [0xFFFFFFFFFD6BE485]
		//	r9 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A129);               //mov r9, [0x000000000643853C]
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x13;           //shr rcx, 0x13
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x26;           //shr rcx, 0x26
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = 0x63FD32E967945525;               //mov rcx, 0x63FD32E967945525
		//	rax -= rcx;             //sub rax, rcx
		//	rax += r11;             //add rax, r11
		//	rcx = 0xB85215B9839B7D9;                //mov rcx, 0xB85215B9839B7D9
		//	rax += rcx;             //add rax, rcx
		//	rax ^= rbx;             //xor rax, rbx
		//	rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//	rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//	rcx ^= r9;              //xor rcx, r9
		//	rcx = (_byteswap_uint64)(rcx);                 //bswap rcx
		//	rax *=  *(uintptr_t*)(rcx + 0x15);             //imul rax, [rcx+0x15]
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x5;            //shr rcx, 0x05
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0xA;            //shr rcx, 0x0A
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x14;           //shr rcx, 0x14
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x28;           //shr rcx, 0x28
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = 0xE52EBF353AE32CDB;               //mov rcx, 0xE52EBF353AE32CDB
		//	rax *= rcx;             //imul rax, rcx
		//	return rax;
		//}
		//}
	}
	uint64_t get_bone_ptr()
	{
		//auto baseModuleAddr = g_data::base;
		//auto Peb = __readgsqword(0x60);
		//uint64_t rax = baseModuleAddr, rbx = baseModuleAddr, rcx = baseModuleAddr, rdx = baseModuleAddr, rdi = baseModuleAddr, rsi = baseModuleAddr, r8 = baseModuleAddr, r9 = baseModuleAddr, r10 = baseModuleAddr, r11 = baseModuleAddr, r12 = baseModuleAddr, r13 = baseModuleAddr, r14 = baseModuleAddr, r15 = baseModuleAddr;
		//rax =  *(uintptr_t*)(baseModuleAddr + 0xC6137A8);
		//if (!rax)
		//	return rax;
		//rbx = Peb;              //mov rbx, gs:[rcx]
		//rcx = rbx;              //mov rcx, rbx
		//rcx >>= 0x1C;           //shr rcx, 0x1C
		//rcx &= 0xF;
		//auto clientSwitch = rcx;
		//switch (rcx) {
		//case 0:
		//{
		//	r9 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A221);               //mov r9, [0x0000000006DF5311]
		//	rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//	rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//	rcx ^= r9;              //xor rcx, r9
		//	rcx = ~rcx;             //not rcx
		//	rax *=  *(uintptr_t*)(rcx + 0x13);             //imul rax, [rcx+0x13]
		//	rax += rbx;             //add rax, rbx
		//	rcx = rax;              //mov rcx, rax
		//	rax >>= 0x13;           //shr rax, 0x13
		//	rcx ^= rax;             //xor rcx, rax
		//	rax = rcx;              //mov rax, rcx
		//	rax >>= 0x26;           //shr rax, 0x26
		//	rax ^= rcx;             //xor rax, rcx
		//	rax -= rbx;             //sub rax, rbx
		//	rcx = 0xD5A6F9222EC0CD8B;               //mov rcx, 0xD5A6F9222EC0CD8B
		//	rax *= rcx;             //imul rax, rcx
		//	rcx = 0xBB2862E8C0DD851B;               //mov rcx, 0xBB2862E8C0DD851B
		//	rax *= rcx;             //imul rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x4;            //shr rcx, 0x04
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x8;            //shr rcx, 0x08
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x10;           //shr rcx, 0x10
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x20;           //shr rcx, 0x20
		//	rax ^= rcx;             //xor rax, rcx
		//	return rax;
		//}
		//case 1:
		//{
		//	r10 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A221);              //mov r10, [0x0000000006DF4EB9]
		//	rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//	rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//	rcx ^= r10;             //xor rcx, r10
		//	rcx = ~rcx;             //not rcx
		//	rax *=  *(uintptr_t*)(rcx + 0x13);             //imul rax, [rcx+0x13]
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x1E;           //shr rcx, 0x1E
		//	rax ^= rcx;             //xor rax, rcx
		//	rdx = baseModuleAddr + 0x6179D5AB;              //lea rdx, [0x000000005F818020]
		//	rdx -= rbx;             //sub rdx, rbx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x3C;           //shr rcx, 0x3C
		//	rdx ^= rcx;             //xor rdx, rcx
		//	rax ^= rdx;             //xor rax, rdx
		//	rax += rbx;             //add rax, rbx
		//	rcx = 0xC430FCF5AB246D6;                //mov rcx, 0xC430FCF5AB246D6
		//	rax += rcx;             //add rax, rcx
		//	rcx = 0x8A220291A10CAF87;               //mov rcx, 0x8A220291A10CAF87
		//	rax *= rcx;             //imul rax, rcx
		//	rcx = 0x7FAA38A95F85A6FD;               //mov rcx, 0x7FAA38A95F85A6FD
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = baseModuleAddr;           //lea rcx, [0xFFFFFFFFFE07AC19]
		//	rax -= rcx;             //sub rax, rcx
		//	return rax;
		//}
		//case 2:
		//{
		//	r14 = baseModuleAddr + 0x2281;          //lea r14, [0xFFFFFFFFFE07CB04]
		//	r13 = baseModuleAddr + 0x66A0AFC0;              //lea r13, [0x0000000064A85834]
		//	r10 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A221);              //mov r10, [0x0000000006DF4A49]
		//	rdx = r13;              //mov rdx, r13
		//	rdx = ~rdx;             //not rdx
		//	rdx *= rbx;             //imul rdx, rbx
		//	rcx = rbx;              //mov rcx, rbx
		//	rcx = ~rcx;             //not rcx
		//	rdx += rcx;             //add rdx, rcx
		//	rcx = baseModuleAddr + 0xBA28;          //lea rcx, [0xFFFFFFFFFE085FA6]
		//	rax += rcx;             //add rax, rcx
		//	rax += rdx;             //add rax, rdx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x13;           //shr rcx, 0x13
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x26;           //shr rcx, 0x26
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = r14;              //mov rcx, r14
		//	rcx ^= rbx;             //xor rcx, rbx
		//	rax += rcx;             //add rax, rcx
		//	rcx = 0x975CC895B7E831F1;               //mov rcx, 0x975CC895B7E831F1
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = 0x3B97C5DC626E056F;               //mov rcx, 0x3B97C5DC626E056F
		//	rax *= rcx;             //imul rax, rcx
		//	rcx = 0x5534067E232C6632;               //mov rcx, 0x5534067E232C6632
		//	rax += rcx;             //add rax, rcx
		//	rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//	rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//	rcx ^= r10;             //xor rcx, r10
		//	rcx = ~rcx;             //not rcx
		//	rax *=  *(uintptr_t*)(rcx + 0x13);             //imul rax, [rcx+0x13]
		//	return rax;
		//}
		//case 3:
		//{
		//	r13 = baseModuleAddr + 0x50F8B6F5;              //lea r13, [0x000000004F0059FD]
		//	r9 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A221);               //mov r9, [0x0000000006DF445F]
		//	rcx = r13;              //mov rcx, r13
		//	rcx = ~rcx;             //not rcx
		//	rcx ^= rbx;             //xor rcx, rbx
		//	rax += rcx;             //add rax, rcx
		//	rcx = baseModuleAddr;           //lea rcx, [0xFFFFFFFFFE079E55]
		//	rax += rcx;             //add rax, rcx
		//	rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//	rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//	rcx ^= r9;              //xor rcx, r9
		//	rcx = ~rcx;             //not rcx
		//	rax *=  *(uintptr_t*)(rcx + 0x13);             //imul rax, [rcx+0x13]
		//	rcx = 0x60F6A79B0C8456B1;               //mov rcx, 0x60F6A79B0C8456B1
		//	rax += rcx;             //add rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x1B;           //shr rcx, 0x1B
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x36;           //shr rcx, 0x36
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = 0x648FCA6FE7D44377;               //mov rcx, 0x648FCA6FE7D44377
		//	rax -= rcx;             //sub rax, rcx
		//	rcx = 0xC462FCF18E2C2995;               //mov rcx, 0xC462FCF18E2C2995
		//	rax *= rcx;             //imul rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x16;           //shr rcx, 0x16
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x2C;           //shr rcx, 0x2C
		//	rax ^= rcx;             //xor rax, rcx
		//	return rax;
		//}
		//case 4:
		//{
		//	r10 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A221);              //mov r10, [0x0000000006DF3EE4]
		//	rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//	rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//	rcx ^= r10;             //xor rcx, r10
		//	rcx = ~rcx;             //not rcx
		//	rax *=  *(uintptr_t*)(rcx + 0x13);             //imul rax, [rcx+0x13]
		//	rax -= rbx;             //sub rax, rbx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x18;           //shr rcx, 0x18
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x30;           //shr rcx, 0x30
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = 0xB1A93FB4C084CAB9;               //mov rcx, 0xB1A93FB4C084CAB9
		//	rax *= rcx;             //imul rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x24;           //shr rcx, 0x24
		//	rcx ^= rax;             //xor rcx, rax
		//	rax = 0x352F8A796F79706B;               //mov rax, 0x352F8A796F79706B
		//	rcx ^= rax;             //xor rcx, rax
		//	rax = baseModuleAddr;           //lea rax, [0xFFFFFFFFFE079919]
		//	rcx -= rax;             //sub rcx, rax
		//	rax = rbx + 0xffffffffa5917e54;                 //lea rax, [rbx-0x5A6E81AC]
		//	rax += rcx;             //add rax, rcx
		//	rcx = 0xA920BAB7A21DDE47;               //mov rcx, 0xA920BAB7A21DDE47
		//	rax *= rcx;             //imul rax, rcx
		//	return rax;
		//}
		//case 5:
		//{
		//	r10 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A221);              //mov r10, [0x0000000006DF3A64]
		//	rdx = baseModuleAddr + 0x661;           //lea rdx, [0xFFFFFFFFFE079E34]
		//	uintptr_t RSP_0x78;
		//	RSP_0x78 = 0x19A86082B9386E61;          //mov rcx, 0x19A86082B9386E61 : RSP+0x78
		//	rax ^= RSP_0x78;                //xor rax, [rsp+0x78]
		//	rcx = rbx;              //mov rcx, rbx
		//	rcx = ~rcx;             //not rcx
		//	uintptr_t RSP_0x30;
		//	RSP_0x30 = baseModuleAddr + 0x89B8;             //lea rcx, [0xFFFFFFFFFE0821C2] : RSP+0x30
		//	rcx ^= RSP_0x30;                //xor rcx, [rsp+0x30]
		//	rax -= rcx;             //sub rax, rcx
		//	rcx = 0x6CF5D40C805C3929;               //mov rcx, 0x6CF5D40C805C3929
		//	rax *= rcx;             //imul rax, rcx
		//	rax -= rbx;             //sub rax, rbx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x23;           //shr rcx, 0x23
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = 0xEA2BDCA216FA84E;                //mov rcx, 0xEA2BDCA216FA84E
		//	rax += rcx;             //add rax, rcx
		//	rcx = rdx;              //mov rcx, rdx
		//	rcx = ~rcx;             //not rcx
		//	rcx -= rbx;             //sub rcx, rbx
		//	rax += rcx;             //add rax, rcx
		//	rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//	rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//	rcx ^= r10;             //xor rcx, r10
		//	rcx = ~rcx;             //not rcx
		//	rax *=  *(uintptr_t*)(rcx + 0x13);             //imul rax, [rcx+0x13]
		//	return rax;
		//}
		//case 6:
		//{
		//	r9 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A221);               //mov r9, [0x0000000006DF3545]
		//	rcx = 0x143119596E0AB6F4;               //mov rcx, 0x143119596E0AB6F4
		//	rcx -= rbx;             //sub rcx, rbx
		//	rax += rcx;             //add rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x14;           //shr rcx, 0x14
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x28;           //shr rcx, 0x28
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = baseModuleAddr;           //lea rcx, [0xFFFFFFFFFE079171]
		//	rax -= rcx;             //sub rax, rcx
		//	rcx = 0xD0FF53657C7A437;                //mov rcx, 0xD0FF53657C7A437
		//	rax += rcx;             //add rax, rcx
		//	rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//	rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//	rcx ^= r9;              //xor rcx, r9
		//	rcx = ~rcx;             //not rcx
		//	rax *=  *(uintptr_t*)(rcx + 0x13);             //imul rax, [rcx+0x13]
		//	rcx = 0x1946435536018835;               //mov rcx, 0x1946435536018835
		//	rax *= rcx;             //imul rax, rcx
		//	rax -= rbx;             //sub rax, rbx
		//	return rax;
		//}
		//case 7:
		//{
		//	r11 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A221);              //mov r11, [0x0000000006DF3134]
		//	rdx = baseModuleAddr + 0x32D6EFEE;              //lea rdx, [0x0000000030DE7EA7]
		//	r8 = 0;                 //and r8, 0xFFFFFFFFC0000000
		//	r8 = _rotl64(r8, 0x10);                 //rol r8, 0x10
		//	r8 ^= r11;              //xor r8, r11
		//	rcx = rbx;              //mov rcx, rbx
		//	rcx = ~rcx;             //not rcx
		//	r8 = ~r8;               //not r8
		//	rax += rcx;             //add rax, rcx
		//	rax += rdx;             //add rax, rdx
		//	rax ^= rbx;             //xor rax, rbx
		//	rax *=  *(uintptr_t*)(r8 + 0x13);              //imul rax, [r8+0x13]
		//	rcx = 0x6B8832A948DD0921;               //mov rcx, 0x6B8832A948DD0921
		//	rax += rcx;             //add rax, rcx
		//	rcx = 0x9D382E284DCFD7C7;               //mov rcx, 0x9D382E284DCFD7C7
		//	rax *= rcx;             //imul rax, rcx
		//	rcx = 0x4A2F2EC6D9595386;               //mov rcx, 0x4A2F2EC6D9595386
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x7;            //shr rcx, 0x07
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0xE;            //shr rcx, 0x0E
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x1C;           //shr rcx, 0x1C
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x38;           //shr rcx, 0x38
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x15;           //shr rcx, 0x15
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x2A;           //shr rcx, 0x2A
		//	rax ^= rcx;             //xor rax, rcx
		//	return rax;
		//}
		//case 8:
		//{
		//	r14 = baseModuleAddr + 0x98C7;          //lea r14, [0xFFFFFFFFFE082217]
		//	r10 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A221);              //mov r10, [0x0000000006DF2B13]
		//	rcx = baseModuleAddr;           //lea rcx, [0xFFFFFFFFFE078560]
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = 0xB9B101CE6C8E2F91;               //mov rcx, 0xB9B101CE6C8E2F91
		//	rax *= rcx;             //imul rax, rcx
		//	rcx = baseModuleAddr;           //lea rcx, [0xFFFFFFFFFE078857]
		//	rax -= rcx;             //sub rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0xC;            //shr rcx, 0x0C
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x18;           //shr rcx, 0x18
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x30;           //shr rcx, 0x30
		//	rax ^= rcx;             //xor rax, rcx
		//	rax -= rbx;             //sub rax, rbx
		//	rcx = 0x4F54898D891371A4;               //mov rcx, 0x4F54898D891371A4
		//	rax += rcx;             //add rax, rcx
		//	rdx = 0;                //and rdx, 0xFFFFFFFFC0000000
		//	rcx = r14;              //mov rcx, r14
		//	rdx = _rotl64(rdx, 0x10);               //rol rdx, 0x10
		//	rcx ^= rbx;             //xor rcx, rbx
		//	rax -= rcx;             //sub rax, rcx
		//	rdx ^= r10;             //xor rdx, r10
		//	rdx = ~rdx;             //not rdx
		//	rax *=  *(uintptr_t*)(rdx + 0x13);             //imul rax, [rdx+0x13]
		//	return rax;
		//}
		//case 9:
		//{
		//	r10 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A221);              //mov r10, [0x0000000006DF26B7]
		//	rcx = 0x5C495DB1FF8A0C7D;               //mov rcx, 0x5C495DB1FF8A0C7D
		//	rax *= rcx;             //imul rax, rcx
		//	rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//	rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//	rcx ^= r10;             //xor rcx, r10
		//	rcx = ~rcx;             //not rcx
		//	rax *=  *(uintptr_t*)(rcx + 0x13);             //imul rax, [rcx+0x13]
		//	rcx = baseModuleAddr;           //lea rcx, [0xFFFFFFFFFE078412]
		//	rax -= rcx;             //sub rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x13;           //shr rcx, 0x13
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x26;           //shr rcx, 0x26
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = baseModuleAddr;           //lea rcx, [0xFFFFFFFFFE0781D1]
		//	rax += rcx;             //add rax, rcx
		//	rdx = baseModuleAddr;           //lea rdx, [0xFFFFFFFFFE0781B8]
		//	rdx += rbx;             //add rdx, rbx
		//	rcx = 0x931F45DADBA6534A;               //mov rcx, 0x931F45DADBA6534A
		//	rax += rcx;             //add rax, rcx
		//	rax += rdx;             //add rax, rdx
		//	rcx = 0x7254D9C4F5E0407F;               //mov rcx, 0x7254D9C4F5E0407F
		//	rax *= rcx;             //imul rax, rcx
		//	return rax;
		//}
		//case 10:
		//{
		//	r9 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A221);               //mov r9, [0x0000000006DF2238]
		//	rcx = 0x16092956D42CB466;               //mov rcx, 0x16092956D42CB466
		//	rax += rcx;             //add rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x28;           //shr rcx, 0x28
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x22;           //shr rcx, 0x22
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0xB;            //shr rcx, 0x0B
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x16;           //shr rcx, 0x16
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x2C;           //shr rcx, 0x2C
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = 0xC2E2E61ED49F5991;               //mov rcx, 0xC2E2E61ED49F5991
		//	rax *= rcx;             //imul rax, rcx
		//	rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//	rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//	rcx ^= r9;              //xor rcx, r9
		//	rcx = ~rcx;             //not rcx
		//	rax *=  *(uintptr_t*)(rcx + 0x13);             //imul rax, [rcx+0x13]
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x4;            //shr rcx, 0x04
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x8;            //shr rcx, 0x08
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x10;           //shr rcx, 0x10
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x20;           //shr rcx, 0x20
		//	rax ^= rcx;             //xor rax, rcx
		//	rax -= rbx;             //sub rax, rbx
		//	return rax;
		//}
		//case 11:
		//{
		//	r13 = baseModuleAddr + 0x3E6FD0B3;              //lea r13, [0x000000003C774BB1]
		//	r10 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A221);              //mov r10, [0x0000000006DF1C6C]
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x1B;           //shr rcx, 0x1B
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x36;           //shr rcx, 0x36
		//	rax ^= rcx;             //xor rax, rcx
		//	r14 = baseModuleAddr;           //lea r14, [0xFFFFFFFFFE077541]
		//	rax += r14;             //add rax, r14
		//	r14 = baseModuleAddr + 0x27799030;              //lea r14, [0x000000002581055A]
		//	rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//	rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//	rcx ^= r10;             //xor rcx, r10
		//	rcx = ~rcx;             //not rcx
		//	rax *=  *(uintptr_t*)(rcx + 0x13);             //imul rax, [rcx+0x13]
		//	rcx = 0x6E5FECB626B1C472;               //mov rcx, 0x6E5FECB626B1C472
		//	rax += rcx;             //add rax, rcx
		//	rdx = r13;              //mov rdx, r13
		//	rdx = ~rdx;             //not rdx
		//	rdx ^= rbx;             //xor rdx, rbx
		//	rcx = 0xF5121CBF37E46BBB;               //mov rcx, 0xF5121CBF37E46BBB
		//	rax += rcx;             //add rax, rcx
		//	rax += rdx;             //add rax, rdx
		//	rcx = r14;              //mov rcx, r14
		//	rcx ^= rbx;             //xor rcx, rbx
		//	rax -= rcx;             //sub rax, rcx
		//	rcx = 0xD2D6E8735A76DE2D;               //mov rcx, 0xD2D6E8735A76DE2D
		//	rax *= rcx;             //imul rax, rcx
		//	return rax;
		//}
		//case 12:
		//{
		//	rdx = baseModuleAddr + 0xF737;          //lea rdx, [0xFFFFFFFFFE086C22]
		//	r13 = baseModuleAddr + 0x1124573F;              //lea r13, [0x000000000F2BCC0C]
		//	r10 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A221);              //mov r10, [0x0000000006DF164E]
		//	rax += rbx;             //add rax, rbx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x10;           //shr rcx, 0x10
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x20;           //shr rcx, 0x20
		//	rax ^= rcx;             //xor rax, rcx
		//	uintptr_t RSP_0x50;
		//	RSP_0x50 = 0x636BE495B0FA383E;          //mov rcx, 0x636BE495B0FA383E : RSP+0x50
		//	rax ^= RSP_0x50;                //xor rax, [rsp+0x50]
		//	rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//	rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//	rcx ^= r10;             //xor rcx, r10
		//	rcx = ~rcx;             //not rcx
		//	rax *=  *(uintptr_t*)(rcx + 0x13);             //imul rax, [rcx+0x13]
		//	rcx = r13;              //mov rcx, r13
		//	rcx ^= rbx;             //xor rcx, rbx
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = 0x8812EF99851F0715;               //mov rcx, 0x8812EF99851F0715
		//	rax *= rcx;             //imul rax, rcx
		//	rcx = rbx;              //mov rcx, rbx
		//	rcx = ~rcx;             //not rcx
		//	rcx += rdx;             //add rcx, rdx
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = baseModuleAddr + 0xB69;           //lea rcx, [0xFFFFFFFFFE077E83]
		//	rcx -= rbx;             //sub rcx, rbx
		//	rax += rcx;             //add rax, rcx
		//	return rax;
		//}
		//case 13:
		//{
		//	r13 = baseModuleAddr + 0x5EF7;          //lea r13, [0xFFFFFFFFFE07CEDB]
		//	r10 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A221);              //mov r10, [0x0000000006DF11AB]
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0xA;            //shr rcx, 0x0A
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x14;           //shr rcx, 0x14
		//	rax ^= rcx;             //xor rax, rcx
		//	rdx = rbx;              //mov rdx, rbx
		//	rcx = rax;              //mov rcx, rax
		//	rdx = ~rdx;             //not rdx
		//	rcx >>= 0x28;           //shr rcx, 0x28
		//	rdx ^= r13;             //xor rdx, r13
		//	rax ^= rcx;             //xor rax, rcx
		//	rax += rdx;             //add rax, rdx
		//	rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//	rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//	rcx ^= r10;             //xor rcx, r10
		//	rcx = ~rcx;             //not rcx
		//	rax *=  *(uintptr_t*)(rcx + 0x13);             //imul rax, [rcx+0x13]
		//	rcx = 0x736E085CD239F4CB;               //mov rcx, 0x736E085CD239F4CB
		//	rax *= rcx;             //imul rax, rcx
		//	rax -= rbx;             //sub rax, rbx
		//	rcx = baseModuleAddr;           //lea rcx, [0xFFFFFFFFFE076AF2]
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = 0x7DA51A97E3053243;               //mov rcx, 0x7DA51A97E3053243
		//	rax *= rcx;             //imul rax, rcx
		//	rcx = 0x785CF31817D043AC;               //mov rcx, 0x785CF31817D043AC
		//	rax ^= rcx;             //xor rax, rcx
		//	return rax;
		//}
		//case 14:
		//{
		//	r13 = baseModuleAddr + 0x16A6;          //lea r13, [0xFFFFFFFFFE0780B5]
		//	r14 = baseModuleAddr + 0xF57E;          //lea r14, [0xFFFFFFFFFE085F7E]
		//	r9 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A221);               //mov r9, [0x0000000006DF0BD0]
		//	rcx = rbx;              //mov rcx, rbx
		//	rcx = ~rcx;             //not rcx
		//	rcx *= r14;             //imul rcx, r14
		//	rax += rcx;             //add rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x1A;           //shr rcx, 0x1A
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x34;           //shr rcx, 0x34
		//	rax ^= rcx;             //xor rax, rcx
		//	r11 = 0xBE40C084BA769FA;                //mov r11, 0xBE40C084BA769FA
		//	rcx = r13;              //mov rcx, r13
		//	rcx *= rbx;             //imul rcx, rbx
		//	rcx += r11;             //add rcx, r11
		//	rax += rcx;             //add rax, rcx
		//	rcx = 0xB92AAB45027C43E2;               //mov rcx, 0xB92AAB45027C43E2
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = baseModuleAddr + 0x4FED906B;              //lea rcx, [0x000000004DF4F771]
		//	rcx += rbx;             //add rcx, rbx
		//	rax += rcx;             //add rax, rcx
		//	rcx = 0xBCCB1C832C79BF0B;               //mov rcx, 0xBCCB1C832C79BF0B
		//	rax *= rcx;             //imul rax, rcx
		//	rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		//	rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		//	rcx ^= r9;              //xor rcx, r9
		//	rcx = ~rcx;             //not rcx
		//	rax *=  *(uintptr_t*)(rcx + 0x13);             //imul rax, [rcx+0x13]
		//	return rax;
		//}
		//case 15:
		//{
		//	r14 = baseModuleAddr + 0x31081C40;              //lea r14, [0x000000002F0F811C]
		//	r10 =  *(uintptr_t*)(baseModuleAddr + 0x8D7A221);              //mov r10, [0x0000000006DF069F]
		//	rdx = 0;                //and rdx, 0xFFFFFFFFC0000000
		//	rdx = _rotl64(rdx, 0x10);               //rol rdx, 0x10
		//	rcx = rax;              //mov rcx, rax
		//	rdx ^= r10;             //xor rdx, r10
		//	rcx >>= 0x20;           //shr rcx, 0x20
		//	rdx = ~rdx;             //not rdx
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = r14;              //mov rcx, r14
		//	rcx ^= rbx;             //xor rcx, rbx
		//	rax *=  *(uintptr_t*)(rdx + 0x13);             //imul rax, [rdx+0x13]
		//	rax -= rcx;             //sub rax, rcx
		//	rcx = 0xFC32828FC4E7EFD1;               //mov rcx, 0xFC32828FC4E7EFD1
		//	rax *= rcx;             //imul rax, rcx
		//	rax -= rbx;             //sub rax, rbx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x3;            //shr rcx, 0x03
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x6;            //shr rcx, 0x06
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0xC;            //shr rcx, 0x0C
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x18;           //shr rcx, 0x18
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x30;           //shr rcx, 0x30
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x8;            //shr rcx, 0x08
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x10;           //shr rcx, 0x10
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = rax;              //mov rcx, rax
		//	rcx >>= 0x20;           //shr rcx, 0x20
		//	rax ^= rcx;             //xor rax, rcx
		//	rcx = 0x8D793715ED015397;               //mov rcx, 0x8D793715ED015397
		//	rax *= rcx;             //imul rax, rcx
		//	return rax;
		//}
		//}
	}
	uint16_t get_bone_index(uint32_t boneIndex)
	{
		//auto baseModuleAddr = g_data::base;
		//auto Peb = __readgsqword(0x60);


		//uint64_t rax = baseModuleAddr, rbx = baseModuleAddr, rcx = baseModuleAddr, rdx = baseModuleAddr, rdi = baseModuleAddr, rsi = baseModuleAddr, r8 = baseModuleAddr, r9 = baseModuleAddr, r10 = baseModuleAddr, r11 = baseModuleAddr, r12 = baseModuleAddr, r13 = baseModuleAddr, r14 = baseModuleAddr, r15 = baseModuleAddr;
		//rdi = boneIndex;
		//rcx = rdi * 0x13C8;
		//rax = 0x1B5C5E9652FDACE7;               //mov rax, 0x1B5C5E9652FDACE7
		//rax = _umul128(rax, rcx, (uintptr_t*)&rdx);             //mul rcx
		//r11 = baseModuleAddr;           //lea r11, [0xFFFFFFFFFDC09168]
		//r10 = 0x19E9C4E0C9861BBD;               //mov r10, 0x19E9C4E0C9861BBD
		//rdx >>= 0xA;            //shr rdx, 0x0A
		//rax = rdx * 0x256D;             //imul rax, rdx, 0x256D
		//rcx -= rax;             //sub rcx, rax
		//rax = 0x4F9FF77A70376427;               //mov rax, 0x4F9FF77A70376427
		//r8 = rcx * 0x256D;              //imul r8, rcx, 0x256D
		//rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
		//rax = r8;               //mov rax, r8
		//rax -= rdx;             //sub rax, rdx
		//rax >>= 0x1;            //shr rax, 0x01
		//rax += rdx;             //add rax, rdx
		//rax >>= 0xD;            //shr rax, 0x0D
		//rax = rax * 0x30D1;             //imul rax, rax, 0x30D1
		//r8 -= rax;              //sub r8, rax
		//rax = 0x70381C0E070381C1;               //mov rax, 0x70381C0E070381C1
		//rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
		//rax = 0x624DD2F1A9FBE77;                //mov rax, 0x624DD2F1A9FBE77
		//rdx >>= 0x6;            //shr rdx, 0x06
		//rcx = rdx * 0x92;               //imul rcx, rdx, 0x92
		//rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
		//rax = r8;               //mov rax, r8
		//rax -= rdx;             //sub rax, rdx
		//rax >>= 0x1;            //shr rax, 0x01
		//rax += rdx;             //add rax, rdx
		//rax >>= 0x6;            //shr rax, 0x06
		//rcx += rax;             //add rcx, rax
		//rax = rcx * 0xFA;               //imul rax, rcx, 0xFA
		//rcx = r8 * 0xFC;                //imul rcx, r8, 0xFC
		//rcx -= rax;             //sub rcx, rax
		//rax = *(uint16_t*)(rcx + r11 * 1 + 0x8E44820);                //movzx eax, word ptr [rcx+r11*1+0x8E44820]
		//r8 = rax * 0x13C8;              //imul r8, rax, 0x13C8
		//rax = r10;              //mov rax, r10
		//rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
		//rcx = r8;               //mov rcx, r8
		//rax = r10;              //mov rax, r10
		//rcx -= rdx;             //sub rcx, rdx
		//rcx >>= 0x1;            //shr rcx, 0x01
		//rcx += rdx;             //add rcx, rdx
		//rcx >>= 0xC;            //shr rcx, 0x0C
		//rcx = rcx * 0x1D0F;             //imul rcx, rcx, 0x1D0F
		//r8 -= rcx;              //sub r8, rcx
		//r9 = r8 * 0x3981;               //imul r9, r8, 0x3981
		//rax = _umul128(rax, r9, (uintptr_t*)&rdx);              //mul r9
		//rax = r9;               //mov rax, r9
		//rax -= rdx;             //sub rax, rdx
		//rax >>= 0x1;            //shr rax, 0x01
		//rax += rdx;             //add rax, rdx
		//rax >>= 0xC;            //shr rax, 0x0C
		//rax = rax * 0x1D0F;             //imul rax, rax, 0x1D0F
		//r9 -= rax;              //sub r9, rax
		//rax = 0xD79435E50D79435F;               //mov rax, 0xD79435E50D79435F
		//rax = _umul128(rax, r9, (uintptr_t*)&rdx);              //mul r9
		//rax = 0xA6810A6810A6811;                //mov rax, 0xA6810A6810A6811
		//rdx >>= 0x6;            //shr rdx, 0x06
		//rcx = rdx * 0x4C;               //imul rcx, rdx, 0x4C
		//rax = _umul128(rax, r9, (uintptr_t*)&rdx);              //mul r9
		//rax = r9;               //mov rax, r9
		//rax -= rdx;             //sub rax, rdx
		//rax >>= 0x1;            //shr rax, 0x01
		//rax += rdx;             //add rax, rdx
		//rax >>= 0x6;            //shr rax, 0x06
		//rcx += rax;             //add rcx, rax
		//rax = rcx * 0xF6;               //imul rax, rcx, 0xF6
		//rcx = r9 * 0xF8;                //imul rcx, r9, 0xF8
		//rcx -= rax;             //sub rcx, rax
		//r15 = *(uint16_t*)(rcx + r11 * 1 + 0x8E4D6C0);                //movsx r15d, word ptr [rcx+r11*1+0x8E4D6C0]
		//return r15;
	}


	player_t get_player(int i)
	{
		uint64_t decryptedPtr = get_client_info();

		if (is_valid_ptr (decryptedPtr))
		{
			uint64_t client_info = get_client_info_base();

			if (is_valid_ptr(client_info))
			{
				return player_t(client_info + (i * player_info::size));
			}
		}
		return player_t(NULL);
	}

	//player_t player_t
	
	//player_t get_local_player()
	//{
	//	auto addr = sdk::get_client_info_base() + (get_local_index() * player_info::size);
	//	if (is_bad_ptr(addr)) return 0;
	//	return addr;


	//}

	player_t get_local_player()
	{
		uint64_t decryptedPtr = get_client_info();

		if (is_bad_ptr(decryptedPtr))return player_t(NULL);

			auto local_index = *(uintptr_t*)(decryptedPtr + player_info::local_index);
			if (is_bad_ptr(local_index))return player_t(NULL);
			auto index = *(int*)(local_index + player_info::local_index_pos);
			return get_player(index);
		
		
	}

	name_t* get_name_ptr(int i)
	{
		uint64_t bgs = *(uint64_t*)(g_data::base + client::name_array);

		if (bgs)
		{
			name_t* pClientInfo = (name_t*)(bgs + client::name_array_padding + ((i + i * 8) << 4));

			if (is_bad_ptr(pClientInfo))return 0;
			else
			return pClientInfo;
			
		}
		return 0;
	}

	refdef_t* get_refdef()
	{
		uint32_t crypt_0 = *(uint32_t*)(g_data::base + view_port::refdef_ptr);
		uint32_t crypt_1 = *(uint32_t*)(g_data::base + view_port::refdef_ptr + 0x4);
		uint32_t crypt_2 = *(uint32_t*)(g_data::base + view_port::refdef_ptr + 0x8);
		// lower 32 bits
		uint32_t entry_1 = (uint32_t)(g_data::base + view_port::refdef_ptr);
		uint32_t entry_2 = (uint32_t)(g_data::base + view_port::refdef_ptr + 0x4);
		// decryption
		uint32_t _low = entry_1 ^ crypt_2;
		uint32_t _high = entry_2 ^ crypt_2;
		uint32_t low_bit = crypt_0 ^ _low * (_low + 2);
		uint32_t high_bit = crypt_1 ^ _high * (_high + 2);
		auto ret = (refdef_t*)(((QWORD)high_bit << 32) + low_bit);
		if (is_bad_ptr(ret)) return 0;
		else
			return ret;
	}

	Vector3 get_camera_pos()
	{
		Vector3 pos = Vector3{};

		auto camera_ptr = *(uint64_t*)(g_data::base + view_port::camera_ptr);

		if (is_bad_ptr(camera_ptr))return pos;
		
		
		pos = *(Vector3*)(camera_ptr + view_port::camera_pos);
		if (pos.IsZero())return {};
		else
		return pos;
	}

	std::string get_player_name(int i)
	{
		uint64_t bgs = *(uint64_t*)(g_data::base + client::name_array);

		if (is_bad_ptr(bgs))return NULL;


		if (bgs)
		{
			name_t* clientInfo_ptr = (name_t*)(bgs + client::name_array_padding + (i * 0xD0));
			if (is_bad_ptr(clientInfo_ptr))return NULL;

			int length = strlen(clientInfo_ptr->name);
			for (int j = 0; j < length; ++j)
			{
				char ch = clientInfo_ptr->name[j];
				bool is_english = ch >= 0 && ch <= 127;
				if (!is_english)
					return xorstr("Player");
			}
			return clientInfo_ptr->name;
		}
		return xorstr("Player");
	}

	
    bool bones_to_screen(Vector3* BonePosArray, Vector2* ScreenPosArray, const long Count)
    {
        for (long i = 0; i < Count; ++i)
        {
            if (!world(BonePosArray[i], &ScreenPosArray[i]))
                return false;
        }
        return true;
    }



	bool get_bone_by_player_index(int i, int bone_id, Vector3* Out_bone_pos)
	{
		uint64_t decrypted_ptr = get_bone_ptr();

		if (is_bad_ptr(decrypted_ptr))return false;
		
			unsigned short index = get_bone_index(i);
			if (index != 0)
			{
				uint64_t bone_ptr = *(uint64_t*)(decrypted_ptr + (index * bones::size) + 0xC0);

				if (is_bad_ptr(bone_ptr))return false;
				
					Vector3 bone_pos = *(Vector3*)(bone_ptr + (bone_id * 0x20) + 0x10);

					if (bone_pos.IsZero())return false;

					uint64_t client_info = get_client_info();

					if (is_bad_ptr(client_info))return false;

					
					
						Vector3 BasePos = *(Vector3*)(client_info + bones::bone_base_pos);

						if (BasePos.IsZero())return false;

						bone_pos.x += BasePos.x;
						bone_pos.y += BasePos.y;
						bone_pos.z += BasePos.z;

						*Out_bone_pos = bone_pos;
						return true;
					
				
			}
		
		return false;
	}

	float valuesRecoilBackup[962][60];
	float valuesSpreadBackup[962][22];
	void no_spread()
	{
		WeaponCompleteDefArr* weapons = (WeaponCompleteDefArr*)(g_data::base + bones::weapon_definitions);
		if (globals::b_spread && in_game)
		{

			for (int count = 0; count < 962; count++)
			{
				if (weapons->weaponCompleteDefArr[count]->weapDef)
				{

					valuesSpreadBackup[count][0] = weapons->weaponCompleteDefArr[count]->weapDef->fHipSpreadDuckedDecay;
					valuesSpreadBackup[count][1] = weapons->weaponCompleteDefArr[count]->weapDef->fHipSpreadProneDecay;
					valuesSpreadBackup[count][2] = weapons->weaponCompleteDefArr[count]->weapDef->hipSpreadSprintDecay;
					valuesSpreadBackup[count][3] = weapons->weaponCompleteDefArr[count]->weapDef->hipSpreadInAirDecay;
					valuesSpreadBackup[count][4] = weapons->weaponCompleteDefArr[count]->weapDef->fHipReticleSidePos;
					valuesSpreadBackup[count][5] = weapons->weaponCompleteDefArr[count]->weapDef->fAdsIdleAmount;
					valuesSpreadBackup[count][6] = weapons->weaponCompleteDefArr[count]->weapDef->fHipIdleAmount;
					valuesSpreadBackup[count][7] = weapons->weaponCompleteDefArr[count]->weapDef->adsIdleSpeed;
					valuesSpreadBackup[count][8] = weapons->weaponCompleteDefArr[count]->weapDef->hipIdleSpeed;
					valuesSpreadBackup[count][9] = weapons->weaponCompleteDefArr[count]->weapDef->fIdleCrouchFactor;
					valuesSpreadBackup[count][10] = weapons->weaponCompleteDefArr[count]->weapDef->fIdleProneFactor;
					valuesSpreadBackup[count][11] = weapons->weaponCompleteDefArr[count]->weapDef->fGunMaxPitch;
					valuesSpreadBackup[count][12] = weapons->weaponCompleteDefArr[count]->weapDef->fGunMaxYaw;
					valuesSpreadBackup[count][13] = weapons->weaponCompleteDefArr[count]->weapDef->fViewMaxPitch;
					valuesSpreadBackup[count][14] = weapons->weaponCompleteDefArr[count]->weapDef->fViewMaxYaw;
					valuesSpreadBackup[count][15] = weapons->weaponCompleteDefArr[count]->weapDef->adsIdleLerpStartTime;
					valuesSpreadBackup[count][16] = weapons->weaponCompleteDefArr[count]->weapDef->adsIdleLerpTime;
					valuesSpreadBackup[count][17] = weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadMin;
					valuesSpreadBackup[count][18] = weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadMax;
					valuesSpreadBackup[count][19] = weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadDecayRate;
					valuesSpreadBackup[count][20] = weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadFireAdd;
					valuesSpreadBackup[count][21] = weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadTurnAdd;
					weapons->weaponCompleteDefArr[22]->weapDef->ballisticInfo.muzzleVelocity;
					// WRITE

					weapons->weaponCompleteDefArr[count]->weapDef->fHipSpreadDuckedDecay = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fHipSpreadProneDecay = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->hipSpreadSprintDecay = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->hipSpreadInAirDecay = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fHipReticleSidePos = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fAdsIdleAmount = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fHipIdleAmount = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->adsIdleSpeed = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->hipIdleSpeed = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fIdleCrouchFactor = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fIdleProneFactor = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fGunMaxPitch = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fGunMaxYaw = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fViewMaxPitch = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fViewMaxYaw = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->adsIdleLerpStartTime = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->adsIdleLerpTime = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadMin = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadMax = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadDecayRate = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadFireAdd = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadTurnAdd = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->ballisticInfo.muzzleVelocity = 0.0f;
				}
			}
		}
		else
		{
			for (int count = 0; count < 962; count++)
			{
				if (weapons->weaponCompleteDefArr[count]->weapDef)
				{
					weapons->weaponCompleteDefArr[count]->weapDef->fHipSpreadDuckedDecay = valuesSpreadBackup[count][0];
					weapons->weaponCompleteDefArr[count]->weapDef->fHipSpreadProneDecay = valuesSpreadBackup[count][1];
					weapons->weaponCompleteDefArr[count]->weapDef->hipSpreadSprintDecay = valuesSpreadBackup[count][2];
					weapons->weaponCompleteDefArr[count]->weapDef->hipSpreadInAirDecay = valuesSpreadBackup[count][3];
					weapons->weaponCompleteDefArr[count]->weapDef->fHipReticleSidePos = valuesSpreadBackup[count][4];
					weapons->weaponCompleteDefArr[count]->weapDef->fAdsIdleAmount = valuesSpreadBackup[count][5];
					weapons->weaponCompleteDefArr[count]->weapDef->fHipIdleAmount = valuesSpreadBackup[count][6];
					weapons->weaponCompleteDefArr[count]->weapDef->adsIdleSpeed = valuesSpreadBackup[count][7];
					weapons->weaponCompleteDefArr[count]->weapDef->hipIdleSpeed = valuesSpreadBackup[count][8];
					weapons->weaponCompleteDefArr[count]->weapDef->fIdleCrouchFactor = valuesSpreadBackup[count][9];
					weapons->weaponCompleteDefArr[count]->weapDef->fIdleProneFactor = valuesSpreadBackup[count][10];
					weapons->weaponCompleteDefArr[count]->weapDef->fGunMaxPitch = valuesSpreadBackup[count][11];
					weapons->weaponCompleteDefArr[count]->weapDef->fGunMaxYaw = valuesSpreadBackup[count][12];
					weapons->weaponCompleteDefArr[count]->weapDef->fViewMaxPitch = valuesSpreadBackup[count][13];
					weapons->weaponCompleteDefArr[count]->weapDef->fViewMaxYaw = valuesSpreadBackup[count][14];
					weapons->weaponCompleteDefArr[count]->weapDef->adsIdleLerpStartTime = valuesSpreadBackup[count][15];
					weapons->weaponCompleteDefArr[count]->weapDef->adsIdleLerpTime = valuesSpreadBackup[count][16];
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadMin = valuesSpreadBackup[count][17];
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadMax = valuesSpreadBackup[count][18];
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadDecayRate = valuesSpreadBackup[count][19];
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadFireAdd = valuesSpreadBackup[count][20];
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadTurnAdd = valuesSpreadBackup[count][21];

				}
			}
		}
	}

	void no_recoil()
	{
		uint64_t characterInfo_ptr = get_client_info();

		if (characterInfo_ptr)
		{
			// up, down
			uint64_t r12 = characterInfo_ptr;
			r12 += player_info::recoil_offset;
			uint64_t rsi = r12 + 0x4;
			DWORD edx = *(uint64_t*)(r12 + 0xC);
			DWORD ecx = (DWORD)r12;
			ecx ^= edx;
			DWORD eax = (DWORD)((uint64_t)ecx + 0x2);
			eax *= ecx;
			ecx = (DWORD)rsi;
			ecx ^= edx;
			DWORD udZero = eax;
			//left, right
			eax = (DWORD)((uint64_t)ecx + 0x2);
			eax *= ecx;
			DWORD lrZero = eax;
			*(DWORD*)(r12) = udZero;
			*(DWORD*)(rsi) = lrZero;

		}
	}

	int get_player_health(int i)
	{
		uint64_t bgs = *(uint64_t*)(g_data::base + client::name_array);

		if (bgs)
		{
			name_t* pClientInfo = (name_t*)(bgs + client::name_array_padding  +(i * 0xD0));

			if (pClientInfo)
			{
				return pClientInfo->health;
			}
		}
		return 0;
	}

	void start_tick()
	{
		static DWORD lastTick = 0;
		DWORD t = GetTickCount();
		bUpdateTick = lastTick < t;

		if (bUpdateTick)
			lastTick = t + nTickTime;
	}

	void update_vel_map(int index, Vector3 vPos)
	{
		if (!bUpdateTick)
			return;

		velocityMap[index].delta = vPos - velocityMap[index].lastPos;
		velocityMap[index].lastPos = vPos;
	}

	void clear_map()
	{
		if (!velocityMap.empty()) { velocityMap.clear(); }
	}

	Vector3 get_speed(int index)
	{
		return velocityMap[index].delta;
	}

	Vector3 get_prediction(int index, Vector3 source, Vector3 destination)
	{
		auto local_velocity = get_speed(local_index());
		auto target_velocity = get_speed(index);

		const auto distance = source.distance_to(destination);
		const auto travel_time = distance / globals::bullet_speed;
		auto pred_destination = destination + (target_velocity - local_velocity) * travel_time;
		/*position.x += travel_time * final_speed.x;
		position.y += travel_time * final_speed.y;
		position.z += 0.5 * globals::bullet_gravity * travel_time * travel_time;
		return position;*/

		pred_destination.z += 0.5f * std::fabsf(globals::bullet_gravity) * travel_time;

		return pred_destination;
	}

	/*int get_client_count()
	{
		auto cl_info = get_client_info();
		if (cl_info)
		{
			auto client_ptr = *(uint64_t*)(cl_info + player_info::local_index);
			if (client_ptr)
			{
				return *(unsigned int*)(client_ptr + 0x1C);
			}
		}

		return 0;
	}*/

	Result MidnightSolver(float a, float b, float c)
	{
		Result res;

		double subsquare = b * b - 4 * a * c;

		if (subsquare < 0)
		{
			res.hasResult = false;
			return res;
		}
		else
		{
			res.hasResult = true,
			res.a = (float)((-b + sqrt(subsquare)) / (2 * a));
			res.b = (float)((-b - sqrt(subsquare)) / (2 * a));
		}
		return res;
	}

	Vector3 prediction_solver(Vector3 local_pos, Vector3 position, int index, float bullet_speed)
	{
		Vector3 aimPosition = Vector3().Zero();
		auto target_speed = get_speed(index);

		local_pos -= position; 

		float a = (target_speed.x * target_speed.x) + (target_speed.y * target_speed.y) + (target_speed.z * target_speed.z) - ((bullet_speed * bullet_speed) * 100);
		float b = (-2 * local_pos.x * target_speed.x) + (-2 * local_pos.y * target_speed.y) + (-2 * local_pos.z * target_speed.z);
		float c = (local_pos.x * local_pos.x) + (local_pos.y * local_pos.y) + (local_pos.z * local_pos.z);

		local_pos += position; 

		Result r = MidnightSolver(a, b, c);

		if (r.a >= 0 && !(r.b >= 0 && r.b < r.a))
		{
			aimPosition = position + target_speed * r.a;
		}
		else if (r.b >= 0)
		{
			aimPosition = position + target_speed * r.b;
		}

		return aimPosition;
	
	}

	uintptr_t get_visible_base()
	{
		for (int32_t j = 4000; j >= 0; --j)
		{
			uintptr_t n_index = (j + (j << 2)) << 0x6;
			uintptr_t vis_base = *(uintptr_t*)(g_data::base + bones::distribute);

			if (!vis_base)
				continue;

			uintptr_t vis_base_ptr = vis_base + n_index;
			uintptr_t cmp_function = *(uintptr_t*)(vis_base_ptr + 0x90);

			if (!cmp_function)
				continue;

			uintptr_t about_visible = g_data::base + bones::visible;
			if (cmp_function == about_visible)
			{
				return vis_base_ptr;
			}
		}
		return NULL;
	}

	bool update_visible_addr(int i)
	{
		auto vis_base_ptr = get_visible_base();

		if (vis_base_ptr)
		{
			uint64_t visible_head_offset = *(uint64_t*)(vis_base_ptr + 0x108);

			if (visible_head_offset)
			{
				auto visible_flag = decrypt_visible_flag(i, visible_head_offset);

				if (visible_flag == 3)
				{
					current_visible_offset = vis_base_ptr;
					return true;
				}
			}
		}
		return false;
	}

	

	// player class methods
	bool player_t::is_valid() {
		if (is_bad_ptr(address))return 0;

		return *(bool*)((uintptr_t)address + player_info::valid);
	}

	bool player_t::is_dead() {
		if (is_bad_ptr(address))return 0;

		auto dead1 = *(bool*)((uintptr_t)address + player_info::dead_1);
		auto dead2 = *(bool*)((uintptr_t)address + player_info::dead_2);
		return dead1 || dead2;
	}

	int player_t::team_id() {

		if (is_bad_ptr(address))return 0;
		return *(int*)((uintptr_t)address + player_info::team_id);
	}

	int player_t::get_stance() {
		
		if (is_bad_ptr(address))return 4;
		auto ret = *(int*)((uintptr_t)address + player_info::stance);
	

		return ret;
	}


	float player_t::get_rotation()
	{
		if (is_bad_ptr(address))return 0;
		auto local_pos_ptr = *(uintptr_t*)((uintptr_t)address + player_info::position_ptr);

		if (is_bad_ptr(local_pos_ptr))return 0;

		auto rotation = *(float*)(local_pos_ptr + 0x58);

		if (rotation < 0)
			rotation = 360.0f - (rotation * -1);

		rotation += 90.0f;

		if (rotation >= 360.0f)
			rotation = rotation - 360.0f;

		return rotation;
	}

	Vector3 player_t::get_pos() 
	{
		if (is_bad_ptr(address))return {};
		auto local_pos_ptr = *(uintptr_t*)((uintptr_t)address + player_info::position_ptr);

		if (is_bad_ptr(local_pos_ptr))return{};
		else
			return *(Vector3*)(local_pos_ptr + 0x40);
		return Vector3{}; 


	}

	uint32_t player_t::get_index()
	{
		if (is_bad_ptr(this->address))return 0;

		auto cl_info_base = get_client_info_base();
		if (is_bad_ptr(cl_info_base))return 0;
		
		
	return ((uintptr_t)this->address - cl_info_base) / player_info::size;
		
	
	}

	bool player_t::is_visible()
	{
		if (is_bad_ptr(g_data::visible_base))return false;

		if (is_bad_ptr(this->address))return false;
		
			uint64_t VisibleList =*(uint64_t*)(g_data::visible_base + 0x108);
			if (is_bad_ptr(VisibleList))
				return false;

			uint64_t rdx = VisibleList + (player_t::get_index() * 9 + 0x14E) * 8;
			if (is_bad_ptr(rdx))
				return false;

			DWORD VisibleFlags = (rdx + 0x10) ^ *(DWORD*)(rdx + 0x14);
			if (!VisibleFlags)
				return false;

			DWORD v511 = VisibleFlags * (VisibleFlags + 2);
			if (!v511)
				return false;

			BYTE VisibleFlags1 = *(DWORD*)(rdx + 0x10) ^ v511 ^ BYTE1(v511);
			if (VisibleFlags1 == 3) {
				return true;
			}
		
		return false;
	}
	



}

