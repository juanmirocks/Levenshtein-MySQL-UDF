MySQL/MariaDB UDF functions implemented in C for:

* [General Levenshtein algorithm](https://en.wikipedia.org/wiki/Levenshtein_distance)
* k-bounded Levenshtein distance algorithm (linear time, constant space).
  * Info: this is when you only care about the distance if it's smaller or equal than your given _k_ (e.g. to test if the spelling difference between two words is of maximum 2). In this case, the algorithm runs faster while using less memory.
* Levenshtein ratio
  * Info: this is syntactic sugar for `levenshtein_ratio(s, t) = 1 - levenshtein(s, t) / max(s.length, t.length)`)
* k-bounded Levenshtein ratio

## Install

First you need to compile the library and tell MySQL/MariaDB about it:

```shell
(Unix) gcc -o levenshtein.so -shared levenshtein.c `mysql_config --include`
(macOS) gcc -bundle -o levenshtein.so levenshtein.c `mysql_config --include`
plugin_dir=$(mysql -u root -p pass -e 'select @@plugin_dir;' | grep -v '@')   # This declares your MySQL's plugin directory where the compiled library should be put to. Adjust your user name & password as needed
cp levenshtein.so $plugin_dir # You may need sudo
```

For newer versions of MariaDB you will simply need to replace mysql root credentials with `sudo` call.

```shell
plugin_dir=$(sudo mysql -e 'select @@plugin_dir;' | grep -v '@')
```

Then in a console, run:

```sql
CREATE FUNCTION levenshtein RETURNS INT SONAME 'levenshtein.so';
CREATE FUNCTION levenshtein_k RETURNS INT SONAME 'levenshtein.so';
CREATE FUNCTION levenshtein_ratio RETURNS REAL SONAME 'levenshtein.so';
CREATE FUNCTION levenshtein_k_ratio RETURNS REAL SONAME 'levenshtein.so';
```

That should be all 🐬 ᶘ ᵒᴥᵒᶅ !

### Note

Just in case the last SQL statements failed, consider that to create and use UDF functions, you need `CREATE ROUTINE, EXECUTE` privileges.

For MariaDB you have to grant additional privileges on `mysql` database. Here is an example of privileges allowing dropping, altering, creating and using functions:
```sql
GRANT INSERT, DELETE, DROP ROUTINE, CREATE ROUTINE, ALTER ROUTINE, EXECUTE ON mysql.* TO 'user'@'%';
```


For more details on setting up UDFs in MySQL see: [UDF Compiling and Installing](https://dev.mysql.com/doc/refman/5.7/en/udf-compiling.html)
