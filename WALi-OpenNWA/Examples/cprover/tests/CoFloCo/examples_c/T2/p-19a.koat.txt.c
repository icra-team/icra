int nondet() { int a; return a; }
int nondet_bool() { int a; return a; }
int __cost;
int foo (int A, int B, int C, int D, int E, int F, int G, int H, int I, int J) {
 goto loc_f1;

 loc_f1:
 {
 __cost++;
   if (nondet_bool()) {
    int J_ = nondet();
    int I_ = nondet();
    int H_ = nondet();
    if (1 >= 0) {
     H = H_;
     I = I_;
     J = J_;
     goto loc_f2;
    }
   }
  goto end;
 }
 loc_f2:
 {
 __cost++;
   if (nondet_bool()) {
    int F_ = nondet();
    int E_ = nondet();
    int D_ = nondet();
    int C_ = nondet();
    int B_ = nondet();
    if (A >= 1) {
     B = B_;
     C = C_;
     D = D_;
     E = E_;
     F = F_;
     goto loc_f2;
    }
   }
   if (nondet_bool()) {
    int G_ = nondet();
    int C_ = nondet();
    int B_ = nondet();
    if (0 >= A) {
     B = B_;
     C = C_;
     G = G_;
     goto loc_f300;
    }
   }
  goto end;
 }
 loc_f300:
 end: __VERIFIER_print_hull(__cost);
 return 0;
}
void main() {
  foo(nondet(), nondet(), nondet(), nondet(), nondet(), nondet(), nondet(), nondet(), nondet(), nondet());
}
