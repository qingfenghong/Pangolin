/*
* Author：王德明
* Email：phight@163.com
* QQ群：220954528
*/


// PangolinDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Pangolin.h"
#include "PangolinDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define CODEC_STATISTICS_TIMER 1

// CPangolinDlg 对话框



CPangolinDlg::CPangolinDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_PANGOLIN_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPangolinDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPangolinDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_NOTIFY(TCN_SELCHANGE, IDC_SETTING, &CPangolinDlg::OnTabChange)
	ON_BN_CLICKED(IDC_PUSH, &CPangolinDlg::OnBnClickedPush)
	ON_CBN_SELCHANGE(IDC_VIDEO_FORMAT, &CPangolinDlg::OnVideoChange)
	ON_CBN_SELCHANGE(IDC_AUDIO_FORMAT, &CPangolinDlg::OnAudioChange)
	ON_CBN_SELCHANGE(IDC_VIDEO_CAP, &CPangolinDlg::OnVideoCaptureChange)
	ON_CBN_SELCHANGE(IDC_AUDIO_CAP, &CPangolinDlg::OnAudioCaptureChange)
END_MESSAGE_MAP()


// CPangolinDlg 消息处理程序

BOOL CPangolinDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
    InitControlPosition();
    CTabCtrl* hTab = (CTabCtrl*)GetDlgItem(IDC_SETTING);
    hTab->InsertItem(0, TEXT("编码参数"));
	hTab->InsertItem(1, TEXT("采集参数"));
	hTab->InsertItem(2, TEXT("关于"));

    // 初始化采集器
	Capture::Init();
    videoCapture = Capture::GetVideoCature(0);
	if (videoCapture==NULL)
	{
		MessageBox(TEXT("未检测到摄像头"));
		CButton* hBtn = (CButton*)this->GetDlgItem(IDC_PUSH);
		hBtn->EnableWindow(FALSE);
	}
    audioCapture = Capture::GetAudioCature(0);

	//初始化控件数据
    CComboBox* hComBox = NULL;
    CEdit* hEdit = NULL;
    CString str;
	int vCnt, aCnt;

	const vector<Capture *> *captureList;
	vector<Capture *>::const_iterator it;

	hComBox = (CComboBox*)this->GetDlgItem(IDC_VIDEO_CAP);
	vCnt = Capture::EnumVideoCature(captureList);
	if (vCnt>0)
	{
		for (it = captureList->begin(); it != captureList->end(); it++)
		{
			Capture * capture = *it;
			hComBox->AddString(capture->GetName());
		}
		hComBox->SetCurSel(0);
	}

	hComBox = (CComboBox*)this->GetDlgItem(IDC_AUDIO_CAP);
	aCnt = Capture::EnumAudioCature(captureList);
	if (aCnt>0)
	{
		for (it = captureList->begin(); it != captureList->end(); it++)
		{
			Capture * capture = *it;
			hComBox->AddString(capture->GetName());
		}
		hComBox->SetCurSel(0);
	}

	InitVideoAttribute();
	InitAudioAttribute();

	hComBox = (CComboBox*)this->GetDlgItem(IDC_VIDEO_CODEC);
	hComBox->SetCurSel(0);
	hEdit = (CEdit*)this->GetDlgItem(IDC_VIDEO_BITRATE);
	hEdit->SetWindowTextW(L"2000");

	hComBox = (CComboBox*)this->GetDlgItem(IDC_AUDIO_CODEC);
	hComBox->SetCurSel(0);
	hEdit = (CEdit*)this->GetDlgItem(IDC_AUDIO_BITRATE);
	hEdit->SetWindowTextW(L"64");

    hEdit = (CEdit*)this->GetDlgItem(IDC_RTMPURL);
    hEdit->SetWindowTextW(L"rtmp://127.0.0.1/live/test");
    hEdit->SetSel(100);
    hEdit->SetFocus();

    ShowCodecParamTab(SW_SHOW);
    ShowCaptureParamTab(SW_HIDE);
    ShowAboutTab(SW_HIDE);

    // 初始化Render和Codec
    Render::Init(GetDlgItem(IDC_RENDER)->GetSafeHwnd());
    render = Render::GetRender();
    codec = new Codec();
    rtmpc = new Rtmpc(codec);

	if (videoCapture)
	{
		videoCapture->AddSink(render);
		videoCapture->AddSink(codec);
		videoCapture->Start();
	}

	if (audioCapture)
	{
		audioCapture->AddSink(codec);
		audioCapture->Start();
	}

	return FALSE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CPangolinDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}


void CPangolinDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (videoCapture)
	{
		videoCapture->Stop();
	}
	if (audioCapture)
	{
		audioCapture->Stop();
	}

	if (rtmpc)
	{
		rtmpc->Stop();
	}
	if (codec)
	{
		codec->Stop();
	}

	CDialogEx::OnClose(); 
}


void CPangolinDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 在此处添加消息处理程序代码

	Capture::Uninit();

	if (rtmpc)
	{
		delete rtmpc;
	}
	if (codec)
	{
		delete codec;
	}

	Render::Uninit();

}


void CPangolinDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (nIDEvent == CODEC_STATISTICS_TIMER)
	{
		if (codec)
		{
			CodecStatistics code_statistics;
			codec->GetCodecStatistics(&code_statistics);

			RtmpStatistics rtmp_statistics;
			rtmpc->GetRtmpStatistics(&rtmp_statistics);

			CString str;
			str.Format(TEXT("FPS: %.1f    Video: %.1fKB/s    Audio: %.1fKB/s\n"), code_statistics.videoDecFps, ((double)rtmp_statistics.videoBitrate)/1000, ((double)rtmp_statistics.audioBitrate)/1000);

			CWnd* hChild = this->GetDlgItem(IDC_STATIC_STATUS);
			hChild->SetWindowText(str);

		}
	}

	CDialogEx::OnTimer(nIDEvent);
}


//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CPangolinDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CPangolinDlg::InitVideoAttribute()
{
	if (videoCapture==NULL)
	{
		return;
	}

	vector<VideoCaptureAttribute*> *pVideoAttribute = NULL;
	VideoCaptureAttribute videoAttribute = { 0 };

	CComboBox* hComBox = (CComboBox*)this->GetDlgItem(IDC_VIDEO_FORMAT);
	hComBox->ResetContent();

	int vCnt = videoCapture->GetSupportAttribute((void*)&pVideoAttribute);
	videoCapture->GetConfig(&videoAttribute);
	for (int i = 0; i < vCnt; i++)
	{
        CString str;
        str.Format(TEXT("%s,%dx%dp%d"), GetFormatName((*pVideoAttribute)[i]->format),(*pVideoAttribute)[i]->width, (*pVideoAttribute)[i]->height, (*pVideoAttribute)[i]->fps);
        hComBox->AddString(str);
        hComBox->SetItemData(i, (DWORD_PTR)(*pVideoAttribute)[i]);
        if ((*pVideoAttribute)[i]->width == videoAttribute.width && (*pVideoAttribute)[i]->height == videoAttribute.height && (*pVideoAttribute)[i]->fps == videoAttribute.fps)
        {
            hComBox->SetCurSel(i);
        }
	}
}


void CPangolinDlg::InitAudioAttribute()
{
	if (audioCapture == NULL)
	{
		return;
	}

	AudioCaptureAttribute audioAttribute = { 0 };
	vector<AudioCaptureAttribute*> *pAudioAttribute = NULL;

	CComboBox* hComBox = (CComboBox*)this->GetDlgItem(IDC_AUDIO_FORMAT);
	hComBox->ResetContent();

	int aCnt = audioCapture->GetSupportAttribute((void*)&pAudioAttribute);
	audioCapture->GetConfig(&audioAttribute);
	for (int i = 0; i < aCnt; i++)
    {
        CString str;
        str.Format(TEXT("%s,%d,%d位"), (*pAudioAttribute)[i]->channel == 2 ? TEXT("立体声") : TEXT("单声道"), (*pAudioAttribute)[i]->samplerate, (*pAudioAttribute)[i]->bitwide);
		hComBox->AddString(str);
		hComBox->SetItemData(i, (DWORD_PTR)(*pAudioAttribute)[i]);
		if (audioAttribute.samplerate == (*pAudioAttribute)[i]->samplerate && audioAttribute.channel == (*pAudioAttribute)[i]->channel && audioAttribute.bitwide == (*pAudioAttribute)[i]->bitwide)
		{
			hComBox->SetCurSel(i);
		}
	}
}


void CPangolinDlg::InitControlPosition()
{
    CRect rect;
    this->GetClientRect(&rect);
    this->SetWindowPos(NULL, 0, 0, 600, 610, FALSE);

    CWnd* hChild = NULL;

    hChild = this->GetDlgItem(IDC_RENDER);
    hChild->SetWindowPos(NULL, 4, 4, 576, 324, 0);

    hChild = this->GetDlgItem(IDC_STATIC1);
    hChild->SetWindowPos(NULL, 14, 340, 70, 30, 0);

    hChild = this->GetDlgItem(IDC_RTMPURL);
    hChild->SetWindowPos(NULL, 84, 338, 400, 24, 0);

    hChild = this->GetDlgItem(IDC_PUSH);
    hChild->SetWindowPos(NULL, 490, 336, 80, 28, 0);

    CTabCtrl* hTab = (CTabCtrl*)GetDlgItem(IDC_SETTING);
    hTab->SetWindowPos(NULL, 4, 370, 300, 180, 0);

    hChild = this->GetDlgItem(IDC_LOG);
	hChild->SetWindowPos(NULL, 314, 370, 260, 180, 0);

	hChild = this->GetDlgItem(IDC_STATIC_STATUS);
	hChild->SetWindowPos(NULL, 260, 552, 300, 30, 0);

	hChild = this->GetDlgItem(IDC_STATIC_COPYRIGHT);
	hChild->SetWindowPos(NULL, 4, 552, 200, 30, 0);

    //编码参数
    hChild = this->GetDlgItem(IDC_STATIC10);
    hChild->SetWindowPos(NULL, 30, 414, 60, 18, 0);
    hChild = this->GetDlgItem(IDC_VIDEO_CODEC);
    hChild->SetWindowPos(NULL, 110, 410, 140, 24, 0);
    hChild = this->GetDlgItem(IDC_STATIC11);
    hChild->SetWindowPos(NULL, 30, 446, 60, 18, 0);
    hChild = this->GetDlgItem(IDC_VIDEO_BITRATE);
    hChild->SetWindowPos(NULL, 110, 442, 140, 24, 0);
    hChild = this->GetDlgItem(IDC_STATIC12);
    hChild->SetWindowPos(NULL, 30, 478, 60, 18, 0);
    hChild = this->GetDlgItem(IDC_AUDIO_CODEC);
    hChild->SetWindowPos(NULL, 110, 474, 140, 24, 0);
    hChild = this->GetDlgItem(IDC_STATIC13);
    hChild->SetWindowPos(NULL, 30, 510, 60, 18, 0);
    hChild = this->GetDlgItem(IDC_AUDIO_BITRATE);
    hChild->SetWindowPos(NULL, 110, 506, 140, 24, 0);

    //采集源
    hChild = this->GetDlgItem(IDC_STATIC20);
    hChild->SetWindowPos(NULL, 30, 414, 40, 18, 0);
    hChild = this->GetDlgItem(IDC_VIDEO_CAP);
    hChild->SetWindowPos(NULL, 90, 410, 185, 24, 0);
    hChild = this->GetDlgItem(IDC_STATIC21);
	hChild->SetWindowPos(NULL, 30, 446, 50, 18, 0);
	hChild = this->GetDlgItem(IDC_VIDEO_FORMAT);
	hChild->SetWindowPos(NULL, 90, 442, 185, 24, 0);
	hChild = this->GetDlgItem(IDC_STATIC22);
	hChild->SetWindowPos(NULL, 30, 478, 40, 18, 0);
	hChild = this->GetDlgItem(IDC_AUDIO_CAP);
	hChild->SetWindowPos(NULL, 90, 474, 185, 24, 0);
	hChild = this->GetDlgItem(IDC_STATIC23);
	hChild->SetWindowPos(NULL, 30, 510, 50, 18, 0);
	hChild = this->GetDlgItem(IDC_AUDIO_FORMAT);
	hChild->SetWindowPos(NULL, 90, 506, 185, 24, 0);

    //关于
    hChild = this->GetDlgItem(IDC_STATIC30);
    hChild->SetWindowPos(NULL, 30, 414, 50, 18, 0);
    hChild = this->GetDlgItem(IDC_LINK_PROJECT);
    hChild->SetWindowPos(NULL, 90, 414, 54, 18, 0);
    hChild = this->GetDlgItem(IDC_STATIC31);
    hChild->SetWindowPos(NULL, 30, 446, 60, 18, 0);
    hChild = this->GetDlgItem(IDC_LINK_QQ);
    hChild->SetWindowPos(NULL, 90, 446, 60, 18, 0);
}


void CPangolinDlg::ShowCodecParamTab(int bShow)
{
    CWnd* hChild = NULL;
    hChild = this->GetDlgItem(IDC_STATIC10);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_VIDEO_CODEC);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_STATIC11);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_VIDEO_BITRATE);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_STATIC12);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_AUDIO_CODEC);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_STATIC13);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_AUDIO_BITRATE);
    hChild->ShowWindow(bShow);
}


void CPangolinDlg::ShowCaptureParamTab(int bShow)
{
    CWnd* hChild = NULL;
    hChild = this->GetDlgItem(IDC_STATIC20);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_VIDEO_CAP);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_STATIC21);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_AUDIO_CAP);
	hChild->ShowWindow(bShow);
	hChild = this->GetDlgItem(IDC_STATIC22);
	hChild->ShowWindow(bShow);
	hChild = this->GetDlgItem(IDC_VIDEO_FORMAT);
	hChild->ShowWindow(bShow);
	hChild = this->GetDlgItem(IDC_STATIC23);
	hChild->ShowWindow(bShow);
	hChild = this->GetDlgItem(IDC_AUDIO_FORMAT);
	hChild->ShowWindow(bShow);
}


void CPangolinDlg::ShowAboutTab(int bShow)
{
    CWnd* hChild = NULL;
    hChild = this->GetDlgItem(IDC_STATIC30);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_LINK_PROJECT);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_STATIC31);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_LINK_QQ);
    hChild->ShowWindow(bShow);
}


void CPangolinDlg::OnTabChange(NMHDR *pNMHDR, LRESULT *pResult)
{
    // TODO: 在此添加控件通知处理程序代码
    CTabCtrl* hTab = (CTabCtrl*)GetDlgItem(IDC_SETTING);
    int sel = hTab->GetCurSel();
    switch (sel)
    {
    case 0:
    {
        ShowCodecParamTab(SW_SHOW);
        ShowCaptureParamTab(SW_HIDE);
        ShowAboutTab(SW_HIDE);
    }
        break;
    case 1:
    {
		ShowCodecParamTab(SW_HIDE);
        ShowCaptureParamTab(SW_SHOW);
        ShowAboutTab(SW_HIDE);
    }
    break;
    case 2:
	{
		ShowCodecParamTab(SW_HIDE);
		ShowCaptureParamTab(SW_HIDE);
		ShowAboutTab(SW_SHOW);
    }
    break;
    }

    *pResult = 0;
}

void CPangolinDlg::EnableAllControl(int bEnable)
{
    CWnd* hChild = NULL;
    hChild = this->GetDlgItem(IDC_RTMPURL);
    ((CEdit*)hChild)->SetReadOnly(!bEnable);

    //编码参数
    hChild = this->GetDlgItem(IDC_VIDEO_CODEC);
    hChild->EnableWindow(bEnable);
    hChild = this->GetDlgItem(IDC_VIDEO_BITRATE);
	hChild->EnableWindow(bEnable);
	hChild = this->GetDlgItem(IDC_AUDIO_CODEC);
	hChild->EnableWindow(bEnable);
	hChild = this->GetDlgItem(IDC_AUDIO_BITRATE);
	hChild->EnableWindow(bEnable);

    //采集源
    hChild = this->GetDlgItem(IDC_VIDEO_CAP);
    hChild->EnableWindow(bEnable);
    hChild = this->GetDlgItem(IDC_AUDIO_CAP);
	hChild->EnableWindow(bEnable);
	hChild = this->GetDlgItem(IDC_VIDEO_FORMAT);
	hChild->EnableWindow(bEnable);
	hChild = this->GetDlgItem(IDC_AUDIO_FORMAT);
	hChild->EnableWindow(bEnable);
}


void CPangolinDlg::GetVideoAttribute(VideoCodecAttribute *pattr)
{
    if (pattr!=NULL)
    {
        int sel = 0;
        CWnd* hChild = NULL;
        CString str;

        hChild = this->GetDlgItem(IDC_VIDEO_CODEC);
        sel = ((CComboBox*)hChild)->GetCurSel();
        pattr->profile = sel;

        hChild = this->GetDlgItem(IDC_VIDEO_BITRATE);
        hChild->GetWindowText(str);
        pattr->bitrate = _ttoi(str);
    }
}


void CPangolinDlg::GetAudioAttribute(AudioCodecAttribute *pattr)
{
    if (pattr != NULL)
    {
        int sel = 0;
        CWnd* hChild = NULL;
        CString str;

        hChild = this->GetDlgItem(IDC_AUDIO_CODEC);
        sel = ((CComboBox*)hChild)->GetCurSel();
        pattr->profile = sel;

        hChild = this->GetDlgItem(IDC_AUDIO_BITRATE);
        hChild->GetWindowText(str);
        pattr->bitrate = _ttoi(str);

    }
}


//////////////////////////////////////////////////////////////////////////
// Control Event Handle
//////////////////////////////////////////////////////////////////////////

void CPangolinDlg::OnBnClickedPush()
{
    static int curState = 0;
    if (curState == 0)
    {
        CWnd* hChild = this->GetDlgItem(IDC_PUSH);
        hChild->SetWindowText(TEXT("停止推流"));
        curState = 1;
        this->EnableAllControl(FALSE);

		VideoCodecAttribute v_attribute = {0};
        this->GetVideoAttribute(&v_attribute);
        codec->SetVideoCodecAttribute(&v_attribute);

		AudioCodecAttribute a_attribute = {0};
        this->GetAudioAttribute(&a_attribute);
        codec->SetAudioCodecAttribute(&a_attribute);

        CString strUrl;
        char url[256] = {0};
        CEdit *pEdit = (CEdit *)this->GetDlgItem(IDC_RTMPURL);
        pEdit->GetWindowText(strUrl);
        WideCharToMultiByte(CP_ACP, 0, strUrl, strUrl.GetLength(), url, 256, NULL, NULL);
        rtmpc->SetConfig(url);

        codec->Start();
        rtmpc->Start();

		SetTimer(CODEC_STATISTICS_TIMER, 1000, NULL);
    }
    else if (curState == 1)
    {
        CWnd* hChild = this->GetDlgItem(IDC_PUSH);
        hChild->SetWindowText(TEXT("开始推流"));
        curState = 0;
        this->EnableAllControl(TRUE);

		KillTimer(CODEC_STATISTICS_TIMER);
		hChild = this->GetDlgItem(IDC_STATIC_STATUS);
		hChild->SetWindowText(TEXT("状态："));

        codec->Stop();
        rtmpc->Stop();
    }
}

void CPangolinDlg::OnVideoChange()
{
	// TODO: 在此添加控件通知处理程序代码

	int sel = 0;
	CWnd* hChild = NULL;

	hChild = this->GetDlgItem(IDC_VIDEO_FORMAT);
	sel = ((CComboBox*)hChild)->GetCurSel();
	VideoCaptureAttribute* pattr = (VideoCaptureAttribute*)((CComboBox*)hChild)->GetItemData(sel);

	videoCapture->SetConfig(pattr);
}


void CPangolinDlg::OnAudioChange()
{
	// TODO: 在此添加控件通知处理程序代码

	int sel = 0;
	CWnd* hChild = NULL;

	hChild = this->GetDlgItem(IDC_AUDIO_FORMAT);
	sel = ((CComboBox*)hChild)->GetCurSel();
	AudioCaptureAttribute* pattr = (AudioCaptureAttribute*)((CComboBox*)hChild)->GetItemData(sel);
	
	audioCapture->SetConfig(pattr);
}


void CPangolinDlg::OnVideoCaptureChange()
{
	// TODO: 在此添加控件通知处理程序代码

	int sel = 0;
	CWnd* hChild = NULL;

	hChild = this->GetDlgItem(IDC_VIDEO_CAP);
	sel = ((CComboBox*)hChild)->GetCurSel();

	if (videoCapture)
	{
		videoCapture->Stop();
	}
	videoCapture = Capture::GetVideoCature(sel);
	if (videoCapture)
	{
		InitVideoAttribute();
		videoCapture->AddSink(codec);
		videoCapture->AddSink(render);
		videoCapture->Start();
	}
}


void CPangolinDlg::OnAudioCaptureChange()
{
	// TODO: 在此添加控件通知处理程序代码

	int sel = 0;
	CWnd* hChild = NULL;

	hChild = this->GetDlgItem(IDC_AUDIO_CAP);
	sel = ((CComboBox*)hChild)->GetCurSel();

	if (audioCapture)
	{
		audioCapture->Stop();
	}
	audioCapture = Capture::GetAudioCature(sel);
	if (audioCapture)
	{
		InitAudioAttribute();
		audioCapture->AddSink(codec);
		audioCapture->Start();
	}
}
