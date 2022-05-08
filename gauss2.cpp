#include <vector>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <chrono>
#include <bitset>
#include <atomic>
#include <thread>
#include <mutex>

using myInt = int;
using PrimeList = std::vector<myInt>;

const myInt modValue = 1'000'000'007;
const myInt GBigValue = 100'000'000;
const long G10Correct = 23'044'331'520'000;
const myInt MaxPrimeIndex = 6'000'000;

PrimeList primes = {2, 3, 5, 7, 11};
std::atomic_int maxPrime;

myInt primeArray[MaxPrimeIndex];

inline myInt ModMul(myInt val1, myInt val2)
{
    long v1 = val1;
    long v2 = val2;
    long x = v1 * v2;
    if (x > modValue)
        x = x % modValue;
    return x;
}

myInt ModMulPower(myInt val, myInt power)
{
    if (power == 0)
        return 1;
    if (power == 1)
        return val;
    if (power == 2)
        return ModMul(val, val);

    myInt dval = ModMul(val, val);
    if (power % 2 == 1)
        return ModMul(ModMulPower(dval, power / 2), val);
    else
        return ModMulPower(dval, power / 2);
}

inline bool HasFactorsForList(const myInt test, PrimeList &factors)
{
    for (auto &&f : factors)
    {
        if (f > test)
            break;
        if (test % f == 0)
            return true;
    }
    return false;
}

inline myInt LastPrime()
{
    return primeArray[maxPrime - 1];
}

inline void PushBackPrime(myInt p)
{
    assert(maxPrime < MaxPrimeIndex);
    primeArray[maxPrime] = p;
    ++maxPrime; //note must set value before incrementing atomic
}

inline bool HasFactors(const myInt test, int size)
{
    myInt sizecheck = test / 2 + 1;

    for (int i = 0; i < size; ++i)
    {
        myInt f = primeArray[i];
        if (test % f == 0)
            return true;
        if (f > sizecheck)
            break;
    }
    return false;
}

inline myInt GetPrime(int index)
{
    long current = maxPrime;
    while (index >= current)
    {
        std::cout << "waiting " << index << " " << current << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(index - current + 10000));
        current = maxPrime;
    }
    return primeArray[index];
}

inline void InitPrimes(myInt LargestPrime)
{

    primeArray[0] = 2;
    maxPrime = 1;
    myInt test = 3;

    while (test < LargestPrime)
    {
        int val = maxPrime;
        for (;;)
        {
            if (!HasFactors(test, val))
            {
                PushBackPrime(test);
                test = test + 2;
                break;
            }
            else
            {
                test = test + 2;
            }
        }
    }
    std::cout << "finished primes\n";
}

void GetPrimeListForValue(myInt test, PrimeList &pList)
{
    myInt index = 0;
    while (test > 1)
    {
        myInt nextp = GetPrime(index);
        while (test % nextp == 0)
        {
            test = test / nextp;
            pList.push_back(nextp); //note can add same value several times!
        }
        ++index;
    }
}

myInt g(myInt val)
{
    PrimeList valFactors;
    myInt prod = 1;
    GetPrimeListForValue(val, valFactors);
    for (myInt i = 2; i < val; ++i)
    {
        if (!HasFactorsForList(i, valFactors))
        {
            prod = ModMul(prod, i);
        }
    }
    return prod;
}

myInt BigFactorForN(myInt G, myInt val, PrimeList &valFactors);

myInt BigFactorForPrime(myInt G, myInt val)
{
    PrimeList valFactors;
    valFactors.push_back(val);
    return BigFactorForN(G, val, valFactors);
}

std::bitset<GBigValue + 1> doubleCount;

myInt BigFactorForN(myInt G, myInt val, PrimeList &valFactors)
{
    myInt res = 0;
    myInt GoverVal = G / val;
    myInt GmodVal = G % val;
    bool isPrime = valFactors.size() == 1;
    assert(val > 1);

    auto CalcNumberPerVal = [&val, &valFactors](long &numberPerVal)
    {
        myInt last = 0;
        const int initVal = numberPerVal;
        if (initVal < 400'000)
        {
            for (int i = 1; i <= initVal; i++)
            {
                doubleCount.reset(i);
            }
        }
        else
        {
            doubleCount.reset();
        }
        numberPerVal = 0;

        for (int i = 0; i < valFactors.size(); ++i)
        {
            myInt p = valFactors[i];
            if (last != p)
            {
                last = p;

                for (; p <= initVal; p += last)
                {
                    if (!doubleCount[p])
                    {
                        numberPerVal++;
                        doubleCount.set(p, true);
                    }
                }
            }
        }
        assert(numberPerVal >= 0);
    };

    if (GoverVal > 1)
    {
        long numberPerVal;
        if (!isPrime)
        {
            numberPerVal = val;
            CalcNumberPerVal(numberPerVal);
        }
        else
        {
            numberPerVal = val - 1;
        }
        assert(numberPerVal > 0);
        res += (GoverVal - 1) * numberPerVal;
    }
    if (isPrime)
    {
        res += GmodVal;
    }
    else
    {
        if (GmodVal > 0)
        {
            long count = GmodVal;
            CalcNumberPerVal(count);
            res += count;
        }
    }
    return res;
}

PrimeList valFactors1;
myInt AddValForG(myInt G, myInt val)
{
    valFactors1.clear();
    GetPrimeListForValue(val, valFactors1);
    return BigFactorForN(G, val, valFactors1);
}

struct Tasks
{
    std::mutex mtx;
    struct SubTask
    {
        int start;
        int finish;
        std::atomic_int remaining;
    };

    std::vector<SubTask> taskArray;
    int currentChunk;

    void initTaskArray(int numTasks, int G)
    {
        taskArray.clear();

        int chunk = G / numTasks;
        int st = 2;
        int fin = st + chunk - 1;
        for (int i = 0; i < numTasks; ++i)
        {
            auto &thisTask = taskArray[i];
            thisTask.start = st;
            thisTask.finish = fin;
            thisTask.remaining = 0;
            st = fin + 1;
            fin = st + chunk - 1;
        }
        taskArray[numTasks - 1].finish = G - 1;
        currentChunk = -1;
    }
    void OutputStatus()
    {
        std::lock_guard<std::mutex> lk(mtx);
        int done = 0;
        for (int i = 0; i < taskArray.size(); ++i)
        {
            if (i <= currentChunk)
            {
                if (taskArray[i].remaining > 0)
                {
                    std::cout << "Chunk " << i << " in progress " << taskArray[i].remaining << "\n";
                    done += taskArray[i].remaining;
                }
                else
                {
                    done = taskArray[i].finish - taskArray[i].start + 1;
                    std::cout << "Chunk " << i << " done\n";
                }
            }
            else
            {
                std::cout << "Chunk " << i << " not started\n";
            }

            taskArray[i].
        }
    }

    SubTask *GetNextChunk()
    {
        std::lock_guard<std::mutex> lk(mtx);
        currentChunk++;
        if (currentChunk >= taskArray.size())
        {
            return nullptr;
        }
        else
        {
            auto nextTaskP = &taskArray[currentChunk];
            nextTaskP->remaining = nextTaskP->finish - nextTaskP->start + 1;
            return nextTaskP;
        }
    }
};

std::atomic_int remaining1;
std::atomic_int remaining2;

auto startTime = std::chrono::steady_clock::now();

myInt CalcGFromScratch(myInt G, myInt start, myInt stop, std::atomic_int *remaining, myInt *g_i)
{

    for (myInt count = start; count <= stop; ++count)
    {
        if (count % 50 == 0)
            *remaining = stop - count;
        myInt addG = AddValForG(G, count);
        (*g_i) = ModMul(*g_i, ModMulPower(count, addG));
    }
    *remaining = 0;
    return *g_i;
}

bool PrintProgress()
{
    bool moreToGo = false;
    bool primesStillGoing = false;
    if (LastPrime() < GBigValue)
    {
        primesStillGoing = true;
        std::cout << "P " << LastPrime() << " " << maxPrime << "\n";
        moreToGo = true;
    }

    auto CheckThread = [&moreToGo](int id, int remaining)
    {
        if (remaining > 0)
        {
            moreToGo = true;
            std::cout << "Thread " << id << " has " << remaining << " to go"
                      << "\n";
        }
    };
    CheckThread(1, remaining1);
    CheckThread(2, remaining2);

    auto endtime = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = endtime - startTime;
    auto seconds = elapsed.count();
    std::cout << "Seconds taken " << seconds << "\n";
    int rem = 0;
    rem += remaining1;

    if (!primesStillGoing)
    {
        rem += remaining2;
    }
    else
    {
        rem += GBigValue / 2;
    }

    std::cout << "Seconds to go " << seconds * rem / (GBigValue - rem) << "\n";
    return moreToGo;
}

void LoopProgress()
{
    do
    {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(30'000ms);
    } while (PrintProgress());
}

int main()
{
    myInt bigG_i_t1 = 1;
    myInt bigG_i_t2 = 1;
    myInt halfWay = GBigValue / 2;
    remaining1 = 0;
    remaining2 = 0;

    valFactors1.reserve(200);
    std::thread progress(LoopProgress);
    std::thread primes(InitPrimes, GBigValue);
    std::thread calcG1(CalcGFromScratch, GBigValue, 2, halfWay, &remaining1, &bigG_i_t1);
    primes.join();
    std::cout << " primes finished\n";
    std::thread calcG2(CalcGFromScratch, GBigValue, halfWay + 1, GBigValue - 1, &remaining2, &bigG_i_t2);
    calcG1.join();
    calcG2.join();
    std::cout << "bigG_i_t1 " << bigG_i_t1 << "\n";
    std::cout << "bigG_i_t12 " << bigG_i_t2 << "\n";
    std::cout << "G(" << GBigValue << ") " << ModMul(bigG_i_t1, bigG_i_t2) << "\n";

    progress.join();
}
