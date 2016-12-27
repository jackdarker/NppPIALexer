
#include "Model.h"
#include "NppPIALexer.h"
#include "NppPIALexerOptions.h"
#include "core/WcharMbcsConverter.h"
#include "SeqParser.h"

extern CNppPIALexer thePlugin;
//??Todo using sqlite with UTF8, should switch to UTF16 since UNICODE is declared? 

Model::Model(){
	//thePlugin.Log(_T("model constructor"));
}

Model::~Model(){
	if(db) sqlite3_close(db);
	//thePlugin.Log(_T("model destructor"));
}
void Model::HandleDBError() {
	thePlugin.Log(_T("Error SQLite3 database"));
	thePlugin.Log(sqlite3_errmsg(db));
    sqlite3_close(db);
	db = NULL;
}
int Model::Export() {
	sqlite3_stmt *res;
	str _result("");
	char _sql[]="SELECT ID,Scope,Object,ClassID,State from ObjectList;";
	int rc;
	rc = sqlite3_prepare(db,_sql, -1, &res, 0);       
	if (rc != SQLITE_OK) {      
		thePlugin.Log(_sql);
		thePlugin.Log(_T("Failed to fetch data")); 
		thePlugin.Log(sqlite3_errmsg(db));
		return 1;
	}    
	rc = sqlite3_step(res);  
	thePlugin.Log("ID\t\tScope\t\tObject\t\tClassID\t\tState");
	while (rc == SQLITE_ROW ) {
		_result.clear();
		_result.append((const char*)sqlite3_column_text(res, 0)).append("\t\t");
		_result.append((const char*)sqlite3_column_text(res, 1)).append("\t\t");
		_result.append((const char*)sqlite3_column_text(res, 2)).append("\t\t");
		_result.append((const char*)sqlite3_column_text(res, 3)).append("\t\t");
		_result.append((const char*)sqlite3_column_text(res, 4)).append("\t\t");
		thePlugin.Log(_result.c_str());
		rc = sqlite3_step(res);
	}
	sqlite3_finalize(res);
	char _sql2[]="Select ID,ClassID,Function,Params,Returns,ClassType,State From ObjectDecl;";
	rc = sqlite3_prepare(db,_sql2, -1, &res, 0);       
	if (rc != SQLITE_OK) {      
		thePlugin.Log(_sql2);
		thePlugin.Log(_T("Failed to fetch data")); 
		thePlugin.Log(sqlite3_errmsg(db));
		return 1;
	}    
	rc = sqlite3_step(res);  
	thePlugin.Log("ID\t\tClassID\t\tFunction\t\tParams\t\tReturns\t\tClassType\t\tState");
	while (rc == SQLITE_ROW ) {
		_result.clear();
		_result.append((const char*)sqlite3_column_text(res, 0)).append("\t\t");
		_result.append((const char*)sqlite3_column_text(res, 1)).append("\t\t");
		_result.append((const char*)sqlite3_column_text(res, 2)).append("\t\t");
		_result.append((const char*)sqlite3_column_text(res, 3)).append("\t\t");
		_result.append((const char*)sqlite3_column_text(res, 4)).append("\t\t");
		_result.append((const char*)sqlite3_column_text(res, 5)).append("\t\t");
		thePlugin.Log(_result.c_str());
		rc = sqlite3_step(res);
	}
	sqlite3_finalize(res);
	return 0;
}
int Model::GetObject(const tstr* BeginsWith, const tstr* Scope, const tstr* Object,tstr* Result ) {
	char *error=0;
	if (Object->empty()) {
		// because output needs to be sorted, the UNION needs to be wrapped in additional SELECT for ordering
		//is it a class-object
		tstr _SQL(_T("select Col1 From (SELECT Object as Col1 from ObjectList inner join ObjectDecl on ObjectList.ClassID==ObjectDecl.ClassID where Scope=='"));
		_SQL+=(*Scope)+_T("' AND ClassType==")+ std::to_wstring((long long)tCTClass)+_T(" AND Object Like('")+(*BeginsWith)+_T("%') ");
		_SQL=_SQL+_T(" UNION ");
		//is it a SEQ-function
		_SQL=_SQL+_T("SELECT Function as Col1 from ObjectList inner join ObjectDecl on ObjectList.ClassID==ObjectDecl.ClassID where Scope=='");	
		_SQL=_SQL+(*Scope)+_T("' AND ClassType==")+ std::to_wstring((long long)tCTSeq) +_T(" AND Function Like('")+(*BeginsWith)+_T("%')");
		_SQL=_SQL+_T(" UNION ");
		//is it a variable of basic type
		_SQL=_SQL+_T("SELECT Object as Col1 from ObjectList inner join ObjectDecl on ObjectList.ClassID==ObjectDecl.ClassID where Scope=='");
		_SQL=_SQL+(*Scope)+_T("' AND ClassType==")+ std::to_wstring((long long)tCTType) +_T(" AND Object Like('")+(*BeginsWith)+_T("%')");
		_SQL=_SQL+_T(") order by Col1 ");
		_SQL=_SQL+_T(";");
		sqlite3_stmt *res;

		std::vector<char>_sql=WcharMbcsConverter::wchar2char(_SQL.c_str());
		int rc = sqlite3_prepare(db,&_sql[0], -1, &res, 0);       
		if (rc != SQLITE_OK) {      
			thePlugin.Log(&_sql[0]);
			thePlugin.Log(_T("Failed to fetch data")); 
			thePlugin.Log(sqlite3_errmsg(db));
			return 1;
		}    
		str _result("");
		rc = sqlite3_step(res);  
		int i=0;
		
		while (rc == SQLITE_ROW && i < AC_LIST_LENGTH_MAX) {
			if (!_result.empty()) _result.append(" ");
			_result.append((const char*)sqlite3_column_text(res, 0));
			rc = sqlite3_step(res);
			i=i+1;
		}
		sqlite3_finalize(res);
		Result->assign(WcharMbcsConverter::char2wcharStr(_result.c_str()));
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
	//char _sql[MAX_PATH];
	//WideCharToMultiByte(CP_ACP, 0, _SQL.c_str(),_SQL.size()+1, _sql , MAX_PATH, NULL, NULL);
	std::vector<char>_sql=WcharMbcsConverter::wchar2char(_SQL.c_str());
	int rc = sqlite3_prepare(db,&_sql[0], -1, &res, 0);       
    if (rc != SQLITE_OK) {      
		thePlugin.Log(&_sql[0]);
		thePlugin.Log(_T("Failed to fetch data")); 
		thePlugin.Log(sqlite3_errmsg(db));
        return 1;
    }    
    str _result("");
    rc = sqlite3_step(res);  
	int i=0;
    while (rc == SQLITE_ROW && i < AC_LIST_LENGTH_MAX) {
		if (!_result.empty()) _result.append(" ");
		_result.append((const char*)sqlite3_column_text(res, 0));
		rc = sqlite3_step(res);
		i=i+1;
    }
    sqlite3_finalize(res);
	Result->assign(WcharMbcsConverter::char2wcharStr(_result.c_str()));
	return 0;
}
int Model::LoadIntelisense(const tstr*  ProjectPath) {
	thePlugin.Log(_T("Opening db ..." ));
	tstr _FullPath;
	_FullPath.assign(ProjectPath->c_str());
	_FullPath.append(FILE_DB());	
	//char Dest[MAX_PATH*2+1];
	//WideCharToMultiByte(CP_ACP, 0, _FullPath.c_str(), wcslen(_FullPath.c_str())+1, Dest , MAX_PATH*2+1, NULL, NULL);
	std::vector<char> Dest=WcharMbcsConverter::wchar2char(_FullPath.c_str());
	LastError = sqlite3_open(&Dest[0], &db);  
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
// Todo would be nice to run this in parallel thread
int Model::RebuildObjList(const tstr*  ProjectPath) {
	thePlugin.Log(_T("Scanning SEQ..."));

	// add the basic types to Intelisense
	std::vector<str>::const_iterator _Iter=Model::BASIC_TYPES().begin();
	for ( ; _Iter != Model::BASIC_TYPES().end( ); _Iter++ ) {
		std::string _A=*_Iter;
		_A.resize(_A.length()-1); // strip off space
		UpdateObjDecl(Model::ObjDecl(_A,Model::tCTType,"","",""));
	}

	SeqParser _parser(this);	
	std::vector<char> _vpath = WcharMbcsConverter::tchar2char(ProjectPath->c_str());
	std::string _path(_vpath.begin(),_vpath.end()-1); // remove 00
	std::string _filepath;	// \subdir\test.seq
	std::string _file;		// test.seq
	std::string _currDir;	// \subdir
	DIR *dp;
	struct dirent *dirp;
	struct stat _filestat;
	std::vector<std::string> _dirs; //stack of directories relative to _path
	_dirs.push_back(DIR_SEQUENCES());	// Todo how about subprojects
	while (_dirs.size()>0) {
		_currDir= _dirs.back();
		_dirs.pop_back();
		thePlugin.Log(_currDir.c_str());
		dp=opendir((_path +_currDir).c_str());
		if(!dp) continue;
		while ((dirp = readdir( dp ))) {
			_file = dirp->d_name;
			_filepath = _currDir + "\\" + _file;
			// If the file is a directory (or is in some way invalid) we'll skip it 
			if (_file=="." || _file=="..") continue;
			if (stat( (_path +_filepath).c_str(), &_filestat )) continue;
			if (S_ISDIR( _filestat.st_mode ))  {
				_dirs.push_back(_currDir + "\\" + _file);
				continue;
			}
			if (_file.substr(_file.length()-4,4)!=".seq") continue; 
			_parser.AnalyseFile(false,_path,_filepath);
		}
		closedir( dp );
	}
	return 0;
}
//after building ObjList we now have to compile each class into ObjDecl
int Model::RebuildClassDefinition(const tstr*  ProjectPath) {
	thePlugin.Log(_T("Scanning Classes..."));
	std::vector<char> _vpath = WcharMbcsConverter::tchar2char(ProjectPath->c_str());  //Todo projectpath as Model-Member
	std::string _path(_vpath.begin(),_vpath.end()-1); // remove 00
	//find each entry in ObjList that is no linked to ObjDecl
	char *error=0;
	SeqParser _parser(this);
	str _SQL("SELECT ObjectList.ClassID,ObjectList.Time as ID2,ObjectDecl.ClassID as ID1 from ObjectList ");
	_SQL=_SQL+("Left outer join ObjectDecl on ObjectList.ClassID==ObjectDecl.ClassID where ID1 IS NULL;");
	str _filepath;
	sqlite3_stmt *res;
	int rc = sqlite3_prepare(db,_SQL.c_str(), -1, &res, 0);       
    if (rc != SQLITE_OK) {   
		thePlugin.Log(_SQL.c_str());
		thePlugin.Log(_T("Failed to fetch data")); 
		thePlugin.Log(sqlite3_errmsg(db));
        return 1;
    }    
    rc = sqlite3_step(res);  
	time_t ltime; // Todo??
    while (rc == SQLITE_ROW) {
		_filepath.assign((const char*)sqlite3_column_text(res, 0));
		rc = sqlite3_step(res);
		_parser.AnalyseFile(true,_path,_filepath);
    }
    sqlite3_finalize(res);
	return 0;
}
int Model::InitDatabase() {
	thePlugin.Log(_T("Creating db..."));
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
		"ID INTEGER PRIMARY KEY AUTOINCREMENT, Scope TEXT, Object TEXT, ClassID TEXT NOT NULL, State INT, Time INT)";
	LastError = sqlite3_exec(db, sqlCreateTable, NULL, NULL, &error);
	if (LastError)
	{
		thePlugin.Log(_T("Error executing SQLite3 statement: "));
		thePlugin.Log(sqlite3_errmsg(db));
		sqlite3_free(error);
		return 1;
	}
	sqlCreateTable = "CREATE TABLE ObjectDecl ("\
		"ID INTEGER PRIMARY KEY AUTOINCREMENT, ClassID TEXT NOT NULL, Function TEXT NOT NULL, Params TEXT, Returns TEXT, ClassType INT, State INT)";
	LastError = sqlite3_exec(db, sqlCreateTable, NULL, NULL, &error);
	if (LastError)
	{
		thePlugin.Log(_T("Error executing SQLite3 statement: "));
		thePlugin.Log(sqlite3_errmsg(db));
		sqlite3_free(error);
		return 1;
	}
	thePlugin.Log(_T("db ready"));
	return 0;
}
int Model::UpdateObjList(Obj& theObj) {
	char *error=0;
	time_t ltime;
	time( &ltime );
	LastError = RefreshObjListID(theObj);
	if (LastError) return 1;
	str _SQL("");
	if(theObj.ID()>0) {
		_SQL= ("Update ObjectList Set Scope='" +theObj.Scope() +
			"',Object='"+theObj.Name()+
			"',ClassID='"+theObj.ClassID()+
			"',State=1"+
			",Time="+std::to_string((long long)ltime)+
			" where ID="+std::to_string((long long)theObj.ID()) );
	}else {
		_SQL = ("INSERT INTO ObjectList (Scope , Object , ClassID, State, Time) VALUES('");
		_SQL.append(theObj.Scope()).append("', '").append(theObj.Name()).append("', '").append(theObj.ClassID());
		_SQL.append("',").append("1").append(",").append(std::to_string((long long)ltime));
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
			"',Params='"+theObj.Params()+
			"',Returns='"+theObj.Returns()+
			"',ClassType="+std::to_string((long long)theObj.ClassType())+
			" ,State=1"+
			" where ID="+std::to_string((long long)theObj.ID()) );
	}else {
		_SQL = ("INSERT INTO ObjectDecl (ClassID, Function, Params, Returns, ClassType, State) VALUES('");
		_SQL.append(theObj.ClassID()).append("', '").append(theObj.Function()).append("', '").append(theObj.Params());
		_SQL.append("', '").append(theObj.Returns()).append("', ").append(std::to_string((long long)theObj.ClassType()));
		_SQL.append(",1");
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