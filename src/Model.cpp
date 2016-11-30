
#include "Model.h"

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
int Model::LoadIntelisense(const TCHAR*  ProjectPath) {
	std::cout << "Opening db ..." << std::endl;
   //LastError = sqlite3_open(".\plugins\\Config\\MyDb.db", &db);
   LastError = sqlite3_open(ProjectPath, &db);
   
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
	char *error;
	const char *sqlCreateTable = "CREATE TABLE MyTable (id INTEGER PRIMARY KEY, value STRING);";
   LastError = sqlite3_exec(db, sqlCreateTable, NULL, NULL, &error);
   if (LastError)
   {
      std::cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << std::endl;
      sqlite3_free(error);
   }
   else
   {
      std::cout << "Created MyTable." << std::endl;
   }
 
   // Execute SQL
   std::cout << "Inserting a value into MyTable ..." << std::endl;
   const char *sqlInsert = "INSERT INTO MyTable VALUES(NULL, 'A Value');";
   LastError = sqlite3_exec(db, sqlInsert, NULL, NULL, &error);
   if (LastError)
   {
      std::cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << std::endl;
      sqlite3_free(error);
   }
   else
   {
      std::cout << "Inserted a value into MyTable." << std::endl ;
   }
	return 0;
}