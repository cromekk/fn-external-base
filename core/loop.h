#pragma once

std::uintptr_t decrypt_gworld( )
{
	cache::encrypted = request->read<uintptr_t>(  mem::base + offsets::GWORLD );

	return _byteswap_uint64( cache::encrypted ^ 0x012F546CULL ) - 1274101633ULL;
}

struct TArrayData {
	uintptr_t Data;
	int Count;
};

void gameCache( )
{
	cache::closestDistance = FLT_MAX;
	cache::closestMesh = NULL;

	cache::uWorld = decrypt_gworld( );
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
