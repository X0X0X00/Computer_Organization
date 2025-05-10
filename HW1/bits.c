/* 
 * CS:APP Data Lab 
 * 
 * <Zhenhao Zhang zzh133 & Yicheng Wang ywang407>
 *
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:

  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code
  must conform to the following style:

  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>

  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.


  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function.
     The max operator count is checked by dlc. Note that '=' is not
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 *
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce
 *      the correct answers.
 */


#endif
/* Copyright (C) 1991-2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */
/* This header is separate from features.h so that the compiler can
   include it implicitly at the start of every compilation.  It must
   not itself include <features.h> or any other header that includes
   <features.h> because the implicit include comes before any feature
   test macros that may be defined in a source file before it first
   explicitly includes a system header.  GCC knows the name of this
   header in order to preinclude it.  */
/* We do support the IEC 559 math functionality, real and complex.  */
/* wchar_t uses ISO/IEC 10646 (2nd ed., published 2011-03-15) /
   Unicode 6.0.  */
/* We do not support C11 <threads.h>.  */

/*
 * bitXor - x^y using only ~ and &
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
    return ~(~x & ~y) & ~(x & y);
}

/*
 * oddBits - return word with all odd-numbered bits set to 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 2
 */
int oddBits(void) {
    int mask = 0xAA;       // 0xAA is 10101010 in binary (sets every odd bit in a byte)
    mask = mask | (mask << 8);  // Extend the pattern to 16 bits
    mask = mask | (mask << 16); // Extend the pattern to 32 bits
    return mask;
}

/*
 * reverseBytes - reverse the bytes of x
 *   Example: reverseBytes(0x01020304) = 0x04030201
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 25
 *   Rating: 3
 */
int reverseBytes(int x) {
    // Extract and shift each byte
    int byte0 = (x & 0xFF) << 24;
    int byte1 = (x & (0xFF << 8)) << 8;
    int byte2 = ((x & (0xFF << 16)) >> 8) & (0xFF << 8);
    int byte3 = ((x & (0xFF << 24)) >> 24) & 0xFF;
    return byte0 | byte1 | byte2 | byte3;
}

/*
 * rotateRight - Rotate x to the right by n
 *   Can assume that 0 <= n <= 31
 *   Examples: rotateRight(0x87654321,4) = 0x18765432
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 25
 *   Rating: 3
 */
int rotateRight(int x, int n) {
    // Compute mask to clear sign bits introduced by arithmetic right shift
    int mask = ~(((1 << n) + ~0) << (32 + ~n + 1));
    // Logical right shift by masking out sign-extended bits
    int shifted = (x >> n) & mask;
    // Capture the bits that wrap around and shift them to the left
    int rotated = (x & ((1 << n) + ~0)) << (32 + ~n + 1);
    // Combine both parts
    return shifted | rotated;
}

/*
 * bitCount - returns count of number of 1's in word
 *   Examples: bitCount(5) = 2, bitCount(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 40
 *   Rating: 4
 */
int bitCount(int x) {
    // Construct masks using allowed constants (0x0 to 0xFF)
    int mask1 = 0x55 | (0x55 << 8);  // 0x5555
    int mask2 = 0x33 | (0x33 << 8);  // 0x3333
    int mask3 = 0x0F | (0x0F << 8);  // 0x0F0F
    int mask4 = 0xFF | (0xFF << 16); // 0x00FF00FF
    int mask5 = 0xFF | (0xFF << 8);  // 0x0000FFFF
    mask1 = mask1 | (mask1 << 16);   // 0x55555555
    mask2 = mask2 | (mask2 << 16);   // 0x33333333
    mask3 = mask3 | (mask3 << 16);   // 0x0F0F0F0F
    // Count the number of 1's using the divide-and-conquer approach
    x = (x & mask1) + ((x >> 1) & mask1);
    x = (x & mask2) + ((x >> 2) & mask2);
    x = (x & mask3) + ((x >> 4) & mask3);
    x = (x & mask4) + ((x >> 8) & mask4);
    x = (x & mask5) + ((x >> 16) & mask5);
    return x;
}

/*
 * TMax - return maximum two's complement integer
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmax(void) {
    return ~(1 << 31);
}

/*
 * addOK - Determine if can compute x+y without overflow
 *   Example: addOK(0x80000000,0x80000000) = 0,
 *            addOK(0x80000000,0x70000000) = 1,
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int addOK(int x, int y) {
    int sum = x + y;
    int overflow = ((x ^ sum) & (y ^ sum)) >> 31;
    // 只有当 x 和 y 的符号相同且与 sum 不同时，结果才会为非零
    // 提取符号位，判断是否有溢出（1 表示溢出，0 表示没有溢出）
    return !overflow;
}

/*
 * rempwr2 - Compute x%(2^n), for 0 <= n <= 30
 *   Negative arguments should yield negative remainders
 *   Examples: rempwr2(15,2) = 3, rempwr2(-35,3) = -3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int rempwr2(int x, int n) {
    int mask = (1 << n) + ~0;
    int rem = x & mask;
    int sign = x >> 31;
    int adjust = sign & (((rem | (~rem + 1)) >> 31) & (mask + 1));
    return rem + (~adjust + 1);
}

/*
 * satMul2 - multiplies by 2, saturating to Tmin or Tmax if overflow
 *   Examples: satMul2(0x30000000) = 0x60000000
 *             satMul2(0x40000000) = 0x7FFFFFFF (saturate to TMax)
 *             satMul2(0xe0000000) = 0x80000000 (saturate to TMin)
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int satMul2(int x) {
    int result = x << 1;
    int overflow = (x ^ result) >> 31; // Check for overflow (sign change)
    int tmax = ~(1 << 31); // TMax: 0x7FFFFFFF
    int tmin = 1 << 31; // TMin: 0x80000000
    // Determine if overflow occurred and whether x was positive or negative
    int is_positive_overflow = overflow & ~(x >> 31); // Overflow and x was positive
    int is_negative_overflow = overflow & (x >> 31);  // Overflow and x was negative

    // Saturate to TMax or TMin based on overflow
    result = (is_positive_overflow & tmax) | (is_negative_overflow & tmin) | (~overflow & result);

    return result;
}

/*
 * isGreater - if x > y  then return 1, else return 0
 *   Example: isGreater(4,5) = 0, isGreater(5,4) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isGreater(int x, int y) {
    int sign_x = x >> 31;             // Sign of x (all 1's if negative)
    int sign_y = y >> 31;             // Sign of y (all 1's if negative)
    int diff_sign = sign_x ^ sign_y;  // 1 if signs differ, 0 otherwise

    // Case 1: Handle differing signs. If x is positive and y is negative, x > y.
    int case1 = (!sign_x) & (diff_sign >> 31);

    // Case 2: Same signs. Compute x - y and check if result is positive.
    int diff = x + ~y + 1;            // Equivalent to x - y
    int case2 = !((diff >> 31) | !diff); // 1 if diff > 0, else 0

    // Select case1 if signs differ, case2 if same
    return (case1 & diff_sign) | (case2 & ~diff_sign);
}

/*
 * multFiveEighths - multiplies by 5/8 rounding toward 0.
 *   Should exactly duplicate effect of C expression (x*5/8),
 *   including overflow behavior.
 *   Examples: multFiveEighths(77) = 48
 *             multFiveEighths(-22) = -13
 *             multFiveEighths(1073741824) = 13421728 (overflow)
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 3
 */
int multFiveEighths(int x) {
    int fiveX = (x << 2) + x;
    int sign = fiveX >> 31;
    return (fiveX + (sign & 7)) >> 3;
}

/*
 * isNonZero - Check whether x is nonzero using
 *              the legal operators except !
 *   Examples: isNonZero(3) = 1, isNonZero(0) = 0
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 10
 *   Rating: 4
 */
int isNonZero(int x) {
    return ((x | (~x + 1)) >> 31) & 1;
}

/*
 * float_abs - Return bit-level equivalent of absolute value of f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned float_abs(unsigned uf) {
    if ((uf & 0x7FFFFFFF) > 0x7F800000) {
        return uf; // NaN
    }
    return uf & 0x7FFFFFFF;
}

/*
 * float_f2i - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int float_f2i(unsigned uf) {
    unsigned sign = uf >> 31;
    unsigned exp = (uf >> 23) & 0xFF;
    unsigned frac = uf & 0x7FFFFF;
    int exp_fix;
    int val;
    unsigned s = (1 << 23) | frac;
    unsigned result_unsigned = 0;
    if (exp == 0xFF) {
        return 0x80000000u;
    }
    exp_fix = exp - 127;

    if (exp_fix >= 31) {
        return 0x80000000u;
    }

    if (exp_fix >= 0) {
        if (exp_fix >= 23) {
            result_unsigned = s << (exp_fix - 23);
        } else {
            result_unsigned = s >> (23 - exp_fix);
        }

        if (sign == 0 && result_unsigned > 0x7FFFFFFF) {
            return 0x80000000u;
        }
        if (sign == 1 && result_unsigned >= 0x80000000) {
            return 0x80000000u;
        }
    }

    if (sign) {
        val = - result_unsigned;
    } else {
        val = result_unsigned;
    }

    return val;
}

/*
 * float_half - Return bit-level equivalent of expression 0.5*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_half(unsigned uf) {
    unsigned sign = uf & 0x80000000;
    unsigned exp = (uf >> 23) & 0xFF;
    unsigned frac = uf & 0x7FFFFF;

    if (exp == 0xFF) return uf; // NaN or infinity

    if (exp == 0) {
        // Denormalized: shift right and round to nearest even
        unsigned shifted_out = frac & 1;
        frac >>= 1;
        if (shifted_out && (frac & 1)) {
            frac++;
        }
    } else if (exp == 1) {
        // Normalized to denormalized: handle rounding after shifting
        unsigned shifted_out = frac & 1;
        frac = (frac >> 1) | 0x400000; // Add implicit leading 1's shifted part
        if (shifted_out && (frac & 1)) {
            frac++;
        }
        exp = 0;
    } else {
        exp--; // Normalized: simply decrement exponent
    }

    return sign | (exp << 23) | frac;
}
