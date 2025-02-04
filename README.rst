======================
Lab 3: Heap management
======================

.. contents:: Contents
    :depth: 2


TL;DR
=====

Deadlines
---------

- Tasks 1 and 2: 2025-02-11, 23:59 Vancouver time
- Tasks 3—5: 2025-02-18, 23:59 Vancouver time


Objective
---------

In this lab, you will build a simple implementation of ``malloc()``, ``free()``, and ``realloc()``.

The ideas here are pretty simple — we covered the structures and actions in detail in lecture. What is hard here is **debugging**. First, you are doing a lot of pointer arithmetic, which is tricky to get right to begin with. Even more insidiously, errors can show up as crashes many operations after you screwed up a block header.


Logistics
---------

As in prior labs, you will use ``castor.ece.ubc.ca`` to complete this lab; we will also use ``castor`` to evaluate your submission. As usual, submit by pushing to the GitHub repo.

As usual, Task 1 and Task 2 are due earlier than the remaining tasks.

As with all work in CPEN 212, everything you submit must be original and written by you from scratch after the lab has been released. You may not refer to any code other than what is found in the textbooks and lectures, and you must not share code with anyone at any time. See the academic integrity policy for details.

Be sure that you are submitting only **working code**. If your code does not compile or crashes, you will receive **0 marks** for the relevant task. It's much better to submit code with less functionality that is well-tested and works than code that has more features but does not compile or crashes.

Do not rename any directories, functions, or variables provided in the templates, and do not change any types in the existing declarations. If you do so, your code will fail to compile and/or link, and you will end up with a 0 for the relevant task.

Make sure you review the memory allocation lectures and understand at the very least how the implicit freelist allocation works, how the allocated block is represented with its header(s), and what is returned by ``malloc()``.


Task 1
======

Introduction
------------

In the ``task1`` folder you will find ``crazylist.h`` and ``crazylist.c``.

They contain an unusual implementation of a linked list cell. Like a normal linked list cell, it contains a value (``car``) and a pointer to the rest of the list (``cdr``). Unlike in a normal list, however, the cons cell pointer points to the *middle* of the cell rather than the beginning.

We've also implemented two functions for you:

- ``cons`` creates a new cell, and
- ``enclosing_struct`` returns a pointer to the *beginning* of the cell (since the actual cell pointer points to the middle).

Why are we doing this crazy thing? If you think about it, this is how blocks on the heap are accessed. Although each block has a header, the pointer returned to the user is to the user-writable space, not to the header. This means you will eventually need to figure out how to get at the header, e.g., when ``free()`` is called. But this is almost exactly how this crazy list works, as well — it's just a lot simpler with the crazy list.


What you need to do
-------------------

First, carefully study the definition of ``cons`` and ``enclosing_struct``.

Next, implement the functions

- ``first``,
- ``rest``,
- ``find``,
- ``insert_sorted``
- ``reverse``, and
- ``print_list``

in ``crazylist.c``.

Here is a tiny example of some of these functions being used::

    uint64_t *list = cons(100, NULL);
    list = cons(30, list);
    list = cons(10, list);
    list = cons(5, list);
    print_list(list); // output: 5 10 30 100
    list = insert_sorted(list, 50);
    print_list(list); // output: 5 10 30 50 100
    *find(list, 10) = 20;
    print_list(list); // output: 5 20 30 50 100
    list = reverse(list);
    print_list(list); // output: 100 50 30 20 5

(You will of course need to test your code more extensively to make sure it works; you can bet we will when we mark your lab.)

Finally, modify the definition of ``enclosing_struct`` so that it *does not use* ``offsetof`` but it does exactly the same thing. You may use ``sizeof`` if you wish, but you don't have to.

Your code is subject to the following restrictions:

- Of the files in the initial repository, you may only modify ``crazylist.c``. We will only use your ``crazylist.c`` together with the original ``crazylist.h``, so if your implementation modifies the ``.h`` file or depends on any additional files, it won't compile and you will receive 0 marks.
  
- Your code must not contain a ``main()`` function, or define any additional non-static functions.

- Your code may not use any libraries other than the ``libc`` linked in by default when you run the C compiler.

- None of the functions may take longer than 5ms on ``castor``


Deliverables
------------

In ``task1``:

- ``crazylist.c``


Hints
-----

- Review pointers in C and make sure you understand how pointer arithmetic works depending on the pointer type. If you don't understand this, you will find this lab far harder than it actually is.



Tasks 2–5 overview
==================

In the remaining tasks, you will implement an allocator similar to a simplified version of what you would find in a C standard library. Each task progressively adds more features.


Source files
------------

In each task folder, you will find the following files:

- The header ``cpen212alloc.h``, which defines the functions you need to implement; you may not modify this file.

- The implementation C file ``cpen212alloc.c``, which you will need to modify to implement your allocator.

- The implementation C file ``cpen212debug.c``, which you should modify to implement heap consistency checking and possibly other debugging functions (like dumping the heap contents so you can examine them).

- The header ``cpen212common.h``, which is included in both ``cpen212alloc.c`` and ``cpen212debug.c``; you may modify this file as part of your implementation.

- The static library ``lib212alloc.a``, which you can link with your allocator to do some trace-based testing (see below).

Currently ``cpen212alloc.c`` contains the dumbest allocator we could come up with: it just allocates blocks sequentially without freeing any of them until it runs out of heap.

When we test your code, we will use only ``cpen212common.h``, and ``cpen212alloc.c`` for most tests, and will evaluate ``cpen212debug.c`` separately; we will be using the original reference version of ``cpen212alloc.h`` as well as our own testing library.

This means that if your implementation depends on any other files — or if ``cpen212alloc.c`` depends on the contents of ``cpen212debug.c``, for example by directly invoking the heap consistency checker — your code will fail to compile and will receive 0 marks.


Constraints
-----------

The code you submit must meet the following restrictions:

1. Code you submit in ``cpen212common.h`` and ``cpen212alloc.h`` may not allocate or map memory (e.g., no ``malloc``), write to the console, interact with files, invoke system calls, and so on. The code in ``cpen212debug.c`` is not subject to this restriction.

2. Your code may not declare global variables (this includes variables declared as ``static`` inside functions).

3. Your code may not modify or access any memory outside the heap area provided to ``cpen212_init``.

4. None of the deliverable files may define ``main``, ``_start``, any other symbols in ``libc``, or any symbols that would prevent linking against the trace driver library.

5. Your code must support initializing multiple independent heaps (via separate calls to ``cpen212_init``) and using them concurrently.

6. The per-heap overhead must not exceed 64 bytes.

7. The minimum possible block size must not exceed 32 bytes, a minimum 8 of which must be user-accessible.

8. The block size granularity must be at most 8 bytes.

9. Your code must support block sizes at least up to the maximum size of virtual addresses on the ARMv8 ISA.

10. Any memory area to the user must start at an address divisible by 8.

11. No single function you implement may take longer than 5ms.

For example, these constraints imply that given a heap size of 96 bytes your allocator must be able to support at least 8 bytes of user-usable space (64B heap overhead + 32B minimum block size).

When testing, we will ensure that the heap size is at least 64 bytes.


Testing
-------

You should first test your code by making a separate file with a ``main`` function and compiling it together only with your ``cpen212alloc.c`` and maybe ``cpen212debug.c``. Don't just 't just put ``main`` inside the deliverable files: you will probably forget to take it out later, and your code will not compile when we test it because our testing library already defines ``main``.

To help you stress your working allocator, we have provided some example memory traces in ``~cpen212/Public/lab3/traces``, as well as a small library that reads them and drives your allocator in ``~cpen212/Public/lab3/lib``. The makefiles provided in each task folder link against this library, so you can examine how to build things manually if you wish.

Once built, you can run the program ``cpen212trace``. You will need to specify the memory trace to run::

    ./cpen212alloc ~cpen212/Public/lab3/traces/trace01.lua

The trace files are just Lua scripts, and all files you provide are evaluated in the same Lua interpreter, so for example if you run::

    ./cpen212alloc foobar.lua ~cpen212/Public/lab3/traces/trace01.lua

then ``foobar.lua`` will be evaluated before the trace, and if you run::
    
    ./cpen212alloc -e"foo=1337" ~cpen212/Public/lab3/traces/trace01.lua

then ``foo`` will be 1337 when the trace file begins. This is useful for quickly experimenting with different heap sizes, for example.

The trace driver is not a replacement for testing your code yourself. It's really only useful once your implementation works most of the time, and so is not very suitable for early development.

Each sample trace uses a specific **heap size** that your allocator should fit in to have a chance of meeting the task requirements. You can of course change this for your tests, but note that if you are not fitting within the original limits then it's likely that you will not pass some of our tests, either.

Note that the sample traces are not intended to test all aspects of your implementation, and do not constitute the full set of inputs we will use for evaluation. For example, the traces fail whenever your allocator can't allocate more memory and returns ``NULL``, but we have tests that *expect* it to do that under certain conditions to pass. Therefore, even if you pass all of the traces, there may be some other tests that you fail that can cost you marks.


Assertions
----------

As you write your code, use ``assert()`` in your C code to verify **every imaginable thing** that you expect to be true at a specific point in your code. If an assertion triggers, either (i) you have a bug in your code, or (ii) you misunderstood some invariant and you need to correct your assertion and probably your code. Both are much better than wading through a giant heap trying to find some header that was corrupted many allocations ago.

For example, our reference solutions are well over 25% asserts in terms of lines of code, and this has been *extremely* helpful in debugging.

When we test your code, we will compile it with ``NDEBUG`` defined to disable assertions, so there is no risk of you failing any tests due to assertions triggering unexpectedly.


Heap consistency checker
------------------------

You almost certainly want to write at least two debugging aids:

- a heap consistency checker, and
- a way to pretty-print your heap in a human-readable format.

Your consistency checker will probably want to walk through the entire heap, examining all the blocks and checking for invariants such as:

- do all the headers have reasonable sizes (e.g., non-zero)?
- do any allocated blocks overlap?
- if you have two copies of some information (e.g., two boundary tags), are the sizes and allocation status bits consistent?
- are any allocated blocks larger than the requested size plus required padding?
- if using an explicit freelist, does it have exactly the same blocks as are marked free on the heap?
- if using segregated freelists, are the sizes correct?
- ...and so on.

The best way to use the heap checker is to invoke it before and after every call to your allocator implementation. This way, if your heap checker is decent, you will discover a lot of problems as soon as they happen. Once you're using traces, you can invoke ``cpen212_debug`` through the ``debug`` command (see trace format below).

Many invariants you might want to check might require tracking some metadata — for example, you might need to keep a copy of all the ``cpen212_alloc``, ``cpen212_realloc``, and ``cpen212_free`` requests and the address ranges for the allocated blocks. You will therefore want to place your checker in ``cpen212debug.c`` so that you can use ``malloc`` etc. to allocate memory dynamically. But be careful to not invoke the code from ``cpen212alloc.c``, because you won't have libc's ``malloc`` and friends available in most tests and in any case we won't link against ``cpen212debug.c`` in most tests.

Your consistency checker does not have to be fast or memory-efficient — you will want to only use it for debugging. This means you can use very basic data structures, such as lists or arrays to represent things like intervals rather than a fancy data structure like an interval tree.

While implementing this is not strictly required, it saves a great of debugging effort, and, more importantly, forces you to think carefully about what the heap should look like — it's one of the best investments of time you can make.


Trace file syntax
-----------------

In addition to running the provided traces, you can create your own traces to test specific features. First, you'll need to initialize the heap::

    h = init(size)

where ``size`` is the total number of bytes on the heap; this must be at least 64 and a multiple of 8. ``init`` will call your ``cpen212_init`` implementation, and will return the allocator state.

You can then allocate some memory::

    p = alloc(h, size)

where ``size`` is the number of bytes you wish to allocate; this invokes your ``cpen212_alloc``. The result ``p`` will be ``nil`` if your ``cpen212_alloc`` returns NULL, so you might want to use ``assert`` to fail if you expect a valid allocation::

    p = assert(alloc(h, size))

You can then free the memory allocated at ``p``::

    p = free(h, p)

which calls ``cpen212_free``, or reallocate it with a different size, perhaps again checking for ``nil``::

    p = assert(realloc(h, p, newsize))

Finally, you can call ``debug`` to invoke your ``cpen212_debug`` function::

    debug(h, n)

where ``n`` is directly passed to your ``cpen212_debug`` as its second argument. None of the traces we provided call this function, but you will find it a lot easier to debug if you implement a heap checker as one of the operations in ``cpen212_debug`` and **call it before and after every other operation** — it's a lot easier to find problems immediately after they've occurred rather than many operations downstream.

The traces are actually Lua programs, so you can write much more sophisticated tests for specific patterns if you wish.



Task 2
======

Implement

- ``cpen212_init``,
- ``cpen212_alloc``,
- ``cpen212_free``, and
- ``cpen212_realloc``

For this task

- your ``cpen212_free`` function **should not do any coalescing**, and
- your ``cpen212_realloc`` should try to allocate a new block, move the data there, and free the previous one.

Your code must also include a comment that explains the fields and layout of the blocks you allocate on the heap, including any headers or footers. If you change this layout in later tasks, you must also document the changes.


Deliverables
------------

In ``task2``:

- ``cpen212alloc.c``
- ``cpen212common.h``


Hints
-----

- Draw things out before you start writing code.

- Try to think of all the invariants that must hold at each point in time, again before starting to write code.

- Remember that the pointer returned to the caller **does not include the header(s)**.

- While the caller may request arbitrary allocation sizes, your allocator must always return blocks allocated on an 8-byte boundary. This means the block size will be a multiple of 8, and you can use the lowest bits for something else if you are careful.

- Review how to set watchpoints on memory locations in GDB. This is really, *really* useful for this lab.

- A heap consistency checker is really helpful for debugging.



Task 3
======

Extend your implementation from the previous task to implement coalescing **after** the freed block (i.e., with the next block if it is not allocated).


Deliverables
------------

In ``task3``:

- ``cpen212alloc.c``
- ``cpen212common.h``


Hints
-----

- Consider implementing your coalescing function(s) as separate ``static`` functions in ``cpen212alloc.c`` and calling them from ``cpen212_free`` as necessary; it's easier to debug this separately then if it's integrated into ``cpen212_free``, and if things don't work five minutes before the deadline it'll be easier to disable parts of your code that way.

- Heap consistency checker. Nuff said.



Task 4
======

Extend your implementation from the previous task to implement coalescing both **before and after** the freed block.


Deliverables
------------

In ``task4``:

- ``cpen212alloc.c``
- ``cpen212common.h``



Task 5
======

Implement a non-trivial ``cpen212_realloc`` in ``cpen212alloc.c`` that is able to extend and shrink existing allocations *in place* if this is possible (including coalescing both ways whenever possible).


Deliverables
------------

In ``task5``:

- ``cpen212alloc.c``
- ``cpen212common.h``


Hints
-----

- Don't forget that there can be free space both *before* and *after* your current block.

- If reallocating at a different address, don't forget to copy the contents to the new location.

- If you need to copy block contents, be careful how you copy the data if the destination overlaps with the source.

- Be careful to not overwrite any headers (or user data) while copying things around.



Marks
=====

To earn marks, you must commit and push each task to the GitHub repo **before the deadline for that task**.

Remember that CPEN 212 labs are **individual**, so you must complete all tasks by yourself; see the academic integrity policy for details.

- Task 1: 2
- Task 2: 2
- Task 3: 2
- Task 4: 2
- Task 5: 2

We test features incrementally, so the tests for later tasks rely on previous tasks working (with the exception of task 1).
