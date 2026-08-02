#!/bin/sh
# Generate the hanconv filter icons.
#
# One glyph per script -- 簡 simplified, 正 traditional, 日 Japanese -- with the
# source at the top left, the target at the bottom right, and a chevron between
# them pointing the way the text is converted. The root icon is 漢 alone, since
# the filter covers all three scripts and two of them cannot stand for it.
#
# Regenerate with:  ./gen_hanconv_icons.sh
#
# Requires ImageMagick and a CJK font.

set -e

FONT=${FONT:-/usr/share/fonts/opentype/noto/NotoSansCJK-Bold.ttc}
SIZE=32
PT=17

SC_COLOR='#2b6cb0'      # simplified: blue
TC_COLOR='#c53030'      # traditional: red
JP_COLOR='#2f855a'      # japanese: green
ARROW='#4a5568'

# $1 out  $2 from-glyph  $3 from-colour  $4 to-glyph  $5 to-colour
pair () {
    out=$1; fg=$2; fc=$3; tg=$4; tc=$5

    # Source top left, target bottom right, so the pair reads in order; a small
    # chevron between them says which way without crowding the glyphs, which
    # have to stay legible at 32px.
    convert -size ${SIZE}x${SIZE} xc:none \
        -font "$FONT" -pointsize $PT \
        -fill "$fc" -annotate +0+17  "$fg" \
        -fill "$tc" -annotate +15+31 "$tg" \
        -fill "$ARROW" -draw "polygon 18,11 23,16 18,16" \
        "$out"
}

# Direction icons.
pair hanconv-sc-tc.png 簡 "$SC_COLOR" 正 "$TC_COLOR"
pair hanconv-tc-sc.png 正 "$TC_COLOR" 簡 "$SC_COLOR"
pair hanconv-tc-jp.png 正 "$TC_COLOR" 日 "$JP_COLOR"
pair hanconv-jp-tc.png 日 "$JP_COLOR" 正 "$TC_COLOR"
pair hanconv-sc-jp.png 簡 "$SC_COLOR" 日 "$JP_COLOR"
pair hanconv-jp-sc.png 日 "$JP_COLOR" 簡 "$SC_COLOR"

# The filter itself: one glyph, legible at 32px in a menu.
convert -size ${SIZE}x${SIZE} xc:none \
    -font "$FONT" -pointsize 30 \
    -fill '#2d3748' -annotate +1+28 漢 \
    hanconv.png

echo "generated: hanconv.png hanconv-{sc-tc,tc-sc,tc-jp,jp-tc,sc-jp,jp-sc}.png"
