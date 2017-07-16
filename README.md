MySQL UDF functions implemented in C for:

* General Levenshtein algorithm
* k-bounded Levenshtein distance algorithm (linear time, constant space),
* Levenshtein ratio (syntactic sugar for: `levenshtein_ratio(s, t) = 1 - levenshtein(s, t) / max(s.length, t.length)`)
* k-bounded Levenshtein ratio

## Quick install guide

To install this UDF using MySQL run:

```bash
gcc -o levenshtein.so -fPIC -shared levenshtein.c -I /usr/include/mysql/
plugin_dir=$(mysql -u root -p pass -e 'select @@plugin_dir;' | grep -v '@')   # adjust your user name & add password if needed
sudo cp levenshtein.so $plugin_dir
```

For newer versions of MariaDB you will simply need to replace mysql root credentials with `sudo` call.

```bash
plugin_dir=$(sudo mysql -e 'select @@plugin_dir;' | grep -v '@')
```


To use the UDF, run:
```sql
CREATE FUNCTION levenshtein RETURNS INTEGER SONAME 'levenshtein.so'
```

To create and use the function, you need `CREATE ROUTINE, EXECUTE` privileges.


For MariaDB you have to grant additional privileges on `mysql` database. Here is an example of privilages allowing droping, altering, creating and using functions:
```sql
GRANT INSERT, DELETE, DROP ROUTINE, CREATE ROUTINE, ALTER ROUTINE, EXECUTE ON mysql.* TO 'user'@'%';
```


For more instructions, see [levenshtein.c file](https://github.com/jmcejuela/Levenshtein-MySQL-UDF/blob/master/levenshtein.c).
For more details on setting up UDF in MySQL see: [UDF Compiling and Installing](https://dev.mysql.com/doc/refman/5.7/en/udf-compiling.html)
