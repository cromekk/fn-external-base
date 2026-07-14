#include "includes.h"
#include <iostream>

int main( )
{
	printf( "Waiting for Fortnite..." );
	while ( game_wnd == 0 )
	{
		Sleep( 1 );
		mem::ProcessID = mem::FindProcess( L"FortniteClient-Win64-Shipping.exe" );
		game_wnd = get_process_wnd( mem::ProcessID );
		Sleep( 1 );
	}

	system( "cls" );

	if ( !mem::SetupDriver( ) )
	{
		printf( "The driver was not initialized" );
		Sleep( 3000 );
		exit( 0 );
	}

	mem::base = mem::GetBase( );

	mem::BypassCR3( );

	if ( !mem::base )
	{
		printf( "The driver couldn't get the base address" );
		Sleep( 3000 );
		exit( 0 );
	}

	hijack_window( );
	directx_init( );
	render_loop( );

	exit( 0 );
}
