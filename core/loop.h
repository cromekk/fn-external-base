#pragma once

static auto get_uworld( ) -> uintptr_t
{
	constexpr uint64_t MASK = 0x93F1FA5800000000ULL;

	uint64_t raw = request->read<uint64_t>( mem::base + offsets::UWORLD );
	if ( !raw || raw == MASK )
		return 0;

	return static_cast< uintptr_t >( _byteswap_uint64( raw ^ MASK ) );
}

struct TArrayData {
	uintptr_t Data;
	int Count;
};

void gameCache( )
{
	cache::closestDistance = FLT_MAX;
	cache::closestMesh = NULL;

	cache::gWorld = get_uworld( );
	if ( !cache::gWorld ) return;

	cache::gameInstance = request->read<uintptr_t>( cache::gWorld + offsets::GAME_INSTANCE );
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

	cache::gameState = request->read<uintptr_t>( cache::gWorld + offsets::GAME_STATE );
	if ( cache::gameState )
	{
		TArrayData pArray = request->read<TArrayData>( cache::gameState + offsets::PLAYER_ARRAY );
		cache::playerArray = pArray.Data;
		cache::playerCount = pArray.Count;
	}
}
