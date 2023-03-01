#include <Windows.h>
#include <stdio.h>
#include <iostream>
#include <string>  
#include <vector>
#include <math.h>
#include "Vector.h"

// Definitions for accessing members of GameObject
#define STR_MERGE_IMPL(a, b) a##b
#define STR_MERGE(a, b) STR_MERGE_IMPL(a, b)
#define MAKE_PAD(size) STR_MERGE(_pad, __COUNTER__)[size]
#define DEFINE_MEMBER_N(type, name, offset) struct {unsigned char MAKE_PAD(offset); type name;};

// GameObject class
class GameObject {
public:
	// Constructor
	GameObject() { position = Vector3(0,0,0); team = 100; visible = false; };

	union {
		DEFINE_MEMBER_N(float, AttackRange, 0x13A4);
		DEFINE_MEMBER_N(short, team, 0x34);
		DEFINE_MEMBER_N(bool, visible, 0x274);
		DEFINE_MEMBER_N(Vector3, position, 0x1DC);
		DEFINE_MEMBER_N(bool, targetable, 0xD04);
	};
};

// Offsets and addresses (for free!)
// Learn how to get these from part 1, can xref and make sigs
DWORD HeroList = 0x18C6BBC;
DWORD oLocalPlayer = 0x3163080;

DWORD oChatClient = 0x3163F94;
DWORD oChatOpen = 0x6BC;

DWORD fGetAttackDelay = 0x296E40;
DWORD fGetAttackCastDelay = 0x296D40;
DWORD fPrintChat = 0x5E0E10;
DWORD fNewIssueOrder = 0x6786D0;
DWORD fIsAlive = 0x190530;

DWORD oHudInstance = 0x18C6B24;

DWORD oRenderer = 0x318F6A0;
DWORD oRendererWidth = 0x8;
DWORD oRendererHeight = 0xC;
DWORD oViewProjMatrices = 0x3189D00;

// Just approximate your ping, I haven't bothered implementing the GetPing() function
double Ping = 50;

// Timestamps to keep track of what you're doing
double lastAttack = 0;
double lastMove = 0;
double lastLoop = 0;

// WorldToScreen variables
int width = 0;
int height = 0;

float viewMatrix[16];
float projMatrix[16];
float viewProjMatrix[16];

// For generating the ViewProjMatrix
void MultiplyMatrices(float* out, float* a, int row1, int col1, float* b, int row2, int col2) {
	int size = row1 * col2;
	for (int i = 0; i < row1; i++) {
		for (int j = 0; j < col2; j++) {
			float sum = 0.f;
			for (int k = 0; k < col1; k++)
				sum = sum + a[i * col1 + k] * b[k * col2 + j];
			out[i * col2 + j] = sum;
		}
	}
}

// Load all relevant values
void LoadW2S() {
	char buff[128];

	DWORD_PTR renderBase = (*(DWORD*)((DWORD)GetModuleHandle(NULL) + oRenderer));

	memcpy(buff, (void*)(renderBase), 128);
	width = *reinterpret_cast<int*>(buff + oRendererWidth);
	height = *reinterpret_cast<int*>(buff + oRendererHeight);

	memcpy(buff, (void*)((DWORD)GetModuleHandle(NULL) + oViewProjMatrices), 128);
	memcpy(viewMatrix, buff, 16 * sizeof(float));
	memcpy(projMatrix, &buff[16 * sizeof(float)], 16 * sizeof(float));

	MultiplyMatrices(viewProjMatrix, viewMatrix, 4, 4, projMatrix, 4, 4);
}

// External WorldToScreen function. I've found it easier/better than internal idk why
Vector2 WorldToScreen(const Vector3& pos) {

	// Initialize the output vector with zeroes
	Vector2 out = { 0.f, 0.f };
	// Get the dimensions of the screen
	Vector2 screen = { (float)width, (float)height };

	// Compute the clip coordinates of the given position
	Vector4 clipCoords;
	clipCoords.x = pos.x * viewProjMatrix[0] + pos.y * viewProjMatrix[4] + pos.z * viewProjMatrix[8] + viewProjMatrix[12];
	clipCoords.y = pos.x * viewProjMatrix[1] + pos.y * viewProjMatrix[5] + pos.z * viewProjMatrix[9] + viewProjMatrix[13];
	clipCoords.z = pos.x * viewProjMatrix[2] + pos.y * viewProjMatrix[6] + pos.z * viewProjMatrix[10] + viewProjMatrix[14];
	clipCoords.w = pos.x * viewProjMatrix[3] + pos.y * viewProjMatrix[7] + pos.z * viewProjMatrix[11] + viewProjMatrix[15];

	// If the clip coordinates have a negative w-value, set it to 1
	if (clipCoords.w < 1.0f)
		clipCoords.w = 1.f;

	// Compute the normalized device coordinates of the given position
	Vector2 M;
	M.x = clipCoords.x / clipCoords.w;
	M.y = clipCoords.y / clipCoords.w;

	// Compute the screen coordinates of the given position
	out.x = (screen.x / 2.f * M.x) + (M.x + screen.x / 2.f);
	out.y = -(screen.y / 2.f * M.y) + (M.y + screen.y / 2.f);

	// Return the computed screen coordinates
	return out;
}

// Getting mouse position from memory...
Vector3 GetMousePos()
{
	DWORD MousePtr = *(DWORD*)(*(DWORD*)((DWORD)GetModuleHandle(NULL) + oHudInstance) + 0x14) + 0x1C;

	Vector3 mousePos;

	mousePos.x = *(float*)(MousePtr + 0x0);
	mousePos.y = *(float*)(MousePtr + 0x4);
	mousePos.z = *(float*)(MousePtr + 0x8);

	return mousePos;
}

// Main orbwalker function
void __stdcall OrbWalker() {

	// Base Address + LocalPlayer + Champions vector
	// Standard setup
	DWORD BaseAddress = (DWORD)GetModuleHandle(NULL);
	GameObject* LocalPlayer = (GameObject*)(*(DWORD*)(BaseAddress + oLocalPlayer));
	std::vector<GameObject*> Champions;

	// This will be our target!
	GameObject* target = new GameObject();
	
	// Defining the functions we care about
	typedef bool(__thiscall* fnIsAlive)(GameObject* obj);
	typedef float(__cdecl* fnGetAttackDelay)(GameObject* obj);
	typedef float(__cdecl* fnGetAttackCastDelay)(GameObject* obj);
	typedef void(__thiscall* fnPrintChat)(DWORD, const char*, int);
	typedef int(__thiscall* fnNewIssueOrder)(DWORD HudThisPtr, int state, int IsAttack, int IsAttackCommand, int x, int y, int AttackAndMove);

	// Function objects
	fnGetAttackDelay GetAttackDelay = (fnGetAttackDelay)(BaseAddress + fGetAttackDelay);
	fnGetAttackCastDelay GetAttackCastDelay = (fnGetAttackCastDelay)(BaseAddress + fGetAttackCastDelay);
	fnPrintChat PrintChat = (fnPrintChat)(BaseAddress + fPrintChat);
	fnNewIssueOrder NewIssueOrder = (fnNewIssueOrder)(BaseAddress + fNewIssueOrder);
	fnIsAlive IsAlive = (fnIsAlive)(BaseAddress + fIsAlive);

	// Testing that we are properly injected
	PrintChat(BaseAddress + oChatClient, "Dunkrius' League of Legends Orbwalker", 0xFFFFFF);

	// Orbwalking
	while (true) {

		// Little delay so your computer doesn't die
		if (GetAsyncKeyState(VK_SPACE) && GetTickCount64() - lastLoop > 3) {

			// Checking if chat is open! Don't want to try spacegliding while typing...
			bool isChatOpen = *(int*)(*(DWORD*)(BaseAddress + oChatClient) + oChatOpen);

			if (!isChatOpen) {
				// Make sure Champions list is empty
				Champions.clear();

				// Load Champions' positions
				int vectorSize = (DWORD)(*(DWORD*)(*(DWORD*)(BaseAddress + HeroList) + 0x8));

				for (int i = 0; i < vectorSize; i++) {
					// Pointer to champion from the HeroClient
					GameObject* heroObject = (GameObject*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(BaseAddress + HeroList) + 0x4) + 0x4 * i));

					// Check if it's the closest target that's been found (within reason)
					if (IsAlive(heroObject) && heroObject->targetable && heroObject->visible && heroObject->team != LocalPlayer->team && heroObject->position.DistanceTo(LocalPlayer->position) < 900 && heroObject->position.DistanceTo(LocalPlayer->position) < target->position.DistanceTo(LocalPlayer->position))
						// If so, that's the one!
						target = heroObject;

					// Add the heroObject to the list of champions anyway, you can use it later for other things
					Champions.push_back(heroObject);
				}

				// First check is to make sure that a target actually exists
				// Second check is to make sure that you aren't attacking during your attack's "cooldown"
				if (target->position.x != 0 && GetTickCount64() >= lastAttack + GetAttackDelay(LocalPlayer) * 1000.f) {

					// Load WorldToScreen
					LoadW2S();

					// Attack closest champion
					Vector2 AttackPosition = WorldToScreen(target->position);

					// Send input to attack
					DWORD HUDInput = *(DWORD*)(*(DWORD*)(BaseAddress + oHudInstance) + 0x24);
					NewIssueOrder(HUDInput, 0, 0, true, AttackPosition.x, AttackPosition.y, 0);
					NewIssueOrder(HUDInput, 1, 0, true, AttackPosition.x, AttackPosition.y, 0);

					// Update timer so that you don't try attacking on cooldown
					lastAttack = GetTickCount64();
				}

				// First check is to make sure you aren't moving too often
				// Second check is to make sure you aren't moving in your windup
				else if (GetTickCount64() > lastMove + 30 && GetTickCount64() >= lastAttack + GetAttackCastDelay(LocalPlayer) * 1000.f + Ping + 10) {

					// Load WorldToScreen
					LoadW2S();

					// Click with your mouse's current position
					Vector2 MovePosition = WorldToScreen(GetMousePos());

					// Send an input to right click at mouse position
					DWORD HUDInput = *(DWORD*)(*(DWORD*)(BaseAddress + oHudInstance) + 0x24);
					NewIssueOrder(HUDInput, 0, 0, true, MovePosition.x, MovePosition.y, 0);
					NewIssueOrder(HUDInput, 1, 0, true, MovePosition.x, MovePosition.y, 0);

					// Update timer so that you don't spam inputs and DC
					lastMove = GetTickCount64();
				}
			}
			// Update lastLoop so you don't overwork the thread
			lastLoop = GetTickCount64();
		}
	}
}

BOOL APIENTRY DllMain(HMODULE Module, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {

		// Create a new thread to run our orbwalker in
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)OrbWalker, 0, 0, 0);

		return TRUE;

	}

	else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		return TRUE;
	}
	return FALSE;
}
