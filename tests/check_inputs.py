#!/usr/bin/env python3
"""Input and derived-dimension regressions; all artifacts are retained."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import tempfile
from check_cfie import fixture, grid

ROOT = Path(__file__).resolve().parents[1]


def cases():
    vertices = [(0.03,0,0),(-0.03,0,0),(0,0.03,0),(0,-0.03,0),(0,0,0.03),(0,0,-0.03)]
    faces = [(0,0,2,4),(0,2,1,4),(0,1,3,4),(0,3,0,4),
             (0,2,0,5),(0,1,2,5),(0,3,1,5),(0,0,3,5)]
    base = fixture(vertices,faces,[0.5]).replace("Preconditioners 2","Preconditioners 0").replace("dir_ff 1","dir_ff 0")
    result=[("valid",base,"tree",None)]
    def change(name,old,new,diagnostic,mode="read"):
        assert old in base;result.append((name,base.replace(old,new,1),mode,diagnostic))
    change("wrong-label","val_soltol","val_soltoll","val_soltol")
    change("extra-field","val_soltol 0.001","val_soltol 0.001 7","val_soltol")
    change("bad-number","val_soltol 0.001","val_soltol nope","val_soltol")
    change("partial-number","val_itermax 192","val_itermax 192oops","val_itermax")
    change("huge-integer","val_itermax 192","val_itermax 999999999999999999999","val_itermax")
    change("long-name","model_name regression","model_name "+"x"*250,"model_name")
    change("missing-header","SimulationInputs","SimulationInput","SimulationInputs")
    change("missing-end","## End-Param","## End-Par","End-Param")
    result.append(("missing-points",base.split("POINTS")[0],"read","POINTS"))
    result.append(("missing-cells",base.split("CELLS")[0],"read","CELLS"))
    result.append(("short-points",base.split("POINTS")[0]+"POINTS 6\n0 0 0\n","read","POINTS"))
    result.append(("short-cells",base.rsplit("\n",3)[0]+"\n","read","CELLS"))
    change("impedance-mode","Formulations 0","Formulations 1","Formulations")
    change("unknown-formulation","Formulations 0","Formulations 9","Formulations")
    for mode in (1,2,3,4,9): change(f"source-{mode}","source_type 0",f"source_type {mode}","source_type")
    change("frequency-sweep","Frequency 1000000000 1000000000 1","Frequency 1000000000 2000000000 2","Frequency")
    change("single-endpoint","Frequency 1000000000 1000000000 1","Frequency 1000000000 2000000000 1",None)
    for value in ("0","-1","nan","inf"):
        change("frequency-"+value,"Frequency 1000000000 1000000000 1",f"Frequency {value} 1000000000 1","Frequency")
    for field,old,values in [
        ("Preconditioners","0",["-1","3"]), ("num_mlfmalevels","1",["0","-1","100"]),
        ("num_digits","3",["0","-1"]), ("num_interpol","2",["0","-1","2147483647"]),
        ("val_itermax","192",["0","-1"]), ("val_soltol","0.001",["0","-1","nan","inf","1"]),
        ("ModelParts","1",["0","-1","51"]), ("POINTS","6",["0","-1","2147483647"]),
        ("CELLS","8",["0","-1","2147483647"]), ("dir_ff","0",["nan","inf"])
    ]:
        for value in values: change(field+"-"+value,field+" "+old,field+" "+value,field)
    for value in ("-0.1","1.1","nan","inf"):
        change("alpha-"+value,"part0 0.5","part0 "+value,"part0")
    change("material-negative","0 0 2 4","-1 0 2 4","CELLS")
    change("material-high","0 0 2 4","1 0 2 4","CELLS")
    change("node-negative","0 0 2 4","0 -1 2 4","CELLS")
    change("node-high","0 0 2 4","0 6 2 4","CELLS")
    change("repeated-node","0 0 2 4","0 0 0 4","CELLS")
    change("nonfinite-coordinate","0.03 0 0","nan 0 0","POINTS")
    # Distinct nodes on a line are topologically valid but geometrically singular.
    result.append(("collinear",fixture([(0,0,0),(1,0,0),(2,0,0)],[(0,0,1,2)],[0.5]),"read","CELLS"))
    result.append(("no-rwgs",fixture([(0,0,0),(1,0,0),(0,1,0)],[(0,0,1,2)],[0.5]),"tree","RWG"))
    for field,old,new in [("source_PW","37 37 1 19 19 1 0","37 37 0 19 19 1 0"),
                          ("source_PW","37 37 1 19 19 1 0","37 37 1 19 19 1 2"),
                          ("source_PW","37 37 1 19 19 1 0","37 37 2147483647 19 19 2 0"),
                          ("parallel_omp","1 1","2 1"), ("parallel_omp","1 1","1 0"),
                          ("parallel_omp","1 1","1 -1")]:
        change(field+"-"+new.replace(" ","_"),field+" "+old,field+" "+new,field)
    change("threads32","parallel_omp 1 1","parallel_omp 1 32",None)
    change("unused-approximation","val_approx 0.8","val_approx 0",None)
    change("unused-inner-limit","val_itermax_pc 15","val_itermax_pc -1",None)
    change("fractional-valid-step","dir_ff 0","dir_ff 7.5",None)
    change("large-declaration","POINTS 6","POINTS 100000000","POINTS")
    change("duplicate-triangle","0 0 3 5","0 0 2 4","CELLS")
    result.append(("extra-cell",base+"0 0 2 4\n","read","CELLS"))
    result.append(("metadata",base+"ExtraSharedSection retained\n","read",None))
    result.append(("parts50",fixture(vertices,[(49,*face[1:]) for face in faces],[0.5]*50),"tree",None))
    for value in ("-0.1","1.1","nan"):
        result.append(("active-approx-"+value,base.replace("Preconditioners 0","Preconditioners 2").replace("val_approx 0.8","val_approx "+value),"read","val_approx"))
    result.append(("inner-negative",base.replace("Preconditioners 0","Preconditioners 2").replace("val_itermax_pc 15","val_itermax_pc -1"),"read","val_itermax_pc"))
    result.append(("stencil",base.replace("num_interpol 2","num_interpol 100").replace("dir_ff 0","dir_ff 1"),"tree","num_interpol"))
    change("unsafe-direct-grid","dir_ff 0","dir_ff 7","dir_ff")
    for value in ("0", "0.0001"):
        text=base.replace("Preconditioners 0","Preconditioners 2").replace("val_approx 0.8","val_approx "+value)
        result.append(("safe-zero-truncation-"+value,text,"tree",None))
        result.append(("unsafe-zero-truncation-"+value,text.replace("num_mlfmalevels 1","num_mlfmalevels 4"),"tree","num_interpol"))
    for field,text in [("dir_ff -1",base.replace("dir_ff 0","dir_ff -1")),("dir_ff 5",base.replace("dir_ff 0","dir_ff 5"))]:
        result.append((field.replace(" ","_"),text,"tree",None))
    points,open_faces=grid()
    result.append(("open",fixture(points,open_faces,[0.5]),"tree",None))
    return result


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-dir",type=Path,default=ROOT/"src")
    parser.add_argument("--output-dir",type=Path)
    parser.add_argument("--sanitize",action="store_true")
    parser.add_argument("--baseline",action="store_true")
    parser.add_argument("--baseline-dir",type=Path)
    args=parser.parse_args()
    output=(args.output_dir or Path(tempfile.mkdtemp(prefix="mlfma-inputs-"))).resolve();output.mkdir(parents=True,exist_ok=True)
    source=args.source_dir.resolve()
    flags=["-std=gnu++17","-fopenmp","-I"+str(source)]
    flags += ["-O1","-g","-fsanitize=address,undefined,float-cast-overflow","-fno-omit-frame-pointer"] if args.sanitize else ["-O3"]
    files=["read_simulationinput.cpp","generate_edgeconnection.cpp","surface_components.cpp","generate_octtree.cpp"]
    if (source/"input_validation.cpp").exists():files.append("input_validation.cpp")
    if (source/"mlfma_workspace.cpp").exists():
        files += ["mlfma_workspace.cpp","sample_locations_weights.cpp","interpolationweights.cpp"]
    command=["g++",*flags,str(ROOT/"tests/input_validation.cpp"),*[str(source/p) for p in files],"-o",str(output/"input_validation")]
    (output/"build-command.json").write_text(json.dumps(command,indent=2)+"\n")
    with (output/"build.log").open("w") as log:subprocess.run(command,stdout=log,stderr=log,check=True)
    dimension_files=["generate_Znearcomponent.cpp","generate_Zfarcomponents.cpp","generate_translationcomponent.cpp","interpolationweights.cpp"]
    if (source/"input_validation.cpp").exists():dimension_files.append("input_validation.cpp")
    command=["g++",*flags,str(ROOT/"tests/input_dimensions.cpp"),*[str(source/p) for p in dimension_files],"-o",str(output/"input_dimensions")]
    with (output/"dimension-build.log").open("w") as log:subprocess.run(command,stdout=log,stderr=log,check=True)
    env=dict(os.environ,ASAN_OPTIONS="detect_leaks=0",UBSAN_OPTIONS="halt_on_error=1",OMP_DYNAMIC="FALSE")
    summary={}
    for name,text,mode,diagnostic in cases():
        d=output/name;d.mkdir(exist_ok=True);(d/"input.pnu").write_text(text)
        with (d/"run.log").open("w") as log:
            try:
                run=subprocess.run([str(output/"input_validation"),mode],cwd=d,stdout=log,stderr=log,env=env,timeout=4)
                code=run.returncode
            except subprocess.TimeoutExpired:code="timeout"
        summary[name]=code
        if not args.baseline:
            assert code==(1 if diagnostic else 0),(name,code,d/"run.log")
            text_log=(d/"run.log").read_text()
            if diagnostic:assert diagnostic in text_log and "Input error" in text_log,(name,text_log)
            if args.baseline_dir and not diagnostic and (args.baseline_dir/name/"parsed.bin").exists():
                assert (d/"parsed.bin").read_bytes()==(args.baseline_dir/name/"parsed.bin").read_bytes(),name
        print(name,code,flush=True)
    dimensions=[("interpolation","num_interpol"),("near","near matrix"),("aggregation","aggregation"),("translation","translation")]
    if not args.baseline:dimensions.append(("limits",None))
    for mode,diagnostic in dimensions:
        d=output/("dimensions-"+mode);d.mkdir(exist_ok=True)
        with (d/"run.log").open("w") as log:
            run=subprocess.run([str(output/"input_dimensions"),mode],cwd=d,stdout=log,stderr=log,env=env,timeout=4)
        summary["dimensions-"+mode]=run.returncode
        if not args.baseline:
            assert run.returncode==(1 if diagnostic else 0),(mode,d/"run.log")
            if diagnostic:assert diagnostic in (d/"run.log").read_text()
        print("dimensions-"+mode,run.returncode,flush=True)
    (output/"results.json").write_text(json.dumps(summary,indent=2)+"\n")
    print("Artifacts:",output)


if __name__=="__main__":main()
