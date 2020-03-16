# ub-tester-tool
## Installation
Firstly, you have to make sure that you have correctly installed LLVM. If you haven't, you should visit this [page](https://clang.llvm.org/docs/LibASTMatchersTutorial.html) and follow the instructions there.

After that, you'll need to set an environment variable **LLVM_HOME** which points to correctly installed LLVM directory (usually it's /usr/local/).
And thirdly, you have to clone this repository anywhere you want and build the project:
```bash
git clone https://github.com/KirillBrilliantov/ub-tester-tool.git
cd ub-tester-tool
mkdir build && cd build
cmake ..
make
```
By executing these commands you'll get a file **ub-tester**
