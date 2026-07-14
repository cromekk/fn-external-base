#pragma once
#include <Windows.h>
#include <cstdint>
#include <TlHelp32.h>
#include <unordered_map>
#include <cassert>
#include <string>
#include "kernel/drv.h"


template <typename T>
class Cache {
	public:
	Cache( ) {}

	~Cache( ) {}

	void insert( uint64_t address, const T& value ) {
		std::memcpy( &cacheData[ address ], &value, sizeof( T ) );
	}

	bool contains( uint64_t address ) const {
		return cacheData.find( address ) != cacheData.end( );
	}

	T retrieve( uint64_t address ) const {
		assert( contains( address ) );
		T value;
		std::memcpy( &value, &cacheData.at( address ), sizeof( T ) );
		return value;
	}

	void clear( ) {
		cacheData.clear( );
	}

	private:
	std::unordered_map<uint64_t, char[ sizeof( T ) ]> cacheData;
};

class Kern
{
	public:

	template <typename T>
	bool IsValidPointerAddress( T address )
	{
		if ( address <= 0x400000 || address == 0xCCCCCCCCCCCCCCCC || reinterpret_cast< void* >( address ) == nullptr || address >
			0x7FFFFFFFFFFFFFFF ) {
			return false;
		}
		return true;
	}


	template <typename T>
	inline T read_( uint64_t address )
	{
		//if ( !IsValidPointerAddress<uintptr_t>( address ) ) return T();
		T buffer { };
		mem::ReadPhysicalMemory( ( PVOID )address, &buffer, sizeof( T ) );
		return buffer;
	}

	template <typename T>
	inline T read( uint64_t address, bool NewFrame = false ) {
		return read_<T>( address );
		//static Cache<T> cache;

		//if (NewFrame) cache.clear();

		//if (address != 0)
		//{
		//	if (cache.contains(address)) {
		//		return cache.retrieve(address);
		//	}
		//	else {
		//		T value = read_<T>(address);
		//		cache.insert(address, value);
		//		return value;
		//	}
		//}
	}

	template <typename T>
	T ReadBit( uint64_t address, uint8_t bitPosition ) {
		return ( read<T>( address ) >> bitPosition );
	}

	template <typename T>
	void WriteBit( uint64_t address, uint8_t bitPosition, T value ) {
		uint8_t byteAtOffset = read<uint8_t>( address );
		byteAtOffset |= ( value << bitPosition );
		write<uint8_t>( address, byteAtOffset );
	}

	template <typename T>
	inline T write( uint64_t address, T buffer )
	{
		if ( !IsValidPointerAddress<uintptr_t>( address ) ) return T( );
		mem::WritePhysicalMemory( ( PVOID )address, &buffer, sizeof( T ) );
		return buffer;
	}

	inline bool read_array( const std::uintptr_t address, void* buffer, const std::size_t size )
	{
		if ( buffer == nullptr || size == 0 ) {
			return false;
		}

		mem::ReadPhysicalMemory( reinterpret_cast< PVOID >( address ), buffer, static_cast< DWORD >( size ) );
	}

	inline uintptr_t GetBaseAddress( )
	{
		return mem::GetBase( );
	}

	std::vector<int> PatternToByte( const char* pattern )
	{
		std::vector<int> bytes;
		char* start = const_cast< char* >( pattern );
		char* end = const_cast< char* >( pattern ) + strlen( pattern );

		for ( char* current = start; current < end; ++current )
		{
			if ( *current == '?' )
			{
				++current;
				if ( *current == '?' ) ++current;
				bytes.push_back( -1 );
			}
			else
			{
				bytes.push_back( strtoul( current, &current, 16 ) );
			}
		}
		return bytes;
	}

	unsigned long long FindSignature( uintptr_t moduleBase, size_t moduleSize, const char* pattern )
	{
		std::vector<int> signature = PatternToByte( pattern );
		size_t signatureSize = signature.size( );

		for ( uintptr_t i = 0; i < moduleSize - signatureSize; ++i )
		{
			bool found = true;
			for ( size_t j = 0; j < signatureSize; ++j )
			{
				if ( signature[ j ] != -1 && signature[ j ] != this->read<unsigned char>( moduleBase + i + j ) )
				{
					found = false;
					break;
				}
			}
			if ( found )
			{
				return moduleBase + i;
			}
		}

		return 0;
	}
}; inline Kern* request;