
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
	tstr _SQL;
	// Todo maybe its more efficient to put this into stored procedure?
	/*tstr _SQL2(_T(" from ObjectLinks inner join ObjectList as tab1 on tab1.ID==ObjectLinks.ID_ObjectList \
		  inner join ObjectDecl on ObjectDecl.ID==ObjectLinks.ID_ObjectDecl\
			inner join ObjectList as tab2 on tab2.ID==ObjectLinks.ID_ObjectListRel "));*/
	tstr _SQL2(_T(" from ObjectLinks inner join ObjectList as tab1 on tab1.ID==ObjectLinks.ID_ObjectList \
		 inner join ObjectList as tab2 on tab2.ID==ObjectLinks.ID_ObjectListRel \
		 left join ObjectDecl on ObjectDecl.ID==ObjectLinks.ID_ObjectDecl "));
	if (Object->empty()) {
		// because output needs to be sorted, the UNION needs to be wrapped in additional SELECT for ordering
		_SQL=_T("select Col1 From ( ");
		//is it a class-object
		_SQL+=_T("SELECT distinct tab2.Object as Col1")+ _SQL2 + _T("where tab1.Scope=='");
		_SQL+=(*Scope)+_T("' AND ClassType==")+ std::to_wstring((long long)tCTClass)+_T(" AND tab2.Object Like('")+(*BeginsWith)+_T("%') ");
		_SQL=_SQL+_T(" UNION ");
		//is it a SEQ-function
		_SQL=_SQL+_T("SELECT distinct Function as Col1")+ _SQL2 + _T("where tab1.Scope=='");	
		_SQL=_SQL+(*Scope)+_T("' AND ClassType==")+ std::to_wstring((long long)tCTSeq) +_T(" AND Function Like('")+(*BeginsWith)+_T("%')");
		_SQL=_SQL+_T(" UNION ");
		//is it a variable of basic type
		_SQL=_SQL+_T("SELECT distinct tab2.Object as Col1")+ _SQL2 + _T("where tab1.Scope=='");
		_SQL=_SQL+(*Scope)+_T("' AND ClassType==")+ std::to_wstring((long long)tCTType) +_T(" AND tab2.Object Like('")+(*BeginsWith)+_T("%')");
		_SQL=_SQL+_T(") order by Col1 ");
		_SQL=_SQL+_T(";");
		
	} else { // its a function of an object
		_SQL=_T("SELECT distinct Function")+ _SQL2 + _T("where tab1.Scope=='");
		_SQL+=(*Scope)+_T("' AND tab2.Object=='")+(*Object)+_T("' AND Function Like('")+(*BeginsWith)+_T("%')")+_T(" order by Function;");

	}
	std::vector<char>_sql=WcharMbcsConverter::wchar2char(_SQL.c_str());
	sqlite3_stmt *res;
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

int Model::GetParams( const tstr* Scope, const tstr* Object,const tstr* Function,tstr* Result ) {
	char *error=0;
	tstr _SQL;
	// Todo maybe its more efficient to put this into stored procedure?
	tstr _SQL2(_T(" from ObjectLinks inner join ObjectList as tab1 on tab1.ID==ObjectLinks.ID_ObjectList \
		  inner join ObjectDecl on ObjectDecl.ID==ObjectLinks.ID_ObjectDecl\
			inner join ObjectList as tab2 on tab2.ID==ObjectLinks.ID_ObjectListRel "));

	_SQL=_T("SELECT distinct Params")+ _SQL2 + _T("where tab1.Scope=='");
	_SQL+=(*Scope)+_T("' AND tab2.Object=='")+(*Object)+_T("' AND Function=='")+(*Function)+_T("'")+_T(" order by Params;");

	std::vector<char>_sql=WcharMbcsConverter::wchar2char(_SQL.c_str());
	sqlite3_stmt *res;
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
		
	while (rc == SQLITE_ROW) {
		if (!_result.empty()) _result.append("\n");
		_result.append((const char*)sqlite3_column_text(res, 0));
		rc = sqlite3_step(res);
		i=i+1;
	}
	sqlite3_finalize(res);
	Result->assign(WcharMbcsConverter::char2wcharStr(_result.c_str()));
	return 0;
}
int Model::GetReturns( const tstr* Scope, const tstr* Object,const tstr* Function,tstr* Result ) {
	char *error=0;
	tstr _SQL;
	// Todo maybe its more efficient to put this into stored procedure?
	tstr _SQL2(_T(" from ObjectLinks inner join ObjectList as tab1 on tab1.ID==ObjectLinks.ID_ObjectList \
		  inner join ObjectDecl on ObjectDecl.ID==ObjectLinks.ID_ObjectDecl\
			inner join ObjectList as tab2 on tab2.ID==ObjectLinks.ID_ObjectListRel "));

	_SQL=_T("SELECT distinct Returns")+ _SQL2 + _T("where tab1.Scope=='");
	_SQL+=(*Scope)+_T("' AND tab2.Object=='")+(*Object)+_T("' AND Function=='")+(*Function)+_T("'")+_T(" order by Params;");

	std::vector<char>_sql=WcharMbcsConverter::wchar2char(_SQL.c_str());
	sqlite3_stmt *res;
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
		
	while (rc == SQLITE_ROW) {
		if (!_result.empty()) _result.append("\n");
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
	m_ProjectPath.assign(WcharMbcsConverter::wchar2charStr(ProjectPath->c_str()));
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
	LastError = InitDatabase();	//Todo only init if not exist or corrupt
	if (LastError ) {
		return 1;
	}
	return 0;
}	

int Model::Rebuild() {
	if (PrepareForUpdate()!=0) return 1;
	if (RebuildObjList()!=0) return 1;
	if (RebuildClassDefinition()!=0) return 1;
	if (CleanupDeadLinks()!=0) return 1;
	return 0;
}
int Model::Update() {
	if (PrepareForUpdate()!=0) return 1;
	if (RebuildObjList()!=0) return 1;
	if (CleanupDeadLinks()!=0) return 1;
	return 0;
}

// marks every entry that needs to be updated
int Model::PrepareForUpdate() {
	str _SQL;
	_SQL.assign("Update ObjectList Set State=0");
	if(ExecuteSimpleQuery(_SQL)!=0) return 1;
	_SQL.assign("Update ObjectDecl Set State=0");
	if(ExecuteSimpleQuery(_SQL)!=0) return 1;
	return 0;
}
//deletes entrys that are not valid anymore and recreates new
int Model::CleanupDeadLinks() {
	//nachdem wir die Klassendeklarationen importiert haben 
	// müssen die Links von der Seq auf die Klasse neu erstellt werden
	// Main.seq-> Calc -> Calculator
	// aber auch tiefer verlinkt
	// Main.seq->test.seq->functions.seq-> Trace-> PrehTrace
	// zu Main.seq -> Trace -> PrehTrace
	char *error=0;
	int rc;
	sqlite3_stmt *res;
	str _SQL,_SQLSelect;
	//clear the old data
	_SQL.assign("delete from ObjectLinksTemp;");
	if(ExecuteSimpleQuery(_SQL)!=0) return 1;
	_SQL.assign("delete from ObjectLinks;");
	if(ExecuteSimpleQuery(_SQL)!=0) return 1;
	
	//erstmal die einfachen Verlinkungen eintragen
	//SELECT Scope,Object,Function,ObjectList.ClassID,ObjectDecl.ClassID,ClassType,
	//ObjectList.ID,ObjectDecl.ID from ObjectList inner join ObjectDecl on ObjectList.ClassID==ObjectDecl.ClassID;
	_SQL.assign("Insert Into ObjectLinks (ID_ObjectList,ID_ObjectDecl, ID_ObjectListRel) \
		SELECT ObjectList.ID,ObjectDecl.ID,ObjectList.ID from ObjectList inner join ObjectDecl on ObjectList.ClassID==ObjectDecl.ClassID;");
	if(ExecuteSimpleQuery(_SQL)!=0) return 1;
	
	//jetzt für jede Seq prüfen in welcher andere Seq sie included ist (ID_ObjectListA -> ID_ObjectListB); in temp. Tabelle eintragen	
	_SQL.assign("Insert Into ObjectLinksTemp (ID_A,ID_B) \
		SELECT distinct tab1.ID,tab2.ID FROM ObjectList as tab1 inner join ObjectList as tab2 on tab1.ClassID==tab2.Scope \
		left join ObjectDecl on ObjectDecl.ClassID==tab2.ClassID ");
	//??_SQL.append("where ClassType==").append(std::to_string((long long)tCTSeq));
	if(ExecuteSimpleQuery(_SQL)!=0) return 1;
	
	// in der temporären Tabelle werden die Seq-Verknüpfungen aufgelöst: 
	// 1) SELECT tab2.id,tab1.value FROM MyTable as tab1 inner join MyTable as tab2 on tab1.id==tab2.value; ausführen
	// 2) zurückgelieferte Ergebnisse in Tabelle anfügen
	// 3) das ganze so lange wiederholen bis Select kein Ergebnis mehr liefert
	// die Tabelle enthält nun für jede Seq auch Verweise auf indirekt eingebundene Seq
	// Meine Fresse diese query verstehe ich schon jetzt nicht mehr
	_SQLSelect.assign("SELECT distinct tab2.ID_A,tab1.ID_B FROM ObjectLinksTemp as tab1 inner join \
		ObjectLinksTemp as tab2 on (tab1.ID_A==tab2.ID_B AND tab1.ID_A!=tab1.ID_B AND tab2.ID_A!=tab2.ID_B )\
		where not exists (SELECT ID_A, ID_B FROM ObjectLinksTemp where ID_A==tab2.ID_A AND ID_B==tab1.ID_B);");
	_SQL.assign("Insert Into ObjectLinksTemp (ID_A,ID_B) ");  //Todo sub-seq are still listed doubled ?!
	_SQL.append(_SQLSelect);
	bool _Finished=false;
	while (!_Finished) {
		if(ExecuteSimpleQuery(_SQL)!=0) return 1;
	
		rc = sqlite3_prepare(db,_SQLSelect.c_str(), -1, &res, 0);       
		if (rc != SQLITE_OK) {   
			thePlugin.Log(_SQL.c_str());
			thePlugin.Log(_T("Failed to fetch data")); 
			thePlugin.Log(sqlite3_errmsg(db));
			return 1;
		}    
		rc = sqlite3_step(res);  
		_Finished=true;
		while (rc == SQLITE_ROW) {	// do we have to run the insert again or are we finished
			_Finished=false;
			rc = sqlite3_step(res);
		}
		sqlite3_finalize(res);
		
	}
	//jetzt für jeden Eintrag in temp. Tabelle die bereits vorhandenen Einträge in ObjectLinks duplizieren 
	// 1) Insert INTO ObjectLinks (ID_ObjectList,ID_ObjectDecl) SELECT Mytable.ID,ID_ObjectDecl from ObjectLinks inner join MyTable on Mytable.value==ID_ObjectList
	_SQL.assign("Insert INTO ObjectLinks (ID_ObjectList,ID_ObjectDecl, ID_ObjectListRel) \
		SELECT ObjectLinksTemp.ID_A,ObjectLinks.ID_ObjectDecl,ObjectLinksTemp.ID_B from ObjectLinks inner join ObjectLinksTemp on ObjectLinksTemp.ID_B==ObjectLinks.ID_ObjectList ");
	if(ExecuteSimpleQuery(_SQL)!=0) return 1;

	return 0;
}

// Todo would be nice to run this in parallel thread
// Todo UI is unresponsive because building blocks UI thread ?
int Model::RebuildObjList() {
	thePlugin.Log(_T("Scanning SEQ..."));
	
	// add the basic types to Intelisense
	std::vector<str>::const_iterator _Iter=Model::BASIC_TYPES().begin();
	for ( ; _Iter != Model::BASIC_TYPES().end( ); _Iter++ ) {
		std::string _A=*_Iter;
		_A.resize(_A.length()-1); // strip off space
		UpdateObjDecl(Model::ObjDecl(_A,Model::tCTType,"","",""));
	}

	SeqParser _parser(this);	
	//??std::vector<char> _vpath = WcharMbcsConverter::tchar2char(ProjectPath->c_str());
	//??std::string _path(_vpath.begin(),_vpath.end()-1); // remove 00
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
		dp=opendir((m_ProjectPath +_currDir).c_str());
		if(!dp) continue;
		while ((dirp = readdir( dp ))) {
			_file = dirp->d_name;
			_filepath = _currDir + "\\" + _file;
			// If the file is a directory (or is in some way invalid) we'll skip it 
			if (_file=="." || _file=="..") continue;
			if (stat( (m_ProjectPath +_filepath).c_str(), &_filestat )) continue;
			if (S_ISDIR( _filestat.st_mode ))  {
				_dirs.push_back(_currDir + "\\" + _file);
				continue;
			}
			if (_file.substr(_file.length()-4,4)!=".seq") continue; 
			_parser.AnalyseFile(false,m_ProjectPath,_filepath);
		}
		closedir( dp );
	}
	return 0;
}
//after building ObjList we now have to compile each class into ObjDecl
int Model::RebuildClassDefinition() {
	thePlugin.Log(_T("Scanning Classes..."));

	//find each entry in ObjList that is not linked to ObjDecl
	char *error=0;
	SeqParser _parser(this);
	str _SQL("SELECT distinct ObjectList.ClassID as ID2,ObjectDecl.ClassID as ID1 from ObjectList ");
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
	// Todo??time_t ltime;  ltime = (time_t)sqlite3_column_int(res, 1);
    while (rc == SQLITE_ROW) {
		_filepath.assign((const char*)sqlite3_column_text(res, 0));
		rc = sqlite3_step(res);
		_parser.AnalyseFile(true,m_ProjectPath,_filepath);
    }
    sqlite3_finalize(res);

	//clear the old data
	_SQL.assign("delete from ObjectList where State==0;");
	if(ExecuteSimpleQuery(_SQL)!=0) return 1;
	_SQL.assign("delete from ObjectDecl where State==0;");
	if(ExecuteSimpleQuery(_SQL)!=0) return 1;
	
	return 0;
}
int Model::InitDatabase() {
	thePlugin.Log(_T("Creating db..."));
	char *error=0;
	//drop all tables; no error if tables already dropped
	const char *sqlDropTable = "DROP TABLE ObjectList";				//Todo refactor with ExecuteSimpleQuery
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
	sqlDropTable = "DROP TABLE ObjectLinks";
	LastError = sqlite3_exec(db, sqlDropTable, NULL, NULL, &error);
	if (LastError)
	{
		thePlugin.Log(_T("Error executing SQLite3 statement: "));
		thePlugin.Log(sqlite3_errmsg(db) );
		sqlite3_free(error);
	}
	sqlDropTable = "DROP TABLE ObjectLinksTemp";
	LastError = sqlite3_exec(db, sqlDropTable, NULL, NULL, &error);
	if (LastError)
	{
		thePlugin.Log(_T("Error executing SQLite3 statement: "));
		thePlugin.Log(sqlite3_errmsg(db) );
		sqlite3_free(error);
	}
	//Create tables
	const char *sqlCreateTable = "CREATE TABLE ObjectList ("\
		"ID INTEGER PRIMARY KEY AUTOINCREMENT,File TEXT, Scope TEXT, Object TEXT, ClassID TEXT NOT NULL, State INT)";
	LastError = sqlite3_exec(db, sqlCreateTable, NULL, NULL, &error);
	if (LastError)
	{
		thePlugin.Log(_T("Error executing SQLite3 statement: "));
		thePlugin.Log(sqlite3_errmsg(db));
		sqlite3_free(error);
		return 1;
	}
	sqlCreateTable = "CREATE TABLE ObjectDecl ("\
		"ID INTEGER PRIMARY KEY AUTOINCREMENT, ClassID TEXT NOT NULL, Function TEXT NOT NULL, Params TEXT, Returns TEXT, ClassType INT, State INT, Time INT)";
	LastError = sqlite3_exec(db, sqlCreateTable, NULL, NULL, &error);
	if (LastError)
	{
		thePlugin.Log(_T("Error executing SQLite3 statement: "));
		thePlugin.Log(sqlite3_errmsg(db));
		sqlite3_free(error);
		return 1;
	}
	sqlCreateTable = "CREATE TABLE ObjectLinks ("\
		"ID INTEGER PRIMARY KEY AUTOINCREMENT, ID_ObjectList INT, ID_ObjectDecl INT, ID_ObjectListRel INT)";
	LastError = sqlite3_exec(db, sqlCreateTable, NULL, NULL, &error);
	if (LastError)
	{
		thePlugin.Log(_T("Error executing SQLite3 statement: "));
		thePlugin.Log(sqlite3_errmsg(db));
		sqlite3_free(error);
		return 1;
	}
	sqlCreateTable = "CREATE TABLE ObjectLinksTemp ("\
		"ID_A INT, ID_B INT)";
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
//insert/update Object
int Model::UpdateObjList(Obj& theObj) {
	char *error=0;
	LastError = RefreshObjListID(theObj);
	if (LastError) return 1;
	str _SQL("");
	if(theObj.ID()>0) {
		_SQL= ("Update ObjectList Set Scope='" +theObj.Scope() +
			"',Object='"+theObj.Name()+
			"',ClassID='"+theObj.ClassID()+
			"',State=1"+
			" where ID="+std::to_string((long long)theObj.ID()) );
	}else {
		_SQL = ("INSERT INTO ObjectList (Scope , Object , ClassID, State) VALUES('");
		_SQL.append(theObj.Scope()).append("', '").append(theObj.Name()).append("', '").append(theObj.ClassID());
		_SQL.append("',").append("1");
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
//insert/update ObjectDeclaration
int Model::UpdateObjDecl(ObjDecl& theObj) {
	char *error=0;
	LastError = RefreshObjDeclID(theObj);
	time_t ltime;
	time( &ltime );
	if (LastError) return 1;
	str _SQL("");
	if(theObj.ID()>0) {
		_SQL= ("Update ObjectDecl Set ClassID='" +theObj.ClassID() +
			"',Function='"+theObj.Function()+
			"',Params='"+theObj.Params()+
			"',Returns='"+theObj.Returns()+
			"',ClassType="+std::to_string((long long)theObj.ClassType())+
			" ,State=1"+
			" ,Time="+std::to_string((long long)ltime)+
			" where ID="+std::to_string((long long)theObj.ID()) );
	}else {
		_SQL = ("INSERT INTO ObjectDecl (ClassID, Function, Params, Returns, ClassType, State, Time) VALUES('");
		_SQL.append(theObj.ClassID()).append("', '").append(theObj.Function()).append("', '").append(theObj.Params());
		_SQL.append("', '").append(theObj.Returns()).append("', ").append(std::to_string((long long)theObj.ClassType()));
		_SQL.append(",1").append(",").append(std::to_string((long long)ltime));
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
int Model::ExecuteSimpleQuery( str SQL) {
	char *error=0;
	LastError = sqlite3_exec(db, SQL.c_str(), NULL, NULL, &error);
	if (LastError)
	{
		thePlugin.Log(_T("Error executing SQLite3 statement: "));
		thePlugin.Log(sqlite3_errmsg(db) );
		sqlite3_free(error);
		return 1;
	}
	return 0;
}