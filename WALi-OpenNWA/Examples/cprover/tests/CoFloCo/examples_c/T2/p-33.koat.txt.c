int nondet() { int a; return a; }
int nondet_bool() { int a; return a; }
int __cost;
int foo (int A, int B) {
 goto loc_f3;

 loc_f2:
 {
 __cost++;
   if (nondet_bool()) {
    int Y_1 = nondet();
    int Y_0 = nondet();
    int A_ = -1 + Y_1;
    if (0 >= 1 + A && 0 >= Y_0) {
     A = A_;
     goto loc_f2;
    }
   }
   if (nondet_bool()) {
    int Y_1 = nondet();
    int Y_0 = nondet();
    int A_ = 1 + Y_1;
    if (0 >= 1 + A && Y_0 >= 2) {
     A = A_;
     goto loc_f2;
    }
   }
   if (nondet_bool()) {
    int Y_1 = nondet();
    int Y_0 = nondet();
    int A_ = -1 + Y_1;
    if (A >= 1 && 0 >= 2 + Y_0) {
     A = A_;
     goto loc_f2;
    }
   }
   if (nondet_bool()) {
    int Y_1 = nondet();
    int Y_0 = nondet();
    int A_ = 1 + Y_1;
    if (A >= 1 && Y_0 >= 0) {
     A = A_;
     goto loc_f2;
    }
   }
   if (nondet_bool()) {
    int B_ = nondet();
    int A_ = 0;
    if (0 >= 1 + A) {
     A = A_;
     B = B_;
     goto loc_f300;
    }
   }
   if (nondet_bool()) {
    int B_ = nondet();
    int A_ = 0;
    if (A >= 1) {
     A = A_;
     B = B_;
     goto loc_f300;
    }
   }
   if (nondet_bool()) {
    int B_ = nondet();
    if (A == 0) {
     B = B_;
     goto loc_f300;
    }
   }
  goto end;
 }
 loc_f3:
 {
 __cost++;
   if (nondet_bool()) {
    if (1 >= 0) {
     goto loc_f2;
    }
   }
  goto end;
 }
 loc_f300:
 end: __VERIFIER_print_hull(__cost);
 return 0;
}
void main() {
  foo(nondet(), nondet());
}
