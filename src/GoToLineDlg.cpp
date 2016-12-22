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

#include "NppPIALexer.h"
#include "NppPIALexerOptions.h"
#include "GoToLineDlg.h"

extern CNppPIALexer        thePlugin;
extern CNppPIALexerOptions g_opt;
//extern NppData nppData;

BOOL DemoDlg::DlgItem_SetText(HWND hDlg, UINT idDlgItem, const TCHAR* pszText)
{
    HWND hDlgItem = GetDlgItem(hDlg, idDlgItem);
    if ( hDlgItem )
    {
        return SetWindowText(hDlgItem, pszText);
    }
    return FALSE;
}
BOOL DemoDlg::DlgItem_GetText(HWND hDlg, UINT idDlgItem, TCHAR* pszText){
	HWND hDlgItem = GetDlgItem(hDlg, idDlgItem);
    if ( hDlgItem )
    {
		int _s=MAX_PATH;//sizeof(pszText);
        return GetWindowText(hDlgItem, pszText,_s );
    }
	return false;
}
BOOL CALLBACK DemoDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
		case WM_COMMAND : 
		{
			switch (wParam)
			{
				case IDOK :
				{
					int line = getLine();
					if (line != -1)
					{
						// Get the current scintilla
						int which = -1;
						::SendMessage(CNppPluginMenu::m_nppMsgr.getNppWnd(), NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
						if (which == -1)
							return FALSE;
						HWND curScintilla = CNppPluginMenu::m_nppMsgr.getCurrentScintillaWnd();/*(which == 0)? nppData._scintillaMainHandle:nppData._scintillaSecondHandle;*/

						::SendMessage(curScintilla, SCI_ENSUREVISIBLE, line-1, 0);
						::SendMessage(curScintilla, SCI_GOTOLINE, line-1, 0);
					}
					return TRUE;
				}
				case ID_GOLINE_RELOAD :
				{
					TCHAR _Dir[MAX_PATH];
					DlgItem_GetText(_hSelf,ID_GOLINE_BASEDIR,_Dir);
					thePlugin.ReloadData(_Dir);
					//this->DlgItem_SetText(_hSelf,ID_GOLINE_MSG,  thePlugin.getDllFileName());
					return TRUE;
				}
				case ID_GOLINE_EXPORT :
				{
					thePlugin.ExportIntelisense();
					return TRUE;
				}
			}
				return FALSE;
		}
		case WM_INITDIALOG:
		{
			DlgItem_SetText(_hSelf,ID_GOLINE_BASEDIR,  g_opt.m_LastProject.c_str());
		}

		default :
			return DockingDlgInterface::run_dlgProc(message, wParam, lParam);
	}
}

