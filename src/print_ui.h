/*
 * print_ui.h
 * Dialox box for printing
 */

/*
   Copyright (C) 1995-2011  Garrick Brian Meeker, Richard Michael Powell

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

#ifndef _PRINT_UI_H_
#define _PRINT_UI_H_

#include <wx/wx.h>
#include <wx/dialog.h>

class ShowPrintDialog;
class CC_show;

class ShowPrintDialog : public wxDialog
{
	DECLARE_CLASS( ShowPrintDialog )
	DECLARE_EVENT_TABLE()

	public:
	ShowPrintDialog( );
	ShowPrintDialog(CC_show *show, bool printEPS,
		wxFrame *parent, wxWindowID id = wxID_ANY, const wxString& caption = wxT("Print Dialog"),
		const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );
	~ShowPrintDialog( );

	void Init();

	bool Create(CC_show *show, bool printEPS,
		wxFrame *parent, wxWindowID id = wxID_ANY, const wxString& caption = wxT("Print Dialog"),
		const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

	void CreateControls();

	void ShowPrintSelect(wxCommandEvent&);
	// because we modify setting, we need some way to reset them
	void ResetDefaults(wxCommandEvent&);

	// use these to get and set default values
	virtual bool TransferDataToWindow();
	virtual bool TransferDataFromWindow();

// to print a show, call this function
	void PrintShow();

private:
	CC_show *mShow;
	bool eps;
	wxTextCtrl *text_cmd, *text_opts, *text_view_cmd, *text_view_opts;
	wxTextCtrl *text_x, *text_y, *text_width, *text_height;
	wxTextCtrl *text_minyards;
	wxRadioBox *radio_orient, *radio_method;
	wxCheckBox *check_cont, *check_pages, *check_overview;

};
#endif
