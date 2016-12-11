#ifndef _NppPIALexer_model_h_
#define _NppPIALexer_model_h_
//---------------------------------------------------------------------------

#include "core/base.h"
#include "core/sqlite3.h"

//Datenmodel für Intelisense

class Model
{
	class Obj {
	public:
		Obj(str scope,str name, int classID) {
			m_ID=0;
			m_Scope=scope;
			m_Name=name;
			m_ClassID=classID;
		};
		~Obj(){};
		int ID() {return m_ID;}
		void updateID(int iD) {m_ID=iD;}
		str Name(){return m_Name;}
		str Scope(){return m_Scope;}
		int ClassID(){return m_ClassID;}
	private:
		int m_ID;
		str m_Scope;
		str m_Name;
		int m_ClassID;
	};
	class ObjDecl {
		int m_ID;
		str m_Function;
		str m_Params;
		str m_Returns;
		int m_ClassID;
	};

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
		int GetObject(const tstr* BeginsWith, const tstr* Scope, tstr* Result );
		//gets a list of possible autocompletion candidates for object-functions
		// Scope is the actual Objectname
		int GetFunction(const TCHAR* BeginsWith, const TCHAR* Scope, const TCHAR*  Object ,const TCHAR* Result ){ return 0;};
	private: 
		int UpdateObjList(Obj& theObj );
		int RefreshObjListID(Obj& theObj);
		void HandleDBError() ;

};

//----------------------------------------------------------------------------
#endif