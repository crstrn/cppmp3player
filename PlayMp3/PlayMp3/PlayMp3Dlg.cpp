
// PlayMp3Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "PlayMp3.h"
#include "PlayMp3Dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SLIDER_TIME_ELAPSE_MAX_POS 100000

// CPlayMp3Dlg dialog




CPlayMp3Dlg::CPlayMp3Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPlayMp3Dlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPlayMp3Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDT_MP3_FILE, m_edtMp3File);
	DDX_Control(pDX, IDC_BTN_PLAY, m_btnPlay);
	DDX_Control(pDX, IDC_BTN_PAUSE, m_btnPause);
	DDX_Control(pDX, IDC_BTN_STOP, m_btnStop);
	DDX_Control(pDX, IDC_SLIDER_VOLUME, m_sliderVolume);
	DDX_Control(pDX, IDC_STATIC_DURATION, m_staticDuration);
	DDX_Control(pDX, IDC_SLIDER_TIME_ELAPSED, m_sliderTimeElapsed);
}

BEGIN_MESSAGE_MAP(CPlayMp3Dlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_PLAY, &CPlayMp3Dlg::OnBnClickedBtnPlay)
	ON_BN_CLICKED(IDC_BTN_PAUSE, &CPlayMp3Dlg::OnBnClickedBtnPause)
	ON_BN_CLICKED(IDC_BTN_STOP, &CPlayMp3Dlg::OnBnClickedBtnStop)
	ON_BN_CLICKED(IDC_BTN_BROWSE, &CPlayMp3Dlg::OnBnClickedBtnBrowse)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()


// CPlayMp3Dlg message handlers

BOOL CPlayMp3Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_playerDll.LoadDLL(L"LibMP3DLL.dll");
	playing = false;
	pause = false;

	m_sliderVolume.SetRangeMin(0);
	m_sliderVolume.SetRangeMax(10000);
	m_sliderVolume.SetPos(7000);

	m_sliderTimeElapsed.SetRangeMin(0);
	m_sliderTimeElapsed.SetRangeMax(SLIDER_TIME_ELAPSE_MAX_POS);
	m_sliderTimeElapsed.SetPos(0);

	m_btnPlay.EnableWindow(TRUE);
	m_btnPause.EnableWindow(FALSE);
	m_btnStop.EnableWindow(FALSE);


	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPlayMp3Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPlayMp3Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CPlayMp3Dlg::OnDestroy()
{
	CDialogEx::OnDestroy();


	if(playing)
	{
		m_playerDll.Stop();
	}

	m_playerDll.Cleanup();
	m_playerDll.UnloadDLL();
}


void CPlayMp3Dlg::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == 0)
	{
		if(playing)
		{
			long EvCode = 0;
			bool b = m_playerDll.WaitForCompletion(0, &EvCode);

			if(b)
			{
				playing = false;
				KillTimer(0);
				TRACE(L"Playing stopped");

				m_btnPlay.EnableWindow(TRUE);
				m_btnPause.EnableWindow(FALSE);
				m_btnStop.EnableWindow(FALSE);
			}

			if(pause==false)
			{
				CString sDuration;
				int duration = m_playerDll.GetDuration()/10000000;
				__int64 curPos = m_playerDll.GetCurrentPosition();

				int timeElapsedSec = curPos/10000000;
				sDuration.Format(L"%d:%02d/%d:%02d", timeElapsedSec/60,  timeElapsedSec%60, duration/60, duration%60);
				m_staticDuration.SetWindowTextW(sDuration);
				__int64 pos = (curPos * SLIDER_TIME_ELAPSE_MAX_POS)/m_playerDll.GetDuration();
				m_sliderTimeElapsed.SetPos((int)(pos));
			}
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CPlayMp3Dlg::OnBnClickedBtnPlay()
{
	if(pause)
	{
		m_playerDll.Play();
		pause = false;

		m_btnPlay.EnableWindow(FALSE);
		m_btnPause.EnableWindow(TRUE);
		m_btnStop.EnableWindow(TRUE);
	}
	else
	{
		CString file;
		m_edtMp3File.GetWindowTextW(file);
		if(file==L"")
		{
			MessageBox(L"No MP3 file is specified!", L"Error");
			return;
		}

		if(FileExists(file)==false)
		{
			MessageBox(L"MP3 file does not exists!", L"Error");
			return;
		}

		if(m_playerDll.Load(file)&&m_playerDll.Play())
		{
			playing = true;
			pause = false;
			this->SetTimer(0, 500, NULL);

			m_playerDll.SetVolume(GetVolume());

			m_sliderTimeElapsed.SetPos(0);

			m_btnPlay.EnableWindow(FALSE);
			m_btnPause.EnableWindow(TRUE);
			m_btnStop.EnableWindow(TRUE);
		}
		else
		{
			MessageBox(L"Cannot play this file!", L"Error");
			return;
		}
	}
}


void CPlayMp3Dlg::OnBnClickedBtnPause()
{
	if(m_playerDll.Pause())
	{
		pause = true;

		m_btnPlay.EnableWindow(TRUE);
		m_btnPause.EnableWindow(FALSE);
		m_btnStop.EnableWindow(FALSE);
	}
}


void CPlayMp3Dlg::OnBnClickedBtnStop()
{
	if(m_playerDll.Stop())
	{
		playing = false;
		pause = false;
		this->KillTimer(0);

		m_btnPlay.EnableWindow(TRUE);
		m_btnPause.EnableWindow(FALSE);
		m_btnStop.EnableWindow(FALSE);
	}
}


void CPlayMp3Dlg::OnBnClickedBtnBrowse()
{
	CFileDialog fOpenDlg(
		TRUE, _T("mp3"), _T("*.mp3"), OFN_HIDEREADONLY|OFN_FILEMUSTEXIST, 
		_T("MP3 files (*.mp3)|*.mp3|All files (*.*)|*.*||"), this);

	fOpenDlg.m_pOFN->lpstrTitle=_T("MP3 file");

	CString szMkvPath;
	if(fOpenDlg.DoModal()==IDOK)
	{
		szMkvPath = fOpenDlg.GetPathName();
		m_edtMp3File.SetWindowText(szMkvPath);
	}
}

/* Return TRUE if file 'fileName' exists */
bool CPlayMp3Dlg::FileExists(const TCHAR *fileName)
{
	DWORD       fileAttr;

	fileAttr = GetFileAttributes(fileName);
	if (0xFFFFFFFF == fileAttr)
		return false;
	return true;
}

int CPlayMp3Dlg::GetVolume()
{
	int vol = m_sliderVolume.GetPos();
	vol = 10000 - vol;
	vol = -vol;

	return vol;
}

void CPlayMp3Dlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CScrollBar* VolScrollBar = (CScrollBar*)(&m_sliderVolume);
	CScrollBar* PosScrollBar = (CScrollBar*)(&m_sliderTimeElapsed);
	if(pScrollBar==VolScrollBar)
		m_playerDll.SetVolume(GetVolume());
	else if(pScrollBar==PosScrollBar)
	{
		if(playing && pause==false)
		{
			__int64 duration = m_playerDll.GetDuration();
			__int64 pos =  m_sliderTimeElapsed.GetPos()* duration/SLIDER_TIME_ELAPSE_MAX_POS;
			m_playerDll.SetPositions(&pos, &duration, true);
		}
		else
		{
			__int64 duration = m_playerDll.GetDuration();
			__int64 pos =  m_sliderTimeElapsed.GetPos()* duration/SLIDER_TIME_ELAPSE_MAX_POS;
			__int64 posStop =  pos;
			m_playerDll.SetPositions(&pos, &posStop, true);

			if(pause)
			{
				m_playerDll.SetPositions(&pos, &duration, true);
				m_playerDll.Pause();
			}
			else
			{
				m_playerDll.SetPositions(&pos, &posStop, true);
			}
		}
		CString sDuration;
		int duration = m_playerDll.GetDuration()/10000000;
		__int64 curPos = m_playerDll.GetCurrentPosition();

		int timeElapsedSec = curPos/10000000;
		sDuration.Format(L"%d:%02d/%d:%02d", timeElapsedSec/60,  timeElapsedSec%60, duration/60, duration%60);
		m_staticDuration.SetWindowTextW(sDuration);
	}

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}
