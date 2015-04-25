
// GImporterDlg.h : 头文件
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


// CGImporterDlg 对话框
class CGImporterDlg : public CDialogEx
{
// 构造
public:
	CGImporterDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_GIMPORTER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
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
};
