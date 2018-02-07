#pragma once

namespace Input
{
	enum PlayerID
	{
		PLAYER_1 = 0, PLAYER_2, PLAYER_3, PLAYER_4, NUM_PLAYERS
	};

	enum EInterface
	{
		KEYBOARD = 0,
		MOUSE,
		PAD,
		NUM_INTERFACES
	};

	using KeyID = int;

	static const int NUM_KEYBOARD_KEYS = 256;

	enum EMouseButton
	{
		MOUSE_LEFT = 0,
		MOUSE_RIGHT,
		MOUSE_MIDDLE,
		MOUSE_BUTTONS
	};

	enum EPadButton
	{
		PAD_A = 0,
		PAD_B,
		PAD_X,
		PAD_Y,
		PAD_L1,
		PAD_L2,
		PAD_L3,
		PAD_R1,
		PAD_R2,
		PAD_R3,
		PAD_START,
		PAD_OPTIONS,
		PAD_LEFT,
		PAD_RIGHT,
		PAD_UP,
		PAD_DOWN,
		PAD_LANALOG_X,
		PAD_LANALOG_Y,
		PAD_RANALOG_X,
		PAD_RANALOG_Y,
		PAD_BUTTONS
	};

	struct TButtonDef
	{
		EInterface type;
		int id;
	};
};
