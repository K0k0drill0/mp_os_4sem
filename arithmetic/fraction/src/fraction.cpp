#include "../include/fraction.h"

#include <not_implemented.h>
#include <sstream>
#include <iostream>

fraction::fraction(
    big_integer &&numerator,
    big_integer &&denominator):
        _numerator(numerator),
        _denominator(denominator)
{
    if (_denominator == big_integer(0)) {
        throw std::logic_error("zero division");
    }

    this->simplify();    
}

fraction::fraction(
    big_integer &numerator,
    big_integer &denominator):
        _numerator(numerator),
        _denominator(denominator)
{
    if (_denominator == big_integer(0)) {
        throw std::logic_error("zero division");
    }

    this->simplify();    
}     


fraction::fraction() :
    _numerator(big_integer(0)), 
    _denominator(big_integer(1)) 
{

}

fraction::~fraction()
{
}

fraction::fraction(
    fraction const &other):
        _numerator(other._numerator),
        _denominator(other._denominator)
{
}

fraction &fraction::operator=(
    fraction const &other)
{
    _numerator = other._numerator;
    _denominator = other._denominator;
    return *this;
}

fraction::fraction(
    fraction &&other) noexcept:
        _numerator(std::move(other._numerator)),
        _denominator(std::move(other._denominator))
{
}

fraction &fraction::operator=(
    fraction &&other) noexcept 
{
    this->_numerator = other._numerator;
    this->_denominator = other._denominator;

    return *this;
}

fraction &fraction::operator+=(
    fraction const &other)
{
    fraction c_other(other);

    if (this->sign() == -1 && other.sign() == -1) {
        this->change_sign();
        c_other.change_sign();
        *this += c_other;
        return this->change_sign();
    }

    if (this->sign() == -1 && other.sign() == 1) {     
        this->change_sign();
        fraction res = other - *this; 
        return (*this = res);
    } 

    if (other.sign() == -1 && this->sign() == 1) {
        // fraction tmp(other);
        c_other.change_sign(); 
        fraction res = *this - c_other;
        return (*this = res);
    }

    make_same_denominator(*this, c_other);

    this->_numerator += c_other._numerator;

    this->simplify();

    return (*this);
}

fraction fraction::operator+(
    fraction const &other) const
{
    return (fraction(*this) += other);
}

fraction &fraction::operator-=(
    fraction const &other)
{
    fraction c_other(other);

    if (this->sign() == -1 && c_other.sign() == -1) {
        c_other.change_sign();
        *this = c_other - *this; 
        return *this;
    } 

    if (this->sign() == 1 && c_other.sign() == -1) {
        c_other.change_sign();
        *this += c_other;
        return *this;
    }

    if (this->sign() == -1 && c_other.sign() == 1) {
        this->change_sign();
        *this += c_other;
        this->change_sign();
        return *this;
    }

    make_same_denominator(*this, c_other);

    this->_numerator -= c_other._numerator;

    if (this->_numerator.sign() == -1) {
        this->_numerator *= big_integer(-1);
        this->_denominator *= big_integer(-1);
    }

    return this->simplify();
}

fraction fraction::operator-(
    fraction const &other) const
{
    return (fraction(*this) -= other);
}

fraction fraction::operator-() const
{
    fraction cpy = const_cast<fraction&>(*this).change_sign();
    const_cast<fraction&>(*this).change_sign();
    return (cpy);
}

fraction &fraction::operator*=(
    fraction const &other)
{
    this->_numerator *= other._numerator;
    this->_denominator *= other._denominator;

    return this->simplify();
}

fraction fraction::operator*(
    fraction const &other) const
{
    return (fraction(*this) *= other);
}

fraction &fraction::operator/=(
    fraction const &other)
{
    if (other._numerator == big_integer(0)) {
        throw std::logic_error("division by zero");
    }

    bool is_negative = false;

    fraction c_other(other);

    if (this->sign() == -1 && c_other.sign() == -1) {
        this->change_sign();
        c_other.change_sign();
    } 

    if (this->sign() == -1 && c_other.sign() == 1) {
        this->change_sign();
        is_negative = true;
    }

    if (this->sign() == 1 && c_other.sign() == -1) {
        c_other.change_sign();
        is_negative = true;
    }

    this->_numerator *= c_other._denominator;
    this->_denominator *= c_other._numerator;

    this->simplify();

    if (is_negative) {
        this->change_sign();
    }

    return *this;
}

fraction fraction::operator/(
    fraction const &other) const
{
    return (fraction(*this) /= other);
}

bool fraction::operator==(
    fraction const &other) const
{
    return (this->_numerator == other._numerator && this->_denominator == other._denominator);
}

bool fraction::operator!=(
    fraction const &other) const
{
    return !(*this == other);
}

bool fraction::operator>=(
    fraction const &other) const
{
    return !(*this < other);
}

bool fraction::operator>(
    fraction const &other) const
{
    if (this->sign() == -1 && other.sign() == -1) {
        return (*this < other);
    } 

    if (this->sign() != other.sign()) {
        return (this->sign() == 1);
    }

    fraction c_this = *this;
    fraction c_other = other;

    make_same_denominator(c_this, c_other);

    return (c_this._numerator > c_other._numerator);
}

bool fraction::operator<=(
    fraction const &other) const
{
    return !(*this > other);
}

bool fraction::operator<(
    fraction const &other) const
{
    if (this->sign() == -1 && other.sign() == -1) {
        return (*this > other);
    } 

    if (this->sign() != other.sign()) {
        return (this->sign() == -1);
    }

    fraction c_this = *this;
    fraction c_other = other;

    make_same_denominator(c_this, c_other);

    return (c_this._numerator < c_other._numerator);
}

std::ostream &operator<<(
    std::ostream &stream,
    fraction const &obj)
{

    std::stringstream ss;
    ss << obj._denominator; 
    std::string denom_no_sign = ss.str();
    

    bool is_negative = false;
    if (denom_no_sign[0] == '-') {
        denom_no_sign.erase(denom_no_sign.begin(), denom_no_sign.begin() + 1);
        is_negative = true;
    }

    if (is_negative) {
        stream << '-';
    } 

    stream << obj._numerator << '/' << denom_no_sign;

    return stream;
}


std::istream &operator>>(
    std::istream &stream,
    fraction &obj)
{
	std::string s, str;
	std::string a, b;
	int i = 0;
	stream >> s;

	std::stringstream ss(s);

	while (getline(ss, str, '/'))
	{
		if (i == 0)
		{
			a = str;
		}
		if (i == 1)
		{
			b = str;
		}
		++i;
	}
	obj = fraction(big_integer(a),big_integer(b));
	return stream;
}

fraction fraction::abs() const
{
    if (this->sign() == 1) {
        return *this;
    }
    fraction tmp(*this);
    return tmp.change_sign();
}

fraction fraction::sin(
    fraction const &epsilon) const
{
    if (!is_valid_eps(epsilon)) {
        throw std::logic_error("invalid epsilon");
    }

    fraction term = *this;
    fraction result(big_integer(0), big_integer(1));
    int n = 1;
    while (term.abs() >= epsilon) {
        result += term;
        
        term = -term * *this * *this / fraction(big_integer((2 * n ) * (2 * n + 1)), big_integer(1));
        n++;
    }

    return result;
}

fraction fraction::cos(
    fraction const &epsilon) const
{
    if (!is_valid_eps(epsilon)) {
        throw std::logic_error("invalid epsilon");
    }

    fraction term(big_integer(1), big_integer(1));
    fraction result = term;
    int n = 1;

    while (term.abs() >= epsilon) {
        std::cout << "res: " << result << std::endl;
        term = -term * *this * *this / fraction(big_integer((n) * (n + 1)), big_integer(1));
        std::cout << "term :" << term << std::endl << std::endl;

        result += term;
        n+=2;
  }

  return result;
}

fraction fraction::tg(
    fraction const &epsilon) const
{
    fraction sin_value = this->sin(epsilon);
	fraction cos_value = this->cos(epsilon);
	fraction result = sin_value / cos_value;
	return result;
}

fraction fraction::ctg(
    fraction const &epsilon) const
{
    fraction sin_value = this->sin(epsilon);
	fraction cos_value = this->cos(epsilon);
	fraction result = cos_value/sin_value;
	return result;
}

fraction fraction::sec(
    fraction const &epsilon) const
{
    return fraction(big_integer(1), big_integer(1)) / this->cos(epsilon);
}

fraction fraction::cosec(
    fraction const &epsilon) const
{
    return fraction(big_integer(1), big_integer(1)) / this->sin(epsilon);
}

fraction fraction::arcsin(
    fraction const &epsilon) const
{
    throw not_implemented("fraction fraction::arcsin(fraction const &) const", "your code should be here...");
}

fraction fraction::arccos(
    fraction const &epsilon) const
{
    throw not_implemented("fraction fraction::arccos(fraction const &) const", "your code should be here...");
}

fraction fraction::arctg(
    fraction const &epsilon) const
{
    throw not_implemented("fraction fraction::arctg(fraction const &) const", "your code should be here...");
}

fraction fraction::arcctg(
    fraction const &epsilon) const
{
    throw not_implemented("fraction fraction::arcctg(fraction const &) const", "your code should be here...");
}

fraction fraction::arcsec(
    fraction const &epsilon) const
{
    throw not_implemented("fraction fraction::arcsec(fraction const &) const", "your code should be here...");
}

fraction fraction::arccosec(
    fraction const &epsilon) const
{
    throw not_implemented("fraction fraction::arccosec(fraction const &) const", "your code should be here...");
}

fraction fraction::pow(
    size_t degree) const
{
    if (degree == 0) {
        return fraction(big_integer(1), big_integer(1)); // Любое число в степени 0 равно 1
    }
    if (degree < 0) {
        return fraction(big_integer(1), big_integer(1)) / this->pow(-degree); // Обратное значение для отрицательных степеней
    }

    fraction base(*this);
    fraction result(big_integer(1), big_integer(1));
    while (degree > 0) {
    if (degree % 2 == 1) { // Если показатель степени нечетный
        result *= base;
        degree--;
    }
    base *= base; // Квадрат основания
    degree /= 2; // Делим показатель степени пополам
    }

    return result;
}

fraction fraction::root(
    size_t degree,
    fraction const &epsilon) const
{
    throw not_implemented("fraction fraction::root(size_t, fraction const &) const", "your code should be here...");
}

fraction fraction::log2(
    fraction const &epsilon) const
{
    throw not_implemented("fraction fraction::log2(fraction const &) const", "your code should be here...");
}

fraction fraction::ln(
    fraction const &epsilon) const
{
    throw not_implemented("fraction fraction::ln(fraction const &) const", "your code should be here...");
}

fraction fraction::lg(
    fraction const &epsilon) const
{
    throw not_implemented("fraction fraction::lg(fraction const &) const", "your code should be here...");
}

big_integer fraction::gcd (big_integer a, big_integer b) {
	while (!b.is_equal_to_zero()) {
		a %= b;
		std::swap (a, b);
	}
	return a;
}

big_integer fraction::lcm(big_integer a, big_integer b) {
    big_integer res = (a * b) / gcd(a, b);
    return res;
}

fraction& fraction::simplify() 
{

    if (_numerator.sign() == -1) 
    {
        _numerator *= big_integer(-1);
        _denominator *= big_integer(-1);
    }

    bool is_negative = false;    

    if (_denominator.sign() == -1) {
        _denominator *= big_integer(-1);
        is_negative = true;
    }

    if (_denominator == big_integer(0)) {
        throw std::logic_error("Null division.");
    }

    big_integer gcd = fraction::gcd(this->_numerator, this->_denominator);

    _numerator /= gcd;
    _denominator /= gcd;

    if (is_negative) {
        _denominator *= big_integer(-1);
    }

    return *this;
}

bool fraction::is_equal_to_zero() const
{
    return this->_numerator.is_equal_to_zero();
}

int fraction::sign() const {
    return _denominator.sign();
}

fraction& fraction::change_sign() {
    _denominator *= big_integer(-1);

    return *this;
}

void fraction::make_same_denominator(fraction &a, fraction &b) 
{
    big_integer nok = lcm(a._denominator, b._denominator);

    big_integer mult1 = nok / a._denominator;
    big_integer mult2 = nok / b._denominator;

    a._numerator *= mult1;
    a._denominator *= mult1;

    b._numerator *= mult2;
    b._denominator *= mult2;
}

int fraction::is_valid_eps(fraction const &eps) 
{
    return !((eps.sign() == -1) || eps.is_equal_to_zero());
}

// fraction calc_pi(fraction eps) {
//     fraction x0(big_integer(3), big_integer(1));
//     fraction x1 = x0;
//     do {
//         x0 = x1;
//         x1 = x0 + (x0.cos(eps) + fraction(big_integer(1), big_integer(1)) / x0.sin(eps)); 
//     } while ((x0 - x1).abs() > eps);
//     return x1;
// }