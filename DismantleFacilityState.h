/*
 * Copyright 2010 Daniel Albano
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef OPENXCOM__DISMANTLEFACILITYSTATE_H
#define OPENXCOM__DISMANTLEFACILITYSTATE_H

#include "State_Interactive.h"
#include "LangString.h"
#include "Palette.h"
#include "TextButton.h"
#include "Window.h"
#include "Text.h"
#include "Base.h"
#include "BaseFacility.h"

/**
 * Window shown when the player tries to
 * dismantle a facility.
 */
class DismantleFacilityState : public State
{
private:
	Base *_base;
	BaseFacility *_fac;

	TextButton *_btnOk, *_btnCancel;
	Window *_window;
	Text *_txtTitle, *_txtFacility;
public:
	/// Creates the Dismantle Facility state.
	DismantleFacilityState(Game *game, Base *base, BaseFacility *fac);
	/// Cleans up the Dismantle Facility state.
	~DismantleFacilityState();
	/// Handler for clicking the OK button.
	void btnOkClick(SDL_Event *ev, int scale);
	/// Handler for clicking the Cancel button.
	void btnCancelClick(SDL_Event *ev, int scale);
};

#endif
