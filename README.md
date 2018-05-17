# icra
ICRA: a static analyzer based on interprocedural compositional recurrence analysis

# Building ICRA

To build ICRA, first install the dependencies of the duet library.  You will need a version of OCaml compiled with -fPIC runtime libs.  If OPAM is installed this can be done with`opam switch 4.02.3+PIC`, for example.
    (see: duet/README.md)

You will also need to install scons, boost, and glog
    sudo apt-get install scons libboost-all-dev libgoogle-glog-dev

Return to the parent directory and type:
    make

If this command completes successfully, you should obtain an executable named "icra".
