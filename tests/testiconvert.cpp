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
 * $Id: testiconvert.cpp,v 1.2 2005/01/05 13:41:13 suzhe Exp $
 *
 */

#define Uses_SCIM_ICONV

#include <cstring>
#include "scim.h"

static const char * utf8_strings [] = 
{
  "Hello World! 世界你好！アカハザカダハフカハサダフワエロイアカサダジフハワエハラカジハサダフ",
  "Hello World! 世界你好！世界你好！世界你好！世界你好！",
  "Hello World! 世界你好！世界你好！世界你好！",
  "Hello World! 世界你好！世界你好！",
  "Hello World! 世界你好！",
  "Japanese いあえらかそいだはふあ",
  NULL
};

int main ()
{
	scim::IConvert to_utf8 ("UTF-8");
	scim::IConvert to_gb18030 ("GB18030");
	scim::IConvert to_big5 ("BIG5");
	scim::IConvert to_eucjp ("EUC-JP");

	scim::String mbs, utf;
	scim::WideString wcs;

	const char **ptr = utf8_strings;
	while (*ptr) {
		std::cout << "Convert: " << *ptr << "\n";

		if (to_utf8.convert (wcs, *ptr, std::strlen (*ptr))) {
			std::cout << "--> UTF-8 OK! ";
			if (to_gb18030.convert (mbs, wcs) && to_gb18030.convert (wcs, mbs)) {
				std::cout << "--> GB18030 OK!\n";
				std::cout << "  GB18030: " << mbs << "\n";
			} else {
				std::cout << "--> GB18030 Failed!\n";
			}
			if (to_big5.convert (mbs, wcs) && to_big5.convert (wcs, mbs)) {
				std::cout << "--> BIG5 OK!\n";
				std::cout << "  BIG5: " << mbs << "\n";
			} else {
				std::cout << "--> BIG5 Failed!\n";
			}
			if (to_eucjp.convert (mbs, wcs) && to_eucjp.convert (wcs, mbs)) {
				std::cout << "--> EUC-JP OK!\n";
				std::cout << "  EUC-JP: " << mbs << "\n";
			} else {
				std::cout << "--> EUC-JP Failed!\n";
			}
		} else {
			std::cout << "--> UTF-8 Failed!\n";
		}
		if (to_utf8.test_convert (*ptr, std::strlen (*ptr))) {
			std::cout << "Test UTF-8 OK!\n";
		} else {
			std::cout << "Test UTF-8 Failed!\n";
		}
		++ptr;
	}
}
