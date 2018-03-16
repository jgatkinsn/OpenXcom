/*
 * Copyright 2010-2016 OpenXcom Developers.
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
#include <sstream>
#include "GlobalResearchState.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Base.h"
#include "ResearchState.h"
#include "../Savegame/ResearchProject.h"
#include "../Mod/RuleResearch.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Research screen.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 */
GlobalResearchState::GlobalResearchState()  
{
	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_btnOk = new TextButton(148, 16, 164, 176);
	_txtTitle = new Text(310, 17, 5, 8);
	_txtProject = new Text(110, 17, 10, 44);
	_txtScientists = new Text(106, 17, 120, 44);
	_txtProgress = new Text(84, 9, 226, 44);
    //FIXME: Set location (may have to shift those above to fit the string)
	_txtBase = new Text(84, 9, 226, 44);
	_lstResearch = new TextList(288, 112, 8, 62);


	// Set palette
	setInterface("ResearchMenu");

	add(_window, "window", "ResearchMenu");
	add(_btnOk, "button", "researchMenu");
	add(_txtTitle, "text", "ResearchMenu");
	add(_txtProject, "text", "ResearchMenu");
	add(_txtScientists, "text", "ResearchMenu");
	add(_txtProgress, "text", "ResearchMenu");
	add(_txtBase, "text", "ResearchMenu");
	add(_lstResearch, "list", "ResearchMenu");

	centerAllSurfaces();

	// Set up objects
	_window->setBackground(_game->getMod()->getSurface("BACK05.SCR"));

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&GlobalResearchState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&GlobalResearchState::btnOkClick, Options::keyCancel);


	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr("STR_CURRENT_GLOBAL_RESEARCH"));

	_txtProject->setWordWrap(true);
	_txtProject->setText(tr("STR_RESEARCH_PROJECT"));

	_txtScientists->setWordWrap(true);
	_txtScientists->setText(tr("STR_SCIENTISTS_ALLOCATED_UC"));

	_txtProgress->setText(tr("STR_PROGRESS"));

	_txtBase->setText(tr("STR_BASE"));

    //JGA this is the list 
	_lstResearch->setColumns(4, 158, 58, 70, 70);
	_lstResearch->setSelectable(true);
	_lstResearch->setBackground(_window);
	_lstResearch->setMargin(2);
	_lstResearch->setWordWrap(true);
    //add the button click on project to open base research screen
	_lstResearch->onMouseClick((ActionHandler)&GlobalResearchState::onSelectBase);
	fillProjectList();
}

/**
 *
 */
GlobalResearchState::~GlobalResearchState()
{
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void GlobalResearchState::btnOkClick(Action *)
{
	_game->popState();
}

/**
 * Goes to the base's research screen when clicking its project
 * @param action Pointer to an action.
 */
void GlobalResearchState::onSelectBase(Action *)
{
//	_game->pushState(new ResearchState(&_bases[_lstResearch->getSelectedRow()]));
	_game->pushState(new ResearchState(_bases->at(_lstResearch->getSelectedRow()))); 
}

/**
 * Updates the research list
 * after going to other screens.
 */
void GlobalResearchState::init()
{
	State::init();
	fillProjectList();
}

/**
 * Fills the list with all the Bases Research Projects and progress. 
 */
void GlobalResearchState::fillProjectList()
{
    //store the list of bases (for mouse click)
    _bases = _game->getSavedGame()->getBases();
	_lstResearch->clearList();
    //grab all the bases than walk their projects
	for (std::vector<Base*>::iterator bIter = _bases->begin(); bIter != _bases->end(); ++bIter)
    {
        const std::vector<ResearchProject *> & baseProjects((*bIter)->getResearch());
        for (std::vector<ResearchProject *>::const_iterator iter = baseProjects.begin(); iter != baseProjects.end(); ++iter)
        {
            std::wostringstream sstr;
            sstr << (*iter)->getAssigned();
            const RuleResearch *r = (*iter)->getRules();

            std::wstring wstr = tr(r->getName());
            _lstResearch->addRow(4, wstr.c_str(), sstr.str().c_str(), tr((*iter)->getResearchProgress()).c_str(), 
                                 (*bIter)->getName(_game->getLanguage()).c_str());
        }
    }
}

}
