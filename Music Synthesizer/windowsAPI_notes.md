## Windows API Notes

- **WAVEOUTCAPS (structure)**
	- A structure detailing all of the information for an audio device, such as:
	- Name (szPname), formats (dwFormats), number of audio channels supported (wChannels), and driver version
	- Used in correlation with waveOutGetDevCaps function to store information
	
- **waveOutGetDevCaps (function)**
	- Function called with parameters (device ID, pointer to WAVEOUTCAPS structure, size of structure))
	- Essentially says, given this device ID, store all of the information about said device within a WAVEOUTCAPS object

- **WAVEFORMATEX (structure)**
	- A structure detailing information regarding the sound being played, such as:
	- Sample rate (nSamplesPerSec), Bit depth (wBitsPerSample), number of channels (nChannels where 1 is mono, 2 is stereo), and format (wFormatTag)
	
- **WAVEOUTOPEN (function)**
	- Function called with parameters (pointer to output device, device ID, pointer to audio format structure, etc)
	- Used to handle what audio device will handle outputting audio, and what audio it will output.