#include <stdio.h>
#include <math.h>
#include <windows.h>
#include "ac3file_dlg.h"
#include "guids.h"
#include "resource.h"

#define SAFE_RELEASE(p) { if (p) p->Release(); p = 0; }

int compact_size(double size)
{
  int iter = 0;
  while (size >= 10000 && iter < 5)
  {
    size /= 1024;
    iter++;
  }
  return (int)size;
}

const char *compact_suffix(double size)
{
  static const char *suffixes[] = { "", "K", "M", "G", "T", "P" };

  int iter = 0;
  while (size >= 10000 && iter < 5)
  {
    size /= 1024;
    iter++;
  }
  return suffixes[iter];
}

///////////////////////////////////////////////////////////////////////////////
// Initialization / Deinitialization
///////////////////////////////////////////////////////////////////////////////

CUnknown * WINAPI AC3FileDlg::Create(LPUNKNOWN lpunk, HRESULT *phr)
{
  DbgLog((LOG_TRACE, 3, "CreateInstance of AC3File property page"));
  CUnknown *punk = new AC3FileDlg("AC3File property page", lpunk, phr, IDD_FILE, IDS_FILE, 0);
  if (punk == NULL) 
    *phr = E_OUTOFMEMORY;
  return punk;
}

AC3FileDlg::AC3FileDlg(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr, int DialogId, int TitleId, int _flags) 
:CBasePropertyPage(pName, pUnk, DialogId, TitleId)
{
  DbgLog((LOG_TRACE, 3, "AC3FileDlg::AC3FileDlg()"));

  filter = 0;
  visible = false;
  refresh = false;
  refresh_time = 500;
}

HRESULT 
AC3FileDlg::OnConnect(IUnknown *pUnknown)
{
  DbgLog((LOG_TRACE, 3, "AC3FileDlg::OnConnect()"));

  pUnknown->QueryInterface(IID_IAC3File, (void **)&filter);
  if (!filter)
  {
    DbgLog((LOG_TRACE, 3, "AC3FileDlg::OnConnect() Failed!"));
    SAFE_RELEASE(filter);
    return E_NOINTERFACE; 
  }

  return NOERROR;
}

HRESULT 
AC3FileDlg::OnDisconnect()
{
  DbgLog((LOG_TRACE, 3, "AC3FileDlg::OnDisconnect()"));
  SAFE_RELEASE(filter);
  return NOERROR;
}

HRESULT 
AC3FileDlg::OnActivate()
{
  DbgLog((LOG_TRACE, 3, "AC3FileDlg::OnActivate()"));

  visible = false;
  refresh = true;

  init_controls();
  set_dynamic_controls();
  set_controls();

  SetTimer(m_hwnd, 1, refresh_time, 0);

  return NOERROR;
}

HRESULT 
AC3FileDlg::OnDeactivate()
{
  DbgLog((LOG_TRACE, 3, "AC3FileDlg::OnDeactivate()"));

  KillTimer(m_hwnd, 1);

  return NOERROR;
}

///////////////////////////////////////////////////////////////////////////////
// Handle messages
///////////////////////////////////////////////////////////////////////////////

INT_PTR 
AC3FileDlg::OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_COMMAND:
      command(LOWORD(wParam), HIWORD(wParam));
      return 1;

    case WM_HSCROLL:
    case WM_VSCROLL:
      command(GetDlgCtrlID((HWND)lParam), LOWORD(wParam));
      return 1;

    case WM_TIMER:
      if (IsWindowVisible(hwnd))
        if (visible)
          set_dynamic_controls();
        else
        {
          refresh = true;
          visible = true;
          set_controls();
          set_dynamic_controls();
        }
      else
        visible = false;

      return 1;

  }

  return CBasePropertyPage::OnReceiveMessage(hwnd, uMsg, wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////
// Controls initalization/update
///////////////////////////////////////////////////////////////////////////////


void 
AC3FileDlg::reload_state()
{}

void 
AC3FileDlg::init_controls()
{
  DbgLog((LOG_TRACE, 3, "AC3FileDlg::init_controls()"));
}

void 
AC3FileDlg::set_dynamic_controls()
{
  static char   info[1024] = "";
  static double frames = 0;
  static double bytes  = 0;
  static double time   = 0;
  
  char   new_info[1024];
  double new_frames;
  double new_bytes;
  double new_time;

  filter->get_info(new_info, sizeof(new_info));
  filter->get_stat(0, &new_bytes,  AC3FILE_BYTES);
  filter->get_stat(0, &new_time,   AC3FILE_TIME);
  filter->get_stat(0, &new_frames, AC3FILE_FRAMES);

  /////////////////////////////////////
  // Stream info

  if (memcmp(new_info, info, strlen(info)) || refresh)
  {
    memcpy(info, new_info, sizeof(info));
    SendDlgItemMessage(m_Dlg, IDC_EDT_INFO, WM_SETTEXT, 0, (LPARAM)(LPSTR)info);
  }

  /////////////////////////////////////
  // Stats

  if (fabs(frames - new_frames) > 1 || refresh)
  {
    frames = new_frames;

    char str[256];
    sprintf(str, "%.0f", frames);
    SetDlgItemText(m_Dlg, IDC_EDT_FRAMES, str);
  }

  if (fabs(new_bytes - bytes) > 1 || refresh)
  {
    bytes = new_bytes;

    char str[256];
    sprintf(str, "%.0f (%i %sB)", bytes, compact_size(bytes), compact_suffix(bytes));
    SetDlgItemText(m_Dlg, IDC_EDT_FILEPOS, str);
  }

  if (fabs(new_time - time) > 0.001 || refresh)
  {
    time = new_time;
    int hr  = int(time / 3600);
    int min = int(time - hr * 3600) / 60;
    int sec = int(time - hr * 3600 - min * 60);
    int ms  = int((time - int(time)) * 1000);

    char str[256];
    sprintf(str, "%i:%02i:%02i.%03i", hr, min, sec, ms);
    SetDlgItemText(m_Dlg, IDC_EDT_TIME, str);
  }

  refresh = false;
}

void 
AC3FileDlg::set_controls()
{}


///////////////////////////////////////////////////////////////////////////////
// Commands
///////////////////////////////////////////////////////////////////////////////

void 
AC3FileDlg::command(int control, int message)
{}
