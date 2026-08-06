#pragma once

#include <bit>

inline auto get_world( ) -> std::uintptr_t
{
	uint64_t v = request->read<uint64_t>( mem::base + 0x1A6536B0 );
	if ( !v )
		return 0;

	v -= 28021308ull;
	v = _rotl64( v,16 );
	v ^= 0x937B782ull;

	return static_cast< uintptr_t >( v );
}

struct TArrayData {
	uintptr_t Data;
	int Count;
};

void gameCache( )
{
	cache::closestDistance = FLT_MAX;
	cache::closestMesh = NULL;

	cache::uWorld = get_world( );
	if ( !cache::uWorld ) return;

	cache::gameInstance = request->read<uintptr_t>( cache::uWorld + offsets::GAME_INSTANCE );
	if ( !cache::gameInstance ) return;

	uintptr_t localPlayerArray = request->read<uintptr_t>( cache::gameInstance + offsets::LOCAL_PLAYERS );
	if ( !localPlayerArray ) return;

	cache::localPlayers = request->read<uintptr_t>( localPlayerArray );
	if ( !cache::localPlayers ) return;

	cache::playerController = request->read<uintptr_t>( cache::localPlayers + offsets::PLAYER_CONTROLLER );
	if ( !cache::playerController ) return;

	cache::localPawn = request->read<uintptr_t>( cache::playerController + offsets::LOCAL_PAWN );
	if ( cache::localPawn )
	{
		cache::rootComponent = request->read<uintptr_t>( cache::localPawn + offsets::ROOT_COMPONENT );
		cache::playerState = request->read<uintptr_t>( cache::localPawn + offsets::PLAYER_STATE );

		if ( cache::playerState ) {
			cache::myTeamID = request->read<int>( cache::playerState + offsets::TEAM_INDEX );
		}
	}

	cache::gameState = request->read<uintptr_t>( cache::uWorld + offsets::GAME_STATE );
	if ( cache::gameState )
	{
		TArrayData pArray = request->read<TArrayData>( cache::gameState + offsets::PLAYER_ARRAY );
		cache::playerArray = pArray.Data;
		cache::playerCount = pArray.Count;
	}
}
