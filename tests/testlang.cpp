/*
 * Smart Common Input Method
 * 
 * Copyright (c) 2004 James Su <suzhe@tsinghua.org.cn>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 *
 * $Id: testlang.cpp,v 1.2.4.1 2006/09/24 16:00:52 suzhe Exp $
 *
 */

#include "scim.h"

using namespace scim;

int main ()
{
	String locale;
	String language;

	locale = scim_get_current_locale ();
	language = scim_get_locale_language (locale);

	std::cout << "Current locale:   " << locale << "\n";
	std::cout << "Current encoding: " << scim_get_locale_encoding (locale) << "\n";
	std::cout << "Current language: " << language << " (" << scim_get_language_name (language) << ")\n";
	std::cout << "Related locales:  " << scim_get_language_locales (language) << "\n";
	std::cout << "Normalized language of " << language.substr (0,2) << " is " << scim_get_normalized_language (language.substr (0,2)) << "\n";
}
