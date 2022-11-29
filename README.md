[![Open in Visual Studio Code](https://classroom.github.com/assets/open-in-vscode-c66648af7eb3fe8bc4f294546bfd86ef473780cde1dea487d3c4ff354943c9ae.svg)](https://classroom.github.com/online_ide?assignment_repo_id=8803378&assignment_repo_type=AssignmentRepo)
# Project 2: Suffix tree construction

You should implement a suffix tree construction algorithm. You can choose to implement the naive O(n²)-time construction algorithm as discussed in class or McCreight’s O(n) construction algorithm. After that, implement a search algorithm (similar to slow-scan) for finding all occurrences of a pattern. This algorithm should run in O(m+z) where m is the length of the pattern and z the number of occurrences.

Write a program, `st` using the suffix tree exact pattern search algorithm (similar to slow-scan) to report all indices in a string where a given pattern occurs. 

The program should take the same options as in project 1: `st genome.fa reads.fq`. The program should output (almost) the same SAM file. Because a search in a suffix tree is not done from the start to the end of the string the output might be in a different order, but if you sort the output from the previous project and for this program, they should be identical.

## Evaluation

Implement the tool `st` that does exact pattern matching using a suffix tree. Test it to the best of your abilities, and then fill out the report below.

# Report

## Specify if you have used a linear time or quadratic time algorithm.

Quadratic time naive insert algorithm. McCreight's algorithm would have been cool, but time, unfortunately, is limited.

## Insights you may have had while implementing and comparing the algorithms.

N/A

## Problems encountered if any.

Testing of trees is quite difficult. Initially, a test was made in order to capture the "semantics" of a suffix tree. That is; we do not care about which order nodes
are in. We only care about who their parent and children are (and their edge labels). This lead to a quite complicated test, which utilized prime factors in order
to reliably decouple the test from the implementation. The reasoning is, that the test was supposed to be written before the implementation - and would aid in actually
writing the implementation.

However, even though it worked alright, there were some drawbacks:
1. It would take too long to write something that tests a lot.
2. It assumed the tree was already built, so the whole idea of using the test to aid the implementation was lost, as it wouldn't give any insight into where it went wrong.

Thus, other tests were written, which tested each insert of a suffix in the tree. These tests were much more implementation dependant. For instance, it assumed
that when splitting an edge, the current suffix which we insert should be the child of the split. This child would have the previous branch root as a sibling.

## Correctness

There's about 1400 LOC in test.c. Every insert in the suffix tree is checked to see if everything looks as expected - all verified manually.
There are also unit tests for searching. We test for no exact matches, one match and multiple matches. 
The diff is empty between larger files produced from the naive exact search implementation vs. this implementation. 

## Running time

Worst case input for the construction algorithm is consecutive characters.
It would take O(n) to search for a place to create a node for the suffix,
and as we have n suffixes to insert, we end up with O(n^2).

Below is a plot of the running time of suffix tree construction. Time _T_ is in seconds, _n_ goes from 1000 to 250.000 by intervals of 1000 per data point:<br><br><br>
<img src="/quad.png">
<br><br>
Here is the same data, but where the vertical axis has been divided by n^2, to realize that it is indeed running in O(n^2):<br><br><br>
<img src="/quad2.png">
