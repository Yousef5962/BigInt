#include <iostream>
#include <string>
#include <cstdint>
#include <algorithm>
#include <vector>

using namespace std;

class BigInt
{
    string number;   // Stores the number as a string
    bool isNegative; // True if number is negative

    // Remove unnecessary leading zeros from the number string
    void removeLeadingZeros()
    {
        size_t i = 0;
        size_t len = number.length();

        while (i < len && number[i] == '0')
        {
            ++i;
        }

        if (i == len)
        {
            number = "0";
            isNegative = false;
            return;
        }

        number.erase(0, i);
    }

    // Compare absolute values of two BigInts (ignore signs)
    // Returns: 1 if |this| > |other|, 0 if equal, -1 if |this| < |other|
    int compareMagnitude(const BigInt &other) const
    {
        if (number.length() > other.number.length())
            return 1;
        if (number.length() < other.number.length())
            return -1;
        if (number > other.number)
            return 1;
        if (number < other.number)
            return -1;
        return 0;
    }

public:
    // Default constructor - initialize to zero
    BigInt()
    {
        number = "0";
        isNegative = false;
    }

    // Constructor from 64-bit integer
    BigInt(int64_t value)
    {
        if (value == 0)
        {
            number = "0";
            isNegative = false;
            return;
        }
        if (value < 0)
        {
            isNegative = true;
            if (value == INT64_MIN)
            {
                // Handle special case to avoid overflow
                number = "9223372036854775808";
                return;
            }
            value = -value;
        }
        else
            isNegative = false;

        number = "";
        int64_t temp = value;
        while (temp > 0)
        {
            number.insert(number.begin(), (temp % 10) + '0');
            temp /= 10;
        }
    }

    // Constructor from string representation
    BigInt(const string &str)
    {
        if (str.empty() || str == "0" || str == "+0" || str == "-0")
        {
            number = "0";
            isNegative = false;
            return;
        }

        size_t start = 0;
        if (str[0] == '-')
        {
            isNegative = true;
            start = 1;
        }
        else if (str[0] == '+')
        {
            isNegative = false;
            start = 1;
        }
        else
        {
            isNegative = false;
        }

        number = str.substr(start);

        // Validate that the string contains only digits
        for (char c : number)
        {
            if (!isdigit(c))
            {
                throw invalid_argument("Invalid character in BigInt string");
            }
        }

        removeLeadingZeros();

        if (number == "0")
            isNegative = false;
    }

    // Copy constructor
    BigInt(const BigInt &other)
    {
        number = other.number;
        isNegative = other.isNegative;
    }

    // Destructor
    ~BigInt()
    {
        // TODO: Implement if needed
        // NO pointer to delete for now
        // cout << "Destructor called for " << toString() << endl;
    }

    // Assignment operator
    BigInt &operator=(const BigInt &other)
    {
        if (this != &other)
        {
            number = other.number;
            isNegative = other.isNegative;
        }
        return *this;
    }

    // Unary negation operator (-x)
    BigInt operator-() const
    {
        BigInt result = *this;
        if (result.number != "0")
        {
            result.isNegative = !isNegative;
        }
        return result;
    }

    // Unary plus operator (+x)
    BigInt operator+() const
    {
        return *this;
    }

    // Addition assignment operator (x += y)
    BigInt &operator+=(const BigInt &other)
    {
        if (isNegative == other.isNegative)
        {
            string result = "";
            int carry = 0;
            int i = (int)number.size() - 1;
            int j = (int)other.number.size() - 1;

            while (i >= 0 || j >= 0 || carry)
            {
                int digit1 = (i >= 0 ? number[i] - '0' : 0);
                int digit2 = (j >= 0 ? other.number[j] - '0' : 0);
                int sum = digit1 + digit2 + carry;

                result.push_back((sum % 10) + '0');
                carry = sum / 10;

                i--;
                j--;
            }

            reverse(result.begin(), result.end());
            number = result;
        }
        else
        {
            *this -= (-other);
        }

        removeLeadingZeros();
        return *this;
    }

    // Subtraction assignment operator (x -= y)
    BigInt &operator-=(const BigInt &other)
    {
        if (isNegative != other.isNegative)
        {
            *this += (-other);
            return *this;
        }

        int mag_cmp = compareMagnitude(other);
        if (mag_cmp == 0)
        {
            *this = BigInt(0);
            return *this;
        }

        string n1 = (mag_cmp > 0) ? this->number : other.number;
        string n2 = (mag_cmp > 0) ? other.number : this->number;
        bool result_negative = (mag_cmp < 0 && !this->isNegative) || (mag_cmp > 0 && this->isNegative);

        string result_str = "";
        int borrow = 0;
        int i = n1.length() - 1;
        int j = n2.length() - 1;

        while (i >= 0)
        {
            int digit1 = n1[i] - '0' - borrow;
            int digit2 = (j >= 0) ? n2[j] - '0' : 0;
            if (digit1 < digit2)
            {
                digit1 += 10;
                borrow = 1;
            }
            else
            {
                borrow = 0;
            }
            result_str.push_back((digit1 - digit2) + '0');
            i--;
            j--;
        }

        reverse(result_str.begin(), result_str.end());
        this->number = result_str;
        this->isNegative = result_negative;
        removeLeadingZeros();

        return *this;
    }

    // Multiplication assignment operator (x *= y)
    BigInt &operator*=(const BigInt &other)
    {
        *this = *this * other;
        return *this;
    }

    // Division assignment operator (x /= y)
    BigInt &operator/=(const BigInt &other)
    {
        if (other.number == "0")
            throw runtime_error("Division by zero");

        bool result_negative = (this->isNegative != other.isNegative);

        // Make absolute values for division
        BigInt dividend = *this;
        dividend.isNegative = false;
        BigInt divisor = other;
        divisor.isNegative = false;

        if (dividend.compareMagnitude(divisor) < 0)
        {
            *this = BigInt(0);
            return *this;
        }

        string quotient_str = "";
        BigInt remainder;

        for (size_t i = 0; i < dividend.number.length(); i++)
        {
            char d = dividend.number[i];
            // Build remainder digit by digit
            remainder.number += d;
            remainder.removeLeadingZeros();

            int count = 0;
            // Count how many times divisor goes into remainder
            while (remainder.compareMagnitude(divisor) >= 0)
            {
                remainder -= divisor;
                count++;
            }
            quotient_str.push_back(count + '0');
        }

        number = quotient_str;
        isNegative = result_negative;
        removeLeadingZeros();

        return *this;
    }

    // Modulus assignment operator (x %= y)
    BigInt &operator%=(const BigInt &other)
    {
        if (other.number == "0")
        {
            throw runtime_error("Invalid operation: Modulo by Zero");
        }

        *this = *this % other;
        return *this;
    }

    // Pre-increment operator (++x)
    BigInt &operator++()
    {
        *this += BigInt(1);
        return *this;
    }

    // Post-increment operator (x++)
    BigInt operator++(int)
    {
        BigInt temp = *this;
        *this += BigInt(1);
        return temp;
    }

    // Pre-decrement operator (--x)
    BigInt &operator--()
    {
        *this -= BigInt(1);
        return *this;
    }

    // Post-decrement operator (x--)
    BigInt operator--(int)
    {
        BigInt temp = *this;
        *this -= BigInt(1);
        return temp;
    }

    // Convert BigInt to string representation
    string toString() const
    {
        return (isNegative && number != "0" ? "-" : "") + number;
    }

    // Output stream operator (for printing)
    friend ostream &operator<<(ostream &os, const BigInt &num)
    {
        if (num.isNegative && num.number != "0")
        {
            os << '-';
        }
        os << num.number;
        return os;
    }

    // Input stream operator (for reading from input)
    friend istream &operator>>(istream &is, BigInt &num)
    {
        string input;
        is >> input;

        num = BigInt(input); // Use the string constructor for parsing logic

        return is;
    }

    // Friend declarations for all binary operators
    friend bool operator==(const BigInt &lhs, const BigInt &rhs);
    friend bool operator<(const BigInt &lhs, const BigInt &rhs);

    friend BigInt operator*(BigInt lhs, const BigInt &rhs);
    friend BigInt operator/(BigInt lhs, const BigInt &rhs);
    friend BigInt operator%(BigInt lhs, const BigInt &rhs);
};

// Binary addition operator (x + y)
BigInt operator+(BigInt lhs, const BigInt &rhs)
{
    lhs += rhs;
    return lhs;
}

// Binary subtraction operator (x - y)
BigInt operator-(BigInt lhs, const BigInt &rhs)
{
    lhs -= rhs;
    return lhs;
}

// Binary multiplication operator (x * y)
BigInt operator*(BigInt lhs, const BigInt &rhs)
{
    if (lhs.number == "0" || rhs.number == "0")
    {
        return BigInt(0);
    }

    BigInt result;
    result.isNegative = (lhs.isNegative != rhs.isNegative);

    string n1 = lhs.number;
    string n2 = rhs.number;
    string res_str(n1.size() + n2.size(), '0');

    for (int i = n1.size() - 1; i >= 0; i--)
    {
        int carry = 0;
        for (int j = n2.size() - 1; j >= 0; j--)
        {
            int p = (n1[i] - '0') * (n2[j] - '0') + (res_str[i + j + 1] - '0') + carry;
            carry = p / 10;
            res_str[i + j + 1] = (p % 10) + '0';
        }
        res_str[i] = ((res_str[i] - '0') + carry) + '0';
    }

    result.number = res_str;
    result.removeLeadingZeros();
    return result;
}

// Binary division operator (x / y)
BigInt operator/(BigInt lhs, const BigInt &rhs)
{
    lhs /= rhs;
    return lhs;
}

// Binary modulus operator (x % y)
BigInt operator%(BigInt lhs, const BigInt &rhs)
{
    if (rhs.number == "0")
    {
        throw runtime_error("Invalid operation: Modulo by Zero");
    }

    bool lhs_negative = lhs.isNegative;
    BigInt quotient = lhs / rhs;
    BigInt result = lhs - (quotient * rhs);

    // Ensure modulus has same sign as dividend (lhs)
    if (lhs_negative && result.number != "0")
    {
        result.isNegative = true;
    }

    return result;
}

// Equality comparison operator (x == y)
bool operator==(const BigInt &lhs, const BigInt &rhs)
{
    return (lhs.isNegative == rhs.isNegative) && (lhs.number == rhs.number);
}

// Inequality comparison operator (x != y)
bool operator!=(const BigInt &lhs, const BigInt &rhs)
{
    return !(lhs == rhs);
}

// Less-than comparison operator (x < y)
bool operator<(const BigInt &lhs, const BigInt &rhs)
{
    // TODO: Implement this operator
    if (lhs.isNegative != rhs.isNegative)
    {
        return lhs.isNegative; // if lhs is negative and rhs isn't, lhs is smaller
    }
    // Both have the same sign
    if (!lhs.isNegative) // Both positive
    {
        return lhs.compareMagnitude(rhs) < 0;
    }
    else // Both negative
    {
        return lhs.compareMagnitude(rhs) > 0;
    }
}

// Less-than-or-equal comparison operator (x <= y)
bool operator<=(const BigInt &lhs, const BigInt &rhs)
{
    return (lhs < rhs || lhs == rhs);
}

// Greater-than comparison operator (x > y)
bool operator>(const BigInt &lhs, const BigInt &rhs)
{
    return !(lhs <= rhs);
}

// Greater-than-or-equal comparison operator (x >= y)
bool operator>=(const BigInt &lhs, const BigInt &rhs)
{
    return !(lhs < rhs);
}

int main()
{
    cout << "=== BigInt Class Test Program ===" << endl
         << endl;
    cout << "NOTE: All functions are currently empty." << endl;
    cout << "Your task is to implement ALL the functions above." << endl;
    cout << "The tests below will work once you implement them correctly." << endl
         << endl;

    // Test 1: Constructors and basic output
    cout << "1. Constructors and output:" << endl;
    BigInt a(12345);                          // Should create BigInt from integer
    BigInt b("-67890");                       // Should create BigInt from string
    BigInt c("0");                            // Should handle zero correctly
    BigInt d = a;                             // Should use copy constructor
    cout << "a (from int): " << a << endl;    // Should print "12345"
    cout << "b (from string): " << b << endl; // Should print "-67890"
    cout << "c (zero): " << c << endl;        // Should print "0"
    cout << "d (copy of a): " << d << endl
         << endl; // Should print "12345"

    // Test 2: Arithmetic operations
    cout << "2. Arithmetic operations:" << endl;
    cout << "a + b = " << a + b << endl; // Should calculate 12345 + (-67890)
    cout << "a - b = " << a - b << endl; // Should calculate 12345 - (-67890)
    cout << "a * b = " << a * b << endl; // Should calculate 12345 * (-67890)
    cout << "b / a = " << b / a << endl; // Should calculate (-67890) / 12345
    cout << "a % 100 = " << a % BigInt(100) << endl
         << endl; // Should calculate 12345 % 100

    // Test 3: Relational operators
    cout << "3. Relational operators:" << endl;
    cout << "a == d: " << (a == d) << endl; // Should be true (12345 == 12345)
    cout << "a != b: " << (a != b) << endl; // Should be true (12345 != -67890)
    cout << "a < b: " << (a < b) << endl;   // Should be false (12345 < -67890)
    cout << "a > b: " << (a > b) << endl;   // Should be true (12345 > -67890)
    cout << "c == 0: " << (c == BigInt(0)) << endl
         << endl; // Should be true (0 == 0)

    // Test 4: Unary operators and increments
    cout << "4. Unary operators and increments:" << endl;
    cout << "-a: " << -a << endl;   // Should print "-12345"
    cout << "++a: " << ++a << endl; // Should increment and print "12346"
    cout << "a--: " << a-- << endl; // Should print "12346" then decrement
    cout << "a after decrement: " << a << endl
         << endl; // Should print "12345"

    // Test 5: Large number operations
    cout << "5. Large number operations:" << endl;
    BigInt num1("12345678901234567890");
    BigInt num2("98765432109876543210");
    cout << "Very large addition: " << num1 + num2 << endl;
    cout << "Very large multiplication: " << num1 * num2 << endl
         << endl;

    // Test 6: Edge cases and error handling
    cout << "6. Edge cases:" << endl;
    BigInt zero(0);
    BigInt one(1);
    try
    {
        BigInt result = one / zero; // Should throw division by zero error
        cout << "Division by zero succeeded (unexpected)" << endl;
    }
    catch (const runtime_error &e)
    {
        cout << "Division by zero correctly threw error: " << e.what() << endl;
    }
    cout << "Multiplication by zero: " << one * zero << endl;              // Should be "0"
    cout << "Negative multiplication: " << BigInt(-5) * BigInt(3) << endl; // Should be "-15"
    cout << "Negative division: " << BigInt(-10) / BigInt(3) << endl;      // Should be "-3"
    cout << "Negative modulus: " << BigInt(-10) % BigInt(3) << endl;       // Should be "-1"

    return 0;
}