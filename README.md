
![](img/logo.svg)
# Malin Language
This is a compiler for the experimental `malin` programming language.

### The Language
Malin is inspired by c++ and rust and compiles to bytecode via llvm.  
Some of the language goals are: 
 - easy to write/understand (especially compared to c++)
 - performance like c/c++
 - avoid boilerplate code (meta-programming)
 - ease usage of the heap with smart pointers

Here is a simple hello world program:
```c++
func main(): i32 {
  let a: i32 = 1;
  // b has also implicit type i32
  let b = 10;

  // a loop
  let i = 0;
  while i <= 5 {
    i = i+1;
  }
  
  return plusAndMul(a, b) + plusAndMul(a, b, multiplyWith= 2);
  // returns 33
}

/**
 * calc sum of a and b and multiply it with multiplyWith afterwards.
 */
func plusAndMul(a: i32, b: i32, multiplyWith: i32 = 1): i32 {
  return (a + b) * multiplyWith;
}
```
For a more complex example see [plotter.malin](test/plotter.malin) in the `test` folder.  
The files directly in the `example` and `test` folder are working with the current compiler.

### Install
Either install `malinc` from source, see [Build-and-install](#Build-and-install), or use the precompiled binaries, see [release assets](https://gitlab.com/JeyRunner/malin-lang/-/releases).

For debian based systems as ubuntu, a debian package for `malinc` can be downloaded from the [release assets](https://gitlab.com/JeyRunner/malin-lang/-/releases) (or directly the latest version [malinc.deb](https://gitlab.com/JeyRunner/malin-lang/-/jobs/artifacts/master/raw/build/malinc-0.0.0-Linux.deb?job=build)).
For installing the downloaded debian package execute `dpkg` (this may require `sudo`):
```bash
dpkg --install malinc-0.0.0-Linux.deb
# fix dependencies
apt-get -f install
```

### Build and install
First install cmake and c++ compiler.
Then llvm dependency and other decencies:
```bash
# llvm
apt install llvm-9-dev libclang-common-9-dev

# clang is used for linking
apt install clang
```
Now build the compiler:
```bash
# clone this repo
git clone https://gitlab.com/JeyRunner/malin-lang

# go into repo
cd malin-lang

# make build dir
mkdir build
cd build

# compile
cmake ..
make
```
Afterwards the build dir contains the malinc executable
and you can start compiling your first malin program:
```bash
# in the build dir
# note that libmalinCGlued.a has to be be compiled before and is expected to be in './std/c'
./malinc -f myMalinProgram.ma
```

## Roadmap
- [x] globals                                   
- [x] functions                                 
- [x] expressions (binary, unary)                     
    - [x] math and logical calculations                  
- [x] variables (mutable, always copy)          
- [x] buildin types (i32, f32)                  
- [x] control flow: if-else            
- [x] control flow: while loop                  
- [ ] static strings like "abc"                 
- [ ] classes (no inheritance)                  
    - [x] member variables and functions   
    - [ ] pass and return object to/from functions                
    - [ ] custom constructors and destructors             
- [ ] arrays            
- [ ] references            
- [ ] casts for numerical types: ```let y = i32(x)```               
- [ ] move, copy behavior + references                        
- [ ] heap functions as unsafe                  
- [ ] smart pointers for heap (shard, unique)   
- [ ] compile time code
    - [ ] compile time evaluation of statements, expression
    - [ ] compile time functions
    - [ ] compile time type reflection
- [ ] inheritance of classes                  