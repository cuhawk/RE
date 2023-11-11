// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"
#include "PrimesCalculator.h"
#include <string>
#include <sstream>

DWORD __stdcall CMainDlg::ThreadWorker(PVOID) {
	auto tick = ::GetTickCount64();

	// exit after 10 seconds

	while (::GetTickCount64() - tick < 10000) {
		::Sleep(500);
	}

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

	m_ListBox.Attach(GetDlgItem(IDC_LIST));

	::SHAutoComplete(GetDlgItem(IDC_PATH), SHACF_FILESYSTEM);
	::SHAutoComplete(GetDlgItem(IDC_EXE), SHACF_FILESYSTEM);

	return TRUE;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	EndDialog(wID);
	return 0;
}

LRESULT CMainDlg::OnCalc(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int first = GetDlgItemInt(IDC_FIRST);
	int last = GetDlgItemInt(IDC_LAST);

	m_ListBox.ResetContent();

	for (auto n : PrimesCalculator::FindPrimes(first, last))
		m_ListBox.AddString(std::to_wstring(n).c_str());

	return 0;
}

LRESULT CMainDlg::OnCreateThread(WORD, WORD, HWND, BOOL&) {
	DWORD id;
	HANDLE hThread = ::CreateThread(nullptr, 0, ThreadWorker, this, 0, &id);
	if (hThread == nullptr)
		AtlMessageBox(*this, L"Failed to create thread!", IDS_TITLE, MB_ICONERROR);
	else
		AtlMessageBox(*this, L"Thread created successfully.", IDS_TITLE, MB_ICONINFORMATION);

	return LRESULT();
}

LRESULT CMainDlg::OnCreateProcess(WORD, WORD, HWND, BOOL&) {
	CString path;
	GetDlgItemText(IDC_EXE, path);

	PROCESS_INFORMATION pi;
	STARTUPINFO si = { sizeof(si) };
	if (::CreateProcess(nullptr, path.GetBuffer(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
		::CloseHandle(pi.hProcess);
		::CloseHandle(pi.hThread);
	}
	else {
		AtlMessageBox(*this, L"Failed to create process!", IDS_TITLE, MB_ICONERROR);
	}
	return 0;
}

LRESULT CMainDlg::OnCreateNewInst(WORD, WORD, HWND, BOOL&) {
	WCHAR path[MAX_PATH];
	::GetModuleFileName(nullptr, path, _countof(path));
	
	PROCESS_INFORMATION pi;
	STARTUPINFO si = { sizeof(si) };
	if (::CreateProcess(nullptr, path, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
		::CloseHandle(pi.hProcess);
		::CloseHandle(pi.hThread);
	}

	return 0;
}

LRESULT CMainDlg::OnSave(WORD, WORD, HWND, BOOL&) {
	CString path;
	GetDlgItemText(IDC_PATH, path);

	HANDLE hFile = ::CreateFile(path, GENERIC_WRITE, 0, nullptr, CREATE_NEW, 0, nullptr);
	if (hFile == INVALID_HANDLE_VALUE) {
		AtlMessageBox(*this, L"Failed to create new file!", IDS_TITLE, MB_ICONERROR);
		return 0;
	}

	DWORD bytes;
	for (int i = 0; i < m_ListBox.GetCount(); i++) {
		CString text;
		m_ListBox.GetText(i, text);
		text += L"\n\r";
		::WriteFile(hFile, text, text.GetLength() * sizeof(WCHAR), &bytes, nullptr);
	}
	::CloseHandle(hFile);

	AtlMessageBox(*this, L"Data saved successfully.", IDS_TITLE, MB_ICONINFORMATION);
	return 0;
}

LRESULT CMainDlg::OnRead(WORD, WORD, HWND, BOOL&) {
	CString saddress;
	GetDlgItemText(IDC_ADDRESS, saddress);
	std::wstringstream ss;
	ss << std::hex << (PCWSTR)saddress;
	void* address;
	ss >> address;

	int len = GetDlgItemInt(IDC_LEN);
	m_ListBox.ResetContent();
	CString value;
	for (int i = 0; i < len; i++) {
		value.Format(L"%02X", *((BYTE*)address + i));
		m_ListBox.AddString(value);
	}

	return 0;
}
