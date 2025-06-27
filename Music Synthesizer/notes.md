## C++ / Audio Programming Notes

- wstrings
	- Difference between a string and wstring is that strings hold 1-byte chars, wstrings hold 2-byte chars allowing for chars from different languages.
	- "Hello, my name is Shawn" would be a string, while "こんにちは" would be a wstring.
	- In the case of this application, it is used to store **Windows API** wide strings, as it is what is expected for most functions.
	- **USE WSTRINGS FOR ALL WINDOWS API FUNCTIONS**
	
- templates
	- Class templates essentially allow for you to dynamically declare types for a given variable.
	- template<class T>, then doing int x = T(5), followed by double y = T(3.14) would allow you to use the same code for both int and double.
	
