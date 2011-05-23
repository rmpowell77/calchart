/*
 * color_select_ui.h
 * Dialox box for selecting colors
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

#ifndef _COLOR_SELECT_UI_H_
#define _COLOR_SELECT_UI_H_

#include <wx/wx.h>
#include <wx/bmpcbox.h>
#include <wx/spinctrl.h>
#include <wx/notebook.h>

class ColorSelectDialog : public wxPanel
{
	DECLARE_CLASS( ColorSelectDialog )
	DECLARE_EVENT_TABLE()

public:
	ColorSelectDialog( );
	ColorSelectDialog( wxWindow *parent,
		wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Color Select"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );
	~ColorSelectDialog( );

	void Init();

	bool Create(wxWindow *parent,
		wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Color Select"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

	void CreateControls();
private:
	void OnCmdSelectColors(wxCommandEvent&);
	void OnCmdSelectWidth(wxSpinEvent&);
	void OnCmdResetColors(wxCommandEvent&);
	void OnCmdResetAll(wxCommandEvent&);
	void OnCmdChooseNewColor(wxCommandEvent&);

	void SetColor(int selection, int width, const wxColour& color);
	wxBitmapComboBox* nameBox;
	wxSpinCtrl* spin;
};

class SetUpPrintingValues : public wxPanel
{
	DECLARE_CLASS( SetUpPrintingValues )
	DECLARE_EVENT_TABLE()

public:
	SetUpPrintingValues( );
	SetUpPrintingValues( wxWindow *parent,
		wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Printing Values"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );
	~SetUpPrintingValues( );

	void Init();

	bool Create(wxWindow *parent,
		wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("Printing Values"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

	void CreateControls();

	// use these to get and set default values
	virtual bool TransferDataToWindow();
	virtual bool TransferDataFromWindow();
private:
	void OnCmdReset(wxCommandEvent&);
};


class CalChartPreferences : public wxDialog
{
	DECLARE_CLASS( CalChartPreferences )
	DECLARE_EVENT_TABLE()

public:
	CalChartPreferences( );
	CalChartPreferences( wxWindow *parent,
		wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("CalChart Preferences"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );
	~CalChartPreferences( );

	void Init();

	bool Create(wxWindow *parent,
		wxWindowID id = wxID_ANY,
		const wxString& caption = wxT("CalChart Preferences"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

	void CreateControls();
};

#endif
