/* Thanks to Markus G. Kuhn <mkuhn@acm.org> for the ksysym<->Unicode
 * mapping functions, from the xterm sources.
 */

/* These tables could be compressed by contiguous ranges, but the benefit of doing so
 * is smallish. It would save about ~1000 bytes total.
 */
static __Uint16Pair __scim_key_to_unicode_tab [] = {
  { 0x01a1, 0x0104 }, /*                     Aogonek Ą LATIN CAPITAL LETTER A WITH OGONEK */
  { 0x01a2, 0x02d8 }, /*                       breve ˘ BREVE */
  { 0x01a3, 0x0141 }, /*                     Lstroke Ł LATIN CAPITAL LETTER L WITH STROKE */
  { 0x01a5, 0x013d }, /*                      Lcaron Ľ LATIN CAPITAL LETTER L WITH CARON */
  { 0x01a6, 0x015a }, /*                      Sacute Ś LATIN CAPITAL LETTER S WITH ACUTE */
  { 0x01a9, 0x0160 }, /*                      Scaron Š LATIN CAPITAL LETTER S WITH CARON */
  { 0x01aa, 0x015e }, /*                    Scedilla Ş LATIN CAPITAL LETTER S WITH CEDILLA */
  { 0x01ab, 0x0164 }, /*                      Tcaron Ť LATIN CAPITAL LETTER T WITH CARON */
  { 0x01ac, 0x0179 }, /*                      Zacute Ź LATIN CAPITAL LETTER Z WITH ACUTE */
  { 0x01ae, 0x017d }, /*                      Zcaron Ž LATIN CAPITAL LETTER Z WITH CARON */
  { 0x01af, 0x017b }, /*                   Zabovedot Ż LATIN CAPITAL LETTER Z WITH DOT ABOVE */
  { 0x01b1, 0x0105 }, /*                     aogonek ą LATIN SMALL LETTER A WITH OGONEK */
  { 0x01b2, 0x02db }, /*                      ogonek ˛ OGONEK */
  { 0x01b3, 0x0142 }, /*                     lstroke ł LATIN SMALL LETTER L WITH STROKE */
  { 0x01b5, 0x013e }, /*                      lcaron ľ LATIN SMALL LETTER L WITH CARON */
  { 0x01b6, 0x015b }, /*                      sacute ś LATIN SMALL LETTER S WITH ACUTE */
  { 0x01b7, 0x02c7 }, /*                       caron ˇ CARON */
  { 0x01b9, 0x0161 }, /*                      scaron š LATIN SMALL LETTER S WITH CARON */
  { 0x01ba, 0x015f }, /*                    scedilla ş LATIN SMALL LETTER S WITH CEDILLA */
  { 0x01bb, 0x0165 }, /*                      tcaron ť LATIN SMALL LETTER T WITH CARON */
  { 0x01bc, 0x017a }, /*                      zacute ź LATIN SMALL LETTER Z WITH ACUTE */
  { 0x01bd, 0x02dd }, /*                 doubleacute ˝ DOUBLE ACUTE ACCENT */
  { 0x01be, 0x017e }, /*                      zcaron ž LATIN SMALL LETTER Z WITH CARON */
  { 0x01bf, 0x017c }, /*                   zabovedot ż LATIN SMALL LETTER Z WITH DOT ABOVE */
  { 0x01c0, 0x0154 }, /*                      Racute Ŕ LATIN CAPITAL LETTER R WITH ACUTE */
  { 0x01c3, 0x0102 }, /*                      Abreve Ă LATIN CAPITAL LETTER A WITH BREVE */
  { 0x01c5, 0x0139 }, /*                      Lacute Ĺ LATIN CAPITAL LETTER L WITH ACUTE */
  { 0x01c6, 0x0106 }, /*                      Cacute Ć LATIN CAPITAL LETTER C WITH ACUTE */
  { 0x01c8, 0x010c }, /*                      Ccaron Č LATIN CAPITAL LETTER C WITH CARON */
  { 0x01ca, 0x0118 }, /*                     Eogonek Ę LATIN CAPITAL LETTER E WITH OGONEK */
  { 0x01cc, 0x011a }, /*                      Ecaron Ě LATIN CAPITAL LETTER E WITH CARON */
  { 0x01cf, 0x010e }, /*                      Dcaron Ď LATIN CAPITAL LETTER D WITH CARON */
  { 0x01d0, 0x0110 }, /*                     Dstroke Đ LATIN CAPITAL LETTER D WITH STROKE */
  { 0x01d1, 0x0143 }, /*                      Nacute Ń LATIN CAPITAL LETTER N WITH ACUTE */
  { 0x01d2, 0x0147 }, /*                      Ncaron Ň LATIN CAPITAL LETTER N WITH CARON */
  { 0x01d5, 0x0150 }, /*                Odoubleacute Ő LATIN CAPITAL LETTER O WITH DOUBLE ACUTE */
  { 0x01d8, 0x0158 }, /*                      Rcaron Ř LATIN CAPITAL LETTER R WITH CARON */
  { 0x01d9, 0x016e }, /*                       Uring Ů LATIN CAPITAL LETTER U WITH RING ABOVE */
  { 0x01db, 0x0170 }, /*                Udoubleacute Ű LATIN CAPITAL LETTER U WITH DOUBLE ACUTE */
  { 0x01de, 0x0162 }, /*                    Tcedilla Ţ LATIN CAPITAL LETTER T WITH CEDILLA */
  { 0x01e0, 0x0155 }, /*                      racute ŕ LATIN SMALL LETTER R WITH ACUTE */
  { 0x01e3, 0x0103 }, /*                      abreve ă LATIN SMALL LETTER A WITH BREVE */
  { 0x01e5, 0x013a }, /*                      lacute ĺ LATIN SMALL LETTER L WITH ACUTE */
  { 0x01e6, 0x0107 }, /*                      cacute ć LATIN SMALL LETTER C WITH ACUTE */
  { 0x01e8, 0x010d }, /*                      ccaron č LATIN SMALL LETTER C WITH CARON */
  { 0x01ea, 0x0119 }, /*                     eogonek ę LATIN SMALL LETTER E WITH OGONEK */
  { 0x01ec, 0x011b }, /*                      ecaron ě LATIN SMALL LETTER E WITH CARON */
  { 0x01ef, 0x010f }, /*                      dcaron ď LATIN SMALL LETTER D WITH CARON */
  { 0x01f0, 0x0111 }, /*                     dstroke đ LATIN SMALL LETTER D WITH STROKE */
  { 0x01f1, 0x0144 }, /*                      nacute ń LATIN SMALL LETTER N WITH ACUTE */
  { 0x01f2, 0x0148 }, /*                      ncaron ň LATIN SMALL LETTER N WITH CARON */
  { 0x01f5, 0x0151 }, /*                odoubleacute ő LATIN SMALL LETTER O WITH DOUBLE ACUTE */
  { 0x01f8, 0x0159 }, /*                      rcaron ř LATIN SMALL LETTER R WITH CARON */
  { 0x01f9, 0x016f }, /*                       uring ů LATIN SMALL LETTER U WITH RING ABOVE */
  { 0x01fb, 0x0171 }, /*                udoubleacute ű LATIN SMALL LETTER U WITH DOUBLE ACUTE */
  { 0x01fe, 0x0163 }, /*                    tcedilla ţ LATIN SMALL LETTER T WITH CEDILLA */
  { 0x01ff, 0x02d9 }, /*                    abovedot ˙ DOT ABOVE */
  { 0x02a1, 0x0126 }, /*                     Hstroke Ħ LATIN CAPITAL LETTER H WITH STROKE */
  { 0x02a6, 0x0124 }, /*                 Hcircumflex Ĥ LATIN CAPITAL LETTER H WITH CIRCUMFLEX */
  { 0x02a9, 0x0130 }, /*                   Iabovedot İ LATIN CAPITAL LETTER I WITH DOT ABOVE */
  { 0x02ab, 0x011e }, /*                      Gbreve Ğ LATIN CAPITAL LETTER G WITH BREVE */
  { 0x02ac, 0x0134 }, /*                 Jcircumflex Ĵ LATIN CAPITAL LETTER J WITH CIRCUMFLEX */
  { 0x02b1, 0x0127 }, /*                     hstroke ħ LATIN SMALL LETTER H WITH STROKE */
  { 0x02b6, 0x0125 }, /*                 hcircumflex ĥ LATIN SMALL LETTER H WITH CIRCUMFLEX */
  { 0x02b9, 0x0131 }, /*                    idotless ı LATIN SMALL LETTER DOTLESS I */
  { 0x02bb, 0x011f }, /*                      gbreve ğ LATIN SMALL LETTER G WITH BREVE */
  { 0x02bc, 0x0135 }, /*                 jcircumflex ĵ LATIN SMALL LETTER J WITH CIRCUMFLEX */
  { 0x02c5, 0x010a }, /*                   Cabovedot Ċ LATIN CAPITAL LETTER C WITH DOT ABOVE */
  { 0x02c6, 0x0108 }, /*                 Ccircumflex Ĉ LATIN CAPITAL LETTER C WITH CIRCUMFLEX */
  { 0x02d5, 0x0120 }, /*                   Gabovedot Ġ LATIN CAPITAL LETTER G WITH DOT ABOVE */
  { 0x02d8, 0x011c }, /*                 Gcircumflex Ĝ LATIN CAPITAL LETTER G WITH CIRCUMFLEX */
  { 0x02dd, 0x016c }, /*                      Ubreve Ŭ LATIN CAPITAL LETTER U WITH BREVE */
  { 0x02de, 0x015c }, /*                 Scircumflex Ŝ LATIN CAPITAL LETTER S WITH CIRCUMFLEX */
  { 0x02e5, 0x010b }, /*                   cabovedot ċ LATIN SMALL LETTER C WITH DOT ABOVE */
  { 0x02e6, 0x0109 }, /*                 ccircumflex ĉ LATIN SMALL LETTER C WITH CIRCUMFLEX */
  { 0x02f5, 0x0121 }, /*                   gabovedot ġ LATIN SMALL LETTER G WITH DOT ABOVE */
  { 0x02f8, 0x011d }, /*                 gcircumflex ĝ LATIN SMALL LETTER G WITH CIRCUMFLEX */
  { 0x02fd, 0x016d }, /*                      ubreve ŭ LATIN SMALL LETTER U WITH BREVE */
  { 0x02fe, 0x015d }, /*                 scircumflex ŝ LATIN SMALL LETTER S WITH CIRCUMFLEX */
  { 0x03a2, 0x0138 }, /*                         kra ĸ LATIN SMALL LETTER KRA */
  { 0x03a3, 0x0156 }, /*                    Rcedilla Ŗ LATIN CAPITAL LETTER R WITH CEDILLA */
  { 0x03a5, 0x0128 }, /*                      Itilde Ĩ LATIN CAPITAL LETTER I WITH TILDE */
  { 0x03a6, 0x013b }, /*                    Lcedilla Ļ LATIN CAPITAL LETTER L WITH CEDILLA */
  { 0x03aa, 0x0112 }, /*                     Emacron Ē LATIN CAPITAL LETTER E WITH MACRON */
  { 0x03ab, 0x0122 }, /*                    Gcedilla Ģ LATIN CAPITAL LETTER G WITH CEDILLA */
  { 0x03ac, 0x0166 }, /*                      Tslash Ŧ LATIN CAPITAL LETTER T WITH STROKE */
  { 0x03b3, 0x0157 }, /*                    rcedilla ŗ LATIN SMALL LETTER R WITH CEDILLA */
  { 0x03b5, 0x0129 }, /*                      itilde ĩ LATIN SMALL LETTER I WITH TILDE */
  { 0x03b6, 0x013c }, /*                    lcedilla ļ LATIN SMALL LETTER L WITH CEDILLA */
  { 0x03ba, 0x0113 }, /*                     emacron ē LATIN SMALL LETTER E WITH MACRON */
  { 0x03bb, 0x0123 }, /*                    gcedilla ģ LATIN SMALL LETTER G WITH CEDILLA */
  { 0x03bc, 0x0167 }, /*                      tslash ŧ LATIN SMALL LETTER T WITH STROKE */
  { 0x03bd, 0x014a }, /*                         ENG Ŋ LATIN CAPITAL LETTER ENG */
  { 0x03bf, 0x014b }, /*                         eng ŋ LATIN SMALL LETTER ENG */
  { 0x03c0, 0x0100 }, /*                     Amacron Ā LATIN CAPITAL LETTER A WITH MACRON */
  { 0x03c7, 0x012e }, /*                     Iogonek Į LATIN CAPITAL LETTER I WITH OGONEK */
  { 0x03cc, 0x0116 }, /*                   Eabovedot Ė LATIN CAPITAL LETTER E WITH DOT ABOVE */
  { 0x03cf, 0x012a }, /*                     Imacron Ī LATIN CAPITAL LETTER I WITH MACRON */
  { 0x03d1, 0x0145 }, /*                    Ncedilla Ņ LATIN CAPITAL LETTER N WITH CEDILLA */
  { 0x03d2, 0x014c }, /*                     Omacron Ō LATIN CAPITAL LETTER O WITH MACRON */
  { 0x03d3, 0x0136 }, /*                    Kcedilla Ķ LATIN CAPITAL LETTER K WITH CEDILLA */
  { 0x03d9, 0x0172 }, /*                     Uogonek Ų LATIN CAPITAL LETTER U WITH OGONEK */
  { 0x03dd, 0x0168 }, /*                      Utilde Ũ LATIN CAPITAL LETTER U WITH TILDE */
  { 0x03de, 0x016a }, /*                     Umacron Ū LATIN CAPITAL LETTER U WITH MACRON */
  { 0x03e0, 0x0101 }, /*                     amacron ā LATIN SMALL LETTER A WITH MACRON */
  { 0x03e7, 0x012f }, /*                     iogonek į LATIN SMALL LETTER I WITH OGONEK */
  { 0x03ec, 0x0117 }, /*                   eabovedot ė LATIN SMALL LETTER E WITH DOT ABOVE */
  { 0x03ef, 0x012b }, /*                     imacron ī LATIN SMALL LETTER I WITH MACRON */
  { 0x03f1, 0x0146 }, /*                    ncedilla ņ LATIN SMALL LETTER N WITH CEDILLA */
  { 0x03f2, 0x014d }, /*                     omacron ō LATIN SMALL LETTER O WITH MACRON */
  { 0x03f3, 0x0137 }, /*                    kcedilla ķ LATIN SMALL LETTER K WITH CEDILLA */
  { 0x03f9, 0x0173 }, /*                     uogonek ų LATIN SMALL LETTER U WITH OGONEK */
  { 0x03fd, 0x0169 }, /*                      utilde ũ LATIN SMALL LETTER U WITH TILDE */
  { 0x03fe, 0x016b }, /*                     umacron ū LATIN SMALL LETTER U WITH MACRON */
  { 0x047e, 0x203e }, /*                    overline ‾ OVERLINE */
  { 0x04a1, 0x3002 }, /*               kana_fullstop 。 IDEOGRAPHIC FULL STOP */
  { 0x04a2, 0x300c }, /*         kana_openingbracket 「 LEFT CORNER BRACKET */
  { 0x04a3, 0x300d }, /*         kana_closingbracket 」 RIGHT CORNER BRACKET */
  { 0x04a4, 0x3001 }, /*                  kana_comma 、 IDEOGRAPHIC COMMA */
  { 0x04a5, 0x30fb }, /*            kana_conjunctive ・ KATAKANA MIDDLE DOT */
  { 0x04a6, 0x30f2 }, /*                     kana_WO ヲ KATAKANA LETTER WO */
  { 0x04a7, 0x30a1 }, /*                      kana_a ァ KATAKANA LETTER SMALL A */
  { 0x04a8, 0x30a3 }, /*                      kana_i ィ KATAKANA LETTER SMALL I */
  { 0x04a9, 0x30a5 }, /*                      kana_u ゥ KATAKANA LETTER SMALL U */
  { 0x04aa, 0x30a7 }, /*                      kana_e ェ KATAKANA LETTER SMALL E */
  { 0x04ab, 0x30a9 }, /*                      kana_o ォ KATAKANA LETTER SMALL O */
  { 0x04ac, 0x30e3 }, /*                     kana_ya ャ KATAKANA LETTER SMALL YA */
  { 0x04ad, 0x30e5 }, /*                     kana_yu ュ KATAKANA LETTER SMALL YU */
  { 0x04ae, 0x30e7 }, /*                     kana_yo ョ KATAKANA LETTER SMALL YO */
  { 0x04af, 0x30c3 }, /*                    kana_tsu ッ KATAKANA LETTER SMALL TU */
  { 0x04b0, 0x30fc }, /*              prolongedsound ー KATAKANA-HIRAGANA PROLONGED SOUND MARK */
  { 0x04b1, 0x30a2 }, /*                      kana_A ア KATAKANA LETTER A */
  { 0x04b2, 0x30a4 }, /*                      kana_I イ KATAKANA LETTER I */
  { 0x04b3, 0x30a6 }, /*                      kana_U ウ KATAKANA LETTER U */
  { 0x04b4, 0x30a8 }, /*                      kana_E エ KATAKANA LETTER E */
  { 0x04b5, 0x30aa }, /*                      kana_O オ KATAKANA LETTER O */
  { 0x04b6, 0x30ab }, /*                     kana_KA カ KATAKANA LETTER KA */
  { 0x04b7, 0x30ad }, /*                     kana_KI キ KATAKANA LETTER KI */
  { 0x04b8, 0x30af }, /*                     kana_KU ク KATAKANA LETTER KU */
  { 0x04b9, 0x30b1 }, /*                     kana_KE ケ KATAKANA LETTER KE */
  { 0x04ba, 0x30b3 }, /*                     kana_KO コ KATAKANA LETTER KO */
  { 0x04bb, 0x30b5 }, /*                     kana_SA サ KATAKANA LETTER SA */
  { 0x04bc, 0x30b7 }, /*                    kana_SHI シ KATAKANA LETTER SI */
  { 0x04bd, 0x30b9 }, /*                     kana_SU ス KATAKANA LETTER SU */
  { 0x04be, 0x30bb }, /*                     kana_SE セ KATAKANA LETTER SE */
  { 0x04bf, 0x30bd }, /*                     kana_SO ソ KATAKANA LETTER SO */
  { 0x04c0, 0x30bf }, /*                     kana_TA タ KATAKANA LETTER TA */
  { 0x04c1, 0x30c1 }, /*                    kana_CHI チ KATAKANA LETTER TI */
  { 0x04c2, 0x30c4 }, /*                    kana_TSU ツ KATAKANA LETTER TU */
  { 0x04c3, 0x30c6 }, /*                     kana_TE テ KATAKANA LETTER TE */
  { 0x04c4, 0x30c8 }, /*                     kana_TO ト KATAKANA LETTER TO */
  { 0x04c5, 0x30ca }, /*                     kana_NA ナ KATAKANA LETTER NA */
  { 0x04c6, 0x30cb }, /*                     kana_NI ニ KATAKANA LETTER NI */
  { 0x04c7, 0x30cc }, /*                     kana_NU ヌ KATAKANA LETTER NU */
  { 0x04c8, 0x30cd }, /*                     kana_NE ネ KATAKANA LETTER NE */
  { 0x04c9, 0x30ce }, /*                     kana_NO ノ KATAKANA LETTER NO */
  { 0x04ca, 0x30cf }, /*                     kana_HA ハ KATAKANA LETTER HA */
  { 0x04cb, 0x30d2 }, /*                     kana_HI ヒ KATAKANA LETTER HI */
  { 0x04cc, 0x30d5 }, /*                     kana_FU フ KATAKANA LETTER HU */
  { 0x04cd, 0x30d8 }, /*                     kana_HE ヘ KATAKANA LETTER HE */
  { 0x04ce, 0x30db }, /*                     kana_HO ホ KATAKANA LETTER HO */
  { 0x04cf, 0x30de }, /*                     kana_MA マ KATAKANA LETTER MA */
  { 0x04d0, 0x30df }, /*                     kana_MI ミ KATAKANA LETTER MI */
  { 0x04d1, 0x30e0 }, /*                     kana_MU ム KATAKANA LETTER MU */
  { 0x04d2, 0x30e1 }, /*                     kana_ME メ KATAKANA LETTER ME */
  { 0x04d3, 0x30e2 }, /*                     kana_MO モ KATAKANA LETTER MO */
  { 0x04d4, 0x30e4 }, /*                     kana_YA ヤ KATAKANA LETTER YA */
  { 0x04d5, 0x30e6 }, /*                     kana_YU ユ KATAKANA LETTER YU */
  { 0x04d6, 0x30e8 }, /*                     kana_YO ヨ KATAKANA LETTER YO */
  { 0x04d7, 0x30e9 }, /*                     kana_RA ラ KATAKANA LETTER RA */
  { 0x04d8, 0x30ea }, /*                     kana_RI リ KATAKANA LETTER RI */
  { 0x04d9, 0x30eb }, /*                     kana_RU ル KATAKANA LETTER RU */
  { 0x04da, 0x30ec }, /*                     kana_RE レ KATAKANA LETTER RE */
  { 0x04db, 0x30ed }, /*                     kana_RO ロ KATAKANA LETTER RO */
  { 0x04dc, 0x30ef }, /*                     kana_WA ワ KATAKANA LETTER WA */
  { 0x04dd, 0x30f3 }, /*                      kana_N ン KATAKANA LETTER N */
  { 0x04de, 0x309b }, /*                 voicedsound ゛ KATAKANA-HIRAGANA VOICED SOUND MARK */
  { 0x04df, 0x309c }, /*             semivoicedsound ゜ KATAKANA-HIRAGANA SEMI-VOICED SOUND MARK */
  { 0x05ac, 0x060c }, /*                Arabic_comma ، ARABIC COMMA */
  { 0x05bb, 0x061b }, /*            Arabic_semicolon ؛ ARABIC SEMICOLON */
  { 0x05bf, 0x061f }, /*        Arabic_question_mark ؟ ARABIC QUESTION MARK */
  { 0x05c1, 0x0621 }, /*                Arabic_hamza ء ARABIC LETTER HAMZA */
  { 0x05c2, 0x0622 }, /*          Arabic_maddaonalef آ ARABIC LETTER ALEF WITH MADDA ABOVE */
  { 0x05c3, 0x0623 }, /*          Arabic_hamzaonalef أ ARABIC LETTER ALEF WITH HAMZA ABOVE */
  { 0x05c4, 0x0624 }, /*           Arabic_hamzaonwaw ؤ ARABIC LETTER WAW WITH HAMZA ABOVE */
  { 0x05c5, 0x0625 }, /*       Arabic_hamzaunderalef إ ARABIC LETTER ALEF WITH HAMZA BELOW */
  { 0x05c6, 0x0626 }, /*           Arabic_hamzaonyeh ئ ARABIC LETTER YEH WITH HAMZA ABOVE */
  { 0x05c7, 0x0627 }, /*                 Arabic_alef ا ARABIC LETTER ALEF */
  { 0x05c8, 0x0628 }, /*                  Arabic_beh ب ARABIC LETTER BEH */
  { 0x05c9, 0x0629 }, /*           Arabic_tehmarbuta ة ARABIC LETTER TEH MARBUTA */
  { 0x05ca, 0x062a }, /*                  Arabic_teh ت ARABIC LETTER TEH */
  { 0x05cb, 0x062b }, /*                 Arabic_theh ث ARABIC LETTER THEH */
  { 0x05cc, 0x062c }, /*                 Arabic_jeem ج ARABIC LETTER JEEM */
  { 0x05cd, 0x062d }, /*                  Arabic_hah ح ARABIC LETTER HAH */
  { 0x05ce, 0x062e }, /*                 Arabic_khah خ ARABIC LETTER KHAH */
  { 0x05cf, 0x062f }, /*                  Arabic_dal د ARABIC LETTER DAL */
  { 0x05d0, 0x0630 }, /*                 Arabic_thal ذ ARABIC LETTER THAL */
  { 0x05d1, 0x0631 }, /*                   Arabic_ra ر ARABIC LETTER REH */
  { 0x05d2, 0x0632 }, /*                 Arabic_zain ز ARABIC LETTER ZAIN */
  { 0x05d3, 0x0633 }, /*                 Arabic_seen س ARABIC LETTER SEEN */
  { 0x05d4, 0x0634 }, /*                Arabic_sheen ش ARABIC LETTER SHEEN */
  { 0x05d5, 0x0635 }, /*                  Arabic_sad ص ARABIC LETTER SAD */
  { 0x05d6, 0x0636 }, /*                  Arabic_dad ض ARABIC LETTER DAD */
  { 0x05d7, 0x0637 }, /*                  Arabic_tah ط ARABIC LETTER TAH */
  { 0x05d8, 0x0638 }, /*                  Arabic_zah ظ ARABIC LETTER ZAH */
  { 0x05d9, 0x0639 }, /*                  Arabic_ain ع ARABIC LETTER AIN */
  { 0x05da, 0x063a }, /*                Arabic_ghain غ ARABIC LETTER GHAIN */
  { 0x05e0, 0x0640 }, /*              Arabic_tatweel ـ ARABIC TATWEEL */
  { 0x05e1, 0x0641 }, /*                  Arabic_feh ف ARABIC LETTER FEH */
  { 0x05e2, 0x0642 }, /*                  Arabic_qaf ق ARABIC LETTER QAF */
  { 0x05e3, 0x0643 }, /*                  Arabic_kaf ك ARABIC LETTER KAF */
  { 0x05e4, 0x0644 }, /*                  Arabic_lam ل ARABIC LETTER LAM */
  { 0x05e5, 0x0645 }, /*                 Arabic_meem م ARABIC LETTER MEEM */
  { 0x05e6, 0x0646 }, /*                 Arabic_noon ن ARABIC LETTER NOON */
  { 0x05e7, 0x0647 }, /*                   Arabic_ha ه ARABIC LETTER HEH */
  { 0x05e8, 0x0648 }, /*                  Arabic_waw و ARABIC LETTER WAW */
  { 0x05e9, 0x0649 }, /*          Arabic_alefmaksura ى ARABIC LETTER ALEF MAKSURA */
  { 0x05ea, 0x064a }, /*                  Arabic_yeh ي ARABIC LETTER YEH */
  { 0x05eb, 0x064b }, /*             Arabic_fathatan ً ARABIC FATHATAN */
  { 0x05ec, 0x064c }, /*             Arabic_dammatan ٌ ARABIC DAMMATAN */
  { 0x05ed, 0x064d }, /*             Arabic_kasratan ٍ ARABIC KASRATAN */
  { 0x05ee, 0x064e }, /*                Arabic_fatha َ ARABIC FATHA */
  { 0x05ef, 0x064f }, /*                Arabic_damma ُ ARABIC DAMMA */
  { 0x05f0, 0x0650 }, /*                Arabic_kasra ِ ARABIC KASRA */
  { 0x05f1, 0x0651 }, /*               Arabic_shadda ّ ARABIC SHADDA */
  { 0x05f2, 0x0652 }, /*                Arabic_sukun ْ ARABIC SUKUN */
  { 0x06a1, 0x0452 }, /*                 Serbian_dje ђ CYRILLIC SMALL LETTER DJE */
  { 0x06a2, 0x0453 }, /*               Macedonia_gje ѓ CYRILLIC SMALL LETTER GJE */
  { 0x06a3, 0x0451 }, /*                 Cyrillic_io ё CYRILLIC SMALL LETTER IO */
  { 0x06a4, 0x0454 }, /*                Ukrainian_ie є CYRILLIC SMALL LETTER UKRAINIAN IE */
  { 0x06a5, 0x0455 }, /*               Macedonia_dse ѕ CYRILLIC SMALL LETTER DZE */
  { 0x06a6, 0x0456 }, /*                 Ukrainian_i і CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I */
  { 0x06a7, 0x0457 }, /*                Ukrainian_yi ї CYRILLIC SMALL LETTER YI */
  { 0x06a8, 0x0458 }, /*                 Cyrillic_je ј CYRILLIC SMALL LETTER JE */
  { 0x06a9, 0x0459 }, /*                Cyrillic_lje љ CYRILLIC SMALL LETTER LJE */
  { 0x06aa, 0x045a }, /*                Cyrillic_nje њ CYRILLIC SMALL LETTER NJE */
  { 0x06ab, 0x045b }, /*                Serbian_tshe ћ CYRILLIC SMALL LETTER TSHE */
  { 0x06ac, 0x045c }, /*               Macedonia_kje ќ CYRILLIC SMALL LETTER KJE */
  { 0x06ae, 0x045e }, /*         Byelorussian_shortu ў CYRILLIC SMALL LETTER SHORT U */
  { 0x06af, 0x045f }, /*               Cyrillic_dzhe џ CYRILLIC SMALL LETTER DZHE */
  { 0x06b0, 0x2116 }, /*                  numerosign № NUMERO SIGN */
  { 0x06b1, 0x0402 }, /*                 Serbian_DJE Ђ CYRILLIC CAPITAL LETTER DJE */
  { 0x06b2, 0x0403 }, /*               Macedonia_GJE Ѓ CYRILLIC CAPITAL LETTER GJE */
  { 0x06b3, 0x0401 }, /*                 Cyrillic_IO Ё CYRILLIC CAPITAL LETTER IO */
  { 0x06b4, 0x0404 }, /*                Ukrainian_IE Є CYRILLIC CAPITAL LETTER UKRAINIAN IE */
  { 0x06b5, 0x0405 }, /*               Macedonia_DSE Ѕ CYRILLIC CAPITAL LETTER DZE */
  { 0x06b6, 0x0406 }, /*                 Ukrainian_I І CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I */
  { 0x06b7, 0x0407 }, /*                Ukrainian_YI Ї CYRILLIC CAPITAL LETTER YI */
  { 0x06b8, 0x0408 }, /*                 Cyrillic_JE Ј CYRILLIC CAPITAL LETTER JE */
  { 0x06b9, 0x0409 }, /*                Cyrillic_LJE Љ CYRILLIC CAPITAL LETTER LJE */
  { 0x06ba, 0x040a }, /*                Cyrillic_NJE Њ CYRILLIC CAPITAL LETTER NJE */
  { 0x06bb, 0x040b }, /*                Serbian_TSHE Ћ CYRILLIC CAPITAL LETTER TSHE */
  { 0x06bc, 0x040c }, /*               Macedonia_KJE Ќ CYRILLIC CAPITAL LETTER KJE */
  { 0x06be, 0x040e }, /*         Byelorussian_SHORTU Ў CYRILLIC CAPITAL LETTER SHORT U */
  { 0x06bf, 0x040f }, /*               Cyrillic_DZHE Џ CYRILLIC CAPITAL LETTER DZHE */
  { 0x06c0, 0x044e }, /*                 Cyrillic_yu ю CYRILLIC SMALL LETTER YU */
  { 0x06c1, 0x0430 }, /*                  Cyrillic_a а CYRILLIC SMALL LETTER A */
  { 0x06c2, 0x0431 }, /*                 Cyrillic_be б CYRILLIC SMALL LETTER BE */
  { 0x06c3, 0x0446 }, /*                Cyrillic_tse ц CYRILLIC SMALL LETTER TSE */
  { 0x06c4, 0x0434 }, /*                 Cyrillic_de д CYRILLIC SMALL LETTER DE */
  { 0x06c5, 0x0435 }, /*                 Cyrillic_ie е CYRILLIC SMALL LETTER IE */
  { 0x06c6, 0x0444 }, /*                 Cyrillic_ef ф CYRILLIC SMALL LETTER EF */
  { 0x06c7, 0x0433 }, /*                Cyrillic_ghe г CYRILLIC SMALL LETTER GHE */
  { 0x06c8, 0x0445 }, /*                 Cyrillic_ha х CYRILLIC SMALL LETTER HA */
  { 0x06c9, 0x0438 }, /*                  Cyrillic_i и CYRILLIC SMALL LETTER I */
  { 0x06ca, 0x0439 }, /*             Cyrillic_shorti й CYRILLIC SMALL LETTER SHORT I */
  { 0x06cb, 0x043a }, /*                 Cyrillic_ka к CYRILLIC SMALL LETTER KA */
  { 0x06cc, 0x043b }, /*                 Cyrillic_el л CYRILLIC SMALL LETTER EL */
  { 0x06cd, 0x043c }, /*                 Cyrillic_em м CYRILLIC SMALL LETTER EM */
  { 0x06ce, 0x043d }, /*                 Cyrillic_en н CYRILLIC SMALL LETTER EN */
  { 0x06cf, 0x043e }, /*                  Cyrillic_o о CYRILLIC SMALL LETTER O */
  { 0x06d0, 0x043f }, /*                 Cyrillic_pe п CYRILLIC SMALL LETTER PE */
  { 0x06d1, 0x044f }, /*                 Cyrillic_ya я CYRILLIC SMALL LETTER YA */
  { 0x06d2, 0x0440 }, /*                 Cyrillic_er р CYRILLIC SMALL LETTER ER */
  { 0x06d3, 0x0441 }, /*                 Cyrillic_es с CYRILLIC SMALL LETTER ES */
  { 0x06d4, 0x0442 }, /*                 Cyrillic_te т CYRILLIC SMALL LETTER TE */
  { 0x06d5, 0x0443 }, /*                  Cyrillic_u у CYRILLIC SMALL LETTER U */
  { 0x06d6, 0x0436 }, /*                Cyrillic_zhe ж CYRILLIC SMALL LETTER ZHE */
  { 0x06d7, 0x0432 }, /*                 Cyrillic_ve в CYRILLIC SMALL LETTER VE */
  { 0x06d8, 0x044c }, /*           Cyrillic_softsign ь CYRILLIC SMALL LETTER SOFT SIGN */
  { 0x06d9, 0x044b }, /*               Cyrillic_yeru ы CYRILLIC SMALL LETTER YERU */
  { 0x06da, 0x0437 }, /*                 Cyrillic_ze з CYRILLIC SMALL LETTER ZE */
  { 0x06db, 0x0448 }, /*                Cyrillic_sha ш CYRILLIC SMALL LETTER SHA */
  { 0x06dc, 0x044d }, /*                  Cyrillic_e э CYRILLIC SMALL LETTER E */
  { 0x06dd, 0x0449 }, /*              Cyrillic_shcha щ CYRILLIC SMALL LETTER SHCHA */
  { 0x06de, 0x0447 }, /*                Cyrillic_che ч CYRILLIC SMALL LETTER CHE */
  { 0x06df, 0x044a }, /*           Cyrillic_hardsign ъ CYRILLIC SMALL LETTER HARD SIGN */
  { 0x06e0, 0x042e }, /*                 Cyrillic_YU Ю CYRILLIC CAPITAL LETTER YU */
  { 0x06e1, 0x0410 }, /*                  Cyrillic_A А CYRILLIC CAPITAL LETTER A */
  { 0x06e2, 0x0411 }, /*                 Cyrillic_BE Б CYRILLIC CAPITAL LETTER BE */
  { 0x06e3, 0x0426 }, /*                Cyrillic_TSE Ц CYRILLIC CAPITAL LETTER TSE */
  { 0x06e4, 0x0414 }, /*                 Cyrillic_DE Д CYRILLIC CAPITAL LETTER DE */
  { 0x06e5, 0x0415 }, /*                 Cyrillic_IE Е CYRILLIC CAPITAL LETTER IE */
  { 0x06e6, 0x0424 }, /*                 Cyrillic_EF Ф CYRILLIC CAPITAL LETTER EF */
  { 0x06e7, 0x0413 }, /*                Cyrillic_GHE Г CYRILLIC CAPITAL LETTER GHE */
  { 0x06e8, 0x0425 }, /*                 Cyrillic_HA Х CYRILLIC CAPITAL LETTER HA */
  { 0x06e9, 0x0418 }, /*                  Cyrillic_I И CYRILLIC CAPITAL LETTER I */
  { 0x06ea, 0x0419 }, /*             Cyrillic_SHORTI Й CYRILLIC CAPITAL LETTER SHORT I */
  { 0x06eb, 0x041a }, /*                 Cyrillic_KA К CYRILLIC CAPITAL LETTER KA */
  { 0x06ec, 0x041b }, /*                 Cyrillic_EL Л CYRILLIC CAPITAL LETTER EL */
  { 0x06ed, 0x041c }, /*                 Cyrillic_EM М CYRILLIC CAPITAL LETTER EM */
  { 0x06ee, 0x041d }, /*                 Cyrillic_EN Н CYRILLIC CAPITAL LETTER EN */
  { 0x06ef, 0x041e }, /*                  Cyrillic_O О CYRILLIC CAPITAL LETTER O */
  { 0x06f0, 0x041f }, /*                 Cyrillic_PE П CYRILLIC CAPITAL LETTER PE */
  { 0x06f1, 0x042f }, /*                 Cyrillic_YA Я CYRILLIC CAPITAL LETTER YA */
  { 0x06f2, 0x0420 }, /*                 Cyrillic_ER Р CYRILLIC CAPITAL LETTER ER */
  { 0x06f3, 0x0421 }, /*                 Cyrillic_ES С CYRILLIC CAPITAL LETTER ES */
  { 0x06f4, 0x0422 }, /*                 Cyrillic_TE Т CYRILLIC CAPITAL LETTER TE */
  { 0x06f5, 0x0423 }, /*                  Cyrillic_U У CYRILLIC CAPITAL LETTER U */
  { 0x06f6, 0x0416 }, /*                Cyrillic_ZHE Ж CYRILLIC CAPITAL LETTER ZHE */
  { 0x06f7, 0x0412 }, /*                 Cyrillic_VE В CYRILLIC CAPITAL LETTER VE */
  { 0x06f8, 0x042c }, /*           Cyrillic_SOFTSIGN Ь CYRILLIC CAPITAL LETTER SOFT SIGN */
  { 0x06f9, 0x042b }, /*               Cyrillic_YERU Ы CYRILLIC CAPITAL LETTER YERU */
  { 0x06fa, 0x0417 }, /*                 Cyrillic_ZE З CYRILLIC CAPITAL LETTER ZE */
  { 0x06fb, 0x0428 }, /*                Cyrillic_SHA Ш CYRILLIC CAPITAL LETTER SHA */
  { 0x06fc, 0x042d }, /*                  Cyrillic_E Э CYRILLIC CAPITAL LETTER E */
  { 0x06fd, 0x0429 }, /*              Cyrillic_SHCHA Щ CYRILLIC CAPITAL LETTER SHCHA */
  { 0x06fe, 0x0427 }, /*                Cyrillic_CHE Ч CYRILLIC CAPITAL LETTER CHE */
  { 0x06ff, 0x042a }, /*           Cyrillic_HARDSIGN Ъ CYRILLIC CAPITAL LETTER HARD SIGN */
  { 0x07a1, 0x0386 }, /*           Greek_ALPHAaccent Ά GREEK CAPITAL LETTER ALPHA WITH TONOS */
  { 0x07a2, 0x0388 }, /*         Greek_EPSILONaccent Έ GREEK CAPITAL LETTER EPSILON WITH TONOS */
  { 0x07a3, 0x0389 }, /*             Greek_ETAaccent Ή GREEK CAPITAL LETTER ETA WITH TONOS */
  { 0x07a4, 0x038a }, /*            Greek_IOTAaccent Ί GREEK CAPITAL LETTER IOTA WITH TONOS */
  { 0x07a5, 0x03aa }, /*         Greek_IOTAdiaeresis Ϊ GREEK CAPITAL LETTER IOTA WITH DIALYTIKA */
  { 0x07a7, 0x038c }, /*         Greek_OMICRONaccent Ό GREEK CAPITAL LETTER OMICRON WITH TONOS */
  { 0x07a8, 0x038e }, /*         Greek_UPSILONaccent Ύ GREEK CAPITAL LETTER UPSILON WITH TONOS */
  { 0x07a9, 0x03ab }, /*       Greek_UPSILONdieresis Ϋ GREEK CAPITAL LETTER UPSILON WITH DIALYTIKA */
  { 0x07ab, 0x038f }, /*           Greek_OMEGAaccent Ώ GREEK CAPITAL LETTER OMEGA WITH TONOS */
  { 0x07ae, 0x0385 }, /*        Greek_accentdieresis ΅ GREEK DIALYTIKA TONOS */
  { 0x07af, 0x2015 }, /*              Greek_horizbar ― HORIZONTAL BAR */
  { 0x07b1, 0x03ac }, /*           Greek_alphaaccent ά GREEK SMALL LETTER ALPHA WITH TONOS */
  { 0x07b2, 0x03ad }, /*         Greek_epsilonaccent έ GREEK SMALL LETTER EPSILON WITH TONOS */
  { 0x07b3, 0x03ae }, /*             Greek_etaaccent ή GREEK SMALL LETTER ETA WITH TONOS */
  { 0x07b4, 0x03af }, /*            Greek_iotaaccent ί GREEK SMALL LETTER IOTA WITH TONOS */
  { 0x07b5, 0x03ca }, /*          Greek_iotadieresis ϊ GREEK SMALL LETTER IOTA WITH DIALYTIKA */
  { 0x07b6, 0x0390 }, /*    Greek_iotaaccentdieresis ΐ GREEK SMALL LETTER IOTA WITH DIALYTIKA AND TONOS */
  { 0x07b7, 0x03cc }, /*         Greek_omicronaccent ό GREEK SMALL LETTER OMICRON WITH TONOS */
  { 0x07b8, 0x03cd }, /*         Greek_upsilonaccent ύ GREEK SMALL LETTER UPSILON WITH TONOS */
  { 0x07b9, 0x03cb }, /*       Greek_upsilondieresis ϋ GREEK SMALL LETTER UPSILON WITH DIALYTIKA */
  { 0x07ba, 0x03b0 }, /* Greek_upsilonaccentdieresis ΰ GREEK SMALL LETTER UPSILON WITH DIALYTIKA AND TONOS */
  { 0x07bb, 0x03ce }, /*           Greek_omegaaccent ώ GREEK SMALL LETTER OMEGA WITH TONOS */
  { 0x07c1, 0x0391 }, /*                 Greek_ALPHA Α GREEK CAPITAL LETTER ALPHA */
  { 0x07c2, 0x0392 }, /*                  Greek_BETA Β GREEK CAPITAL LETTER BETA */
  { 0x07c3, 0x0393 }, /*                 Greek_GAMMA Γ GREEK CAPITAL LETTER GAMMA */
  { 0x07c4, 0x0394 }, /*                 Greek_DELTA Δ GREEK CAPITAL LETTER DELTA */
  { 0x07c5, 0x0395 }, /*               Greek_EPSILON Ε GREEK CAPITAL LETTER EPSILON */
  { 0x07c6, 0x0396 }, /*                  Greek_ZETA Ζ GREEK CAPITAL LETTER ZETA */
  { 0x07c7, 0x0397 }, /*                   Greek_ETA Η GREEK CAPITAL LETTER ETA */
  { 0x07c8, 0x0398 }, /*                 Greek_THETA Θ GREEK CAPITAL LETTER THETA */
  { 0x07c9, 0x0399 }, /*                  Greek_IOTA Ι GREEK CAPITAL LETTER IOTA */
  { 0x07ca, 0x039a }, /*                 Greek_KAPPA Κ GREEK CAPITAL LETTER KAPPA */
  { 0x07cb, 0x039b }, /*                Greek_LAMBDA Λ GREEK CAPITAL LETTER LAMDA */
  { 0x07cc, 0x039c }, /*                    Greek_MU Μ GREEK CAPITAL LETTER MU */
  { 0x07cd, 0x039d }, /*                    Greek_NU Ν GREEK CAPITAL LETTER NU */
  { 0x07ce, 0x039e }, /*                    Greek_XI Ξ GREEK CAPITAL LETTER XI */
  { 0x07cf, 0x039f }, /*               Greek_OMICRON Ο GREEK CAPITAL LETTER OMICRON */
  { 0x07d0, 0x03a0 }, /*                    Greek_PI Π GREEK CAPITAL LETTER PI */
  { 0x07d1, 0x03a1 }, /*                   Greek_RHO Ρ GREEK CAPITAL LETTER RHO */
  { 0x07d2, 0x03a3 }, /*                 Greek_SIGMA Σ GREEK CAPITAL LETTER SIGMA */
  { 0x07d4, 0x03a4 }, /*                   Greek_TAU Τ GREEK CAPITAL LETTER TAU */
  { 0x07d5, 0x03a5 }, /*               Greek_UPSILON Υ GREEK CAPITAL LETTER UPSILON */
  { 0x07d6, 0x03a6 }, /*                   Greek_PHI Φ GREEK CAPITAL LETTER PHI */
  { 0x07d7, 0x03a7 }, /*                   Greek_CHI Χ GREEK CAPITAL LETTER CHI */
  { 0x07d8, 0x03a8 }, /*                   Greek_PSI Ψ GREEK CAPITAL LETTER PSI */
  { 0x07d9, 0x03a9 }, /*                 Greek_OMEGA Ω GREEK CAPITAL LETTER OMEGA */
  { 0x07e1, 0x03b1 }, /*                 Greek_alpha α GREEK SMALL LETTER ALPHA */
  { 0x07e2, 0x03b2 }, /*                  Greek_beta β GREEK SMALL LETTER BETA */
  { 0x07e3, 0x03b3 }, /*                 Greek_gamma γ GREEK SMALL LETTER GAMMA */
  { 0x07e4, 0x03b4 }, /*                 Greek_delta δ GREEK SMALL LETTER DELTA */
  { 0x07e5, 0x03b5 }, /*               Greek_epsilon ε GREEK SMALL LETTER EPSILON */
  { 0x07e6, 0x03b6 }, /*                  Greek_zeta ζ GREEK SMALL LETTER ZETA */
  { 0x07e7, 0x03b7 }, /*                   Greek_eta η GREEK SMALL LETTER ETA */
  { 0x07e8, 0x03b8 }, /*                 Greek_theta θ GREEK SMALL LETTER THETA */
  { 0x07e9, 0x03b9 }, /*                  Greek_iota ι GREEK SMALL LETTER IOTA */
  { 0x07ea, 0x03ba }, /*                 Greek_kappa κ GREEK SMALL LETTER KAPPA */
  { 0x07eb, 0x03bb }, /*                Greek_lambda λ GREEK SMALL LETTER LAMDA */
  { 0x07ec, 0x03bc }, /*                    Greek_mu μ GREEK SMALL LETTER MU */
  { 0x07ed, 0x03bd }, /*                    Greek_nu ν GREEK SMALL LETTER NU */
  { 0x07ee, 0x03be }, /*                    Greek_xi ξ GREEK SMALL LETTER XI */
  { 0x07ef, 0x03bf }, /*               Greek_omicron ο GREEK SMALL LETTER OMICRON */
  { 0x07f0, 0x03c0 }, /*                    Greek_pi π GREEK SMALL LETTER PI */
  { 0x07f1, 0x03c1 }, /*                   Greek_rho ρ GREEK SMALL LETTER RHO */
  { 0x07f2, 0x03c3 }, /*                 Greek_sigma σ GREEK SMALL LETTER SIGMA */
  { 0x07f3, 0x03c2 }, /*       Greek_finalsmallsigma ς GREEK SMALL LETTER FINAL SIGMA */
  { 0x07f4, 0x03c4 }, /*                   Greek_tau τ GREEK SMALL LETTER TAU */
  { 0x07f5, 0x03c5 }, /*               Greek_upsilon υ GREEK SMALL LETTER UPSILON */
  { 0x07f6, 0x03c6 }, /*                   Greek_phi φ GREEK SMALL LETTER PHI */
  { 0x07f7, 0x03c7 }, /*                   Greek_chi χ GREEK SMALL LETTER CHI */
  { 0x07f8, 0x03c8 }, /*                   Greek_psi ψ GREEK SMALL LETTER PSI */
  { 0x07f9, 0x03c9 }, /*                 Greek_omega ω GREEK SMALL LETTER OMEGA */
/*  0x08a1                               leftradical ? ??? */
/*  0x08a2                            topleftradical ? ??? */
/*  0x08a3                            horizconnector ? ??? */
  { 0x08a4, 0x2320 }, /*                 topintegral ⌠ TOP HALF INTEGRAL */
  { 0x08a5, 0x2321 }, /*                 botintegral ⌡ BOTTOM HALF INTEGRAL */
  { 0x08a6, 0x2502 }, /*               vertconnector │ BOX DRAWINGS LIGHT VERTICAL */
/*  0x08a7                          topleftsqbracket ? ??? */
/*  0x08a8                          botleftsqbracket ? ??? */
/*  0x08a9                         toprightsqbracket ? ??? */
/*  0x08aa                         botrightsqbracket ? ??? */
/*  0x08ab                             topleftparens ? ??? */
/*  0x08ac                             botleftparens ? ??? */
/*  0x08ad                            toprightparens ? ??? */
/*  0x08ae                            botrightparens ? ??? */
/*  0x08af                      leftmiddlecurlybrace ? ??? */
/*  0x08b0                     rightmiddlecurlybrace ? ??? */
/*  0x08b1                          topleftsummation ? ??? */
/*  0x08b2                          botleftsummation ? ??? */
/*  0x08b3                 topvertsummationconnector ? ??? */
/*  0x08b4                 botvertsummationconnector ? ??? */
/*  0x08b5                         toprightsummation ? ??? */
/*  0x08b6                         botrightsummation ? ??? */
/*  0x08b7                      rightmiddlesummation ? ??? */
  { 0x08bc, 0x2264 }, /*               lessthanequal ≤ LESS-THAN OR EQUAL TO */
  { 0x08bd, 0x2260 }, /*                    notequal ≠ NOT EQUAL TO */
  { 0x08be, 0x2265 }, /*            greaterthanequal ≥ GREATER-THAN OR EQUAL TO */
  { 0x08bf, 0x222b }, /*                    integral ∫ INTEGRAL */
  { 0x08c0, 0x2234 }, /*                   therefore ∴ THEREFORE */
  { 0x08c1, 0x221d }, /*                   variation ∝ PROPORTIONAL TO */
  { 0x08c2, 0x221e }, /*                    infinity ∞ INFINITY */
  { 0x08c5, 0x2207 }, /*                       nabla ∇ NABLA */
  { 0x08c8, 0x2245 }, /*                 approximate ≅ APPROXIMATELY EQUAL TO */
/*  0x08c9                              similarequal ? ??? */
  { 0x08cd, 0x21d4 }, /*                    ifonlyif ⇔ LEFT RIGHT DOUBLE ARROW */
  { 0x08ce, 0x21d2 }, /*                     implies ⇒ RIGHTWARDS DOUBLE ARROW */
  { 0x08cf, 0x2261 }, /*                   identical ≡ IDENTICAL TO */
  { 0x08d6, 0x221a }, /*                     radical √ SQUARE ROOT */
  { 0x08da, 0x2282 }, /*                  includedin ⊂ SUBSET OF */
  { 0x08db, 0x2283 }, /*                    includes ⊃ SUPERSET OF */
  { 0x08dc, 0x2229 }, /*                intersection ∩ INTERSECTION */
  { 0x08dd, 0x222a }, /*                       union ∪ UNION */
  { 0x08de, 0x2227 }, /*                  logicaland ∧ LOGICAL AND */
  { 0x08df, 0x2228 }, /*                   logicalor ∨ LOGICAL OR */
  { 0x08ef, 0x2202 }, /*           partialderivative ∂ PARTIAL DIFFERENTIAL */
  { 0x08f6, 0x0192 }, /*                    function ƒ LATIN SMALL LETTER F WITH HOOK */
  { 0x08fb, 0x2190 }, /*                   leftarrow ← LEFTWARDS ARROW */
  { 0x08fc, 0x2191 }, /*                     uparrow ↑ UPWARDS ARROW */
  { 0x08fd, 0x2192 }, /*                  rightarrow → RIGHTWARDS ARROW */
  { 0x08fe, 0x2193 }, /*                   downarrow ↓ DOWNWARDS ARROW */
  { 0x09df, 0x2422 }, /*                       blank ␢ BLANK SYMBOL */
  { 0x09e0, 0x25c6 }, /*                soliddiamond ◆ BLACK DIAMOND */
  { 0x09e1, 0x2592 }, /*                checkerboard ▒ MEDIUM SHADE */
  { 0x09e2, 0x2409 }, /*                          ht ␉ SYMBOL FOR HORIZONTAL TABULATION */
  { 0x09e3, 0x240c }, /*                          ff ␌ SYMBOL FOR FORM FEED */
  { 0x09e4, 0x240d }, /*                          cr ␍ SYMBOL FOR CARRIAGE RETURN */
  { 0x09e5, 0x240a }, /*                          lf ␊ SYMBOL FOR LINE FEED */
  { 0x09e8, 0x2424 }, /*                          nl ␤ SYMBOL FOR NEWLINE */
  { 0x09e9, 0x240b }, /*                          vt ␋ SYMBOL FOR VERTICAL TABULATION */
  { 0x09ea, 0x2518 }, /*              lowrightcorner ┘ BOX DRAWINGS LIGHT UP AND LEFT */
  { 0x09eb, 0x2510 }, /*               uprightcorner ┐ BOX DRAWINGS LIGHT DOWN AND LEFT */
  { 0x09ec, 0x250c }, /*                upleftcorner ┌ BOX DRAWINGS LIGHT DOWN AND RIGHT */
  { 0x09ed, 0x2514 }, /*               lowleftcorner └ BOX DRAWINGS LIGHT UP AND RIGHT */
  { 0x09ee, 0x253c }, /*               crossinglines ┼ BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL */
/*  0x09ef                            horizlinescan1 ? ??? */
/*  0x09f0                            horizlinescan3 ? ??? */
  { 0x09f1, 0x2500 }, /*              horizlinescan5 ─ BOX DRAWINGS LIGHT HORIZONTAL */
/*  0x09f2                            horizlinescan7 ? ??? */
/*  0x09f3                            horizlinescan9 ? ??? */
  { 0x09f4, 0x251c }, /*                       leftt ├ BOX DRAWINGS LIGHT VERTICAL AND RIGHT */
  { 0x09f5, 0x2524 }, /*                      rightt ┤ BOX DRAWINGS LIGHT VERTICAL AND LEFT */
  { 0x09f6, 0x2534 }, /*                        bott ┴ BOX DRAWINGS LIGHT UP AND HORIZONTAL */
  { 0x09f7, 0x252c }, /*                        topt ┬ BOX DRAWINGS LIGHT DOWN AND HORIZONTAL */
  { 0x09f8, 0x2502 }, /*                     vertbar │ BOX DRAWINGS LIGHT VERTICAL */
  { 0x0aa1, 0x2003 }, /*                     emspace   EM SPACE */
  { 0x0aa2, 0x2002 }, /*                     enspace   EN SPACE */
  { 0x0aa3, 0x2004 }, /*                    em3space   THREE-PER-EM SPACE */
  { 0x0aa4, 0x2005 }, /*                    em4space   FOUR-PER-EM SPACE */
  { 0x0aa5, 0x2007 }, /*                  digitspace   FIGURE SPACE */
  { 0x0aa6, 0x2008 }, /*                  punctspace   PUNCTUATION SPACE */
  { 0x0aa7, 0x2009 }, /*                   thinspace   THIN SPACE */
  { 0x0aa8, 0x200a }, /*                   hairspace   HAIR SPACE */
  { 0x0aa9, 0x2014 }, /*                      emdash — EM DASH */
  { 0x0aaa, 0x2013 }, /*                      endash – EN DASH */
/*  0x0aac                               signifblank ? ??? */
  { 0x0aae, 0x2026 }, /*                    ellipsis … HORIZONTAL ELLIPSIS */
/*  0x0aaf                           doubbaselinedot ? ??? */
  { 0x0ab0, 0x2153 }, /*                    onethird ⅓ VULGAR FRACTION ONE THIRD */
  { 0x0ab1, 0x2154 }, /*                   twothirds ⅔ VULGAR FRACTION TWO THIRDS */
  { 0x0ab2, 0x2155 }, /*                    onefifth ⅕ VULGAR FRACTION ONE FIFTH */
  { 0x0ab3, 0x2156 }, /*                   twofifths ⅖ VULGAR FRACTION TWO FIFTHS */
  { 0x0ab4, 0x2157 }, /*                 threefifths ⅗ VULGAR FRACTION THREE FIFTHS */
  { 0x0ab5, 0x2158 }, /*                  fourfifths ⅘ VULGAR FRACTION FOUR FIFTHS */
  { 0x0ab6, 0x2159 }, /*                    onesixth ⅙ VULGAR FRACTION ONE SIXTH */
  { 0x0ab7, 0x215a }, /*                  fivesixths ⅚ VULGAR FRACTION FIVE SIXTHS */
  { 0x0ab8, 0x2105 }, /*                      careof ℅ CARE OF */
  { 0x0abb, 0x2012 }, /*                     figdash ‒ FIGURE DASH */
  { 0x0abc, 0x2329 }, /*            leftanglebracket 〈 LEFT-POINTING ANGLE BRACKET */
  { 0x0abd, 0x002e }, /*                decimalpoint . FULL STOP */
  { 0x0abe, 0x232a }, /*           rightanglebracket 〉 RIGHT-POINTING ANGLE BRACKET */
/*  0x0abf                                    marker ? ??? */
  { 0x0ac3, 0x215b }, /*                   oneeighth ⅛ VULGAR FRACTION ONE EIGHTH */
  { 0x0ac4, 0x215c }, /*                threeeighths ⅜ VULGAR FRACTION THREE EIGHTHS */
  { 0x0ac5, 0x215d }, /*                 fiveeighths ⅝ VULGAR FRACTION FIVE EIGHTHS */
  { 0x0ac6, 0x215e }, /*                seveneighths ⅞ VULGAR FRACTION SEVEN EIGHTHS */
  { 0x0ac9, 0x2122 }, /*                   trademark ™ TRADE MARK SIGN */
  { 0x0aca, 0x2613 }, /*               signaturemark ☓ SALTIRE */
/*  0x0acb                         trademarkincircle ? ??? */
  { 0x0acc, 0x25c1 }, /*            leftopentriangle ◁ WHITE LEFT-POINTING TRIANGLE */
  { 0x0acd, 0x25b7 }, /*           rightopentriangle ▷ WHITE RIGHT-POINTING TRIANGLE */
  { 0x0ace, 0x25cb }, /*                emopencircle ○ WHITE CIRCLE */
  { 0x0acf, 0x25a1 }, /*             emopenrectangle □ WHITE SQUARE */
  { 0x0ad0, 0x2018 }, /*         leftsinglequotemark ‘ LEFT SINGLE QUOTATION MARK */
  { 0x0ad1, 0x2019 }, /*        rightsinglequotemark ’ RIGHT SINGLE QUOTATION MARK */
  { 0x0ad2, 0x201c }, /*         leftdoublequotemark “ LEFT DOUBLE QUOTATION MARK */
  { 0x0ad3, 0x201d }, /*        rightdoublequotemark ” RIGHT DOUBLE QUOTATION MARK */
  { 0x0ad4, 0x211e }, /*                prescription ℞ PRESCRIPTION TAKE */
  { 0x0ad6, 0x2032 }, /*                     minutes ′ PRIME */
  { 0x0ad7, 0x2033 }, /*                     seconds ″ DOUBLE PRIME */
  { 0x0ad9, 0x271d }, /*                  latincross ✝ LATIN CROSS */
/*  0x0ada                                  hexagram ? ??? */
  { 0x0adb, 0x25ac }, /*            filledrectbullet ▬ BLACK RECTANGLE */
  { 0x0adc, 0x25c0 }, /*         filledlefttribullet ◀ BLACK LEFT-POINTING TRIANGLE */
  { 0x0add, 0x25b6 }, /*        filledrighttribullet ▶ BLACK RIGHT-POINTING TRIANGLE */
  { 0x0ade, 0x25cf }, /*              emfilledcircle ● BLACK CIRCLE */
  { 0x0adf, 0x25a0 }, /*                emfilledrect ■ BLACK SQUARE */
  { 0x0ae0, 0x25e6 }, /*            enopencircbullet ◦ WHITE BULLET */
  { 0x0ae1, 0x25ab }, /*          enopensquarebullet ▫ WHITE SMALL SQUARE */
  { 0x0ae2, 0x25ad }, /*              openrectbullet ▭ WHITE RECTANGLE */
  { 0x0ae3, 0x25b3 }, /*             opentribulletup △ WHITE UP-POINTING TRIANGLE */
  { 0x0ae4, 0x25bd }, /*           opentribulletdown ▽ WHITE DOWN-POINTING TRIANGLE */
  { 0x0ae5, 0x2606 }, /*                    openstar ☆ WHITE STAR */
  { 0x0ae6, 0x2022 }, /*          enfilledcircbullet • BULLET */
  { 0x0ae7, 0x25aa }, /*            enfilledsqbullet ▪ BLACK SMALL SQUARE */
  { 0x0ae8, 0x25b2 }, /*           filledtribulletup ▲ BLACK UP-POINTING TRIANGLE */
  { 0x0ae9, 0x25bc }, /*         filledtribulletdown ▼ BLACK DOWN-POINTING TRIANGLE */
  { 0x0aea, 0x261c }, /*                 leftpointer ☜ WHITE LEFT POINTING INDEX */
  { 0x0aeb, 0x261e }, /*                rightpointer ☞ WHITE RIGHT POINTING INDEX */
  { 0x0aec, 0x2663 }, /*                        club ♣ BLACK CLUB SUIT */
  { 0x0aed, 0x2666 }, /*                     diamond ♦ BLACK DIAMOND SUIT */
  { 0x0aee, 0x2665 }, /*                       heart ♥ BLACK HEART SUIT */
  { 0x0af0, 0x2720 }, /*                maltesecross ✠ MALTESE CROSS */
  { 0x0af1, 0x2020 }, /*                      dagger † DAGGER */
  { 0x0af2, 0x2021 }, /*                doubledagger ‡ DOUBLE DAGGER */
  { 0x0af3, 0x2713 }, /*                   checkmark ✓ CHECK MARK */
  { 0x0af4, 0x2717 }, /*                 ballotcross ✗ BALLOT X */
  { 0x0af5, 0x266f }, /*                musicalsharp ♯ MUSIC SHARP SIGN */
  { 0x0af6, 0x266d }, /*                 musicalflat ♭ MUSIC FLAT SIGN */
  { 0x0af7, 0x2642 }, /*                  malesymbol ♂ MALE SIGN */
  { 0x0af8, 0x2640 }, /*                femalesymbol ♀ FEMALE SIGN */
  { 0x0af9, 0x260e }, /*                   telephone ☎ BLACK TELEPHONE */
  { 0x0afa, 0x2315 }, /*           telephonerecorder ⌕ TELEPHONE RECORDER */
  { 0x0afb, 0x2117 }, /*         phonographcopyright ℗ SOUND RECORDING COPYRIGHT */
  { 0x0afc, 0x2038 }, /*                       caret ‸ CARET */
  { 0x0afd, 0x201a }, /*          singlelowquotemark ‚ SINGLE LOW-9 QUOTATION MARK */
  { 0x0afe, 0x201e }, /*          doublelowquotemark „ DOUBLE LOW-9 QUOTATION MARK */
/*  0x0aff                                    cursor ? ??? */
  { 0x0ba3, 0x003c }, /*                   leftcaret < LESS-THAN SIGN */
  { 0x0ba6, 0x003e }, /*                  rightcaret > GREATER-THAN SIGN */
  { 0x0ba8, 0x2228 }, /*                   downcaret ∨ LOGICAL OR */
  { 0x0ba9, 0x2227 }, /*                     upcaret ∧ LOGICAL AND */
  { 0x0bc0, 0x00af }, /*                     overbar ¯ MACRON */
  { 0x0bc2, 0x22a4 }, /*                    downtack ⊤ DOWN TACK */
  { 0x0bc3, 0x2229 }, /*                      upshoe ∩ INTERSECTION */
  { 0x0bc4, 0x230a }, /*                   downstile ⌊ LEFT FLOOR */
  { 0x0bc6, 0x005f }, /*                    underbar _ LOW LINE */
  { 0x0bca, 0x2218 }, /*                         jot ∘ RING OPERATOR */
  { 0x0bcc, 0x2395 }, /*                        quad ⎕ APL FUNCTIONAL SYMBOL QUAD (Unicode 3.0) */
  { 0x0bce, 0x22a5 }, /*                      uptack ⊥ UP TACK */
  { 0x0bcf, 0x25cb }, /*                      circle ○ WHITE CIRCLE */
  { 0x0bd3, 0x2308 }, /*                     upstile ⌈ LEFT CEILING */
  { 0x0bd6, 0x222a }, /*                    downshoe ∪ UNION */
  { 0x0bd8, 0x2283 }, /*                   rightshoe ⊃ SUPERSET OF */
  { 0x0bda, 0x2282 }, /*                    leftshoe ⊂ SUBSET OF */
  { 0x0bdc, 0x22a3 }, /*                    lefttack ⊣ LEFT TACK */
  { 0x0bfc, 0x22a2 }, /*                   righttack ⊢ RIGHT TACK */
  { 0x0cdf, 0x2017 }, /*        hebrew_doublelowline ‗ DOUBLE LOW LINE */
  { 0x0ce0, 0x05d0 }, /*                hebrew_aleph א HEBREW LETTER ALEF */
  { 0x0ce1, 0x05d1 }, /*                  hebrew_bet ב HEBREW LETTER BET */
  { 0x0ce2, 0x05d2 }, /*                hebrew_gimel ג HEBREW LETTER GIMEL */
  { 0x0ce3, 0x05d3 }, /*                hebrew_dalet ד HEBREW LETTER DALET */
  { 0x0ce4, 0x05d4 }, /*                   hebrew_he ה HEBREW LETTER HE */
  { 0x0ce5, 0x05d5 }, /*                  hebrew_waw ו HEBREW LETTER VAV */
  { 0x0ce6, 0x05d6 }, /*                 hebrew_zain ז HEBREW LETTER ZAYIN */
  { 0x0ce7, 0x05d7 }, /*                 hebrew_chet ח HEBREW LETTER HET */
  { 0x0ce8, 0x05d8 }, /*                  hebrew_tet ט HEBREW LETTER TET */
  { 0x0ce9, 0x05d9 }, /*                  hebrew_yod י HEBREW LETTER YOD */
  { 0x0cea, 0x05da }, /*            hebrew_finalkaph ך HEBREW LETTER FINAL KAF */
  { 0x0ceb, 0x05db }, /*                 hebrew_kaph כ HEBREW LETTER KAF */
  { 0x0cec, 0x05dc }, /*                hebrew_lamed ל HEBREW LETTER LAMED */
  { 0x0ced, 0x05dd }, /*             hebrew_finalmem ם HEBREW LETTER FINAL MEM */
  { 0x0cee, 0x05de }, /*                  hebrew_mem מ HEBREW LETTER MEM */
  { 0x0cef, 0x05df }, /*             hebrew_finalnun ן HEBREW LETTER FINAL NUN */
  { 0x0cf0, 0x05e0 }, /*                  hebrew_nun נ HEBREW LETTER NUN */
  { 0x0cf1, 0x05e1 }, /*               hebrew_samech ס HEBREW LETTER SAMEKH */
  { 0x0cf2, 0x05e2 }, /*                 hebrew_ayin ע HEBREW LETTER AYIN */
  { 0x0cf3, 0x05e3 }, /*              hebrew_finalpe ף HEBREW LETTER FINAL PE */
  { 0x0cf4, 0x05e4 }, /*                   hebrew_pe פ HEBREW LETTER PE */
  { 0x0cf5, 0x05e5 }, /*            hebrew_finalzade ץ HEBREW LETTER FINAL TSADI */
  { 0x0cf6, 0x05e6 }, /*                 hebrew_zade צ HEBREW LETTER TSADI */
  { 0x0cf7, 0x05e7 }, /*                 hebrew_qoph ק HEBREW LETTER QOF */
  { 0x0cf8, 0x05e8 }, /*                 hebrew_resh ר HEBREW LETTER RESH */
  { 0x0cf9, 0x05e9 }, /*                 hebrew_shin ש HEBREW LETTER SHIN */
  { 0x0cfa, 0x05ea }, /*                  hebrew_taw ת HEBREW LETTER TAV */
  { 0x0da1, 0x0e01 }, /*                  Thai_kokai ก THAI CHARACTER KO KAI */
  { 0x0da2, 0x0e02 }, /*                Thai_khokhai ข THAI CHARACTER KHO KHAI */
  { 0x0da3, 0x0e03 }, /*               Thai_khokhuat ฃ THAI CHARACTER KHO KHUAT */
  { 0x0da4, 0x0e04 }, /*               Thai_khokhwai ค THAI CHARACTER KHO KHWAI */
  { 0x0da5, 0x0e05 }, /*                Thai_khokhon ฅ THAI CHARACTER KHO KHON */
  { 0x0da6, 0x0e06 }, /*             Thai_khorakhang ฆ THAI CHARACTER KHO RAKHANG */
  { 0x0da7, 0x0e07 }, /*                 Thai_ngongu ง THAI CHARACTER NGO NGU */
  { 0x0da8, 0x0e08 }, /*                Thai_chochan จ THAI CHARACTER CHO CHAN */
  { 0x0da9, 0x0e09 }, /*               Thai_choching ฉ THAI CHARACTER CHO CHING */
  { 0x0daa, 0x0e0a }, /*               Thai_chochang ช THAI CHARACTER CHO CHANG */
  { 0x0dab, 0x0e0b }, /*                   Thai_soso ซ THAI CHARACTER SO SO */
  { 0x0dac, 0x0e0c }, /*                Thai_chochoe ฌ THAI CHARACTER CHO CHOE */
  { 0x0dad, 0x0e0d }, /*                 Thai_yoying ญ THAI CHARACTER YO YING */
  { 0x0dae, 0x0e0e }, /*                Thai_dochada ฎ THAI CHARACTER DO CHADA */
  { 0x0daf, 0x0e0f }, /*                Thai_topatak ฏ THAI CHARACTER TO PATAK */
  { 0x0db0, 0x0e10 }, /*                Thai_thothan ฐ THAI CHARACTER THO THAN */
  { 0x0db1, 0x0e11 }, /*          Thai_thonangmontho ฑ THAI CHARACTER THO NANGMONTHO */
  { 0x0db2, 0x0e12 }, /*             Thai_thophuthao ฒ THAI CHARACTER THO PHUTHAO */
  { 0x0db3, 0x0e13 }, /*                  Thai_nonen ณ THAI CHARACTER NO NEN */
  { 0x0db4, 0x0e14 }, /*                  Thai_dodek ด THAI CHARACTER DO DEK */
  { 0x0db5, 0x0e15 }, /*                  Thai_totao ต THAI CHARACTER TO TAO */
  { 0x0db6, 0x0e16 }, /*               Thai_thothung ถ THAI CHARACTER THO THUNG */
  { 0x0db7, 0x0e17 }, /*              Thai_thothahan ท THAI CHARACTER THO THAHAN */
  { 0x0db8, 0x0e18 }, /*               Thai_thothong ธ THAI CHARACTER THO THONG */
  { 0x0db9, 0x0e19 }, /*                   Thai_nonu น THAI CHARACTER NO NU */
  { 0x0dba, 0x0e1a }, /*               Thai_bobaimai บ THAI CHARACTER BO BAIMAI */
  { 0x0dbb, 0x0e1b }, /*                  Thai_popla ป THAI CHARACTER PO PLA */
  { 0x0dbc, 0x0e1c }, /*               Thai_phophung ผ THAI CHARACTER PHO PHUNG */
  { 0x0dbd, 0x0e1d }, /*                   Thai_fofa ฝ THAI CHARACTER FO FA */
  { 0x0dbe, 0x0e1e }, /*                Thai_phophan พ THAI CHARACTER PHO PHAN */
  { 0x0dbf, 0x0e1f }, /*                  Thai_fofan ฟ THAI CHARACTER FO FAN */
  { 0x0dc0, 0x0e20 }, /*             Thai_phosamphao ภ THAI CHARACTER PHO SAMPHAO */
  { 0x0dc1, 0x0e21 }, /*                   Thai_moma ม THAI CHARACTER MO MA */
  { 0x0dc2, 0x0e22 }, /*                  Thai_yoyak ย THAI CHARACTER YO YAK */
  { 0x0dc3, 0x0e23 }, /*                  Thai_rorua ร THAI CHARACTER RO RUA */
  { 0x0dc4, 0x0e24 }, /*                     Thai_ru ฤ THAI CHARACTER RU */
  { 0x0dc5, 0x0e25 }, /*                 Thai_loling ล THAI CHARACTER LO LING */
  { 0x0dc6, 0x0e26 }, /*                     Thai_lu ฦ THAI CHARACTER LU */
  { 0x0dc7, 0x0e27 }, /*                 Thai_wowaen ว THAI CHARACTER WO WAEN */
  { 0x0dc8, 0x0e28 }, /*                 Thai_sosala ศ THAI CHARACTER SO SALA */
  { 0x0dc9, 0x0e29 }, /*                 Thai_sorusi ษ THAI CHARACTER SO RUSI */
  { 0x0dca, 0x0e2a }, /*                  Thai_sosua ส THAI CHARACTER SO SUA */
  { 0x0dcb, 0x0e2b }, /*                  Thai_hohip ห THAI CHARACTER HO HIP */
  { 0x0dcc, 0x0e2c }, /*                Thai_lochula ฬ THAI CHARACTER LO CHULA */
  { 0x0dcd, 0x0e2d }, /*                   Thai_oang อ THAI CHARACTER O ANG */
  { 0x0dce, 0x0e2e }, /*               Thai_honokhuk ฮ THAI CHARACTER HO NOKHUK */
  { 0x0dcf, 0x0e2f }, /*              Thai_paiyannoi ฯ THAI CHARACTER PAIYANNOI */
  { 0x0dd0, 0x0e30 }, /*                  Thai_saraa ะ THAI CHARACTER SARA A */
  { 0x0dd1, 0x0e31 }, /*             Thai_maihanakat ั THAI CHARACTER MAI HAN-AKAT */
  { 0x0dd2, 0x0e32 }, /*                 Thai_saraaa า THAI CHARACTER SARA AA */
  { 0x0dd3, 0x0e33 }, /*                 Thai_saraam ำ THAI CHARACTER SARA AM */
  { 0x0dd4, 0x0e34 }, /*                  Thai_sarai ิ THAI CHARACTER SARA I */
  { 0x0dd5, 0x0e35 }, /*                 Thai_saraii ี THAI CHARACTER SARA II */
  { 0x0dd6, 0x0e36 }, /*                 Thai_saraue ึ THAI CHARACTER SARA UE */
  { 0x0dd7, 0x0e37 }, /*                Thai_sarauee ื THAI CHARACTER SARA UEE */
  { 0x0dd8, 0x0e38 }, /*                  Thai_sarau ุ THAI CHARACTER SARA U */
  { 0x0dd9, 0x0e39 }, /*                 Thai_sarauu ู THAI CHARACTER SARA UU */
  { 0x0dda, 0x0e3a }, /*                Thai_phinthu ฺ THAI CHARACTER PHINTHU */
  { 0x0dde, 0x0e3e }, /*      Thai_maihanakat_maitho ฾ ??? */
  { 0x0ddf, 0x0e3f }, /*                   Thai_baht ฿ THAI CURRENCY SYMBOL BAHT */
  { 0x0de0, 0x0e40 }, /*                  Thai_sarae เ THAI CHARACTER SARA E */
  { 0x0de1, 0x0e41 }, /*                 Thai_saraae แ THAI CHARACTER SARA AE */
  { 0x0de2, 0x0e42 }, /*                  Thai_sarao โ THAI CHARACTER SARA O */
  { 0x0de3, 0x0e43 }, /*          Thai_saraaimaimuan ใ THAI CHARACTER SARA AI MAIMUAN */
  { 0x0de4, 0x0e44 }, /*         Thai_saraaimaimalai ไ THAI CHARACTER SARA AI MAIMALAI */
  { 0x0de5, 0x0e45 }, /*            Thai_lakkhangyao ๅ THAI CHARACTER LAKKHANGYAO */
  { 0x0de6, 0x0e46 }, /*               Thai_maiyamok ๆ THAI CHARACTER MAIYAMOK */
  { 0x0de7, 0x0e47 }, /*              Thai_maitaikhu ็ THAI CHARACTER MAITAIKHU */
  { 0x0de8, 0x0e48 }, /*                  Thai_maiek ่ THAI CHARACTER MAI EK */
  { 0x0de9, 0x0e49 }, /*                 Thai_maitho ้ THAI CHARACTER MAI THO */
  { 0x0dea, 0x0e4a }, /*                 Thai_maitri ๊ THAI CHARACTER MAI TRI */
  { 0x0deb, 0x0e4b }, /*            Thai_maichattawa ๋ THAI CHARACTER MAI CHATTAWA */
  { 0x0dec, 0x0e4c }, /*            Thai_thanthakhat ์ THAI CHARACTER THANTHAKHAT */
  { 0x0ded, 0x0e4d }, /*               Thai_nikhahit ํ THAI CHARACTER NIKHAHIT */
  { 0x0df0, 0x0e50 }, /*                 Thai_leksun ๐ THAI DIGIT ZERO */
  { 0x0df1, 0x0e51 }, /*                Thai_leknung ๑ THAI DIGIT ONE */
  { 0x0df2, 0x0e52 }, /*                Thai_leksong ๒ THAI DIGIT TWO */
  { 0x0df3, 0x0e53 }, /*                 Thai_leksam ๓ THAI DIGIT THREE */
  { 0x0df4, 0x0e54 }, /*                  Thai_leksi ๔ THAI DIGIT FOUR */
  { 0x0df5, 0x0e55 }, /*                  Thai_lekha ๕ THAI DIGIT FIVE */
  { 0x0df6, 0x0e56 }, /*                 Thai_lekhok ๖ THAI DIGIT SIX */
  { 0x0df7, 0x0e57 }, /*                Thai_lekchet ๗ THAI DIGIT SEVEN */
  { 0x0df8, 0x0e58 }, /*                Thai_lekpaet ๘ THAI DIGIT EIGHT */
  { 0x0df9, 0x0e59 }, /*                 Thai_lekkao ๙ THAI DIGIT NINE */
  { 0x0ea1, 0x3131 }, /*               Hangul_Kiyeog ㄱ HANGUL LETTER KIYEOK */
  { 0x0ea2, 0x3132 }, /*          Hangul_SsangKiyeog ㄲ HANGUL LETTER SSANGKIYEOK */
  { 0x0ea3, 0x3133 }, /*           Hangul_KiyeogSios ㄳ HANGUL LETTER KIYEOK-SIOS */
  { 0x0ea4, 0x3134 }, /*                Hangul_Nieun ㄴ HANGUL LETTER NIEUN */
  { 0x0ea5, 0x3135 }, /*           Hangul_NieunJieuj ㄵ HANGUL LETTER NIEUN-CIEUC */
  { 0x0ea6, 0x3136 }, /*           Hangul_NieunHieuh ㄶ HANGUL LETTER NIEUN-HIEUH */
  { 0x0ea7, 0x3137 }, /*               Hangul_Dikeud ㄷ HANGUL LETTER TIKEUT */
  { 0x0ea8, 0x3138 }, /*          Hangul_SsangDikeud ㄸ HANGUL LETTER SSANGTIKEUT */
  { 0x0ea9, 0x3139 }, /*                Hangul_Rieul ㄹ HANGUL LETTER RIEUL */
  { 0x0eaa, 0x313a }, /*          Hangul_RieulKiyeog ㄺ HANGUL LETTER RIEUL-KIYEOK */
  { 0x0eab, 0x313b }, /*           Hangul_RieulMieum ㄻ HANGUL LETTER RIEUL-MIEUM */
  { 0x0eac, 0x313c }, /*           Hangul_RieulPieub ㄼ HANGUL LETTER RIEUL-PIEUP */
  { 0x0ead, 0x313d }, /*            Hangul_RieulSios ㄽ HANGUL LETTER RIEUL-SIOS */
  { 0x0eae, 0x313e }, /*           Hangul_RieulTieut ㄾ HANGUL LETTER RIEUL-THIEUTH */
  { 0x0eaf, 0x313f }, /*          Hangul_RieulPhieuf ㄿ HANGUL LETTER RIEUL-PHIEUPH */
  { 0x0eb0, 0x3140 }, /*           Hangul_RieulHieuh ㅀ HANGUL LETTER RIEUL-HIEUH */
  { 0x0eb1, 0x3141 }, /*                Hangul_Mieum ㅁ HANGUL LETTER MIEUM */
  { 0x0eb2, 0x3142 }, /*                Hangul_Pieub ㅂ HANGUL LETTER PIEUP */
  { 0x0eb3, 0x3143 }, /*           Hangul_SsangPieub ㅃ HANGUL LETTER SSANGPIEUP */
  { 0x0eb4, 0x3144 }, /*            Hangul_PieubSios ㅄ HANGUL LETTER PIEUP-SIOS */
  { 0x0eb5, 0x3145 }, /*                 Hangul_Sios ㅅ HANGUL LETTER SIOS */
  { 0x0eb6, 0x3146 }, /*            Hangul_SsangSios ㅆ HANGUL LETTER SSANGSIOS */
  { 0x0eb7, 0x3147 }, /*                Hangul_Ieung ㅇ HANGUL LETTER IEUNG */
  { 0x0eb8, 0x3148 }, /*                Hangul_Jieuj ㅈ HANGUL LETTER CIEUC */
  { 0x0eb9, 0x3149 }, /*           Hangul_SsangJieuj ㅉ HANGUL LETTER SSANGCIEUC */
  { 0x0eba, 0x314a }, /*                Hangul_Cieuc ㅊ HANGUL LETTER CHIEUCH */
  { 0x0ebb, 0x314b }, /*               Hangul_Khieuq ㅋ HANGUL LETTER KHIEUKH */
  { 0x0ebc, 0x314c }, /*                Hangul_Tieut ㅌ HANGUL LETTER THIEUTH */
  { 0x0ebd, 0x314d }, /*               Hangul_Phieuf ㅍ HANGUL LETTER PHIEUPH */
  { 0x0ebe, 0x314e }, /*                Hangul_Hieuh ㅎ HANGUL LETTER HIEUH */
  { 0x0ebf, 0x314f }, /*                    Hangul_A ㅏ HANGUL LETTER A */
  { 0x0ec0, 0x3150 }, /*                   Hangul_AE ㅐ HANGUL LETTER AE */
  { 0x0ec1, 0x3151 }, /*                   Hangul_YA ㅑ HANGUL LETTER YA */
  { 0x0ec2, 0x3152 }, /*                  Hangul_YAE ㅒ HANGUL LETTER YAE */
  { 0x0ec3, 0x3153 }, /*                   Hangul_EO ㅓ HANGUL LETTER EO */
  { 0x0ec4, 0x3154 }, /*                    Hangul_E ㅔ HANGUL LETTER E */
  { 0x0ec5, 0x3155 }, /*                  Hangul_YEO ㅕ HANGUL LETTER YEO */
  { 0x0ec6, 0x3156 }, /*                   Hangul_YE ㅖ HANGUL LETTER YE */
  { 0x0ec7, 0x3157 }, /*                    Hangul_O ㅗ HANGUL LETTER O */
  { 0x0ec8, 0x3158 }, /*                   Hangul_WA ㅘ HANGUL LETTER WA */
  { 0x0ec9, 0x3159 }, /*                  Hangul_WAE ㅙ HANGUL LETTER WAE */
  { 0x0eca, 0x315a }, /*                   Hangul_OE ㅚ HANGUL LETTER OE */
  { 0x0ecb, 0x315b }, /*                   Hangul_YO ㅛ HANGUL LETTER YO */
  { 0x0ecc, 0x315c }, /*                    Hangul_U ㅜ HANGUL LETTER U */
  { 0x0ecd, 0x315d }, /*                  Hangul_WEO ㅝ HANGUL LETTER WEO */
  { 0x0ece, 0x315e }, /*                   Hangul_WE ㅞ HANGUL LETTER WE */
  { 0x0ecf, 0x315f }, /*                   Hangul_WI ㅟ HANGUL LETTER WI */
  { 0x0ed0, 0x3160 }, /*                   Hangul_YU ㅠ HANGUL LETTER YU */
  { 0x0ed1, 0x3161 }, /*                   Hangul_EU ㅡ HANGUL LETTER EU */
  { 0x0ed2, 0x3162 }, /*                   Hangul_YI ㅢ HANGUL LETTER YI */
  { 0x0ed3, 0x3163 }, /*                    Hangul_I ㅣ HANGUL LETTER I */
  { 0x0ed4, 0x11a8 }, /*             Hangul_J_Kiyeog ᆨ HANGUL JONGSEONG KIYEOK */
  { 0x0ed5, 0x11a9 }, /*        Hangul_J_SsangKiyeog ᆩ HANGUL JONGSEONG SSANGKIYEOK */
  { 0x0ed6, 0x11aa }, /*         Hangul_J_KiyeogSios ᆪ HANGUL JONGSEONG KIYEOK-SIOS */
  { 0x0ed7, 0x11ab }, /*              Hangul_J_Nieun ᆫ HANGUL JONGSEONG NIEUN */
  { 0x0ed8, 0x11ac }, /*         Hangul_J_NieunJieuj ᆬ HANGUL JONGSEONG NIEUN-CIEUC */
  { 0x0ed9, 0x11ad }, /*         Hangul_J_NieunHieuh ᆭ HANGUL JONGSEONG NIEUN-HIEUH */
  { 0x0eda, 0x11ae }, /*             Hangul_J_Dikeud ᆮ HANGUL JONGSEONG TIKEUT */
  { 0x0edb, 0x11af }, /*              Hangul_J_Rieul ᆯ HANGUL JONGSEONG RIEUL */
  { 0x0edc, 0x11b0 }, /*        Hangul_J_RieulKiyeog ᆰ HANGUL JONGSEONG RIEUL-KIYEOK */
  { 0x0edd, 0x11b1 }, /*         Hangul_J_RieulMieum ᆱ HANGUL JONGSEONG RIEUL-MIEUM */
  { 0x0ede, 0x11b2 }, /*         Hangul_J_RieulPieub ᆲ HANGUL JONGSEONG RIEUL-PIEUP */
  { 0x0edf, 0x11b3 }, /*          Hangul_J_RieulSios ᆳ HANGUL JONGSEONG RIEUL-SIOS */
  { 0x0ee0, 0x11b4 }, /*         Hangul_J_RieulTieut ᆴ HANGUL JONGSEONG RIEUL-THIEUTH */
  { 0x0ee1, 0x11b5 }, /*        Hangul_J_RieulPhieuf ᆵ HANGUL JONGSEONG RIEUL-PHIEUPH */
  { 0x0ee2, 0x11b6 }, /*         Hangul_J_RieulHieuh ᆶ HANGUL JONGSEONG RIEUL-HIEUH */
  { 0x0ee3, 0x11b7 }, /*              Hangul_J_Mieum ᆷ HANGUL JONGSEONG MIEUM */
  { 0x0ee4, 0x11b8 }, /*              Hangul_J_Pieub ᆸ HANGUL JONGSEONG PIEUP */
  { 0x0ee5, 0x11b9 }, /*          Hangul_J_PieubSios ᆹ HANGUL JONGSEONG PIEUP-SIOS */
  { 0x0ee6, 0x11ba }, /*               Hangul_J_Sios ᆺ HANGUL JONGSEONG SIOS */
  { 0x0ee7, 0x11bb }, /*          Hangul_J_SsangSios ᆻ HANGUL JONGSEONG SSANGSIOS */
  { 0x0ee8, 0x11bc }, /*              Hangul_J_Ieung ᆼ HANGUL JONGSEONG IEUNG */
  { 0x0ee9, 0x11bd }, /*              Hangul_J_Jieuj ᆽ HANGUL JONGSEONG CIEUC */
  { 0x0eea, 0x11be }, /*              Hangul_J_Cieuc ᆾ HANGUL JONGSEONG CHIEUCH */
  { 0x0eeb, 0x11bf }, /*             Hangul_J_Khieuq ᆿ HANGUL JONGSEONG KHIEUKH */
  { 0x0eec, 0x11c0 }, /*              Hangul_J_Tieut ᇀ HANGUL JONGSEONG THIEUTH */
  { 0x0eed, 0x11c1 }, /*             Hangul_J_Phieuf ᇁ HANGUL JONGSEONG PHIEUPH */
  { 0x0eee, 0x11c2 }, /*              Hangul_J_Hieuh ᇂ HANGUL JONGSEONG HIEUH */
  { 0x0eef, 0x316d }, /*     Hangul_RieulYeorinHieuh ㅭ HANGUL LETTER RIEUL-YEORINHIEUH */
  { 0x0ef0, 0x3171 }, /*    Hangul_SunkyeongeumMieum ㅱ HANGUL LETTER KAPYEOUNMIEUM */
  { 0x0ef1, 0x3178 }, /*    Hangul_SunkyeongeumPieub ㅸ HANGUL LETTER KAPYEOUNPIEUP */
  { 0x0ef2, 0x317f }, /*              Hangul_PanSios ㅿ HANGUL LETTER PANSIOS */
/*  0x0ef3                  Hangul_KkogjiDalrinIeung ? ??? */
  { 0x0ef4, 0x3184 }, /*   Hangul_SunkyeongeumPhieuf ㆄ HANGUL LETTER KAPYEOUNPHIEUPH */
  { 0x0ef5, 0x3186 }, /*          Hangul_YeorinHieuh ㆆ HANGUL LETTER YEORINHIEUH */
  { 0x0ef6, 0x318d }, /*                Hangul_AraeA ㆍ HANGUL LETTER ARAEA */
  { 0x0ef7, 0x318e }, /*               Hangul_AraeAE ㆎ HANGUL LETTER ARAEAE */
  { 0x0ef8, 0x11eb }, /*            Hangul_J_PanSios ᇫ HANGUL JONGSEONG PANSIOS */
/*  0x0ef9                Hangul_J_KkogjiDalrinIeung ? ??? */
  { 0x0efa, 0x11f9 }, /*        Hangul_J_YeorinHieuh ᇹ HANGUL JONGSEONG YEORINHIEUH */
  { 0x0eff, 0x20a9 }, /*                  Korean_Won ₩ WON SIGN */
  { 0x13bc, 0x0152 }, /*                          OE Œ LATIN CAPITAL LIGATURE OE */
  { 0x13bd, 0x0153 }, /*                          oe œ LATIN SMALL LIGATURE OE */
  { 0x13be, 0x0178 }, /*                  Ydiaeresis Ÿ LATIN CAPITAL LETTER Y WITH DIAERESIS */
  { 0x20a0, 0x20a0 }, /*                     EcuSign ₠ EURO-CURRENCY SIGN */
  { 0x20a1, 0x20a1 }, /*                   ColonSign ₡ COLON SIGN */
  { 0x20a2, 0x20a2 }, /*                CruzeiroSign ₢ CRUZEIRO SIGN */
  { 0x20a3, 0x20a3 }, /*                  FFrancSign ₣ FRENCH FRANC SIGN */
  { 0x20a4, 0x20a4 }, /*                    LiraSign ₤ LIRA SIGN */
  { 0x20a5, 0x20a5 }, /*                    MillSign ₥ MILL SIGN */
  { 0x20a6, 0x20a6 }, /*                   NairaSign ₦ NAIRA SIGN */
  { 0x20a7, 0x20a7 }, /*                  PesetaSign ₧ PESETA SIGN */
  { 0x20a8, 0x20a8 }, /*                   RupeeSign ₨ RUPEE SIGN */
  { 0x20a9, 0x20a9 }, /*                     WonSign ₩ WON SIGN */
  { 0x20aa, 0x20aa }, /*               NewSheqelSign ₪ NEW SHEQEL SIGN */
  { 0x20ab, 0x20ab }, /*                    DongSign ₫ DONG SIGN */
  { 0x20ac, 0x20ac }, /*                    EuroSign € EURO SIGN */

  /* Following items added to SCIM, not in the xterm table */

  /* Function keys */
  { 0xFF09, 0x0009 },
  { 0xFF0A, 0x000a },
  { 0xFF0D, 0x000d },

  /* Numeric keypad */
  { 0xFF80 /* Space */, ' ' },
  { 0xFF89 /* Space */, 0x09 },
  { 0xFF8D /* Enter */, 0x0d },
  { 0xFFAA /* Multiply */, '*' },
  { 0xFFAB /* Add */, '+' },
  { 0xFFAD /* Subtract */, '-' },
  { 0xFFAE /* Decimal */, '.' },
  { 0xFFAF /* Divide */, '/' },
  { 0xFFB0 /* 0 */, '0' },
  { 0xFFB1 /* 1 */, '1' },
  { 0xFFB2 /* 2 */, '2' },
  { 0xFFB3 /* 3 */, '3' },
  { 0xFFB4 /* 4 */, '4' },
  { 0xFFB5 /* 5 */, '5' },
  { 0xFFB6 /* 6 */, '6' },
  { 0xFFB7 /* 7 */, '7' },
  { 0xFFB8 /* 8 */, '8' },
  { 0xFFB9 /* 9 */, '9' },
  { 0xFFBD /* Equal */, '=' },  

  /* End numeric keypad */
};

static __KeyName __scim_keys_by_code [] = {
  { 0x0020, "space" },
  { 0x0021, "exclam" },
  { 0x0022, "quotedbl" },
  { 0x0023, "numbersign" },
  { 0x0024, "dollar" },
  { 0x0025, "percent" },
  { 0x0026, "ampersand" },
  { 0x0027, "apostrophe" },
  { 0x0027, "quoteright" },
  { 0x0028, "parenleft" },
  { 0x0029, "parenright" },
  { 0x002a, "asterisk" },
  { 0x002b, "plus" },
  { 0x002c, "comma" },
  { 0x002d, "minus" },
  { 0x002e, "period" },
  { 0x002f, "slash" },
  { 0x0030, "0" },
  { 0x0031, "1" },
  { 0x0032, "2" },
  { 0x0033, "3" },
  { 0x0034, "4" },
  { 0x0035, "5" },
  { 0x0036, "6" },
  { 0x0037, "7" },
  { 0x0038, "8" },
  { 0x0039, "9" },
  { 0x003a, "colon" },
  { 0x003b, "semicolon" },
  { 0x003c, "less" },
  { 0x003d, "equal" },
  { 0x003e, "greater" },
  { 0x003f, "question" },
  { 0x0040, "at" },
  { 0x0041, "A" },
  { 0x0042, "B" },
  { 0x0043, "C" },
  { 0x0044, "D" },
  { 0x0045, "E" },
  { 0x0046, "F" },
  { 0x0047, "G" },
  { 0x0048, "H" },
  { 0x0049, "I" },
  { 0x004a, "J" },
  { 0x004b, "K" },
  { 0x004c, "L" },
  { 0x004d, "M" },
  { 0x004e, "N" },
  { 0x004f, "O" },
  { 0x0050, "P" },
  { 0x0051, "Q" },
  { 0x0052, "R" },
  { 0x0053, "S" },
  { 0x0054, "T" },
  { 0x0055, "U" },
  { 0x0056, "V" },
  { 0x0057, "W" },
  { 0x0058, "X" },
  { 0x0059, "Y" },
  { 0x005a, "Z" },
  { 0x005b, "bracketleft" },
  { 0x005c, "backslash" },
  { 0x005d, "bracketright" },
  { 0x005e, "asciicircum" },
  { 0x005f, "underscore" },
  { 0x0060, "grave" },
  { 0x0060, "quoteleft" },
  { 0x0061, "a" },
  { 0x0062, "b" },
  { 0x0063, "c" },
  { 0x0064, "d" },
  { 0x0065, "e" },
  { 0x0066, "f" },
  { 0x0067, "g" },
  { 0x0068, "h" },
  { 0x0069, "i" },
  { 0x006a, "j" },
  { 0x006b, "k" },
  { 0x006c, "l" },
  { 0x006d, "m" },
  { 0x006e, "n" },
  { 0x006f, "o" },
  { 0x0070, "p" },
  { 0x0071, "q" },
  { 0x0072, "r" },
  { 0x0073, "s" },
  { 0x0074, "t" },
  { 0x0075, "u" },
  { 0x0076, "v" },
  { 0x0077, "w" },
  { 0x0078, "x" },
  { 0x0079, "y" },
  { 0x007a, "z" },
  { 0x007b, "braceleft" },
  { 0x007c, "bar" },
  { 0x007d, "braceright" },
  { 0x007e, "asciitilde" },
  { 0x00a0, "nobreakspace" },
  { 0x00a1, "exclamdown" },
  { 0x00a2, "cent" },
  { 0x00a3, "sterling" },
  { 0x00a4, "currency" },
  { 0x00a5, "yen" },
  { 0x00a6, "brokenbar" },
  { 0x00a7, "section" },
  { 0x00a8, "diaeresis" },
  { 0x00a9, "copyright" },
  { 0x00aa, "ordfeminine" },
  { 0x00ab, "guillemotleft" },
  { 0x00ac, "notsign" },
  { 0x00ad, "hyphen" },
  { 0x00ae, "registered" },
  { 0x00af, "macron" },
  { 0x00b0, "degree" },
  { 0x00b1, "plusminus" },
  { 0x00b2, "twosuperior" },
  { 0x00b3, "threesuperior" },
  { 0x00b4, "acute" },
  { 0x00b5, "mu" },
  { 0x00b6, "paragraph" },
  { 0x00b7, "periodcentered" },
  { 0x00b8, "cedilla" },
  { 0x00b9, "onesuperior" },
  { 0x00ba, "masculine" },
  { 0x00bb, "guillemotright" },
  { 0x00bc, "onequarter" },
  { 0x00bd, "onehalf" },
  { 0x00be, "threequarters" },
  { 0x00bf, "questiondown" },
  { 0x00c0, "Agrave" },
  { 0x00c1, "Aacute" },
  { 0x00c2, "Acircumflex" },
  { 0x00c3, "Atilde" },
  { 0x00c4, "Adiaeresis" },
  { 0x00c5, "Aring" },
  { 0x00c6, "AE" },
  { 0x00c7, "Ccedilla" },
  { 0x00c8, "Egrave" },
  { 0x00c9, "Eacute" },
  { 0x00ca, "Ecircumflex" },
  { 0x00cb, "Ediaeresis" },
  { 0x00cc, "Igrave" },
  { 0x00cd, "Iacute" },
  { 0x00ce, "Icircumflex" },
  { 0x00cf, "Idiaeresis" },
  { 0x00d0, "ETH" },
  { 0x00d0, "Eth" },
  { 0x00d1, "Ntilde" },
  { 0x00d2, "Ograve" },
  { 0x00d3, "Oacute" },
  { 0x00d4, "Ocircumflex" },
  { 0x00d5, "Otilde" },
  { 0x00d6, "Odiaeresis" },
  { 0x00d7, "multiply" },
  { 0x00d8, "Ooblique" },
  { 0x00d9, "Ugrave" },
  { 0x00da, "Uacute" },
  { 0x00db, "Ucircumflex" },
  { 0x00dc, "Udiaeresis" },
  { 0x00dd, "Yacute" },
  { 0x00de, "THORN" },
  { 0x00de, "Thorn" },
  { 0x00df, "ssharp" },
  { 0x00e0, "agrave" },
  { 0x00e1, "aacute" },
  { 0x00e2, "acircumflex" },
  { 0x00e3, "atilde" },
  { 0x00e4, "adiaeresis" },
  { 0x00e5, "aring" },
  { 0x00e6, "ae" },
  { 0x00e7, "ccedilla" },
  { 0x00e8, "egrave" },
  { 0x00e9, "eacute" },
  { 0x00ea, "ecircumflex" },
  { 0x00eb, "ediaeresis" },
  { 0x00ec, "igrave" },
  { 0x00ed, "iacute" },
  { 0x00ee, "icircumflex" },
  { 0x00ef, "idiaeresis" },
  { 0x00f0, "eth" },
  { 0x00f1, "ntilde" },
  { 0x00f2, "ograve" },
  { 0x00f3, "oacute" },
  { 0x00f4, "ocircumflex" },
  { 0x00f5, "otilde" },
  { 0x00f6, "odiaeresis" },
  { 0x00f7, "division" },
  { 0x00f8, "oslash" },
  { 0x00f9, "ugrave" },
  { 0x00fa, "uacute" },
  { 0x00fb, "ucircumflex" },
  { 0x00fc, "udiaeresis" },
  { 0x00fd, "yacute" },
  { 0x00fe, "thorn" },
  { 0x00ff, "ydiaeresis" },
  { 0x01a1, "Aogonek" },
  { 0x01a2, "breve" },
  { 0x01a3, "Lstroke" },
  { 0x01a5, "Lcaron" },
  { 0x01a6, "Sacute" },
  { 0x01a9, "Scaron" },
  { 0x01aa, "Scedilla" },
  { 0x01ab, "Tcaron" },
  { 0x01ac, "Zacute" },
  { 0x01ae, "Zcaron" },
  { 0x01af, "Zabovedot" },
  { 0x01b1, "aogonek" },
  { 0x01b2, "ogonek" },
  { 0x01b3, "lstroke" },
  { 0x01b5, "lcaron" },
  { 0x01b6, "sacute" },
  { 0x01b7, "caron" },
  { 0x01b9, "scaron" },
  { 0x01ba, "scedilla" },
  { 0x01bb, "tcaron" },
  { 0x01bc, "zacute" },
  { 0x01bd, "doubleacute" },
  { 0x01be, "zcaron" },
  { 0x01bf, "zabovedot" },
  { 0x01c0, "Racute" },
  { 0x01c3, "Abreve" },
  { 0x01c5, "Lacute" },
  { 0x01c6, "Cacute" },
  { 0x01c8, "Ccaron" },
  { 0x01ca, "Eogonek" },
  { 0x01cc, "Ecaron" },
  { 0x01cf, "Dcaron" },
  { 0x01d0, "Dstroke" },
  { 0x01d1, "Nacute" },
  { 0x01d2, "Ncaron" },
  { 0x01d5, "Odoubleacute" },
  { 0x01d8, "Rcaron" },
  { 0x01d9, "Uring" },
  { 0x01db, "Udoubleacute" },
  { 0x01de, "Tcedilla" },
  { 0x01e0, "racute" },
  { 0x01e3, "abreve" },
  { 0x01e5, "lacute" },
  { 0x01e6, "cacute" },
  { 0x01e8, "ccaron" },
  { 0x01ea, "eogonek" },
  { 0x01ec, "ecaron" },
  { 0x01ef, "dcaron" },
  { 0x01f0, "dstroke" },
  { 0x01f1, "nacute" },
  { 0x01f2, "ncaron" },
  { 0x01f5, "odoubleacute" },
  { 0x01f8, "rcaron" },
  { 0x01f9, "uring" },
  { 0x01fb, "udoubleacute" },
  { 0x01fe, "tcedilla" },
  { 0x01ff, "abovedot" },
  { 0x02a1, "Hstroke" },
  { 0x02a6, "Hcircumflex" },
  { 0x02a9, "Iabovedot" },
  { 0x02ab, "Gbreve" },
  { 0x02ac, "Jcircumflex" },
  { 0x02b1, "hstroke" },
  { 0x02b6, "hcircumflex" },
  { 0x02b9, "idotless" },
  { 0x02bb, "gbreve" },
  { 0x02bc, "jcircumflex" },
  { 0x02c5, "Cabovedot" },
  { 0x02c6, "Ccircumflex" },
  { 0x02d5, "Gabovedot" },
  { 0x02d8, "Gcircumflex" },
  { 0x02dd, "Ubreve" },
  { 0x02de, "Scircumflex" },
  { 0x02e5, "cabovedot" },
  { 0x02e6, "ccircumflex" },
  { 0x02f5, "gabovedot" },
  { 0x02f8, "gcircumflex" },
  { 0x02fd, "ubreve" },
  { 0x02fe, "scircumflex" },
  { 0x03a2, "kappa" },
  { 0x03a2, "kra" },
  { 0x03a3, "Rcedilla" },
  { 0x03a5, "Itilde" },
  { 0x03a6, "Lcedilla" },
  { 0x03aa, "Emacron" },
  { 0x03ab, "Gcedilla" },
  { 0x03ac, "Tslash" },
  { 0x03b3, "rcedilla" },
  { 0x03b5, "itilde" },
  { 0x03b6, "lcedilla" },
  { 0x03ba, "emacron" },
  { 0x03bb, "gcedilla" },
  { 0x03bc, "tslash" },
  { 0x03bd, "ENG" },
  { 0x03bf, "eng" },
  { 0x03c0, "Amacron" },
  { 0x03c7, "Iogonek" },
  { 0x03cc, "Eabovedot" },
  { 0x03cf, "Imacron" },
  { 0x03d1, "Ncedilla" },
  { 0x03d2, "Omacron" },
  { 0x03d3, "Kcedilla" },
  { 0x03d9, "Uogonek" },
  { 0x03dd, "Utilde" },
  { 0x03de, "Umacron" },
  { 0x03e0, "amacron" },
  { 0x03e7, "iogonek" },
  { 0x03ec, "eabovedot" },
  { 0x03ef, "imacron" },
  { 0x03f1, "ncedilla" },
  { 0x03f2, "omacron" },
  { 0x03f3, "kcedilla" },
  { 0x03f9, "uogonek" },
  { 0x03fd, "utilde" },
  { 0x03fe, "umacron" },
  { 0x047e, "overline" },
  { 0x04a1, "kana_fullstop" },
  { 0x04a2, "kana_openingbracket" },
  { 0x04a3, "kana_closingbracket" },
  { 0x04a4, "kana_comma" },
  { 0x04a5, "kana_conjunctive" },
  { 0x04a5, "kana_middledot" },
  { 0x04a6, "kana_WO" },
  { 0x04a7, "kana_a" },
  { 0x04a8, "kana_i" },
  { 0x04a9, "kana_u" },
  { 0x04aa, "kana_e" },
  { 0x04ab, "kana_o" },
  { 0x04ac, "kana_ya" },
  { 0x04ad, "kana_yu" },
  { 0x04ae, "kana_yo" },
  { 0x04af, "kana_tsu" },
  { 0x04af, "kana_tu" },
  { 0x04b0, "prolongedsound" },
  { 0x04b1, "kana_A" },
  { 0x04b2, "kana_I" },
  { 0x04b3, "kana_U" },
  { 0x04b4, "kana_E" },
  { 0x04b5, "kana_O" },
  { 0x04b6, "kana_KA" },
  { 0x04b7, "kana_KI" },
  { 0x04b8, "kana_KU" },
  { 0x04b9, "kana_KE" },
  { 0x04ba, "kana_KO" },
  { 0x04bb, "kana_SA" },
  { 0x04bc, "kana_SHI" },
  { 0x04bd, "kana_SU" },
  { 0x04be, "kana_SE" },
  { 0x04bf, "kana_SO" },
  { 0x04c0, "kana_TA" },
  { 0x04c1, "kana_CHI" },
  { 0x04c1, "kana_TI" },
  { 0x04c2, "kana_TSU" },
  { 0x04c2, "kana_TU" },
  { 0x04c3, "kana_TE" },
  { 0x04c4, "kana_TO" },
  { 0x04c5, "kana_NA" },
  { 0x04c6, "kana_NI" },
  { 0x04c7, "kana_NU" },
  { 0x04c8, "kana_NE" },
  { 0x04c9, "kana_NO" },
  { 0x04ca, "kana_HA" },
  { 0x04cb, "kana_HI" },
  { 0x04cc, "kana_FU" },
  { 0x04cc, "kana_HU" },
  { 0x04cd, "kana_HE" },
  { 0x04ce, "kana_HO" },
  { 0x04cf, "kana_MA" },
  { 0x04d0, "kana_MI" },
  { 0x04d1, "kana_MU" },
  { 0x04d2, "kana_ME" },
  { 0x04d3, "kana_MO" },
  { 0x04d4, "kana_YA" },
  { 0x04d5, "kana_YU" },
  { 0x04d6, "kana_YO" },
  { 0x04d7, "kana_RA" },
  { 0x04d8, "kana_RI" },
  { 0x04d9, "kana_RU" },
  { 0x04da, "kana_RE" },
  { 0x04db, "kana_RO" },
  { 0x04dc, "kana_WA" },
  { 0x04dd, "kana_N" },
  { 0x04de, "voicedsound" },
  { 0x04df, "semivoicedsound" },
  { 0x05ac, "Arabic_comma" },
  { 0x05bb, "Arabic_semicolon" },
  { 0x05bf, "Arabic_question_mark" },
  { 0x05c1, "Arabic_hamza" },
  { 0x05c2, "Arabic_maddaonalef" },
  { 0x05c3, "Arabic_hamzaonalef" },
  { 0x05c4, "Arabic_hamzaonwaw" },
  { 0x05c5, "Arabic_hamzaunderalef" },
  { 0x05c6, "Arabic_hamzaonyeh" },
  { 0x05c7, "Arabic_alef" },
  { 0x05c8, "Arabic_beh" },
  { 0x05c9, "Arabic_tehmarbuta" },
  { 0x05ca, "Arabic_teh" },
  { 0x05cb, "Arabic_theh" },
  { 0x05cc, "Arabic_jeem" },
  { 0x05cd, "Arabic_hah" },
  { 0x05ce, "Arabic_khah" },
  { 0x05cf, "Arabic_dal" },
  { 0x05d0, "Arabic_thal" },
  { 0x05d1, "Arabic_ra" },
  { 0x05d2, "Arabic_zain" },
  { 0x05d3, "Arabic_seen" },
  { 0x05d4, "Arabic_sheen" },
  { 0x05d5, "Arabic_sad" },
  { 0x05d6, "Arabic_dad" },
  { 0x05d7, "Arabic_tah" },
  { 0x05d8, "Arabic_zah" },
  { 0x05d9, "Arabic_ain" },
  { 0x05da, "Arabic_ghain" },
  { 0x05e0, "Arabic_tatweel" },
  { 0x05e1, "Arabic_feh" },
  { 0x05e2, "Arabic_qaf" },
  { 0x05e3, "Arabic_kaf" },
  { 0x05e4, "Arabic_lam" },
  { 0x05e5, "Arabic_meem" },
  { 0x05e6, "Arabic_noon" },
  { 0x05e7, "Arabic_ha" },
  { 0x05e7, "Arabic_heh" },
  { 0x05e8, "Arabic_waw" },
  { 0x05e9, "Arabic_alefmaksura" },
  { 0x05ea, "Arabic_yeh" },
  { 0x05eb, "Arabic_fathatan" },
  { 0x05ec, "Arabic_dammatan" },
  { 0x05ed, "Arabic_kasratan" },
  { 0x05ee, "Arabic_fatha" },
  { 0x05ef, "Arabic_damma" },
  { 0x05f0, "Arabic_kasra" },
  { 0x05f1, "Arabic_shadda" },
  { 0x05f2, "Arabic_sukun" },
  { 0x06a1, "Serbian_dje" },
  { 0x06a2, "Macedonia_gje" },
  { 0x06a3, "Cyrillic_io" },
  { 0x06a4, "Ukrainian_ie" },
  { 0x06a4, "Ukranian_je" },
  { 0x06a5, "Macedonia_dse" },
  { 0x06a6, "Ukrainian_i" },
  { 0x06a6, "Ukranian_i" },
  { 0x06a7, "Ukrainian_yi" },
  { 0x06a7, "Ukranian_yi" },
  { 0x06a8, "Cyrillic_je" },
  { 0x06a8, "Serbian_je" },
  { 0x06a9, "Cyrillic_lje" },
  { 0x06a9, "Serbian_lje" },
  { 0x06aa, "Cyrillic_nje" },
  { 0x06aa, "Serbian_nje" },
  { 0x06ab, "Serbian_tshe" },
  { 0x06ac, "Macedonia_kje" },
  { 0x06ae, "Byelorussian_shortu" },
  { 0x06af, "Cyrillic_dzhe" },
  { 0x06af, "Serbian_dze" },
  { 0x06b0, "numerosign" },
  { 0x06b1, "Serbian_DJE" },
  { 0x06b2, "Macedonia_GJE" },
  { 0x06b3, "Cyrillic_IO" },
  { 0x06b4, "Ukrainian_IE" },
  { 0x06b4, "Ukranian_JE" },
  { 0x06b5, "Macedonia_DSE" },
  { 0x06b6, "Ukrainian_I" },
  { 0x06b6, "Ukranian_I" },
  { 0x06b7, "Ukrainian_YI" },
  { 0x06b7, "Ukranian_YI" },
  { 0x06b8, "Cyrillic_JE" },
  { 0x06b8, "Serbian_JE" },
  { 0x06b9, "Cyrillic_LJE" },
  { 0x06b9, "Serbian_LJE" },
  { 0x06ba, "Cyrillic_NJE" },
  { 0x06ba, "Serbian_NJE" },
  { 0x06bb, "Serbian_TSHE" },
  { 0x06bc, "Macedonia_KJE" },
  { 0x06be, "Byelorussian_SHORTU" },
  { 0x06bf, "Cyrillic_DZHE" },
  { 0x06bf, "Serbian_DZE" },
  { 0x06c0, "Cyrillic_yu" },
  { 0x06c1, "Cyrillic_a" },
  { 0x06c2, "Cyrillic_be" },
  { 0x06c3, "Cyrillic_tse" },
  { 0x06c4, "Cyrillic_de" },
  { 0x06c5, "Cyrillic_ie" },
  { 0x06c6, "Cyrillic_ef" },
  { 0x06c7, "Cyrillic_ghe" },
  { 0x06c8, "Cyrillic_ha" },
  { 0x06c9, "Cyrillic_i" },
  { 0x06ca, "Cyrillic_shorti" },
  { 0x06cb, "Cyrillic_ka" },
  { 0x06cc, "Cyrillic_el" },
  { 0x06cd, "Cyrillic_em" },
  { 0x06ce, "Cyrillic_en" },
  { 0x06cf, "Cyrillic_o" },
  { 0x06d0, "Cyrillic_pe" },
  { 0x06d1, "Cyrillic_ya" },
  { 0x06d2, "Cyrillic_er" },
  { 0x06d3, "Cyrillic_es" },
  { 0x06d4, "Cyrillic_te" },
  { 0x06d5, "Cyrillic_u" },
  { 0x06d6, "Cyrillic_zhe" },
  { 0x06d7, "Cyrillic_ve" },
  { 0x06d8, "Cyrillic_softsign" },
  { 0x06d9, "Cyrillic_yeru" },
  { 0x06da, "Cyrillic_ze" },
  { 0x06db, "Cyrillic_sha" },
  { 0x06dc, "Cyrillic_e" },
  { 0x06dd, "Cyrillic_shcha" },
  { 0x06de, "Cyrillic_che" },
  { 0x06df, "Cyrillic_hardsign" },
  { 0x06e0, "Cyrillic_YU" },
  { 0x06e1, "Cyrillic_A" },
  { 0x06e2, "Cyrillic_BE" },
  { 0x06e3, "Cyrillic_TSE" },
  { 0x06e4, "Cyrillic_DE" },
  { 0x06e5, "Cyrillic_IE" },
  { 0x06e6, "Cyrillic_EF" },
  { 0x06e7, "Cyrillic_GHE" },
  { 0x06e8, "Cyrillic_HA" },
  { 0x06e9, "Cyrillic_I" },
  { 0x06ea, "Cyrillic_SHORTI" },
  { 0x06eb, "Cyrillic_KA" },
  { 0x06ec, "Cyrillic_EL" },
  { 0x06ed, "Cyrillic_EM" },
  { 0x06ee, "Cyrillic_EN" },
  { 0x06ef, "Cyrillic_O" },
  { 0x06f0, "Cyrillic_PE" },
  { 0x06f1, "Cyrillic_YA" },
  { 0x06f2, "Cyrillic_ER" },
  { 0x06f3, "Cyrillic_ES" },
  { 0x06f4, "Cyrillic_TE" },
  { 0x06f5, "Cyrillic_U" },
  { 0x06f6, "Cyrillic_ZHE" },
  { 0x06f7, "Cyrillic_VE" },
  { 0x06f8, "Cyrillic_SOFTSIGN" },
  { 0x06f9, "Cyrillic_YERU" },
  { 0x06fa, "Cyrillic_ZE" },
  { 0x06fb, "Cyrillic_SHA" },
  { 0x06fc, "Cyrillic_E" },
  { 0x06fd, "Cyrillic_SHCHA" },
  { 0x06fe, "Cyrillic_CHE" },
  { 0x06ff, "Cyrillic_HARDSIGN" },
  { 0x07a1, "Greek_ALPHAaccent" },
  { 0x07a2, "Greek_EPSILONaccent" },
  { 0x07a3, "Greek_ETAaccent" },
  { 0x07a4, "Greek_IOTAaccent" },
  { 0x07a5, "Greek_IOTAdiaeresis" },
  { 0x07a7, "Greek_OMICRONaccent" },
  { 0x07a8, "Greek_UPSILONaccent" },
  { 0x07a9, "Greek_UPSILONdieresis" },
  { 0x07ab, "Greek_OMEGAaccent" },
  { 0x07ae, "Greek_accentdieresis" },
  { 0x07af, "Greek_horizbar" },
  { 0x07b1, "Greek_alphaaccent" },
  { 0x07b2, "Greek_epsilonaccent" },
  { 0x07b3, "Greek_etaaccent" },
  { 0x07b4, "Greek_iotaaccent" },
  { 0x07b5, "Greek_iotadieresis" },
  { 0x07b6, "Greek_iotaaccentdieresis" },
  { 0x07b7, "Greek_omicronaccent" },
  { 0x07b8, "Greek_upsilonaccent" },
  { 0x07b9, "Greek_upsilondieresis" },
  { 0x07ba, "Greek_upsilonaccentdieresis" },
  { 0x07bb, "Greek_omegaaccent" },
  { 0x07c1, "Greek_ALPHA" },
  { 0x07c2, "Greek_BETA" },
  { 0x07c3, "Greek_GAMMA" },
  { 0x07c4, "Greek_DELTA" },
  { 0x07c5, "Greek_EPSILON" },
  { 0x07c6, "Greek_ZETA" },
  { 0x07c7, "Greek_ETA" },
  { 0x07c8, "Greek_THETA" },
  { 0x07c9, "Greek_IOTA" },
  { 0x07ca, "Greek_KAPPA" },
  { 0x07cb, "Greek_LAMBDA" },
  { 0x07cb, "Greek_LAMDA" },
  { 0x07cc, "Greek_MU" },
  { 0x07cd, "Greek_NU" },
  { 0x07ce, "Greek_XI" },
  { 0x07cf, "Greek_OMICRON" },
  { 0x07d0, "Greek_PI" },
  { 0x07d1, "Greek_RHO" },
  { 0x07d2, "Greek_SIGMA" },
  { 0x07d4, "Greek_TAU" },
  { 0x07d5, "Greek_UPSILON" },
  { 0x07d6, "Greek_PHI" },
  { 0x07d7, "Greek_CHI" },
  { 0x07d8, "Greek_PSI" },
  { 0x07d9, "Greek_OMEGA" },
  { 0x07e1, "Greek_alpha" },
  { 0x07e2, "Greek_beta" },
  { 0x07e3, "Greek_gamma" },
  { 0x07e4, "Greek_delta" },
  { 0x07e5, "Greek_epsilon" },
  { 0x07e6, "Greek_zeta" },
  { 0x07e7, "Greek_eta" },
  { 0x07e8, "Greek_theta" },
  { 0x07e9, "Greek_iota" },
  { 0x07ea, "Greek_kappa" },
  { 0x07eb, "Greek_lambda" },
  { 0x07eb, "Greek_lamda" },
  { 0x07ec, "Greek_mu" },
  { 0x07ed, "Greek_nu" },
  { 0x07ee, "Greek_xi" },
  { 0x07ef, "Greek_omicron" },
  { 0x07f0, "Greek_pi" },
  { 0x07f1, "Greek_rho" },
  { 0x07f2, "Greek_sigma" },
  { 0x07f3, "Greek_finalsmallsigma" },
  { 0x07f4, "Greek_tau" },
  { 0x07f5, "Greek_upsilon" },
  { 0x07f6, "Greek_phi" },
  { 0x07f7, "Greek_chi" },
  { 0x07f8, "Greek_psi" },
  { 0x07f9, "Greek_omega" },
  { 0x08a1, "leftradical" },
  { 0x08a2, "topleftradical" },
  { 0x08a3, "horizconnector" },
  { 0x08a4, "topintegral" },
  { 0x08a5, "botintegral" },
  { 0x08a6, "vertconnector" },
  { 0x08a7, "topleftsqbracket" },
  { 0x08a8, "botleftsqbracket" },
  { 0x08a9, "toprightsqbracket" },
  { 0x08aa, "botrightsqbracket" },
  { 0x08ab, "topleftparens" },
  { 0x08ac, "botleftparens" },
  { 0x08ad, "toprightparens" },
  { 0x08ae, "botrightparens" },
  { 0x08af, "leftmiddlecurlybrace" },
  { 0x08b0, "rightmiddlecurlybrace" },
  { 0x08b1, "topleftsummation" },
  { 0x08b2, "botleftsummation" },
  { 0x08b3, "topvertsummationconnector" },
  { 0x08b4, "botvertsummationconnector" },
  { 0x08b5, "toprightsummation" },
  { 0x08b6, "botrightsummation" },
  { 0x08b7, "rightmiddlesummation" },
  { 0x08bc, "lessthanequal" },
  { 0x08bd, "notequal" },
  { 0x08be, "greaterthanequal" },
  { 0x08bf, "integral" },
  { 0x08c0, "therefore" },
  { 0x08c1, "variation" },
  { 0x08c2, "infinity" },
  { 0x08c5, "nabla" },
  { 0x08c8, "approximate" },
  { 0x08c9, "similarequal" },
  { 0x08cd, "ifonlyif" },
  { 0x08ce, "implies" },
  { 0x08cf, "identical" },
  { 0x08d6, "radical" },
  { 0x08da, "includedin" },
  { 0x08db, "includes" },
  { 0x08dc, "intersection" },
  { 0x08dd, "union" },
  { 0x08de, "logicaland" },
  { 0x08df, "logicalor" },
  { 0x08ef, "partialderivative" },
  { 0x08f6, "function" },
  { 0x08fb, "leftarrow" },
  { 0x08fc, "uparrow" },
  { 0x08fd, "rightarrow" },
  { 0x08fe, "downarrow" },
  { 0x09df, "blank" },
  { 0x09e0, "soliddiamond" },
  { 0x09e1, "checkerboard" },
  { 0x09e2, "ht" },
  { 0x09e3, "ff" },
  { 0x09e4, "cr" },
  { 0x09e5, "lf" },
  { 0x09e8, "nl" },
  { 0x09e9, "vt" },
  { 0x09ea, "lowrightcorner" },
  { 0x09eb, "uprightcorner" },
  { 0x09ec, "upleftcorner" },
  { 0x09ed, "lowleftcorner" },
  { 0x09ee, "crossinglines" },
  { 0x09ef, "horizlinescan1" },
  { 0x09f0, "horizlinescan3" },
  { 0x09f1, "horizlinescan5" },
  { 0x09f2, "horizlinescan7" },
  { 0x09f3, "horizlinescan9" },
  { 0x09f4, "leftt" },
  { 0x09f5, "rightt" },
  { 0x09f6, "bott" },
  { 0x09f7, "topt" },
  { 0x09f8, "vertbar" },
  { 0x0aa1, "emspace" },
  { 0x0aa2, "enspace" },
  { 0x0aa3, "em3space" },
  { 0x0aa4, "em4space" },
  { 0x0aa5, "digitspace" },
  { 0x0aa6, "punctspace" },
  { 0x0aa7, "thinspace" },
  { 0x0aa8, "hairspace" },
  { 0x0aa9, "emdash" },
  { 0x0aaa, "endash" },
  { 0x0aac, "signifblank" },
  { 0x0aae, "ellipsis" },
  { 0x0aaf, "doubbaselinedot" },
  { 0x0ab0, "onethird" },
  { 0x0ab1, "twothirds" },
  { 0x0ab2, "onefifth" },
  { 0x0ab3, "twofifths" },
  { 0x0ab4, "threefifths" },
  { 0x0ab5, "fourfifths" },
  { 0x0ab6, "onesixth" },
  { 0x0ab7, "fivesixths" },
  { 0x0ab8, "careof" },
  { 0x0abb, "figdash" },
  { 0x0abc, "leftanglebracket" },
  { 0x0abd, "decimalpoint" },
  { 0x0abe, "rightanglebracket" },
  { 0x0abf, "marker" },
  { 0x0ac3, "oneeighth" },
  { 0x0ac4, "threeeighths" },
  { 0x0ac5, "fiveeighths" },
  { 0x0ac6, "seveneighths" },
  { 0x0ac9, "trademark" },
  { 0x0aca, "signaturemark" },
  { 0x0acb, "trademarkincircle" },
  { 0x0acc, "leftopentriangle" },
  { 0x0acd, "rightopentriangle" },
  { 0x0ace, "emopencircle" },
  { 0x0acf, "emopenrectangle" },
  { 0x0ad0, "leftsinglequotemark" },
  { 0x0ad1, "rightsinglequotemark" },
  { 0x0ad2, "leftdoublequotemark" },
  { 0x0ad3, "rightdoublequotemark" },
  { 0x0ad4, "prescription" },
  { 0x0ad6, "minutes" },
  { 0x0ad7, "seconds" },
  { 0x0ad9, "latincross" },
  { 0x0ada, "hexagram" },
  { 0x0adb, "filledrectbullet" },
  { 0x0adc, "filledlefttribullet" },
  { 0x0add, "filledrighttribullet" },
  { 0x0ade, "emfilledcircle" },
  { 0x0adf, "emfilledrect" },
  { 0x0ae0, "enopencircbullet" },
  { 0x0ae1, "enopensquarebullet" },
  { 0x0ae2, "openrectbullet" },
  { 0x0ae3, "opentribulletup" },
  { 0x0ae4, "opentribulletdown" },
  { 0x0ae5, "openstar" },
  { 0x0ae6, "enfilledcircbullet" },
  { 0x0ae7, "enfilledsqbullet" },
  { 0x0ae8, "filledtribulletup" },
  { 0x0ae9, "filledtribulletdown" },
  { 0x0aea, "leftpointer" },
  { 0x0aeb, "rightpointer" },
  { 0x0aec, "club" },
  { 0x0aed, "diamond" },
  { 0x0aee, "heart" },
  { 0x0af0, "maltesecross" },
  { 0x0af1, "dagger" },
  { 0x0af2, "doubledagger" },
  { 0x0af3, "checkmark" },
  { 0x0af4, "ballotcross" },
  { 0x0af5, "musicalsharp" },
  { 0x0af6, "musicalflat" },
  { 0x0af7, "malesymbol" },
  { 0x0af8, "femalesymbol" },
  { 0x0af9, "telephone" },
  { 0x0afa, "telephonerecorder" },
  { 0x0afb, "phonographcopyright" },
  { 0x0afc, "caret" },
  { 0x0afd, "singlelowquotemark" },
  { 0x0afe, "doublelowquotemark" },
  { 0x0aff, "cursor" },
  { 0x0ba3, "leftcaret" },
  { 0x0ba6, "rightcaret" },
  { 0x0ba8, "downcaret" },
  { 0x0ba9, "upcaret" },
  { 0x0bc0, "overbar" },
  { 0x0bc2, "downtack" },
  { 0x0bc3, "upshoe" },
  { 0x0bc4, "downstile" },
  { 0x0bc6, "underbar" },
  { 0x0bca, "jot" },
  { 0x0bcc, "quad" },
  { 0x0bce, "uptack" },
  { 0x0bcf, "circle" },
  { 0x0bd3, "upstile" },
  { 0x0bd6, "downshoe" },
  { 0x0bd8, "rightshoe" },
  { 0x0bda, "leftshoe" },
  { 0x0bdc, "lefttack" },
  { 0x0bfc, "righttack" },
  { 0x0cdf, "hebrew_doublelowline" },
  { 0x0ce0, "hebrew_aleph" },
  { 0x0ce1, "hebrew_bet" },
  { 0x0ce1, "hebrew_beth" },
  { 0x0ce2, "hebrew_gimel" },
  { 0x0ce2, "hebrew_gimmel" },
  { 0x0ce3, "hebrew_dalet" },
  { 0x0ce3, "hebrew_daleth" },
  { 0x0ce4, "hebrew_he" },
  { 0x0ce5, "hebrew_waw" },
  { 0x0ce6, "hebrew_zain" },
  { 0x0ce6, "hebrew_zayin" },
  { 0x0ce7, "hebrew_chet" },
  { 0x0ce7, "hebrew_het" },
  { 0x0ce8, "hebrew_tet" },
  { 0x0ce8, "hebrew_teth" },
  { 0x0ce9, "hebrew_yod" },
  { 0x0cea, "hebrew_finalkaph" },
  { 0x0ceb, "hebrew_kaph" },
  { 0x0cec, "hebrew_lamed" },
  { 0x0ced, "hebrew_finalmem" },
  { 0x0cee, "hebrew_mem" },
  { 0x0cef, "hebrew_finalnun" },
  { 0x0cf0, "hebrew_nun" },
  { 0x0cf1, "hebrew_samech" },
  { 0x0cf1, "hebrew_samekh" },
  { 0x0cf2, "hebrew_ayin" },
  { 0x0cf3, "hebrew_finalpe" },
  { 0x0cf4, "hebrew_pe" },
  { 0x0cf5, "hebrew_finalzade" },
  { 0x0cf5, "hebrew_finalzadi" },
  { 0x0cf6, "hebrew_zade" },
  { 0x0cf6, "hebrew_zadi" },
  { 0x0cf7, "hebrew_kuf" },
  { 0x0cf7, "hebrew_qoph" },
  { 0x0cf8, "hebrew_resh" },
  { 0x0cf9, "hebrew_shin" },
  { 0x0cfa, "hebrew_taf" },
  { 0x0cfa, "hebrew_taw" },
  { 0x0da1, "Thai_kokai" },
  { 0x0da2, "Thai_khokhai" },
  { 0x0da3, "Thai_khokhuat" },
  { 0x0da4, "Thai_khokhwai" },
  { 0x0da5, "Thai_khokhon" },
  { 0x0da6, "Thai_khorakhang" },
  { 0x0da7, "Thai_ngongu" },
  { 0x0da8, "Thai_chochan" },
  { 0x0da9, "Thai_choching" },
  { 0x0daa, "Thai_chochang" },
  { 0x0dab, "Thai_soso" },
  { 0x0dac, "Thai_chochoe" },
  { 0x0dad, "Thai_yoying" },
  { 0x0dae, "Thai_dochada" },
  { 0x0daf, "Thai_topatak" },
  { 0x0db0, "Thai_thothan" },
  { 0x0db1, "Thai_thonangmontho" },
  { 0x0db2, "Thai_thophuthao" },
  { 0x0db3, "Thai_nonen" },
  { 0x0db4, "Thai_dodek" },
  { 0x0db5, "Thai_totao" },
  { 0x0db6, "Thai_thothung" },
  { 0x0db7, "Thai_thothahan" },
  { 0x0db8, "Thai_thothong" },
  { 0x0db9, "Thai_nonu" },
  { 0x0dba, "Thai_bobaimai" },
  { 0x0dbb, "Thai_popla" },
  { 0x0dbc, "Thai_phophung" },
  { 0x0dbd, "Thai_fofa" },
  { 0x0dbe, "Thai_phophan" },
  { 0x0dbf, "Thai_fofan" },
  { 0x0dc0, "Thai_phosamphao" },
  { 0x0dc1, "Thai_moma" },
  { 0x0dc2, "Thai_yoyak" },
  { 0x0dc3, "Thai_rorua" },
  { 0x0dc4, "Thai_ru" },
  { 0x0dc5, "Thai_loling" },
  { 0x0dc6, "Thai_lu" },
  { 0x0dc7, "Thai_wowaen" },
  { 0x0dc8, "Thai_sosala" },
  { 0x0dc9, "Thai_sorusi" },
  { 0x0dca, "Thai_sosua" },
  { 0x0dcb, "Thai_hohip" },
  { 0x0dcc, "Thai_lochula" },
  { 0x0dcd, "Thai_oang" },
  { 0x0dce, "Thai_honokhuk" },
  { 0x0dcf, "Thai_paiyannoi" },
  { 0x0dd0, "Thai_saraa" },
  { 0x0dd1, "Thai_maihanakat" },
  { 0x0dd2, "Thai_saraaa" },
  { 0x0dd3, "Thai_saraam" },
  { 0x0dd4, "Thai_sarai" },
  { 0x0dd5, "Thai_saraii" },
  { 0x0dd6, "Thai_saraue" },
  { 0x0dd7, "Thai_sarauee" },
  { 0x0dd8, "Thai_sarau" },
  { 0x0dd9, "Thai_sarauu" },
  { 0x0dda, "Thai_phinthu" },
  { 0x0dde, "Thai_maihanakat_maitho" },
  { 0x0ddf, "Thai_baht" },
  { 0x0de0, "Thai_sarae" },
  { 0x0de1, "Thai_saraae" },
  { 0x0de2, "Thai_sarao" },
  { 0x0de3, "Thai_saraaimaimuan" },
  { 0x0de4, "Thai_saraaimaimalai" },
  { 0x0de5, "Thai_lakkhangyao" },
  { 0x0de6, "Thai_maiyamok" },
  { 0x0de7, "Thai_maitaikhu" },
  { 0x0de8, "Thai_maiek" },
  { 0x0de9, "Thai_maitho" },
  { 0x0dea, "Thai_maitri" },
  { 0x0deb, "Thai_maichattawa" },
  { 0x0dec, "Thai_thanthakhat" },
  { 0x0ded, "Thai_nikhahit" },
  { 0x0df0, "Thai_leksun" },
  { 0x0df1, "Thai_leknung" },
  { 0x0df2, "Thai_leksong" },
  { 0x0df3, "Thai_leksam" },
  { 0x0df4, "Thai_leksi" },
  { 0x0df5, "Thai_lekha" },
  { 0x0df6, "Thai_lekhok" },
  { 0x0df7, "Thai_lekchet" },
  { 0x0df8, "Thai_lekpaet" },
  { 0x0df9, "Thai_lekkao" },
  { 0x0ea1, "Hangul_Kiyeog" },
  { 0x0ea2, "Hangul_SsangKiyeog" },
  { 0x0ea3, "Hangul_KiyeogSios" },
  { 0x0ea4, "Hangul_Nieun" },
  { 0x0ea5, "Hangul_NieunJieuj" },
  { 0x0ea6, "Hangul_NieunHieuh" },
  { 0x0ea7, "Hangul_Dikeud" },
  { 0x0ea8, "Hangul_SsangDikeud" },
  { 0x0ea9, "Hangul_Rieul" },
  { 0x0eaa, "Hangul_RieulKiyeog" },
  { 0x0eab, "Hangul_RieulMieum" },
  { 0x0eac, "Hangul_RieulPieub" },
  { 0x0ead, "Hangul_RieulSios" },
  { 0x0eae, "Hangul_RieulTieut" },
  { 0x0eaf, "Hangul_RieulPhieuf" },
  { 0x0eb0, "Hangul_RieulHieuh" },
  { 0x0eb1, "Hangul_Mieum" },
  { 0x0eb2, "Hangul_Pieub" },
  { 0x0eb3, "Hangul_SsangPieub" },
  { 0x0eb4, "Hangul_PieubSios" },
  { 0x0eb5, "Hangul_Sios" },
  { 0x0eb6, "Hangul_SsangSios" },
  { 0x0eb7, "Hangul_Ieung" },
  { 0x0eb8, "Hangul_Jieuj" },
  { 0x0eb9, "Hangul_SsangJieuj" },
  { 0x0eba, "Hangul_Cieuc" },
  { 0x0ebb, "Hangul_Khieuq" },
  { 0x0ebc, "Hangul_Tieut" },
  { 0x0ebd, "Hangul_Phieuf" },
  { 0x0ebe, "Hangul_Hieuh" },
  { 0x0ebf, "Hangul_A" },
  { 0x0ec0, "Hangul_AE" },
  { 0x0ec1, "Hangul_YA" },
  { 0x0ec2, "Hangul_YAE" },
  { 0x0ec3, "Hangul_EO" },
  { 0x0ec4, "Hangul_E" },
  { 0x0ec5, "Hangul_YEO" },
  { 0x0ec6, "Hangul_YE" },
  { 0x0ec7, "Hangul_O" },
  { 0x0ec8, "Hangul_WA" },
  { 0x0ec9, "Hangul_WAE" },
  { 0x0eca, "Hangul_OE" },
  { 0x0ecb, "Hangul_YO" },
  { 0x0ecc, "Hangul_U" },
  { 0x0ecd, "Hangul_WEO" },
  { 0x0ece, "Hangul_WE" },
  { 0x0ecf, "Hangul_WI" },
  { 0x0ed0, "Hangul_YU" },
  { 0x0ed1, "Hangul_EU" },
  { 0x0ed2, "Hangul_YI" },
  { 0x0ed3, "Hangul_I" },
  { 0x0ed4, "Hangul_J_Kiyeog" },
  { 0x0ed5, "Hangul_J_SsangKiyeog" },
  { 0x0ed6, "Hangul_J_KiyeogSios" },
  { 0x0ed7, "Hangul_J_Nieun" },
  { 0x0ed8, "Hangul_J_NieunJieuj" },
  { 0x0ed9, "Hangul_J_NieunHieuh" },
  { 0x0eda, "Hangul_J_Dikeud" },
  { 0x0edb, "Hangul_J_Rieul" },
  { 0x0edc, "Hangul_J_RieulKiyeog" },
  { 0x0edd, "Hangul_J_RieulMieum" },
  { 0x0ede, "Hangul_J_RieulPieub" },
  { 0x0edf, "Hangul_J_RieulSios" },
  { 0x0ee0, "Hangul_J_RieulTieut" },
  { 0x0ee1, "Hangul_J_RieulPhieuf" },
  { 0x0ee2, "Hangul_J_RieulHieuh" },
  { 0x0ee3, "Hangul_J_Mieum" },
  { 0x0ee4, "Hangul_J_Pieub" },
  { 0x0ee5, "Hangul_J_PieubSios" },
  { 0x0ee6, "Hangul_J_Sios" },
  { 0x0ee7, "Hangul_J_SsangSios" },
  { 0x0ee8, "Hangul_J_Ieung" },
  { 0x0ee9, "Hangul_J_Jieuj" },
  { 0x0eea, "Hangul_J_Cieuc" },
  { 0x0eeb, "Hangul_J_Khieuq" },
  { 0x0eec, "Hangul_J_Tieut" },
  { 0x0eed, "Hangul_J_Phieuf" },
  { 0x0eee, "Hangul_J_Hieuh" },
  { 0x0eef, "Hangul_RieulYeorinHieuh" },
  { 0x0ef0, "Hangul_SunkyeongeumMieum" },
  { 0x0ef1, "Hangul_SunkyeongeumPieub" },
  { 0x0ef2, "Hangul_PanSios" },
  { 0x0ef3, "Hangul_KkogjiDalrinIeung" },
  { 0x0ef4, "Hangul_SunkyeongeumPhieuf" },
  { 0x0ef5, "Hangul_YeorinHieuh" },
  { 0x0ef6, "Hangul_AraeA" },
  { 0x0ef7, "Hangul_AraeAE" },
  { 0x0ef8, "Hangul_J_PanSios" },
  { 0x0ef9, "Hangul_J_KkogjiDalrinIeung" },
  { 0x0efa, "Hangul_J_YeorinHieuh" },
  { 0x0eff, "Korean_Won" },
  { 0x13bc, "OE" },
  { 0x13bd, "oe" },
  { 0x13be, "Ydiaeresis" },

  { 0x1e9f, "combining_tilde" },
  { 0x1ef2, "combining_grave" },
  { 0x1ef3, "combining_acute" },
  { 0x1efa, "Ohorn" },
  { 0x1efb, "ohorn" },
  { 0x1efc, "Uhorn" },
  { 0x1efd, "uhorn" },
  { 0x1efe, "combining_hook" },
  { 0x1eff, "combining_belowdot" },

  { 0x20a0, "EcuSign" },
  { 0x20a1, "ColonSign" },
  { 0x20a2, "CruzeiroSign" },
  { 0x20a3, "FFrancSign" },
  { 0x20a4, "LiraSign" },
  { 0x20a5, "MillSign" },
  { 0x20a6, "NairaSign" },
  { 0x20a7, "PesetaSign" },
  { 0x20a8, "RupeeSign" },
  { 0x20a9, "WonSign" },
  { 0x20aa, "NewSheqelSign" },
  { 0x20ab, "DongSign" },
  { 0x20ac, "EuroSign" },
  { 0xfd01, "3270_Duplicate" },
  { 0xfd02, "3270_FieldMark" },
  { 0xfd03, "3270_Right2" },
  { 0xfd04, "3270_Left2" },
  { 0xfd05, "3270_BackTab" },
  { 0xfd06, "3270_EraseEOF" },
  { 0xfd07, "3270_EraseInput" },
  { 0xfd08, "3270_Reset" },
  { 0xfd09, "3270_Quit" },
  { 0xfd0a, "3270_PA1" },
  { 0xfd0b, "3270_PA2" },
  { 0xfd0c, "3270_PA3" },
  { 0xfd0d, "3270_Test" },
  { 0xfd0e, "3270_Attn" },
  { 0xfd0f, "3270_CursorBlink" },
  { 0xfd10, "3270_AltCursor" },
  { 0xfd11, "3270_KeyClick" },
  { 0xfd12, "3270_Jump" },
  { 0xfd13, "3270_Ident" },
  { 0xfd14, "3270_Rule" },
  { 0xfd15, "3270_Copy" },
  { 0xfd16, "3270_Play" },
  { 0xfd17, "3270_Setup" },
  { 0xfd18, "3270_Record" },
  { 0xfd19, "3270_ChangeScreen" },
  { 0xfd1a, "3270_DeleteWord" },
  { 0xfd1b, "3270_ExSelect" },
  { 0xfd1c, "3270_CursorSelect" },
  { 0xfd1d, "3270_PrintScreen" },
  { 0xfd1e, "3270_Enter" },
  { 0xfe01, "ISO_Lock" },
  { 0xfe02, "ISO_Level2_Latch" },
  { 0xfe03, "ISO_Level3_Shift" },
  { 0xfe04, "ISO_Level3_Latch" },
  { 0xfe05, "ISO_Level3_Lock" },
  { 0xfe06, "ISO_Group_Latch" },
  { 0xfe07, "ISO_Group_Lock" },
  { 0xfe08, "ISO_Next_Group" },
  { 0xfe09, "ISO_Next_Group_Lock" },
  { 0xfe0a, "ISO_Prev_Group" },
  { 0xfe0b, "ISO_Prev_Group_Lock" },
  { 0xfe0c, "ISO_First_Group" },
  { 0xfe0d, "ISO_First_Group_Lock" },
  { 0xfe0e, "ISO_Last_Group" },
  { 0xfe0f, "ISO_Last_Group_Lock" },
  { 0xfe20, "ISO_Left_Tab" },
  { 0xfe21, "ISO_Move_Line_Up" },
  { 0xfe22, "ISO_Move_Line_Down" },
  { 0xfe23, "ISO_Partial_Line_Up" },
  { 0xfe24, "ISO_Partial_Line_Down" },
  { 0xfe25, "ISO_Partial_Space_Left" },
  { 0xfe26, "ISO_Partial_Space_Right" },
  { 0xfe27, "ISO_Set_Margin_Left" },
  { 0xfe28, "ISO_Set_Margin_Right" },
  { 0xfe29, "ISO_Release_Margin_Left" },
  { 0xfe2a, "ISO_Release_Margin_Right" },
  { 0xfe2b, "ISO_Release_Both_Margins" },
  { 0xfe2c, "ISO_Fast_Cursor_Left" },
  { 0xfe2d, "ISO_Fast_Cursor_Right" },
  { 0xfe2e, "ISO_Fast_Cursor_Up" },
  { 0xfe2f, "ISO_Fast_Cursor_Down" },
  { 0xfe30, "ISO_Continuous_Underline" },
  { 0xfe31, "ISO_Discontinuous_Underline" },
  { 0xfe32, "ISO_Emphasize" },
  { 0xfe33, "ISO_Center_Object" },
  { 0xfe34, "ISO_Enter" },
  { 0xfe50, "dead_grave" },
  { 0xfe51, "dead_acute" },
  { 0xfe52, "dead_circumflex" },
  { 0xfe53, "dead_tilde" },
  { 0xfe54, "dead_macron" },
  { 0xfe55, "dead_breve" },
  { 0xfe56, "dead_abovedot" },
  { 0xfe57, "dead_diaeresis" },
  { 0xfe58, "dead_abovering" },
  { 0xfe59, "dead_doubleacute" },
  { 0xfe5a, "dead_caron" },
  { 0xfe5b, "dead_cedilla" },
  { 0xfe5c, "dead_ogonek" },
  { 0xfe5d, "dead_iota" },
  { 0xfe5e, "dead_voiced_sound" },
  { 0xfe5f, "dead_semivoiced_sound" },
  { 0xfe60, "dead_belowdot" },
  { 0xfe61, "dead_hook" },
  { 0xfe62, "dead_horn" },
  { 0xfe70, "AccessX_Enable" },
  { 0xfe71, "AccessX_Feedback_Enable" },
  { 0xfe72, "RepeatKeys_Enable" },
  { 0xfe73, "SlowKeys_Enable" },
  { 0xfe74, "BounceKeys_Enable" },
  { 0xfe75, "StickyKeys_Enable" },
  { 0xfe76, "MouseKeys_Enable" },
  { 0xfe77, "MouseKeys_Accel_Enable" },
  { 0xfe78, "Overlay1_Enable" },
  { 0xfe79, "Overlay2_Enable" },
  { 0xfe7a, "AudibleBell_Enable" },
  { 0xfed0, "First_Virtual_Screen" },
  { 0xfed1, "Prev_Virtual_Screen" },
  { 0xfed2, "Next_Virtual_Screen" },
  { 0xfed4, "Last_Virtual_Screen" },
  { 0xfed5, "Terminate_Server" },
  { 0xfee0, "Pointer_Left" },
  { 0xfee1, "Pointer_Right" },
  { 0xfee2, "Pointer_Up" },
  { 0xfee3, "Pointer_Down" },
  { 0xfee4, "Pointer_UpLeft" },
  { 0xfee5, "Pointer_UpRight" },
  { 0xfee6, "Pointer_DownLeft" },
  { 0xfee7, "Pointer_DownRight" },
  { 0xfee8, "Pointer_Button_Dflt" },
  { 0xfee9, "Pointer_Button1" },
  { 0xfeea, "Pointer_Button2" },
  { 0xfeeb, "Pointer_Button3" },
  { 0xfeec, "Pointer_Button4" },
  { 0xfeed, "Pointer_Button5" },
  { 0xfeee, "Pointer_DblClick_Dflt" },
  { 0xfeef, "Pointer_DblClick1" },
  { 0xfef0, "Pointer_DblClick2" },
  { 0xfef1, "Pointer_DblClick3" },
  { 0xfef2, "Pointer_DblClick4" },
  { 0xfef3, "Pointer_DblClick5" },
  { 0xfef4, "Pointer_Drag_Dflt" },
  { 0xfef5, "Pointer_Drag1" },
  { 0xfef6, "Pointer_Drag2" },
  { 0xfef7, "Pointer_Drag3" },
  { 0xfef8, "Pointer_Drag4" },
  { 0xfef9, "Pointer_EnableKeys" },
  { 0xfefa, "Pointer_Accelerate" },
  { 0xfefb, "Pointer_DfltBtnNext" },
  { 0xfefc, "Pointer_DfltBtnPrev" },
  { 0xfefd, "Pointer_Drag5" },
  { 0xff08, "BackSpace" },
  { 0xff09, "Tab" },
  { 0xff0a, "Linefeed" },
  { 0xff0b, "Clear" },
  { 0xff0d, "Return" },
  { 0xff13, "Pause" },
  { 0xff14, "Scroll_Lock" },
  { 0xff15, "Sys_Req" },
  { 0xff1b, "Escape" },
  { 0xff20, "Multi_key" },
  { 0xff21, "Kanji" },
  { 0xff22, "Muhenkan" },
  { 0xff23, "Henkan" },
  { 0xff23, "Henkan_Mode" },
  { 0xff24, "Romaji" },
  { 0xff25, "Hiragana" },
  { 0xff26, "Katakana" },
  { 0xff27, "Hiragana_Katakana" },
  { 0xff28, "Zenkaku" },
  { 0xff29, "Hankaku" },
  { 0xff2a, "Zenkaku_Hankaku" },
  { 0xff2b, "Touroku" },
  { 0xff2c, "Massyo" },
  { 0xff2d, "Kana_Lock" },
  { 0xff2e, "Kana_Shift" },
  { 0xff2f, "Eisu_Shift" },
  { 0xff30, "Eisu_toggle" },
  { 0xff31, "Hangul" },
  { 0xff32, "Hangul_Start" },
  { 0xff33, "Hangul_End" },
  { 0xff34, "Hangul_Hanja" },
  { 0xff35, "Hangul_Jamo" },
  { 0xff36, "Hangul_Romaja" },
  { 0xff37, "Codeinput" },
  { 0xff38, "Hangul_Jeonja" },
  { 0xff39, "Hangul_Banja" },
  { 0xff3a, "Hangul_PreHanja" },
  { 0xff3b, "Hangul_PostHanja" },
  { 0xff3c, "SingleCandidate" },
  { 0xff3d, "MultipleCandidate" },
  { 0xff3e, "PreviousCandidate" },
  { 0xff3f, "Hangul_Special" },
  { 0xff50, "Home" },
  { 0xff51, "Left" },
  { 0xff52, "Up" },
  { 0xff53, "Right" },
  { 0xff54, "Down" },
  { 0xff55, "Page_Up" },
  { 0xff55, "Prior" },
  { 0xff56, "Page_Down" },
  { 0xff56, "Next" },
  { 0xff57, "End" },
  { 0xff58, "Begin" },
  { 0xff60, "Select" },
  { 0xff61, "Print" },
  { 0xff62, "Execute" },
  { 0xff63, "Insert" },
  { 0xff65, "Undo" },
  { 0xff66, "Redo" },
  { 0xff67, "Menu" },
  { 0xff68, "Find" },
  { 0xff69, "Cancel" },
  { 0xff6a, "Help" },
  { 0xff6b, "Break" },
  { 0xff7e, "Mode_switch" },
  { 0xff7e, "Arabic_switch" },
  { 0xff7e, "Greek_switch" },
  { 0xff7e, "Hangul_switch" },
  { 0xff7e, "Hebrew_switch" },
  { 0xff7e, "ISO_Group_Shift" },
  { 0xff7e, "kana_switch" },
  { 0xff7e, "script_switch" },
  { 0xff7f, "Num_Lock" },
  { 0xff80, "KP_Space" },
  { 0xff89, "KP_Tab" },
  { 0xff8d, "KP_Enter" },
  { 0xff91, "KP_F1" },
  { 0xff92, "KP_F2" },
  { 0xff93, "KP_F3" },
  { 0xff94, "KP_F4" },
  { 0xff95, "KP_Home" },
  { 0xff96, "KP_Left" },
  { 0xff97, "KP_Up" },
  { 0xff98, "KP_Right" },
  { 0xff99, "KP_Down" },
  { 0xff9a, "KP_Page_Up" },
  { 0xff9a, "KP_Prior" },
  { 0xff9b, "KP_Page_Down" },
  { 0xff9b, "KP_Next" },
  { 0xff9c, "KP_End" },
  { 0xff9d, "KP_Begin" },
  { 0xff9e, "KP_Insert" },
  { 0xff9f, "KP_Delete" },
  { 0xffaa, "KP_Multiply" },
  { 0xffab, "KP_Add" },
  { 0xffac, "KP_Separator" },
  { 0xffad, "KP_Subtract" },
  { 0xffae, "KP_Decimal" },
  { 0xffaf, "KP_Divide" },
  { 0xffb0, "KP_0" },
  { 0xffb1, "KP_1" },
  { 0xffb2, "KP_2" },
  { 0xffb3, "KP_3" },
  { 0xffb4, "KP_4" },
  { 0xffb5, "KP_5" },
  { 0xffb6, "KP_6" },
  { 0xffb7, "KP_7" },
  { 0xffb8, "KP_8" },
  { 0xffb9, "KP_9" },
  { 0xffbd, "KP_Equal" },
  { 0xffbe, "F1" },
  { 0xffbf, "F2" },
  { 0xffc0, "F3" },
  { 0xffc1, "F4" },
  { 0xffc2, "F5" },
  { 0xffc3, "F6" },
  { 0xffc4, "F7" },
  { 0xffc5, "F8" },
  { 0xffc6, "F9" },
  { 0xffc7, "F10" },
  { 0xffc8, "F11" },
  { 0xffc9, "F12" },
  { 0xffca, "F13" },
  { 0xffcb, "F14" },
  { 0xffcc, "F15" },
  { 0xffcd, "F16" },
  { 0xffce, "F17" },
  { 0xffcf, "F18" },
  { 0xffd0, "F19" },
  { 0xffd1, "F20" },
  { 0xffd2, "F21" },
  { 0xffd3, "F22" },
  { 0xffd4, "F23" },
  { 0xffd5, "F24" },
  { 0xffd6, "F25" },
  { 0xffd7, "F26" },
  { 0xffd8, "F27" },
  { 0xffd9, "F28" },
  { 0xffda, "F29" },
  { 0xffdb, "F30" },
  { 0xffdc, "F31" },
  { 0xffdd, "F32" },
  { 0xffde, "F33" },
  { 0xffdf, "F34" },
  { 0xffe0, "F35" },
  { 0xffe1, "Shift_L" },
  { 0xffe2, "Shift_R" },
  { 0xffe3, "Control_L" },
  { 0xffe4, "Control_R" },
  { 0xffe5, "Caps_Lock" },
  { 0xffe6, "Shift_Lock" },
  { 0xffe7, "Meta_L" },
  { 0xffe8, "Meta_R" },
  { 0xffe9, "Alt_L" },
  { 0xffea, "Alt_R" },
  { 0xffeb, "Super_L" },
  { 0xffec, "Super_R" },
  { 0xffed, "Hyper_L" },
  { 0xffee, "Hyper_R" },
  { 0xffff, "Delete" }
};

static __KeyName __scim_keys_by_name [] = {
  { 0x0030, "0" },
  { 0x0031, "1" },
  { 0x0032, "2" },
  { 0x0033, "3" },
  { 0xFD10, "3270_AltCursor" },
  { 0xFD0E, "3270_Attn" },
  { 0xFD05, "3270_BackTab" },
  { 0xFD19, "3270_ChangeScreen" },
  { 0xFD15, "3270_Copy" },
  { 0xFD0F, "3270_CursorBlink" },
  { 0xFD1C, "3270_CursorSelect" },
  { 0xFD1A, "3270_DeleteWord" },
  { 0xFD01, "3270_Duplicate" },
  { 0xFD1E, "3270_Enter" },
  { 0xFD06, "3270_EraseEOF" },
  { 0xFD07, "3270_EraseInput" },
  { 0xFD1B, "3270_ExSelect" },
  { 0xFD02, "3270_FieldMark" },
  { 0xFD13, "3270_Ident" },
  { 0xFD12, "3270_Jump" },
  { 0xFD11, "3270_KeyClick" },
  { 0xFD04, "3270_Left2" },
  { 0xFD0A, "3270_PA1" },
  { 0xFD0B, "3270_PA2" },
  { 0xFD0C, "3270_PA3" },
  { 0xFD16, "3270_Play" },
  { 0xFD1D, "3270_PrintScreen" },
  { 0xFD09, "3270_Quit" },
  { 0xFD18, "3270_Record" },
  { 0xFD08, "3270_Reset" },
  { 0xFD03, "3270_Right2" },
  { 0xFD14, "3270_Rule" },
  { 0xFD17, "3270_Setup" },
  { 0xFD0D, "3270_Test" },
  { 0x0034, "4" },
  { 0x0035, "5" },
  { 0x0036, "6" },
  { 0x0037, "7" },
  { 0x0038, "8" },
  { 0x0039, "9" },
  { 0x0041, "A" },
  { 0x00C6, "AE" },
  { 0x00C1, "Aacute" },
  { 0x01C3, "Abreve" },
  { 0xFE70, "AccessX_Enable" },
  { 0xFE71, "AccessX_Feedback_Enable" },
  { 0x00C2, "Acircumflex" },
  { 0x00C4, "Adiaeresis" },
  { 0x00C0, "Agrave" },
  { 0xFFE9, "Alt_L" },
  { 0xFFEA, "Alt_R" },
  { 0x03C0, "Amacron" },
  { 0x01A1, "Aogonek" },
  { 0x05D9, "Arabic_ain" },
  { 0x05C7, "Arabic_alef" },
  { 0x05E9, "Arabic_alefmaksura" },
  { 0x05C8, "Arabic_beh" },
  { 0x05AC, "Arabic_comma" },
  { 0x05D6, "Arabic_dad" },
  { 0x05CF, "Arabic_dal" },
  { 0x05EF, "Arabic_damma" },
  { 0x05EC, "Arabic_dammatan" },
  { 0x05EE, "Arabic_fatha" },
  { 0x05EB, "Arabic_fathatan" },
  { 0x05E1, "Arabic_feh" },
  { 0x05DA, "Arabic_ghain" },
  { 0x05E7, "Arabic_ha" },
  { 0x05CD, "Arabic_hah" },
  { 0x05C1, "Arabic_hamza" },
  { 0x05C3, "Arabic_hamzaonalef" },
  { 0x05C4, "Arabic_hamzaonwaw" },
  { 0x05C6, "Arabic_hamzaonyeh" },
  { 0x05C5, "Arabic_hamzaunderalef" },
  { 0x05E7, "Arabic_heh" },
  { 0x05CC, "Arabic_jeem" },
  { 0x05E3, "Arabic_kaf" },
  { 0x05F0, "Arabic_kasra" },
  { 0x05ED, "Arabic_kasratan" },
  { 0x05CE, "Arabic_khah" },
  { 0x05E4, "Arabic_lam" },
  { 0x05C2, "Arabic_maddaonalef" },
  { 0x05E5, "Arabic_meem" },
  { 0x05E6, "Arabic_noon" },
  { 0x05E2, "Arabic_qaf" },
  { 0x05BF, "Arabic_question_mark" },
  { 0x05D1, "Arabic_ra" },
  { 0x05D5, "Arabic_sad" },
  { 0x05D3, "Arabic_seen" },
  { 0x05BB, "Arabic_semicolon" },
  { 0x05F1, "Arabic_shadda" },
  { 0x05D4, "Arabic_sheen" },
  { 0x05F2, "Arabic_sukun" },
  { 0xFF7E, "Arabic_switch" },
  { 0x05D7, "Arabic_tah" },
  { 0x05E0, "Arabic_tatweel" },
  { 0x05CA, "Arabic_teh" },
  { 0x05C9, "Arabic_tehmarbuta" },
  { 0x05D0, "Arabic_thal" },
  { 0x05CB, "Arabic_theh" },
  { 0x05E8, "Arabic_waw" },
  { 0x05EA, "Arabic_yeh" },
  { 0x05D8, "Arabic_zah" },
  { 0x05D2, "Arabic_zain" },
  { 0x00C5, "Aring" },
  { 0x00C3, "Atilde" },
  { 0xFE7A, "AudibleBell_Enable" },
  { 0x0042, "B" },
  { 0xFF08, "BackSpace" },
  { 0xFF58, "Begin" },
  { 0xFE74, "BounceKeys_Enable" },
  { 0xFF6B, "Break" },
  { 0x06BE, "Byelorussian_SHORTU" },
  { 0x06AE, "Byelorussian_shortu" },
  { 0x0043, "C" },
  { 0x02C5, "Cabovedot" },
  { 0x01C6, "Cacute" },
  { 0xFF69, "Cancel" },
  { 0xFFE5, "Caps_Lock" },
  { 0x01C8, "Ccaron" },
  { 0x00C7, "Ccedilla" },
  { 0x02C6, "Ccircumflex" },
  { 0xFF0B, "Clear" },
  { 0xFF37, "Codeinput" },
  { 0x20A1, "ColonSign" },
  { 0xFFE3, "Control_L" },
  { 0xFFE4, "Control_R" },
  { 0x20A2, "CruzeiroSign" },
  { 0x06E1, "Cyrillic_A" },
  { 0x06E2, "Cyrillic_BE" },
  { 0x06FE, "Cyrillic_CHE" },
  { 0x06E4, "Cyrillic_DE" },
  { 0x06BF, "Cyrillic_DZHE" },
  { 0x06FC, "Cyrillic_E" },
  { 0x06E6, "Cyrillic_EF" },
  { 0x06EC, "Cyrillic_EL" },
  { 0x06ED, "Cyrillic_EM" },
  { 0x06EE, "Cyrillic_EN" },
  { 0x06F2, "Cyrillic_ER" },
  { 0x06F3, "Cyrillic_ES" },
  { 0x06E7, "Cyrillic_GHE" },
  { 0x06E8, "Cyrillic_HA" },
  { 0x06FF, "Cyrillic_HARDSIGN" },
  { 0x06E9, "Cyrillic_I" },
  { 0x06E5, "Cyrillic_IE" },
  { 0x06B3, "Cyrillic_IO" },
  { 0x06B8, "Cyrillic_JE" },
  { 0x06EB, "Cyrillic_KA" },
  { 0x06B9, "Cyrillic_LJE" },
  { 0x06BA, "Cyrillic_NJE" },
  { 0x06EF, "Cyrillic_O" },
  { 0x06F0, "Cyrillic_PE" },
  { 0x06FB, "Cyrillic_SHA" },
  { 0x06FD, "Cyrillic_SHCHA" },
  { 0x06EA, "Cyrillic_SHORTI" },
  { 0x06F8, "Cyrillic_SOFTSIGN" },
  { 0x06F4, "Cyrillic_TE" },
  { 0x06E3, "Cyrillic_TSE" },
  { 0x06F5, "Cyrillic_U" },
  { 0x06F7, "Cyrillic_VE" },
  { 0x06F1, "Cyrillic_YA" },
  { 0x06F9, "Cyrillic_YERU" },
  { 0x06E0, "Cyrillic_YU" },
  { 0x06FA, "Cyrillic_ZE" },
  { 0x06F6, "Cyrillic_ZHE" },
  { 0x06C1, "Cyrillic_a" },
  { 0x06C2, "Cyrillic_be" },
  { 0x06DE, "Cyrillic_che" },
  { 0x06C4, "Cyrillic_de" },
  { 0x06AF, "Cyrillic_dzhe" },
  { 0x06DC, "Cyrillic_e" },
  { 0x06C6, "Cyrillic_ef" },
  { 0x06CC, "Cyrillic_el" },
  { 0x06CD, "Cyrillic_em" },
  { 0x06CE, "Cyrillic_en" },
  { 0x06D2, "Cyrillic_er" },
  { 0x06D3, "Cyrillic_es" },
  { 0x06C7, "Cyrillic_ghe" },
  { 0x06C8, "Cyrillic_ha" },
  { 0x06DF, "Cyrillic_hardsign" },
  { 0x06C9, "Cyrillic_i" },
  { 0x06C5, "Cyrillic_ie" },
  { 0x06A3, "Cyrillic_io" },
  { 0x06A8, "Cyrillic_je" },
  { 0x06CB, "Cyrillic_ka" },
  { 0x06A9, "Cyrillic_lje" },
  { 0x06AA, "Cyrillic_nje" },
  { 0x06CF, "Cyrillic_o" },
  { 0x06D0, "Cyrillic_pe" },
  { 0x06DB, "Cyrillic_sha" },
  { 0x06DD, "Cyrillic_shcha" },
  { 0x06CA, "Cyrillic_shorti" },
  { 0x06D8, "Cyrillic_softsign" },
  { 0x06D4, "Cyrillic_te" },
  { 0x06C3, "Cyrillic_tse" },
  { 0x06D5, "Cyrillic_u" },
  { 0x06D7, "Cyrillic_ve" },
  { 0x06D1, "Cyrillic_ya" },
  { 0x06D9, "Cyrillic_yeru" },
  { 0x06C0, "Cyrillic_yu" },
  { 0x06DA, "Cyrillic_ze" },
  { 0x06D6, "Cyrillic_zhe" },
  { 0x0044, "D" },
  { 0x01CF, "Dcaron" },
  { 0xFFFF, "Delete" },
  { 0x20AB, "DongSign" },
  { 0xFF54, "Down" },
  { 0x01D0, "Dstroke" },
  { 0x0045, "E" },
  { 0x03BD, "ENG" },
  { 0x00D0, "ETH" },
  { 0x03CC, "Eabovedot" },
  { 0x00C9, "Eacute" },
  { 0x01CC, "Ecaron" },
  { 0x00CA, "Ecircumflex" },
  { 0x20A0, "EcuSign" },
  { 0x00CB, "Ediaeresis" },
  { 0x00C8, "Egrave" },
  { 0xFF2F, "Eisu_Shift" },
  { 0xFF30, "Eisu_toggle" },
  { 0x03AA, "Emacron" },
  { 0xFF57, "End" },
  { 0x01CA, "Eogonek" },
  { 0xFF1B, "Escape" },
  { 0x00D0, "Eth" },
  { 0x20AC, "EuroSign" },
  { 0xFF62, "Execute" },
  { 0x0046, "F" },
  { 0xFFBE, "F1" },
  { 0xFFC7, "F10" },
  { 0xFFC8, "F11" },
  { 0xFFC9, "F12" },
  { 0xFFCA, "F13" },
  { 0xFFCB, "F14" },
  { 0xFFCC, "F15" },
  { 0xFFCD, "F16" },
  { 0xFFCE, "F17" },
  { 0xFFCF, "F18" },
  { 0xFFD0, "F19" },
  { 0xFFBF, "F2" },
  { 0xFFD1, "F20" },
  { 0xFFD2, "F21" },
  { 0xFFD3, "F22" },
  { 0xFFD4, "F23" },
  { 0xFFD5, "F24" },
  { 0xFFD6, "F25" },
  { 0xFFD7, "F26" },
  { 0xFFD8, "F27" },
  { 0xFFD9, "F28" },
  { 0xFFDA, "F29" },
  { 0xFFC0, "F3" },
  { 0xFFDB, "F30" },
  { 0xFFDC, "F31" },
  { 0xFFDD, "F32" },
  { 0xFFDE, "F33" },
  { 0xFFDF, "F34" },
  { 0xFFE0, "F35" },
  { 0xFFC1, "F4" },
  { 0xFFC2, "F5" },
  { 0xFFC3, "F6" },
  { 0xFFC4, "F7" },
  { 0xFFC5, "F8" },
  { 0xFFC6, "F9" },
  { 0x20A3, "FFrancSign" },
  { 0xFF68, "Find" },
  { 0xFED0, "First_Virtual_Screen" },
  { 0x0047, "G" },
  { 0x02D5, "Gabovedot" },
  { 0x02AB, "Gbreve" },
  { 0x03AB, "Gcedilla" },
  { 0x02D8, "Gcircumflex" },
  { 0x07C1, "Greek_ALPHA" },
  { 0x07A1, "Greek_ALPHAaccent" },
  { 0x07C2, "Greek_BETA" },
  { 0x07D7, "Greek_CHI" },
  { 0x07C4, "Greek_DELTA" },
  { 0x07C5, "Greek_EPSILON" },
  { 0x07A2, "Greek_EPSILONaccent" },
  { 0x07C7, "Greek_ETA" },
  { 0x07A3, "Greek_ETAaccent" },
  { 0x07C3, "Greek_GAMMA" },
  { 0x07C9, "Greek_IOTA" },
  { 0x07A4, "Greek_IOTAaccent" },
  { 0x07A5, "Greek_IOTAdiaeresis" },
  { 0x07CA, "Greek_KAPPA" },
  { 0x07CB, "Greek_LAMBDA" },
  { 0x07CB, "Greek_LAMDA" },
  { 0x07CC, "Greek_MU" },
  { 0x07CD, "Greek_NU" },
  { 0x07D9, "Greek_OMEGA" },
  { 0x07AB, "Greek_OMEGAaccent" },
  { 0x07CF, "Greek_OMICRON" },
  { 0x07A7, "Greek_OMICRONaccent" },
  { 0x07D6, "Greek_PHI" },
  { 0x07D0, "Greek_PI" },
  { 0x07D8, "Greek_PSI" },
  { 0x07D1, "Greek_RHO" },
  { 0x07D2, "Greek_SIGMA" },
  { 0x07D4, "Greek_TAU" },
  { 0x07C8, "Greek_THETA" },
  { 0x07D5, "Greek_UPSILON" },
  { 0x07A8, "Greek_UPSILONaccent" },
  { 0x07A9, "Greek_UPSILONdieresis" },
  { 0x07CE, "Greek_XI" },
  { 0x07C6, "Greek_ZETA" },
  { 0x07AE, "Greek_accentdieresis" },
  { 0x07E1, "Greek_alpha" },
  { 0x07B1, "Greek_alphaaccent" },
  { 0x07E2, "Greek_beta" },
  { 0x07F7, "Greek_chi" },
  { 0x07E4, "Greek_delta" },
  { 0x07E5, "Greek_epsilon" },
  { 0x07B2, "Greek_epsilonaccent" },
  { 0x07E7, "Greek_eta" },
  { 0x07B3, "Greek_etaaccent" },
  { 0x07F3, "Greek_finalsmallsigma" },
  { 0x07E3, "Greek_gamma" },
  { 0x07AF, "Greek_horizbar" },
  { 0x07E9, "Greek_iota" },
  { 0x07B4, "Greek_iotaaccent" },
  { 0x07B6, "Greek_iotaaccentdieresis" },
  { 0x07B5, "Greek_iotadieresis" },
  { 0x07EA, "Greek_kappa" },
  { 0x07EB, "Greek_lambda" },
  { 0x07EB, "Greek_lamda" },
  { 0x07EC, "Greek_mu" },
  { 0x07ED, "Greek_nu" },
  { 0x07F9, "Greek_omega" },
  { 0x07BB, "Greek_omegaaccent" },
  { 0x07EF, "Greek_omicron" },
  { 0x07B7, "Greek_omicronaccent" },
  { 0x07F6, "Greek_phi" },
  { 0x07F0, "Greek_pi" },
  { 0x07F8, "Greek_psi" },
  { 0x07F1, "Greek_rho" },
  { 0x07F2, "Greek_sigma" },
  { 0xFF7E, "Greek_switch" },
  { 0x07F4, "Greek_tau" },
  { 0x07E8, "Greek_theta" },
  { 0x07F5, "Greek_upsilon" },
  { 0x07B8, "Greek_upsilonaccent" },
  { 0x07BA, "Greek_upsilonaccentdieresis" },
  { 0x07B9, "Greek_upsilondieresis" },
  { 0x07EE, "Greek_xi" },
  { 0x07E6, "Greek_zeta" },
  { 0x0048, "H" },
  { 0xFF31, "Hangul" },
  { 0x0EBF, "Hangul_A" },
  { 0x0EC0, "Hangul_AE" },
  { 0x0EF6, "Hangul_AraeA" },
  { 0x0EF7, "Hangul_AraeAE" },
  { 0xFF39, "Hangul_Banja" },
  { 0x0EBA, "Hangul_Cieuc" },
  { 0x0EA7, "Hangul_Dikeud" },
  { 0x0EC4, "Hangul_E" },
  { 0x0EC3, "Hangul_EO" },
  { 0x0ED1, "Hangul_EU" },
  { 0xFF33, "Hangul_End" },
  { 0xFF34, "Hangul_Hanja" },
  { 0x0EBE, "Hangul_Hieuh" },
  { 0x0ED3, "Hangul_I" },
  { 0x0EB7, "Hangul_Ieung" },
  { 0x0EEA, "Hangul_J_Cieuc" },
  { 0x0EDA, "Hangul_J_Dikeud" },
  { 0x0EEE, "Hangul_J_Hieuh" },
  { 0x0EE8, "Hangul_J_Ieung" },
  { 0x0EE9, "Hangul_J_Jieuj" },
  { 0x0EEB, "Hangul_J_Khieuq" },
  { 0x0ED4, "Hangul_J_Kiyeog" },
  { 0x0ED6, "Hangul_J_KiyeogSios" },
  { 0x0EF9, "Hangul_J_KkogjiDalrinIeung" },
  { 0x0EE3, "Hangul_J_Mieum" },
  { 0x0ED7, "Hangul_J_Nieun" },
  { 0x0ED9, "Hangul_J_NieunHieuh" },
  { 0x0ED8, "Hangul_J_NieunJieuj" },
  { 0x0EF8, "Hangul_J_PanSios" },
  { 0x0EED, "Hangul_J_Phieuf" },
  { 0x0EE4, "Hangul_J_Pieub" },
  { 0x0EE5, "Hangul_J_PieubSios" },
  { 0x0EDB, "Hangul_J_Rieul" },
  { 0x0EE2, "Hangul_J_RieulHieuh" },
  { 0x0EDC, "Hangul_J_RieulKiyeog" },
  { 0x0EDD, "Hangul_J_RieulMieum" },
  { 0x0EE1, "Hangul_J_RieulPhieuf" },
  { 0x0EDE, "Hangul_J_RieulPieub" },
  { 0x0EDF, "Hangul_J_RieulSios" },
  { 0x0EE0, "Hangul_J_RieulTieut" },
  { 0x0EE6, "Hangul_J_Sios" },
  { 0x0ED5, "Hangul_J_SsangKiyeog" },
  { 0x0EE7, "Hangul_J_SsangSios" },
  { 0x0EEC, "Hangul_J_Tieut" },
  { 0x0EFA, "Hangul_J_YeorinHieuh" },
  { 0xFF35, "Hangul_Jamo" },
  { 0xFF38, "Hangul_Jeonja" },
  { 0x0EB8, "Hangul_Jieuj" },
  { 0x0EBB, "Hangul_Khieuq" },
  { 0x0EA1, "Hangul_Kiyeog" },
  { 0x0EA3, "Hangul_KiyeogSios" },
  { 0x0EF3, "Hangul_KkogjiDalrinIeung" },
  { 0x0EB1, "Hangul_Mieum" },
  { 0x0EA4, "Hangul_Nieun" },
  { 0x0EA6, "Hangul_NieunHieuh" },
  { 0x0EA5, "Hangul_NieunJieuj" },
  { 0x0EC7, "Hangul_O" },
  { 0x0ECA, "Hangul_OE" },
  { 0x0EF2, "Hangul_PanSios" },
  { 0x0EBD, "Hangul_Phieuf" },
  { 0x0EB2, "Hangul_Pieub" },
  { 0x0EB4, "Hangul_PieubSios" },
  { 0xFF3B, "Hangul_PostHanja" },
  { 0xFF3A, "Hangul_PreHanja" },
  { 0x0EA9, "Hangul_Rieul" },
  { 0x0EB0, "Hangul_RieulHieuh" },
  { 0x0EAA, "Hangul_RieulKiyeog" },
  { 0x0EAB, "Hangul_RieulMieum" },
  { 0x0EAF, "Hangul_RieulPhieuf" },
  { 0x0EAC, "Hangul_RieulPieub" },
  { 0x0EAD, "Hangul_RieulSios" },
  { 0x0EAE, "Hangul_RieulTieut" },
  { 0x0EEF, "Hangul_RieulYeorinHieuh" },
  { 0xFF36, "Hangul_Romaja" },
  { 0x0EB5, "Hangul_Sios" },
  { 0xFF3F, "Hangul_Special" },
  { 0x0EA8, "Hangul_SsangDikeud" },
  { 0x0EB9, "Hangul_SsangJieuj" },
  { 0x0EA2, "Hangul_SsangKiyeog" },
  { 0x0EB3, "Hangul_SsangPieub" },
  { 0x0EB6, "Hangul_SsangSios" },
  { 0xFF32, "Hangul_Start" },
  { 0x0EF0, "Hangul_SunkyeongeumMieum" },
  { 0x0EF4, "Hangul_SunkyeongeumPhieuf" },
  { 0x0EF1, "Hangul_SunkyeongeumPieub" },
  { 0x0EBC, "Hangul_Tieut" },
  { 0x0ECC, "Hangul_U" },
  { 0x0EC8, "Hangul_WA" },
  { 0x0EC9, "Hangul_WAE" },
  { 0x0ECE, "Hangul_WE" },
  { 0x0ECD, "Hangul_WEO" },
  { 0x0ECF, "Hangul_WI" },
  { 0x0EC1, "Hangul_YA" },
  { 0x0EC2, "Hangul_YAE" },
  { 0x0EC6, "Hangul_YE" },
  { 0x0EC5, "Hangul_YEO" },
  { 0x0ED2, "Hangul_YI" },
  { 0x0ECB, "Hangul_YO" },
  { 0x0ED0, "Hangul_YU" },
  { 0x0EF5, "Hangul_YeorinHieuh" },
  { 0xFF7E, "Hangul_switch" },
  { 0xFF29, "Hankaku" },
  { 0x02A6, "Hcircumflex" },
  { 0xFF7E, "Hebrew_switch" },
  { 0xFF6A, "Help" },
  { 0xFF23, "Henkan" },
  { 0xFF23, "Henkan_Mode" },
  { 0xFF25, "Hiragana" },
  { 0xFF27, "Hiragana_Katakana" },
  { 0xFF50, "Home" },
  { 0x02A1, "Hstroke" },
  { 0xFFED, "Hyper_L" },
  { 0xFFEE, "Hyper_R" },
  { 0x0049, "I" },
  { 0xFE33, "ISO_Center_Object" },
  { 0xFE30, "ISO_Continuous_Underline" },
  { 0xFE31, "ISO_Discontinuous_Underline" },
  { 0xFE32, "ISO_Emphasize" },
  { 0xFE34, "ISO_Enter" },
  { 0xFE2F, "ISO_Fast_Cursor_Down" },
  { 0xFE2C, "ISO_Fast_Cursor_Left" },
  { 0xFE2D, "ISO_Fast_Cursor_Right" },
  { 0xFE2E, "ISO_Fast_Cursor_Up" },
  { 0xFE0C, "ISO_First_Group" },
  { 0xFE0D, "ISO_First_Group_Lock" },
  { 0xFE06, "ISO_Group_Latch" },
  { 0xFE07, "ISO_Group_Lock" },
  { 0xFF7E, "ISO_Group_Shift" },
  { 0xFE0E, "ISO_Last_Group" },
  { 0xFE0F, "ISO_Last_Group_Lock" },
  { 0xFE20, "ISO_Left_Tab" },
  { 0xFE02, "ISO_Level2_Latch" },
  { 0xFE04, "ISO_Level3_Latch" },
  { 0xFE05, "ISO_Level3_Lock" },
  { 0xFE03, "ISO_Level3_Shift" },
  { 0xFE01, "ISO_Lock" },
  { 0xFE22, "ISO_Move_Line_Down" },
  { 0xFE21, "ISO_Move_Line_Up" },
  { 0xFE08, "ISO_Next_Group" },
  { 0xFE09, "ISO_Next_Group_Lock" },
  { 0xFE24, "ISO_Partial_Line_Down" },
  { 0xFE23, "ISO_Partial_Line_Up" },
  { 0xFE25, "ISO_Partial_Space_Left" },
  { 0xFE26, "ISO_Partial_Space_Right" },
  { 0xFE0A, "ISO_Prev_Group" },
  { 0xFE0B, "ISO_Prev_Group_Lock" },
  { 0xFE2B, "ISO_Release_Both_Margins" },
  { 0xFE29, "ISO_Release_Margin_Left" },
  { 0xFE2A, "ISO_Release_Margin_Right" },
  { 0xFE27, "ISO_Set_Margin_Left" },
  { 0xFE28, "ISO_Set_Margin_Right" },
  { 0x02A9, "Iabovedot" },
  { 0x00CD, "Iacute" },
  { 0x00CE, "Icircumflex" },
  { 0x00CF, "Idiaeresis" },
  { 0x00CC, "Igrave" },
  { 0x03CF, "Imacron" },
  { 0xFF63, "Insert" },
  { 0x03C7, "Iogonek" },
  { 0x03A5, "Itilde" },
  { 0x004A, "J" },
  { 0x02AC, "Jcircumflex" },
  { 0x004B, "K" },
  { 0xFFB0, "KP_0" },
  { 0xFFB1, "KP_1" },
  { 0xFFB2, "KP_2" },
  { 0xFFB3, "KP_3" },
  { 0xFFB4, "KP_4" },
  { 0xFFB5, "KP_5" },
  { 0xFFB6, "KP_6" },
  { 0xFFB7, "KP_7" },
  { 0xFFB8, "KP_8" },
  { 0xFFB9, "KP_9" },
  { 0xFFAB, "KP_Add" },
  { 0xFF9D, "KP_Begin" },
  { 0xFFAE, "KP_Decimal" },
  { 0xFF9F, "KP_Delete" },
  { 0xFFAF, "KP_Divide" },
  { 0xFF99, "KP_Down" },
  { 0xFF9C, "KP_End" },
  { 0xFF8D, "KP_Enter" },
  { 0xFFBD, "KP_Equal" },
  { 0xFF91, "KP_F1" },
  { 0xFF92, "KP_F2" },
  { 0xFF93, "KP_F3" },
  { 0xFF94, "KP_F4" },
  { 0xFF95, "KP_Home" },
  { 0xFF9E, "KP_Insert" },
  { 0xFF96, "KP_Left" },
  { 0xFFAA, "KP_Multiply" },
  { 0xFF9B, "KP_Next" },
  { 0xFF9B, "KP_Page_Down" },
  { 0xFF9A, "KP_Page_Up" },
  { 0xFF9A, "KP_Prior" },
  { 0xFF98, "KP_Right" },
  { 0xFFAC, "KP_Separator" },
  { 0xFF80, "KP_Space" },
  { 0xFFAD, "KP_Subtract" },
  { 0xFF89, "KP_Tab" },
  { 0xFF97, "KP_Up" },
  { 0xFF2D, "Kana_Lock" },
  { 0xFF2E, "Kana_Shift" },
  { 0xFF21, "Kanji" },
  { 0xFF26, "Katakana" },
  { 0x03D3, "Kcedilla" },
  { 0x0EFF, "Korean_Won" },
  { 0x004C, "L" },
  { 0x01C5, "Lacute" },
  { 0xFED4, "Last_Virtual_Screen" },
  { 0x01A5, "Lcaron" },
  { 0x03A6, "Lcedilla" },
  { 0xFF51, "Left" },
  { 0xFF0A, "Linefeed" },
  { 0x20A4, "LiraSign" },
  { 0x01A3, "Lstroke" },
  { 0x004D, "M" },
  { 0x06B5, "Macedonia_DSE" },
  { 0x06B2, "Macedonia_GJE" },
  { 0x06BC, "Macedonia_KJE" },
  { 0x06A5, "Macedonia_dse" },
  { 0x06A2, "Macedonia_gje" },
  { 0x06AC, "Macedonia_kje" },
  { 0xFF2C, "Massyo" },
  { 0xFF67, "Menu" },
  { 0xFFE7, "Meta_L" },
  { 0xFFE8, "Meta_R" },
  { 0x20A5, "MillSign" },
  { 0xFF7E, "Mode_switch" },
  { 0xFE77, "MouseKeys_Accel_Enable" },
  { 0xFE76, "MouseKeys_Enable" },
  { 0xFF22, "Muhenkan" },
  { 0xFF20, "Multi_key" },
  { 0xFF3D, "MultipleCandidate" },
  { 0x004E, "N" },
  { 0x01D1, "Nacute" },
  { 0x20A6, "NairaSign" },
  { 0x01D2, "Ncaron" },
  { 0x03D1, "Ncedilla" },
  { 0x20AA, "NewSheqelSign" },
  { 0xFF56, "Next" },
  { 0xFED2, "Next_Virtual_Screen" },
  { 0x00D1, "Ntilde" },
  { 0xFF7F, "Num_Lock" },
  { 0x004F, "O" },
  { 0x13BC, "OE" },
  { 0x00D3, "Oacute" },
  { 0x00D4, "Ocircumflex" },
  { 0x00D6, "Odiaeresis" },
  { 0x01D5, "Odoubleacute" },
  { 0x00D2, "Ograve" },
  { 0x1EFA, "Ohorn" },
  { 0x03D2, "Omacron" },
  { 0x00D8, "Ooblique" },
  { 0x00D5, "Otilde" },
  { 0xFE78, "Overlay1_Enable" },
  { 0xFE79, "Overlay2_Enable" },
  { 0x0050, "P" },
  { 0xFF56, "Page_Down" },
  { 0xFF55, "Page_Up" },
  { 0xFF13, "Pause" },
  { 0x20A7, "PesetaSign" },
  { 0xFEFA, "Pointer_Accelerate" },
  { 0xFEE9, "Pointer_Button1" },
  { 0xFEEA, "Pointer_Button2" },
  { 0xFEEB, "Pointer_Button3" },
  { 0xFEEC, "Pointer_Button4" },
  { 0xFEED, "Pointer_Button5" },
  { 0xFEE8, "Pointer_Button_Dflt" },
  { 0xFEEF, "Pointer_DblClick1" },
  { 0xFEF0, "Pointer_DblClick2" },
  { 0xFEF1, "Pointer_DblClick3" },
  { 0xFEF2, "Pointer_DblClick4" },
  { 0xFEF3, "Pointer_DblClick5" },
  { 0xFEEE, "Pointer_DblClick_Dflt" },
  { 0xFEFB, "Pointer_DfltBtnNext" },
  { 0xFEFC, "Pointer_DfltBtnPrev" },
  { 0xFEE3, "Pointer_Down" },
  { 0xFEE6, "Pointer_DownLeft" },
  { 0xFEE7, "Pointer_DownRight" },
  { 0xFEF5, "Pointer_Drag1" },
  { 0xFEF6, "Pointer_Drag2" },
  { 0xFEF7, "Pointer_Drag3" },
  { 0xFEF8, "Pointer_Drag4" },
  { 0xFEFD, "Pointer_Drag5" },
  { 0xFEF4, "Pointer_Drag_Dflt" },
  { 0xFEF9, "Pointer_EnableKeys" },
  { 0xFEE0, "Pointer_Left" },
  { 0xFEE1, "Pointer_Right" },
  { 0xFEE2, "Pointer_Up" },
  { 0xFEE4, "Pointer_UpLeft" },
  { 0xFEE5, "Pointer_UpRight" },
  { 0xFED1, "Prev_Virtual_Screen" },
  { 0xFF3E, "PreviousCandidate" },
  { 0xFF61, "Print" },
  { 0xFF55, "Prior" },
  { 0x0051, "Q" },
  { 0x0052, "R" },
  { 0x01C0, "Racute" },
  { 0x01D8, "Rcaron" },
  { 0x03A3, "Rcedilla" },
  { 0xFF66, "Redo" },
  { 0xFE72, "RepeatKeys_Enable" },
  { 0xFF0D, "Return" },
  { 0xFF53, "Right" },
  { 0xFF24, "Romaji" },
  { 0x20A8, "RupeeSign" },
  { 0x0053, "S" },
  { 0x01A6, "Sacute" },
  { 0x01A9, "Scaron" },
  { 0x01AA, "Scedilla" },
  { 0x02DE, "Scircumflex" },
  { 0xFF14, "Scroll_Lock" },
  { 0xFF60, "Select" },
  { 0x06B1, "Serbian_DJE" },
  { 0x06BF, "Serbian_DZE" },
  { 0x06B8, "Serbian_JE" },
  { 0x06B9, "Serbian_LJE" },
  { 0x06BA, "Serbian_NJE" },
  { 0x06BB, "Serbian_TSHE" },
  { 0x06A1, "Serbian_dje" },
  { 0x06AF, "Serbian_dze" },
  { 0x06A8, "Serbian_je" },
  { 0x06A9, "Serbian_lje" },
  { 0x06AA, "Serbian_nje" },
  { 0x06AB, "Serbian_tshe" },
  { 0xFFE1, "Shift_L" },
  { 0xFFE6, "Shift_Lock" },
  { 0xFFE2, "Shift_R" },
  { 0xFF3C, "SingleCandidate" },
  { 0xFE73, "SlowKeys_Enable" },
  { 0xFE75, "StickyKeys_Enable" },
  { 0xFFEB, "Super_L" },
  { 0xFFEC, "Super_R" },
  { 0xFF15, "Sys_Req" },
  { 0x0054, "T" },
  { 0x00DE, "THORN" },
  { 0xFF09, "Tab" },
  { 0x01AB, "Tcaron" },
  { 0x01DE, "Tcedilla" },
  { 0xFED5, "Terminate_Server" },
  { 0x0DDF, "Thai_baht" },
  { 0x0DBA, "Thai_bobaimai" },
  { 0x0DA8, "Thai_chochan" },
  { 0x0DAA, "Thai_chochang" },
  { 0x0DA9, "Thai_choching" },
  { 0x0DAC, "Thai_chochoe" },
  { 0x0DAE, "Thai_dochada" },
  { 0x0DB4, "Thai_dodek" },
  { 0x0DBD, "Thai_fofa" },
  { 0x0DBF, "Thai_fofan" },
  { 0x0DCB, "Thai_hohip" },
  { 0x0DCE, "Thai_honokhuk" },
  { 0x0DA2, "Thai_khokhai" },
  { 0x0DA5, "Thai_khokhon" },
  { 0x0DA3, "Thai_khokhuat" },
  { 0x0DA4, "Thai_khokhwai" },
  { 0x0DA6, "Thai_khorakhang" },
  { 0x0DA1, "Thai_kokai" },
  { 0x0DE5, "Thai_lakkhangyao" },
  { 0x0DF7, "Thai_lekchet" },
  { 0x0DF5, "Thai_lekha" },
  { 0x0DF6, "Thai_lekhok" },
  { 0x0DF9, "Thai_lekkao" },
  { 0x0DF1, "Thai_leknung" },
  { 0x0DF8, "Thai_lekpaet" },
  { 0x0DF3, "Thai_leksam" },
  { 0x0DF4, "Thai_leksi" },
  { 0x0DF2, "Thai_leksong" },
  { 0x0DF0, "Thai_leksun" },
  { 0x0DCC, "Thai_lochula" },
  { 0x0DC5, "Thai_loling" },
  { 0x0DC6, "Thai_lu" },
  { 0x0DEB, "Thai_maichattawa" },
  { 0x0DE8, "Thai_maiek" },
  { 0x0DD1, "Thai_maihanakat" },
  { 0x0DDE, "Thai_maihanakat_maitho" },
  { 0x0DE7, "Thai_maitaikhu" },
  { 0x0DE9, "Thai_maitho" },
  { 0x0DEA, "Thai_maitri" },
  { 0x0DE6, "Thai_maiyamok" },
  { 0x0DC1, "Thai_moma" },
  { 0x0DA7, "Thai_ngongu" },
  { 0x0DED, "Thai_nikhahit" },
  { 0x0DB3, "Thai_nonen" },
  { 0x0DB9, "Thai_nonu" },
  { 0x0DCD, "Thai_oang" },
  { 0x0DCF, "Thai_paiyannoi" },
  { 0x0DDA, "Thai_phinthu" },
  { 0x0DBE, "Thai_phophan" },
  { 0x0DBC, "Thai_phophung" },
  { 0x0DC0, "Thai_phosamphao" },
  { 0x0DBB, "Thai_popla" },
  { 0x0DC3, "Thai_rorua" },
  { 0x0DC4, "Thai_ru" },
  { 0x0DD0, "Thai_saraa" },
  { 0x0DD2, "Thai_saraaa" },
  { 0x0DE1, "Thai_saraae" },
  { 0x0DE4, "Thai_saraaimaimalai" },
  { 0x0DE3, "Thai_saraaimaimuan" },
  { 0x0DD3, "Thai_saraam" },
  { 0x0DE0, "Thai_sarae" },
  { 0x0DD4, "Thai_sarai" },
  { 0x0DD5, "Thai_saraii" },
  { 0x0DE2, "Thai_sarao" },
  { 0x0DD8, "Thai_sarau" },
  { 0x0DD6, "Thai_saraue" },
  { 0x0DD7, "Thai_sarauee" },
  { 0x0DD9, "Thai_sarauu" },
  { 0x0DC9, "Thai_sorusi" },
  { 0x0DC8, "Thai_sosala" },
  { 0x0DAB, "Thai_soso" },
  { 0x0DCA, "Thai_sosua" },
  { 0x0DEC, "Thai_thanthakhat" },
  { 0x0DB1, "Thai_thonangmontho" },
  { 0x0DB2, "Thai_thophuthao" },
  { 0x0DB7, "Thai_thothahan" },
  { 0x0DB0, "Thai_thothan" },
  { 0x0DB8, "Thai_thothong" },
  { 0x0DB6, "Thai_thothung" },
  { 0x0DAF, "Thai_topatak" },
  { 0x0DB5, "Thai_totao" },
  { 0x0DC7, "Thai_wowaen" },
  { 0x0DC2, "Thai_yoyak" },
  { 0x0DAD, "Thai_yoying" },
  { 0x00DE, "Thorn" },
  { 0xFF2B, "Touroku" },
  { 0x03AC, "Tslash" },
  { 0x0055, "U" },
  { 0x00DA, "Uacute" },
  { 0x02DD, "Ubreve" },
  { 0x00DB, "Ucircumflex" },
  { 0x00DC, "Udiaeresis" },
  { 0x01DB, "Udoubleacute" },
  { 0x00D9, "Ugrave" },
  { 0x1EFC, "Uhorn" },
  { 0x06B6, "Ukrainian_I" },
  { 0x06B4, "Ukrainian_IE" },
  { 0x06B7, "Ukrainian_YI" },
  { 0x06A6, "Ukrainian_i" },
  { 0x06A4, "Ukrainian_ie" },
  { 0x06A7, "Ukrainian_yi" },
  { 0x06B6, "Ukranian_I" },
  { 0x06B4, "Ukranian_JE" },
  { 0x06B7, "Ukranian_YI" },
  { 0x06A6, "Ukranian_i" },
  { 0x06A4, "Ukranian_je" },
  { 0x06A7, "Ukranian_yi" },
  { 0x03DE, "Umacron" },
  { 0xFF65, "Undo" },
  { 0x03D9, "Uogonek" },
  { 0xFF52, "Up" },
  { 0x01D9, "Uring" },
  { 0x03DD, "Utilde" },
  { 0x0056, "V" },
  { 0x0057, "W" },
  { 0x20A9, "WonSign" },
  { 0x0058, "X" },
  { 0x0059, "Y" },
  { 0x00DD, "Yacute" },
  { 0x13BE, "Ydiaeresis" },
  { 0x005A, "Z" },
  { 0x01AF, "Zabovedot" },
  { 0x01AC, "Zacute" },
  { 0x01AE, "Zcaron" },
  { 0xFF28, "Zenkaku" },
  { 0xFF2A, "Zenkaku_Hankaku" },
  { 0x0061, "a" },
  { 0x00E1, "aacute" },
  { 0x01FF, "abovedot" },
  { 0x01E3, "abreve" },
  { 0x00E2, "acircumflex" },
  { 0x00B4, "acute" },
  { 0x00E4, "adiaeresis" },
  { 0x00E6, "ae" },
  { 0x00E0, "agrave" },
  { 0x03E0, "amacron" },
  { 0x0026, "ampersand" },
  { 0x01B1, "aogonek" },
  { 0x0027, "apostrophe" },
  { 0x08C8, "approximate" },
  { 0x00E5, "aring" },
  { 0x005E, "asciicircum" },
  { 0x007E, "asciitilde" },
  { 0x002A, "asterisk" },
  { 0x0040, "at" },
  { 0x00E3, "atilde" },
  { 0x0062, "b" },
  { 0x005C, "backslash" },
  { 0x0AF4, "ballotcross" },
  { 0x007C, "bar" },
  { 0x09DF, "blank" },
  { 0x08A5, "botintegral" },
  { 0x08AC, "botleftparens" },
  { 0x08A8, "botleftsqbracket" },
  { 0x08B2, "botleftsummation" },
  { 0x08AE, "botrightparens" },
  { 0x08AA, "botrightsqbracket" },
  { 0x08B6, "botrightsummation" },
  { 0x09F6, "bott" },
  { 0x08B4, "botvertsummationconnector" },
  { 0x007B, "braceleft" },
  { 0x007D, "braceright" },
  { 0x005B, "bracketleft" },
  { 0x005D, "bracketright" },
  { 0x01A2, "breve" },
  { 0x00A6, "brokenbar" },
  { 0x0063, "c" },
  { 0x02E5, "cabovedot" },
  { 0x01E6, "cacute" },
  { 0x0AB8, "careof" },
  { 0x0AFC, "caret" },
  { 0x01B7, "caron" },
  { 0x01E8, "ccaron" },
  { 0x00E7, "ccedilla" },
  { 0x02E6, "ccircumflex" },
  { 0x00B8, "cedilla" },
  { 0x00A2, "cent" },
  { 0x09E1, "checkerboard" },
  { 0x0AF3, "checkmark" },
  { 0x0BCF, "circle" },
  { 0x0AEC, "club" },
  { 0x003A, "colon" },
  { 0x1EF3, "combining_acute" },
  { 0x1EFF, "combining_belowdot" },
  { 0x1EF2, "combining_grave" },
  { 0x1EFE, "combining_hook" },
  { 0x1E9F, "combining_tilde" },
  { 0x002C, "comma" },
  { 0x00A9, "copyright" },
  { 0x09E4, "cr" },
  { 0x09EE, "crossinglines" },
  { 0x00A4, "currency" },
  { 0x0AFF, "cursor" },
  { 0x0064, "d" },
  { 0x0AF1, "dagger" },
  { 0x01EF, "dcaron" },
  { 0xFE56, "dead_abovedot" },
  { 0xFE58, "dead_abovering" },
  { 0xFE51, "dead_acute" },
  { 0xFE60, "dead_belowdot" },
  { 0xFE55, "dead_breve" },
  { 0xFE5A, "dead_caron" },
  { 0xFE5B, "dead_cedilla" },
  { 0xFE52, "dead_circumflex" },
  { 0xFE57, "dead_diaeresis" },
  { 0xFE59, "dead_doubleacute" },
  { 0xFE50, "dead_grave" },
  { 0xFE61, "dead_hook" },
  { 0xFE62, "dead_horn" },
  { 0xFE5D, "dead_iota" },
  { 0xFE54, "dead_macron" },
  { 0xFE5C, "dead_ogonek" },
  { 0xFE5F, "dead_semivoiced_sound" },
  { 0xFE53, "dead_tilde" },
  { 0xFE5E, "dead_voiced_sound" },
  { 0x0ABD, "decimalpoint" },
  { 0x00B0, "degree" },
  { 0x00A8, "diaeresis" },
  { 0x0AED, "diamond" },
  { 0x0AA5, "digitspace" },
  { 0x00F7, "division" },
  { 0x0024, "dollar" },
  { 0x0AAF, "doubbaselinedot" },
  { 0x01BD, "doubleacute" },
  { 0x0AF2, "doubledagger" },
  { 0x0AFE, "doublelowquotemark" },
  { 0x08FE, "downarrow" },
  { 0x0BA8, "downcaret" },
  { 0x0BD6, "downshoe" },
  { 0x0BC4, "downstile" },
  { 0x0BC2, "downtack" },
  { 0x01F0, "dstroke" },
  { 0x0065, "e" },
  { 0x03EC, "eabovedot" },
  { 0x00E9, "eacute" },
  { 0x01EC, "ecaron" },
  { 0x00EA, "ecircumflex" },
  { 0x00EB, "ediaeresis" },
  { 0x00E8, "egrave" },
  { 0x0AAE, "ellipsis" },
  { 0x0AA3, "em3space" },
  { 0x0AA4, "em4space" },
  { 0x03BA, "emacron" },
  { 0x0AA9, "emdash" },
  { 0x0ADE, "emfilledcircle" },
  { 0x0ADF, "emfilledrect" },
  { 0x0ACE, "emopencircle" },
  { 0x0ACF, "emopenrectangle" },
  { 0x0AA1, "emspace" },
  { 0x0AAA, "endash" },
  { 0x0AE6, "enfilledcircbullet" },
  { 0x0AE7, "enfilledsqbullet" },
  { 0x03BF, "eng" },
  { 0x0AE0, "enopencircbullet" },
  { 0x0AE1, "enopensquarebullet" },
  { 0x0AA2, "enspace" },
  { 0x01EA, "eogonek" },
  { 0x003D, "equal" },
  { 0x00F0, "eth" },
  { 0x0021, "exclam" },
  { 0x00A1, "exclamdown" },
  { 0x0066, "f" },
  { 0x0AF8, "femalesymbol" },
  { 0x09E3, "ff" },
  { 0x0ABB, "figdash" },
  { 0x0ADC, "filledlefttribullet" },
  { 0x0ADB, "filledrectbullet" },
  { 0x0ADD, "filledrighttribullet" },
  { 0x0AE9, "filledtribulletdown" },
  { 0x0AE8, "filledtribulletup" },
  { 0x0AC5, "fiveeighths" },
  { 0x0AB7, "fivesixths" },
  { 0x0AB5, "fourfifths" },
  { 0x08F6, "function" },
  { 0x0067, "g" },
  { 0x02F5, "gabovedot" },
  { 0x02BB, "gbreve" },
  { 0x03BB, "gcedilla" },
  { 0x02F8, "gcircumflex" },
  { 0x0060, "grave" },
  { 0x003E, "greater" },
  { 0x08BE, "greaterthanequal" },
  { 0x00AB, "guillemotleft" },
  { 0x00BB, "guillemotright" },
  { 0x0068, "h" },
  { 0x0AA8, "hairspace" },
  { 0x02B6, "hcircumflex" },
  { 0x0AEE, "heart" },
  { 0x0CE0, "hebrew_aleph" },
  { 0x0CF2, "hebrew_ayin" },
  { 0x0CE1, "hebrew_bet" },
  { 0x0CE1, "hebrew_beth" },
  { 0x0CE7, "hebrew_chet" },
  { 0x0CE3, "hebrew_dalet" },
  { 0x0CE3, "hebrew_daleth" },
  { 0x0CDF, "hebrew_doublelowline" },
  { 0x0CEA, "hebrew_finalkaph" },
  { 0x0CED, "hebrew_finalmem" },
  { 0x0CEF, "hebrew_finalnun" },
  { 0x0CF3, "hebrew_finalpe" },
  { 0x0CF5, "hebrew_finalzade" },
  { 0x0CF5, "hebrew_finalzadi" },
  { 0x0CE2, "hebrew_gimel" },
  { 0x0CE2, "hebrew_gimmel" },
  { 0x0CE4, "hebrew_he" },
  { 0x0CE7, "hebrew_het" },
  { 0x0CEB, "hebrew_kaph" },
  { 0x0CF7, "hebrew_kuf" },
  { 0x0CEC, "hebrew_lamed" },
  { 0x0CEE, "hebrew_mem" },
  { 0x0CF0, "hebrew_nun" },
  { 0x0CF4, "hebrew_pe" },
  { 0x0CF7, "hebrew_qoph" },
  { 0x0CF8, "hebrew_resh" },
  { 0x0CF1, "hebrew_samech" },
  { 0x0CF1, "hebrew_samekh" },
  { 0x0CF9, "hebrew_shin" },
  { 0x0CFA, "hebrew_taf" },
  { 0x0CFA, "hebrew_taw" },
  { 0x0CE8, "hebrew_tet" },
  { 0x0CE8, "hebrew_teth" },
  { 0x0CE5, "hebrew_waw" },
  { 0x0CE9, "hebrew_yod" },
  { 0x0CF6, "hebrew_zade" },
  { 0x0CF6, "hebrew_zadi" },
  { 0x0CE6, "hebrew_zain" },
  { 0x0CE6, "hebrew_zayin" },
  { 0x0ADA, "hexagram" },
  { 0x08A3, "horizconnector" },
  { 0x09EF, "horizlinescan1" },
  { 0x09F0, "horizlinescan3" },
  { 0x09F1, "horizlinescan5" },
  { 0x09F2, "horizlinescan7" },
  { 0x09F3, "horizlinescan9" },
  { 0x02B1, "hstroke" },
  { 0x09E2, "ht" },
  { 0x00AD, "hyphen" },
  { 0x0069, "i" },
  { 0x00ED, "iacute" },
  { 0x00EE, "icircumflex" },
  { 0x08CF, "identical" },
  { 0x00EF, "idiaeresis" },
  { 0x02B9, "idotless" },
  { 0x08CD, "ifonlyif" },
  { 0x00EC, "igrave" },
  { 0x03EF, "imacron" },
  { 0x08CE, "implies" },
  { 0x08DA, "includedin" },
  { 0x08DB, "includes" },
  { 0x08C2, "infinity" },
  { 0x08BF, "integral" },
  { 0x08DC, "intersection" },
  { 0x03E7, "iogonek" },
  { 0x03B5, "itilde" },
  { 0x006A, "j" },
  { 0x02BC, "jcircumflex" },
  { 0x0BCA, "jot" },
  { 0x006B, "k" },
  { 0x04B1, "kana_A" },
  { 0x04C1, "kana_CHI" },
  { 0x04B4, "kana_E" },
  { 0x04CC, "kana_FU" },
  { 0x04CA, "kana_HA" },
  { 0x04CD, "kana_HE" },
  { 0x04CB, "kana_HI" },
  { 0x04CE, "kana_HO" },
  { 0x04CC, "kana_HU" },
  { 0x04B2, "kana_I" },
  { 0x04B6, "kana_KA" },
  { 0x04B9, "kana_KE" },
  { 0x04B7, "kana_KI" },
  { 0x04BA, "kana_KO" },
  { 0x04B8, "kana_KU" },
  { 0x04CF, "kana_MA" },
  { 0x04D2, "kana_ME" },
  { 0x04D0, "kana_MI" },
  { 0x04D3, "kana_MO" },
  { 0x04D1, "kana_MU" },
  { 0x04DD, "kana_N" },
  { 0x04C5, "kana_NA" },
  { 0x04C8, "kana_NE" },
  { 0x04C6, "kana_NI" },
  { 0x04C9, "kana_NO" },
  { 0x04C7, "kana_NU" },
  { 0x04B5, "kana_O" },
  { 0x04D7, "kana_RA" },
  { 0x04DA, "kana_RE" },
  { 0x04D8, "kana_RI" },
  { 0x04DB, "kana_RO" },
  { 0x04D9, "kana_RU" },
  { 0x04BB, "kana_SA" },
  { 0x04BE, "kana_SE" },
  { 0x04BC, "kana_SHI" },
  { 0x04BF, "kana_SO" },
  { 0x04BD, "kana_SU" },
  { 0x04C0, "kana_TA" },
  { 0x04C3, "kana_TE" },
  { 0x04C1, "kana_TI" },
  { 0x04C4, "kana_TO" },
  { 0x04C2, "kana_TSU" },
  { 0x04C2, "kana_TU" },
  { 0x04B3, "kana_U" },
  { 0x04DC, "kana_WA" },
  { 0x04A6, "kana_WO" },
  { 0x04D4, "kana_YA" },
  { 0x04D6, "kana_YO" },
  { 0x04D5, "kana_YU" },
  { 0x04A7, "kana_a" },
  { 0x04A3, "kana_closingbracket" },
  { 0x04A4, "kana_comma" },
  { 0x04A5, "kana_conjunctive" },
  { 0x04AA, "kana_e" },
  { 0x04A1, "kana_fullstop" },
  { 0x04A8, "kana_i" },
  { 0x04A5, "kana_middledot" },
  { 0x04AB, "kana_o" },
  { 0x04A2, "kana_openingbracket" },
  { 0xFF7E, "kana_switch" },
  { 0x04AF, "kana_tsu" },
  { 0x04AF, "kana_tu" },
  { 0x04A9, "kana_u" },
  { 0x04AC, "kana_ya" },
  { 0x04AE, "kana_yo" },
  { 0x04AD, "kana_yu" },
  { 0x03A2, "kappa" },
  { 0x03F3, "kcedilla" },
  { 0x03A2, "kra" },
  { 0x006C, "l" },
  { 0x01E5, "lacute" },
  { 0x0AD9, "latincross" },
  { 0x01B5, "lcaron" },
  { 0x03B6, "lcedilla" },
  { 0x0ABC, "leftanglebracket" },
  { 0x08FB, "leftarrow" },
  { 0x0BA3, "leftcaret" },
  { 0x0AD2, "leftdoublequotemark" },
  { 0x08AF, "leftmiddlecurlybrace" },
  { 0x0ACC, "leftopentriangle" },
  { 0x0AEA, "leftpointer" },
  { 0x08A1, "leftradical" },
  { 0x0BDA, "leftshoe" },
  { 0x0AD0, "leftsinglequotemark" },
  { 0x09F4, "leftt" },
  { 0x0BDC, "lefttack" },
  { 0x003C, "less" },
  { 0x08BC, "lessthanequal" },
  { 0x09E5, "lf" },
  { 0x08DE, "logicaland" },
  { 0x08DF, "logicalor" },
  { 0x09ED, "lowleftcorner" },
  { 0x09EA, "lowrightcorner" },
  { 0x01B3, "lstroke" },
  { 0x006D, "m" },
  { 0x00AF, "macron" },
  { 0x0AF7, "malesymbol" },
  { 0x0AF0, "maltesecross" },
  { 0x0ABF, "marker" },
  { 0x00BA, "masculine" },
  { 0x002D, "minus" },
  { 0x0AD6, "minutes" },
  { 0x00B5, "mu" },
  { 0x00D7, "multiply" },
  { 0x0AF6, "musicalflat" },
  { 0x0AF5, "musicalsharp" },
  { 0x006E, "n" },
  { 0x08C5, "nabla" },
  { 0x01F1, "nacute" },
  { 0x01F2, "ncaron" },
  { 0x03F1, "ncedilla" },
  { 0x09E8, "nl" },
  { 0x00A0, "nobreakspace" },
  { 0x08BD, "notequal" },
  { 0x00AC, "notsign" },
  { 0x00F1, "ntilde" },
  { 0x0023, "numbersign" },
  { 0x06B0, "numerosign" },
  { 0x006F, "o" },
  { 0x00F3, "oacute" },
  { 0x00F4, "ocircumflex" },
  { 0x00F6, "odiaeresis" },
  { 0x01F5, "odoubleacute" },
  { 0x13BD, "oe" },
  { 0x01B2, "ogonek" },
  { 0x00F2, "ograve" },
  { 0x1EFB, "ohorn" },
  { 0x03F2, "omacron" },
  { 0x0AC3, "oneeighth" },
  { 0x0AB2, "onefifth" },
  { 0x00BD, "onehalf" },
  { 0x00BC, "onequarter" },
  { 0x0AB6, "onesixth" },
  { 0x00B9, "onesuperior" },
  { 0x0AB0, "onethird" },
  { 0x0AE2, "openrectbullet" },
  { 0x0AE5, "openstar" },
  { 0x0AE4, "opentribulletdown" },
  { 0x0AE3, "opentribulletup" },
  { 0x00AA, "ordfeminine" },
  { 0x00F8, "oslash" },
  { 0x00F5, "otilde" },
  { 0x0BC0, "overbar" },
  { 0x047E, "overline" },
  { 0x0070, "p" },
  { 0x00B6, "paragraph" },
  { 0x0028, "parenleft" },
  { 0x0029, "parenright" },
  { 0x08EF, "partialderivative" },
  { 0x0025, "percent" },
  { 0x002E, "period" },
  { 0x00B7, "periodcentered" },
  { 0x0AFB, "phonographcopyright" },
  { 0x002B, "plus" },
  { 0x00B1, "plusminus" },
  { 0x0AD4, "prescription" },
  { 0x04B0, "prolongedsound" },
  { 0x0AA6, "punctspace" },
  { 0x0071, "q" },
  { 0x0BCC, "quad" },
  { 0x003F, "question" },
  { 0x00BF, "questiondown" },
  { 0x0022, "quotedbl" },
  { 0x0060, "quoteleft" },
  { 0x0027, "quoteright" },
  { 0x0072, "r" },
  { 0x01E0, "racute" },
  { 0x08D6, "radical" },
  { 0x01F8, "rcaron" },
  { 0x03B3, "rcedilla" },
  { 0x00AE, "registered" },
  { 0x0ABE, "rightanglebracket" },
  { 0x08FD, "rightarrow" },
  { 0x0BA6, "rightcaret" },
  { 0x0AD3, "rightdoublequotemark" },
  { 0x08B0, "rightmiddlecurlybrace" },
  { 0x08B7, "rightmiddlesummation" },
  { 0x0ACD, "rightopentriangle" },
  { 0x0AEB, "rightpointer" },
  { 0x0BD8, "rightshoe" },
  { 0x0AD1, "rightsinglequotemark" },
  { 0x09F5, "rightt" },
  { 0x0BFC, "righttack" },
  { 0x0073, "s" },
  { 0x01B6, "sacute" },
  { 0x01B9, "scaron" },
  { 0x01BA, "scedilla" },
  { 0x02FE, "scircumflex" },
  { 0xFF7E, "script_switch" },
  { 0x0AD7, "seconds" },
  { 0x00A7, "section" },
  { 0x003B, "semicolon" },
  { 0x04DF, "semivoicedsound" },
  { 0x0AC6, "seveneighths" },
  { 0x0ACA, "signaturemark" },
  { 0x0AAC, "signifblank" },
  { 0x08C9, "similarequal" },
  { 0x0AFD, "singlelowquotemark" },
  { 0x002F, "slash" },
  { 0x09E0, "soliddiamond" },
  { 0x0020, "space" },
  { 0x00DF, "ssharp" },
  { 0x00A3, "sterling" },
  { 0x0074, "t" },
  { 0x01BB, "tcaron" },
  { 0x01FE, "tcedilla" },
  { 0x0AF9, "telephone" },
  { 0x0AFA, "telephonerecorder" },
  { 0x08C0, "therefore" },
  { 0x0AA7, "thinspace" },
  { 0x00FE, "thorn" },
  { 0x0AC4, "threeeighths" },
  { 0x0AB4, "threefifths" },
  { 0x00BE, "threequarters" },
  { 0x00B3, "threesuperior" },
  { 0x08A4, "topintegral" },
  { 0x08AB, "topleftparens" },
  { 0x08A2, "topleftradical" },
  { 0x08A7, "topleftsqbracket" },
  { 0x08B1, "topleftsummation" },
  { 0x08AD, "toprightparens" },
  { 0x08A9, "toprightsqbracket" },
  { 0x08B5, "toprightsummation" },
  { 0x09F7, "topt" },
  { 0x08B3, "topvertsummationconnector" },
  { 0x0AC9, "trademark" },
  { 0x0ACB, "trademarkincircle" },
  { 0x03BC, "tslash" },
  { 0x0AB3, "twofifths" },
  { 0x00B2, "twosuperior" },
  { 0x0AB1, "twothirds" },
  { 0x0075, "u" },
  { 0x00FA, "uacute" },
  { 0x02FD, "ubreve" },
  { 0x00FB, "ucircumflex" },
  { 0x00FC, "udiaeresis" },
  { 0x01FB, "udoubleacute" },
  { 0x00F9, "ugrave" },
  { 0x1EFD, "uhorn" },
  { 0x03FE, "umacron" },
  { 0x0BC6, "underbar" },
  { 0x005F, "underscore" },
  { 0x08DD, "union" },
  { 0x03F9, "uogonek" },
  { 0x08FC, "uparrow" },
  { 0x0BA9, "upcaret" },
  { 0x09EC, "upleftcorner" },
  { 0x09EB, "uprightcorner" },
  { 0x0BC3, "upshoe" },
  { 0x0BD3, "upstile" },
  { 0x0BCE, "uptack" },
  { 0x01F9, "uring" },
  { 0x03FD, "utilde" },
  { 0x0076, "v" },
  { 0x08C1, "variation" },
  { 0x09F8, "vertbar" },
  { 0x08A6, "vertconnector" },
  { 0x04DE, "voicedsound" },
  { 0x09E9, "vt" },
  { 0x0077, "w" },
  { 0x0078, "x" },
  { 0x0079, "y" },
  { 0x00FD, "yacute" },
  { 0x00FF, "ydiaeresis" },
  { 0x00A5, "yen" },
  { 0x007A, "z" },
  { 0x01BF, "zabovedot" },
  { 0x01BC, "zacute" },
  { 0x01BE, "zcaron" }
};

static __KeyName __scim_key_mask_names [] =
{
    {SCIM_KEY_ShiftMask,        "Shift"},
    {SCIM_KEY_CapsLockMask,     "CapsLock"},
    {SCIM_KEY_ControlMask,      "Control"},
    {SCIM_KEY_AltMask,          "Alt"},
    {SCIM_KEY_MetaMask,         "Meta"},
    {SCIM_KEY_SuperMask,        "Super"},
    {SCIM_KEY_HyperMask,        "Hyper"},
    {SCIM_KEY_NumLockMask,      "NumLock"},
    {SCIM_KEY_QuirkKanaRoMask,  "QuirkKanaRo"},
    {SCIM_KEY_ReleaseMask,      "KeyRelease"}
};

#define SCIM_NUM_KEY_UNICODES (sizeof (__scim_key_to_unicode_tab) / sizeof (__scim_key_to_unicode_tab[0]))

#define SCIM_NUM_KEY_NAMES    (sizeof (__scim_keys_by_code) / sizeof (__scim_keys_by_code [0]))

#define SCIM_NUM_KEY_MASKS    (sizeof (__scim_key_mask_names) / sizeof (__scim_key_mask_names[0]))

/*
vi:ts=4:nowrap:ai:expandtab
*/
