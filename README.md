# Malin Language
This is a compiler for the experimental `malin` programming language.

### The Language
Malin is inspired by c++ and rust and compiles to bytecode via llvm. It has no garbage collection.
Here are some examples: 
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
The files directly in the `example` folder are working with the current compiler.


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
- [x] math expressions ( :large_orange_diamond: todo: i32/i32 = f32)                      
- [x] variables (mutable, always copy)          
- [x] buildin types (i32, f32)                  
- [x] control flow: if-else                     
- [x] control flow: while loop                  
- [ ] static strings like "abc"                 
- [ ] classes (no inheritance)                  
- [ ] move, copy behavior                       
- [ ] heap functions as unsafe                  
- [ ] smart pointers for heap (shard, unique)   