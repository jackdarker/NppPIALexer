#include "NppPIALexer.h"
#include "NppPIALexerOptions.h"
#include "core/WcharMbcsConverter.h"
#include "core/npp_stuff/resource.h"

// can be _T(x), but _T(x) may be incompatible with ANSI mode
#define _TCH(x)  (x)

extern CNppPIALexer thePlugin;
CNppPIALexerOptions g_opt;

const TCHAR* CNppPIALexer::PLUGIN_NAME = _T("NppPIALexer");
const char* CNppPIALexer::strBrackets[tbtCount - 1] = {
    "()",
    "[]",
    "{}",
    "\"\"",
    "\'\'",
    "<>",
    "</>"
};

char charbuf[MAX_PATH];
TCHAR tcharbuf[MAX_PATH];
std::vector<wchar_t> wcharbuf;

bool CNppPIALexer::isNppMacroStarted = false;
bool CNppPIALexer::isNppWndUnicode = true;
WNDPROC CNppPIALexer::nppOriginalWndProc = NULL;
        
/*
// from "resource.h" (Notepad++)
#define ID_MACRO                       20000
#define ID_MACRO_LIMIT                 20200
#define IDCMD                          50000
#define IDC_EDIT_TOGGLEMACRORECORDING  (IDCMD + 5)
#define MACRO_USER                     (WM_USER + 4000)
#define WM_MACRODLGRUNMACRO            (MACRO_USER + 02)
*/

void CNppPIALexer::DockableDlgDemo()
{
	m_DockDlg.setParent(m_nppMsgr.getNppWnd());
	tTbData	data = {0};

	if (!m_DockDlg.isCreated())
	{
		m_DockDlg.create(&data);
		
		// define the default docking behaviour
		data.uMask = DWS_DF_CONT_RIGHT;

		data.pszModuleName = thePlugin.getDllFileName();//_goToLine.getPluginFileName();

		// the dlgDlg should be the index of funcItem where the current function pointer is
		// in this case is DOCKABLE_DEMO_INDEX
		data.dlgID = CNppPIALexerMenu::N_GOTO;
		::SendMessage(m_nppMsgr.getNppWnd(), NPPM_DMMREGASDCKDLG, 0, (LPARAM)&data);
	}
	m_DockDlg.display();
}

LRESULT CNppPIALexer::nppCallWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return ( isNppWndUnicode ?
               ::CallWindowProcW(nppOriginalWndProc, hWnd, uMsg, wParam, lParam) :
                 ::CallWindowProcA(nppOriginalWndProc, hWnd, uMsg, wParam, lParam) );
}

LRESULT CALLBACK CNppPIALexer::nppNewWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if ( uMsg == WM_COMMAND )
    {
        const WORD id = LOWORD(wParam);
        switch ( id )
        {	
            case IDM_MACRO_STARTRECORDINGMACRO:
                thePlugin.OnNppMacro(MACRO_START);
                break;

            case IDM_MACRO_STOPRECORDINGMACRO:
                thePlugin.OnNppMacro(MACRO_STOP);
                break;

            case IDC_EDIT_TOGGLEMACRORECORDING:
                thePlugin.OnNppMacro(MACRO_TOGGLE);
                break;

            case IDM_MACRO_PLAYBACKRECORDEDMACRO:
                {
                    thePlugin.OnNppMacro(MACRO_START);
                    LRESULT lResult = nppCallWndProc(hWnd, uMsg, wParam, lParam);
                    thePlugin.OnNppMacro(MACRO_STOP);
                    return lResult;
                }
                break;
            
            default:
                if ( (id >= ID_MACRO) && (id < ID_MACRO_LIMIT) )
                {
                    thePlugin.OnNppMacro(MACRO_START);
                    LRESULT lResult = nppCallWndProc(hWnd, uMsg, wParam, lParam);
                    thePlugin.OnNppMacro(MACRO_STOP);
                    return lResult;
                }
                break;
        }
    }
    else if ( uMsg == WM_MACRODLGRUNMACRO )
    {
        thePlugin.OnNppMacro(MACRO_START);
        LRESULT lResult = nppCallWndProc(hWnd, uMsg, wParam, lParam);
        thePlugin.OnNppMacro(MACRO_STOP);
        return lResult;
    }
	
    return nppCallWndProc(hWnd, uMsg, wParam, lParam);
}

CNppPIALexer::CNppPIALexer()
{
    m_nAutoRightBracketPos = -1;
    m_nFileType = tftNone;
    m_bSupportedFileType = true;
	m_Log = new std::wofstream; // On the heap
	m_Log->open( "NppPIALexer.log",std::ios_base::out | std::ios_base::ate,_SH_DENYWR ); //out-stream adding at eof
	m_Model = new Model();
	m_File = new tstr();
	m_Scope = new tstr();
	m_Search = new tstr();
	m_Found = new tstr();
	m_Object= new tstr();
}

CNppPIALexer::~CNppPIALexer()
{
	if(m_Model) delete m_Model;
	if(m_File) delete m_File;
	if(m_Scope) delete m_Scope;
	if(m_Search) delete m_Search;
	if(m_Found) delete m_Found;
	if(m_Object) delete m_Object;
	if (m_Log) { 
		m_Log->close();
		delete m_Log;
		m_Log=NULL;
	}
}
void CNppPIALexer::Log(const TCHAR* log){
	m_DockDlg.PrintLog(log);
	if(!m_Log) return;
	struct tm today = { 1, 1, 1, 1, 1, 1 };
	time_t ltime;
	time( &ltime );
	_localtime64_s( &today, &ltime );  
	TCHAR _Ttime[128];
	wcsftime( _Ttime, 128, _T("%c   "), &today );
	*m_Log<<_Ttime<<log<<std::endl;

}
void CNppPIALexer::Log(const char* log){
	TCHAR _Tlog[2*MAX_PATH + 1]=_T("");
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, log, (int)strlen(log), NULL, 0);
	MultiByteToWideChar(CP_UTF8, 0, log, (int)strlen(log), _Tlog, size_needed);
	Log(_Tlog);
}
FuncItem* CNppPIALexer::nppGetFuncsArray(int* pnbFuncItems)
{
    *pnbFuncItems = CNppPIALexerMenu::N_NBFUNCITEMS;
    return CNppPIALexerMenu::arrFuncItems;
}
const TCHAR* CNppPIALexer::nppGetName()
{
    return PLUGIN_NAME;
}
void CNppPIALexer::nppBeNotified(SCNotification* pscn)
{
    if ( pscn->nmhdr.hwndFrom == m_nppMsgr.getNppWnd() )
    {
        // >>> notifications from Notepad++
        switch ( pscn->nmhdr.code )
        {
            case NPPN_BUFFERACTIVATED:
                OnNppBufferActivated(pscn->nmhdr.idFrom);
                break;
        
            case NPPN_FILEOPENED:
                OnNppFileOpened();
                break;
            
            case NPPN_FILESAVED:
                OnNppFileSaved();
                break;
            
            case NPPN_READY:
                OnNppReady();
                break;

            case NPPN_SHUTDOWN:
                OnNppShutdown();
                break;
            
            default:
                break;
        }
        // <<< notifications from Notepad++
    }
    else
    {
        // >>> notifications from Notepad++
        switch ( pscn->nmhdr.code )
        {
            case SCN_CHARADDED:
                OnSciCharAdded(pscn->ch);
                break;

            default:
                break;
        }
        // <<< notifications from Notepad++
    }
}
void CNppPIALexer::OnNppSetInfo(const NppData& nppd)
{
    m_PluginMenu.setNppData(nppd);
    isNppWndUnicode = ::IsWindowUnicode(nppd._nppHandle) ? true : false;
}
void CNppPIALexer::OnNppBufferActivated(int ID)
{
    UpdateFileType();
	if(ID>-1) {
		m_nppMsgr.getBufferFullPath(ID,MAX_PATH,tcharbuf);
		//??Log(_T("Buffer activated"));
		//Log(tcharbuf);
	}
}
void CNppPIALexer::OnNppFileOpened()
{
    //this handler is not needed because file opening
    //is handled by OnNppBufferActivated()
}
void CNppPIALexer::OnNppFileSaved()
{
    UpdateFileType();
}
void CNppPIALexer::OnNppReady()
{
    ReadOptions();
    CNppPIALexerMenu::UpdateMenuState();
    UpdateFileType();

    if ( isNppWndUnicode )
    {
        nppOriginalWndProc = (WNDPROC) SetWindowLongPtrW( 
          m_nppMsgr.getNppWnd(), GWLP_WNDPROC, (LONG_PTR) nppNewWndProc );
    }
    else
    {
        nppOriginalWndProc = (WNDPROC) SetWindowLongPtrA( 
          m_nppMsgr.getNppWnd(), GWLP_WNDPROC, (LONG_PTR) nppNewWndProc );
    }
		// Initialize dockable demo dialog
	m_DockDlg.init((HINSTANCE) thePlugin.getDllModule(), NULL);
}
void CNppPIALexer::OnNppShutdown()
{
    if ( nppOriginalWndProc )
    {
        if ( isNppWndUnicode )
        {
            ::SetWindowLongPtrW( m_nppMsgr.getNppWnd(), 
                GWLP_WNDPROC, (LONG_PTR) nppOriginalWndProc );
        }
        else
        {
            ::SetWindowLongPtrA( m_nppMsgr.getNppWnd(), 
                GWLP_WNDPROC, (LONG_PTR) nppOriginalWndProc );
        }
    }

    SaveOptions();
}
//Todo brauchen wir das Makrozeug?
void CNppPIALexer::OnNppMacro(int nMacroState)
{
    static int nPrevAutoComplete = -1; // uninitialized
    
    switch ( nMacroState )
    {
        case MACRO_START:
            isNppMacroStarted = true;
            break;
        case MACRO_STOP:
            isNppMacroStarted = false;
            break;
        default:
            isNppMacroStarted = !isNppMacroStarted;
            break;
    }

    if ( isNppMacroStarted )
    {
        nPrevAutoComplete = g_opt.m_bBracketsAutoComplete ? 1 : 0;
        g_opt.m_bBracketsAutoComplete = false;
    }
    else
    {
        if ( nPrevAutoComplete < 0 ) // initialize now
            nPrevAutoComplete = g_opt.m_bBracketsAutoComplete ? 1 : 0;

        g_opt.m_bBracketsAutoComplete = (nPrevAutoComplete > 0);
        UpdateFileType();
    }

    CNppPIALexerMenu::AllowAutocomplete(!isNppMacroStarted);
}

/*bool SciTEBase::StartAutoComplete() {
	SString line = GetLine();
	int current = GetCaretInLine();

	int startword = current;

	while ((startword > 0) &&
	        (calltipWordCharacters.contains(line[startword - 1]) ||
	         autoCompleteStartCharacters.contains(line[startword - 1]))) {
		startword--;
	}

	SString root = line.substr(startword, current - startword);
	if (apis) {
		char *words = GetNearestWords(root.c_str(), root.length(),
			calltipParametersStart.c_str(), autoCompleteIgnoreCase);
		if (words) {
			EliminateDuplicateWords(words);
			wEditor.Call(SCI_AUTOCSETSEPARATOR, ' ');
			wEditor.CallString(SCI_AUTOCSHOW, root.length(), words);
			delete []words;
		}
	}
	return true;
}*/

//Resets state of Autocomplete-Tracker
void CNppPIALexer::ResetAutoComplete() {
	CSciMessager sciMsgr(m_nppMsgr.getCurrentScintillaWnd());
	m_nppMsgr.getCurrentFileFullPath(MAX_PATH,tcharbuf);
	char utf8buf[]={'.','('}; //Todo muss nur einmal konfiguriert werden?
	LRESULT x = sciMsgr.SendSciMsg(SCI_AUTOCSETFILLUPS,(WPARAM) 0,(LPARAM)&utf8buf[0]);
	tstr _File(tcharbuf);
	//Todo relativen Filepath ermitteln; das hier funktioniert nur bei Unterverzeichnissen
	_File.erase(0,g_opt.m_LastProject.length()+1);
	m_File->assign(_File);
	m_Search->assign(_T(""));
	m_Found->assign(_T(""));
	m_Object->assign(_T(""));
	m_CurrPos=-1;
	m_InComment=false;
	m_InString=false;
	m_AfterCmd=false;	
	m_Number=false;	
}
int _wordStart;
void CNppPIALexer::OnSciCharAdded(const int ch)
{
    if ( !g_opt.m_bBracketsAutoComplete )
        return;

    if ( !m_bSupportedFileType )
        return;

    CSciMessager sciMsgr(m_nppMsgr.getCurrentScintillaWnd());

    int nSelections = (int) sciMsgr.SendSciMsg(SCI_GETSELECTIONS);
    if ( nSelections > 1 )
        return; // nothing to do with multiple selections
	LRESULT x;
	switch ( ch )
    {
		case _TCH('.') :
		case _TCH('(') :
			ResetAutoComplete();	
			//x=sciMsgr.SendSciMsg(SCI_AUTOCSETDROPRESTOFWORD,(WPARAM) 1,(LPARAM)0);
			m_CurrPos = sciMsgr.getCurrentPos();
			_wordStart=sciMsgr.getWordStartPos(m_CurrPos-1,true);
			sciMsgr.getTextRange(_wordStart,m_CurrPos-1,charbuf);
			wcharbuf=WcharMbcsConverter::char2wchar(charbuf);
			m_Object->assign(tstr(wcharbuf.begin(),wcharbuf.end()-1));
			//m_nppMsgr.getCurrentWord(MAX_PATH,prevword); doesnt return correct word?
            break;
		case _TCH(' ') :
		case _TCH('\x0D') :
		case _TCH('\x0A') :
		case _TCH('\t') :
            ResetAutoComplete();	
            break;
		default:
			wchar_t y=WcharMbcsConverter::char2wchar((char)ch);
			
			m_Search->append(&y,1); 
			m_Model->GetObject(m_Search,m_File,m_Object,m_Found);
			std::vector<char> utf8buf =WcharMbcsConverter::tchar2char(m_Found->c_str());
			//LRESULT x=sciMsgr.SendSciMsg(SCI_AUTOCSHOW,(WPARAM) length, (LPARAM) strAC);
			if(utf8buf.size()>1){ //ends always with 0x00 
				m_CurrPos = sciMsgr.getCurrentPos();
				_wordStart=sciMsgr.getWordStartPos(m_CurrPos-1,true);
				x=sciMsgr.SendSciMsg(SCI_AUTOCSHOW,(WPARAM)(m_CurrPos-_wordStart),(LPARAM)&utf8buf[0]);
			}
			break;

	}

    int nBracketType = tbtNone;   
    if ( m_nAutoRightBracketPos >= 0 )
    {
        // the right bracket has been just added (automatically)
        // but you may duplicate it manually
        int nRightBracketType = tbtNone;

        // OK for both ANSI and Unicode (ch can be wide character)
        switch ( /*ch*/ 0 )
        {
            case _TCH(')') :
                nRightBracketType = tbtBracket;
                break;
            case _TCH(']') :
                nRightBracketType = tbtSquare;
                break;
            case _TCH('}') :
                nRightBracketType = tbtBrace;
                break;
            case _TCH('\"') :
                nRightBracketType = tbtDblQuote;
                break;
            case _TCH('\'') :
                if ( g_opt.m_bBracketsDoSingleQuote )
                    nRightBracketType = tbtSglQuote;
                break;
            case _TCH('>') :
                if ( g_opt.m_bBracketsDoTag )
                    nRightBracketType = tbtTag; 
                // no break here
            case _TCH('/') :
                if ( g_opt.m_bBracketsDoTag2 )
                    nRightBracketType = tbtTag2;
                break;
        }
    
        if ( nRightBracketType != tbtNone )
        {
            int pos = sciMsgr.getCurrentPos() - 1;
        
            if ( pos == m_nAutoRightBracketPos )
            {
                // previous character
                char prev_ch = sciMsgr.getCharAt(pos - 1);
                if ( prev_ch == strBrackets[nRightBracketType - tbtBracket][0] )
                {
                    char next_ch = sciMsgr.getCharAt(pos + 1);
                    if ( next_ch == strBrackets[nRightBracketType - tbtBracket][1] )
                    {
                        sciMsgr.beginUndoAction();
                        // remove just typed right bracket
                        sciMsgr.setSel(pos, pos + 1);
                        sciMsgr.setSelText("");
                        // move the caret
                        ++pos;
                        if ( nRightBracketType == tbtTag2 )
                            ++pos;
                        sciMsgr.setSel(pos, pos);
                        sciMsgr.endUndoAction();

                        m_nAutoRightBracketPos = -1;
                        return;
                    }
                }
            }
        }
    }

    m_nAutoRightBracketPos = -1;

    // OK for both ANSI and Unicode (ch can be wide character)
    switch ( ch )
    {
        case _TCH('(') :
            nBracketType = tbtBracket;
            break;
        case _TCH('[') :
            nBracketType = tbtSquare;
            break;
        case _TCH('{') :
            nBracketType = tbtBrace;
            break;
        case _TCH('\"') :
            nBracketType = tbtDblQuote;
            break;
        case _TCH('\'') :
            if ( g_opt.m_bBracketsDoSingleQuote )
                nBracketType = tbtSglQuote;
            break;
        case _TCH('<') :
            if ( g_opt.m_bBracketsDoTag )
                nBracketType = tbtTag;
            break;
    }

    if ( nBracketType != tbtNone )
    {
        // a typed character is a bracket
        AutoBracketsFunc(nBracketType);
    }
}

void CNppPIALexer::ReadOptions()
{
    TCHAR szPath[2*MAX_PATH + 1];

    m_nppMsgr.getPluginsConfigDir(2*MAX_PATH, szPath);
    lstrcat(szPath, _T("\\"));
    lstrcat(szPath, m_szIniFileName);

    g_opt.ReadOptions(szPath);
}
void CNppPIALexer::SaveOptions()
{
    if ( g_opt.MustBeSaved() )
    {
        TCHAR szPath[2*MAX_PATH + 1];

        m_nppMsgr.getPluginsConfigDir(2*MAX_PATH, szPath);
        lstrcat(szPath, _T("\\"));
        lstrcat(szPath, m_szIniFileName);
        
        g_opt.SaveOptions(szPath);
    }
}
void CNppPIALexer::ReloadData(const TCHAR*  ProjectPath)
{
	tstr _path(ProjectPath);
	ReloadData(&_path);
}
void CNppPIALexer::ReloadData(const tstr*  ProjectPath)
{
	m_DockDlg.PrintLog(_T("Loading..."));
	m_DockDlg.PrintLog(ProjectPath->c_str());
	int RC = m_Model->LoadIntelisense(ProjectPath);
	if (RC !=0) {
		m_DockDlg.PrintLog(_T("Loading failed"));
		return;
	}
	m_DockDlg.PrintLog(_T("Loading sucessful"));
	g_opt.m_LastProject=ProjectPath->c_str();
	SaveOptions();
	m_Model->RebuildIntelisense(ProjectPath);	
}
void  CNppPIALexer::ExportIntelisense() {
	if(!m_Model) return;
	m_Model->Export();
}
void CNppPIALexer::UpdateFileType() // <-- call it when the plugin becomes active!!!
{
    if ( !g_opt.m_bBracketsAutoComplete )
        return;
    
    m_nAutoRightBracketPos = -1;
    m_nFileType = getFileType(m_bSupportedFileType);
}

static void getEscapedPrefixPos(const int nOffset, int* pnPos, int* pnLen)
{
    if ( nOffset > CNppPIALexer::MAX_ESCAPED_PREFIX )
    {
        *pnPos = nOffset - CNppPIALexer::MAX_ESCAPED_PREFIX;
        *pnLen = CNppPIALexer::MAX_ESCAPED_PREFIX;
    }
    else
    {
        *pnPos = 0;
        *pnLen = nOffset;
    }
}

static bool isEscapedPrefix(const char* str, int len)
{
    int k = 0;
    while ( (len > 0) && (str[--len] == '\\') )
    {
        ++k;
    }
    return (k % 2) ? true : false;
}

void CNppPIALexer::AutoBracketsFunc(int nBracketType)
{
	return; //??Brackets werden von NPP automatisch vervollständigt

    if ( nBracketType == tbtTag )
    {
        if ( g_opt.m_bBracketsDoTagIf && (m_nFileType != tftHtmlCompatible) )
            return;
    }
    
    CSciMessager sciMsgr(m_nppMsgr.getCurrentScintillaWnd());
    int nEditPos = sciMsgr.getSelectionStart();
    int nEditEndPos = sciMsgr.getSelectionEnd();

    // is something selected?
    if ( nEditEndPos != nEditPos )
    {
        if ( sciMsgr.getSelectionMode() == SC_SEL_RECTANGLE )
            return;

        // removing selection
        sciMsgr.setSelText("");
    }

    // Theory: 
    // - The character just pressed is a standard bracket symbol
    // - Thus, this character takes 1 byte in both ANSI and UTF-8
    // - Thus, we check next (and previous) byte in Scintilla to
    //   determine whether the bracket autocompletion is allowed
    // - In general, previous byte can be a trailing byte of one
    //   multi-byte UTF-8 character, but we just ignore this :)
    //   (it is safe because non-leading byte in UTF-8 
    //    is always >= 0x80 whereas the character codes 
    //    of standard Latin symbols are < 0x80)

    bool  bPrevCharOK = true;
    bool  bNextCharOK = false;
    char  next_ch = sciMsgr.getCharAt(nEditPos);

    if ( next_ch == '\x0D' ||
         next_ch == '\x0A' ||
         next_ch == '\x00' ||
         next_ch == ' '  ||
         next_ch == '\t' ||
         next_ch == '.'  ||
         next_ch == ','  ||
         next_ch == '!'  ||
         next_ch == '?'  ||
         next_ch == ':'  ||
         next_ch == ';'  ||
         next_ch == '<' )
    {
        bNextCharOK = true;
    }
    else if (
      ( (next_ch == ')')  &&
        (g_opt.m_bBracketsRightExistsOK || (nBracketType != tbtBracket)) )  ||
      ( (next_ch == ']')  &&
        (g_opt.m_bBracketsRightExistsOK || (nBracketType != tbtSquare)) )   ||
      ( (next_ch == '}')  &&
        (g_opt.m_bBracketsRightExistsOK || (nBracketType != tbtBrace)) )    ||
      ( (next_ch == '\"') &&
        (g_opt.m_bBracketsRightExistsOK || (nBracketType != tbtDblQuote)) ) ||
      ( (next_ch == '\'') &&
        ((!g_opt.m_bBracketsDoSingleQuote) || g_opt.m_bBracketsRightExistsOK ||
         (nBracketType != tbtSglQuote)) ) ||
      ( (next_ch == '>' || next_ch == '/') &&
        ((!g_opt.m_bBracketsDoTag) || g_opt.m_bBracketsRightExistsOK ||
         (nBracketType != tbtTag)) ) )
    {
        bNextCharOK = true;
    }

    if ( bNextCharOK && (nBracketType == tbtDblQuote || nBracketType == tbtSglQuote) )
    {
        bPrevCharOK = false;

        // previous character
        char prev_ch = (nEditPos >= 2) ? sciMsgr.getCharAt(nEditPos - 2) : 0;

        if ( prev_ch == '\x0D' ||
             prev_ch == '\x0A' ||
             prev_ch == '\x00' ||
             prev_ch == ' '  ||
             prev_ch == '\t' ||
             prev_ch == '('  ||
             prev_ch == '['  ||
             prev_ch == '{'  ||
             prev_ch == '<'  ||
             prev_ch == '=' )
        {
            bPrevCharOK = true;
        }
    }

    if ( bPrevCharOK && bNextCharOK && g_opt.m_bBracketsSkipEscaped )
    {
        char szPrefix[MAX_ESCAPED_PREFIX + 2];
        int  pos, len;

        getEscapedPrefixPos(nEditPos - 1, &pos, &len);
        len = sciMsgr.getTextRange(pos, pos + len, szPrefix);
        if ( isEscapedPrefix(szPrefix, len) )
            bPrevCharOK = false;
    }

    if ( bPrevCharOK && bNextCharOK )
    {
        if ( nBracketType == tbtTag )
        {
            if ( g_opt.m_bBracketsDoTag2 )
                nBracketType = tbtTag2;
        }

        sciMsgr.beginUndoAction();
        // selection
        sciMsgr.setSel(nEditPos - 1, nEditPos);
        // inserting brackets
        sciMsgr.setSelText(strBrackets[nBracketType - tbtBracket]);
        // placing the caret between brackets
        sciMsgr.setSel(nEditPos, nEditPos);
        sciMsgr.endUndoAction();

        m_nAutoRightBracketPos = nEditPos;
    }

}

int CNppPIALexer::getFileType(bool& isSupported)
{
    TCHAR szExt[CNppPIALexerOptions::MAX_EXT];
    int   nType = tftNone;

    isSupported = true;
    szExt[0] = 0;
    m_nppMsgr.getCurrentFileExtPart(CNppPIALexerOptions::MAX_EXT - 1, szExt);

    if ( szExt[0] )
    {
        TCHAR* pszExt = szExt;
        
        if ( *pszExt == _T('.') )
        {
            ++pszExt;
            if ( !(*pszExt) )
                return nType;
        }

        ::CharLower(pszExt);

        if ( lstrcmp(pszExt, _T("c")) == 0 ||
             lstrcmp(pszExt, _T("cc")) == 0 ||
             lstrcmp(pszExt, _T("cpp")) == 0 ||
             lstrcmp(pszExt, _T("cxx")) == 0 )
        {
            nType = tftC_Cpp;
        }
        else if ( lstrcmp(pszExt, _T("h")) == 0 ||
                  lstrcmp(pszExt, _T("hh")) == 0 ||
                  lstrcmp(pszExt, _T("hpp")) == 0 ||
                  lstrcmp(pszExt, _T("hxx")) == 0 )
        {
            nType = tftH_Hpp;
        }
        else if ( lstrcmp(pszExt, _T("pas")) == 0 )
        {
            nType = tftPas;
        }
        else if ( g_opt.IsHtmlCompatible(pszExt) )
        {
            nType = tftHtmlCompatible;
        }
        else
        {
            nType = tftText;
        }

        isSupported = g_opt.IsSupportedFile(pszExt);
    }

    return nType;
}
