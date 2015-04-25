
// GImporterDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "GImporter.h"
#include "GImporterDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CGImporterDlg 对话框


CGImporterDlg::CGImporterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGImporterDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_riak = NULL;
}

void CGImporterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CMB_RIAK_FOLDER, m_cmb_riak_folder);
	DDX_Control(pDX, IDC_CMB_RIAK_STORE, m_cmb_riak_store);
	DDX_Control(pDX, IDC_CMB_TILE_LEVEL, m_cmb_tile_level);
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
END_MESSAGE_MAP()


// CGImporterDlg 消息处理程序

BOOL CGImporterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	//GetDlgItem(IDC_RIAK_SERVER)->SetWindowText("192.168.111.151");
	//GetDlgItem(IDC_RIAK_SERVER)->SetWindowText("123.57.207.198");
	GetDlgItem(IDC_RIAK_SERVER)->SetWindowText("192.168.111.104");
	GetDlgItem(IDC_RIAK_PORT)->SetWindowText("8087");

	//m_cmb_riak_folder.AddString("google");
	//m_cmb_riak_folder.SetCurSel(0);
	//m_cmb_riak_store.AddString("level_1");
	//m_cmb_riak_store.SetCurSel(0);

	GetDlgItem(IDC_TILE_FOLDER)->SetWindowText("E:\\Project\\Project\\GA\\Data\\world_png\\google");
	GetDlgItem(IDC_TILE_FOLDER)->SetWindowText("E:\\Project\\Project\\GA\\Data\\world_png\\google");

	GetDlgItem(IDC_STATIC_LOG)->SetWindowText("");

	CString str;
	for(int i=1;i<20; i++)
	{
		str.Format("%d", i);
		m_cmb_tile_level.AddString(str);
	}
	m_cmb_tile_level.SetCurSel(0);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CGImporterDlg::OnPaint()
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

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
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

void CGImporterDlg::OnBnClickedBtnRiak()
{
	if(m_riak!=NULL)
	{
		m_riak->Close();
		m_riak->Release();
		m_riak = NULL;

		GetDlgItem(IDC_BTN_RIAK)->SetWindowText("连接");
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
			AfxMessageBox("无法连接到riak");

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

			GetDlgItem(IDC_BTN_RIAK)->SetWindowText("断开");

			AfxMessageBox("连接成功");
		}
	}
}


void CGImporterDlg::OnCbnSelchangeCmbRiakFolder()
{
	// TODO: Add your control notification handler code here
	if(!m_riak)
	{
		return;
	}

	CString strFolder;
	//m_cmb_riak_folder.GetWindowText(strFolder);
	int item = m_cmb_riak_folder.GetCurSel();//(strFolder);
	if(item<0)
	{
		return;
	}
	m_cmb_riak_folder.GetLBText(item, strFolder);
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
	// TODO: Add your control notification handler code here
	TCHAR pszPath[MAX_PATH];
	BROWSEINFO bi; 
	bi.hwndOwner      = this->GetSafeHwnd();
	bi.pidlRoot       = NULL;
	bi.pszDisplayName = NULL; 
	bi.lpszTitle      = TEXT("请选择文件夹"); 
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
	if(m_riak==NULL)
	{
		AfxMessageBox("尚未连接Riak");
		return;
	}

	CString strDir,strLevel,strRiakFolder, strStoreName;
	GetDlgItem(IDC_TILE_FOLDER)->GetWindowText(strDir);
	GetDlgItem(IDC_CMB_TILE_LEVEL)->GetWindowText(strLevel);
	GetDlgItem(IDC_CMB_RIAK_FOLDER)->GetWindowText(strRiakFolder);
	GetDlgItem(IDC_CMB_RIAK_STORE)->GetWindowText(strStoreName);

	char store_path[_MAX_PATH];
	_makepath(store_path, NULL, strDir, strLevel, NULL);

	char filter[_MAX_PATH];
	_makepath(filter, NULL, store_path, "*.png", NULL);

	WIN32_FIND_DATA fd; 
	HANDLE hFind = ::FindFirstFile(filter, &fd);
	TRACE0("\n");
	if(hFind != INVALID_HANDLE_VALUE)
	{
		radi::RiakFile* root = m_riak->GetRoot();
		radi::RiakFile* folder = root->GetRiakFile(strRiakFolder);
		if(folder==NULL)
		{
			folder = root->CreateFolder(strRiakFolder);
		}
		if(folder==NULL)
		{
			AfxMessageBox("无法连接到创建folder文件夹");
			return;
		}
		radi::RiakFile* rfile = folder->GetRiakFile(strStoreName);
		if(rfile==NULL)
		{
			rfile = folder->CreateRiakFile(strStoreName, 1,18,-180,-90,180,90, RADI_DATA_TYPE_GOOGLE_CRS84_QUAD);
		}
		if(rfile==NULL)
		{
			AfxMessageBox("无法创建store");
			folder->Release();
			return;
		}

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
				TRACE1("[%s]导入失败", tkey);
			}
		}
		while(::FindNextFile(hFind, &fd));
		::FindClose(hFind);
		
		AfxMessageBox("导入完毕");
	}
	else
	{
		AfxMessageBox("文件路径不存在");
	}	
}


void CGImporterDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}
