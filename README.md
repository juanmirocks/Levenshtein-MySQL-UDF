# MySQL/MariaDB UDF functions implemented in C

Features:

* [General Levenshtein algorithm](https://en.wikipedia.org/wiki/Levenshtein_distance)
* k-bounded Levenshtein distance algorithm (linear time, constant space).
  * Info: this is when you only care about the distance if it's smaller or equal than your given _k_ (e.g. to test if the spelling difference between two words is of maximum 1). In this case, the algorithm runs faster while using less memory.
* Levenshtein ratio
  * Info: this is syntactic sugar for `levenshtein_ratio(s, t) = 1 - levenshtein(s, t) / max(s.length, t.length)`)
* k-bounded Levenshtein ratio

## Install

First you need to compile the library and tell MySQL/MariaDB about it:

```shell
# Unix; on Linux x64 you may need to add the -fPIC flag
gcc -o levenshtein.so -shared levenshtein.c `mysql_config --include`  

# macOS
gcc -bundle -o levenshtein.so levenshtein.c `mysql_config --include`

# all systems
cp levenshtein.so `mysql_config --plugindir` # You may need sudo
```

_If you run into issues, please take a look at [this workaround](https://github.com/juanmirocks/Levenshtein-MySQL-UDF/issues/12#issuecomment-384419463) or [this](https://github.com/juanmirocks/Levenshtein-MySQL-UDF/issues/16#issuecomment-682452839)._


Then in a console, run:

```sql
CREATE FUNCTION levenshtein RETURNS INT SONAME 'levenshtein.so';
CREATE FUNCTION levenshtein_k RETURNS INT SONAME 'levenshtein.so';
CREATE FUNCTION levenshtein_ratio RETURNS REAL SONAME 'levenshtein.so';
CREATE FUNCTION levenshtein_k_ratio RETURNS REAL SONAME 'levenshtein.so';
```

That should be all 🐬 ᵒᴥᵒᶅ !

#### Note

Just in case the last SQL statements failed, consider that to create and use UDF functions, you need `CREATE ROUTINE, EXECUTE` privileges.

For MariaDB you have to grant additional privileges on `mysql` database. Here is an example of privileges allowing dropping, altering, creating and using functions:
```sql
GRANT INSERT, DELETE, DROP ROUTINE, CREATE ROUTINE, ALTER ROUTINE, EXECUTE ON mysql.* TO 'user'@'%';
```


For more details on setting up UDFs in MySQL see: [UDF Compiling and Installing](https://dev.mysql.com/doc/refman/5.7/en/udf-compiling.html)


## Run

For example:

```sql
select 3 = levenshtein('maneuver', 'manoeuvre');
select 3 = levenshtein_k('maneuver', 'manoeuvre', 5);  -- k=5, allow the distance to be up to 5, otherwise it's 6 or greater
select 2 = levenshtein_k('maneuver', 'manoeuvre', 1);  -- k=1, allow the distance to be up to 1, otherwise it's 2 or greater

select 0.6666666666666667 = levenshtein_ratio('maneuver', 'manoeuvre'); -- that is, 1 - 3/9
select 0.6666666666666667 = levenshtein_k_ratio('maneuver', 'manoeuvre', 5); -- same
select 0 = levenshtein_k_ratio('maneuver', 'manoeuvre', 1); -- 0 because the distance (3) is greater than the maximum k allowed (1)
```


## Test (for Development)

Your contributions are very much welcome!

Please before making a pull request, run the unit tests file.

This should return 1 at the end:

```shell
mysql -uroot < unittest.sql  # Change your username & password as needed
```
