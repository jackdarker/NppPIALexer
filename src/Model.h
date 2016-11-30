#ifndef _NppPIALexer_model_h_
#define _NppPIALexer_model_h_
//---------------------------------------------------------------------------

#include "core/base.h"
#include "core/sqlite3.h"

//Datenmodel für Intelisense

class Model
{
    protected:
		sqlite3 *db;
		int LastError;

    public:
        Model();
        ~Model();

		//Loads Data from Database; if there is none, rebuildObjList is called
		int LoadIntelisense(const TCHAR*  ProjectPath) ;
		//new database is created and filled with parsed data from source, also rebuild ObjList
		

		//if there is no Database new one is created
		//parses all SEQ for Obj-definitions and in-sequence-functions of the current working directory
		//calls RebuildIntelisense 
		int RebuildObjList() {return 0;};

		//search for data for Obj-definitions in Objects and function-declaration in seq
		int RebuildIntelisense(const TCHAR*  ProjectPath) {return 0;};

		//setup a projectdatabase for intelisense; if there is one it will be overwritten
		int InitDatabase() ;

		//gets a list of possible autocompletion candidates for Objects/in-sequence-functions
		// Scope is the actual SEQ
		int GetObject(const TCHAR* BeginsWith, const TCHAR* Scope, const TCHAR* Result ) {return 0;};
		//gets a list of possible autocompletion candidates for object-functions
		// Scope is the actual Objectname
		int GetFunction(const TCHAR* BeginsWith, const TCHAR* Scope, const TCHAR*  Object ,const TCHAR* Result ){ return 0;};
	private: 
		void HandleDBError() ;

};

//----------------------------------------------------------------------------
#endif