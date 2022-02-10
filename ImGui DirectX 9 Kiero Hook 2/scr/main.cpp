#pragma once
#include "includes.h"
#include "opcodes.h"
#include <string>
#include <Windows.h>
#include <TlHelp32.h>

#ifdef _WIN64
#define GWL_WNDPROC GWLP_WNDPROC
#endif

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

EndScene oEndScene = NULL;
WNDPROC oWndProc;
static HWND window = NULL;
bool attached = false;

template <int size>
bool patch_bytes(uintptr_t addr, const char* data) {
	void* address = reinterpret_cast<void*>(addr);
	DWORD old = 0;
	if (!VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &old))
		return false;
	memcpy(address, (const void*)data, size);
	return VirtualProtect(address, size, old, &old);
}

void InitImGui(LPDIRECT3DDEVICE9 pDevice)
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(pDevice);
}

bool init = false;
bool show = true;
bool cb_antibounce = false;
bool cb_anticheckpoint = false;
bool cb_antiglue = false;
bool cb_antiwater = false;
bool cb_modfly = false;
bool cb_noclip = false;
bool cb_ghost = false;
bool cb_giveaway = false;
bool cb_growz = false;

long __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice)
{
	if (!init)
	{
		InitImGui(pDevice);
		init = true;
	}

	if (GetAsyncKeyState(VK_INSERT) & 1)
		show = !show;

	if (GetAsyncKeyState(VK_END))
	{
		kiero::shutdown();
		return 0;
	}

	if (show)
	{
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("viNTERNAL");

		ImGui::Checkbox("AntiBounce", &cb_antibounce);
		ImGui::Checkbox("AntiCheckpoint", &cb_anticheckpoint);
		ImGui::Checkbox("AntiGlue", &cb_antiglue);
		ImGui::Checkbox("AntiWater", &cb_antiwater);
		ImGui::Checkbox("ModFly", &cb_modfly);
		ImGui::Checkbox("NoClip", &cb_noclip);
		ImGui::Checkbox("Ghost", &cb_ghost);
		ImGui::Checkbox("Giveaway", &cb_giveaway);
		ImGui::Checkbox("Growz", &cb_growz);

		ImGui::End();

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	return oEndScene(pDevice);
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
	DWORD wndProcId;
	GetWindowThreadProcessId(handle, &wndProcId);

	if (GetCurrentProcessId() != wndProcId)
		return TRUE; // skip to next window

	window = handle;
	return FALSE; // window found abort search
}

HWND GetProcessWindow()
{
	window = NULL;
	EnumWindows(EnumWindowsCallback, NULL);
	return window;
}

DWORD WINAPI MainThread(LPVOID lpReserved)
{
	attached = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D9) == kiero::Status::Success)
		{
			kiero::bind(42, (void**)& oEndScene, hkEndScene);
			do
				window = GetProcessWindow();
			while (window == NULL);
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWL_WNDPROC, (LONG_PTR)WndProc);
			attached = true;
		}
	} while (!attached);
	return TRUE;
}

DWORD WINAPI HackThread(HMODULE hModule)
{

	bool antibounce = false;
	bool anticheckpoint = false;
	bool antiglue = false;
	bool antiwater = false;
	bool modfly = false;
	bool noclip = false;
	bool ghost = false;
	bool giveaway = false;
	bool growz = false;

	while (!attached)
	{
		Sleep(100);
	}

	AllocConsole();
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);

	patch_bytes<2>(baseAdd + offset_banbypass, "\x90\x90");

	printf("viNTERNAL successfully injected\n\n");

	while (!GetAsyncKeyState(VK_END))
	{
		if (cb_antibounce && !antibounce)
		{
			patch_bytes<6>( baseAdd + offset_antibounce, "\xE9\x04\x01\x00\x00\x90" );
			printf( "AntiBounce: ON\n" );
			antibounce = true;
		}
		else if ( !cb_antibounce && antibounce )
		{
			patch_bytes<6>( baseAdd + offset_antibounce, bAntiBounce );
			printf("AntiBounce: OFF\n");
			antibounce = false;
		}

		if (cb_anticheckpoint && !anticheckpoint)
		{
			patch_bytes<1>(baseAdd + offset_anticheckpoint, jmp);
			printf("AntiCheckpoint: ON\n");
			anticheckpoint = true;
		}
		else if (!cb_anticheckpoint && anticheckpoint)
		{
			patch_bytes<1>(baseAdd + offset_anticheckpoint, je);
			printf("AntiCheckpoint: OFF\n");
			anticheckpoint = false;
		}

		if (cb_antiglue && !antiglue)
		{
			patch_bytes<1>(baseAdd + offset_antiglue, jmp);
			printf("AntiGlue: ON\n");
			antiglue = true;
		}
		else if (!cb_antiglue && antiglue)
		{
			patch_bytes<1>(baseAdd + offset_antiglue, je);
			printf("AntiGlue: OFF\n");
			antiglue = false;
		}

		if (cb_antiwater && !antiwater)
		{
			patch_bytes<1>(baseAdd + offset_antiwater, jmp);
			printf("AntiWater: ON\n");
			antiwater = true;
		}
		else if (!cb_antiwater && antiwater)
		{
			patch_bytes<1>(baseAdd + offset_antiwater, je);
			printf("AntiWater: OFF\n");
			antiwater = false;
		}

		if ( cb_modfly && !modfly && !(GetAsyncKeyState('S') < 0) )
		{
			patch_bytes<2>( baseAdd + offset_modfly, "\x90\x90" );
			printf( "ModFly: ON\n" );
			modfly = true;
		}
		else if ( (GetKeyState('S') < 0 || !cb_modfly) && modfly )
		{
			if ( !giveaway ) patch_bytes<2>( baseAdd + offset_modfly, bModFly );
			printf( "ModFly: OFF\n" );
			modfly = false;
		}

		if ( cb_noclip && !noclip )
		{
			patch_bytes<2>(baseAdd + offset_noclip, "\x90\x90");
			printf("NoClip: ON\n");
			noclip = true;
		}
		else if ( !cb_noclip && noclip )
		{
			if (!giveaway) patch_bytes<2>(baseAdd + offset_noclip, bNoClip);
			printf("NoClip: OFF\n");
			noclip = false;
		}

		if ( cb_ghost && !ghost )
		{
			patch_bytes<1>( baseAdd + offset_ghost, jmp );
			printf( "Ghost: ON\n" );
			ghost = true;
		}
		else if ( !cb_ghost && ghost )
		{
			if (!giveaway) patch_bytes<1>( baseAdd + offset_ghost, je );
			printf( "Ghost: OFF\n" );
			ghost = false;
		}

		if ( cb_giveaway && !giveaway && !(GetAsyncKeyState('S') < 0) )
		{
			patch_bytes<2>( baseAdd + offset_modfly, "\x90\x90" );
			patch_bytes<2>( baseAdd + offset_noclip, "\x90\x90" );
			patch_bytes<1>( baseAdd + offset_ghost, jmp);
			printf( "Giveaway: ON\n" );
			giveaway = true;
		}
		else if ( GetKeyState('S') < 0 && giveaway )
		{
			patch_bytes<2>( baseAdd + offset_modfly, bModFly );
			printf( "Giveaway: ModFly OFF\n" );
			giveaway = false;
		}
		else if (!cb_giveaway && giveaway)
		{
			if (!cb_modfly) patch_bytes<2>(baseAdd + offset_modfly, bModFly);
			if (!cb_noclip) patch_bytes<2>(baseAdd + offset_noclip, bNoClip);
			if (!cb_ghost) patch_bytes<1>(baseAdd + offset_ghost, je);
			printf("Giveaway: OFF\n");
			giveaway = false;
		}

		if (cb_growz && !growz)
		{
			patch_bytes<4>(baseAdd + offset_growz, "\x90\x90\x90\x90");
			printf("Growz: ON\n");
			growz = true;
		}
		else if (!cb_growz && growz)
		{
			patch_bytes<4>(baseAdd + offset_growz, bGrowz);
			printf("Growz: OFF\n");
			growz = false;
		}

		Sleep(25);
	}

	FreeConsole();
	FreeLibraryAndExitThread(hModule, 0);
	return 0;
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hMod);
		baseAdd = (uintptr_t)GetModuleHandle(NULL);
		CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HackThread, hMod, 0, nullptr);
		break;
	case DLL_PROCESS_DETACH:
		kiero::shutdown();
		break;
	}
	return TRUE;
}
