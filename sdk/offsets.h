#pragma once

#include <cstdint>

// if any issues with offsets or sth not working dm me on discord @cromekk

class offsets
{
public:
	static constexpr std::uintptr_t GWORLD = 0x1A73C8B0;
	static constexpr std::uintptr_t GAME_INSTANCE = 0x250;
	static constexpr std::uintptr_t LOCAL_PLAYERS = 0x38;
	static constexpr std::uintptr_t PLAYER_CONTROLLER = 0x30;
	static constexpr std::uintptr_t LOCAL_PAWN = 0x318;
	static constexpr std::uintptr_t PAWN_PRIVATE = 0x2E8;
	static constexpr std::uintptr_t ROOT_COMPONENT = 0x1B0;
	static constexpr std::uintptr_t PLAYER_STATE = 0x290;
	static constexpr std::uintptr_t TEAM_INDEX = 0xF31;
	static constexpr std::uintptr_t GAME_STATE = 0x1D8;
	static constexpr std::uintptr_t PLAYER_ARRAY = 0x288;
	static constexpr std::uintptr_t MESH = 0x2F0;
	static constexpr std::uintptr_t COMPONENT_TO_WORLD = 0x1E0;
	static constexpr std::uintptr_t BONE_ARRAY = 0x648;
	static constexpr std::uintptr_t BONE_ARRAY_CACHE = 0x658;
};
