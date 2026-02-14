// sync.cpp : Defines the entry point for the application.
//

#include <thread>
#include <Windows.h>
#include <vector>
#include <string>
#include <iostream>

using namespace std;

const int NUM_RENDERERS = 4;

	int main()
{
	vector<HANDLE> renderSemas(NUM_RENDERERS);
	vector<HANDLE> renderingSemas(NUM_RENDERERS);
	vector<PROCESS_INFORMATION> processes(NUM_RENDERERS);
	
	// Create named semaphores for each renderer
	for (int i = 0; i < NUM_RENDERERS; ++i)
	{
		wstring renderSemaName = L"Global\\RenderSignal" + to_wstring(i);
		wstring renderingSemaName = L"Global\\RenderingDone" + to_wstring(i);
		
		renderSemas[i] = CreateSemaphoreW(NULL, 0, 10, renderSemaName.c_str());
		renderingSemas[i] = CreateSemaphoreW(NULL, 1, 10, renderingSemaName.c_str());
		
		if (renderSemas[i] == NULL || renderingSemas[i] == NULL)
		{
			cerr << "Failed to create semaphores for renderer " << i << ". Error: " << GetLastError() << endl;
			return 1;
		}
	}
	
	// Start renderer processes
	cout << "Starting " << NUM_RENDERERS << " renderer processes..." << endl;
	
	for (int i = 0; i < NUM_RENDERERS; ++i)
	{
		STARTUPINFOW si = { sizeof(si) };
		
		wstring cmdLine = L"renderer.exe " + to_wstring(i);
		vector<wchar_t> cmdLineBuf(cmdLine.begin(), cmdLine.end());
		cmdLineBuf.push_back(0);
		
		if (!CreateProcessW(NULL, cmdLineBuf.data(), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &processes[i]))
		{
			cerr << "Failed to start renderer " << i << ". Error: " << GetLastError() << endl;
			return 1;
		}
		
		cout << "Started renderer " << i << endl;
	}
	
	// Wait a bit for renderers to initialize
	this_thread::sleep_for(chrono::milliseconds(500));
	
	// run for 100 frames, cycling through renderers
	for (int frame = 0; frame < 100; ++frame)
	{
		int rendererIdx = frame % NUM_RENDERERS;
		
		WaitForSingleObject(renderingSemas[rendererIdx], INFINITE);
		cout << "Frame " << frame + 1 << ": Signaling renderer " << rendererIdx << "..." << endl;
		ReleaseSemaphore(renderSemas[rendererIdx], 1, NULL);
		this_thread::sleep_for(chrono::milliseconds(1));
	}
	
	// Terminate all renderer processes
	cout << "Terminating renderer processes..." << endl;
	for (int i = 0; i < NUM_RENDERERS; ++i)
	{
		TerminateProcess(processes[i].hProcess, 0);
		WaitForSingleObject(processes[i].hProcess, 1000);
		
		CloseHandle(processes[i].hProcess);
		CloseHandle(processes[i].hThread);
		CloseHandle(renderSemas[i]);
		CloseHandle(renderingSemas[i]);
	}
	
	cout << "All frames completed." << endl;
	return 0;
}
