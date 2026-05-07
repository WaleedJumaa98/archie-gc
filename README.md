# Archie GC — Garbage Collector in C

A hand-written garbage collector implemented in C from scratch, built as a deep dive into manual memory management and runtime internals.

## What it does

The Archie runtime manages a small dynamic type system (integers, floats, strings, vectors, and arrays) and uses a **mark-and-sweep garbage collector** to automatically reclaim memory no longer reachable from any active scope.

It also includes a simpler **reference counting** implementation (see the `refcounting` branch) for comparison.

## Why I built this

Most developers use languages where memory management is handled for them (Python, C#, Java). This project is an exploration of what those runtimes are actually doing under the hood — how objects are tracked, how cycles are detected, and why some GC strategies fail where others succeed.

## Garbage collection strategies

### Reference counting
Each object tracks how many pointers point at it. When the count drops to zero, the object is freed.

**Problem:** Cycles. If object A points at B and B points at A, both counts stay above zero even when nothing else references either object. They leak forever.

### Mark and sweep (this project)
The GC runs in three phases:

1. **Mark** — Walk every active stack frame and mark every directly referenced object as reachable.
2. **Trace** — For each marked object, trace into its contents (arrays, vectors) and mark those too.
3. **Sweep** — Walk every object the VM knows about. Free anything unmarked. Reset marks on survivors for the next cycle.

**Advantage over refcounting:** Reachability is determined from live scopes, not from reference counts. Cycles between objects that are no longer referenced from any scope are correctly collected.

## Project structure

```
archie-gc/
├── include/
│   ├── archie_object.h   # Object types, tagged union, free function
│   ├── archie_new.h      # Constructors for each object type
│   ├── vm.h              # Virtual machine and frame types, GC interface
│   └── stack.h           # Generic stack data structure
├── src/
│   ├── archie_object.c   # Object implementation (free, array get/set, add)
│   ├── archie_new.c      # Object constructors
│   ├── vm.c              # VM lifecycle, frame management, GC (mark/trace/sweep)
│   ├── stack.c           # Stack implementation
│   └── main.c            # Demo program
└── Makefile
```

## Object types

| Type      | Description                              |
|-----------|------------------------------------------|
| `INTEGER` | 32-bit signed integer                    |
| `FLOAT`   | 32-bit float                             |
| `STRING`  | Heap-allocated null-terminated string    |
| `VECTOR3` | Three archie object pointers (x, y, z)   |
| `ARRAY`   | Dynamically sized array of object pointers |

All types are represented as a tagged union (`archie_object_t`) with a `kind` field that determines how the data is interpreted at runtime.

## Building and running

```bash
make
./archie-gc
```

Expected output demonstrates four scenarios:
- Basic GC: an object is freed when its frame is popped
- Multiple frames: unreferenced objects are collected while referenced ones survive
- Cyclic reference: two arrays pointing at each other are correctly collected once no frame holds either (this is the case that breaks reference counting)
- Vector3 tracing: the GC traces into a vector to keep its components alive

## Key concepts

**Stack frames** represent active scopes (function calls). Each frame holds references to the objects used in that scope. When a scope exits, its frame is popped, and any objects it was the last to reference become candidates for collection.

**The VM** maintains two stacks: one for frames (active scopes) and one for all tracked objects. The GC walks the frames stack to find roots, then sweeps the objects stack to collect garbage.

**The mark bit** (`is_marked`) on each object is set during the mark and trace phases, then cleared by the sweep phase on surviving objects. Objects without the mark bit after tracing are freed.

## What I learned

- Manual memory management: `malloc`, `calloc`, `realloc`, `free`
- Pointer arithmetic and double pointers (`void **`, `archie_object_t **`)
- Tagged unions for polymorphic runtime types
- Stack data structure built from scratch in C
- Why reference counting leaks on cyclic references
- How tracing GCs solve the cycle problem
- Stop-the-world GC pauses and their tradeoffs vs. incremental collection
- The difference between stack-allocated and heap-allocated memory, and why long-lived objects must live on the heap
