#include <iostream>
#include <cmath>
#include <windows.h>
using namespace std;

#include "soundMaker.h"
atomic<double> frequency = 0.0;
NoiseMaker<int>* sound = nullptr;
double MakeSineNoise(double dTime)
{
	double dOutput = sin(frequency * 2.0 * 3.14159 * dTime);
	return dOutput * 0.5; // Master Volume
}
double MakeSquareNoise(double dTime)
{
	double amplitude = 1.0;
	double squareness = 3.0;
	double dOutput = ((amplitude) / (pow(abs(squareness), (sin((squareness * frequency * 3.14159 * dTime) - 0.0)))) + 1.0) - (amplitude / 2.0);
	return dOutput * 0.5; // Master Volume
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_KEYDOWN:
		switch (wParam) {
		case 0x51: // Q key
			frequency = 523.25; // C5
			sound->SetUserFunction(MakeSquareNoise);
			break;
		case 0x5A: // Z key
			frequency = 261.626; // C4
			sound->SetUserFunction(MakeSquareNoise);
			break;
		};
	};
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	AllocConsole();
	freopen("CONOUT$", "w", stdout); // Redirect std::cout to the console
	freopen("CONOUT$", "w", stderr); // Redirect std::cerr to the console
	freopen("CONIN$", "r", stdin);   // Redirect std::cin to the console

	vector<wstring> devices = NoiseMaker<int>::GetDevices();
	for (auto d : devices) wcout << "Found Output Device: " << d << endl;
	wcout << "Using Device: " << devices[0] << endl;
	NoiseMaker<int> sound(devices[0]); // make a new noiseMaker object sound, pass in name of our device as construction parameter
	MSG msg;
	while (1) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}