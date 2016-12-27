**********************************************************************************************
* NppPIALexer-Plugin for Notepad++ 5.9 (and above)
* by JK, 20016.
**********************************************************************************************
This plugin creates Intelisense data for PIA-Framework-projects and provides it for Auto-complete operations in the editor.
________________________________________________________________________________________________________________
Links
________________________________________________________________________________________________________________
uses http://www.sqlite.org for database (included as .h )
uses https://github.com/tronkko/dirent for directory-browsing (included as .h )

http://www.cplusplus.com/reference/cstring/
https://www.tutorialspoint.com/sqlite/sqlite_data_types.htm

http://docs.notepad-plus-plus.org/index.php/Messages_And_Notifications
http://www.scintilla.org/ScintillaDoc.html#Notifications
________________________________________________________________________________________________________________
Installation
________________________________________________________________________________________________________________
) NppPIALexer.dll nach .../Npp/plugins kopieren

) unter Plugins->NppPIALexer->ControlCenter Project-Pfad eingeben

) Reload Data zum erzeugen der Intelisense drücken

) log-file wird gespeichert in .../Npp/NppPIALexer.log
________________________________________________________________________________________________________________
Erzeugen der Intelisense
________________________________________________________________________________________________________________

) Plugin parst Sequencen nach using ... um Objekte und deren Klassen zu kennen
	using "Objects\calculator\calcuclator.vi" as Calc

) Plugin parst Sequencen nach Funktionsdeklarationen  
	function kabum(bool x) -> bool,string

) Plugin parst Sequencen nach include

) einbinden der Klassen-Intelisense s.u.

) das ganze wiederholen wenn Dateien geladen/gespeichert werden
________________________________________________________________________________________________________________
Datenmodel
________________________________________________________________________________________________________________
Intelisense wird in SQLite3 db gespeichert in Projektordner

Tabelle ObjectList 
ID		INT PRIMARY KEY     AUTOINCREMENT
Scope		TEXT		
Object		TEXT
ClassID		TEXT    NOT NULL
State		INT
Time		INT					<= Zeitstempel als Sekunden seit Mitternacht des 1. Januar 1970

Tabelle ObjectDecl
ID		INT PRIMARY KEY     AUTOINCREMENT
ClassID		TEXT    NOT NULL
ClassType	INT	NOT NULL
Function	TEXT    NOT NULL
ParameterList	TEXT    
ReturnList	TEXT    
Descr		TEXT
State		INT

) Bedeutung von State: 
	0 = Objekt nicht auffindbar (d.h. eventuel aus DB löschen)
	1 = Objekt existiert 

) Verknüpfung der Tabellen über:
SELECT * from ObjectList inner join ObjectDecl on ObjectList.ClassID==ObjectDecl.ClassID;

z.B. ObjectList
Scope		Object	ClassID		
---------------------------------------------------------------------------------------------------------------------------------
Main.seq	Calc	Calculator			<= Definition Objekt-Klasse
Main.seq	Functions.seq	Functions.seq			<= Funktion in include-sequenz
Functions.seq	Functions.seq	Functions.seq			<= Funktion in dieser sequenz
Main.seq	Main.seq	Main.seq
Main.seq	bOK	bool				<= Variable in dieser Sequenz (global)
Main.seq::Main	sText	string				<= Variable in dieser Funktion (lokal)

z.B. ObjectDecl
ClassID		ClassType	Function	ParameterList		ReturnList		Descr
---------------------------------------------------------------------------------------------------------------------------------
Calculator	Class		boolAnd		(bool A,bool B)		-> bool X		x = A & B
Calculator	Class		floatEquals	(float A, float B)	-> bool X
Preh_Trace	Class		CheckSNState	(string SN,string Type)	-> bool OK ,[string ACK]
Functions.seq	Seq		Homing		()			-> bool OK, string Text
Functions.seq	Seq		CloseDoor	(bool bClose)		-> bool OK
Main.seq	Seq		Main
bool		Typ
string		Typ


________________________________________________________________________________________________________________
Abrufen von Intelisense
________________________________________________________________________________________________________________
1) der Lexer erkennt welches Sequenzfile bearbeitet wird 
2) bei Eingabe von "." oder " " oder "(" oder "->" startet der Lexer; nach 2-3 Zeichen Vorschlagliste einblenden
	(in Functions.seq eintippen) "Calc.bo"
3) Nachschlagen: Scope=Functions.seq::CloseDoor	Obj=Calc Search=bo Typ=Class (weil ".")
	gibt es ein Objekt Calc im Scope Functions.seq::CloseDoor oder Functions.seq?
	ist es vom Typ Class?
	welche Function fängt mit "bo" an?

) (in Main.seq eintippen) "Ho"  das könnte eine Funktion, ein Klassen-Objekt oder ein Typ sein
) Nachschlagen in ObjList: Scope=Main.seq	Obj Like('Ho%') 	=> KlassenObjekt
  Nachschlagen in ObjDecl: ClassID Like("Ho%") ClassTyp=Typ		=> Typ
  Nachschlagen in ObjList join ObjDecl on ClassID: Function Like("Ho%")	Typ=Seq	=> Function

) kein Intelisense bei "//" ";" oder innerhalb ""

Project
|-->Main.seq
	|--> Calculator Calc	(by using)
	|--> bool bOK
	|--> Main
		|--> string sText
	|--> Functions.seq	(by include)
		|--> CloseDoor

|-->Tools.seq
	|--> Functions.seq	(by include)
		|--> CloseDoor

________________________________________________________________________________________________________________
Import von Klassen-Intelisense		
________________________________________________________________________________________________________________
) wird im Object gespeichert
	Objects\calculator\xyz_Commander.seq

) gleicher Syntax wie in .seq als Funktionsdeklaration
	//desc: returns the logical AND of 2 variables
	//param: bA 
	//param: bB
	//return: bReturn
	function boolAnd(bool bA, bool bB) -> bReturn


________________________________________________________________________________________________________________
  History  
________________________________________________________________________________________________________________
2016.11.06	JKubik
- created based on XBrackets Lite Plugin source

