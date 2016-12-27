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

	// removes comment blocks (complete lines or at end of code) in this text line
	// only // for now, not  /**/
	//  function foo("no // comment") -> sText //comment
	//  function foo("no // comment") -> sText 
	static std::string RemoveComment(std::string in) {
		int i = in.length();
		std::string::const_iterator str_Iter;
		std::string out;
		bool _text = false, _text2=false; //found starting/end of ""
		int _Comment = 0; // >=2 -> comment
		bool _Start=false;
		for ( str_Iter = in.begin( ); str_Iter != in.end( ); str_Iter++ ) {
			if(*str_Iter=='"') {
				_text ? (_text=false, _text2=true) : (_text=true, _text2=false);
			}
			if(_text || _text2) {  // in between "
			} else if (*str_Iter=='/') { // is this a comment?
				(_Comment<2)? _Comment=_Comment+1 : _Comment=_Comment+0;
			} else if (_Comment==1) { // there was / before but none following
				out.push_back('/'); //so we have to restore it
				_Comment=_Comment-1;
			}

			if (_Comment<1 ){	
				out.push_back(*str_Iter);
			}
			_text2=false;
		}
		return out;
	}
	// removes unnecessary whitespaces and tabs in this text line
	//   function  foo ( bool X , "this that & his" ,  string _A ) -> double Y , string test ;
	// function foo(bool X,"this that & his",string _A)->double Y, string test;
	static std::string RemoveSpaces(std::string in) {
		int i = in.length();
		std::string::const_iterator str_Iter;
		std::string out;
		//remove whitespace if double, before or after special characters
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
			if(_text || _text2) {  // in between "
				_Start=true;
				if ((_white & 0x2)>0) out.push_back(' '); //add space if there was at leat one before
				out.push_back(*str_Iter);
			} else {
				if ((*str_Iter==' ' || *str_Iter=='\x09') && (_special & 0x2)==0) { //set flag if there is space but no special before
					_white=_white+1;
				} else if (filter.find(*str_Iter)!=std::string::npos) { //special char; scrap space before
					_special=_special +1;
					_white = 0;	
				} else if ((*str_Iter==' ' || *str_Iter=='\x09') && (_special & 0x2)>0){ //scrap space after special
					_white = 2;	
				} else if ((_special & 0x2)>0){ //reset flag if normal char
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
			_Start ? (_white=(_white*2)& 0x3): (_white=0); // shift out the flag
			_special=(_special*2 | (_special & 0x2))& 0x3; //keep the flag; required if multiple spaces after special flag
			_text2=false;
		}
		return out;
	};

private:
	Model *m_Model;
	std::string m_BasePath;
	std::string m_RelFilePath;	// this is the seq-file or Class-File
	std::string m_DescFilePath; // this is the original seq-file or the description-file for Class
	bool m_IsClassDefinition;
	int ParseLine(std::string Line);
	PFN OnInclude;
	tstr *m_Includes;
	tstr *m_Usings;
};