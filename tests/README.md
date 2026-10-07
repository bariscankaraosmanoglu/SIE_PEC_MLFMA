# Thread parallelization regression checks

For per-RWG CFIE coefficients and open-surface enforcement, see
[README_CFIE.md](README_CFIE.md).

Run from the repository root:

```sh
python3 -B tests/check_thread_parallel.py
python3 -B tests/check_thread_parallel.py --sanitize
```

The default mesh is the tracked `src/input.pnu`. For the smaller local sphere
fixture used during this fix, add `--input src/test_29_sphere_0p03m.pnu`.
That fixture was already present locally but is not tracked. All test binaries,
logs, inputs, and result files are written to a temporary directory. Use
`--output-dir /tmp/chosen-directory` to retain them at a specified location.

The operator driver changes the fixture's preconditioner to 2 and its
approximation factor to 0.8 to exercise both far operators with valid
interpolation dimensions. It uses a deterministic complex vector independently
of GMRES and checks:

- Exactly-once partition coverage for empty, zero-work, uneven, and large-weight
  cases, including more workers than boxes and large team-index arithmetic.
- Actual OpenMP teams, including serialized execution and a two-thread limit.
- Explicit rejection of zero and negative requested thread counts.
- Near MVM against an independently accumulated stored-block product.
- Byte-identical near matrices and accelerated far-field text across thread
  configurations; near, accurate far, and approximate far vectors must agree
  within relative L2 error `1e-12`, stricter than the fixture's `0.001` tolerance.
- Requested teams of 1, 2, 8, 16, and 32, OpenMP disabled with 8 requested,
  `OMP_THREAD_LIMIT=2` with 32 requested, and dynamic team adjustment.

`--sanitize` uses AddressSanitizer and UBSan. Leak detection is disabled because
the solver has existing leaks outside this fix. This does not disable address
or undefined-behavior checks. `--baseline-dir` can compare against outputs saved
from the unmodified one-thread operator driver.

## Validation of this change

Baseline sources and artifacts were saved under `/tmp/mlfma-rank1` before edits.
The unmodified partitioner failed AddressSanitizer at
`calculate_loadbalance.cpp:54`, writing past `thread_boxes_Rad`. With OpenMP
disabled and eight threads requested, the unmodified near matrix differed from
the one-thread reference by relative L2 error `0.9378956627575311`.

Both regression commands passed after the fix on the small sphere fixture.
All configurations matched the unmodified one-thread near matrix, operator
vectors, and accelerated far-field file byte for byte. The independent near
product error was `4.93e-16`. Dynamic adjustment produced seven actual workers
for a request of 32 on this host; the thread-limit case used two.

The solver was rebuilt before and after using the root `AGENTS.md` commands:

```sh
rm run_mlfma
g++ -Wall -fexceptions -O3 -std=gnu++17 -fopenmp src/*.cpp -o run_mlfma
```

Existing compiler warnings remain. The unmodified small sphere input was also
run through the full solver in temporary working directories. RWG coefficients,
triangle currents, contribution diagnostics, and far fields were byte-identical
before and after.

Three sequential runs per configuration were measured with
`/usr/bin/time -f '%e %M' -o resources.txt <executable>`, using
`OMP_DYNAMIC=FALSE` and no thread limit. The operator measurement includes mesh
setup, assembly, one near/accurate-far/approximate-far product, and field output.

| Check | Median seconds before / after | Median peak RSS KiB before / after |
| --- | --- | --- |
| Operators, 1 thread | 1.06 / 1.07 | 31,240 / 31,300 |
| Operators, 8 threads | 0.39 / 0.38 | 31,944 / 31,972 |
| Full solver, 8 threads | 1.51 / 1.50 | 33,352 / 33,348 |

These small-case timings do not establish a performance improvement. The host
reports eight logical CPUs; 16- and 32-worker checks establish correctness under
oversubscription, not throughput scaling on hardware with that many cores.

Helgrind was run on the actual operator driver with a small octahedron and two
workers, before and after:

```sh
valgrind --tool=helgrind --fair-sched=yes --history-level=full \
  --error-exitcode=97 --xml=yes --xml-file=helgrind.xml <operator-driver> 1 2
```

Contexts involving `calculate_sin_cos` dropped from 51 to zero, including 20
contexts containing conflicting sine/cosine stacks. Both runs still reported
other accesses involving libgomp and the solver. This is targeted evidence for
the table fix, not a clean whole-program race audit.

Existing CRLF source line endings were preserved. Check patch whitespace with:

```sh
git -c core.whitespace=blank-at-eol,blank-at-eof,space-before-tab,cr-at-eol diff --check
```
