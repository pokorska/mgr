s1 ALL ->^ s2 - NOTHING
s2 ALL ->* s3 - NEXT_CHAR
s3 ALL -> s4 - NOTHING
s4 z -> s5 R NOTHING
s4 Z -> s5 R NOTHING
s5 ALL -> s6 - .
s6 ALL -> END - NOTHING
