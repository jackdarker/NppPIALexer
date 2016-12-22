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
	
	// removes unnecessary whitespaces
	//   function  foo ( bool X , "this that & his" ,  string _A ) -> double Y , string test ;
	// function foo(bool X,"this that & his",string _A)->double Y, string test;
	static std::string RemoveSpaces(std::string in) {
		int i = in.length();
		std::string::const_iterator str_Iter;
		std::string out;
		//remove whitespace if double, before or after ()-><+\/,;.!=?:
		// AND if not in between ""
		std::string filter("()-><+\\/,;.!=?:*%&|");
		bool _text = false, _text2=false; //found starting/end of ""
		int _white = 0; // is/was whitespace
		int _special=0; // is/was special char
		bool _Start=false;
		for ( str_Iter = in.begin( ); str_Iter != in.end( ); str_Iter++ ) {
			if(*str_Iter=='"') {
				_text ? (_text=false, _text2=true) : (_text=true, _text2=false);
			}
			if(_text || _text2) {  // in between comment
				_Start=true;
				if ((_white & 0x2)>0) out.push_back(' ');
				out.push_back(*str_Iter);
			} else {
				if (*str_Iter==' ' && (_special & 0x2)==0) {
					_white=_white+1;
				} else if (filter.find(*str_Iter)!=std::string::npos) {
					_special=_special +1;
					_white = 0;	//special char; scrap space before
				} else if ((_special & 0x2)>0){
					_white = 0;
					_special = 0;
				}

				if ((_white & 0x2)>0 && (_white & 0x1 )==0 && 
					(_special & 0x2)==0 && (_special & 0x1 )==0) { //normal character, but spaces before; insert one
					out.push_back(' ');
					out.push_back(*str_Iter);
				} else if((_white & 0x2)==0 && (_white & 0x1)==0) { //normal character
					_Start=true;
					out.push_back(*str_Iter);
				}
			}
			if (_Start)	{  _white=(_white*2)& 0x3; 
			} else {
				_white=0;
			}
			_special=(_special*2 | (_special & 0x2))& 0x3;
		}
		return out;
	};

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