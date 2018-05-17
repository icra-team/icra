# icra
ICRA is a static analyzer based on interprocedural compositional recurrence analysis. In short, ICRA analyzes an inter-procedural
equation system by reducing the analysis task to intra-procedural queries.

We are in the process of restructuring the ICRA code, but currently it is conceptually structured as follows. The relevant 
inter-procedural transformations are currently contained in this repository under the src directory. Currently these transformations
also rely on functionality from [Wali](https://github.com/WaliDev/WALi-OpenNWA/tree/NewtonDuetGaussian) which is currently a subtree
of this repository. The intra-procedural analysis along with underlying domain is implemented in 
[duet](https://github.com/zkincaid/duet/tree/Newton-ark2) which is also a subtree of this repository.

Currently our underlying intra-procedural reasoning is mostly over unbounded integers. Duet has some support for other data-types,
but ICRA works best on integer programs or integer models. To see our current example suite, see the C files in
/WALi-OpenNWA/Examples/cprover/tests/

## Building ICRA

To build ICRA, first install the dependencies of the duet library.  You will need a version of OCaml compiled with -fPIC runtime libs.  If OPAM is installed this can be done with`opam switch 4.02.3+PIC`, for example.
    (see: duet/README.md)

You will also need to install scons, boost, and glog
    sudo apt-get install scons libboost-all-dev libgoogle-glog-dev

Return to the parent directory and type:
    make

If this command completes successfully, you should obtain an executable named "icra".

## Using ICRA
To run ICRA on a single C program:
```    
./icra <input_program.c>
```
ICRA has a few command line arguments and will also pass arguments to duet.

The most relevant command line options for ICRA is as follows:

* `--newton-verbosity <0-8>`

    Changes the verbosity of output. 0 is the least verbose and prints only assertion checking results and bounds on variables. 8 is
    the most verbose and the default verbosity. This output is mostly used for debugging.

* `-cra-split-loops`

    Loop splitting is a duet option which allows some refinement of loop bodies. By default loop splitting is turned off, because
    it has the potential to increase analysis time and may, although extremely rarely, decrease precision. However, we have found
    that it almost increases precision without extra overall analysis time. Typically, it is best to run CRA with `-cra-split-loops`.
  
At the end of ICRA's output, bounds on variables are shown under the heading,
`Bounds on Variables`. Underneath this heading, you will see the upper and
lower bounds on variables that ICRA has computed for the input program. If an input program
contains the statement \_\_VERIFIER\_print\_hull(v) for some variable v at some
program point, then ICRA will attempt to generate bounds on the value of v at
that point in the program. ICRA is capable of generating bounds for any variable. 
  
In particular, if the \_\_VERIFIER\_print\_hull(v) statement is at some location
L in some procedure P, then ICRA will attempt to find bounds on the value of
v at L. The bounds are expressed in terms of the values (upon entry to P) of
global variables, including the variables param0, param1, ..., which are used
to represent the formal parameters to P. For example, if the
\_\_VERIFIER\_print\_hull(v) appears at the end of procedure P , and ICRA prints
the bound v <= n \* n, that means that the value of v at the end of P is no
greater than the square of the value of the global variable n upon entry
to P.
    
In the second-to-last section of its output, ICRA prints information about
the assertions in the program. This occurs under the heading `Assertion
Checking at Error Points`. For each occurrence of \_\_VERIFIER\_assert(expr) in
the input program, ICRA will attempt to prove that the assertion holds. If
the proof succeeds, ICRA will print `Assertion on line XX PASSED`, and
otherwise `Assertion on line XX FAILED`.

The following commands may be used to augment a C program:

* \_\_VERIFIER\_assert( b\_expr );

    This instructs ICRA to check the assertion that b_expr is always true
    whenever control reaches the assert statement.

* \_\_VERIFIER\_assume( b\_expr );

    This instructs ICRA to assume that b_expr is always true whenever
    control reaches the assume statement.
    
* \_\_VERIFIER\_print\_hull( i\_var );

    This instructs ICRA to discover expressions that upper-bound and
    lower-bound the values that i_var will take when control reaches the 
    print_hull statement.  Variables in the bounding expressions are
    evaluated upon entry to the procedure that contains the print_hull
    statement.  See the section of this README labeled "Understanding 
    ICRA's Output" for more details.

* \_\_VERIFIER\_nondet\_int();

    This non-deterministic function may return any (signed) integer.
    To obtain a non-negative integer, you can write something like:
    ```
    N = __VERIFIER_nondet_int();  
    __VERIFIER_assume(N >= 0);
    ```
See the C files under /WALi-OpenNWA/Examples/cprover/tests/ for 
examples of how these commands are used.

## Relevant Publications
The main paper is [3], which combined the ideas
of [1] and [2] into a unified framework. [3] has now been extended in [4] to allow for additional non-linear reasoning capabilities.

1. A. Farzan and Z. Kincaid. Compositional Recurrence Analysis. In *FMCAD*, 2015
2. T. Reps, E. Turetsky, P. Prabhu. Newtonian Program Analysis via Tensor Product. In *POPL* 2016. (Journal version *TOPLAS* 2017)
3. Z. Kincaid, J. Breck, A. Forouhi Boroujeni, and T. Reps. Compositional Recurrence Analysis Revisited. In *PLDI*, 2017
4. Z. Kincaid, J. Cyphert, J. Breck, and T. Reps. Non-linear Reasoning for Invariant Synthesis. In *POPL*, 2018
