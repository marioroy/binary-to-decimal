/*  prime0.cpp - print number of decimal digits in biggest known prime number
    By Anthony C. Hay, 2013. See http://howtowriteaprogram.blogspot.com/

    This is free and unencumbered public domain software; see http://unlicense.org/
    I believe this code to be correct, but I may be wrong; use at your own risk.

    Compiled under OS X with g++ 4.2
        g++ -O3 prime0.cpp -o prime0

    Compiled under Windows with VC++ 2010
        cl /nologo /EHs /O2 prime0.cpp /Feprime0.exe
*/

#include <iostream>
#include <string>
#include <vector>
#include <climits>
#include <stdint.h>


// our arbitrary length unsigned number will be represented by a vector of
// "fragments", each fragment will be one of these
typedef uint32_t num_frag_t;
const int num_frag_t_size = sizeof(num_frag_t) * CHAR_BIT;

// arbitrary length unsigned number; least significant bits in lowest fragment
typedef std::vector<num_frag_t> num_vec_t;



// return given 'num' as a decimal string
std::string to_string(num_vec_t num) // pass-by-value because algorithm is destructive
{
    std::vector<char> result;
    // keep dividing num by 10 until it is 0; the remainder of each
    // division is the next least significant decimal digit in the result
    while (!num.empty()) {
        uint64_t remainder = 0;
        for (num_vec_t::size_type i = num.size(); i--; ) {
            remainder = (remainder << num_frag_t_size) + num[i];
            num[i] = static_cast<num_frag_t>(remainder / 10);
            remainder %= 10;
        }
        // (C++ standard guarantees '0'..'9' are consecutive)
        result.push_back(static_cast<char>(remainder) + '0');

        // discard all 0-value number fragments from most significant end
        num_vec_t::size_type i = num.size();
        while (i && num[i - 1] == 0)
            --i;
        num.resize(i);
    }

    // the least-significant decimal digit is first so return result reversed
    return std::string(result.rbegin(), result.rend());
}


// set given 'p' to the value of (2^n)-1 for given 'n'; requires n > 0
void make_prime(int n, num_vec_t & p)
{
    // a binary representation of (2^n)-1 is just an n-bit number with
    // every bit set; e.g. if n=4, (2^n)-1=15 decimal, or 1111 binary
    const int len = (n + num_frag_t_size - 1) / num_frag_t_size;
    const num_frag_t ones = ~static_cast<num_frag_t>(0);
    p = num_vec_t(len, ones);
    if (n % num_frag_t_size) {
        // the bits in p[len - 1] are already correct if n is a whole multiple
        // of num_frag_t_size (and anyway, ones>>num_frag_t_size is undefined in C++)
        p[len - 1] = ones >> (num_frag_t_size - n % num_frag_t_size);
    }
}


// return a decimal string representation of (2^n)-1 for the given 'n'
std::string prime_str(int n)
{
    num_vec_t p;
    make_prime(n, p);
    return to_string(p);
}


#ifndef PRIME_UNDER_TEST

int main()
{
    // output the number of decimal digits in (2^57,885,161)-1, i.e. 17,425,170
    std::cout << prime_str(57885161).size() << '\n';
}

#endif
