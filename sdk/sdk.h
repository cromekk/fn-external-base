#pragma once
#include <d3d9.h>
#include <vector>
#include <cmath>

#include "math/matrix.h"

struct Camera
{
	Vector3 location;
	Vector3 rotation;
	float fov;
};

struct FNRot
{
	double a;
	double b;
	double c;
};

namespace cache
{
	inline uintptr_t encrypted;
	inline uintptr_t uWorld;
	inline uintptr_t gameInstance;
	inline uintptr_t localPlayers;
	inline uintptr_t playerController;
	inline uintptr_t localPawn;
	inline uintptr_t rootComponent;
	inline uintptr_t playerState;
	inline int myTeamID;
	inline uintptr_t gameState;
	inline uintptr_t playerArray;
	inline int playerCount;
	inline float closestDistance;
	inline uintptr_t closestMesh;
	inline Camera localCamera;
}

Camera getCamera()
{
	Camera view_point{};
	uintptr_t location_pointer = request->read<uintptr_t>(cache::uWorld + 0x178);
	uintptr_t rotation_pointer = request->read<uintptr_t>(cache::uWorld + 0x188);
	FNRot fnrot{};
	fnrot.a = request->read<double>(rotation_pointer);
	fnrot.b = request->read<double>(rotation_pointer + 0x20);
	fnrot.c = request->read<double>(rotation_pointer + 0x1D0);
	view_point.location = request->read<Vector3>(location_pointer);
	view_point.rotation.x = asin(fnrot.c) * (180.0 / M_PI);
	view_point.rotation.y = ((atan2(fnrot.a * -1, fnrot.b) * (180.0 / M_PI)) * -1) * -1;
	view_point.fov = request->read<float>(cache::playerController + 0x374) * 90.f;
	return view_point;
}

Vector2 PW2S(Vector3 world_location)
{
	cache::localCamera = getCamera();
	_MATRIX temp_matrix = Matrix(cache::localCamera.rotation);
	auto vaxisx = Vector3(temp_matrix.m[0][0], temp_matrix.m[0][1], temp_matrix.m[0][2]);
	auto vaxisy = Vector3(temp_matrix.m[1][0], temp_matrix.m[1][1], temp_matrix.m[1][2]);
	auto vaxisz = Vector3(temp_matrix.m[2][0], temp_matrix.m[2][1], temp_matrix.m[2][2]);
	Vector3 vdelta = world_location - cache::localCamera.location;
	auto vtransformed = Vector3(vdelta.dot(vaxisy), vdelta.dot(vaxisz), vdelta.dot(vaxisx));
	if (vtransformed.z < 1) vtransformed.z = 1;
	return Vector2(settings.ScreenCenterX + vtransformed.x * ((settings.ScreenCenterX / tanf(cache::localCamera.fov * M_PI / 360))) / vtransformed.z, settings.ScreenCenterY - vtransformed.y * ((settings.ScreenCenterX / tanf(cache::localCamera.fov * M_PI / 360))) / vtransformed.z);
}

inline Vector3 getEntityBone( const std::uintptr_t mesh,const int bone_id )
{
	uintptr_t bone_array = request->read<uintptr_t>( mesh + offsets::BONE_ARRAY );
	if ( bone_array == 0 ) bone_array = request->read<uintptr_t>( mesh + offsets::BONE_ARRAY_CACHE );

	FTransform bone = request->read<FTransform>( bone_array + ( bone_id * 0x60 ) );

	FTransform component_to_world = request->read<FTransform>( mesh + offsets::COMPONENT_TO_WORLD );

	D3DMATRIX matrix = MatrixMultiplication( bone.ToMatrixWithScale( ),component_to_world.ToMatrixWithScale( ) );
	return Vector3( matrix._41,matrix._42,matrix._43 );
}

auto isVisible( uintptr_t mesh ) -> bool
{
	auto visibilityProxy = request->read<std::uintptr_t>( std::uintptr_t( mesh ) + 0xB0 );
	if ( !visibilityProxy )
		return false;
	auto visProxyLastRenderTime = request->read<float>( visibilityProxy + 0x7DC );
	auto visTolerance = request->read<float>( std::uintptr_t( mesh ) + 0x330 );
	auto visProxyLastSubmit = request->read<double>( visibilityProxy + 0x7B8 );
	return ( std::fmax( 0.f,visProxyLastRenderTime + 0.0001f ) >= visProxyLastSubmit - static_cast< double >( visTolerance ) );
}
