#include "NppPIALexerMenu.h"
#include "NppPIALexer.h"
#include "NppPIALexerOptions.h"
#include "SettingsDlg.h"
#include "resource.h"


extern CNppPIALexer        thePlugin;
extern CNppPIALexerOptions g_opt;


FuncItem CNppPIALexerMenu::arrFuncItems[N_NBFUNCITEMS] = {
    { _T("Autocomplete brackets"), funcAutocomplete, 0, false, NULL },
    { _T("Settings..."),           funcSettings,     0, false, NULL },
    { _T(""),                      NULL,             0, false, NULL }, // separator
    { _T("About"),                 funcAbout,        0, false, NULL }
};

void CNppPIALexerMenu::funcAutocomplete()
{
    g_opt.m_bBracketsAutoComplete = !g_opt.m_bBracketsAutoComplete;
    UpdateMenuState();
}

void CNppPIALexerMenu::funcSettings()
{
    if ( PluginDialogBox(IDD_SETTINGS, SettingsDlgProc) == 1 )
    {
        UpdateMenuState();
        thePlugin.SaveOptions();
    }
}

void CNppPIALexerMenu::funcAbout()
{
    ::MessageBox( 
        m_nppMsgr.getNppWnd(),
        _T("NppPIALexer \r\n") \
        _T("(C) JK 2016\r\n") ,
        _T("NppPIALexer plugin for Notepad++"),
        MB_OK
      );
}

INT_PTR CNppPIALexerMenu::PluginDialogBox(WORD idDlg, DLGPROC lpDlgFunc)
{
    // static function uses static (global) variable 'thePlugin'
    return ::DialogBox( (HINSTANCE) thePlugin.getDllModule(),
               MAKEINTRESOURCE(idDlg), m_nppMsgr.getNppWnd(), lpDlgFunc );
}

void CNppPIALexerMenu::UpdateMenuState()
{
    HMENU hMenu = ::GetMenu( m_nppMsgr.getNppWnd() );
    ::CheckMenuItem(hMenu, arrFuncItems[N_AUTOCOMPLETE]._cmdID,
        MF_BYCOMMAND | (g_opt.m_bBracketsAutoComplete ? MF_CHECKED : MF_UNCHECKED));

    if ( g_opt.m_bBracketsAutoComplete )
    {
        thePlugin.OnNppBufferActivated();
    }
}

void CNppPIALexerMenu::AllowAutocomplete(bool bAllow)
{
    HMENU hMenu = ::GetMenu( m_nppMsgr.getNppWnd() );
    ::EnableMenuItem(hMenu, arrFuncItems[N_AUTOCOMPLETE]._cmdID,
        MF_BYCOMMAND | (bAllow ? MF_ENABLED : MF_GRAYED));
    ::EnableMenuItem(hMenu, arrFuncItems[N_SETTINGS]._cmdID,
        MF_BYCOMMAND | (bAllow ? MF_ENABLED : MF_GRAYED));
}
