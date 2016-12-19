***********************************************
* NppPIALexer-Plugin for Notepad++ 5.9 (and above)
* by JK, 20016..
***********************************************
Links:
http://www.sqlite.org
https://www.tutorialspoint.com/sqlite/sqlite_data_types.htm
http://docs.notepad-plus-plus.org/index.php/Messages_And_Notifications
http://www.scintilla.org/ScintillaDoc.html#Notifications


1) Tool um aus Objects\xyz\doc\help.txt  eine SQLlite-Datenbasis zu erstellen

2) Plugin parst \Seq\StandardObj.seq usw. nach using ... um Objekte und deren Klassen zu kennen
	using "Objects\calculator\calcuclator.vi" as Calc

) Plugin parst Sequencen nach Funktionsdeklarationen  function kabum(bool x) -> bool,string

) Plugin parst Sequencen nach includes

) beim Eingeben von Zeichen Plugin schaut in Datenbasis

	- 1.Wort ist das Objekt
	-> in Datenbasis Klasse ermitteln ODER function aus seq oder include

	- nach . autoComplete für Funktion basierend auf Klasse filtern
	Calc.boolAnd
	
	 -nach ( parameterliste der funktion einblenden ?
	




***************************************
*  History  
***************************************
2016.11.06	JKubik
- created based on XBrackets Lite Plugin source

