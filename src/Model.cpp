
#include "Model.h"
#include "NppPIALexer.h"
#include "NppPIALexerOptions.h"
#include "core/WcharMbcsConverter.h"

extern CNppPIALexer thePlugin;
//??Todo using sqlite with UTF8, should switch to UTF16 since UNICODE is declared? 

Model::Model(){
	thePlugin.Log(_T("model constructor"));
}

Model::~Model(){
	if(db) sqlite3_close(db);
	thePlugin.Log(_T("model destructor"));
}
void Model::HandleDBError() {
	thePlugin.Log(_T("Error SQLite3 database"));
	thePlugin.Log(sqlite3_errmsg(db));
    sqlite3_close(db);
	db = NULL;
}
/*int Model::GetObjectCB(void *data, int argc, char **argv, char **azColName){
   int i;
   fprintf(stderr, "%s: ", (const char*)data);
   for(i=0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}*/
int Model::GetObject(const tstr* BeginsWith, const tstr* Scope, const tstr* Object,tstr* Result ) {
	char *error=0;
	if (Object->empty()) {
		tstr _SQL(_T("SELECT Object from ObjectList where Scope=='"));
		_SQL+=(*Scope)+_T("' AND Object Like('")+(*BeginsWith)+_T("%');");
		sqlite3_stmt *res;
		char _sql[MAX_PATH];
		WideCharToMultiByte(CP_ACP, 0, _SQL.c_str(),_SQL.size()+1, _sql , MAX_PATH, NULL, NULL);
		int rc = sqlite3_prepare(db,_sql, -1, &res, 0);       
		if (rc != SQLITE_OK) {      
			thePlugin.Log(_sql);
			thePlugin.Log(_T("Failed to fetch data")); 
			thePlugin.Log(sqlite3_errmsg(db));
			return 1;
		}    
		str _result("");
		rc = sqlite3_step(res);  
		while (rc == SQLITE_ROW) {
			_result.append((const char*)sqlite3_column_text(res, 0)).append(" ");
			rc = sqlite3_step(res);
		}
		sqlite3_finalize(res);
		std::vector<wchar_t> buf=WcharMbcsConverter::char2wchar(_result.c_str());
		Result->assign(tstr(buf.begin(),buf.end()));
		return 0;
	} else {
		return GetFunction(BeginsWith,Scope,Object,Result);
	}	
}
int Model::GetFunction(const tstr* BeginsWith, const tstr* Scope, const tstr* Object,tstr* Result ) {
	char *error=0;

	tstr _SQL(_T("SELECT Function from ObjectList inner join ObjectDecl on ObjectList.ClassID==ObjectDecl.ClassID where Scope=='"));
	_SQL+=(*Scope)+_T("' AND Object=='")+(*Object)+_T("' order by Function;");
	sqlite3_stmt *res;
	char _sql[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, _SQL.c_str(),_SQL.size()+1, _sql , MAX_PATH, NULL, NULL);
	int rc = sqlite3_prepare(db,_sql, -1, &res, 0);       
    if (rc != SQLITE_OK) {      
		thePlugin.Log(_sql);
		thePlugin.Log(_T("Failed to fetch data")); 
		thePlugin.Log(sqlite3_errmsg(db));
        return 1;
    }    
    str _result("");
    rc = sqlite3_step(res);  
    while (rc == SQLITE_ROW) {
		_result.append((const char*)sqlite3_column_text(res, 0)).append(" ");
		rc = sqlite3_step(res);
    }
    sqlite3_finalize(res);
	std::vector<wchar_t> buf=WcharMbcsConverter::char2wchar(_result.c_str());
	Result->assign(tstr(buf.begin(),buf.end()));
	return 0;
}

int Model::LoadIntelisense(const tstr*  ProjectPath) {
	thePlugin.Log(_T("Opening db ..." ));
	tstr _FullPath;
	_FullPath.assign(ProjectPath->c_str());
	_FullPath.append(tstr(_T("\\Intelisense.db")));	//<- this is the name of the Project-Database
	char Dest[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, _FullPath.c_str(), wcslen(_FullPath.c_str())+1, Dest , MAX_PATH, NULL, NULL);

   LastError = sqlite3_open(Dest, &db);  
   if (LastError )
   {
      HandleDBError();
      return 1;
   }
   LastError = InitDatabase();
   if (LastError ) {
	   return 1;
   }
   
   return 0;
}
		
int Model::InitDatabase() {
	char *error=0;
	const char *sqlDropTable = "DROP TABLE ObjectList";
	LastError = sqlite3_exec(db, sqlDropTable, NULL, NULL, &error);
	if (LastError)
	{
		thePlugin.Log(_T("Error executing SQLite3 statement: "));
		thePlugin.Log(sqlite3_errmsg(db) );
		sqlite3_free(error);
	}
	sqlDropTable = "DROP TABLE ObjectDecl";
	LastError = sqlite3_exec(db, sqlDropTable, NULL, NULL, &error);
	if (LastError)
	{
		thePlugin.Log(_T("Error executing SQLite3 statement: "));
		thePlugin.Log(sqlite3_errmsg(db) );
		sqlite3_free(error);
	}
	const char *sqlCreateTable = "CREATE TABLE ObjectList ("\
		"ID INTEGER PRIMARY KEY AUTOINCREMENT, Scope TEXT, Object TEXT, ClassID TEXT NOT NULL)";
	LastError = sqlite3_exec(db, sqlCreateTable, NULL, NULL, &error);
	if (LastError)
	{
		thePlugin.Log(_T("Error executing SQLite3 statement: "));
		thePlugin.Log(sqlite3_errmsg(db));
		sqlite3_free(error);
		return 1;
	}
	sqlCreateTable = "CREATE TABLE ObjectDecl ("\
		"ID INTEGER PRIMARY KEY AUTOINCREMENT, ClassID TEXT NOT NULL, Function TEXT NOT NULL, Params TEXT, Returns TEXT, ClassType INT)";
	LastError = sqlite3_exec(db, sqlCreateTable, NULL, NULL, &error);
	if (LastError)
	{
		thePlugin.Log(_T("Error executing SQLite3 statement: "));
		thePlugin.Log(sqlite3_errmsg(db));
		sqlite3_free(error);
		return 1;
	}
	// add some test data
	UpdateObjList(Obj("Main.seq","Calc","Calculator"));
	UpdateObjList(Obj("Main.seq","Cal","Calibrator"));
	UpdateObjDecl(ObjDecl("Calculator",tCTClass,"boolAnd","bool A,bool B","bool bReturn"));
	UpdateObjDecl(ObjDecl("Calibrator",tCTClass,"calcY","string Channel,float X","float fY"));

	return 0;
}

int Model::UpdateObjList(Obj& theObj) {
	char *error=0;
	LastError = RefreshObjListID(theObj);
	if (LastError) return 1;
	str _SQL("");
	if(theObj.ID()>0) {
		_SQL= ("Update ObjectList Set Scope='" +theObj.Scope() +
			"',Object='"+theObj.Name()+
			"',IDObjectDecl="+std::to_string((long long)theObj.ID())+
			" where ID="+std::to_string((long long)theObj.ID()) );
	}else {
		_SQL = ("INSERT INTO ObjectList (Scope , Object , ClassID) VALUES('");
		_SQL.append(theObj.Scope()).append("', '").append(theObj.Name()).append("', '").append(theObj.ClassID());
		//append(std::to_string((long long)theObj.ClassID()));
		_SQL.append("');");
	}
   LastError = sqlite3_exec(db, _SQL.c_str(), NULL, NULL, &error);
   if (LastError)
   {
	   thePlugin.Log(_SQL.c_str());
      thePlugin.Log(_T("Error executing SQLite3 statement: "));
	  thePlugin.Log(sqlite3_errmsg(db));
      sqlite3_free(error);
	  return 1;
   }
   LastError = RefreshObjListID(theObj);	//get Ids after insert

	return 0;
}
//fetch the primary key for the dataset or set to invalid
int Model::RefreshObjListID(Obj& theObj) {
	char *error=0;
	str _SQL("SELECT ID from ObjectList where Scope=='");
	_SQL+=theObj.Scope() +"' AND Object='" +theObj.Name()+"';";
	
	sqlite3_stmt *res;
	int rc = sqlite3_prepare(db,_SQL.c_str(), -1, &res, 0);       
    if (rc != SQLITE_OK) {   
		thePlugin.Log(_SQL.c_str());
		thePlugin.Log(_T("Failed to fetch data")); 
		thePlugin.Log(sqlite3_errmsg(db));
        return 1;
    }    
	theObj.updateID(0);
    rc = sqlite3_step(res);  
    if (rc == SQLITE_ROW) {
		theObj.updateID(sqlite3_column_int(res, 0));
    }
    sqlite3_finalize(res);
	return 0;
}
int Model::UpdateObjDecl(ObjDecl& theObj) {
	char *error=0;
	LastError = RefreshObjDeclID(theObj);
	if (LastError) return 1;
	str _SQL("");
	if(theObj.ID()>0) {
		_SQL= ("Update ObjectDecl Set ClassID='" +theObj.ClassID() +
			"',Function='"+theObj.Function()+
			"',Params="+theObj.Params()+
			"',Returns="+theObj.Returns()+
			"',ClassType="+std::to_string((long long)theObj.ClassType())+
			" where ID="+std::to_string((long long)theObj.ID()) );
	}else {
		_SQL = ("INSERT INTO ObjectDecl (ClassID, Function, Params, Returns, ClassType) VALUES('");
		_SQL.append(theObj.ClassID()).append("', '").append(theObj.Function()).append("', '").append(theObj.Params());
		_SQL.append("', '").append(theObj.Returns()).append("', ").append(std::to_string((long long)theObj.ClassType()));
		_SQL.append(");");
	}
   LastError = sqlite3_exec(db, _SQL.c_str(), NULL, NULL, &error);
   if (LastError)
   {
	   thePlugin.Log(_SQL.c_str());
      thePlugin.Log(_T("Error executing SQLite3 statement: "));
	  thePlugin.Log(sqlite3_errmsg(db));
      sqlite3_free(error);
	  return 1;
   }
   LastError = RefreshObjDeclID(theObj);	//get Ids after insert

	return 0;
}
//fetch the primary key for the dataset or set to invalid
int Model::RefreshObjDeclID(ObjDecl& theObj) {
	char *error=0;
	str _SQL("SELECT ID from ObjectDecl where ClassID=='");
	_SQL+=theObj.ClassID() +"' AND Function='" +theObj.Function()+"';";
	
	sqlite3_stmt *res;
	int rc = sqlite3_prepare(db,_SQL.c_str(), -1, &res, 0);       
    if (rc != SQLITE_OK) {   
		thePlugin.Log(_SQL.c_str());
		thePlugin.Log(_T("Failed to fetch data")); 
		thePlugin.Log(sqlite3_errmsg(db));
        return 1;
    }    
	theObj.updateID(0);
    rc = sqlite3_step(res);  
    if (rc == SQLITE_ROW) {
		theObj.updateID(sqlite3_column_int(res, 0));
    }
    sqlite3_finalize(res);
	return 0;
}