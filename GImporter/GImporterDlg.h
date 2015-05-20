
// GImporterDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"

#include "RiakFS.h"
#include "RiakTile.h"
#include "RiakFile.h"
#include "RiakFileSet.h"
#include "RiakTileStore.h"
#include "RiakTileStoreSet.h"

#include "FolderComboBox.h"


// CGImporterDlg �Ի���
class CGImporterDlg : public CDialogEx
{
// ����
public:
	CGImporterDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_GIMPORTER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnUpdateLog(WPARAM wParam,LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnFolder();
	afx_msg void OnBnClickedBtnImport();
	CComboBox m_cmb_riak_folder;
	CComboBox m_cmb_riak_store;
	CComboBox m_cmb_tile_level;

private:
	void	UpdateRiakStoreCombo(CString strFolder);
	radi::RiakFS* m_riak;
public:
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedBtnRiak();
	afx_msg void OnCbnSelchangeCmbRiakFolder();
	afx_msg void OnBnClickedOk();

	

private:
	void	ImportLevel(CString strDir, CString strLevel);
public:
	CString m_riak_server;
	CString m_riak_port;
	CString m_riak_folder;
	CString m_riak_store;
	CString m_tile_folder;
	CString m_tile_store;
	CStatic m_static_log;

	HANDLE	m_thread_handler;
};
