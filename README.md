MySQL UDF functions implemented in C for:

* General Levenshtein algorithm
* k-bounded Levenshtein distance algorithm (linear time, constant space),
* Levenshtein ratio (syntactic sugar: levenshtein_ratio(s, t) = 1 - levenshtein(s, t) / max(s.length, t.length))
* k-bounded Levenshtein ratio

**Installation** instructions in the same [.c file](https://github.com/jmcejuela/Levenshtein-MySQL-UDF/blob/master/levenshtein.c).
