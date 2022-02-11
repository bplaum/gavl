#!/bin/sh
NEW_YEAR=`date +%Y`
OLD_YEAR=`echo "$NEW_YEAR - 1" | bc`
OLD_YEARS="2001 - $OLD_YEAR"
NEW_YEARS="2001 - $NEW_YEAR"
echo "Replacing \"$OLD_YEARS\" with \"$NEW_YEARS\""
find . -name '*.[ch]' -exec sed -i "s/$OLD_YEARS/$NEW_YEARS/g" {} \;
