#include "SeqParser.h"

// BasePath = c:\projects\xyz
// RelFilePath =			\prg\seq\default\Main.seq

int SeqParser::AnalyseFile(bool IsClassDefinition, std::string BasePath,std::string RelFilePath){ 
	std::ifstream _filestream;
	std::string Path;
	m_RelFilePath = RelFilePath;
	m_BasePath = BasePath;
	m_IsClassDefinition = IsClassDefinition;
	
	//Todo make this really a relative path
	Path.append(m_BasePath).append("\\").append(m_RelFilePath);
	_filestream.open(Path,std::ios::in);
	if (_filestream.fail()) {
		return 1;
	}
	m_Model->UpdateObjList(Model::Obj(m_RelFilePath,m_RelFilePath,m_RelFilePath)); //each SEQ includes itself

	std::string buffer;
	while (std::getline(_filestream, buffer)) { 
		ParseLine(buffer);
	}
	return 0;
}

int SeqParser::ParseLine(std::string Line) {
	int _offset=0;
	int _found, _foundB,_foundC;
	std::string _A;
	std::string _B;
	std::string _C;
	Line = SeqParser::RemoveSpaces(Line); //could cause issue with filepaths containing spaces?
	int _foundComment=Line.find("//",_offset);
	if (_foundComment!= std::string::npos) { // Todo skip if comment
	}
	if ((_found=Line.find("#using ",_offset),_found != std::string::npos)) {
		//   #using "sfswf\sf\Calculator_Commander.vi" as Calc
		_offset = _offset+_found+8; // after #using "
		if ((_found=Line.find(" as ",_offset),_found != std::string::npos)) {
			_A= Line.substr(_offset, _found-_offset-1);  //strip of ""
			_A="\\"+_A;
			_B= Line.substr(_found + 4);
			m_Model->UpdateObjList(Model::Obj(m_RelFilePath,_B,_A));
		}
	} else if ((_found=Line.find("#include ",_offset),_found != std::string::npos)) {
		// #include test1.seq
		_offset = _offset+_found+9; // after #include
		_A= Line.substr(_offset);
		m_Model->UpdateObjList(Model::Obj(m_RelFilePath,_A,Model::DIR_SEQUENCES() +"\\"+ _A));
	} else if ((_found=Line.find("function ",_offset), _found != std::string::npos)) {
		// function boolAnd (bool bA, bool bB) ->bool bReturn
		_offset = _offset+_found+9; // after function
		if ((_foundC=Line.find("->",_offset),_foundC != std::string::npos)) {
			_C= Line.substr(_foundC+2);	// after ->
			if ((_foundB=Line.find("(",_offset),_foundB != std::string::npos)) {
				_B= Line.substr(_foundB+1,_foundC-_foundB-2 ); // in between ( )
				_A= Line.substr(_offset,_foundB-_offset);
				m_Model->UpdateObjDecl(Model::ObjDecl(m_RelFilePath,
					m_IsClassDefinition? Model::tCTClass : Model::tCTSeq ,
					_A,_B,_C));
			}
		}

	}
	return 0;
}