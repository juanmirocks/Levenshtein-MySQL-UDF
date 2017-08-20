/*
 * Copyright 2012-2015 Juan Miguel Cejuela (@jmcejuela)
 *
 * -------------------------------------------------------------------------
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------
 *
 * Some credit for simple levenshtein to: Joshua Drew, SpinWeb Net Designs
 *
 * Thanks to the contributors! (on GitHub):
 *
 *   - @popthestack
 *   - @lilobase
 *   - @peterhost
 *   - @krassowski
 *   - ... see complete list here: https://github.com/juanmirocks/Levenshtein-MySQL-UDF/graphs/contributors
 *
 * -------------------------------------------------------------------------
 *
 * INSTALLATION
 *
 * See: https://github.com/juanmirocks/Levenshtein-MySQL-UDF
 *
 */

#ifdef STANDARD
/* STANDARD is defined, don't use any mysql functions */
#include <string.h>
#ifdef __WIN__
typedef unsigned __int64 ulonglong; /* Microsofts 64 bit types */
typedef __int64 longlong;
#else
typedef unsigned long long ulonglong;
typedef long long longlong;
#endif /*__WIN__*/
#else
#include <my_global.h>
#include <my_sys.h>
#if defined(MYSQL_SERVER)
#include <m_string.h>
#else
/* when compiled as standalone */
#include <string.h>
#endif
#endif
#include <mysql.h>
#include <ctype.h>

#ifdef HAVE_DLOPEN

/* (Expected) maximum number of digits to return */
#define LEVENSHTEIN_MAX 3

static inline int minimum(int a, int b, int c) {
  int min = a;
  if (b < min)
    min = b;
  if (c < min)
    min = c;
  return min;
}

static inline int maximum(int a, int b) {
  if (a > b) return a;
  else return b;
}

/**
 * Levenshtein distance
 *
 * @param s string 1 to compare, length n
 * @param t string 2 to compare, length m
 * @result levenshtein distance between s and t
 *
 * @time O(nm), quadratic
 * @space O(nm)
 */
my_bool  levenshtein_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void     levenshtein_deinit(UDF_INIT *initid);
longlong levenshtein(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);


/**
 * Levenshtein distance with threshold k (maximum allowed distance)
 *
 * @param s string 1 to compare, length n
 * @param t string 2 to compare, length m
 * @param k maximum threshold
 * @result levenshtein distance between s and t or >k (not specified) if the distance is greater than k
 *
 * @time O(kl), linear; where l = min(n, m)
 * @space O(k), constant
 */
my_bool  levenshtein_k_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void     levenshtein_k_deinit(UDF_INIT *initid);
longlong levenshtein_k(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);


/**
 * Levenshtein ratio
 *
 * @param s string 1 to compare, length n
 * @param t string 2 to compare, length m
 * @result levenshtein ratio between s and t
 *
 * @time O(nm), quadratic
 * @space O(nm)
 */
my_bool levenshtein_ratio_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void    levenshtein_ratio_deinit(UDF_INIT *initid);
double  levenshtein_ratio(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);


/**
 * Levenshtein ratio with threshold k (maximum allowed distance)
 *
 * @param s string 1 to compare, length n
 * @param t string 2 to compare, length m
 * @param k maximum threshold
 * @result levenshtein ratio between s and t if (levenshtein distance <= k), otherwise 0.0
 *
 * @time O(kl), linear: where 1 = min(n, m)
 * @space O(k), constant
 */
my_bool levenshtein_k_ratio_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void    levenshtein_k_ratio_deinit(UDF_INIT *initid);
double  levenshtein_k_ratio(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);


//-------------------------------------------------------------------------


my_bool levenshtein_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  if ((args->arg_count != 2) ||
      (args->arg_type[0] != STRING_RESULT || args->arg_type[1] != STRING_RESULT)) {
    strcpy(message, "Function requires 2 arguments, (string, string)");
    return 1;
  }

  //matrix for levenshtein calculations of size n+1 x m+1 (+1 for base values)
  int *d = (int *) malloc(sizeof(int) * (args->lengths[0] + 1) * (args->lengths[1] + 1));
  if (d == NULL) {
    strcpy(message, "Failed to allocate memory");
    return 1;
  }

  initid->ptr = (char*) d;
  initid->max_length = LEVENSHTEIN_MAX;
  initid->maybe_null = 0; //doesn't return null

  return 0;
}

void levenshtein_deinit(UDF_INIT *initid) {
  if (initid->ptr != NULL)
    free(initid->ptr);
}

longlong levenshtein(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
  const char *s = args->args[0];
  const char *t = args->args[1];

  int n = (s == NULL) ? 0 : args->lengths[0];
  int m = (t == NULL) ? 0 : args->lengths[1];

  if (0 == n)
    return m;
  if (0 == m)
    return n;

  int *d = (int*) initid->ptr;

  /* Initialization */
  n++; m++;

  int i;
  for (i = 0; i < n; i++)
    d[i] = i;

  int j;
  for (j = 0; j < m; j++)
    d[n * j] = j;

  /* Recurrence */

  int p, h; //indices for d matrix seen as a vector, see below

  int im1 = 0; //i minus 1
  for (i = 1; i < n; i++) {
    p = i;
    int jm1 = 0; //j minus 1
    for (j = 1; j < m; j++) {
      h = p; // d[h] = d[i,j-1], h = (j * n + i - n)  = ((j - 1) * n + i)
      p += n; // d[p] = d[i,j], p = (j * n + i)

      if (s[im1] == t[jm1]) {
        d[p] = d[h-1]; //no operation required, d[i-1,j-1]
      }

      else {
        d[p] = minimum(d[p-1],  //deletion, d[i-1, j]
                       d[h],  //insertion, d[i, j-1]
                       d[h-1] //substitution, d[i-1,j-1]
                       ) + 1;  //can put +1 outside because the cost is the same
      }

      jm1 = j;
    }

    im1 = i;
  }

  return (longlong) d[p];
}

//-------------------------------------------------------------------------

my_bool levenshtein_ratio_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  if ((args->arg_count != 2) ||
      (args->arg_type[0] != STRING_RESULT || args->arg_type[1] != STRING_RESULT)) {
    strcpy(message, "Function requires 2 arguments, (string, string)");
    return 1;
  }

  //matrix for levenshtein calculations of size n+1 x m+1 (+1 for base values)
  int *d = (int *) malloc(sizeof(int) * (args->lengths[0] + 1) * (args->lengths[1] + 1));
  if (d == NULL) {
    strcpy(message, "Failed to allocate memory");
    return 1;
  }

  initid->ptr = (char*) d;
  initid->max_length = LEVENSHTEIN_MAX;
  initid->maybe_null = 0; //doesn't return null

  return 0;
}

void levenshtein_ratio_deinit(UDF_INIT *initid) {
  if (initid->ptr != NULL)
    free(initid->ptr);
}

double levenshtein_ratio(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
  const char *s = args->args[0];
  const char *t = args->args[1];

  int n = (s == NULL) ? 0 : args->lengths[0];
  int m = (t == NULL) ? 0 : args->lengths[1];
  double maxlen = maximum(n, m);
  if (maxlen == 0)
    return 0.0;

  double dist = (double) levenshtein(initid, args, is_null, error);
  return 1.0 - dist/maxlen;
}

//-------------------------------------------------------------------------

my_bool levenshtein_k_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  if ((args->arg_count != 3) ||
      (args->arg_type[0] != STRING_RESULT || args->arg_type[1] != STRING_RESULT || args->arg_type[2] != INT_RESULT)) {
    strcpy(message, "Function requires 3 arguments, (string, string, int)");
    return 1;
  }

  initid->max_length = LEVENSHTEIN_MAX;
  initid->maybe_null = 0; //doesn't return null

  return 0;
}

//Not necessary
//
/* void levenshtein_k_deinit(UDF_INIT *initid) { */
/*   //nothing */
/*   fflush(stderr); //for debugging */
/* } */

/*
 * 1st observation: time O(kl)
 * (see Algorithms on Strings, Trees, and Sequences, Dan Gusfield, pg. 263)
 *
 * When the levenshtein distance is limited to k the alignment path cannot be >k
 * cells off the main diagonal, either from the left (insertions) or from the
 * right (deletions) This means we don't have to fill the whole recurrence
 * matrix; it suffices to fill a strip of 2k + 1 cells in a row. Also, r <= k,
 * for r = m - n, is a necessary condition for there to be any solution.
 *
 *
 * 2nd observation: exactly k + 1
 *
 * The number of _unpaired_ insertions/deletions (the absolute difference in the
 * number of these operations) is as maximum k. When n == m (r == 0) any
 * movement to the left, insertion, in the recurrence matrix is corresponded to
 * a movement to the right, deletion, or viceversa. Analogously when m > n only
 * a maximum of k insertions/deletions operations can be left unpaired. This
 * makes the required strip size to be k + 1 cells.
 *
 * Call l = min(n, m). Place the string with length l on the y axis, and the
 * other on the x axis. By doing this, the number of rows is equal or less than
 * the number of columns and so the algorithm effectively runs in O(kl) Also, by
 * this arrangement of s and t in the recurrence matrix, r indicates the forced
 * number of unpaired deletions, i.e., since r>=0, there cannot be unpaired
 * deletions. Observe that for there to be any insertion k must be equal or
 * greater than 2.
 *
 * This arrangement leaves (k-r)/2 possible insertions (left) and (k-r)/2 + r
 * possible deletions (right) --> (k-r)/2 + (k-r)/2 + r + 1 == k + 1
 *
 *
 * 3rd observation: space O(k)
 *
 * As with the original levenshtein algorithm, only the current and last rows
 * are needed (that is, when we don't do traceback because we need not the
 * alignment path). By the previous observation, we just need 2 rows of k + 1
 * cells. For this, considering the last row, the previous diagonal is actually
 * in the same column (substitution) and the above cell is actually in the next
 * column (deletion). Observe that by doing this, we're virtually creating a
 * recurrence matrix of n * (k+1) being now the original diagonal in the middle
 * column (-r) (matrix which could be used to do the traceback)
 *
 */
longlong levenshtein_k(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
  char *s = args->args[0];
  char *t = args->args[1];

  int n = (s == NULL) ? 0 : args->lengths[0];
  int m = (t == NULL) ? 0 : args->lengths[1];

  //order the strings so that the first always has the minimum length l
  if (n > m) {
    int aux = n;
    n = m;
    m = aux;
    char *auxs = s;
    s = t;
    t = auxs;
  }

  const int k = *((int*) args->args[2]);
  const int ignore = k + 1; //lev dist between s and t is at least greater than k
  const int r = m - n;

  if (0 == n)
    return (m > k) ? ignore : m;
  if (0 == m)
    return (n > k) ? ignore : n;
  if (r > k)
    return ignore;

  const int lsize = (((k > m) ? m : k) - r) / 2; //left space for insertions
  const int rsize = lsize + r; //right space for deletions, rsize >= lsize (rsize == lsize iff r == 0)
  const int stripsize = lsize + rsize + 1; // + 1 for the diagonal cell
  const int stripsizem1 = stripsize - 1; //see later, not to repeat calculations

  int d[2 * stripsize]; //Current and last rows
  int currentrow;
  int lastrow;

  /* Initialization */

  //currentrow = 0
  int i;
  for (i = lsize; i < stripsize; i++) //start from diagonal cell
    d[i] = i - lsize;

  /* Recurrence */

  currentrow = stripsize;
  lastrow = 0;

  //j index for virtual recurrence matrix, jv index for rows
  //bl & br = left & right bounds for j
  int j, jv, bl, br;
  int im1 = 0, jm1;
  int a, b, c, min; //for minimum function, coded directly here for maximum speed
  for (i = 1; i <= n; i++) {

    //bl = max(i - lsize, 0), br = min(i + rsize, m)
    bl = i - lsize;
    if (bl < 0) {
      jv = abs(bl); //no space for all allowed insertions
      bl = 0;
    }
    else
      jv = 0;
    br = i + rsize;
    if (br > m)
      br = m;

    jm1 = bl - 1;
    for (j = bl; j <= br; j++) {
      if (0 == j) //postponed part of initialization
        d[currentrow + jv] = i;
      else {
        //By observation 3, the indices change for the lastrow (always +1)
        if (s[im1] == t[jm1]) {
          d[currentrow + jv] = d[lastrow + jv];
        }
        else {
          //get the minimum of these 3 operations
          a = (0 == jv) ? ignore : d[currentrow + jv - 1]; //deletion
          b = (stripsizem1 == jv) ? ignore : d[lastrow + jv + 1]; //insertion
          c = d[lastrow + jv]; //substitution

          min = a;
          if (b < min)
            min = b;
          if (c < min)
            min = c;

          d[currentrow + jv] = min + 1;
        }
      }
      jv++;
      jm1 = j;
    }

    //obsv: the cost of a following diagonal never decreases
    if (d[currentrow + lsize + r] > k)
      return ignore;

    im1 = i;

    //swap
    currentrow = currentrow ^ stripsize;
    lastrow = lastrow ^ stripsize;
  }

  //only here if levenhstein(s, t) <= k
  return (longlong) d[lastrow + lsize + r]; //d[n, m]
}

//-------------------------------------------------------------------------

my_bool levenshtein_k_ratio_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  if ((args->arg_count != 3) ||
      (args->arg_type[0] != STRING_RESULT || args->arg_type[1] != STRING_RESULT || args->arg_type[2] != INT_RESULT)) {
    strcpy(message, "Function requires 3 arguments, (string, string, int)");
    return 1;
  }

  initid->max_length = LEVENSHTEIN_MAX;
  initid->maybe_null = 0; //doesn't return null

  return 0;
}

double levenshtein_k_ratio(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
  const char *s = args->args[0];
  const char *t = args->args[1];
  const int k = *((int*) args->args[2]);

  int n = (s == NULL) ? 0 : args->lengths[0];
  int m = (t == NULL) ? 0 : args->lengths[1];
  double maxlen = maximum(n, m);
  if (maxlen == 0)
    return 0.0;

  double dist = (double) levenshtein_k(initid, args, is_null, error);
  if (dist > k)
    return 0.0;
  else
    return 1.0 - dist/maxlen;
}

#endif /* HAVE_DLOPEN */
