"/smoveto { transform round exch round exch itransform moveto } def\n"
"/slineto { transform round exch round exch itransform lineto } def\n"
"/dotplain {\n"
"   /y exch def\n"
"   /x exch def\n"
"   newpath\n"
"   x y w 0 360 arc\n"
"   closepath\n"
"   stroke\n"
"} bind def\n"
"/dotsolid {\n"
"   /y exch def\n"
"   /x exch def\n"
"   newpath\n"
"   x y w 0 360 arc\n"
"   closepath\n"
"   fill\n"
"} bind def\n"
"/dotbs {\n"
"   /y exch def\n"
"   /x exch def\n"
"   newpath\n"
"   x y w 0 360 arc\n"
"   closepath\n"
"   stroke\n"
"   x plinew sub y plinew add smoveto\n"
"   x plinew add y plinew sub slineto\n"
"   stroke\n"
"} bind def\n"
"/dotsl {\n"
"   /y exch def\n"
"   /x exch def\n"
"   newpath\n"
"   x y w 0 360 arc\n"
"   closepath\n"
"   stroke\n"
"   x plinew add y plinew add smoveto\n"
"   x plinew sub y plinew sub slineto\n"
"   stroke\n"
"} bind def\n"
"/dotx {\n"
"   /y exch def\n"
"   /x exch def\n"
"   newpath\n"
"   x y w 0 360 arc\n"
"   closepath\n"
"   stroke\n"
"   x plinew sub y plinew add smoveto\n"
"   x plinew add y plinew sub slineto\n"
"   stroke\n"
"   x plinew add y plinew add smoveto\n"
"   x plinew sub y plinew sub slineto\n"
"   stroke\n"
"} bind def\n"
"/dotsolbs {\n"
"   /y exch def\n"
"   /x exch def\n"
"   newpath\n"
"   x y w 0 360 arc\n"
"   closepath\n"
"   fill\n"
"   x slinew sub y slinew add smoveto\n"
"   x slinew add y slinew sub slineto\n"
"   stroke\n"
"} bind def\n"
"/dotsolsl {\n"
"   /y exch def\n"
"   /x exch def\n"
"   newpath\n"
"   x y w 0 360 arc\n"
"   closepath\n"
"   fill\n"
"   x slinew add y slinew add smoveto\n"
"   x slinew sub y slinew sub slineto\n"
"   stroke\n"
"} bind def\n"
"/dotsolx {\n"
"   /y exch def\n"
"   /x exch def\n"
"   newpath\n"
"   x y w 0 360 arc\n"
"   closepath\n"
"   fill\n"
"   x slinew sub y slinew add smoveto\n"
"   x slinew add y slinew sub slineto\n"
"   stroke\n"
"   x slinew add y slinew add smoveto\n"
"   x slinew sub y slinew sub slineto\n"
"   stroke\n"
"} bind def\n"
"/leftText {\n"
"   x y moveto dup stringwidth pop x add /x exch def show\n"
"} bind def\n"
"/rightText {\n"
"   dup stringwidth pop\n"
"   rmargin exch sub\n"
"   y smoveto show\n"
"} bind def\n"
"/centerText {\n"
"   dup stringwidth pop 2 div\n"
"   rmargin lmargin sub 2 div\n"
"   exch sub\n"
"   lmargin add\n"
"   y smoveto show\n"
"} bind def\n"
"/drawTextfield {\n"
"   lmargin y smoveto rmargin y slineto stroke\n"
"   /y y h 1.1 mul sub def centerText\n"
"} bind def\n"
"/space_over {\n"
"   /x x ( ) stringwidth pop add def\n"
"} bind def\n"
"/do_tab {\n"
"   /tabnum exch def\n"
"   x tabnum lt {/x tabnum def} {space_over} ifelse\n"
"} bind def\n"
"/donumber {\n"
"   w 1.25 mul add /y exch def /rmargin exch def rightText\n"
"} bind def\n"
"/donumber2 {\n"
"   w 1.25 mul add /y exch def /x exch def leftText\n"
"} bind def\n"
"/doarrows {\n"
"   /mainfont findfont sprstepsize 1.5 mul scalefont setfont\n"
"   /x sprstepsize def\n"
"   /lmargin 0 def /rmargin sprstepsize 8 mul def\n"
"   /y sprstepsize neg 4 mul def\n"
"   newpath rmargin y smoveto 0 y slineto stroke\n"
"   x y x sub smoveto 0 y slineto x y x add slineto stroke\n"
"   /y y x sub sprstepsize 1.5 mul sub def (south) centerText\n"
"   /y sprstepsize 4 mul fieldh add def\n"
"   newpath rmargin y smoveto 0 y slineto\n"
"   x y x sub smoveto 0 y slineto x y x add slineto stroke\n"
"   /y y x add def (south) centerText\n"
"   /lmargin fieldw sprstepsize 8 mul sub def /rmargin fieldw def\n"
"   /y sprstepsize neg 4 mul def\n"
"   newpath lmargin y smoveto rmargin y slineto\n"
"   rmargin x sub y x sub smoveto\n"
"   rmargin y slineto\n"
"   rmargin x sub y x add slineto stroke\n"
"   /y y x sub sprstepsize 1.5 mul sub def (north) centerText\n"
"   /y sprstepsize 4 mul fieldh add def\n"
"   newpath lmargin y smoveto rmargin y slineto\n"
"   rmargin x sub y x sub smoveto\n"
"   rmargin y slineto\n"
"   rmargin x sub y x add slineto stroke\n"
"   /y y x add def (north) centerText\n"
"   /lmargin fieldw .9 mul def /rmargin fieldw def\n"
"   newpath lmargin fieldy neg smoveto\n"
"   rmargin fieldy neg slineto\n"
"   rmargin sprstepsize 4 mul fieldy sub slineto\n"
"   lmargin sprstepsize 4 mul fieldy sub slineto closepath stroke\n"
"   /mainfont findfont sprstepsize 3 mul scalefont setfont\n"
"   /y sprstepsize 2 div fieldy sub def pagenumtext centerText\n"
"} bind def\n"
"/drawfield {\n"
"   sfieldx sfieldy translate\n"
"   /w stepsize 8 mul def\n"
"   /x 0 def nfieldw 8 idiv 1 add {\n"
"      x 0 smoveto x sfieldh slineto stroke /x x w add def\n"
"   } repeat\n"
"   [ stepsize ] 0 setdash\n"
"   /x w 2 div def nfieldw 8 idiv {\n"
"      x sfieldh smoveto x 0 slineto stroke /x x w add def\n"
"   } repeat\n"
"   [] 0 setdash\n"
"   /h sfieldh nfieldh div 8 mul def\n"
"   /y sfieldh def nfieldh 3 add 8 idiv 1 add {\n"
"      0 y smoveto sfieldw y slineto stroke /y y h sub def\n"
"   } repeat\n"
"   [ sfieldh nfieldh div ] 0 setdash\n"
"   /y sfieldh h 2 div sub def nfieldh 7 add 8 idiv {\n"
"      0 y smoveto sfieldw y slineto stroke /y y h sub def\n"
"   } repeat\n"
"   [] 0 setdash\n"
"   sfieldx neg sfieldy neg translate\n"
"   doarrows\n"
"   /headfont findfont sprstepsize headsize mul scalefont setfont\n"
"   /lmargin 0 def /rmargin fieldw def\n"
"   /y sprstepsize 16 headsize sub mul fieldh add def\n"
"   (UNIVERSITY OF CALIFORNIA MARCHING BAND) centerText\n"
"   /h sprstepsize 1.5 mul def\n"
"   /mainfont findfont h scalefont setfont\n"
"   /lmargin fieldw 4 div def /rmargin fieldw .75 mul def\n"
"   /y sprstepsize 8 mul fieldh add def\n"
"   (Music) drawTextfield\n"
"   /y sprstepsize 4 mul fieldh add def\n"
"   (Formation) drawTextfield\n"
"   /lmargin 0 def /rmargin fieldw 8 div def\n"
"   /y sprstepsize 10 mul fieldh add def\n"
"   (date) drawTextfield\n"
"   /lmargin fieldw .875 mul def /rmargin fieldw def\n"
"   /y sprstepsize 10 mul fieldh add def\n"
"   (page) drawTextfield\n"
"   /mainfont findfont sprstepsize 3 mul scalefont setfont\n"
"   /y sprstepsize 10.5 mul fieldh add def pagenumtext centerText\n"
"} bind def\n"
"/ReencodeFont {\n"
"   findfont\n"
"   dup length 1 add dict begin\n"
"      {1 index /FID ne {def} {pop pop} ifelse} forall\n"
"      /Encoding exch def\n"
"      currentdict\n"
"   end\n"
"   definefont pop\n"
"} bind def\n"
"/BeginEPSF {\n"
"   /b4_Inc_state save def\n"
"   /dict_count countdictstack def\n"
"   /op_count count 1 sub def\n"
"   userdict begin\n"
"   /showpage {} def\n"
"   0 setgray 0 setlinecap\n"
"   1 setlinewidth 0 setlinejoin\n"
"   10 setmiterlimit [] 0 setdash newpath\n"
"   /languagelevel where\n"
"   {pop languagelevel\n"
"   1 ne\n"
"      {false setstrokeadjust false setoverprint\n"
"      } if\n"
"   } if\n"
"} bind def\n"
"/EndEPSF {\n"
"   count op_count sub {pop} repeat\n"
"   countdictstack dict_count sub {end} repeat\n"
"   b4_Inc_state restore\n"
"} bind def\n"
