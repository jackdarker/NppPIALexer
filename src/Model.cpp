
#include "Model.h"

//??Todo using sqlite with UTF8, should switch to UTF16 since UNICODE is declared? 

Model::Model(){
	std::cout <<"model constructor"<< std::endl;
}

Model::~Model(){
	std::cout <<"model destructor"<< std::endl;
}
void Model::HandleDBError() {
	  std::cerr << "Error SQLite3 database: " << sqlite3_errmsg(db) << std::endl;
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
int Model::GetObject(const TCHAR* BeginsWith, const TCHAR* Scope, TCHAR* Result ) {
	char *error=0;

	char *sql = "SELECT Object from ObjectList";
	const char* data = "Callback function called";
	sqlite3_stmt *res;
	int rc = sqlite3_prepare(db,sql, -1, &res, 0);       
    if (rc != SQLITE_OK) {      
		std::cerr <<"Failed to fetch data: "<< sqlite3_errmsg(db) <<std::endl;       
        return 1;
    }    
    
    rc = sqlite3_step(res);  
    if (rc == SQLITE_ROW) {
		const char* name = ((const char*)sqlite3_column_text(res, 0));
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, name, (int)strlen(name), NULL, 0);
		MultiByteToWideChar(CP_UTF8, 0, name, (int)strlen(name), Result, size_needed);
    }
    sqlite3_finalize(res);
	return 0;
}

int Model::LoadIntelisense(const TCHAR*  ProjectPath) {
	std::cout << "Opening db ..." << std::endl;
   //LastError = sqlite3_open(".\plugins\\Config\\MyDb.db", &db);

	char Dest[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, ProjectPath, wcslen(ProjectPath)+1, Dest , MAX_PATH, NULL, NULL);

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
   sqlite3_close(db);
   return 0;
}
		
int Model::InitDatabase() {
	char *error=0;
	const char *sqlDropTable = "DROP TABLE ObjectList";
   LastError = sqlite3_exec(db, sqlDropTable, NULL, NULL, &error);
   if (LastError)
   {
      std::cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << std::endl;
      sqlite3_free(error);
   }

	const char *sqlCreateTable = "CREATE TABLE ObjectList ("\
		"ID INTEGER PRIMARY KEY AUTOINCREMENT, Scope TEXT, Object TEXT, IDObjectDecl INT NOT NULL)";
   LastError = sqlite3_exec(db, sqlCreateTable, NULL, NULL, &error);
   if (LastError)
   {
      std::cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << std::endl;
      sqlite3_free(error);
	  return 1;
   }

 
   // Execute SQL
   std::cout << "Inserting a value into ..." << std::endl;
   const char *sqlInsert = "INSERT INTO ObjectList (Scope , Object , IDObjectDecl) VALUES('Main.seq', 'Calc', 1)";
   LastError = sqlite3_exec(db, sqlInsert, NULL, NULL, &error);
   if (LastError)
   {
      std::cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << std::endl;
      sqlite3_free(error);
	  return 1;
   }

   TCHAR Name[2*MAX_PATH + 1];
   this->GetObject(_T(""),_T(""),Name);
	return 0;
}