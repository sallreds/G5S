#include <Common.h>

#include <System/Strawberry.h>

#include <Scripts/CleanWorld.h>
#include <Scripts/Game.h>

#include <shv/main.h>
#include <shv/natives.h>

NAMESPACE_BEGIN;

Strawberry* _pGame = nullptr;

Strawberry::Strawberry(HMODULE hInstance)
{
	m_hInstance = hInstance;

	memset(m_keyStates, 0, sizeof(m_keyStates));

	scriptRegister(hInstance, scriptCleanWorld);
	scriptRegister(hInstance, scriptGame);
}

Strawberry::~Strawberry()
{
	scriptUnregister(m_hInstance);
}

void Strawberry::Initialize()
{
	logWrite("Client initializing.");

	PED::ADD_RELATIONSHIP_GROUP("SYNCPED", (Hash*)&m_pedRelGroup);

	m_player.Initialize();
	m_network.Initialize();
	m_chat.Initialize();
}

static int TestGetGameTimer()
{
	return _pGame->m_gameTime;
}

void Strawberry::Update()
{
	m_gameTime = GAMEPLAY::GET_GAME_TIMER();

	m_player.Update();
	m_network.Update();

	if (m_network.IsConnected()) {
		m_chat.Update();
	}
}

void Strawberry::OnConnected()
{
	UI::_SET_NOTIFICATION_TEXT_ENTRY("CELL_EMAIL_BCON");
	UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME("~g~Connected");
	UI::_DRAW_NOTIFICATION(false, true);
}

void Strawberry::OnDisconnected()
{
	m_chat.Clear();

	//TODO: Also delete local entities (m_entitiesLocal in Strawberry?)
	m_network.ClearEntities();

	UI::_SET_NOTIFICATION_TEXT_ENTRY("CELL_EMAIL_BCON");
	UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME("~r~Disconnected");
	UI::_DRAW_NOTIFICATION(false, true);
}

void Strawberry::OnKeyDown(uint32_t key)
{
	assert(key < 256);
	if (key >= 256) {
		logWrite("OnKeyDown out of range: %u", key);
		return;
	}

	m_keyStates[key] = 1;

	if (key == VK_F3) {
		m_network.Connect(m_settings.ConnectToHost.c_str(), m_settings.ConnectToPort);
	} else if (key == VK_F4) {
		m_network.Disconnect();
	} else if (key == VK_F5) {
		int veh = PED::GET_VEHICLE_PED_IS_IN(m_player.GetLocalHandle(), false);
		if (veh > 0) {
			VEHICLE::SET_VEHICLE_FIXED(veh);
		}
	}

	if (m_network.IsConnected()) {
		m_chat.OnKeyDown(key);
	}
}

void Strawberry::OnKeyUp(uint32_t key)
{
	assert(key < 256);
	if (key >= 256) {
		logWrite("OnKeyUp out of range: %u", key);
		return;
	}

	m_keyStates[key] = 0;
}

bool Strawberry::IsKeyDown(uint8_t key)
{
	return (m_keyStates[key] == 1);
}

NAMESPACE_END;
