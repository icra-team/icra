int nondet() { int a; return a; }
int nondet_bool() { int a; return a; }
int __cost;
int foo (int A, int B) {
 goto loc_f3;

 loc_f0:
 {
 __cost++;
   if (nondet_bool()) {
    int A_ = 1 + A;
    if (1 >= 0) {
     A = A_;
     goto loc_f0;
    }
   }
  goto end;
 }
 loc_f3:
 {
 __cost++;
   if (nondet_bool()) {
    int B_ = 1;
    int A_ = 1;
    if (1 >= 0) {
     A = A_;
     B = B_;
     goto loc_f0;
    }
   }
  goto end;
 }
 end: __VERIFIER_print_hull(__cost);
 return 0;
}
void main() {
  foo(nondet(), nondet());
}
