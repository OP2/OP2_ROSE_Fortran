void laplace(float A[2][2], const float x[2]) {
  const float hinv = 1./(x[1] - x[0]);
  A[0][0] = hinv;
  A[0][1] = -hinv;
  A[1][0] = -hinv;
  A[1][1] = hinv;
}

