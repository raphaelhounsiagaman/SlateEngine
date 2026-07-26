#pragma once

#include "Core/Application.h"

#include <Windows.h>

#include <memory>
#include <stdexcept>
#include <utility>

int WINAPI wWinMain(
	HINSTANCE instanceHandle,
	HINSTANCE prevInstanceHandle,
	PWSTR commandLine,
	int commandShow)
{

	Slate::WindowInformation windowInfo
	{
		L"Slate Application",
		1280,
		720,
		instanceHandle,
		commandLine,
		commandShow
	};

	try
	{
		std::unique_ptr<Slate::Application> app =
			Slate::CreateApplication(std::move(windowInfo));
		app->Run();

	}
	catch (const std::exception& exception)
	{
		MessageBoxA(
			nullptr, 
			exception.what(), 
			"Error", 
			MB_OK | MB_ICONERROR
		);

		return -1;
	}

	return 0;
}
