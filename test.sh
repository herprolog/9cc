#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 'main(){return 0;}'
assert 3 'main(){a=1;b=2;return a+b;}'
assert 1 'main(){if(1)return 1;else return 2;}'
assert 2 'main(){if(0)return 1;else return 2;}'
assert 5 'main(){i=0;while(i<5)i=i+1;return i;}'
assert 5 'main(){a=0;for(i=0;i<5;i=i+1){a=a+1;}return a;}'
assert 3 'add(a,b){return a+b;}main(){add(1,2);}'

echo OK
