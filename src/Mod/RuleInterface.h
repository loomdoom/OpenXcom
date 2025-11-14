#pragma once
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
#include <string>
#include <map>
#include "../Engine/Yaml.h"
#include "ModScript.h"

namespace OpenXcom
{

class Mod;
class SavedGame;

struct LayoutElementProperties
{
	private:
	std::unique_ptr<std::map<std::string, std::string,std::less<void>>> prps = {};

	public:
			void Add(std::string_view key, std::string_view value)
			{
					if (!prps)
					{
							prps.reset(new std::map<std::string, std::string,std::less<void>>());
					}
					prps->emplace(key, value);
			}

			const std::string_view Get(std::string_view key, std::string_view defaultValue = "") const
			{
					if (!prps)
					{
							return defaultValue;
					}
					auto it = prps->find(key);
					return it != prps->end() ? std::string_view(it->second) : defaultValue;
			}

			template<typename T>
			const T Get(std::string_view key, const T& defaultValue = T()) const
			{
				return
			}

			const bool Has(std::string_view key) const
			{
					if (!prps)
					{
							return false;
					}
					auto it = prps->find(key);
					return it != prps->end();
			}

			bool Empty() const
			{
				return !prps || prps->empty();
			}
};

struct LayoutElement
{
	std::string element;

	LayoutElementProperties content;

	int order = INT_MAX;

	ModScript::LayoutElementScripts::Container _layoutElementScripts;
	ScriptValues<LayoutElement> _scriptValues;
	/// Name of class used in script.
	static constexpr const char *ScriptName = "LayoutElement";
	/// Register all useful function used by script.
	static void ScriptRegister(ScriptParserBase* parser);

	template<typename Script>
	const typename Script::Container &getScript() const { return _layoutElementScripts.get<Script>(); }
};

struct Element
{
	/// basic rect info.
	int x = INT_MAX;
	int y = INT_MAX;
	int w = INT_MAX;
	int h = INT_MAX;

	/// 3 colors
	int color = INT_MAX;
	int color2 = INT_MAX;
	int border = INT_MAX;

	/// Custom value
	int custom = 0;

	/// defines inversion behaviour
	bool TFTDMode = false;
};

class RuleInterface
{
private:
	std::string _type;
	std::string _palette;
	std::string _parent;
	std::string _backgroundImage;
	std::string _altBackgroundImage;
	std::string _music;
	int _sound;
	std::vector< std::pair<std::string, std::string> > _upgBackgroundImage;

	std::map <std::string, Element> _elements;
	std::map <std::string, LayoutElement> _layoutElements;
public:
	/// Constructor.
	RuleInterface(const std::string & type);
	/// Destructor.
	~RuleInterface();
	/// Load from YAML.
	void load(const YAML::YamlNodeReader& reader, Mod *mod, const ModScript& parsers);
	/// Get an element. Can be `nullptr`.
	const Element *getElementOptional(const std::string &id) const;
	/// Get an element.
	const Element *getElement(const std::string &id) const;
	/// Get palette.
	const std::string &getPalette() const;
	/// Get parent interface rule.
	const std::string &getParent() const;
	/// Get background image.
	const std::string &getBackgroundImage(const Mod* mod, const SavedGame* save) const;
	/// Get alternative background image (for battlescape theme).
	const std::string &getAltBackgroundImage() const;
	/// Get music.
	const std::string &getMusic() const;
	/// Get sound.
	int getSound() const;
	/// Get layout elements
	const std::map <std::string, LayoutElement> &getLayoutElements() const;


};

}
