#pragma once

void renderVisuals( )
{
	if ( visual.enable )
	{
		for ( int i = 0; i < cache::playerCount; i++ )
		{
			uintptr_t playerState = request->read<uintptr_t>( cache::playerArray + ( i * sizeof( uintptr_t ) ) );
			if ( !playerState ) continue;

			int playerTeamID = request->read<int>( playerState + offsets::TEAM_INDEX );
			if ( playerTeamID == cache::myTeamID ) continue;

			uintptr_t pawnPrivate = request->read<uintptr_t>( playerState + offsets::PAWN_PRIVATE );
			if ( !pawnPrivate ) continue;
			if ( pawnPrivate == cache::localPawn ) continue;

			uintptr_t mesh = request->read<uintptr_t>( pawnPrivate + offsets::MESH );
			if ( !mesh ) continue;

			Vector3 HeadBone = getEntityBone( mesh,110 );
			Vector3 BaseBone = getEntityBone( mesh,0 );

			Vector2 Head = PW2S( HeadBone );
			Vector2 Root = PW2S( BaseBone );

			float boxHeight = std::abs( Head.y - Root.y );
			float boxWidth = boxHeight * 0.40f;

			const Vector2 HeadBox = PW2S( Vector3( HeadBone.x,HeadBone.y,HeadBone.z + 16 ) );

			float distance = cache::localCamera.location.distance( BaseBone ) / 100;

			bool isEnemyVisible = isVisible( mesh );
			ImColor visColor = isEnemyVisible ? ImColor( 0,255,0,255 ) : ImColor( 255,0,0,255 );

			if ( visual.box )
			{
				draw2DBox( boxWidth,Root,HeadBox,visColor );
			}
			if ( visual.line )
			{
				drawLine( Root,visColor );
			}
			if ( visual.distance )
			{
				drawDistance( Root,distance,visColor );
			}
		}
	}
}
