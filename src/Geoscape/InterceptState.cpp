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
#include "InterceptState.h"
#include <sstream>
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Mod/AlienRace.h"
#include "../Mod/RuleStartingCondition.h"
#include "../Mod/RuleInterface.h"
#include "../Engine/LocalizedText.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Savegame/Base.h"
#include "../Savegame/Craft.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Ufo.h"
#include "../Savegame/MissionSite.h"
#include "../Savegame/AlienBase.h"
#include "../Engine/Options.h"
#include "Globe.h"
#include "SelectDestinationState.h"
#include "ConfirmDestinationState.h"
#include "../Basescape/BasescapeState.h"
#include "../Ufopaedia/Ufopaedia.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Intercept window.
 * @param game Pointer to the core game.
 * @param globe Pointer to the Geoscape globe.
 * @param base Pointer to base to show contained crafts (NULL to show all crafts).
 * @param target Pointer to target to intercept (NULL to ask user for target).
 */
InterceptState::InterceptState(Globe *globe, Base *base, Target *target) : _globe(globe), _base(base), _target(target)
{
	const int WIDTH_RANGE_STATUS = 8;
	const int WIDTH_CRAFT = 72;
	const int WIDTH_STATUS = 94;
	const int WIDTH_BASE = 74;
	const int WIDTH_WEAPONS = 48;
    int targetColumnOffset = 0;
	_screen = false;
    if(_target != 0 && Options::interceptorRangeStatus)
    {
        targetColumnOffset = 8;
    }

	// Create objects
	_window = new Window(this, 320, 140, 0, 30, POPUP_HORIZONTAL);
	_btnCancel = new TextButton(_base ? 142 : 288, 16, 16, 146);
	_btnGotoBase = new TextButton(142, 16, 162, 146);
	_txtTitle = new Text(300, 17, 10, 46);
	int x = 14+targetColumnOffset;
	_txtCraft = new Text(WIDTH_CRAFT, 9, x, 70);
	x += WIDTH_CRAFT;
	_txtStatus = new Text(WIDTH_STATUS, 9, x, 70);
	x += WIDTH_STATUS;
	_txtBase = new Text(WIDTH_BASE, 9, x, 70);
	x += WIDTH_BASE;
	_txtWeapons = new Text(WIDTH_WEAPONS+4, 17, x-4, 62);
	_lstCrafts = new TextList(290+targetColumnOffset, 64, 12, 78);

	// Set palette
	setInterface("intercept");

	add(_window, "window", "intercept");
	add(_btnCancel, "button", "intercept");
	add(_btnGotoBase, "button", "intercept");
	add(_txtTitle, "text1", "intercept");
	add(_txtCraft, "text2", "intercept");
	add(_txtStatus, "text2", "intercept");
	add(_txtBase, "text2", "intercept");
	add(_txtWeapons, "text2", "intercept");
	add(_lstCrafts, "list", "intercept");

	centerAllSurfaces();

	// Set up objects
	_window->setBackground(_game->getMod()->getSurface("BACK12.SCR"));

	_btnCancel->setText(tr("STR_CANCEL"));
	_btnCancel->onMouseClick((ActionHandler)&InterceptState::btnCancelClick);
	_btnCancel->onKeyboardPress((ActionHandler)&InterceptState::btnCancelClick, Options::keyCancel);
	_btnCancel->onKeyboardPress((ActionHandler)&InterceptState::btnCancelClick, Options::keyGeoIntercept);

	_btnGotoBase->setText(tr("STR_GO_TO_BASE"));
	_btnGotoBase->onMouseClick((ActionHandler)&InterceptState::btnGotoBaseClick);
	_btnGotoBase->setVisible(_base != 0);

	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setBig();
	_txtTitle->setText(tr("STR_LAUNCH_INTERCEPTION"));

	_txtCraft->setText(tr("STR_CRAFT"));

	_txtStatus->setText(tr("STR_STATUS"));

	_txtBase->setText(tr("STR_BASE"));

	_txtWeapons->setAlign(ALIGN_RIGHT);
	_txtWeapons->setText(tr("STR_WEAPONS_CREW_HWPS"));

    if(_target != 0 && Options::interceptorRangeStatus)
    {
        _lstCrafts->setColumns(5,WIDTH_RANGE_STATUS, WIDTH_CRAFT, WIDTH_STATUS, WIDTH_BASE, WIDTH_WEAPONS);
        _lstCrafts->setAlign(ALIGN_RIGHT, 4);
    }
    else
    {
        _lstCrafts->setColumns(4, WIDTH_CRAFT, WIDTH_STATUS, WIDTH_BASE, WIDTH_WEAPONS);
        _lstCrafts->setAlign(ALIGN_RIGHT, 3);
    }

	_lstCrafts->setSelectable(true);
	_lstCrafts->setBackground(_window);
	_lstCrafts->setMargin(2);
	_lstCrafts->onMouseClick((ActionHandler)&InterceptState::lstCraftsLeftClick);
	_lstCrafts->onMouseClick((ActionHandler)&InterceptState::lstCraftsRightClick, SDL_BUTTON_RIGHT);
	_lstCrafts->onMouseClick((ActionHandler)&InterceptState::lstCraftsMiddleClick, SDL_BUTTON_MIDDLE);

	int row = 0;
	for (std::vector<Base*>::iterator i = _game->getSavedGame()->getBases()->begin(); i != _game->getSavedGame()->getBases()->end(); ++i)
	{
		if (_base != 0 && (*i) != _base)
			continue;
		for (std::vector<Craft*>::iterator j = (*i)->getCrafts()->begin(); j != (*i)->getCrafts()->end(); ++j)
		{
			std::wostringstream ssStatus;
			std::string status = (*j)->getStatus();

			bool hasEnoughPilots = (*j)->arePilotsOnboard();
			if (status == "STR_OUT")
			{
				// QoL: let's give the player a bit more info
				if ((*j)->getDestination() == 0 || (*j)->getIsAutoPatrolling())
				{
					ssStatus << tr("STR_PATROLLING");
				}
				else if ((*j)->getLowFuel() || (*j)->getMissionComplete() || (*j)->getDestination() == (Target*)(*j)->getBase())
				{
					ssStatus << tr("STR_RETURNING");
					//ssStatus << tr("STR_RETURNING_TO_BASE"); // vanilla craft info
				}
				else
				{
					Ufo *u = dynamic_cast<Ufo*>((*j)->getDestination());
					MissionSite *m = dynamic_cast<MissionSite*>((*j)->getDestination());
					AlienBase *b = dynamic_cast<AlienBase*>((*j)->getDestination());
					Craft *x = dynamic_cast<Craft*>((*j)->getDestination());
					if (u != 0)
					{
						if ((*j)->isInDogfight())
						{
							ssStatus << tr("STR_TAILING_UFO");
						}
						else if (u->getStatus() == Ufo::FLYING)
						{
							ssStatus << tr("STR_INTERCEPTING");
							//ssStatus << tr("STR_INTERCEPTING_UFO").arg(u->getId()); // vanilla craft info
						}
						else
						{
							ssStatus << tr("STR_EN_ROUTE");
						}
					}
					else if (x != 0)
					{
						ssStatus << tr("STR_ESCORTING");
					}
					else if (m != 0 || b != 0)
					{
						ssStatus << tr("STR_EN_ROUTE");
					}
					else
					{
						ssStatus << tr(status); // "STR_OUT"
					}
				}
			}
			else
			{
				if (!hasEnoughPilots && status == "STR_READY")
				{
					ssStatus << tr("STR_PILOT_MISSING");
				}
				else
				{
					ssStatus << tr(status);
				}
			}
			if (status != "STR_READY" && status != "STR_OUT")
			{
				unsigned int maintenanceHours = 0;

				maintenanceHours += (*j)->calcRepairTime();
				maintenanceHours += (*j)->calcRefuelTime();
				maintenanceHours += (*j)->calcRearmTime();

				int days = maintenanceHours / 24;
				int hours = maintenanceHours % 24;
				ssStatus << L" (";
				if (days > 0)
				{
					ssStatus << tr("STR_DAY_SHORT").arg(days);
				}
				if (hours > 0)
				{
					if (days > 0)
					{
						ssStatus << L"/";
					}
					ssStatus << tr("STR_HOUR_SHORT").arg(hours);
				}
				ssStatus << L")";
			}

			std::wostringstream ss;
			if ((*j)->getNumWeapons() > 0)
			{
				ss << L'\x01' << (*j)->getNumWeapons() << L'\x01';
			}
			else
			{
				ss << 0;
			}
			ss << "/";
			if ((*j)->getNumSoldiers() > 0)
			{
				ss << L'\x01' << (*j)->getNumSoldiers() << L'\x01';
			}
			else
			{
				ss << 0;
			}
			ss << "/";
			if ((*j)->getNumVehicles() > 0)
			{
				ss << L'\x01' << (*j)->getNumVehicles() << L'\x01';
			}
			else
			{
				ss << 0;
			}
			_crafts.push_back(*j);

            if(_target != 0 && Options::interceptorRangeStatus)
            {
                _lstCrafts->addRow(5," ",(*j)->getName(_game->getLanguage()).c_str(), ssStatus.str().c_str(), (*i)->getName().c_str(), ss.str().c_str());
            }
            else
            {
                _lstCrafts->addRow(4, (*j)->getName(_game->getLanguage()).c_str(), ssStatus.str().c_str(), (*i)->getName().c_str(), ss.str().c_str());
            }

			if (hasEnoughPilots && status == "STR_READY")
			{
                //handle ranges here
                if(_target != 0 && Options::interceptorRangeStatus)
                {
                    setInterceptorRangeStatus(row, true, (*j));
                    _lstCrafts->setCellColor(row, 2, _lstCrafts->getSecondaryColor());

                }
                else
                {
                    _lstCrafts->setCellColor(row, 1, _lstCrafts->getSecondaryColor());
                }
			}
            else
            {
                if(_target != 0 && Options::interceptorRangeStatus)
                {
                    setInterceptorRangeStatus(row, false, (*j));
                }
            }
			row++;
		}
	}
}

/**
 *
 */
InterceptState::~InterceptState()
{

}

/**
 * Determine if the craft is allowed to go to the
 * target destination.  Liberally stolen from
 * ConfirmDestinationState - used to show if
 * interceptor can go there
 * @param pointer to craft
 */
bool InterceptState::isCraftAllowed(Craft *craft)
{
	Ufo* u = dynamic_cast<Ufo*>(_target);
	MissionSite* m = dynamic_cast<MissionSite*>(_target);
	AlienBase* b = dynamic_cast<AlienBase*>(_target);

	AlienDeployment *ruleDeploy = 0;
	if (u != 0)
	{
		ruleDeploy = _game->getMod()->getDeployment(u->getRules()->getType());
	}
	else if (m != 0)
	{
		ruleDeploy = _game->getMod()->getDeployment(m->getDeployment()->getType());
	}
	else if (b != 0)
	{
		AlienRace *race = _game->getMod()->getAlienRace(b->getAlienRace());
		ruleDeploy = _game->getMod()->getDeployment(race->getBaseCustomMission());
		if (!ruleDeploy) ruleDeploy = _game->getMod()->getDeployment(b->getDeployment()->getType());
	}
	else
	{
		// for example just a waypoint
		return true;
	}

	if (ruleDeploy == 0)
	{
		// e.g. UFOs without alien deployment :(
		return true;
	}

	RuleStartingCondition *rule = _game->getMod()->getStartingCondition(ruleDeploy->getStartingCondition());
	if (rule == 0)
	{
		// rule doesn't exist (mod upgrades?)
		return true;
	}

	if (rule->isCraftAllowed(craft->getRules()->getType()))
	{
		// craft is allowed
		return true;
	}
    return false;
}

/**
 * Sets the intercetpor range status (color and shape) 
 * based on the status of craft and range to target
 * @param row of the list 
 * @param ready - is craft ready? 
 * @param Craft pointer 
 */
void InterceptState::setInterceptorRangeStatus(int row, bool ready, Craft *craft)
{
    if(_game->getMod()->getInterface("interceptorRangeStatusReady") == 0 ||
       _game->getMod()->getInterface("interceptorRangeStatusNotReady") == 0 )
    {
        return;
    }

    double targetDistance = _target->getDistance(craft) * 60.0 * (180.0 / M_PI);
    double maxFlightDistance = craft->getMaxFlightRange();

	RuleInterface *ruleInterface;
    if(ready)
    {
        ruleInterface = _game->getMod()->getInterface("interceptorRangeStatusReady");
    }
    else
    {
        ruleInterface = _game->getMod()->getInterface("interceptorRangeStatusNotReady");
    }

    if(ruleInterface)
    {
        Element *element;
        if(!isCraftAllowed(craft))
        {
            element = ruleInterface->getElement("craftNotAllowed");
        }
        else if(targetDistance > maxFlightDistance)
        {
            element = ruleInterface->getElement("outOfRange");
        }
        else if (targetDistance <= maxFlightDistance && targetDistance > maxFlightDistance * 0.9)
        {
            element = ruleInterface->getElement("mightBeInRange");
        }
        else
        {
            element = ruleInterface->getElement("inRange");
        }

        if(element)
        {
            wchar_t shape = wchar_t(element->textShape);
            std::wstring sShape(&shape);
            _lstCrafts->setCellText(row,0, std::wstring(sShape.c_str()));
            _lstCrafts->setCellColor(row, 0, element->color);
        }
    }

}

/**
 * Closes the window.
 * @param action Pointer to an action.
 */
void InterceptState::btnCancelClick(Action *)
{
	_game->popState();
}

/**
 * Goes to the base for the respective craft.
 * @param action Pointer to an action.
 */
void InterceptState::btnGotoBaseClick(Action *)
{
	_game->popState();
	_game->pushState(new BasescapeState(_base, _globe));
}

/**
 * Pick a target for the selected craft.
 * @param action Pointer to an action.
 */
void InterceptState::lstCraftsLeftClick(Action *)
{
	Craft* c = _crafts[_lstCrafts->getSelectedRow()];
	if (c->getStatus() == "STR_READY" || ((c->getStatus() == "STR_OUT" || Options::craftLaunchAlways) && !c->getLowFuel() && !c->getMissionComplete()))
	{
		_game->popState();
		if (_target == 0)
		{
			_game->pushState(new SelectDestinationState(c, _globe));
		}
		else
		{
			_game->pushState(new ConfirmDestinationState(c, _target));
		}
	}
}

/**
 * Centers on the selected craft.
 * @param action Pointer to an action.
 */
void InterceptState::lstCraftsRightClick(Action *)
{
	Craft* c = _crafts[_lstCrafts->getSelectedRow()];
	if (c->getStatus() == "STR_OUT")
	{
		_globe->center(c->getLongitude(), c->getLatitude());
		_game->popState();
	}
}

/**
* Opens the corresponding Ufopaedia article.
* @param action Pointer to an action.
*/
void InterceptState::lstCraftsMiddleClick(Action *)
{
	Craft* c = _crafts[_lstCrafts->getSelectedRow()];
	if (c)
	{
		std::string articleId = c->getRules()->getType();
		Ufopaedia::openArticle(_game, articleId);
	}
}

}
