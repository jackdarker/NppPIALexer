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

		//Loads Data from Database; if there is none, rebuild is called
		int LoadIntelisense(char * ProjectPath) {return 0;};
		//new database is created and filled with parsed data from source, also rebuild ObjList
		int RebuildIntelisense(char * ProjectPath) {return 0;};

		//parses all SEQ for Obj-definitions and in-sequence-functions
		int RebuildObjList() {return 0;};

		//gets a list of possible autocompletion candidates for Objects/in-sequence-functions
		// Scope is the actual SEQ
		int GetObject(char *BeginsWith, char *Scope, char *Result ) {return 0;};
		//gets a list of possible autocompletion candidates for object-functions
		// Scope is the actual Objectname
		int GetFunction(char *BeginsWith, char *Scope, char * Object ,char *Result ){ return 0;};
};

//----------------------------------------------------------------------------
#endif