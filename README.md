# SIE_PEC_MLFMA

A C++ solver for electromagnetic scattering from perfect electric conductor
(PEC) surfaces in free space. It discretizes surface integral equations with
Rao–Wilton–Glisson (RWG) basis functions on triangular meshes and accelerates
matrix-vector products with the Multilevel Fast Multipole Algorithm (MLFMA).
Outputs include surface-current coefficients, triangle-current magnitudes,
scattered far fields, and convergence diagnostics.

Author: **Bariscan Karaosmanoglu, PhD**.

## Scope

- Closed PEC surfaces support a combined-field integral equation (CFIE), with
  an electric-field integral equation (EFIE) weight `a` supplied per model part.
  Each testing RWG uses the average of its two triangles' part weights.
- Open connected surfaces use EFIE: the solver forces `a = 1` for every RWG on
  the component, including interior RWGs away from boundary edges.
- The executable supports one frequency per run and plane-wave illumination,
  with optional sweeps over incident directions and two polarization selectors.
- Near interactions are assembled into stored MoM blocks; far interactions use
  MLFMA aggregation, translation, and disaggregation. GMRES or flexible GMRES
  solves the resulting linear system.
- Parallelism uses shared-memory OpenMP threads. There is no MPI or GPU backend.
  Angular tables, interpolation stencils, and eight child-shift phases per level
  are reused; full per-RWG radiation-pattern caching is not used.

The shared input format retains `Formulations` and `source_type` for compatibility
with related repositories. This PEC executable accepts only `Formulations 0`
and `source_type 0`. Impedance boundary conditions, other illumination types,
and frequency sweeps are rejected rather than silently substituted.

## Build

Requirements:

- GNU Make and a C++ compiler supporting GNU C++17 and OpenMP.
- The compiler's OpenMP runtime (for GCC, `libgomp`).
- No external BLAS, LAPACK, or mesh library is required. Python 3 is needed only
  for the optional regression scripts.

From the repository root:

```sh
make -j2
```

This produces `./run_mlfma`; intermediate objects and generated header
dependencies are stored in `build/`. Choose the `-j` count to suit available
build memory. Build jobs are independent of the solver's runtime thread count.
Building does not run the solver or tests.

Default options preserve the existing build:

```sh
g++ -Wall -fexceptions -O3 -std=gnu++17 -fopenmp src/*.cpp -o run_mlfma
```

The Makefile compiles individual translation units and links them incrementally.
Header changes rebuild affected objects; changes to the compiler, flags, or
source list rebuild the configuration. GCC 11.4.0 and GNU Make 4.3 have been used
locally. Existing source warnings are not treated as errors.

Standard overrides are available: `CXX`, `CPPFLAGS`, `CXXFLAGS`, `LDFLAGS`,
`LDLIBS`, plus `OPENMP_FLAGS`, `BUILD_DIR`, and `TARGET`. OpenMP options are
applied during compilation and linking. Keep GNU C++17 and OpenMP enabled.
For a separate debug build:

```sh
make -j2 BUILD_DIR=build/debug TARGET=build/debug/run_mlfma \
  CXXFLAGS='-Wall -fexceptions -O0 -g -std=gnu++17'
```

`make help` lists targets. `make clean` explicitly removes the selected
configuration's objects, dependency files, configuration record, and executable;
it leaves simulation inputs, outputs, and logs intact. Supply the same
`BUILD_DIR` and `TARGET` overrides when cleaning a custom build. No cleanup runs
automatically.

### Optional compiler optimizations

The default build remains unchanged. The following optional GCC switches apply
at both compilation and linking, and changes trigger a rebuild:

| Make variable | Effect |
| --- | --- |
| `LTO=2` | Enable link-time optimization with up to two compiler jobs at the LTO stage. Default `LTO=0` disables it. |
| `TUNE=native` | Tune for the build CPU while retaining the selected instruction set. Default: no additional tuning flag. |
| `ARCH=native` | Enable the build CPU's instruction set. The executable may require that CPU's features on other systems. Default: no additional architecture flag. |
| `PGO=generate` | Build an instrumented executable for profile collection, using atomic counter updates for OpenMP workers. |
| `PGO=use` | Rebuild using collected profiles; missing or mismatched profiles are compilation errors. Default `PGO=off` disables profiling. |
| `PROFILE_DIR=path` | Profile-data location, resolved to an absolute path. Default: `$(BUILD_DIR)/profiles`. |

`ARCH` and `TUNE` also accept explicit GCC CPU names. For an initial LTO/tuning
candidate that retains the default instruction set:

```sh
make -j2 LTO=2 TUNE=native \
  BUILD_DIR=build/lto TARGET=build/lto/run_mlfma
```

For an executable intended for the build machine's CPU features:

```sh
make -j2 LTO=2 ARCH=native \
  BUILD_DIR=build/native TARGET=build/native/run_mlfma
```

The separate directories retain the default executable and build products.
LTO can increase build time and compiler memory. CPU-specific instructions,
particularly fused multiply-add, can change floating-point rounding. These
options are candidates for measurement, not a guarantee of faster solves or
bit-identical outputs. Check physical residuals, currents, and far fields against
the default build at the same numerical settings before adopting a candidate.
`-Ofast` and `-ffast-math` are not enabled by these switches because their
relaxed floating-point assumptions can affect solver arithmetic and checks.
See GCC's [optimization options](https://gcc.gnu.org/onlinedocs/gcc-11.4.0/gcc/Optimize-Options.html)
and [x86 options](https://gcc.gnu.org/onlinedocs/gcc-11.4.0/gcc/x86-Options.html).

Profile-guided optimization requires a separate training stage:

1. Build the instrumented executable:

   ```sh
   make -j2 PGO=generate BUILD_DIR=build/pgo TARGET=build/pgo/run_mlfma
   ```

2. Run that executable on representative inputs using the separate working
   directories described below. Profiles go to the absolute `PROFILE_DIR`, even
   when the solver runs elsewhere. Training is never performed by `make`.
   Agent-executed training requires explicit approval under `AGENTS.md`.
3. Preserve the instrumented executable and rebuild using the collected data:

   ```sh
   cp build/pgo/run_mlfma build/pgo/run_mlfma.instrumented
   make -j2 PGO=use BUILD_DIR=build/pgo TARGET=build/pgo/run_mlfma
   ```

Keep the same object directory, compiler, sources, and optimization settings
between collection and use; change only `PGO`. Use a fresh profile directory
when sources or settings change. Updated `.gcda` data triggers recompilation
in `PGO=use` mode, including nested profile directories from absolute object
paths. `make clean` retains profile data. Instrumented execution has
profiling overhead and should not be used as the optimized timing result.
See GCC's [instrumentation options](https://gcc.gnu.org/onlinedocs/gcc-11.4.0/gcc/Instrumentation-Options.html).

## Run a simulation

The executable takes no command-line arguments. It always reads **`input.pnu`
from the current working directory**, and writes results to that directory.
The mesh is embedded in the input; `model_name` is a label and does not cause
the solver to open a separate VTK file.

Run each case in a fresh directory so earlier results remain available. For
example, after building from the repository root:

```sh
repo_dir="$PWD"
run_dir=$(mktemp -d /tmp/mlfma-run.XXXXXX)
cp src/test_29_sphere_0p03m.pnu "$run_dir/input.pnu"
(
  cd "$run_dir" || exit 1
  OMP_DYNAMIC=FALSE "$repo_dir/run_mlfma" > solver.log 2>&1
)
solver_status=$?
printf 'Exit status: %s\nResults: %s\n' "$solver_status" "$run_dir"
```

This uses the smaller included sphere: 1,362 points, 2,720 triangles, 1 GHz,
one illumination, preconditioner 0, tolerance `0.001`, and eight requested
threads. The directory and its contents are retained. Inspect `solver.log`
and `result_solver_status.txt` before interpreting fields or currents.

Other included inputs:

| Input | Geometry | Current settings of interest |
| --- | --- | --- |
| `src/input.pnu` | Sphere, 12,531 points / 25,058 triangles | 3 GHz, one illumination, preconditioner 2, accelerated far-field grid |
| `src/test_23_sphere_0p01m.pnu` | Same sphere mesh | 3 GHz, incident-direction sweep, preconditioner 2, monostatic far field |
| `src/input_som.pnu` | SOM target, 91,923 points / 183,842 triangles | 5 GHz, one illumination, preconditioner 2, monostatic far field |

Read the input header before running: larger meshes, angular sweeps, and nested
solves can require substantially more runtime and memory than the smaller
sphere. Change settings in a copy of the input to preserve the original case.

## Input format and settings

The text reader expects the named parameter records in the order below.
An optional preamble precedes `SimulationInputs`; blank lines are allowed.
Do not add inline comments to parameter or mesh records: their field counts
are checked exactly. Coordinates are in metres, frequency in hertz, and input
angles in degrees. Theta is the polar angle from +z; phi is the azimuth about z.

| Record | Meaning |
| --- | --- |
| `model_name name` | Single-token model label, at most 199 bytes. |
| `Formulations 0` | PEC formulation selector; only 0 is supported here. |
| `Frequency start end count` | Positive frequency endpoints; count must be 1. The start frequency is used. |
| `source_type 0` | Plane-wave illumination; only 0 is supported here. |
| `source_PW theta_start theta_end n_theta phi_start phi_end n_phi pol` | Incident propagation directions sampled inclusively. Counts must be positive. A count of 1 uses the corresponding start angle. `pol` is 0 or 1. |
| `Preconditioners mode` | 0: GMRES without preconditioning; 1: GMRES with left block-diagonal preconditioning using pivoted LU factors; 2: FGMRES with an inner approximate-MLFMA GMRES solve. |
| `num_mlfmalevels levels` | 1 selects the tree depth automatically; larger values request an explicit depth within checked index limits. |
| `dir_ff selector` | Far-field output mode; see below. |
| `num_digits value` | Positive parameter controlling MLFMA truncation; it is not a guarantee of that many accurate output digits. |
| `val_approx value` | Mode-2 harmonic reduction factor in `[0,1]`. Reduced grids must still support the interpolation stencil. |
| `num_interpol value` | Positive angular interpolation stencil width; must fit every grid where it is used. |
| `val_itermax_pc value` | Positive inner GMRES iteration limit for mode 2. Inner tolerance is currently fixed at `0.1` in `pc_solution()`. |
| `val_soltol value` | Outer relative residual tolerance, strictly between 0 and 1. |
| `dir_cfc theta phi` | Direction for the post-solve current/field contribution diagnostic. |
| `val_itermax value` | Positive outer iteration budget; production FGMRES also uses it as the restart length. |
| `parallel_omp enabled threads` | `enabled` is 0 or 1; requested thread count must be positive even when disabled. |

The solver sets the requested OpenMP team size from `parallel_omp`; setting
`OMP_NUM_THREADS` alone does not replace that input. Runtime limits such as
`OMP_THREAD_LIMIT` and dynamic adjustment can reduce the actual team. Use
`OMP_DYNAMIC=FALSE` for repeatable team requests. The solver handles actual teams
that differ from the request, but speedup depends on the case and hardware.

For polarization, `calculate_dir_pw()` constructs the electric-field direction
as `(theta_E, phi_E) = (fmod(theta + 90, 180), phi)` for `pol = 0`, and
`(90, fmod(phi + 90, 360))` for `pol = 1`, using C++ `fmod`.
This states the implementation's sign
convention explicitly. Illuminations are ordered with theta outermost and phi
innermost; result indices start at zero.

After the parameters, the mesh sections have this structure (angle brackets
indicate placeholders, not literal input):

```text
## End-Param
ModelParts <number_of_parts>
<part_name_0> <a_0>
... one name and CFIE weight per part ...
POINTS <number_of_points>
<x_0> <y_0> <z_0>
... one coordinate record per point ...
CELLS <number_of_triangles>
<part_index> <node_0> <node_1> <node_2>
... one connectivity record per triangle ...
```

Part and node indices are **zero based**. `ModelParts` contains 1–50 parts,
each with `0 <= a <= 1`; `a = 1` selects EFIE. Part names do not define different
material properties in this PEC solver. Use consistently oriented triangles,
with outward normals for closed surfaces. The reader checks finite coordinates,
valid indices, nonzero edges/areas, and duplicate triangles; topology checks
reject nonmanifold edges and vertices. These checks do not repair mesh winding.
RWGs span shared triangle edges; boundary edges do not create half-RWG unknowns.

`dir_ff` selects:

| Value | Output |
| --- | --- |
| `< 0` | Direct far fields on three principal-plane cuts (ZX, ZY, XY), 361 directions each. |
| `0` | Direct monostatic backscatter, one observation direction per illumination. |
| `0 < value < 5` | Accelerated MLFMA far field on a fixed 1-degree grid: theta 0–180, phi 0–360. The value selects the mode, not the angular spacing. |
| `>= 5` | Direct full angular grid using this value as the step in degrees. Use a step dividing both 180 and 360 exactly, such as 5 or 10. Incompatible dimensions are rejected. |

## Outputs and convergence

Files use the historical spelling **`farefield`** in their names.

| File | Contents |
| --- | --- |
| `result_solver_status.txt` | One row per illumination: overall status, Krylov status, iterations, outer Krylov matrix-product count, true Krylov residual, true physical residual, and tolerance; column names appear in the header. |
| `result_farefield.txt` | Monostatic results (`dir_ff = 0`), appended in illumination order within the run. |
| `result_farefield_<index>.txt` | Direct cuts or direct angular grid for each illumination. |
| `result_farefield_mlfma_<index>.txt` | Accelerated angular grid for each illumination. |
| `result_rwgcoefficient.txt` | Real and imaginary parts of the final illumination's RWG solution, in the solver's internal RWG ordering. |
| `result_currentontris.txt` | One current-vector magnitude per input triangle, for the final illumination. |
| `result_cfcontris.txt` | Leaf-box field-contribution magnitude divided by total-field magnitude in `dir_cfc`, assigned to triangles in the box, for the final illumination. |

Far-field columns are theta, phi, `abs(E_theta)`, `phase(E_theta)`,
`abs(E_phi)`, `phase(E_phi)`. Direction angles are in **degrees** and complex
phases in **radians**. These are field amplitudes, not an RCS-in-dB output.
The field routines return far-zone coefficients with the radial propagation
factor omitted; their normalization is implemented in `calculate_farfield()`
and `calculate_farfield_mlfma()`.

The physical residual is evaluated as `||b - A*x|| / ||b||` with the original
RHS and accurate, unpreconditioned MLFMA operator. A zero RHS uses an absolute
residual. For mode 1, this can differ from the Krylov residual of the
left-preconditioned system. The reported outer product count excludes inner
preconditioner products and the additional physical-residual check.

Exit status 0 means all illuminations reported convergence and the status file
opened successfully. Status 1 indicates a reported solve failure or input error
(or failure to open the status file). Fields and coefficients can still be
written for failed solves, so their presence alone does not establish convergence.
Meeting the residual tolerance checks the implemented discrete MLFMA equation;
it does not by itself establish mesh, quadrature, or MLFMA approximation accuracy.

## Code layout and solution path

`src/main.cpp` runs the following stages:

1. `read_simulationinput()` parses the parameters and embedded mesh, validates
   them, and derives free-space electromagnetic constants.
2. `generate_edgeconnection()` classifies open components and builds RWG
   connectivity, triangle geometry, and normals.
3. `generate_octtree()`, `generate_nearfarboxes()`, and the `generate_Z*` /
   `generate_translationcomponent()` functions construct the hierarchy and
   near/far interaction data.
4. `calculate_loadbalance()`, `calculate_Znearmatrix()`, and
   `calculate_translationoperator()` prepare work partitions, near blocks, and
   far translations. `calculate_blockdiagonalpreconditioner()` factors blocks
   only for preconditioner mode 1.
5. `stage_solution()` builds each plane-wave RHS, runs GMRES/FGMRES, checks the
   physical residual, and writes the far field for each illumination.
6. `stage_aftersolution()` writes final-illumination currents and diagnostics;
   `clear_memory()` releases simulation storage and cached workspace.

The operator is in `mvm.cpp`, `mvm_Zneardotx.cpp`, and `mvm_Zfardotx.cpp`;
`mvm_Zfardotx_in.cpp` supplies the approximate far operator for nested solves.
Krylov implementations are in `gmres.cpp` and `krylov.cpp`, with flexible-solver
entry points in `fgmres.cpp` / `fgmres_in.cpp`. `mlfma_workspace.h/.cpp` owns
reusable scratch storage and shared angular/interpolation/shift tables.
Global simulation state means the solver is not a reentrant library interface.

## Regression checks and development notes

Existing Python regression drivers build their own test executables and retain
artifacts. They are separate from `make`; see [tests/README.md](tests/README.md)
for thread checks and links to CFIE, Krylov, input-validation, block-factorization,
workspace, and angular-cache suites. Follow the repository's `AGENTS.md` approval
requirements before an agent executes tests, simulations, or benchmarks.

[CHANGELOG_DEV.md](CHANGELOG_DEV.md) records development and validation history;
[HANDOFF.md](HANDOFF.md) records the current task state and retained artifacts.

## License

Project-owned code and documentation are licensed under the [MIT License](LICENSE).
`src/times.hpp` retains its existing copyright notice for Mehmet Nuri; the
project license does not override third-party rights or grant permissions for
that separately attributed material.
