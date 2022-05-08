#include <iostream>
#include <vector>
#include <algorithm>
//#include <thread>
//#include <mutex>

/*
g(10) = 1.3.7.9

P(10) = 1.2.3.4.5.6.7.8.9

P(10) = 1.2.3.2^2.5.2.3.7.2^3.9
*/
using myInt = unsigned long long;
using myVec = std::vector<myInt>;

const myInt modValue = 1'000'000'007;
const myInt GBigValue = 100'000'000;
const myInt G10Correct = 23'044'331'520'000;

std::ostream &operator<<(std::ostream &os, const myVec &vec)
{
    int i = 0;
    os << " vector size " << vec.size() << " ";
    for (auto &&next : vec)
    {
        os << " " << next;
        if (++i % 10 == 0)
        {
            os << "\n";
        }
    }
    return os;
}

inline myInt ModMul(myInt val1, myInt val2)
{
    //return val1 * val2;
    return (val1 * val2) ;
}

inline myInt ModDiv(myInt val1, myInt val2)
{
    if (val1 < val2)
        val1 += modValue;
    myInt res = val1 / val2;
    res = res % modValue;
    return res;
}

inline myInt ModMod(myInt val1, myInt val2)
{
    if (val1 < val2)
        val1 += modValue;
    return val1 % val2;
}

inline bool HasFactors(const myInt test, myVec &factors)
{
    for (auto &&f : factors)
    {
        if ( ModMod( test, f ) == 0 ) 
            return true;
        if ( ModDiv( test, f ) < 2 ) 
            break; 
    }
    return false; 
}

myVec primes = {2, 3, 5, 7, 11};
//std::mutex g_1_mut;

inline myInt NextPrime(myInt index)
{

    while (index >= primes.size())
    {
        myInt test = primes.back() + 2;
        for (;;)
        {
            if (!HasFactors(test, primes))
            {
                {
                    //const std::lock_guard<std::mutex> lock(g_1_mut);
                    primes.push_back(test);
                }
                if (primes.size() % 1000 == 0)
                {
                    std::cout << "P " << primes.back() << " is " << primes.size() << "\n";
                }

                break;
            }
            else
            {
                test = test + 2;
            }
        }
    }
    return primes[index];
}

void GetPrimeFactors(myInt test, myVec &factors)
{
    myInt index = 0;
    factors.clear();
    while (test > 1)
    {
        myInt nextp = NextPrime(index);
        while (test % nextp == 0)
        {
            test = test / nextp;
            if ((factors.size() == 0) || (factors.back() != nextp))
            {
                factors.push_back(nextp);
            }
        }
        ++index;
    }
}

myInt g(myInt val)
{
    myVec valFactors;
    myInt prod = 1;
    GetPrimeFactors(val, valFactors);
    for (myInt i = 2; i < val; ++i)
    {
        if (!HasFactors(i, valFactors))
        {
            prod = (prod * i) % modValue;
        }
    }
    return prod;
}

myInt Nextg(myInt val, myInt &PrevProd)
{
    myInt g = PrevProd;
    myVec valFactors;
    GetPrimeFactors(val, valFactors);

    for (auto &&prime : valFactors)
    {
        for (myInt i = 1; i * prime < val; ++i)
        {
            myInt relNotPrime = ModMul(i, prime);
            if (ModMod(g, relNotPrime) == 0)
            {
                g = ModDiv(g, relNotPrime);
            }
        }
    }
    PrevProd = ModMul(PrevProd, val);
    return g;
}

myInt Prod(myInt val)
{
    myInt ProdVal = 1;
    for (myInt i = 1; i <= val; ++i)
    {
        ProdVal = ModMul(ProdVal, i);
    }
    return ProdVal;
}

myInt Fastg(myInt val)
{
    if (val < 5)
    {
        return g(val);
    }
    myInt prod = Prod(val - 1);
    return Nextg(val, prod);
}

void BigGNext(myInt val, myInt &PrevProd, myInt &PrevG)
{
    myInt g = Nextg(val, PrevProd);
    PrevG = ModMul(PrevG, g);
}

myInt BigGStart(myInt val)
{
    myInt prod = 1;
    for (myInt i = 2; i <= val; ++i)
    {
        myInt gi = g(i);
        prod = ModMul(prod, gi);
        if (i % 10'000 == 0)
        {
            std::cout << "G(" << i << ") =" << prod << "\n";
        }
    }
    return prod;
}

myInt BigG(myInt val)
{
    if (val < 5)
    {
        return BigGStart(val);
    }
    myInt PrevProd = 1 * 2 * 3;
    myInt PrevG = BigGStart(3);
    for (myInt i = 4; i <= val; ++i)
    {

        BigGNext(i, PrevProd, PrevG);
        //if (i % 10'000 == 0)
        {
            std::cout << "G(" << i << ")" << PrevG << "\n";
        }
    }
    return PrevG;
}

int main()
{
    std::cout << "g(10) is " << g(10) << "\n";
    std::cout << "gFast(10) is " << Fastg(10) << "\n";
    primes.reserve(1'000'000);
    {
        myVec modFactors;
        GetPrimeFactors(GBigValue, modFactors);
        std::cout << "factors of 10 to the 8 are " << modFactors << "\n";
    }

    //std::thread t1(NextPrime, (myInt)1'000'000);

    std::cout << "g(10) is " << g(10) << "\n";
    std::cout << "BigG(10)  is " << BigG(10) << " correct is " << ModMul(G10Correct, 1) << "\n";

    //myInt BigGX = BigG(GBigValue);
    //std::cout << "G(10^8) is " << BigGX << "\n";
    //t1.join();
}