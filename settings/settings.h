#pragma once

#include <Windows.h>

class Settings {
public:
	int ScreenWidth = GetSystemMetrics( SM_CXSCREEN );
	int ScreenHeight = GetSystemMetrics( SM_CYSCREEN );
	int ScreenCenterX = ScreenWidth / 2;
	int ScreenCenterY = ScreenHeight / 2;
}; inline Settings settings;

class menuSettings {
public:
	bool showMenu = true;
	int tab = 0;
}; inline menuSettings menu;

class aimSettings {
public:
	bool showFov = false;
	float fovValue = 150;
}; inline aimSettings aim;

class visualSettings {
public:
	bool enable = false;
	bool box = false;
	bool fillBox = false;
	bool line = false;
	bool distance = false;
}; inline visualSettings visual;