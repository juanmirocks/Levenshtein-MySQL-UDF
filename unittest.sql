-- Unit tests
--
-- Everything run together should output 1. When 0, a test failed. In that case
-- run logically-independent groups of assertions to spot the bug.

-- levenshtein
select 0 = levenshtein(null, null) union
select 0 = levenshtein(null, '') union
select 0 = levenshtein('', null) union
select 0 = levenshtein('', '') union


-- levenshtein_k
select 0 = levenshtein_k(null, null, 0) union
select 0 = levenshtein_k(null, '', 0) union
select 0 = levenshtein_k('', null, 0) union
select 0 = levenshtein_k('', '', 0) union

select 0 = levenshtein_k('p', 'p', 0) union
select 1 = levenshtein_k('p', 'c', 0) union
select 2 = levenshtein_k('aa', 'bb', 2) union
select 2 = levenshtein_k('aa', 'bb', 999) union
select 2 = levenshtein_k('aa', 'bb', 1) union
select 2 = levenshtein_k('aa', 'bbbb', 1) union


-- levenshtein_ratio
select 0 = levenshtein_ratio(null, null) union
select 0 = levenshtein_ratio(null, '') union
select 0 = levenshtein_ratio('', null) union
select 0 = levenshtein_ratio('', '')
