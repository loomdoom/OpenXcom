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

#include "RuleInterface.h"
#include "Mod.h"
#include <climits>
#include "../Savegame/SavedGame.h"
#include "../Engine/ScriptBind.h"

namespace OpenXcom
{

/**
 * Creates a blank ruleset for a certain
 * type of interface, containing an index of elements that make it up.
 * @param type String defining the type.
 */
RuleInterface::RuleInterface(const std::string & type) : _type(type), _sound(-1)
{
}

RuleInterface::~RuleInterface()
{
}

/**
 * Loads the elements from a YAML file.
 * @param node YAML node.
 */
void RuleInterface::load(const YAML::YamlNodeReader& reader, Mod *mod, const ModScript& parsers)
{
	if (const auto& parent = reader["refNode"])
	{
		load(parent, mod, parsers);
	}

	reader.tryRead("palette", _palette);
	reader.tryRead("parent", _parent);
	reader.tryRead("backgroundImage", _backgroundImage);
	reader.tryRead("altBackgroundImage", _altBackgroundImage);
	reader.tryRead("upgBackgroundImage", _upgBackgroundImage);
	reader.tryRead("music", _music);
	mod->loadSoundOffset(_type, _sound, reader["sound"], "GEO.CAT");
	for (const auto& elementReader : reader["elements"].children())
	{
		Element& element = _elements[elementReader["id"].readVal<std::string>()];
		if (elementReader["size"])
		{
			std::pair<int, int> pos = elementReader["size"].readVal<std::pair<int, int> >();
			element.w = pos.first;
			element.h = pos.second;
		}
		if (elementReader["pos"])
		{
			std::pair<int, int> pos = elementReader["pos"].readVal<std::pair<int, int> >();
			element.x = pos.first;
			element.y = pos.second;
		}
		elementReader.tryRead("color", element.color);
		elementReader.tryRead("color2", element.color2);
		elementReader.tryRead("border", element.border);
		elementReader.tryRead("custom", element.custom);
		elementReader.tryRead("TFTDMode", element.TFTDMode);
	}

	if(const auto& layoutReader = reader["layout"] )
	{
		for (const auto& layoutElementReader : layoutReader["elements"].children())
		{
			auto id = layoutElementReader["id"].readVal<std::string>();
			LayoutElement& layoutElement = _layoutElements[id];

			layoutElement._scriptValues.load(reader, parsers.getShared());
			layoutElement._layoutElementScripts.load(id, layoutElementReader, parsers.layoutElementScripts);

			if(layoutElementReader["content"])
			{
				for (const auto& contentReader : layoutElementReader["content"].children())
				{
					auto k = contentReader.key();
					auto v = contentReader.val();
					layoutElement.content.Add(k, v);
				}
			}
			layoutElementReader.tryRead("element", layoutElement.element);
			layoutElementReader.tryRead("order", layoutElement.order);
		}
	}
}

/**
 * Retrieves info on an element
 * @param id String defining the element.
 */
const Element *RuleInterface::getElementOptional(const std::string &id) const
{
	auto i = _elements.find(id);
	if (_elements.end() != i) return &i->second; else return 0;
}

/**
 * Retrieves info on an element
 * @param id String defining the element.
 */
const Element *RuleInterface::getElement(const std::string &id) const
{
	auto i = getElementOptional(id);
	if (i == nullptr)
	{
		throw Exception("Missing interface Element '" + id + "' in '" + _type + "'");
	}
	return i;
}

const std::string &RuleInterface::getPalette() const
{
	return _palette;
}

const std::string &RuleInterface::getParent() const
{
	return _parent;
}

const std::string &RuleInterface::getBackgroundImage(const Mod* mod, const SavedGame* save) const
{
	if (save)
	{
		for (auto& pair : _upgBackgroundImage)
		{
			auto r = mod->getResearch(pair.first, false);
			if (r && save->isResearched(r))
			{
				return pair.second;
			}
		}
	}
	return _backgroundImage;
}

const std::string &RuleInterface::getAltBackgroundImage() const
{
	return _altBackgroundImage;
}

const std::string &RuleInterface::getMusic() const
{
	return _music;
}

int RuleInterface::getSound() const
{
	return _sound;
}

const std::map <std::string, LayoutElement> &RuleInterface::getLayoutElements() const
{
	return _layoutElements;
}


////////////////////////////////////////////////////////////
//					Script binding
////////////////////////////////////////////////////////////

namespace
{

std::string debugDisplayScript(const LayoutElement* c)
{
	if (c)
	{
		std::string s;
		s += LayoutElement::ScriptName;
		s += "(name: \"";
		// s += c->getRules()->getType();
		s += "\")";
		return s;
	}
	else
	{
		return "null";
	}
}

} // namespace

void LayoutElement::ScriptRegister(ScriptParserBase* parser)
{
	parser->registerPointerType<LayoutElement>();

	Bind<LayoutElement> c = { parser };

	// c.addRules<RuleCountry, &LayoutElement::getRules>("getRuleCountry");

	// c.add<&LayoutElement::getPact>("getPact", "Get if the country has signed an alien pact or not.");

	// c.add<&LayoutElement::getCurrentFunding>("getCurrentFunding", "Get the country's current funding.");
	// c.add<&LayoutElement::getCurrentActivityAlien>("getCurrentActivityAlien", "Get the country's current alien activity.");
	// c.add<&LayoutElement::getCurrentActivityXcom>("getCurrentActivityXcom", "Get the country's current xcom activity.");

	c.addScriptValue<&LayoutElement::_scriptValues>();
	c.addDebugDisplay<&debugDisplayScript>();

	// c.addCustomConst("SATISFACTION_ALIENPACT", 0);
	// c.addCustomConst("SATISFACTION_UNHAPPY", 1);
	// c.addCustomConst("SATISFACTION_SATISIFIED", 2);
	// c.addCustomConst("SATISFACTION_HAPPY", 3);
}

/**
 * Constructor of new month country script parser.
 * Called every new month for every country.
 */
ModScript::LayoutElementPositionParser::LayoutElementPositionParser(ScriptGlobal* shared, const std::string& name, Mod* mod) : ScriptParserEvents{ shared, name,
	"x", "y",
	"element","width","height","screenWidth","screenHeight" }
{
	BindBase b { this };

	b.addCustomPtr<const Mod>("rules", mod);
}

ModScript::LayoutElementSizeParser::LayoutElementSizeParser(ScriptGlobal* shared, const std::string& name, Mod* mod) : ScriptParserEvents{ shared, name,
	"w", "h",
	"element","screenWidth","screenHeight" }
{
	BindBase b { this };

	b.addCustomPtr<const Mod>("rules", mod);
}

ModScript::GeoscapeTimerScriptParser::GeoscapeTimerScriptParser(ScriptGlobal* shared, const std::string& name, Mod* mod) : ScriptParserEvents{ shared, name,
	"timePart"}
{
	BindBase b { this };

	b.addCustomPtr<const Mod>("rules", mod);
	b.addCustomConst("YEAR", 0);
	b.addCustomConst("MONTH", 1);
	b.addCustomConst("DAY", 2);
	b.addCustomConst("HOURS", 3);
	b.addCustomConst("MINUTES", 4);
	b.addCustomConst("SECONDS", 5);
	b.addCustomConst("WEEKDAY", 6);


}

} // namespace OpenXcom
