#ifndef _NppPIALexer_model_h_
#define _NppPIALexer_model_h_
//---------------------------------------------------------------------------

#include "core/base.h"
#include "core/sqlite3.h"

//Datenmodel f�r Intelisense

class Model
{
	enum TClassType {
            tCTClass = 0,
			tCTSeq =1,
			tCTType=2
    };

	class Obj {
	public:
		Obj(str scope,str name, str classID) {
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
		str ClassID(){return m_ClassID;}
	private:
		int m_ID;
		str m_Scope;
		str m_Name;
		str m_ClassID;
	};
	class ObjDecl {
	public:
		ObjDecl(str classID, TClassType type,str function,str params,str returns ) {
			m_ID=0;
			m_ClassID=classID;
			m_ClassType=type;
			m_Params=params;
			m_Returns=returns;
			m_Function=function;
		};
		~ObjDecl(){};
		int ID() {return m_ID;}
		void updateID(int iD) {m_ID=iD;}
		str Function(){return m_Function;}
		str Params(){return m_Params;}
		str Returns(){return m_Returns;}
		str ClassID(){return m_ClassID;}
		TClassType ClassType() {return m_ClassType;}
	private:
		int m_ID;
		TClassType m_ClassType;
		str m_Function;
		str m_Params;
		str m_Returns;
		str m_ClassID;
	};

    protected:
		sqlite3 *db;
		int LastError;

    public:
        Model();
        ~Model();

		//Loads Data from Database; if there is none, rebuildObjList is called
		int LoadIntelisense(const tstr*  ProjectPath) ;
		//new database is created and filled with parsed data from source, also rebuild ObjList
		

		//if there is no Database new one is created
		//parses all SEQ for Obj-definitions and in-sequence-functions of the current working directory
		//calls RebuildIntelisense 
		int RebuildObjList() {return 0;};

		//search for data for Obj-definitions in Objects and function-declaration in seq
		int RebuildIntelisense(const tstr*  ProjectPath) {return 0;};

		//setup a projectdatabase for intelisense; if there is one it will be overwritten
		int InitDatabase() ;

		//gets a list of possible autocompletion candidates for Objects/in-sequence-functions
		// Scope is the actual SEQ
		int GetObject(const tstr* BeginsWith, const tstr* Scope,const tstr* Object, tstr* Result );
		//gets a list of possible autocompletion candidates for object-functions
		// Scope is the actual Objectname
		int GetFunction(const tstr* BeginsWith, const tstr* Scope, const tstr*  Object ,tstr* Result );
	private: 
		int UpdateObjList(Obj& theObj );
		int RefreshObjListID(Obj& theObj);
		int UpdateObjDecl(ObjDecl& theObj );
		int RefreshObjDeclID(ObjDecl& theObj);
		void HandleDBError() ;

};

//----------------------------------------------------------------------------
#endif