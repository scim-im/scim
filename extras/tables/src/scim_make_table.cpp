/*
 * Smart Common Input Method
 * 
 * Copyright (c) 2002-2005 James Su <suzhe@tsinghua.org.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: scim_make_table.cpp,v 1.2 2005/10/26 07:53:53 suzhe Exp $
 *
 */

#define Uses_STL_FUNCTIONAL
#define Uses_STL_VECTOR
#define Uses_STL_IOSTREAM
#define Uses_STL_FSTREAM
#define Uses_STL_ALGORITHM
#define Uses_STL_MAP
#define Uses_STL_UTILITY
#define Uses_STL_IOMANIP
#define Uses_C_STDIO
#define Uses_C_WCTYPE
#define Uses_C_LOCALE
#define Uses_SCIM_UTILITY

#include <scim.h>
#include "scim_generic_table.h"
#include "scim_table_private.h"

using namespace scim;

void self_learn (GenericTableLibrary &lib, const char *file)
{
    std::vector <ucs4_t> buffer;
    std::ifstream ifs(file);

    if (!ifs) return;

    uint32 byte = 0;
    uint32 kb = 0;

    WideString str;

    std::vector <uint32> phrases;
    std::vector <uint32>::const_iterator pit;

    uint32 maxlen = lib.get_max_phrase_length ();

    ucs4_t wc;
    bool skip;
    char wheel [] = {'-', '\\', '|', '/', 0};
    int wheel_state;

    buffer.reserve (1048576*32);

    skip = false;

    wheel_state = 0;

    while (!ifs.eof()) {
        buffer.clear ();
        // Read a line
        while (!ifs.eof ()) {
            if ((wc = utf8_read_wchar (ifs)) == 0) break;
            if (wc == L'\n') break;
            else if (iswpunct (wc) || iswspace (wc) || iswdigit (wc) ) {
                if (!skip) {
                    buffer.push_back (0);
                    skip = true;
                }
            } else {
                buffer.push_back (wc);
                skip = false;
            }
        }

        buffer.push_back (0);
        for (int i=0; i<buffer.size (); i++) {
            str = WideString ();
            for (int j=0; j<maxlen; j++) {
                if (buffer [j+i] == 0)
                    break;
                str.push_back (buffer [j+i]);

                phrases.clear ();
                if (lib.find_phrase (phrases, str)) {
                    for (pit = phrases.begin (); pit != phrases.end (); ++ pit) 
                        lib.set_phrase_frequency (*pit, lib.get_phrase_frequency (*pit) + 1);
                }
            }
            byte ++;
            if (byte == 1024) {
                byte = 0;
                kb ++;
                std::cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b";
                std::cout << kb << "\tK ("
                     << wheel [wheel_state/2] << ") " << std::flush;
                wheel_state = (wheel_state+1) % 8;
            }
        }
    }

    std::cout << std::endl;
}

int main (int argc, char * argv [])
{
    bool binary = false;

    bool no_save = false;

    char *output = NULL;

    char *corpus = NULL;

    char *output_freq = NULL;

    char *input_freq = NULL;

    GenericTableLibrary phrase_lib;

    bindtextdomain (GETTEXT_PACKAGE, SCIM_TABLE_LOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE,
        scim_get_locale_encoding (scim_get_current_locale ()).c_str ());

    if (argc < 2) {
        std::cerr    << _("Too few arguments!\n"
                "Usage:\n"
                "  scim-make-table <table_file> [options]\n\n"
                "  table_file\tthe table file for table module\n"
                "  -b\t\tconvert to binary format, otherwise to text format\n"
                "  -o output\tsave new table to file output\n"
                "  -no\t\tdo not save new phrase table\n"
                "  -if ifreq\tload phrase frequencies from this file\n"
                "  -of ofreq\tsave phrase frequencies to this file\n"
                "  -s file\tspecifiy the source file to count phrase ages.\n");
        return -1;
    }

    int i = 1;
    while (i<argc) {
        if (++i >= argc) break;

        if (String ("-b") == argv [i]) {
            binary = true;
            continue;
        }

        if (String ("-no") == argv [i]) {
            if (output != NULL) {
                std::cerr << _("option -no cannot be used with -o\n");
                return -1;
            }
            no_save = true;
            continue;
        }

        if (String ("-o") == argv [i]) {
            if (no_save) {
                std::cerr << _("option -o cannot be used with -no\n");
                return -1;
            }
            if (++i >= argc) {
                std::cerr << _("No argument for option ") << argv [i-1] << std::endl;
                return -1;
            }
            output = argv [i];
            continue;
        }

        if (String ("-if") == argv [i]) {
            if (++i >= argc) {
                std::cerr << _("No argument for option ") << argv [i-1] << std::endl;
                return -1;
            }
            input_freq = argv [i];
            continue;
        }

        if (String ("-of") == argv [i]) {
            if (++i >= argc) {
                std::cerr << _("No argument for option ") << argv [i-1] << std::endl;
                return -1;
            }
            output_freq = argv [i];
            continue;
        }

        if (String ("-s") == argv [i]) {
            if (++i >= argc) {
                std::cerr << "No argument for option " << argv [i-1] << std::endl;
                return -1;
            }
            corpus = argv [i];
            continue;
        }

        std::cerr << _("Invalid option: ") << argv [i] << std::endl;
        return -1;
    };

    if (output == NULL) output = argv [1];

    std::cout << _("Loading table file ") << argv [1] << _(" ...\n");

    if (!phrase_lib.init (argv [1], "", (input_freq?input_freq:""))) {
        std::cerr << _("table file load failed!") << std::endl;
        return -1;
    }

    if (corpus != NULL) {
        std::cout << "Counting phrase frequency...\n";
        self_learn (phrase_lib, corpus);
    }

    if (output_freq != NULL) {
        std::cout << _("Saving frequency table file ") << output_freq << _(" ...\n");
        if (!phrase_lib.save ("", "", output_freq, binary)) 
            std::cerr << _("frequency table file load failed!") << std::endl;
    }

    if (!no_save && output) {
        std::cout << _("Saving table file ") << output << _(" ...\n");

        if (!phrase_lib.save (output, "", "", binary)) {
            std::cerr << _("Table file save failed!") << std::endl;
        }
    }

    return 0;
}
/*
vi:ts=4:nowrap:ai:expandtab
*/
