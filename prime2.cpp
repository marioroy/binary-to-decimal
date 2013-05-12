/*  prime2.cpp - print number of decimal digits in biggest known prime number
    By Anthony C. Hay, 2013. See http://howtowriteaprogram.blogspot.com/

    This is free and unencumbered public domain software; see http://unlicense.org/
    I believe this code to be correct, but I may be wrong; use at your own risk.

    mul() and div() are modified versions of algorithms published in
    Hacker's Delight: "You are free to use, copy, and distribute
    any of the code on this web site, whether modified by you or not."
    see http://hackersdelight.org/permissions.htm

    Compiled under OS X with g++ 4.2
        g++ -O3 prime2.cpp -o prime2

    Compiled under Windows with VC++ 2010
        cl /nologo /EHs /O2 prime2.cpp /Feprime2.exe
*/

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <stdint.h>


// our arbitrary length unsigned number will be represented by a vector of
// "fragments", each fragment will be one of these
typedef uint32_t num_frag_t;
const int num_frag_t_size = sizeof(num_frag_t) * CHAR_BIT;

// arbitrary length unsigned number; least significant bits in lowest fragment
typedef std::vector<num_frag_t> num_vec_t;



// "normalise" given 'num'; if num is normalised the following will be true
//   1. num.size() > 0
//   2. if num is zero then num.size() == 1 && num[0] == 0
//   3. if num is non-zero then num.back() != 0
void normalise(num_vec_t & num)
{
    num_vec_t::size_type i = num.size();
    if (i == 0) {
        // assume an empty vector represents a zero value
        num.push_back(0);
    }
    else {
        // discard all 0-value number fragments from most significant end
        --i;
        while (i && num[i] == 0)
            --i;
        num.resize(i + 1);
    }
}


// return number of leading 0 bits in given 'x'
int count_leading_zeros(num_frag_t x)
{
    // this may not be the fastest method but has the advantage
    // of working correctly without modification regardless of
    // the number of bits in num_frag_t
    int n = num_frag_t_size;
    while (x) {
        x >>= 1;
        --n;
    }
    return n;
}


// return total number of bits in given 'num'
int num_bits(const num_vec_t & num)
{
    return num.empty()
        ? 0
        : num.size() * num_frag_t_size - count_leading_zeros(num.back());
}


// product <- a * b; a and b must be normalised
void mul(num_vec_t & product, const num_vec_t & a, const num_vec_t & b)
{
    // adapted from Hacker's Delight; originally from Knuth TAoCP V2 4.3.1
    const unsigned a_len = a.size();
    const unsigned b_len = b.size();
    num_vec_t p(a_len + b_len, 0);

    for (unsigned ai = 0; ai < a_len; ++ai) {
        uint64_t k = 0;
        for (unsigned bi = 0; bi < b_len; ++bi) {
            const uint64_t t =
                static_cast<uint64_t>(a[ai]) * static_cast<uint64_t>(b[bi])
                + static_cast<uint64_t>(p[ai + bi]) + k;
            p[ai + bi] = static_cast<num_frag_t>(t);
            k = t >> num_frag_t_size;
        }
        p[ai + b_len] = static_cast<num_frag_t>(k);
    }

    normalise(p);
    product.swap(p);
}

inline num_vec_t & operator*=(num_vec_t & lhs, const num_vec_t & rhs)
{
    mul(lhs, rhs, lhs);
    return lhs;
}


// quotient <- u / v; remainder <- u % v
void div(num_vec_t & quotient, num_vec_t & remainder,
    const num_vec_t & u, const num_vec_t & v)
{
    // adapted from Hacker's Delight; originally from Knuth TAoCP V2 4.3.1
    const uint64_t base = static_cast<uint64_t>(1) << num_frag_t_size;
    const int m = static_cast<int>(u.size());
    const int n = static_cast<int>(v.size());
    if (m < n || n <= 0 || v[n-1] == 0)
        throw std::runtime_error("div() unsupported input");
    num_vec_t q(m - n + 1, 0);
    num_vec_t r(n, 0);

    if (n == 1) {
        // the single digit divisor special case
        uint64_t k = 0;
        for (int j = m - 1; j >= 0; --j) {
            q[j] = static_cast<num_frag_t>((k * base + u[j]) / v[0]);
            k = (k * base + u[j]) - q[j] * v[0];
        }
        r[0] = static_cast<num_frag_t>(k);
        normalise(q); quotient.swap(q);
        normalise(r); remainder.swap(r); 
        return;
    }

    // copy v to vn shifting it to the left so the most significant bit is 1
    const int s = count_leading_zeros(v[n-1]); // 0 <= s <= 31.
    num_vec_t vn(n); // (Hacker's Delight allocates 2n)
    for (int i = n - 1; i > 0; --i)
        vn[i] = (v[i] << s) | (v[i-1] >> (num_frag_t_size - s));
    vn[0] = v[0] << s;

    // copy u to un shifting it to the left by same amount as we did v above
    num_vec_t un(m + 1); // (Hacker's Delight allocates 2(m + 1))
    un[m] = u[m-1] >> (num_frag_t_size - s);
    for (int i = m - 1; i > 0; i--)
        un[i] = (u[i] << s) | (u[i-1] >> (num_frag_t_size - s));
    un[0] = u[0] << s;

    for (int j = m - n; j >= 0; --j) {
        // compute estimate qhat of q[j]
        uint64_t qhat = (static_cast<uint64_t>(un[j+n])*base
            + static_cast<uint64_t>(un[j+n-1])) / vn[n-1];
        uint64_t rhat = (static_cast<uint64_t>(un[j+n])*base
            + static_cast<uint64_t>(un[j+n-1])) - qhat * vn[n-1];
        while (qhat >= base || qhat*vn[n-2] > base*rhat + un[j+n-2]) {
            --qhat;
            rhat += vn[n-1];
            if (rhat >= base)
                break;
        }

        // multiply and subtract
        int64_t k = 0;
        for (int i = 0; i < n; ++i) {
            uint64_t p = qhat * vn[i];
            int64_t t = static_cast<int64_t>(un[i+j]) - k - (p & 0xFFFFFFFF);
            un[i+j] = static_cast<num_frag_t>(t);
            k = (p >> num_frag_t_size) - (t >> num_frag_t_size);
        }
        int64_t t = static_cast<int64_t>(un[j+n]) - k;
        un[j+n] = static_cast<num_frag_t>(t);

        q[j] = static_cast<num_frag_t>(qhat);
        if (t < 0) {
            // we subtracted too much - add it back
            q[j]--;
            k = 0;
            for (int i = 0; i < n; ++i) {
                t = static_cast<int64_t>(un[i+j]) + static_cast<int64_t>(vn[i]) + k;
                un[i+j] = static_cast<num_frag_t>(t);
                k = t >> num_frag_t_size;
            }
            un[j+n] = un[j+n] + static_cast<num_frag_t>(k);
        }
    }

    for (int i = 0; i < n; ++i)
        r[i] = (un[i] >> s) | (un[i+1] << (num_frag_t_size - s));

    normalise(q); quotient.swap(q);
    normalise(r); remainder.swap(r); 
}


// return decimal representation of given 'num' zero padded to 'width'
std::string to_string_fixed_width(
    num_vec_t num, // pass-by-value because algorithm is destructive
    int width) // result will be at least this wide, zero filled if necessary
{
    std::vector<char> result;
    // keep dividing num by 1,000,000,000 until it is 0; the remainder of each
    // division is the next least significant 9 decimal digits in the result
    while (!num.empty()) {
        uint64_t remainder = 0;
        for (num_vec_t::size_type i = num.size(); i--; ) {
            remainder = (remainder << num_frag_t_size) + num[i];
            num[i] = static_cast<num_frag_t>(remainder / 1000000000);
            remainder %= 1000000000;
        }

        // discard all zero-value 32-bit elements from most significant end
        num_vec_t::size_type i = num.size();
        while (i && num[i - 1] == 0)
            --i;
        num.resize(i);

        // extract the next 9 digits from the remainder
        for (int j = 0; j < 9; ++j) {
            result.push_back(static_cast<char>(remainder % 10) + '0');
            remainder /= 10;
            if (remainder == 0 && num.empty())
                break; // don't append superfluous zeros; e.g. 000000012 => 12
        }
    }

    // add any necessary leading zeros to make the result the required width
    width -= static_cast<int>(result.size());
    while (width-- > 0)
        result.push_back('0');

    // the least-significant decimal digit is first so return result reversed
    return std::string(result.rbegin(), result.rend());
}


struct power_of_ten {
    num_vec_t n;    // n = 10^power
    int power;      // which power? e.g. if n = 1000 then power = 3
    int bit_count;  // number of bits in n; e.g. if n = 1000 then bit_count = 10

    power_of_ten(const num_vec_t & n = num_vec_t(), int power = 0)
    : n(n), power(power), bit_count(num_bits(n))
    {
    }
};
typedef std::vector<power_of_ten> pot_vec_t;


// return a list of values, each a power of ten, as required by to_string_helper()
pot_vec_t powers_of_ten(int s)
{
    // - the first value in the list will be 10^1024
    // - each value will be the previous value squared
    // - the last value will be the first power that has at least s bits
    // - if s is less than the number of bits in 10^512 the list will be empty
    pot_vec_t result;
    num_vec_t p;
    p.push_back(10);
    int power(1); // p = 10^power
    while (num_bits(p) < s) {
        p *= p;     // the next value is the previous value squared,
        power *= 2; // which means the power has doubled
        if (power > 1000)
            result.push_back(power_of_ten(p, power));
    }
    return result;
}


// return given 'num' as a decimal string
std::string to_string_helper(
    const num_vec_t & num,
    pot_vec_t::const_reverse_iterator p,
    const pot_vec_t::const_reverse_iterator p_end,
    int width)
{
    if (p == p_end || num_bits(num) <= p->bit_count) {
        // don't need to break number into smaller pieces; it's small
        // enough to use our O(n^2) algorithm directly
        return to_string_fixed_width(num, width);
    }
    else {
        // num is too big to convert directly; break it up first
        std::vector<num_vec_t> parts(num_bits(num) / p->bit_count);
        int count = 0;
        num_vec_t quotient;

        // first repeatedly divide by p->n until quotient is no longer than p->n
        div(quotient, parts[count++], num, p->n);
        while (num_bits(quotient) > p->bit_count)
            div(quotient, parts[count++], quotient, p->n);

        // then do same again for each part, collecting results into one string
        std::string result(to_string_helper(quotient, p + 1, p_end, p->power));
        while (count)
            result += to_string_helper(parts[--count], p + 1, p_end, p->power);

        return result;
    }
}


// return given 'num' as a decimal string
std::string to_string(const num_vec_t & num)
{
    // construct a table of powers of 10 appropriate to the size of the given
    // number and use it convert the number to decimal
    const pot_vec_t powers(powers_of_ten(num_bits(num) / 2));
    std::string result(to_string_helper(num, powers.rbegin(), powers.rend(), 0));

    // remove all leading zeros from the result and return it,
    // or return "0" if the result was nothing but zeros
    result.erase(0, result.find_first_not_of('0'));
    return result.empty() ? std::string("0") : result;
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
        // of num_frag_t_size (also, ones>>num_frag_t_size is undefined in C++)
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
