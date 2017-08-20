-- Unit tests
--
-- Everything run together should output 1. When 0, a test failed. In that case
-- run logically-independent groups of assertions to spot the bug.

-- Test edge cases:

select 0 = levenshtein(null, null) as 'This must return 1 at the end:' union
select 0 = levenshtein(null, '') union
select 0 = levenshtein('', null) union
select 0 = levenshtein('', '') union

select 0 = levenshtein_k(null, null, 0) union
select 0 = levenshtein_k(null, '', 0) union
select 0 = levenshtein_k('', null, 0) union
select 0 = levenshtein_k('', '', 0) union

select 0 = levenshtein_ratio(null, null) union
select 0 = levenshtein_ratio(null, '') union
select 0 = levenshtein_ratio('', null) union
select 0 = levenshtein_ratio('', '') union

select 0 = levenshtein_k_ratio(null, null, 0) union
select 0 = levenshtein_k_ratio(null, '', 0) union
select 0 = levenshtein_k_ratio('', null, 0) union
select 0 = levenshtein_k_ratio('', '', 0) union

-- Test levenshtein_k

select 0 = levenshtein_k('p', 'p', 0) union
select 1 = levenshtein_k('p', 'c', 0) union
select 2 = levenshtein_k('aa', 'bb', 2) union
select 2 = levenshtein_k('aa', 'bb', 999) union
select 2 = levenshtein_k('aa', 'bb', 1) union
select 2 = levenshtein_k('aa', 'bbbb', 1) union

-- Test comprehensive cases:

select 3 = levenshtein('maneuver', 'manoeuvre') union
select 3 = levenshtein_k('maneuver', 'manoeuvre', 5) union  -- k=5, allow the distance to be up to 5, otherwise it's 6 or greater
select 2 = levenshtein_k('maneuver', 'manoeuvre', 1) union  -- k=1, allow the distance to be up to 1, otherwise it's 2 or greater

select 0.6666666666666667 = levenshtein_ratio('maneuver', 'manoeuvre') union -- that is, 1 - 3/9
select 0.6666666666666667 = levenshtein_k_ratio('maneuver', 'manoeuvre', 5) union -- same
select 0 = levenshtein_k_ratio('maneuver', 'manoeuvre', 1); -- 0 because the distance (3) is greater than the maximum k allowed (1)
