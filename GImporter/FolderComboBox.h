#pragma once


// CFolderComboBox

class CFolderComboBox : public CComboBox
{
	DECLARE_DYNAMIC(CFolderComboBox)

public:
	CFolderComboBox();
	virtual ~CFolderComboBox();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchange();
};


