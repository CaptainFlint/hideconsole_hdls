#include <Windows.h>

__forceinline void ZeroMem(void* buf, size_t sz)
{
	for (size_t i = 0; i < sz; ++i)
		((BYTE*)buf)[i] = 0;
}

int wWinMainCRTStartup()
{
	LPWSTR CmdLine = GetCommandLine();
	// Remove the main executable from command line
	bool InsideQuotes = false;
	while ((*CmdLine > L' ') || (*CmdLine && InsideQuotes))
	{
		// Flip the InsideQuotes if current character is a double quote
		if (*CmdLine == L'"')
			InsideQuotes = !InsideQuotes;
		++CmdLine;
	}

	// Skip past any white space preceeding the second token.
	while (*CmdLine && (*CmdLine <= L' '))
		++CmdLine;

	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	ZeroMem(&pi, sizeof(pi));
	ZeroMem(&si, sizeof(si));
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	if (CreateProcess(NULL, CmdLine, NULL, NULL, TRUE, CREATE_UNICODE_ENVIRONMENT | CREATE_NO_WINDOW, NULL, NULL, &si, &pi) == 0)
	{
		DWORD err = GetLastError();
		const LPCWSTR MsgPrefix = L"Failed to run command:\n";
		LPWSTR MsgError;
		LPWSTR MsgResult;

		size_t MsgPrefixLen = lstrlenW(MsgPrefix);
		size_t MsgErrorLen = FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			err,
			MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
			(LPWSTR)&MsgError,
			0,
			NULL
		);
		size_t BufSize = (MsgPrefixLen + MsgErrorLen + 1) * sizeof(WCHAR);
		MsgResult = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, BufSize);
		lstrcpyW(MsgResult, MsgPrefix);
		lstrcpyW(MsgResult + MsgPrefixLen, MsgError);
		MessageBox(NULL, MsgResult, L"Console Hider", MB_ICONERROR | MB_OK);
		LocalFree(MsgResult);
		LocalFree(MsgError);
		ExitProcess(1);
	}
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	ExitProcess(0);
}
