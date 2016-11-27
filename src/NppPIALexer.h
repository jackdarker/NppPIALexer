#ifndef _NppPIALexer_npp_plugin_h_
#define _NppPIALexer_npp_plugin_h_
//---------------------------------------------------------------------------
#include "core/NppPlugin.h"
#include "NppPIALexerMenu.h"
#include "Model.h"
#include "GoToLineDlg.h"

class CNppPIALexer : public CNppPlugin
{
    public:
        enum TFileType {
            tftNone = 0,
            tftText,
            tftC_Cpp,
            tftH_Hpp,
            tftPas,
            tftHtmlCompatible
        };
        
        enum TBracketType {
            tbtNone = 0,
            tbtBracket,  //  (
            tbtSquare,   //  [
            tbtBrace,    //  {
            tbtDblQuote, //  "
            tbtSglQuote, //  '
            tbtTag,      //  <
            tbtTag2,

            tbtCount
        };

        enum eConsts {
            MAX_ESCAPED_PREFIX  = 20
        };

        static const TCHAR* PLUGIN_NAME;
        static const char*  strBrackets[tbtCount - 1];
    protected:
        // plugin menu
        CNppPIALexerMenu m_PluginMenu;
        DemoDlg m_DockDlg;
        // internal vars
        int  m_nAutoRightBracketPos;
        int  m_nFileType;
        bool m_bSupportedFileType;
		Model *m_Model;

    public:
        CNppPIALexer();
        virtual ~CNppPIALexer();

        // standard n++ plugin functions
        virtual void         nppBeNotified(SCNotification* pscn);
        virtual FuncItem*    nppGetFuncsArray(int* pnbFuncItems);
        virtual const TCHAR* nppGetName();

        // common n++ notification
        virtual void OnNppSetInfo(const NppData& nppd);

        // custom n++ notifications
        void OnNppBufferActivated();
        void OnNppFileOpened();
        void OnNppFileSaved();
        void OnNppReady();
        void OnNppShutdown();
        void OnNppMacro(int nMacroState);
		void DockableDlgDemo();

        // custom scintilla notifications
        void OnSciCharAdded(const int ch);

        // custom functions
        void ReadOptions();
        void SaveOptions();

		void ReloadData(const TCHAR*  ProjectPath);

    protected:
        // custom functions
        void AutoBracketsFunc(int nBracketType);
        void UpdateFileType();
        int  getFileType(bool& isSupported);

    protected:
        enum eMacroState {
            MACRO_TOGGLE = -1, // invert
            MACRO_STOP   = 0,  // false
            MACRO_START  = 1   // true
        };
        
        static bool    isNppMacroStarted;
        static bool    isNppWndUnicode;
        static WNDPROC nppOriginalWndProc;
        static LRESULT nppCallWndProc(HWND, UINT, WPARAM, LPARAM);
        static LRESULT CALLBACK nppNewWndProc(HWND, UINT, WPARAM, LPARAM);
};

//---------------------------------------------------------------------------
#endif
