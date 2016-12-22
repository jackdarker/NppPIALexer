#include "core/base.h"
#include "Model.h"

//
typedef int(*PFN)(int,int);

/*int bar2(int i, int j, Foo* pFoo, PFN pfn) {
    return (pFoo->*pfn)(i,j);
}*/
class SeqParser
{

public:
	SeqParser(Model *model){
		m_Model=model;
	};
	~SeqParser(){
		m_Model=NULL;
	};

	//this will load the file in readonly-mode and parse it
	int AnalyseFile(bool IsClassDefinition, std::string BasePath,std::string RelFilePath);

	//exisitiert der Pfad
	//lässt er sich öffnen
	//zeilenweise einlesen
	//nach Schlüsselwort function using include suchen
	

private:
	Model *m_Model;
	std::string m_BasePath;
	std::string m_RelFilePath;
	bool m_IsClassDefinition;
	int ParseLine(std::string Line);
	PFN OnInclude;
	tstr *m_Includes;
	tstr *m_Usings;
};