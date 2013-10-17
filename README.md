jitedjs
=======
A new javascript engine with just in time compiler tech inside.

This is a project trying to create a javascript virtual machine (VM) from scratch. 


Project Targets
---------------
* Milestone 1. Create a VM for a subset of javascript language (js-minus) running inside a interpreter
* Milestone 2. Update the interpreter to use just in time compiler.
* Milestone 3. Introduce some of the builtin functions (such as some pow)
* Milestone 4. Update the js-minus languague to full versioned javascript language (should be break down into some smaller targets)
* Milestone 5. Try sunpider / v8 benchmark.

Future:

How to build
------------

Build Environments:

Ubuntu 11.10 x86-64 tested. But should be ok for most linux system.

Step:

* Checkout the tags you want.
* $ source ./env.sh
* $ ./build.sh


How to test
-----------


buildin functions
=================

* print
* println
* clock
* format
* disassemble
* collectgarbage
* readfile
* writefile
* loadfile
* type
* insert
* remove
* tostring
* random
* srand
* time
