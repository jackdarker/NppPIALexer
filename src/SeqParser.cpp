#include "SeqParser.h"
#include "NppPIALexer.h"

extern CNppPIALexer thePlugin;
// BasePath = c:\projects\xyz
// RelFilePath =			\prg\seq\default\Main.seq  or Main.xxx

int SeqParser::AnalyseFile(bool IsClassDefinition, std::string BasePath,std::string RelFilePath){ 
	std::ifstream _filestream;
	std::string Path;
	m_RelFilePath = RelFilePath;
	m_DescFilePath = RelFilePath;
	m_BasePath = BasePath;
	m_IsClassDefinition = IsClassDefinition;
	
	struct stat _filestat;
	int i=m_DescFilePath.rfind('.');
	if(i==std::string::npos) return 1; // no .xxx
	m_DescFilePath.replace(i,4,".seq");
	//Todo make this really a relative path
	Path.append(m_BasePath).append(m_DescFilePath);			//??.append("\\").append(m_DescFilePath);
	thePlugin.Log(m_DescFilePath.c_str());	
	//check if the Object exists
	// f.e Source\Objects\Calculator\Calculator_Commander.vi
	// -> if c:\Projects\test\Source\Objects\Calculator\Calculator_Commander.SEQ exists, open and parse it
	if (stat( Path.c_str(), &_filestat )) {
		thePlugin.Log(_T("error reading file"));
		return 1;
	}
	if (S_ISDIR( _filestat.st_mode )) {
		thePlugin.Log(_T("skipped because is directory"));
		return 1;
	}
	_filestream.open(Path,std::ios::in);
	if (_filestream.fail()) {
		return 1;
	}
	if(!m_IsClassDefinition) 
		m_Model->UpdateObjList(Model::Obj(m_RelFilePath,"",m_RelFilePath)); //each SEQ includes itself

	std::string buffer;
	_Longlong n=0;
	while (std::getline(_filestream, buffer)) { 
		ParseLine(buffer);
		n+=1;
	}
	//??thePlugin.Log(str("lines: ").append(std::to_string(n)).c_str());
	return 0;
}

int SeqParser::ParseLine(std::string Line) {
	int _offset=0;
	int _found, _foundB,_foundC;
	std::string _A;
	std::string _B;
	std::string _C;
	Line = SeqParser::RemoveComment(Line);
	Line = SeqParser::RemoveSpaces(Line); //Todo could cause issue with filepaths containing spaces?
	if ((_found=Line.find("using ",_offset),_found != std::string::npos)) {
		//   #using "Objects\Calculator\Calculator_Commander.vi" as Calc		
		_offset = _offset+_found+7; // after using "
		if ((_found=Line.find(" as ",_offset),_found != std::string::npos)) { // could also trigger by SetDO(HOusing as =true)??
			_A= Line.substr(_offset, _found-_offset-1);  //strip of ""
			_A="\\SOURCE\\"+_A;	//we have to prepend \SOURCE\.. !
			_B= Line.substr(_found + 4);
			m_Model->UpdateObjList(Model::Obj(m_RelFilePath,_B,_A));
		}
	} else if ((_found=Line.find("#include ",_offset),_found != std::string::npos)) {
		// #include test1.seq
		_offset = _offset+_found+9; // after #include 
		_A= Line.substr(_offset);
		_A= _A.substr(1,_A.length()-2);//strip off ""
		m_Model->UpdateObjList(Model::Obj(m_RelFilePath,_A,Model::DIR_SEQUENCES() +"\\"+ _A));
	} else if ((_found=Line.find("function ",_offset), _found != std::string::npos)) {
		//    function boolAnd (bool bA, bool bB) ->bool bReturn
		//or  function boolAnd (bool bA, bool bB)
		_offset = _offset+_found+9; // after function
		_foundC = Line.length()-1;
		if ((_foundC=Line.find("->",_offset),_foundC != std::string::npos)) {
			_C= Line.substr(_foundC+2);	// after ->	
		} 
		if ((_foundB=Line.find("(",_offset),_foundB != std::string::npos)) {
				_B= Line.substr(_foundB+1,_foundC-_foundB-2 ); // in between ( )
				_A= Line.substr(_offset,_foundB-_offset);
				m_Model->UpdateObjDecl(Model::ObjDecl(m_RelFilePath,
					m_IsClassDefinition? Model::tCTClass : Model::tCTSeq ,
					_A,_B,_C));
		}
	} else {// parse basic type
		// double fVolt=1.2
		std::vector<str>::const_iterator _Iter=Model::BASIC_TYPES().begin();
		for ( ; _Iter != Model::BASIC_TYPES().end( ); _Iter++ ) {
			_found=Line.find(*_Iter,0);
			if (_found==0) { //datatype should be at start of line, not in between ()
				_A=*_Iter;
				_A.resize(_A.length()-1); // strip off space
				_foundB = _found+(_Iter->length());
				_foundC = Line.find('=',_foundB);	// double fVolt=2.5
				if (_foundC == std::string::npos) {
					_foundC = Line.find(';',_foundB);	// int iX;		
					if (_foundC == std::string::npos) {
						_foundC=Line.length(); // int iX<lf>
					}
				} 
				_B=Line.substr(_foundB,_foundC-_foundB);
				m_Model->UpdateObjList(Model::Obj(m_RelFilePath,_B,_A));
			}
		}
	}
	return 0;
}