rm -rf ./build
mkdir ./build
for file in *.c; do
  without_extension=${file%.*}
  original=${file%_*}
  gcc -lcmocka $file ../$original.c -o ./build/$without_extension
  echo "Compiled: $file to ./build/$without_extension linked with ../$original.c"
done

$failed=0
for t in ./build/*; do
  echo "********************************"
  echo " running $t"
  echo "********************************"
  $t
  if [ "$?" -ne 0 ]; then
    failed=1
  fi
done

if [ "$failed" -eq 1 ]; then
  echo "TESTS FAILED"
fi

if [ "$1" == "integration" ]; then
  echo "********************************"
  echo " RUNNING INTEGRATION TESTS"
  echo "********************************"
  cd ./integration && ./run_test.pl
fi
