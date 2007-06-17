#define Uses_SCIM_EVENT
#include <scim.h>
#include <iostream>

using namespace scim;

struct ComposeSequence
{
	uint32 keys [8];
	ucs4_t unicode;
};

class ComposeSequenceEqualByKeys
{
public:
    bool operator () (const ComposeSequence &lhs, const ComposeSequence &rhs) const {
        return  lhs.keys [0] == rhs.keys [0] &&
        	lhs.keys [1] == rhs.keys [1] &&
        	lhs.keys [2] == rhs.keys [2] &&
        	lhs.keys [3] == rhs.keys [3] &&
        	lhs.keys [4] == rhs.keys [4] &&
        	lhs.keys [5] == rhs.keys [5] &&
        	lhs.keys [6] == rhs.keys [6] &&
        	lhs.keys [7] == rhs.keys [7];
    }
};

class ComposeSequenceLessByKeys
{
public:
    bool operator () (const ComposeSequence &lhs, const ComposeSequence &rhs) const {
        if (lhs.keys [0] < rhs.keys [0]) return true;
        else if (lhs.keys [0] > rhs.keys [0]) return false;

        if (lhs.keys [1] < rhs.keys [1]) return true;
        else if (lhs.keys [1] > rhs.keys [1]) return false;

        if (lhs.keys [2] < rhs.keys [2]) return true;
        else if (lhs.keys [2] > rhs.keys [2]) return false;

        if (lhs.keys [3] < rhs.keys [3]) return true;
        else if (lhs.keys [3] > rhs.keys [3]) return false;

        if (lhs.keys [4] < rhs.keys [4]) return true;
        else if (lhs.keys [4] > rhs.keys [4]) return false;

        if (lhs.keys [5] < rhs.keys [5]) return true;
        else if (lhs.keys [5] > rhs.keys [5]) return false;

        if (lhs.keys [6] < rhs.keys [6]) return true;
        else if (lhs.keys [6] > rhs.keys [6]) return false;

        if (lhs.keys [7] < rhs.keys [7]) return true;

        return false;
    }

    bool operator () (const ComposeSequence &lhs, const uint16 *rhs) const {
        if (lhs.keys [0] < rhs [0]) return true;
        else if (lhs.keys [0] > rhs [0]) return false;

        if (lhs.keys [1] < rhs [1]) return true;
        else if (lhs.keys [1] > rhs [1]) return false;

        if (lhs.keys [2] < rhs [2]) return true;
        else if (lhs.keys [2] > rhs [2]) return false;

        if (lhs.keys [3] < rhs [3]) return true;
        else if (lhs.keys [3] > rhs [3]) return false;

        if (lhs.keys [4] < rhs [4]) return true;
        else if (lhs.keys [4] > rhs [4]) return false;

        if (lhs.keys [5] < rhs [5]) return true;
        else if (lhs.keys [5] > rhs [5]) return false;

        if (lhs.keys [6] < rhs [6]) return true;
        else if (lhs.keys [6] > rhs [6]) return false;

        if (lhs.keys [7] < rhs [7]) return true;

        return false;
    }

    bool operator () (const uint16 *lhs, const ComposeSequence &rhs) const {
        if (lhs [0] < rhs.keys [0]) return true;
        else if (lhs [0] > rhs.keys [0]) return false;

        if (lhs [1] < rhs.keys [1]) return true;
        else if (lhs [1] > rhs.keys [1]) return false;

        if (lhs [2] < rhs.keys [2]) return true;
        else if (lhs [2] > rhs.keys [2]) return false;

        if (lhs [3] < rhs.keys [3]) return true;
        else if (lhs [3] > rhs.keys [3]) return false;

        if (lhs [4] < rhs.keys [4]) return true;
        else if (lhs [4] > rhs.keys [4]) return false;

        if (lhs [5] < rhs.keys [5]) return true;
        else if (lhs [5] > rhs.keys [5]) return false;

        if (lhs [6] < rhs.keys [6]) return true;
        else if (lhs [6] > rhs.keys [6]) return false;

        if (lhs [7] < rhs.keys [7]) return true;

        return false;
    }

    bool operator () (const uint16 *lhs, const uint16 *rhs) const {
        if (lhs [0] < rhs [0]) return true;
        else if (lhs [0] > rhs [0]) return false;

        if (lhs [1] < rhs [1]) return true;
        else if (lhs [1] > rhs [1]) return false;

        if (lhs [2] < rhs [2]) return true;
        else if (lhs [2] > rhs [2]) return false;

        if (lhs [3] < rhs [3]) return true;
        else if (lhs [3] > rhs [3]) return false;

        if (lhs [4] < rhs [4]) return true;
        else if (lhs [4] > rhs [4]) return false;

        if (lhs [5] < rhs [5]) return true;
        else if (lhs [5] > rhs [5]) return false;

        if (lhs [6] < rhs [6]) return true;
        else if (lhs [6] > rhs [6]) return false;

        if (lhs [7] < rhs [7]) return true;

        return false;
    }

};

static const ComposeSequence __scim_compose_seqs[] = {
#include "scim_compose_key_data.h"
};

#define SCIM_NUM_COMPOSE_SEQS (sizeof (__scim_compose_seqs) / sizeof (__scim_compose_seqs [0]))

int main ()
{
#if 0
	std::vector <ComposeSequence> seqs (__scim_compose_seqs, __scim_compose_seqs + SCIM_NUM_COMPOSE_SEQS);

#else
	std::vector <ComposeSequence> seqs;

	char buf [1000];

	std::vector <String> strvec;
	std::vector <String> keyvec;
	std::vector <String> resvec;
	String keystr;

	ComposeSequence seq;
	KeyEvent key;

	while (!std::cin.eof ()) {
		std::cin.getline (buf, 1000);

		if (buf [0] == '#') continue;

		// split keys and result parts.
		if (scim_split_string_list (strvec, buf, ':') == 2) {
			// get the result unicode.
			if (scim_split_string_list (resvec, strvec [1], '"') != 3 ||
			    resvec [1][0] == '\\') {
				std::cerr << "Invalid entry: " << buf << "\n";
				continue;
			}

			utf8_mbtowc (& seq.unicode, (const unsigned char *)resvec [1].c_str (), resvec[1].length ());

			keystr = String ();
			for (String::iterator it = strvec [0].begin (); it != strvec [0].end (); ++it) {
				if (String ("<> \t").find (*it) == String::npos)
					keystr.push_back (*it);
				if (*it == '>')
					keystr.push_back (',');
			}

			scim_split_string_list (keyvec, keystr, ',');

			bool ok = true;
			for (int i = 0; i < 8; ++i) {
				if (i < keyvec.size ()) {
					if (!scim_string_to_key (key, keyvec [i])) {
						std::cerr << "Invalid key: " << keyvec [i] << "\n";
						ok = false;
						break;
					}
					seq.keys [i] = (uint32) key.code;
				} else {
					seq.keys [i] = 0;
				}
#if 0
				if (seq.keys [i] >= 0x01000000) {
					ok = false;
					break;
				}
#endif
			}

			if (ok) seqs.push_back (seq);
			else std::cerr << "Invalid entry: " << buf << "\n";
		} else {
			std::cerr << "Invalid entry: " << buf << "\n";
		}
	}
#endif

	std::sort (seqs.begin (), seqs.end (), ComposeSequenceLessByKeys ());

	seqs.erase (std::unique (seqs.begin (), seqs.end (), ComposeSequenceEqualByKeys ()), seqs.end ());

	int length [5] = { 0, 0, 0, 0, 0, };

	for (size_t i = 0; i < seqs.size (); ++i) {
		for (int j = 0; j < 5; ++j) {
			String str;
			if (seqs [i].keys [j]) {
				str = KeyEvent (seqs [i].keys [j], 0).get_key_string ();
				if (str [0] != '0' || str [1] != 'x') str = String ("SCIM_KEY_") + str;
			} else {
				str = "0";
			}
			if (length [j] < str.length ())
				length [j] = str.length ();
		}
	}

	for (size_t i = 0; i < seqs.size (); ++i) {
		std::cout << "  {{";
		for (int j = 0; j < 5; ++j) {
			String str;
			if (seqs [i].keys [j]) {
				str = KeyEvent (seqs [i].keys [j], 0).get_key_string ();
				if (str [0] != '0' || str [1] != 'x') str = String ("SCIM_KEY_") + str;
			} else {
				str = "0";
			}
			std::cout << str;
		        if (j < 4) std::cout << ",";
			for (int k = 0; k <= length [j] - str.length (); ++k)
				std::cout << " ";
		}
		std::cout << "}, ";

		char unic [8];
		snprintf (unic, 8, "0x%04X", seqs [i].unicode);
		std::cout << unic << "},\n";
	}
}

