int nondet() { int a; return a; }
int nondet_bool() { int a; return a; }
int __cost;
int foo (int A, int B, int C, int D, int E, int F, int G, int H, int I, int J, int K, int L, int M, int N, int O, int P, int Q, int R, int S, int T, int U) {
 goto loc_start;

 loc_f10:
 {
 __cost++;
   if (nondet_bool()) {
    int E_ = 0;
    if (A >= C) {
     E = E_;
     goto loc_f15;
    }
   }
   if (nondet_bool()) {
    if (C >= 1 + A) {
     goto loc_f1;
    }
   }
  goto end;
 }
 loc_f15:
 {
 __cost++;
   if (nondet_bool()) {
    if (D >= A) {
     goto loc_f24;
    }
   }
   if (nondet_bool()) {
    int G_ = nondet();
    int F_ = nondet();
    int I_ = 0;
    int H_ = F_ + G_;
    if (A >= 1 + D) {
     F = F_;
     G = G_;
     H = H_;
     I = I_;
     goto loc_f24;
    }
   }
   if (nondet_bool()) {
    int I_ = nondet();
    int G_ = nondet();
    int F_ = nondet();
    int H_ = F_ + G_;
    int D_ = 1 + D;
    if (A >= 1 + D && 0 >= 1 + I_) {
     D = D_;
     F = F_;
     G = G_;
     H = H_;
     I = I_;
     goto loc_f15;
    }
   }
   if (nondet_bool()) {
    int I_ = nondet();
    int G_ = nondet();
    int F_ = nondet();
    int H_ = F_ + G_;
    int D_ = 1 + D;
    if (A >= 1 + D && I_ >= 1) {
     D = D_;
     F = F_;
     G = G_;
     H = H_;
     I = I_;
     goto loc_f15;
    }
   }
  goto end;
 }
 loc_f2:
 {
 __cost++;
   if (nondet_bool()) {
    int B_ = 1 + B;
    if (A >= B) {
     B = B_;
     goto loc_f2;
    }
   }
   if (nondet_bool()) {
    if (B >= 1 + A) {
     goto loc_f10;
    }
   }
  goto end;
 }
 loc_f24:
 {
 __cost++;
   if (nondet_bool()) {
    int D_ = C;
    if (C == D) {
     D = D_;
     goto loc_f75;
    }
   }
   if (nondet_bool()) {
    int J_ = E;
    int E_ = 1 + E;
    if (C >= 1 + D) {
     E = E_;
     J = J_;
     goto loc_f28;
    }
   }
   if (nondet_bool()) {
    int J_ = E;
    int E_ = 1 + E;
    if (D >= 1 + C) {
     E = E_;
     J = J_;
     goto loc_f28;
    }
   }
  goto end;
 }
 loc_f28:
 {
 __cost++;
   if (nondet_bool()) {
    int L_ = nondet();
    int K_ = nondet();
    if (29 >= J) {
     K = K_;
     L = L_;
     goto loc_f32;
    }
   }
   if (nondet_bool()) {
    int L_ = nondet();
    int K_ = nondet();
    if (J >= 31) {
     K = K_;
     L = L_;
     goto loc_f32;
    }
   }
   if (nondet_bool()) {
    int L_ = nondet();
    int K_ = nondet();
    int J_ = 30;
    if (J == 30) {
     J = J_;
     K = K_;
     L = L_;
     goto loc_f32;
    }
   }
  goto end;
 }
 loc_f32:
 {
 __cost++;
   if (nondet_bool()) {
    int Y_2 = nondet();
    int Y_3 = nondet();
    int Y_0 = nondet();
    int Y_1 = nondet();
    int N_ = nondet();
    int M_ = nondet();
    int Q_ = 0;
    int P_ = 1;
    int O_ = 1;
    int K_ = Y_2 + -Y_3 + Y_0;
    if (K >= 0 && K * Y_0 + N_ * Y_0 + Y_0 >= 1 + Y_1 && Y_1 >= K * Y_0 + N_ * Y_0) {
     K = K_;
     M = M_;
     N = N_;
     O = O_;
     P = P_;
     Q = Q_;
     goto loc_f42;
    }
   }
   if (nondet_bool()) {
    int Y_2 = nondet();
    int Y_3 = nondet();
    int Y_0 = nondet();
    int Y_1 = nondet();
    int R_ = nondet();
    int Q_ = 0;
    int P_ = 1;
    int O_ = 1;
    int N_ = -R_;
    int K_ = Y_2 + -Y_3 + Y_0;
    if (0 >= 1 + K && K * Y_0 + Y_0 >= 1 + R_ * Y_0 + Y_1 && Y_1 + R_ * Y_0 >= K * Y_0) {
     K = K_;
     N = N_;
     O = O_;
     P = P_;
     Q = Q_;
     R = R_;
     goto loc_f42;
    }
   }
  goto end;
 }
 loc_f42:
 {
 __cost++;
   if (nondet_bool()) {
    if (C >= 1 + B) {
     goto loc_f68;
    }
   }
   if (nondet_bool()) {
    int Y_0 = nondet();
    int Y_1 = nondet();
    int T_ = O * Y_1;
    int S_ = P * Y_0;
    int L_ = 0;
    if (B >= C) {
     L = L_;
     S = S_;
     T = T_;
     goto loc_f68;
    }
   }
   if (nondet_bool()) {
    int Y_8 = nondet();
    int Y_9 = nondet();
    int Y_2 = nondet();
    int Y_0 = nondet();
    int Y_1 = nondet();
    int Y_3 = nondet();
    int Y_4 = nondet();
    int Y_5 = nondet();
    int Y_6 = nondet();
    int Y_7 = nondet();
    int Q_ = nondet();
    int P_ = nondet();
    int O_ = nondet();
    int L_ = nondet();
    int T_ = O * Y_9;
    int S_ = P * Y_2;
    int K_ = Y_8 + -O * Y_9;
    if (Q_ >= Y_0 && L_ * Y_0 * Y_1 + Y_0 >= 1 + L_ * P * Y_2 && L_ * P * Y_2 >= L_ * Y_0 * Y_1 && Y_3 >= Q_ && L_ * Y_1 * Y_3 + Y_3 >= 1 + L_ * P * Y_2 && L_ * P * Y_2 >= L_ * Y_1 * Y_3 && O_ >= Y_4 && Y_1 * Y_4 + Y_4 >= 1 + K && K >= Y_1 * Y_4 && Y_5 >= O_ && Y_1 * Y_5 + Y_5 >= 1 + K && K >= Y_1 * Y_5 && P_ >= Y_6 && Y_1 * Y_6 + Y_6 >= 1 + P * Y_2 && P * Y_2 >= Y_1 * Y_6 && Y_7 >= P_ && Y_1 * Y_7 + Y_7 >= 1 + P * Y_2 && P * Y_2 >= Y_1 * Y_7 && B >= C && 0 >= 1 + Y_1 && L_ * Y_1 * Y_8 + Y_8 >= 1 + K * L_ && K * L_ >= L_ * Y_1 * Y_8) {
     K = K_;
     L = L_;
     O = O_;
     P = P_;
     Q = Q_;
     S = S_;
     T = T_;
     goto loc_f60;
    }
   }
   if (nondet_bool()) {
    int Y_8 = nondet();
    int Y_9 = nondet();
    int Y_2 = nondet();
    int Y_0 = nondet();
    int Y_1 = nondet();
    int Y_3 = nondet();
    int Y_4 = nondet();
    int Y_5 = nondet();
    int Y_6 = nondet();
    int Y_7 = nondet();
    int Q_ = nondet();
    int P_ = nondet();
    int O_ = nondet();
    int L_ = nondet();
    int T_ = O * Y_9;
    int S_ = P * Y_2;
    int K_ = Y_8 + -O * Y_9;
    if (Q_ >= Y_0 && L_ * Y_0 * Y_1 + Y_0 >= 1 + L_ * P * Y_2 && L_ * P * Y_2 >= L_ * Y_0 * Y_1 && Y_3 >= Q_ && L_ * Y_1 * Y_3 + Y_3 >= 1 + L_ * P * Y_2 && L_ * P * Y_2 >= L_ * Y_1 * Y_3 && O_ >= Y_4 && Y_1 * Y_4 + Y_4 >= 1 + K && K >= Y_1 * Y_4 && Y_5 >= O_ && Y_1 * Y_5 + Y_5 >= 1 + K && K >= Y_1 * Y_5 && P_ >= Y_6 && Y_1 * Y_6 + Y_6 >= 1 + P * Y_2 && P * Y_2 >= Y_1 * Y_6 && Y_7 >= P_ && Y_1 * Y_7 + Y_7 >= 1 + P * Y_2 && P * Y_2 >= Y_1 * Y_7 && B >= C && Y_1 >= 1 && L_ * Y_1 * Y_8 + Y_8 >= 1 + K * L_ && K * L_ >= L_ * Y_1 * Y_8) {
     K = K_;
     L = L_;
     O = O_;
     P = P_;
     Q = Q_;
     S = S_;
     T = T_;
     goto loc_f60;
    }
   }
  goto end;
 }
 loc_f60:
 {
 __cost++;
   if (nondet_bool()) {
    int S_ = nondet();
    int U_ = 1 + U;
    if (A >= U) {
     S = S_;
     U = U_;
     goto loc_f60;
    }
   }
   if (nondet_bool()) {
    int B_ = -1 + B;
    if (U >= 1 + A) {
     B = B_;
     goto loc_f42;
    }
   }
  goto end;
 }
 loc_f68:
 {
 __cost++;
   if (nondet_bool()) {
    int L_ = 0;
    if (L == 0 && B >= C) {
     L = L_;
     goto loc_f75;
    }
   }
   if (nondet_bool()) {
    if (0 >= 1 + L) {
     goto loc_f75;
    }
   }
   if (nondet_bool()) {
    if (L >= 1) {
     goto loc_f75;
    }
   }
   if (nondet_bool()) {
    int L_ = 0;
    if (L == 0 && C >= 1 + B) {
     L = L_;
     goto loc_f75;
    }
   }
  goto end;
 }
 loc_f75:
 {
 __cost++;
   if (nondet_bool()) {
    if (C >= 1 + D) {
     goto loc_f15;
    }
   }
   if (nondet_bool()) {
    if (D >= 1 + C) {
     goto loc_f15;
    }
   }
   if (nondet_bool()) {
    int D_ = C;
    int C_ = 1 + C;
    if (C == D) {
     C = C_;
     D = D_;
     goto loc_f10;
    }
   }
  goto end;
 }
 loc_start:
 {
 __cost++;
   if (nondet_bool()) {
    if (1 >= 0) {
     goto loc_f2;
    }
   }
  goto end;
 }
 loc_f1:
 end: __VERIFIER_print_hull(__cost);
 return 0;
}
void main() {
  foo(nondet(), nondet(), nondet(), nondet(), nondet(), nondet(), nondet(), nondet(), nondet(), nondet(), nondet(), nondet(), nondet(), nondet(), nondet(), nondet(), nondet(), nondet(), nondet(), nondet(), nondet());
}
