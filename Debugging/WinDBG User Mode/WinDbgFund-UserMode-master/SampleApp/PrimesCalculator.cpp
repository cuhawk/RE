#include "stdafx.h"
#include "PrimesCalculator.h"
#include <math.h>

std::vector<int> PrimesCalculator::FindPrimes(int first, int last) {
    std::vector<int> primes;

    for (int i = first; i <= last; i++)
        if (IsPrime(i))
            primes.push_back(i);

    return primes;
}

bool PrimesCalculator::IsPrime(int n) {
    int limit = (int)sqrt(n);
    for (int i = 2; i <= limit; i++)
        if (n % i == 0)
            return false;

    return true;
}
