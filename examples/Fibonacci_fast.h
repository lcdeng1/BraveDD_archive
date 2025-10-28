#include <array>

using Matrix = std::array<std::array<long long, 2>, 2>;

Matrix mul(const Matrix& a, const Matrix& b) {
    Matrix c = {{{0, 0}, {0, 0}}};
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 2; ++j)
            for (int k = 0; k < 2; ++k)
                c[i][j] += a[i][k] * b[k][j];
    return c;
}

Matrix pow(Matrix base, long long n) {
    Matrix result = {{{1, 0}, {0, 1}}};  // Identity
    while (n > 0) {
        if (n & 1) result = mul(result, base);
        base = mul(base, base);
        n >>= 1;
    }
    return result;
}

long long fib(long long n) {
    if (n == 0) return 0;
    Matrix F = {{{1, 1}, {1, 0}}};
    Matrix Fn = pow(F, n - 1);
    return Fn[0][0];
}
