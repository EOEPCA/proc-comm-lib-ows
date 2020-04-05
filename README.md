# proc-comm-lib-ows
proc-comm-lib-ows

build example

```ssh

git clone https://github.com/EOEPCA/proc-comm-lib-ows.git
cd proc-comm-lib-ows
git checkout develop

export LOCAL_DOCKERIMAGE='eoepca/eoepca-build-cpp:1.0'
export CMAKERELEASE='Release'
docker run --rm -ti  -v $PWD:/project/ -w /project/build/  ${LOCAL_DOCKERIMAGE} cmake -DCMAKE_BUILD_TYPE=${CMAKERELEASE} -G "CodeBlocks - Unix Makefiles" ..

docker run --rm -ti  -v $PWD:/project/ -w /project/build/  ${LOCAL_DOCKERIMAGE} make

cd build

./runner
```

expected output:

```txt
LIB version: 1
LIB name: EOEPCA OWS Parser
```

Run tests

```shell script
./build/tests/libtest-test --gtest_break_on_failure
```
