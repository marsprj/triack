// FolderComboBox.cpp : implementation file
//

#include "stdafx.h"
#include "GImporter.h"
#include "FolderComboBox.h"


// CFolderComboBox

IMPLEMENT_DYNAMIC(CFolderComboBox, CComboBox)

CFolderComboBox::CFolderComboBox()
{

}

CFolderComboBox::~CFolderComboBox()
{
}


BEGIN_MESSAGE_MAP(CFolderComboBox, CComboBox)
	ON_CONTROL_REFLECT(CBN_SELCHANGE, &CFolderComboBox::OnCbnSelchange)
END_MESSAGE_MAP()



// CFolderComboBox message handlers




void CFolderComboBox::OnCbnSelchange()
{
	// TODO: Add your control notification handler code here
	AfxMessageBox("sss");
}
