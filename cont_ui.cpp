/* cont_ui.cc
 * Continuity editors
 *
 * Modification history:
 * 1-10-96    Garrick Meeker              Created
 *
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include "cont_ui.h"

extern wxFont *contPlainFont;
extern wxFont *contBoldFont;
extern wxFont *contItalFont;
extern wxFont *contBoldItalFont;

static void toolbar_printcont_sym0(CoolToolBar *tb);
static void toolbar_printcont_sym1(CoolToolBar *tb);
static void toolbar_printcont_sym2(CoolToolBar *tb);
static void toolbar_printcont_sym3(CoolToolBar *tb);
static void toolbar_printcont_sym4(CoolToolBar *tb);
static void toolbar_printcont_sym5(CoolToolBar *tb);
static void toolbar_printcont_sym6(CoolToolBar *tb);
static void toolbar_printcont_sym7(CoolToolBar *tb);

ToolBarEntry printcont_tb[] = {
  { NULL, "Insert plainmen", toolbar_printcont_sym0 },
  { NULL, "Insert solidmen", toolbar_printcont_sym1 },
  { NULL, "Insert plain backslash men", toolbar_printcont_sym2 },
  { NULL, "Insert plain slash men", toolbar_printcont_sym3 },
  { NULL, "Insert plain x men", toolbar_printcont_sym4 },
  { NULL, "Insert solid backslash men", toolbar_printcont_sym5 },
  { NULL, "Insert solid slash men", toolbar_printcont_sym6 },
  { NULL, "Insert solid x men", toolbar_printcont_sym7 }
};

CC_WinNodeCont::CC_WinNodeCont(CC_WinList *lst, ContinuityEditor *req)
: CC_WinNode(lst), editor(req) {}

void CC_WinNodeCont::SetShow(CC_show *) {
  editor->DetachText();
  editor->Update(TRUE);
}

void CC_WinNodeCont::GotoSheet(unsigned) {
  editor->Update();
}

void CC_WinNodeCont::DeleteSheet(unsigned sht) {
  if (sht == editor->GetShowDescr()->curr_ss) {
    editor->DetachText();
  }
}

void CC_WinNodeCont::AddContinuity(unsigned sht, unsigned cont) {
  editor->Update();
}

void CC_WinNodeCont::DeleteContinuity(unsigned sht, unsigned cont) {
  editor->Update();
}

void CC_WinNodeCont::FlushContinuity() {
  editor->FlushText();
}

void CC_WinNodeCont::SetContinuity(wxWindow *win,
				   unsigned sht, unsigned cont) {
  if ((win != editor) && (sht == editor->GetShowDescr()->curr_ss) &&
      (cont == editor->GetCurrent())) {
    editor->UpdateText(TRUE);
  }
}

CC_WinNodePrintCont::CC_WinNodePrintCont(CC_WinList *lst,
					 PrintContEditor *req)
: CC_WinNode(lst), editor(req) {}

void CC_WinNodePrintCont::SetShow(CC_show *) {
  editor->canvas->Update();
}

void CC_WinNodePrintCont::GotoSheet(unsigned) {
  editor->canvas->Update();
}

static void ContEditSet(wxButton& button, wxEvent&) {
  ContinuityEditor* editor =
    (ContinuityEditor*)(button.GetParent()->GetParent());

  editor->SetPoints();
}

static void ContEditSelect(wxButton& button, wxEvent&) {
  ContinuityEditor* editor =
    (ContinuityEditor*)(button.GetParent()->GetParent());

  editor->SelectPoints();
}

static void ContEditCurrent(wxChoice& choice, wxEvent&) {
  ContinuityEditor *editor =
    (ContinuityEditor*)(choice.GetParent()->GetParent());
  editor->SetCurrent(choice.GetSelection());
}

ContinuityEditor::ContinuityEditor(CC_descr *dcr, CC_WinList *lst,
				   wxFrame *parent, char *title,
				   int x, int y, int width, int height):
wxFrame(parent, title, x, y, width, height, wxSDI | wxDEFAULT_FRAME),
descr(dcr), curr_cont(0), text_sheet(NULL), text_contnum(0) {
  CreateStatusLine();

  panel = new wxPanel(this);

  (void)new wxButton(panel, (wxFunction)ContEditSet, "Set Points");
  (void)new wxButton(panel, (wxFunction)ContEditSelect, "Select Points");

  conts = new wxChoice(panel, (wxFunction)ContEditCurrent, "");

  text = new FancyTextWin(this);
  
  wxMenu *cont_menu = new wxMenu;
  cont_menu->Append(CALCHART__CONT_NEW, "New");
  cont_menu->Append(CALCHART__CONT_DELETE, "Delete");
  cont_menu->Append(CALCHART__CONT_CLOSE, "Close window");
  wxMenuBar *menu_bar = new wxMenuBar;
  menu_bar->Append(cont_menu, "Continuity");
  SetMenuBar(menu_bar);

  OnSize(-1, -1);
  Show(TRUE);

  Update();

  node = new CC_WinNodeCont(lst, this);
}

ContinuityEditor::~ContinuityEditor() {
  if (node) {
    node->Remove();
    delete node;
  }
}

void ContinuityEditor::OnSize(int, int) {
  int width, height;
  int text_x, text_y;

  GetClientSize(&width, &height);
  panel->Fit();
  panel->GetSize(&text_x, &text_y);
  panel->SetSize(0, 0, width, text_y);
  text->SetSize(0, text_y, width, height-text_y);
}

Bool ContinuityEditor::OnClose(void) {
  FlushText();
  return TRUE;
}

void ContinuityEditor::OnMenuCommand(int id) {
  switch(id) {
  case CALCHART__CONT_NEW:
    break;
  case CALCHART__CONT_DELETE:
    break;
  case CALCHART__CONT_CLOSE:
    Close();
  }
}

void ContinuityEditor::OnMenuSelect(int id) {
  char *msg = NULL;

  switch (id) {
  case CALCHART__CONT_NEW:
    msg = "Add new continuity";
    break;
  case CALCHART__CONT_DELETE:
    msg = "Delete this continuity";
    break;
  case CALCHART__CONT_CLOSE:
    msg = "Close window";
    break;
  }
  if (msg) SetStatusText(msg);
}

void ContinuityEditor::Update(Bool quick) {
  CC_sheet *sht = descr->CurrSheet();
  CC_continuity *curranimcont;

  conts->Clear();
  for (curranimcont = sht->animcont; curranimcont != NULL;
       curranimcont = curranimcont->next) {
    conts->Append((char *)((const char *)curranimcont->name));
  }
  conts->SetSelection(0);
  curr_cont = 0;

  UpdateText(quick);
}

void ContinuityEditor::UpdateText(Bool quick) {
  CC_sheet *sht = descr->CurrSheet();
  CC_continuity *c;

  if (quick) {
    c = sht->GetNthContinuity(curr_cont);
  } else {
    c = sht->UserGetNthContinuity(curr_cont);
  }
  text_sheet = sht;
  text_contnum = curr_cont;
  text->Clear();
  if (c != NULL) {
    if (c->text)
      text->WriteText((char *)((const char *)c->text));
  }
}

void ContinuityEditor::FlushText() {
  char *conttext;
  CC_continuity *cont;

  if (text_sheet) {
    cont = text_sheet->GetNthContinuity(text_contnum);
    if (cont != NULL) {
      conttext = text->GetContents();
      if (strcmp(conttext, cont->text) != 0) {
	text_sheet->UserSetNthContinuity(conttext, text_contnum, this);
      }
      delete conttext;
    }
  }
}

void ContinuityEditor::SelectPoints() {
  CC_sheet *sht = descr->CurrSheet();
  CC_continuity *c;

  c = sht->GetNthContinuity(curr_cont);
  if (c != NULL) {
    if (sht->SelectContinuity(c->num)) {
      descr->show->winlist->UpdateSelections();
    }
  }
}

void ContinuityEditor::SetPoints() {
  CC_sheet *sht = descr->CurrSheet();
  CC_continuity *c;

  c = sht->GetNthContinuity(curr_cont);
  if (c != NULL) {
    sht->SetContinuity(c->num);
  }
}

PrintContCanvas::PrintContCanvas(wxFrame *frame, CC_descr *dcr):
wxCanvas(frame, -1, -1, -1, -1, 0), show_descr(dcr), ourframe(frame),
topline(0) {
  GetDC()->SetMapMode(MM_TEXT);
  GetDC()->SetBackground(wxWHITE_BRUSH);
}

PrintContCanvas::~PrintContCanvas() {}

void PrintContCanvas::OnPaint() {
  cc_text *cont, *c;
  CC_sheet *sht = show_descr->CurrSheet();
  Bool do_tab;
  unsigned i;
  float x, y;
  float textw, texth, textd, maxtexth;
  int devx, devy;
  unsigned tabnum;
  float tabw;

  BeginDrawing();
  SetTextForeground(wxBLACK);
  Clear();

  cont = sht->continuity;
  for (i = 0; (i < topline) && (cont != NULL); i++, cont = cont->next);
  y = 0.0;
  SetFont(contPlainFont);
  GetTextExtent("012345", &tabw, &texth, &textd); // Get size of tab
  while (cont) {
    x = 0.0;
    if (cont->center) {
      for (c = cont; c != NULL; c = c->more) {
	do_tab = FALSE;
	switch (c->font) {
	case PSFONT_SYMBOL:
	  x += contPlainFont->GetPointSize()*strlen(c->text);
	  break;
	case PSFONT_NORM:
	  SetFont(contPlainFont);
	  break;
	case PSFONT_BOLD:
	  SetFont(contBoldFont);
	  break;
	case PSFONT_ITAL:
	  SetFont(contItalFont);
	  break;
	case PSFONT_BOLDITAL:
	  SetFont(contBoldItalFont);
	  break;
	case PSFONT_TAB:
	  do_tab = TRUE;
	  break;
	}
	if (!do_tab && (c->font != PSFONT_SYMBOL)) {
	  GetTextExtent(c->text, &textw, &texth, &textd);
	  x += textw;
	}
      }
      GetVirtualSize(&devx, &devy);
      x = (GetDC()->DeviceToLogicalX(devx) - x) / 2;
      if (x < 0.0) x = 0.0;
    }
    maxtexth = 0.0;
    tabnum = 0;
    for (c = cont; c != NULL; c = c->more) {
      do_tab = FALSE;
      switch (c->font) {
      case PSFONT_NORM:
	SetFont(contPlainFont);
	break;
      case PSFONT_BOLD:
	SetFont(contBoldFont);
	break;
      case PSFONT_ITAL:
	SetFont(contItalFont);
	break;
      case PSFONT_BOLDITAL:
	SetFont(contBoldItalFont);
	break;
      case PSFONT_TAB:
	tabnum++;
	textw = tabnum * tabw;
	if (textw >= x) x = textw;
	else x += tabw/6;
	do_tab = TRUE;
	break;
      default:
	break;
      }
      if (c->font == PSFONT_SYMBOL) {
	int pointsize = contPlainFont->GetPointSize();
	x += pointsize*strlen(c->text);
	if (pointsize > maxtexth) maxtexth = pointsize;
      } else {
	if (!do_tab) {
	  GetTextExtent(c->text, &textw, &texth, &textd);
	  if (texth > maxtexth) maxtexth = texth;
	  DrawText(c->text, x, y);
	  x += textw;
	}
      }
    }
    y += maxtexth;
    cont = cont->next;
  }

  SetFont(NULL);
  EndDrawing();
}

void PrintContCanvas::OnEvent(wxMouseEvent& event) {
}

void PrintContCanvas::OnChar(wxKeyEvent& event) {
}

void PrintContClose(wxButton& button, wxEvent&) {
  ((PrintContEditor*)(button.GetParent()->GetParent()))->Close();
}

PrintContEditor::PrintContEditor(CC_descr *dcr, CC_WinList *lst,
				 wxFrame *parent, char *title,
				 int x, int y, int width, int height)
: wxFrameWithStuff(parent, title, x, y, width, height) {
  CreateStatusLine(1);

  // Add a toolbar
  CoolToolBar *ribbon = new CoolToolBar(this, 0, 0, -1, -1, 0,
					wxHORIZONTAL, 20);
  ribbon->SetupBar(printcont_tb, sizeof(printcont_tb)/sizeof(ToolBarEntry));
  SetToolBar(ribbon);

  // Add the canvas
  canvas = new PrintContCanvas(this, dcr);
  SetCanvas(canvas);

  // Add the buttons
  SetPanel(new wxPanel(this));
  (void)new wxButton(framePanel, (wxFunction)PrintContClose, "Close");

  node = new CC_WinNodePrintCont(lst, this);

  SetLayoutMethod(wxFRAMESTUFF_PNL_TB);
  OnSize(-1, -1);
  Show(TRUE);
}

PrintContEditor::~PrintContEditor() {
  if (node) {
    node->Remove();
    delete node;
  }
}

static void toolbar_printcont_sym0(CoolToolBar *tb) {
}
static void toolbar_printcont_sym1(CoolToolBar *tb) {
}
static void toolbar_printcont_sym2(CoolToolBar *tb) {
}
static void toolbar_printcont_sym3(CoolToolBar *tb) {
}
static void toolbar_printcont_sym4(CoolToolBar *tb) {
}
static void toolbar_printcont_sym5(CoolToolBar *tb) {
}
static void toolbar_printcont_sym6(CoolToolBar *tb) {
}
static void toolbar_printcont_sym7(CoolToolBar *tb) {
}
