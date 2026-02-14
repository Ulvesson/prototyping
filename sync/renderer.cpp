// renderer.cpp : Rendering program that runs independently
//

#include <iostream>
#include <thread>
#include <chrono>
#include <Windows.h>
#include <string>

using namespace std;

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		cerr << "Usage: renderer.exe <id>" << endl;
		return 1;
	}
	
	int rendererId = atoi(argv[1]);
	
	// Open existing named semaphores with renderer-specific names
	wstring renderSemaName = L"Global\\RenderSignal" + to_wstring(rendererId);
	wstring renderingSemaName = L"Global\\RenderingDone" + to_wstring(rendererId);
	
	HANDLE renderSema = OpenSemaphoreW(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, FALSE, renderSemaName.c_str());
	HANDLE renderingSema = OpenSemaphoreW(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, FALSE, renderingSemaName.c_str());
	
	if (renderSema == NULL || renderingSema == NULL)
	{
		cerr << "Renderer " << rendererId << ": Failed to open semaphores. Error: " << GetLastError() << endl;
		return 1;
	}
	
	cout << "Renderer " << rendererId << " started." << endl;
	
	while (true)
	{
		cout << "Renderer " << rendererId << ": Waiting for render signal..." << endl;
		DWORD result = WaitForSingleObject(renderSema, INFINITE);
		
		if (result == WAIT_OBJECT_0)
		{
			cout << "Renderer " << rendererId << ": Rendering..." << endl;
			this_thread::sleep_for(chrono::milliseconds(1000));
			ReleaseSemaphore(renderingSema, 1, NULL);
		}
	}
	
	CloseHandle(renderSema);
	CloseHandle(renderingSema);
	
	return 0;
}