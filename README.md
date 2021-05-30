![](img/logo.svg)
 
[![pipeline status](https://gitlab.com/JeyRunner/malin-lang/badges/master/pipeline.svg)](https://gitlab.com/JeyRunner/malin-lang/-/commits/master) 

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
Either install `malinc` from source, see [Build from source](#Build-from-source), or use the precompiled binaries, see [release assets](https://gitlab.com/JeyRunner/malin-lang/-/releases),
or install the debian package.

#### Precompiled binary
Download the precompiled binary archive or the installer script from the [release assets](https://gitlab.com/JeyRunner/malin-lang/-/releases).
Note, that you also have to install the `clang` dependency:
* For debian based systems:
  ```bash
  apt install clang
  ```
* For arch systems:
  ```bash
  pacman -S clang
  ```  

#### Debian package
For debian based systems as ubuntu, a debian package for `malinc` can be downloaded from the [release assets](https://gitlab.com/JeyRunner/malin-lang/-/releases) (or directly the latest version [malinc.deb](https://gitlab.com/JeyRunner/malin-lang/-/jobs/artifacts/master/raw/build/malinc-0.0.0-Linux.deb?job=build)).
For installing the downloaded debian package execute `dpkg` (this may require `sudo`):
```bash
dpkg --install malinc-0.0.0-Linux.deb
# fix dependencies
apt-get -f install
```

### Compile a malin program
After installing the compiler, you can start compiling your first malin program:
```bash
# note that libmalinCGlued.a has to be be compiled before and is expected to be in './std/c' (only when malinc was not globally installed)
malinc -f myMalinProgram.ma

# run the compiled executable
./bin.o
```
As a shortcut you can directly compile and run your program at once:
```bash
malinc -f myMalinProgram.ma --run
```
To show all available calling options call `malinc --help`.


### Build from source
First install cmake and c++ compiler.
Then llvm dependency and other decencies:
* For debian based systems:
  ```bash
  # llvm
  apt install llvm-10-dev libclang-common-10-dev
  
  # clang is used for linking
  apt install clang
  ```
* For arch systems:
  ```bash
  pacman -S llvm10
  pacman -S clang
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

Then you can either install the compiler with `make install` and use `malinc` globally on your system or 
directly use the resulting binary in the `build` folder.

When not installing `malinc` globally you have to call it from the `build` folder with `./malinc`.



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
