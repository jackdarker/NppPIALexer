//this file is part of notepad++
//Copyright (C)2003 Don HO ( donho@altern.org )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef GOTILINE_DLG_H
#define GOTILINE_DLG_H

#include "core\NppPluginMenu.h"
#include "DockingDlgInterface.h"
#include "resource.h"


class DemoDlg : public DockingDlgInterface,CNppPluginMenu
{
public :
	DemoDlg() : DockingDlgInterface(IDD_PLUGINGOLINE_DEMO){
		m_Log.clear();
		
	};

    virtual void display(bool toShow = true) const {
        DockingDlgInterface::display(toShow);
        if (toShow) {
            ::SetFocus(::GetDlgItem(_hSelf, ID_GOLINE_EDITDEMO));
			
		}
    };

	void setParent(HWND parent2set){
		_hParent = parent2set;
	};
	void PrintLog(const TCHAR* pszText) {
		if(m_Log.length()>1000) m_Log.clear();
		m_Log.append(pszText);
		m_Log.append(_T("\r\n"));
		//Todo cutoff log
		this->DlgItem_SetText(_hSelf,ID_GOLINE_MSG,  m_Log.c_str());
	};

	/*CNppPIALexer* GetLexer() {
		return m_Plugin; };*/
protected :
	virtual BOOL CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
	//??static CNppMessager m_nppMsgr;
	
	BOOL DlgItem_SetText(HWND hDlg, UINT idDlgItem, const TCHAR* pszText);
	BOOL DlgItem_GetText(HWND hDlg, UINT idDlgItem, TCHAR* pszText);
private :
	std::wstring m_Log ;
	int getLine() const {
        BOOL isSuccessful;
        int line = ::GetDlgItemInt(_hSelf, ID_GOLINE_EDITDEMO, &isSuccessful, FALSE);
        return (isSuccessful?line:-1);
    };

};

#endif //GOTILINE_DLG_H
