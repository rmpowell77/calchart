/* print_ui.cpp
 * Dialox box for printing
 *
 * Modification history:
 * 7-16-95    Garrick Meeker              Created
 *
 */

/*
   Copyright (C) 1995-2008  Garrick Brian Meeker

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef __GNUG__
#pragma implementation
#endif

#include "print_ui.h"
#include "show_ui.h"
#include "confgr.h"

enum {
  CC_PRINT_ORIENT_PORTRAIT,
  CC_PRINT_ORIENT_LANDSCAPE
};

enum {
  CC_PRINT_ACTION_PRINTER,
  CC_PRINT_ACTION_FILE,
  CC_PRINT_ACTION_PREVIEW
};

CC_WinNodePrint::CC_WinNodePrint(CC_WinList *lst, ShowPrintDialog *req)
: CC_WinNode(lst), printreq(req) {
}

void CC_WinNodePrint::SetShow(CC_show *) {
  printreq->Update();
}

void CC_WinNodePrint::ChangePrint(wxWindow* win) {
  if (win != printreq) {
    // Make sure a different window made the changes
    printreq->Update();
  }
}

static void ShowPrintOk(wxButton& button, wxEvent&) {
  const char *s;
#ifdef PRINT__RUN_CMD
  wxString buf;
#endif
  bool overview;
  int minyards;

  ShowPrintDialog* dialog = (ShowPrintDialog*) button.GetParent();

  StringToFloat(dialog->text_x->GetValue(), &page_offset_x);
  StringToFloat(dialog->text_y->GetValue(), &page_offset_y);
  StringToFloat(dialog->text_width->GetValue(), &page_width);
  StringToFloat(dialog->text_height->GetValue(), &page_height);
  StringToInt(dialog->text_minyards->GetValue(), &minyards);

  dialog->show_descr->show->GetboolLandscape() =
    (dialog->radio_orient->GetSelection() == CC_PRINT_ORIENT_LANDSCAPE);

  dialog->show_descr->show->GetboolDoCont() = dialog->check_cont->GetValue();
  if (!dialog->eps)
    dialog->show_descr->show->GetboolDoContSheet() =
      dialog->check_pages->GetValue();
  overview = dialog->check_overview->GetValue();

  switch (dialog->radio_method->GetSelection()) {
  case CC_PRINT_ACTION_PREVIEW:
#ifdef PRINT__RUN_CMD
    print_view_cmd = dialog->text_view_cmd->GetValue();
    print_view_opts = dialog->text_view_opts->GetValue();
    s = tmpnam(NULL);
    buf.sprintf("%s %s %s", print_view_cmd.Chars(),print_view_opts.Chars(),s);
#endif
    break;
  case CC_PRINT_ACTION_FILE:
    s = wxFileSelector("Print to file", NULL, NULL, NULL,
		       dialog->eps ? "*.eps":"*.ps");
    if (!s) return;
    break;
  case CC_PRINT_ACTION_PRINTER:
#ifdef PRINT__RUN_CMD
    print_cmd = dialog->text_cmd->GetValue();
    print_opts = dialog->text_opts->GetValue();
    s = tmpnam(NULL);
    buf.sprintf("%s %s %s", print_cmd.Chars(), print_opts.Chars(), s);
#else
    s = dialog->text_cmd->GetValue();
    print_file = s;
#endif
    break;
  default:
    s = NULL;
  }
  
  // Update other windows
  dialog->show_descr->show->winlist->ChangePrint(dialog);

  FILE *fp;

  fp = fopen(s, "w");
  if (fp) {
    int n;
    wxString tempbuf;

    wxBeginBusyCursor();
    n = dialog->show_descr->show->Print(fp, dialog->eps, overview,
					dialog->show_descr->curr_ss, minyards);
    fflush(fp);
    fclose(fp);

#ifdef PRINT__RUN_CMD
    switch (dialog->radio_method->GetSelection()) {
    case CC_PRINT_ACTION_FILE:
      break;
    default:
      if (dialog->show_descr->show->Ok()) {
	system(buf);
      }
      wxRemoveFile(s);
      break;
    }
#endif
    wxEndBusyCursor();

    if (dialog->show_descr->show->Ok()) {
      tempbuf.sprintf("Printed %d pages.", n);
      (void)wxMessageBox(tempbuf.GetData(),
			 (char *)dialog->show_descr->show->GetName());
    } else {
      (void)wxMessageBox((char *)dialog->show_descr->show->GetError(),
			 (char *)dialog->show_descr->show->GetName());
    }
  } else {
    (void)wxMessageBox("Unable to open print file for writing!",
		       (char *)dialog->show_descr->show->GetName());
  }
}

static void ShowPrintCancel(wxButton& button, wxEvent&) {
  ShowPrintDialog* dialog = (ShowPrintDialog*) button.GetParent();
  dialog->Close();
}

static void ShowPrintSelect(wxButton& button, wxEvent&) {
  ShowPrintDialog* dialog = (ShowPrintDialog*) button.GetParent();
  (void)new StuntSheetPicker(dialog->show_descr->show, dialog->node->GetList(),
			     true, dialog->frame, "Select stuntsheets");
}

ShowPrintDialog::ShowPrintDialog(CC_descr *dcr, CC_WinList *lst,
				 bool printEPS, wxFrame *parent, char *title,
				 bool isModal, int x, int y,
				 int width, int height):
wxDialogBox(parent, title, isModal, x, y, width, height),
show_descr(dcr), eps(printEPS), frame(parent) {
  wxString buf;

  node = new CC_WinNodePrint(lst, this);

  SetLabelPosition(wxVERTICAL);

  wxButton *okBut = new wxButton(this, (wxFunction)ShowPrintOk, "&Print");
  (void) new wxButton(this, (wxFunction)ShowPrintCancel, "&Close");
  NewLine();
  NewLine();
  okBut->SetDefault();

#ifdef PRINT__RUN_CMD
  text_cmd = new wxText(this, (wxFunction)NULL, "Printer Command: ",
			print_cmd.GetData(),
			-1, -1, 100, -1);

  text_opts = new wxText(this, (wxFunction)NULL, "Printer Options: ",
			 print_opts.GetData(),
			 -1, -1, 150, -1);
  NewLine();
  text_view_cmd = new wxText(this, (wxFunction)NULL, "Preview Command: ",
			     print_view_cmd.GetData(),
			     -1, -1, 100, -1);

  text_view_opts = new wxText(this, (wxFunction)NULL, "Preview Options: ",
			      print_view_opts.GetData(),
			      -1, -1, 150, -1);
  NewLine();
  NewLine();
#endif

#ifndef PRINT__RUN_CMD
  text_cmd = new wxText(this, (wxFunction)NULL, "Printer &Device: ",
			print_file.GetData(),
			-1, -1, 100, -1);

  NewLine();
  NewLine();
#endif

  char *orientation[2];
  orientation[0] = "Portrait";
  orientation[1] = "Landscape";
  radio_orient = new wxRadioBox(this, (wxFunction)NULL, "&Orientation: ",
		    -1,-1,-1,-1,2,orientation,2,wxHORIZONTAL|wxFLAT);
  radio_orient->SetSelection((int)show_descr->show->GetboolLandscape());

  char *print_modes[3];
  print_modes[0] = "Send to Printer";
  print_modes[1] = "Print to File";
  print_modes[2] = "Preview Only";
#ifdef PRINT__RUN_CMD
  int features = 3;
#else
  int features = 2;
#endif
  radio_method = new wxRadioBox(this, (wxFunction)NULL, "Post&Script:",
				-1,-1,-1,-1, features, print_modes,
				features, wxHORIZONTAL|wxFLAT);
  radio_method->SetSelection(0);
  
  NewLine();
  NewLine();

  check_overview = new wxCheckBox(this, (wxFunction)NULL, "Over&view");
  check_overview->SetValue(false);
  check_cont = new wxCheckBox(this, (wxFunction)NULL, "Continuit&y");
  check_cont->SetValue(show_descr->show->GetboolDoCont());
  if (!eps) {
    check_pages = new wxCheckBox(this, (wxFunction)NULL, "Cove&r pages");
    check_pages->SetValue(show_descr->show->GetboolDoContSheet());
  }

  if (!eps) {
    NewLine();
    (void) new wxButton(this, (wxFunction)ShowPrintSelect,"S&elect sheets...");
  }

  NewLine();
  NewLine();

  buf.sprintf("%.2f", page_width);
  text_width = new wxText(this, (wxFunction)NULL, "Page &width: ",
			  buf.GetData(), -1, -1, 100, -1);

  buf.sprintf("%.2f", page_height);
  text_height = new wxText(this, (wxFunction)NULL, "Page &height: ",
			   buf.GetData(), -1, -1, 100, -1);

  buf.sprintf("%.2f", page_offset_x);
  text_x = new wxText(this, (wxFunction)NULL, "&Left margin: ",
		      buf.GetData(), -1, -1, 100, -1);
  
  buf.sprintf("%.2f", page_offset_y);
  text_y = new wxText(this, (wxFunction)NULL, "&Top margin: ",
		      buf.GetData(), -1, -1, 100, -1);

  text_minyards = new wxText(this, (wxFunction)NULL, "Yards: ",
			     "50", -1, -1, 100, -1);

  NewLine();
  NewLine();

  Fit();

  Show(true);
}

ShowPrintDialog::~ShowPrintDialog() {
  if (node) {
    node->Remove();
    delete node;
  }
}

bool ShowPrintDialog::OnClose(void) {
  return true;
}

void ShowPrintDialog::Update() {
  radio_orient->SetSelection((int)show_descr->show->GetboolLandscape());
  check_cont->SetValue(show_descr->show->GetboolDoCont());
  if (!eps) check_pages->SetValue(show_descr->show->GetboolDoContSheet());
}
