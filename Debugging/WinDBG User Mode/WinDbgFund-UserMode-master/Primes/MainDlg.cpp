// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"
#include <math.h>

struct PrimeData {
	int From, To;
	int Count;
	HANDLE hFile;
};

struct PrimeData2 {
	int From, To;
	int Count;
	HANDLE hFile;
	CRITICAL_SECTION* cs;
};

bool CMainDlg::IsPrime(int n) {
	int limit = (int)sqrt(n);
	for (int i = 2; i <= limit; i++)
		if (n % i == 0)
			return false;

	return true;
}

DWORD WINAPI CalcPrimesThread1(PVOID p) {
	PrimeData* data = (PrimeData*)p;
	int from = data->From, to = data->To;
	int count = 0;
	CStringA text;
	DWORD bytes;

	for (int i = from; i <= to; i++) {
		if (CMainDlg::IsPrime(i)) {
			count++;
			::SetFilePointer(data->hFile, 0, nullptr, FILE_END);
			text.Format("%8d\n", i);
			::WriteFile(data->hFile, text.GetBuffer(), text.GetLength(), &bytes, nullptr);
		}
	}

	data->Count = count;

	return 0;
}

DWORD WINAPI CalcPrimesThread3(PVOID p) {
	PrimeData* data = (PrimeData*)p;
	int from = data->From, to = data->To;
	int count = 0;
	CStringA text;
	DWORD bytes;

	for (int i = from; i <= to; i++) {
		if (CMainDlg::IsPrime(i)) {
			count++;
			::SetFilePointer(data->hFile, 0, nullptr, FILE_END);
			text.Format("%8d\n", i);
			::WriteFile(data->hFile, text.GetBuffer(), text.GetLength(), &bytes, nullptr);
		}
	}

	data->Count = count;

	return 0;
}

DWORD WINAPI CalcPrimesThread2(PVOID p) {
	PrimeData2* data = (PrimeData2*)p;
	int from = data->From, to = data->To;
	int count = 0;
	CStringA text;
	DWORD bytes;

	for (int i = from; i <= to; i++) {
		if (CMainDlg::IsPrime(i)) {
			count++;
			text.Format("%8d\n", i);
			::EnterCriticalSection(data->cs);
			::SetFilePointer(data->hFile, 0, nullptr, FILE_END);
			::WriteFile(data->hFile, text.GetBuffer(), text.GetLength(), &bytes, nullptr);
			::LeaveCriticalSection(data->cs);
		}
	}

	data->Count = count;

	return 0;
}

DWORD CMainDlg::StartCalc1() {
	int threads = m_Threads;
	int from = m_First, to = m_Last;

	int range = (to - from + 1) / threads;
	PrimeData* data = (PrimeData*)malloc(sizeof(PrimeData) * threads);
	HANDLE* hThread = (HANDLE*)malloc(sizeof(HANDLE) * threads);
	HANDLE hFile = INVALID_HANDLE_VALUE;
	if (!m_FileName.IsEmpty()) {
		hFile = ::CreateFile(m_FileName, GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, 0, nullptr);
		::SetEndOfFile(hFile);
	}

	for (int i = 0; i < threads; i++) {
		data[i].From = i * range + from;
		data[i].To = i == threads - 1 ? to : (i + 1) * range + from - 1;
		data[i].hFile = hFile;
		hThread[i] = ::CreateThread(nullptr, 0, CalcPrimesThread1, &data[i], 0, nullptr);
	}
	::WaitForMultipleObjects(threads, hThread, TRUE, INFINITE);
	if (!m_FileName.IsEmpty()) {
		::CloseHandle(hFile);
	}

	int count = 0;
	for (int i = 0; i < threads; i++)
		count += data[i].Count;

	for (int i = 0; i < threads; i++)
		::CloseHandle(hThread[i]);

	free(data);
	free(hThread);

	PostMessage(WM_DONE, count);

	return 0;
}

DWORD CMainDlg::StartCalc2() {
	int threads = m_Threads;
	int from = m_First, to = m_Last;

	int range = (to - from + 1) / threads;
	PrimeData2* data = (PrimeData2*)malloc(sizeof(PrimeData2) * threads);
	HANDLE* hThread = (HANDLE*)malloc(sizeof(HANDLE) * threads);
	HANDLE hFile = INVALID_HANDLE_VALUE;
	if (!m_FileName.IsEmpty()) {
		hFile = ::CreateFile(m_FileName, GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, 0, nullptr);
		::SetEndOfFile(hFile);
	}
	CRITICAL_SECTION cs;
	::InitializeCriticalSection(&cs);

	for (int i = 0; i < threads; i++) {
		data[i].From = i * range + from;
		data[i].To = i == threads - 1 ? to : (i + 1) * range + from - 1;
		data[i].hFile = hFile;
		data[i].cs = &cs;
		hThread[i] = ::CreateThread(nullptr, 0, CalcPrimesThread2, &data[i], 0, nullptr);
	}
	::WaitForMultipleObjects(threads, hThread, TRUE, INFINITE);

	if (!m_FileName.IsEmpty()) {
		::CloseHandle(hFile);
	}

	int count = 0;
	for (int i = 0; i < threads; i++)
		count += data[i].Count;

	PostMessage(WM_DONE, count);

	return 0;
}

DWORD CMainDlg::StartCalc3() {
	int threads = m_Threads;
	int from = m_First, to = m_Last;

	int range = (to - from + 1) / threads;
	PrimeData2* data = (PrimeData2*)malloc(sizeof(PrimeData2) * threads);
	HANDLE* hThread = (HANDLE*)malloc(sizeof(HANDLE) * threads);
	HANDLE hFile = INVALID_HANDLE_VALUE;
	if (!m_FileName.IsEmpty()) {
		hFile = ::CreateFile(m_FileName, GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, 0, nullptr);
		::SetEndOfFile(hFile);
	}
	CRITICAL_SECTION cs;
	::InitializeCriticalSection(&cs);

	for (int i = 0; i < threads; i++) {
		data[i].From = i * range + from;
		data[i].To = i == threads - 1 ? to : (i + 1) * range + from - 1;
		data[i].hFile = hFile;
		data[i].cs = &cs;
		hThread[i] = ::CreateThread(nullptr, 0, CalcPrimesThread2, &data[i], 0, nullptr);
	}
	::WaitForMultipleObjects(threads, hThread, TRUE, INFINITE);
	::DeleteCriticalSection(&cs);

	if (!m_FileName.IsEmpty()) {
		::CloseHandle(hFile);
	}

	for (int i = 0; i < threads; i++)
		::CloseHandle(hThread[i]);

	free(data);
	free(hThread);

	int count = 0;
	for (int i = 0; i < threads; i++)
		count += data[i].Count;

	PostMessage(WM_DONE, count);

	return 0;
}

DWORD CMainDlg::StartCalc4() {
	int threads = m_Threads;
	int from = m_First, to = m_Last;

	int range = (to - from + 1) / threads;
	PrimeData2* data = (PrimeData2*)malloc(threads);
	HANDLE* hThread = (HANDLE*)malloc(sizeof(HANDLE) * threads);
	HANDLE hFile = INVALID_HANDLE_VALUE;
	if (!m_FileName.IsEmpty()) {
		hFile = ::CreateFile(m_FileName, GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, 0, nullptr);
		::SetEndOfFile(hFile);
	}
	CRITICAL_SECTION cs;
	::InitializeCriticalSection(&cs);

	for (int i = 0; i < threads; i++) {
		data[i].From = i * range + from;
		data[i].To = i == threads - 1 ? to : (i + 1) * range + from - 1;
		data[i].hFile = hFile;
		data[i].cs = &cs;
		hThread[i] = ::CreateThread(nullptr, 0, CalcPrimesThread2, &data[i], 0, nullptr);
	}
	::WaitForMultipleObjects(threads, hThread, TRUE, INFINITE);
	::DeleteCriticalSection(&cs);

	if (!m_FileName.IsEmpty()) {
		::CloseHandle(hFile);
	}

	for (int i = 0; i < threads; i++)
		::CloseHandle(hThread[i]);

	int count = 0;
	for (int i = 0; i < threads; i++)
		count += data[i].Count;

	free(data);
	free(hThread);

	PostMessage(WM_DONE, count);

	return 0;
}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
	SetIcon(hIconSmall, FALSE);

	::SHAutoComplete(GetDlgItem(IDC_PATH), SHACF_FILESYSTEM);
	SetDlgItemInt(IDC_FIRST, 3);
	SetDlgItemInt(IDC_LAST, 1000000);
	SetDlgItemText(IDC_PATH, L"c:\\temp\\primes.txt");
	SetDlgItemInt(IDC_THREADS, min(8, ::GetActiveProcessorCount(ALL_PROCESSOR_GROUPS)));

	return TRUE;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	EndDialog(wID);
	return 0;
}

LRESULT CMainDlg::OnCalc1(WORD, WORD wID, HWND, BOOL&) {
	if (!GetData())
		return 0;

	m_StartTime = ::GetTickCount64();
	SetDlgItemText(IDC_MESSAGE, L"Working...");

	::QueueUserWorkItem([](auto context) -> DWORD {
		return ((CMainDlg*)context)->StartCalc1();
		}, this, WT_EXECUTEDEFAULT);

	return 0;
}

LRESULT CMainDlg::OnCalc2(WORD, WORD wID, HWND, BOOL&) {
	if (!GetData())
		return 0;

	m_StartTime = ::GetTickCount64();
	SetDlgItemText(IDC_MESSAGE, L"Working...");

	::QueueUserWorkItem([](auto context) -> DWORD {
		return ((CMainDlg*)context)->StartCalc2();
		}, this, WT_EXECUTEDEFAULT);

	return 0;
}

LRESULT CMainDlg::OnCalc3(WORD, WORD wID, HWND, BOOL&) {
	if (!GetData())
		return 0;

	m_StartTime = ::GetTickCount64();
	SetDlgItemText(IDC_MESSAGE, L"Working...");

	::QueueUserWorkItem([](auto context) -> DWORD {
		return ((CMainDlg*)context)->StartCalc3();
		}, this, WT_EXECUTEDEFAULT);

	return 0;
}

LRESULT CMainDlg::OnCalc4(WORD, WORD wID, HWND, BOOL&) {
	if (!GetData())
		return 0;

	m_StartTime = ::GetTickCount64();
	SetDlgItemText(IDC_MESSAGE, L"Working...");

	::QueueUserWorkItem([](auto context) -> DWORD {
		return ((CMainDlg*)context)->StartCalc4();
		}, this, WT_EXECUTEDEFAULT);

	return 0;
}


LRESULT CMainDlg::OnDone(UINT, WPARAM count, LPARAM, BOOL&) {
	auto time = ::GetTickCount64() - m_StartTime;
	CString text;
	text.Format(L"Done! Primes: %u Elapsed: %llu msec", (DWORD)count, time);
	SetDlgItemText(IDC_MESSAGE, text);
	
	return 0;
}

bool CMainDlg::GetData() {
	int first = GetDlgItemInt(IDC_FIRST);
	int last = GetDlgItemInt(IDC_LAST);
	if (first < 3 || last < 3 || last < first) {
		AtlMessageBox(*this, L"Please provide a positive range of numbers", IDR_MAINFRAME, MB_ICONERROR);
		return false;
	}
	m_Threads = GetDlgItemInt(IDC_THREADS);
	if (m_Threads < 1 || m_Threads > 64) {
		AtlMessageBox(*this, L"Valid thread count is from 1 to 64", IDR_MAINFRAME, MB_ICONERROR);
		return false;
	}

	m_First = first;
	m_Last = last;
	GetDlgItemText(IDC_PATH, m_FileName);
	return true;
}
