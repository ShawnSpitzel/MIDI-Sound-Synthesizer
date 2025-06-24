## C++ / Audio Programming Notes

- wstrings
	- Difference between a string and a wstring is that while strings hold 1-byte long characters, wstrings hold 2-byte long characters allowing for characters from different languages.
	- "Hello, my name is Shawn" would be a string, while "こんにちは" would be a wstring.
	- In the case of this application, it is used to store Windows API wide strings.