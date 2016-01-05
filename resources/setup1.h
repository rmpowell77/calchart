#ifndef _SETUP1_H_
#define _SETUP1_H_
// file generated via the command:
// cat file.ps | sed 's/^/"/' | sed 's/$/\\n"/'
static char setup1_ps[] = "% run vmstatus.ps to get these values if changes made\n"
                          "%%BeginResource: font CalChart 5007 4717\n"
                          "8 dict begin\n"
                          "/FontType 3 def\n"
                          "/FontMatrix [.001 0 0 .001 0 0] def\n"
                          "/FontBBox [0 0 1000 1000] def\n"
                          "/Encoding 256 array def\n"
                          "0 1 255 {Encoding exch /.notdef put} for\n"
                          "Encoding 65 /plainman put\n"
                          "Encoding 66 /solman put\n"
                          "Encoding 67 /bsman put\n"
                          "Encoding 68 /slman put\n"
                          "Encoding 69 /xman put\n"
                          "Encoding 70 /solbsman put\n"
                          "Encoding 71 /solslman put\n"
                          "Encoding 72 /solxman put\n"
                          "/CharProcs 9 dict def\n"
                          "CharProcs begin\n"
                          "/.notdef {} def\n"
                          "/plainman {\n"
                          "   30 setlinewidth\n"
                          "   newpath\n"
                          "   300 300 250 0 360 arc\n"
                          "   closepath\n"
                          "   stroke\n"
                          "   0.25 setlinewidth\n"
                          "} bind def\n"
                          "/solman {\n"
                          "   newpath\n"
                          "   300 300 250 0 360 arc\n"
                          "   closepath\n"
                          "   fill\n"
                          "} bind def\n"
                          "/bsman {\n"
                          "   30 setlinewidth\n"
                          "   newpath\n"
                          "   300 300 250 0 360 arc\n"
                          "   closepath\n"
                          "   stroke\n"
                          "   0 600 moveto\n"
                          "   600 0 lineto\n"
                          "   stroke\n"
                          "   0.25 setlinewidth\n"
                          "} bind def\n"
                          "/slman {\n"
                          "   30 setlinewidth\n"
                          "   newpath\n"
                          "   300 300 250 0 360 arc\n"
                          "   closepath\n"
                          "   stroke\n"
                          "   600 600 moveto\n"
                          "   0 0 lineto\n"
                          "   stroke\n"
                          "   0.25 setlinewidth\n"
                          "} bind def\n"
                          "/xman {\n"
                          "   30 setlinewidth\n"
                          "   newpath\n"
                          "   300 300 250 0 360 arc\n"
                          "   closepath\n"
                          "   stroke\n"
                          "   0 600 moveto\n"
                          "   600 0 lineto\n"
                          "   stroke\n"
                          "   600 600 moveto\n"
                          "   0 0 lineto\n"
                          "   stroke\n"
                          "   0.25 setlinewidth\n"
                          "} bind def\n"
                          "/solbsman {\n"
                          "   30 setlinewidth\n"
                          "   newpath\n"
                          "   300 300 250 0 360 arc\n"
                          "   closepath\n"
                          "   fill\n"
                          "   0 600 moveto\n"
                          "   600 0 lineto\n"
                          "   stroke\n"
                          "   0.25 setlinewidth\n"
                          "} bind def\n"
                          "/solslman {\n"
                          "   30 setlinewidth\n"
                          "   newpath\n"
                          "   300 300 250 0 360 arc\n"
                          "   closepath\n"
                          "   fill\n"
                          "   600 600 moveto\n"
                          "   0 0 lineto\n"
                          "   stroke\n"
                          "   0.25 setlinewidth\n"
                          "} bind def\n"
                          "/solxman {\n"
                          "   30 setlinewidth\n"
                          "   newpath\n"
                          "   300 300 250 0 360 arc\n"
                          "   closepath\n"
                          "   fill\n"
                          "   0 600 moveto\n"
                          "   600 0 lineto\n"
                          "   stroke\n"
                          "   600 600 moveto\n"
                          "   0 0 lineto\n"
                          "   stroke\n"
                          "   0.25 setlinewidth\n"
                          "} bind def\n"
                          "end\n"
                          "/BuildGlyph {\n"
                          "   700 0 0 0 600 600 setcachedevice\n"
                          "   exch /CharProcs get exch\n"
                          "   2 copy known not {pop /.notdef} if\n"
                          "   get exec\n"
                          "} bind def\n"
                          "/BuildChar {\n"
                          "   1 index /Encoding get exch get\n"
                          "   1 index /BuildGlyph get exec\n"
                          "} bind def\n"
                          "currentdict\n"
                          "end\n"
                          "/CalChart exch definefont pop\n"
                          "%%EndResource\n"
                          "/Enc-iso8859 [ /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef "
                          "/.notdef\n"
                          "     /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef "
                          "/.notdef\n"
                          "     /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef "
                          "/.notdef\n"
                          "     /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef "
                          "/.notdef\n"
                          "     /.notdef /space /exclam /quotedbl /numbersign /dollar /percent "
                          "/ampersand\n"
                          "     /quoteright /parenleft /parenright /asterisk /plus /comma /hyphen "
                          "/period\n"
                          "     /slash /zero /one /two /three /four /five /six /seven /eight /nine "
                          "/colon\n"
                          "     /semicolon /less /equal /greater /question /at /A /B /C /D /E /F /G "
                          "/H /I\n"
                          "     /J /K /L /M /N /O /P /Q /R /S /T /U /V /W /X /Y /Z /bracketleft "
                          "/backslash\n"
                          "     /bracketright /asciicircum /underscore /quoteleft /a /b /c /d /e /f "
                          "/g /h\n"
                          "     /i /j /k /l /m /n /o /p /q /r /s /t /u /v /w /x /y /z /braceleft "
                          "/bar\n"
                          "     /braceright /asciitilde /.notdef /.notdef /.notdef /.notdef "
                          "/.notdef\n"
                          "     /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef "
                          "/.notdef\n"
                          "     /.notdef /.notdef /.notdef /.notdef /dotlessi /grave /acute "
                          "/circumflex\n"
                          "     /tilde /macron /breve /dotaccent /dieresis /.notdef /ring /cedilla\n"
                          "     /.notdef /hungarumlaut /ogonek /caron /space /exclamdown /cent "
                          "/sterling\n"
                          "     /currency /yen /brokenbar /section /dieresis /copyright "
                          "/ordfeminine\n"
                          "     /guillemotleft /logicalnot /hyphen /registered /macron /degree "
                          "/plusminus\n"
                          "     /twosuperior /threesuperior /acute /mu /paragraph /periodcentered "
                          "/cedilla\n"
                          "     /onesuperior /ordmasculine /guillemotright /onequarter /onehalf\n"
                          "     /threequarters /questiondown /Agrave /Aacute /Acircumflex /Atilde\n"
                          "     /Adieresis /Aring /AE /Ccedilla /Egrave /Eacute /Ecircumflex "
                          "/Edieresis\n"
                          "     /Igrave /Iacute /Icircumflex /Idieresis /Eth /Ntilde /Ograve "
                          "/Oacute\n"
                          "     /Ocircumflex /Otilde /Odieresis /multiply /Oslash /Ugrave /Uacute\n"
                          "     /Ucircumflex /Udieresis /Yacute /Thorn /germandbls /agrave /aacute\n"
                          "     /acircumflex /atilde /adieresis /aring /ae /ccedilla /egrave "
                          "/eacute\n"
                          "     /ecircumflex /edieresis /igrave /iacute /icircumflex /idieresis "
                          "/eth\n"
                          "     /ntilde /ograve /oacute /ocircumflex /otilde /odieresis /divide "
                          "/oslash\n"
                          "     /ugrave /uacute /ucircumflex /udieresis /yacute /thorn /ydieresis] "
                          "def\n"
                          "/headfont Enc-iso8859 headfont0 ReencodeFont\n"
                          "/mainfont Enc-iso8859 mainfont0 ReencodeFont\n"
                          "/numberfont Enc-iso8859 numberfont0 ReencodeFont\n"
                          "/contfont Enc-iso8859 contfont0 ReencodeFont\n"
                          "/boldfont Enc-iso8859 boldfont0 ReencodeFont\n"
                          "/italfont Enc-iso8859 italfont0 ReencodeFont\n"
                          "/bolditalfont Enc-iso8859 bolditalfont0 ReencodeFont\n";
#endif // _SETUP1_H_
