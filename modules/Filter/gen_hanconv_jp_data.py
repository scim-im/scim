#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Generate scim_hanconv_jp_data.h from the Joyo Kanji Table.

Source of the Japanese forms
----------------------------
The kyujitai/shinjitai correspondences come from the Joyo Kanji Table
(常用漢字表), issued as Cabinet Notification No. 2 of 2010-11-30 (平成22年
内閣告示第2号).  A Cabinet Notification is excluded from copyright by Article
13 of the Japanese Copyright Act, so the data may be redistributed freely.

    https://www.bunka.go.jp/kokugo_nihongo/sisaku/joho/joho/kijun/naikaku/
        pdf/joyokanjihyo_20101130.pdf

In that table each entry gives the 通用字体 followed, where one exists, by the
"いわゆる康熙字典体" in full-width parentheses -- "亜（亞）".  Exactly one entry,
弁, carries more than one old form and is laid out differently; it is handled
explicitly below and the script fails if that assumption ever breaks.

The Simplified Chinese side is not taken from this table.  It is composed from
the existing SC<->TC data in scim_sctc_filter_data.h, so SC<->JP is derived,
not independently sourced.  Where composition is ambiguous the pair is dropped
rather than guessed.

Usage
-----
    pdftotext -enc UTF-8 joyokanjihyo_20101130.pdf joyo.txt
    ./gen_hanconv_jp_data.py joyo.txt scim_sctc_filter_data.h \\
        > scim_hanconv_jp_data.h
"""

import re
import sys
import io
import datetime

HAN = r'[㐀-鿿]'

# The sole entry in the 2010 table with several old forms; see the module
# docstring. Kept explicit so it is visible rather than buried in a heuristic.
MULTI = {'弁': ['辨', '瓣', '辯']}

# Single-form entries in the 2010 notification, used to notice a changed source.
EXPECTED_SINGLE_FORM = 297


def read_joyo(path):
    """-> {shinjitai: [kyujitai, ...]} from the notification's text."""
    txt = io.open(path, encoding='utf-8').read()

    pairs = re.findall(r'^(%s)（(%s)）' % (HAN, HAN), txt, re.M)
    out = {}
    for new, old in pairs:
        out.setdefault(new, []).append(old)

    # The table is laid out in columns, so scanning for "another entry that
    # looks multi-form" just matches unrelated neighbours. Guard the assumption
    # with a count instead: the 2010 notification has 297 single-form entries,
    # and any change to that means the source moved and the MULTI list below
    # should be re-checked against it.
    if len(out) != EXPECTED_SINGLE_FORM:
        sys.stderr.write('warning: %d single-form entries, expected %d -- the '
                         'source has changed; re-check for entries with more '
                         'than one old form\n' % (len(out), EXPECTED_SINGLE_FORM))

    for new, olds in MULTI.items():
        if new in out:
            sys.exit('error: %s now parses as a single-form entry; '
                     'remove it from MULTI' % new)
        for old in olds:
            if old not in txt:
                sys.exit('error: expected old form %s of %s not in the source'
                         % (old, new))
        out[new] = list(olds)

    return out


def read_sctc(path):
    """-> (sc_to_tc, tc_to_sc) from the existing generated header."""
    txt = io.open(path, encoding='utf-8').read()

    def table(name):
        body = txt.split('__%s_table [] = {' % name, 1)[1].split('};', 1)[0]
        return {int(a, 16): int(b, 16)
                for a, b in re.findall(r'\{\s*(0x[0-9a-f]+),\s*(0x[0-9a-f]+)\s*\}',
                                       body)}

    return table('sc_to_tc'), table('tc_to_sc')


def emit(name, mapping, out):
    out.write('static UShortPair __%s_table [] = {\n' % name)
    items = sorted(mapping.items())
    for i in range(0, len(items), 4):
        row = ''.join('{ 0x%04x, 0x%04x },' % (k, v) for k, v in items[i:i + 4])
        out.write('  %s\n' % row)
    out.write('  { 0x0000, 0x0000 }\n};\n\n')


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)

    joyo = read_joyo(sys.argv[1])
    sc_to_tc, tc_to_sc = read_sctc(sys.argv[2])

    # --- TC -> JP: every old form maps to its modern form. Many-to-one is fine
    #     here; it is the direction that loses nothing.
    tc_to_jp = {}
    for new, olds in joyo.items():
        for old in olds:
            if old != new:
                tc_to_jp[ord(old)] = ord(new)

    # --- JP -> TC: only where the modern form has exactly one old form, so the
    #     reverse is unambiguous. 弁 is dropped: 辨/瓣/辯 cannot be chosen between.
    jp_to_tc = {ord(new): ord(olds[0])
                for new, olds in joyo.items()
                if len(olds) == 1 and olds[0] != new}

    dropped = sorted(new for new, olds in joyo.items() if len(olds) > 1)

    # --- SC <-> JP, composed. Start from the traditional form of each pair and
    #     take its simplified form (itself when SC and TC agree).
    sc_to_jp, conflicts = {}, set()
    for tc, jp in tc_to_jp.items():
        sc = tc_to_sc.get(tc, tc)
        if sc == jp:
            continue
        if sc in sc_to_jp and sc_to_jp[sc] != jp:
            conflicts.add(sc)
            continue
        sc_to_jp[sc] = jp
    for sc in conflicts:
        sc_to_jp.pop(sc, None)

    jp_to_sc = {}
    for jp, tc in jp_to_tc.items():
        sc = tc_to_sc.get(tc, tc)
        if sc != jp:
            jp_to_sc[jp] = sc

    out = sys.stdout
    out.write('/** @file scim_hanconv_jp_data.h\n'
              ' *  Generated by gen_hanconv_jp_data.py -- do not edit.\n'
              ' *\n'
              ' *  Japanese forms: Joyo Kanji Table (常用漢字表), Cabinet\n'
              ' *  Notification No. 2 of 2010-11-30, which Article 13 of the\n'
              ' *  Japanese Copyright Act excludes from copyright.\n'
              ' *\n'
              ' *  SC<->JP is composed from that table and the SC<->TC data in\n'
              ' *  scim_sctc_filter_data.h; it is derived, not sourced.\n'
              ' *\n'
              ' *  JP->TC and JP->SC carry only unambiguous reversals: %d modern\n'
              ' *  form(s) with several old forms (%s) are omitted rather than\n'
              ' *  guessed.\n'
              ' *\n'
              ' *  Generated %s.\n'
              ' */\n\n'
              % (len(dropped), ' '.join(dropped),
                 datetime.date.today().isoformat()))
    out.write('#if !defined (__SCIM_HANCONV_JP_DATA_H)\n'
              '#define __SCIM_HANCONV_JP_DATA_H\n\n')

    for name, table in (('tc_to_jp', tc_to_jp), ('jp_to_tc', jp_to_tc),
                        ('sc_to_jp', sc_to_jp), ('jp_to_sc', jp_to_sc)):
        emit(name, table, out)

    out.write('#endif\n')

    sys.stderr.write('  tc_to_jp %d, jp_to_tc %d, sc_to_jp %d, jp_to_sc %d'
                     ' (dropped as ambiguous: %s)\n'
                     % (len(tc_to_jp), len(jp_to_tc), len(sc_to_jp),
                        len(jp_to_sc), ' '.join(dropped) or 'none'))


if __name__ == '__main__':
    main()
