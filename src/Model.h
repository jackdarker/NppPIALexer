#ifndef _NppPIALexer_model_h_
#define _NppPIALexer_model_h_
//---------------------------------------------------------------------------

#include "core/base.h"
#include "core/sqlite3.h"

//Datenmodel für Intelisense

class Model
{
public:
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
		static const int AC_LIST_LENGTH_MAX =10; //limits the maximum number of entrys in Autocompletion-list
		static const std::string& DIR_SEQUENCES(){ //<- this is the SEQ-directory relative to project
			//shitty workaround because static const string variables not possible ?!
			static std::string foo("\\PRG\\SEQ");
			return foo;
		}
		static const tstr& FILE_SEQ(){	//<- this is the extension of Sequences
			static tstr foo(_T(".seq"));
			return foo;
		}
		static const tstr& FILE_DB(){	//<- this is the name of the Project-Database relative to project
			static tstr foo(_T("\\Intelisense.db"));
			return foo;
		}
		static const std::vector<str>& BASIC_TYPES() { //list all "basic" types; dont forget the space
			static std::vector<str> foo;
			if (foo.empty()) {
				foo.push_back("bool ");
				foo.push_back("double ");
				foo.push_back("int ");
				foo.push_back("string ");
				foo.push_back("variant ");
			}
			return foo;
		}
		Model();
        ~Model();

		//Loads Data from Database; if there is none, rebuildObjList is called
		int LoadIntelisense(const tstr*  ProjectPath) ;
		//new database is created and filled with parsed data from source, also rebuild ObjList
		

		//if there is no Database new one is created
		//parses all SEQ for Obj-definitions and in-sequence-functions of the current working directory
		//calls RebuildClassDefinition 
		int RebuildObjList(const tstr*  ProjectPath);

		//search for data of classes (declaration of functions)
		int RebuildClassDefinition(const tstr*  ProjectPath);

		//setup a projectdatabase for intelisense; if there is one it will be overwritten
		int InitDatabase() ;

		//gets a list of possible autocompletion candidates for Objects/in-sequence-functions
		// Scope is the actual SEQ
		int GetObject(const tstr* BeginsWith, const tstr* Scope,const tstr* Object, tstr* Result );
		//gets a list of possible autocompletion candidates for object-functions
		// Scope is the actual Objectname
		int GetFunction(const tstr* BeginsWith, const tstr* Scope, const tstr*  Object ,tstr* Result );

		int Export();
		int UpdateObjList(Obj& theObj );
		int UpdateObjDecl(ObjDecl& theObj );
	private: 
		//Todo ?? removes entrys that dont exist anymore
		// -> setzt checkedMarker=false in jedem eintrag
		// -> RebuildIntelisense wird aufgerufen, was checkedMarker=true setzt für jeden Eintrag der bestätigt wird
		// -> CleanupDeadLink löscht alle wo immer noch checkedMarker==false
		int CleanupDeadLinks() {return 0;};
		int RefreshObjListID(Obj& theObj);
		int RefreshObjDeclID(ObjDecl& theObj);
		void HandleDBError() ;

};

//----------------------------------------------------------------------------
#endif