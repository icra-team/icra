int nondet() { int a; return a; }
int nondet_bool() { int a; return a; }
int __cost;
int foo (int A, int B, int C, int D, int E) {
 goto loc_evalfstart;

 loc_evalfbb10in:
 {
 __cost++;
   if (nondet_bool()) {
    int C_ = 1;
    if (B >= A) {
     C = C_;
     goto loc_evalfbb8in;
    }
   }
   if (nondet_bool()) {
    if (A >= 1 + B) {
     goto loc_evalfreturnin;
    }
   }
  goto end;
 }
 loc_evalfbb3in:
 {
 __cost++;
   if (nondet_bool()) {
    int E_ = 1 + E;
    if (1 >= 0) {
     E = E_;
     goto loc_evalfbb4in;
    }
   }
  goto end;
 }
 loc_evalfbb4in:
 {
 __cost++;
   if (nondet_bool()) {
    if (D >= E) {
     goto loc_evalfbb3in;
    }
   }
   if (nondet_bool()) {
    if (E >= 1 + D) {
     goto loc_evalfbb5in;
    }
   }
  goto end;
 }
 loc_evalfbb5in:
 {
 __cost++;
   if (nondet_bool()) {
    int D_ = 1 + D;
    if (1 >= 0) {
     D = D_;
     goto loc_evalfbb6in;
    }
   }
  goto end;
 }
 loc_evalfbb6in:
 {
 __cost++;
   if (nondet_bool()) {
    int E_ = 1;
    if (B >= D) {
     E = E_;
     goto loc_evalfbb4in;
    }
   }
   if (nondet_bool()) {
    if (D >= 1 + B) {
     goto loc_evalfbb7in;
    }
   }
  goto end;
 }
 loc_evalfbb7in:
 {
 __cost++;
   if (nondet_bool()) {
    int C_ = 1 + C;
    if (1 >= 0) {
     C = C_;
     goto loc_evalfbb8in;
    }
   }
  goto end;
 }
 loc_evalfbb8in:
 {
 __cost++;
   if (nondet_bool()) {
    int D_ = 1 + A;
    if (A >= C) {
     D = D_;
     goto loc_evalfbb6in;
    }
   }
   if (nondet_bool()) {
    int A_ = 1 + A;
    if (C >= 1 + A) {
     A = A_;
     goto loc_evalfbb10in;
    }
   }
  goto end;
 }
 loc_evalfentryin:
 {
 __cost++;
   if (nondet_bool()) {
    int A_ = 1;
    if (1 >= 0) {
     A = A_;
     goto loc_evalfbb10in;
    }
   }
  goto end;
 }
 loc_evalfreturnin:
 {
 __cost++;
   if (nondet_bool()) {
    if (1 >= 0) {
     goto loc_evalfstop;
    }
   }
  goto end;
 }
 loc_evalfstart:
 {
 __cost++;
   if (nondet_bool()) {
    if (1 >= 0) {
     goto loc_evalfentryin;
    }
   }
  goto end;
 }
 loc_evalfstop:
 end: __VERIFIER_print_hull(__cost);
 return 0;
}
void main() {
  foo(nondet(), nondet(), nondet(), nondet(), nondet());
}
