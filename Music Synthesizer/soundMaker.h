#pragma once

#pragma comment(lib, "winmm.lib")

#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <condition_variable>
using namespace std;
#include <Windows.h>
const double PI = 2.0 * acos(0.0);
template<class T>
class NoiseMaker
{
public:
	NoiseMaker(wstring sOutputDevice, unsigned int nSampleRate = 44100, unsigned int nChannels = 1, unsigned int nBlocks = 8, unsigned int nBlockSamples = 512) //leave device name for user input
	{
		Create(sOutputDevice, nSampleRate, nChannels, nBlocks, nBlockSamples);
	}

	~NoiseMaker()
	{
		Destroy();
	}

	bool Create(wstring sOutputDevice, unsigned int nSampleRate = 44100, unsigned int nChannels = 1, unsigned int nBlocks = 8, unsigned int nBlockSamples = 512)
	{
		m_bReady = false;
		m_nSampleRate = nSampleRate;
		m_nChannels = nChannels;
		m_nBlockCount = nBlocks;
		m_nBlockSamples = nBlockSamples;
		m_nBlockFree = m_nBlockCount;
		m_nBlockCurrent = 0;
		m_pBlockMemory = nullptr;
		m_pWaveHeaders = nullptr;

		m_userFunction = nullptr;

		// Validate device
		vector<wstring> devices = GetDevices(); //get list of all devices
		auto d = std::find(devices.begin(), devices.end(), sOutputDevice); //Find the given output device within list of available devices
		if (d != devices.end())
		{
			// Device is available	
			int nDeviceID = distance(devices.begin(), d);
			WAVEFORMATEX waveFormat; //information regarding our audio
			waveFormat.wFormatTag = WAVE_FORMAT_PCM; //raw, uncompressed audio format, most common
			waveFormat.nSamplesPerSec = m_nSampleRate; //use our inputted sample rate
			waveFormat.wBitsPerSample = 32; //16 bits is good, 24 is great, 32 is excellent
			waveFormat.nChannels = m_nChannels; //use our inputted number of channels (1 mono, 2 stereo)
			waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels; //standard formula
			waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign; //standard formula
			waveFormat.cbSize = 0;

			// Open Device if valid
			if (waveOutOpen(&m_hwDevice, nDeviceID, &waveFormat, (DWORD_PTR)waveOutProcWrap, (DWORD_PTR)this, CALLBACK_FUNCTION) != S_OK)
				return Destroy();
		}

		// Allocate Wave|Block Memory
		m_pBlockMemory = new T[m_nBlockCount * m_nBlockSamples];
		if (m_pBlockMemory == nullptr)
			return Destroy();
		ZeroMemory(m_pBlockMemory, sizeof(T) * m_nBlockCount * m_nBlockSamples);

		m_pWaveHeaders = new WAVEHDR[m_nBlockCount];
		if (m_pWaveHeaders == nullptr)
			return Destroy();
		ZeroMemory(m_pWaveHeaders, sizeof(WAVEHDR) * m_nBlockCount);

		// Link headers to block memory
		for (unsigned int n = 0; n < m_nBlockCount; n++)
		{
			m_pWaveHeaders[n].dwBufferLength = m_nBlockSamples * sizeof(T);
			m_pWaveHeaders[n].lpData = (LPSTR)(m_pBlockMemory + (n * m_nBlockSamples));
		}

		m_bReady = true;

		m_thread = thread(&NoiseMaker::MainThread, this);

		// Start the ball rolling
		unique_lock<mutex> lm(m_muxBlockNotZero);
		m_cvBlockNotZero.notify_one();

		return true;
	}

	bool Destroy()
	{
		if (m_bReady) {
			Stop();
		}
		return false;
	}

	void Stop()
	{
		m_bReady = false;
		m_thread.join();
	}

	// Override to process current sample
	virtual double UserProcess(double dTime)
	{
		return 0.0;
	}

	double GetTime()
	{
		return m_dGlobalTime;
	}


		
public:
	static vector<wstring> GetDevices() //Use wstring to hold wide character strings for device names
	{
		u_int nDeviceCount = waveOutGetNumDevs(); //windows API call to get a list of all available audio devices on the system
		vector<wstring> sDevices;
		WAVEOUTCAPS deviceInfo; //Device info struct
		for (int n = 0; n < nDeviceCount; n++) //iterate through all available devices
			if (waveOutGetDevCaps(n, &deviceInfo, sizeof(WAVEOUTCAPS)) == S_OK) //get information for each device via device ID, store within device object
				sDevices.push_back(deviceInfo.szPname);
		return sDevices;
	}

	void SetUserFunction(double(*func)(double))
	{
		m_userFunction = func;
	}

	double clip(double dSample, double dMax)
	{
		if (dSample >= 0.0)
			return fmin(dSample, dMax);
		else
			return fmax(dSample, -dMax);
	}


private:
	double(*m_userFunction)(double);

	unsigned int m_nSampleRate;
	unsigned int m_nChannels;
	unsigned int m_nBlockCount;
	unsigned int m_nBlockSamples;
	unsigned int m_nBlockCurrent;

	T* m_pBlockMemory;
	WAVEHDR* m_pWaveHeaders;
	HWAVEOUT m_hwDevice; //output device

	thread m_thread;
	atomic<bool> m_bReady;
	atomic<unsigned int> m_nBlockFree;
	condition_variable m_cvBlockNotZero;
	mutex m_muxBlockNotZero;

	atomic<double> m_dGlobalTime;

	// Handler for soundcard request for more data
	void waveOutProc(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwParam1, DWORD dwParam2)
	{
		//Make space for a new block to be used once audio is finished playing
		if (uMsg != WOM_DONE) return;

		m_nBlockFree++;
		unique_lock<mutex> lm(m_muxBlockNotZero);
		m_cvBlockNotZero.notify_one();
	}

	// Static wrapper for sound card handler
	static void CALLBACK waveOutProcWrap(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
	{
		((NoiseMaker*)dwInstance)->waveOutProc(hWaveOut, uMsg, dwParam1, dwParam2);
	}

	// Main thread. This loop responds to requests from the soundcard to fill 'blocks'
	// with audio data. If no requests are available it goes dormant until the sound
	// card is ready for more data. The block is fille by the "user" in some manner
	// and then issued to the soundcard.
	void MainThread()
	{
		m_dGlobalTime = 0.0;
		double dTimeStep = 1.0 / (double)m_nSampleRate;

		// Goofy hack to get maximum integer for a type at run-time
		T nMaxSample = (T)pow(2, (sizeof(T) * 8) - 1) - 1;
		double dMaxSample = (double)nMaxSample;
		T nPreviousSample = 0;

		while (m_bReady)
		{
			// Wait for block to become available
			if (m_nBlockFree == 0)
			{
				unique_lock<mutex> lm(m_muxBlockNotZero);
				m_cvBlockNotZero.wait(lm);
			}

			// Block is here, so use it
			m_nBlockFree--;

			// Prepare block for processing
			if (m_pWaveHeaders[m_nBlockCurrent].dwFlags & WHDR_PREPARED)
				waveOutUnprepareHeader(m_hwDevice, &m_pWaveHeaders[m_nBlockCurrent], sizeof(WAVEHDR));

			T nNewSample = 0;
			int nCurrentBlock = m_nBlockCurrent * m_nBlockSamples;

			for (unsigned int n = 0; n < m_nBlockSamples; n++)
			{
				// User Process
				if (m_userFunction == nullptr)
					nNewSample = (T)(clip(UserProcess(m_dGlobalTime), 1.0) * dMaxSample);
				else
					nNewSample = (T)(clip(m_userFunction(m_dGlobalTime), 1.0) * dMaxSample);

				m_pBlockMemory[nCurrentBlock + n] = nNewSample;
				nPreviousSample = nNewSample;
				m_dGlobalTime = m_dGlobalTime + dTimeStep;
			}

			// Send block to sound device
			waveOutPrepareHeader(m_hwDevice, &m_pWaveHeaders[m_nBlockCurrent], sizeof(WAVEHDR));
			waveOutWrite(m_hwDevice, &m_pWaveHeaders[m_nBlockCurrent], sizeof(WAVEHDR));
			m_nBlockCurrent++;
			m_nBlockCurrent %= m_nBlockCount;
		}
	}
};
