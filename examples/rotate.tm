START: init
init ALL -> rotation - NOTHING
rotation ALL -> rotation1 - NEXT_CHAR
rotation1 ALL -> rotation2 - NEXT_CHAR
rotation2 ALL -> rotation3 - NEXT_CHAR
rotation3 ALL -> letters R NOTHING
letters ALL ->* letters1 - NOTHING
letters1 ALL -> change L NOTHING
change 0 -> print R NOTHING
change NON_ZERO -> change1 R PREV_CHAR
change1 z -> change L a
change1 Z -> change L A
change1 ALL -> change L NEXT_CHAR
print ALL ->^ END - NOTHING
