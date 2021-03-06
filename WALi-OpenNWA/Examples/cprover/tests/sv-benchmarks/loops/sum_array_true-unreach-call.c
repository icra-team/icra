#define LIMIT 1000000

int main()
{
  unsigned int M = __VERIFIER_nondet_uint();
  int A[M], B[M], C[M];
  unsigned int  i;
  
  for(i=0;i<M;i++) {
    A[i] = __VERIFIER_nondet_int();
    __VERIFIER_assume(A[i] <= LIMIT);

  }
  
  for(i=0;i<M;i++) {
    B[i] = __VERIFIER_nondet_int();
    __VERIFIER_assume(B[i] <= LIMIT);
  }

  for(i=0;i<M;i++)
     C[i]=A[i]+B[i];
  
  for(i=0;i<M;i++)
     __VERIFIER_assert(C[i]==A[i]+B[i]);
}

