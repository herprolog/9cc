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

assert 0  "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 41 " 12 + 34 - 5 ;"
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'
assert 10 '-10+20;'
assert 10 '- -10;'
assert 10 '- - +10;'

assert 0 '0==1;'
assert 1 '42==42;'
assert 1 '0!=1;'
assert 0 '42!=42;'

assert 1 '0<1;'
assert 0 '1<1;'
assert 0 '2<1;'
assert 1 '0<=1;'
assert 1 '1<=1;'
assert 0 '2<=1;'

assert 1 '1>0;'
assert 0 '1>1;'
assert 0 '1>2;'
assert 1 '1>=0;'
assert 1 '1>=1;'
assert 0 '1>=2;'

assert 1 'return 1;'
assert 0 'return 0;return1;'
assert 1 'a=1;return a;'
assert 0 'tmp=0;return tmp;'
assert 5 'A=2+3;return A;'

assert 1 'if(1)return 1;return 0;'
assert 2 'if(0)return 1;return 2;'
assert 1 'if(0)return 0;else return 1;return 2;'
assert 2 'a=0;if(1)if(1)a=2;return a;'
assert 0 'a=0;if(1)if(0)a=2;return a;'
assert 1 'a=0;if(0)a=1;else if(1)a=1;else a=2;return a;'
assert 0 'if(2<3*4)return 0;return 1;'
assert 1 'if(5<100/1002)return 0;return 1;'

assert 5 'i=0;while(i<5)i=i+1;return i;'
assert 0 'i=0;while(i<0)i=i+1;return i;'

assert 5 'a=0;for(i=0;i<5;i=i+1)a=a+1;return a;'

assert 0 '{}return 0;'
assert 2 '1;return 2;'
assert 0 'a=5;{a=a-6;a=a+1;}return a;'
assert 1 'a=0;if(2<5){a=1;}return a;'
assert 2 'a=0;for(i=0;i<2;i=i+1){a=a+1;}return a;'
assert 0 'a=10;while(a){a=a-1;}return a;'
assert 0 'a=-10;for(i=0;i<20;i=i+1){a=a+1;if(a>=0){return a;}}return a;'

echo OK
