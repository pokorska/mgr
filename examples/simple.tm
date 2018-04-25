START: s1
s1 ALL ->* s2 - NOTHING
s2 ALL ->^ s3 - NOTHING
s3 b -> s4 R NOTHING
s3 B -> s4 R NOTHING
s4 ALL -> s5 - .
s5 ALL ->^ s6 - NOTHING
s6 ALL -> END - NOTHING
