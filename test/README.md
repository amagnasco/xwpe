# Test files #

Update the test code with new testcases if you create or modify code. It is 
a work in progress. It helps if new code has one or more testcases
to confirm it is still working properly after future changes. 

# Regression test #

This is meant to be a regression testset, not an unit test. You do not have to
test every little bit of your new code. Just the bit that proves it works ok if someone
changes the code.

# use #

From the project root do

    `cd build`
    `make chake`

to execute the testcases. Check `test-suite.log` to check the results if you receive errors.

Check works with fork. If you want to debug the testprogram, use `CK_NOFORK`
in the call to `srunner_set_fork_status()` in `main.c`. If you don't you will have to
follow the inferior processes (which is possible if you use `gdb`). Removing is the
simplest solution.
