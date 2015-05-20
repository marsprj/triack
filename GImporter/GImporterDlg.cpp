
// GImporterDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "GImporter.h"
#include "GImporterDlg.h"
#include "afxdialogex.h"
#include "ImageFilter.h"
#include <io.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_UPDATE_LOG WM_USER +6

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

#define THREAD_NUM 8

HANDLE g_hEvent_1 = NULL;
HANDLE g_hEvent_2 = NULL;
//CRITICAL_SECTION  g_cs;

bool g_exit = false;
char g_tile_folder[_MAX_PATH];
char g_tile_store[_MAX_PATH];
char g_level[_MAX_PATH];
char g_tile_file[_MAX_FNAME];

unsigned char m_r=0;
unsigned char m_g=0;
unsigned char m_b=0;

HANDLE g_hEvent = NULL;

void ImportLevel(CString strDir, CString strLevel);

DWORD WINAPI TestFunc(LPVOID lpParam)
{
	while(true)
	{
		if(g_exit)
		{
			break;
		}
		SetEvent(g_hEvent);
		TRACE1("[%d]:%s\n", GetCurrentThreadId(),g_tile_store);
	}
	
	return 0;
}

DWORD WINAPI ImportTileFunc(LPVOID lpParam)
{
	char tile_file[_MAX_FNAME];
	char tile_path[_MAX_PATH];
	char tile_level[_MAX_FNAME];
	CGImporterDlg* dlg = (CGImporterDlg*)lpParam;
	const char* riak_server = dlg->m_riak_server;
	const char* riak_port = dlg->m_riak_port;
	const char* riak_folder = dlg->m_riak_folder;
	const char* riak_store= dlg->m_riak_store;
	const char* tile_store = dlg->m_tile_folder;

	radi::RiakFS* riak = new radi::RiakFS(riak_server, atoi(riak_port));
	riak->Connect();
	radi::RiakFile* root = riak->GetRoot();
	radi::RiakFile* folder = root->GetRiakFile(riak_folder);
	if(folder==NULL)
	{
		riak->Release();
		return 0;
	}
	radi::RiakFile* rfile = folder->GetRiakFile(riak_store);
	if(rfile==NULL)
	{
		folder->Release();
		riak->Release();
		return 0;
	}
	radi::RiakTileStore* rstore = rfile->GetTileStore();

	CImageFilter filter(m_r,m_g,m_b);

	while(true)
	{
		DWORD id = GetCurrentThreadId();

		if(g_exit)
		{ 
			break;
		}

		WaitForSingleObject(g_hEvent_2, INFINITE);
		strcpy(tile_file, g_tile_file);
		strcpy(tile_level, g_level);
		SetEvent(g_hEvent_1);
		_makepath(tile_path, NULL, g_tile_store, tile_file, NULL);

		if(isdigit(tile_file[0]))
		{
			int row=0,col=0;
			sscanf(tile_file,"%d_%d.png", &row, &col);

			
			//if(filter.Process(g_tile_store, tile_file))
			//{
			//	rstore->PutTile(atoi(tile_level), row, col, filter.GetOutImagePath());
			//	DeleteFile(filter.GetOutImagePath());
			//}
			//else
			{
				rstore->PutTile(atoi(tile_level), row, col, tile_path);
			}

			//EnterCriticalSection(&g_cs);
			char* msg = new char[_MAX_PATH];
			sprintf(msg, "[����]:%s��\t%d��\t%d��",tile_level, row, col);
			::PostMessage(dlg->GetSafeHwnd(), WM_UPDATE_LOG, NULL, (LPARAM)msg);
		}
	}

	riak->Close();
	riak->Release();

	return 0;
}

DWORD WINAPI ImportLevelFunc(LPVOID lpParam)
{
	CGImporterDlg* dlg = (CGImporterDlg*)lpParam;

	CString tile_folder = dlg->m_tile_folder;
	
	char filter[_MAX_PATH];
	_makepath(g_tile_store, NULL, tile_folder, g_level, NULL);
	_makepath(filter, NULL, g_tile_store, "*", "png");
	g_exit = false;

	WIN32_FIND_DATA fd; 
	HANDLE hFind = ::FindFirstFile(filter, &fd);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		char tpath[_MAX_PATH];
		char tkey[_MAX_PATH];

		do
		{	
			if(g_exit)
			{
				break;
			}
			strcpy(g_tile_file, fd.cFileName);
			SetEvent(g_hEvent_2);
			WaitForSingleObject(g_hEvent_1,INFINITE);
		}
		while(::FindNextFile(hFind, &fd));
		::FindClose(hFind);

	}
	return 0;
}

DWORD WINAPI ImportTileStoreFunc(LPVOID lpParam)
{
	CGImporterDlg* dlg = (CGImporterDlg*)lpParam;
	const char* tile_folder = dlg->m_tile_folder;

	char filter[_MAX_PATH];
	_makepath(filter, NULL, tile_folder, "*", NULL);

	DWORD ts = GetTickCount();

	WIN32_FIND_DATA fd; 
	HANDLE hFind = ::FindFirstFile(filter, &fd);
	TRACE0("\n");
	if(hFind != INVALID_HANDLE_VALUE)
	{
		g_hEvent_1 = CreateEvent(NULL, FALSE, FALSE, NULL);  
		g_hEvent_2 = CreateEvent(NULL, FALSE, FALSE, NULL); 
		//InitializeCriticalSection(&g_cs);  
		HANDLE thread_handles[THREAD_NUM];	

		for(int i=0; i<THREAD_NUM; i++)
		{
			thread_handles[i] = CreateThread(NULL,0,ImportTileFunc, lpParam, 0, NULL);
		}

		do
		{	
			if(g_exit)
			{
				break;
			}
			if((fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
			{
				if(fd.cFileName[0]!='.')
				{	
					strcpy(g_level, fd.cFileName);
					ImportLevelFunc(lpParam);
				}
			}
		}
		while(::FindNextFile(hFind, &fd));
		::FindClose(hFind);

		g_exit = true;

		for(int i=0; i<THREAD_NUM; i++)
		{
			SetEvent(g_hEvent_2);
		}
		WaitForMultipleObjects(THREAD_NUM, thread_handles, TRUE, INFINITE);
		//DeleteCriticalSection(&g_cs);  
		CloseHandle(g_hEvent_1);
		CloseHandle(g_hEvent_2);

		DWORD te = GetTickCount();
		char* msg = new char[_MAX_PATH];
		sprintf(msg, "ʱ��:%8.3f��",(te-ts)/1000.0f);
		::PostMessage(dlg->GetSafeHwnd(), WM_UPDATE_LOG,NULL,(LPARAM)msg);

		AfxMessageBox("�������");
	}
	else
	{
		AfxMessageBox("�ļ�·��������");
	}	

	return 0;
}

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	
END_MESSAGE_MAP()


// CGImporterDlg �Ի���


CGImporterDlg::CGImporterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGImporterDlg::IDD, pParent)
	, m_riak_server(_T(""))
	, m_riak_port(_T(""))
	, m_riak_folder(_T(""))
	, m_riak_store(_T(""))
	, m_tile_folder(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_riak = NULL;
	m_thread_handler = NULL;
}

void CGImporterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CMB_RIAK_FOLDER, m_cmb_riak_folder);
	DDX_Control(pDX, IDC_CMB_RIAK_STORE, m_cmb_riak_store);
	DDX_Control(pDX, IDC_CMB_TILE_LEVEL, m_cmb_tile_level);
	DDX_Text(pDX, IDC_RIAK_SERVER, m_riak_server);
	DDX_Text(pDX, IDC_RIAK_PORT, m_riak_port);
	DDX_CBString(pDX, IDC_CMB_RIAK_FOLDER, m_riak_folder);
	DDX_CBString(pDX, IDC_CMB_RIAK_STORE, m_riak_store);
	DDX_Text(pDX, IDC_TILE_FOLDER, m_tile_folder);
	DDX_Control(pDX, IDC_STATIC_LOG, m_static_log);
}

BEGIN_MESSAGE_MAP(CGImporterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_FOLDER, &CGImporterDlg::OnBnClickedBtnFolder)
	ON_BN_CLICKED(IDC_BTN_IMPORT, &CGImporterDlg::OnBnClickedBtnImport)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_RIAK, &CGImporterDlg::OnBnClickedBtnRiak)
	ON_CBN_SELCHANGE(IDC_CMB_RIAK_FOLDER, &CGImporterDlg::OnCbnSelchangeCmbRiakFolder)
	ON_BN_CLICKED(IDOK, &CGImporterDlg::OnBnClickedOk)
	ON_MESSAGE(WM_UPDATE_LOG, OnUpdateLog)
END_MESSAGE_MAP()


// CGImporterDlg ��Ϣ�������

BOOL CGImporterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	GetDlgItem(IDC_RIAK_SERVER)->SetWindowText("192.168.111.151");
	//GetDlgItem(IDC_RIAK_SERVER)->SetWindowText("123.57.207.198");
	//GetDlgItem(IDC_RIAK_SERVER)->SetWindowText("192.168.111.104");
	//GetDlgItem(IDC_RIAK_SERVER)->SetWindowText("192.168.1.172");
	GetDlgItem(IDC_RIAK_PORT)->SetWindowText("8087");

	//m_cmb_riak_folder.AddString("google");
	//m_cmb_riak_folder.SetCurSel(0);
	//m_cmb_riak_store.AddString("level_1");
	//m_cmb_riak_store.SetCurSel(0);

	//GetDlgItem(IDC_TILE_FOLDER)->SetWindowText("E:\\Project\\Project\\GA\\Data\\world_png\\google");
	//GetDlgItem(IDC_TILE_FOLDER)->SetWindowText("G:\\Data\\tar\\song\\world20150430103347");
	GetDlgItem(IDC_TILE_FOLDER)->SetWindowText("E:\\Project\\Project\\GA\\Data\\world_png\\google");

	GetDlgItem(IDC_STATIC_LOG)->SetWindowText("");

	CString str;
	for(int i=1;i<20; i++)
	{
		str.Format("%d", i);
		m_cmb_tile_level.AddString(str);
	}
	m_cmb_tile_level.SetCurSel(0);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CGImporterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CGImporterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CGImporterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CGImporterDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here
	if(m_riak!=NULL)
	{
		m_riak->Close();
		m_riak->Release();
		m_riak = NULL;
	}
}

LRESULT CGImporterDlg::OnUpdateLog(WPARAM wParam,LPARAM lParam)
{
	char* msg = (char*)lParam;
	GetDlgItem(IDC_STATIC_LOG)->SetWindowText(msg);
	delete[] msg;
	return  0;
}

void CGImporterDlg::OnBnClickedBtnRiak()
{
	if(m_riak!=NULL)
	{
		m_riak->Close();
		m_riak->Release();
		m_riak = NULL;

		GetDlgItem(IDC_BTN_RIAK)->SetWindowText("����");
	}
	else
	{
		CString server;
		CString port;
		GetDlgItem(IDC_RIAK_SERVER)->GetWindowText(server);
		GetDlgItem(IDC_RIAK_PORT)->GetWindowText(port);

		m_riak = new radi::RiakFS(server, atoi(port));
		if(!m_riak->Connect())
		{
			AfxMessageBox("�޷����ӵ�riak");

			m_riak->Release();
			m_riak = NULL;
		}
		else
		{
			m_cmb_riak_folder.ResetContent();

			radi::RiakFile* root = m_riak->GetRoot();
			radi::RiakFileSet* pSet = root->GetFiles();
			int count = pSet->GetCount();
			for(int i=0; i<count; i++)
			{
				radi::RiakFile* folder = pSet->GetRiakFile(i);
				if(folder->IsFolder())
				{
					m_cmb_riak_folder.AddString(folder->GetName());
				}
			}
			pSet->Release();
			root->Release();

			if(m_cmb_riak_folder.GetCount()==0)
			{
				m_cmb_riak_folder.AddString("Google");
			}

			m_cmb_riak_folder.SetCurSel(0);
			
			CString strFolder;
			//m_cmb_riak_folder.GetWindowText(strFolder);
			int item = m_cmb_riak_folder.GetCurSel();//(strFolder);
			if(item<0)
			{
				return;
			}
			m_cmb_riak_folder.GetLBText(item, strFolder);
			UpdateRiakStoreCombo(strFolder);

			GetDlgItem(IDC_BTN_RIAK)->SetWindowText("�Ͽ�");

			AfxMessageBox("���ӳɹ�");
		}
	}
}


void CGImporterDlg::OnCbnSelchangeCmbRiakFolder()
{
	// TODO: Add your control notification handler code here
	
	CString strFolder;
	//m_cmb_riak_folder.GetWindowText(strFolder);
	int item = m_cmb_riak_folder.GetCurSel();//(strFolder);
	if(item<0)
	{
		return;
	}
	m_cmb_riak_folder.GetLBText(item, strFolder);
	UpdateRiakStoreCombo(strFolder);

}

void CGImporterDlg::UpdateRiakStoreCombo(CString strFolder)
{
	if(!m_riak)
	{
		return;
	}
	m_cmb_riak_store.ResetContent();
	radi::RiakFile* root = m_riak->GetRoot();
	radi::RiakFile* folder = root->GetRiakFile(strFolder);
	if(folder!=NULL)
	{
		radi::RiakFileSet* pSet = folder->GetFiles();
		int count = pSet->GetCount();
		for(int i=0; i<count; i++)
		{
			radi::RiakFile* rfile = pSet->GetRiakFile(i);
			if(!rfile->IsFolder())
			{
				m_cmb_riak_store.AddString(rfile->GetName());
			}
		}
		pSet->Release();
		folder->Release();
	}

	if(m_cmb_riak_store.GetCount()==0)
	{
		m_cmb_riak_store.AddString("L00");
	}
	m_cmb_riak_store.SetCurSel(0);
	
	root->Release();
}



void CGImporterDlg::OnBnClickedBtnFolder()
{
	//char* msg = new char[_MAX_PATH];
	//strcpy(msg, "�ķ��͵����͵�");
	//::PostMessage(this->GetSafeHwnd(),WM_UPDATE_LOG, NULL, (LPARAM)msg);
	// TODO: Add your control notification handler code here
	TCHAR pszPath[MAX_PATH];
	BROWSEINFO bi; 
	bi.hwndOwner      = this->GetSafeHwnd();
	bi.pidlRoot       = NULL;
	bi.pszDisplayName = NULL; 
	bi.lpszTitle      = TEXT("��ѡ���ļ���"); 
	bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;
	bi.lpfn           = NULL; 
	bi.lParam         = 0;
	bi.iImage         = 0; 

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if (pidl == NULL)
	{
		return;
	}

	if (SHGetPathFromIDList(pidl, pszPath))
	{
		GetDlgItem(IDC_TILE_FOLDER)->SetWindowText(pszPath);
	}
}


void CGImporterDlg::OnBnClickedBtnImport()
{
	//if(m_riak==NULL)
	//{
	//	AfxMessageBox("��δ����Riak");
	//	return;
	//}

	UpdateData(TRUE);

	if(access(m_tile_folder,4))
	{
		AfxMessageBox("��Ƭ�ļ��в�����");
		return;
	}

	radi::RiakFile* root = m_riak->GetRoot();
	radi::RiakFile* folder = root->GetRiakFile(m_riak_folder);
	if(folder==NULL)
	{
		folder = root->CreateFolder(m_riak_folder);
	}
	if(folder==NULL)
	{
		AfxMessageBox("�޷����ӵ�����riak folder�ļ���");
		return;
	}
	radi::RiakFile* rfile = folder->GetRiakFile(m_riak_store);
	if(rfile==NULL)
	{
		rfile = folder->CreateRiakFile(m_riak_store, 1,18,-180,-90,180,90, RADI_DATA_TYPE_GOOGLE_CRS84_QUAD);
	}
	if(rfile==NULL)
	{
		AfxMessageBox("�޷�����riak store");
		folder->Release();
		return;
	}

	m_thread_handler = CreateThread(NULL,0, ImportTileStoreFunc, this, 0, NULL);
	//CreateThread(NULL,0,ImportTileFunc, lpParam, 0, NULL);
	
	//char filter[_MAX_PATH];
	//_makepath(filter, NULL, m_tile_folder, "*", NULL);

	//DWORD ts = GetTickCount();

	//WIN32_FIND_DATA fd; 
	//HANDLE hFind = ::FindFirstFile(filter, &fd);
	//TRACE0("\n");
	//if(hFind != INVALID_HANDLE_VALUE)
	//{
	//	g_hEvent_1 = CreateEvent(NULL, FALSE, FALSE, NULL);  
	//	g_hEvent_2 = CreateEvent(NULL, FALSE, FALSE, NULL); 
	//	//InitializeCriticalSection(&g_cs);  
	//	HANDLE thread_handles[THREAD_NUM];	

	//	for(int i=0; i<THREAD_NUM; i++)
	//	{
	//		thread_handles[i] = CreateThread(NULL,0,ImportTileFunc, this, 0, NULL);
	//	}

	//	do
	//	{	
	//		if((fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
	//		{
	//			if(fd.cFileName[0]!='.')
	//			{	
	//				strcpy(g_level, fd.cFileName);
	//				ImportLevelFunc(this);
	//			}
	//		}
	//	}
	//	while(::FindNextFile(hFind, &fd));
	//	::FindClose(hFind);

	//	g_exit = true;

	//	for(int i=0; i<THREAD_NUM; i++)
	//	{
	//		SetEvent(g_hEvent_2);
	//	}
	//	WaitForMultipleObjects(THREAD_NUM, thread_handles, TRUE, INFINITE);
	//	//DeleteCriticalSection(&g_cs);  
	//	CloseHandle(g_hEvent_1);
	//	CloseHandle(g_hEvent_2);

	//	//DWORD te = GetTickCount();
	//	//CString str;
	//	//str.Format("ʱ��:%8.3f��",(te-ts)/1000.0f);
	//	//GetDlgItem(IDC_STATIC_LOG)->SetWindowText(str);

	//	AfxMessageBox("�������");
	//}
	//else
	//{
	//	AfxMessageBox("�ļ�·��������");
	//}	

}

//
//void CGImporterDlg::OnBnClickedBtnImport()
//{
//	if(m_riak==NULL)
//	{
//		AfxMessageBox("��δ����Riak");
//		return;
//	}
//
//	UpdateData(TRUE);
//
//	if(access(m_tile_folder,4))
//	{
//		AfxMessageBox("��Ƭ�ļ��в�����");
//		return;
//	}
//	
//	radi::RiakFile* root = m_riak->GetRoot();
//	radi::RiakFile* folder = root->GetRiakFile(m_riak_folder);
//	if(folder==NULL)
//	{
//		folder = root->CreateFolder(m_riak_folder);
//	}
//	if(folder==NULL)
//	{
//		AfxMessageBox("�޷����ӵ�����folder�ļ���");
//		return;
//	}
//	radi::RiakFile* rfile = folder->GetRiakFile(m_riak_store);
//	if(rfile==NULL)
//	{
//		rfile = folder->CreateRiakFile(m_riak_store, 1,18,-180,-90,180,90, RADI_DATA_TYPE_GOOGLE_CRS84_QUAD);
//	}
//	if(rfile==NULL)
//	{
//		AfxMessageBox("�޷�����store");
//		folder->Release();
//		return;
//	}
//
//	char filter[_MAX_PATH];
//	_makepath(filter, NULL, m_tile_folder, "*", NULL);
//
//	WIN32_FIND_DATA fd; 
//	HANDLE hFind = ::FindFirstFile(filter, &fd);
//	TRACE0("\n");
//	if(hFind != INVALID_HANDLE_VALUE)
//	{
//		radi::RiakTileStore* store = rfile->GetTileStore();
//		char tpath[_MAX_PATH];
//		char tkey[_MAX_PATH];
//
//		do
//		{	
//			if((fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
//			{
//				if(fd.cFileName[0]!='.')
//				{	
//					CString strLevel = fd.cFileName;
//					ImportLevel(m_tile_folder, strLevel);
//				}
//			}
//		}
//		while(::FindNextFile(hFind, &fd));
//		::FindClose(hFind);
//
//		AfxMessageBox("�������");
//	}
//	else
//	{
//		AfxMessageBox("�ļ�·��������");
//	}	
//
//}

void CGImporterDlg::ImportLevel(CString strDir, CString strLevel)
{
	CString strServer = m_riak_server;
	CString strPort = m_riak_port;
	CString strFolder = m_riak_folder;
	CString strStore = m_riak_store;

	radi::RiakFS* riak = new radi::RiakFS(strServer, atoi(strPort));
	if(!riak->Connect())
	{
		riak->Release();
		return;
	}

	radi::RiakFile* root = m_riak->GetRoot();
	radi::RiakFile* folder = root->GetRiakFile(strFolder);
	if(folder==NULL)
	{
		riak->Close();
		riak->Release();
		return;
	}
	radi::RiakFile* rfile = folder->GetRiakFile(strStore);
	if(rfile==NULL)
	{
		folder->Release();
		riak->Close();
		riak->Release();
		return;
	}

	char store_path[_MAX_PATH];
	_makepath(store_path, NULL, strDir, strLevel, NULL);

	char filter[_MAX_PATH];
	_makepath(filter, NULL, store_path, "*.png", NULL);

	WIN32_FIND_DATA fd; 
	HANDLE hFind = ::FindFirstFile(filter, &fd);
	TRACE0("\n");
	if(hFind != INVALID_HANDLE_VALUE)
	{
		radi::RiakTileStore* store = rfile->GetTileStore();
		char tpath[_MAX_PATH];
		char tkey[_MAX_PATH];

		int level=atoi(strLevel), row=0, col=0;
		do
		{	
			sscanf(fd.cFileName, "%ld_%ld.png", &row, &col);
			sprintf(tkey, "%dx%dx%d", level, row, col);
			GetDlgItem(IDC_STATIC_LOG)->SetWindowText(tkey);

			_makepath(tpath, NULL, store_path, fd.cFileName, NULL);
			if(!store->PutTile(tkey, tpath)) 
			{
				TRACE1("[%s]����ʧ��", tkey);
			}
		}
		while(::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
}


void CGImporterDlg::OnBnClickedOk()
{
	if(m_thread_handler)
	{
		g_exit = true;
		//WaitForSingleObject(m_thread_handler, INFINITE);
	}
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}
