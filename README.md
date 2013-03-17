MySQL UDF functions implemented in C for:

* General Levenshtein algorithm
* k-bounded Levenshtein distance algorithm (linear time, constant space),
* Levenshtein ratio -- syntactic sugar for levenshtein_ratio(s, t) = 1 - levenshtein(s, t) / max(s.length, t.length)

**Installation** instructions in the same [.c file](https://github.com/jmcejuela/Levenshtein-MySQL-UDF/blob/master/levenshtein.c).