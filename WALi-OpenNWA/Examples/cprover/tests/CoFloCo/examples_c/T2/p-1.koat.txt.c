int nondet() { int a; return a; }
int nondet_bool() { int a; return a; }
int __cost;
int foo (int A, int B, int C, int D, int E, int F, int G, int H) {
 goto loc_f2;

 loc_f1:
 {
 __cost++;
   if (nondet_bool()) {
    int E_ = nondet();
    int D_ = nondet();
    int C_ = nondet();
    if (B >= 1 + A) {
     C = C_;
     D = D_;
     E = E_;
     goto loc_f1;
    }
   }
   if (nondet_bool()) {
    int F_ = nondet();
    int D_ = nondet();
    int C_ = nondet();
    if (A >= B) {
     C = C_;
     D = D_;
     F = F_;
     goto loc_f300;
    }
   }
  goto end;
 }
 loc_f2:
 {
 __cost++;
   if (nondet_bool()) {
    int H_ = nondet();
    int G_ = nondet();
    if (1 >= 0) {
     G = G_;
     H = H_;
     goto loc_f1;
    }
   }
  goto end;
 }
 loc_f300:
 end: __VERIFIER_print_hull(__cost);
 return 0;
}
void main() {
  foo(nondet(), nondet(), nondet(), nondet(), nondet(), nondet(), nondet(), nondet());
}
