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
void RuleInterface::load(const YAML::YamlNodeReader& reader, Mod *mod)
{
	if (const auto& parent = reader["refNode"])
	{
		load(parent, mod);
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

		for (auto propertyReader  : elementReader.children())
		{
			auto k = propertyReader.key();
			if(k == "id")
			{
				continue;
			}
			else if(k == "size")
			{
				std::pair<std::string,std::string> values;
				propertyReader.tryReadVal(values);
				bool isWInteger = !values.first.empty() && std::all_of(values.first.begin(), values.first.end(), ::isdigit);
				bool isHInteger = !values.second.empty() && std::all_of(values.second.begin(), values.second.end(), ::isdigit);
				if (isWInteger && isHInteger)
				{
					//for backwards compatibility
					element.w = std::stoi(values.first);
					element.h = std::stoi(values.second);
				}

				element.properties.Add("size", propertyReader.val());
				element.properties.Add("w", values.first);
				element.properties.Add("h", values.second);
				continue;
			}
			else if (k == "pos") {
				std::pair<std::string,std::string> values;
				propertyReader.tryReadVal(values);
				bool isXInteger = !values.first.empty() && std::all_of(values.first.begin(), values.first.end(), ::isdigit);
				bool isYInteger = !values.second.empty() && std::all_of(values.second.begin(), values.second.end(), ::isdigit);
				if (isXInteger && isYInteger)
				{
					//for backwards compatibility
					element.x = std::stoi(values.first);
					element.y = std::stoi(values.second);
				}

				element.properties.Add("pos", propertyReader.val());
				element.properties.Add("x", values.first);
				element.properties.Add("y", values.second);
				continue;
			}
			else if (k == "color") {
				propertyReader.tryReadVal(element.color);
				continue;
			}
			else if (k == "color2") {
				propertyReader.tryReadVal(element.color2);
				continue;
			}
			else if (k == "border") {
				propertyReader.tryReadVal(element.border);
				continue;
			}
			else if (k == "custom") {
				propertyReader.tryReadVal(element.custom);
				continue;
			}
			else if (k == "TFTDMode") {
				propertyReader.tryReadVal(element.TFTDMode);
				continue;
			}
			else {
				auto v = propertyReader.val();
				element.properties.Add(k, v);
			}
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

const std::map <std::string, Element> &RuleInterface::getAllElements() const
{
	return _elements;
}

}
