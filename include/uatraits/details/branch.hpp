// uatraits is a simple tool for user agent detection
// Copyright (C) 2011 Yandex <highpower@yandex-team.ru>

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#ifndef UATRAITS_DETAILS_BRANCH_HPP_INCLUDED
#define UATRAITS_DETAILS_BRANCH_HPP_INCLUDED

#include <list>
#include <iostream>
#include <algorithm>

#include "uatraits/config.hpp"
#include "uatraits/shared.hpp"
#include "uatraits/shared_ptr.hpp"
#include "uatraits/details/definition.hpp"
#include "uatraits/details/pcre_utils.hpp"

namespace uatraits { namespace details {

template <typename Traits>
class branch : public shared {

public:
	branch(char const *xpath);
	virtual ~branch();

	typedef branch<Traits> type;
	typedef definition<Traits> definition_type;
	
	void add_match(char const *pattern);
	void add_child(shared_ptr<type> const &child);
	void add_definition(shared_ptr<definition_type> const &value);
	void add_regex_match(char const *pattern);
	void detect(char const *begin, char const *end, Traits &traits) const;
	void checked_detect(char const *begin, char const *end, Traits &traits, std::ostream &out) const;
	virtual bool matched(char const *begin, char const *end) const;

private:
	branch(branch const &);
	branch& operator = (branch const &);

	typedef shared_ptr<type> pointer;
	typedef shared_ptr<definition_type> definition_pointer;
	typedef std::pair<pcre*, pcre_extra*> regex_data;

private:
	std::string xpath_;
	std::list<pointer> children_;
	std::list<definition_pointer> definitions_;
	std::list<regex_data> regex_matches_;
	std::list<std::string> string_matches_;
};

template <typename Traits> 
class root_branch : public branch<Traits> {

public:
	root_branch();
	virtual bool matched(char const *begin, char const *end) const;
};

template <typename Traits> inline
branch<Traits>::branch(char const *xpath) :
    xpath_(xpath)
{
}

template <typename Traits> inline 
branch<Traits>::~branch() {
	for (std::list<regex_data>::iterator i = regex_matches_.begin(), end = regex_matches_.end(); i != end; ++i) {
		pcre_free_regex(*i);
	}
}

template <typename Traits> inline void
branch<Traits>::add_match(char const *pattern) {
	string_matches_.push_back(std::string(pattern));
}

template <typename Traits> inline void
branch<Traits>::add_child(shared_ptr<type> const &child) {
	children_.push_back(child);
}

template <typename Traits> inline void
branch<Traits>::add_definition(shared_ptr<definition_type> const &value) {
	definitions_.push_back(value);
}

template <typename Traits> inline void
branch<Traits>::add_regex_match(char const *pattern) {
	regex_matches_.push_back(pcre_compile_regex(pattern));
}

template <typename Traits> inline void
branch<Traits>::detect(char const *begin, char const *end, Traits &traits) const {
	if (matched(begin, end)) {
		for (typename std::list<definition_pointer>::const_iterator i = definitions_.begin(), list_end = definitions_.end(); i != list_end; ++i) {
			(*i)->detect(begin, end, traits);
		}
		for (typename std::list<pointer>::const_iterator i = children_.begin(), list_end = children_.end(); i != list_end; ++i) {
			(*i)->detect(begin, end, traits);
		}
	}
}

template <typename Traits> inline void
branch<Traits>::checked_detect(char const *begin, char const *end, Traits &traits, std::ostream &out) const {
	if (matched(begin, end)) {
	    out << "branch at [" << xpath_ << "] triggered" << std::endl;
		for (typename std::list<definition_pointer>::const_iterator i = definitions_.begin(), list_end = definitions_.end(); i != list_end; ++i) {
			(*i)->checked_detect(begin, end, traits, out);
		}
		for (typename std::list<pointer>::const_iterator i = children_.begin(), list_end = children_.end(); i != list_end; ++i) {
			(*i)->checked_detect(begin, end, traits, out);
		}
	}
}

template <typename Traits> inline bool
branch<Traits>::matched(char const *begin, char const *end) const {
	for (std::list<std::string>::const_iterator i = string_matches_.begin(), list_end = string_matches_.end(); i != list_end; ++i) {
		if (std::search(begin, end, i->begin(), i->end()) != end) {
			return true;
		}
	}
	for (std::list<regex_data>::const_iterator i = regex_matches_.begin(), list_end = regex_matches_.end(); i != list_end; ++i) {
		if (0 == pcre_exec(i->first, i->second, begin, end - begin, 0, 0, 0, 0)) {
		 	return true;
		}
	}
	return false;
}

template <typename Traits> inline
root_branch<Traits>::root_branch() :
    branch<Traits>("")
{
}

template <typename Traits> inline bool
root_branch<Traits>::matched(char const *begin, char const *end) const {
	(void) begin; (void) end;
	return true;
}


}} // namespaces

#endif // UATRAITS_DETAILS_BRANCH_HPP_INCLUDED
