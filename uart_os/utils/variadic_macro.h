/*
 * variadic-macro.h
 *
 *  Created on: 21.04.2013
 *      Author: Wolfgang
 */

#ifndef VARIADIC_MACRO_H_
#define VARIADIC_MACRO_H_

/*
 * Variadic macro iteration
 */
#define STRINGIZE(arg)  STRINGIZE1(arg)
#define STRINGIZE1(arg) STRINGIZE2(arg)
#define STRINGIZE2(arg) #arg

#define CONCATENATE(arg1, arg2)   CONCATENATE1(arg1, arg2)
#define CONCATENATE1(arg1, arg2)  CONCATENATE2(arg1, arg2)
#define CONCATENATE2(arg1, arg2)  arg1##arg2

#define FOR_EACH_RSEQ_N() 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, \
                          29, 28, 27, 26, 25, 24, 23, 22, 21, 20, \
                          19, 18, 17, 16, 15, 14, 13, 12, 11, 10, \
                          9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#define FOR_EACH_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, \
              _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
              _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, \
              _31, _32, _33, _34, _35, _36, _37, _38, _39, _IGNORED, N, ...) N

#define FOR_EACH_NARG_(...) FOR_EACH_ARG_N(__VA_ARGS__)
#define FOR_EACH_NARG(...) FOR_EACH_NARG_("_IGNORED", ##__VA_ARGS__, FOR_EACH_RSEQ_N())

#define FOR_EACH_0(what, ...)
#define FOR_EACH_1(what, x, ...)  what(1, x)
#define FOR_EACH_2(what, x, ...)  what(2, x)  FOR_EACH_1(what, __VA_ARGS__)
#define FOR_EACH_3(what, x, ...)  what(3, x)  FOR_EACH_2(what, __VA_ARGS__)
#define FOR_EACH_4(what, x, ...)  what(4, x)  FOR_EACH_3(what, __VA_ARGS__)
#define FOR_EACH_5(what, x, ...)  what(5, x)  FOR_EACH_4(what, __VA_ARGS__)
#define FOR_EACH_6(what, x, ...)  what(6, x)  FOR_EACH_5(what, __VA_ARGS__)
#define FOR_EACH_7(what, x, ...)  what(7, x)  FOR_EACH_6(what, __VA_ARGS__)
#define FOR_EACH_8(what, x, ...)  what(8, x)  FOR_EACH_7(what, __VA_ARGS__)
#define FOR_EACH_9(what, x, ...)  what(9, x)  FOR_EACH_8(what, __VA_ARGS__)
#define FOR_EACH_10(what, x, ...) what(10, x) FOR_EACH_9(what, __VA_ARGS__)
#define FOR_EACH_11(what, x, ...) what(11, x) FOR_EACH_10(what, __VA_ARGS__)
#define FOR_EACH_12(what, x, ...) what(12, x) FOR_EACH_11(what, __VA_ARGS__)
#define FOR_EACH_13(what, x, ...) what(13, x) FOR_EACH_12(what, __VA_ARGS__)
#define FOR_EACH_14(what, x, ...) what(14, x) FOR_EACH_13(what, __VA_ARGS__)
#define FOR_EACH_15(what, x, ...) what(15, x) FOR_EACH_14(what, __VA_ARGS__)
#define FOR_EACH_16(what, x, ...) what(16, x) FOR_EACH_15(what, __VA_ARGS__)
#define FOR_EACH_17(what, x, ...) what(17, x) FOR_EACH_16(what, __VA_ARGS__)
#define FOR_EACH_18(what, x, ...) what(18, x) FOR_EACH_17(what, __VA_ARGS__)
#define FOR_EACH_19(what, x, ...) what(19, x) FOR_EACH_18(what, __VA_ARGS__)
#define FOR_EACH_20(what, x, ...) what(20, x) FOR_EACH_19(what, __VA_ARGS__)
#define FOR_EACH_21(what, x, ...) what(21, x) FOR_EACH_20(what, __VA_ARGS__)
#define FOR_EACH_22(what, x, ...) what(22, x) FOR_EACH_21(what, __VA_ARGS__)
#define FOR_EACH_23(what, x, ...) what(23, x) FOR_EACH_22(what, __VA_ARGS__)
#define FOR_EACH_24(what, x, ...) what(24, x) FOR_EACH_23(what, __VA_ARGS__)
#define FOR_EACH_25(what, x, ...) what(25, x) FOR_EACH_24(what, __VA_ARGS__)
#define FOR_EACH_26(what, x, ...) what(26, x) FOR_EACH_25(what, __VA_ARGS__)
#define FOR_EACH_27(what, x, ...) what(27, x) FOR_EACH_26(what, __VA_ARGS__)
#define FOR_EACH_28(what, x, ...) what(28, x) FOR_EACH_27(what, __VA_ARGS__)
#define FOR_EACH_29(what, x, ...) what(29, x) FOR_EACH_28(what, __VA_ARGS__)
#define FOR_EACH_30(what, x, ...) what(30, x) FOR_EACH_29(what, __VA_ARGS__)
#define FOR_EACH_31(what, x, ...) what(31, x) FOR_EACH_30(what, __VA_ARGS__)
#define FOR_EACH_32(what, x, ...) what(32, x) FOR_EACH_31(what, __VA_ARGS__)
#define FOR_EACH_33(what, x, ...) what(33, x) FOR_EACH_32(what, __VA_ARGS__)
#define FOR_EACH_34(what, x, ...) what(34, x) FOR_EACH_33(what, __VA_ARGS__)
#define FOR_EACH_35(what, x, ...) what(35, x) FOR_EACH_34(what, __VA_ARGS__)
#define FOR_EACH_36(what, x, ...) what(36, x) FOR_EACH_35(what, __VA_ARGS__)
#define FOR_EACH_37(what, x, ...) what(37, x) FOR_EACH_36(what, __VA_ARGS__)
#define FOR_EACH_38(what, x, ...) what(38, x) FOR_EACH_37(what, __VA_ARGS__)
#define FOR_EACH_39(what, x, ...) what(39, x) FOR_EACH_38(what, __VA_ARGS__)



#define R_FOR_EACH_0(what, ...)
#define R_FOR_EACH_1(what, x, ...)  what(1, x)
#define R_FOR_EACH_2(what, x, ...)  R_FOR_EACH_1(what, __VA_ARGS__)  what(2, x)
#define R_FOR_EACH_3(what, x, ...)  R_FOR_EACH_2(what, __VA_ARGS__)  what(3, x)
#define R_FOR_EACH_4(what, x, ...)  R_FOR_EACH_3(what, __VA_ARGS__)  what(4, x)
#define R_FOR_EACH_5(what, x, ...)  R_FOR_EACH_4(what, __VA_ARGS__)  what(5, x)
#define R_FOR_EACH_6(what, x, ...)  R_FOR_EACH_5(what, __VA_ARGS__)  what(6, x)
#define R_FOR_EACH_7(what, x, ...)  R_FOR_EACH_6(what, __VA_ARGS__)  what(7, x)
#define R_FOR_EACH_8(what, x, ...)  R_FOR_EACH_7(what, __VA_ARGS__)  what(8, x)
#define R_FOR_EACH_9(what, x, ...)  R_FOR_EACH_8(what, __VA_ARGS__)  what(9, x)
#define R_FOR_EACH_10(what, x, ...) R_FOR_EACH_9(what, __VA_ARGS__)  what(10, x)
#define R_FOR_EACH_11(what, x, ...) R_FOR_EACH_10(what, __VA_ARGS__) what(11, x)
#define R_FOR_EACH_12(what, x, ...) R_FOR_EACH_11(what, __VA_ARGS__) what(12, x)
#define R_FOR_EACH_13(what, x, ...) R_FOR_EACH_12(what, __VA_ARGS__) what(13, x)
#define R_FOR_EACH_14(what, x, ...) R_FOR_EACH_13(what, __VA_ARGS__) what(14, x)
#define R_FOR_EACH_15(what, x, ...) R_FOR_EACH_14(what, __VA_ARGS__) what(15, x)
#define R_FOR_EACH_16(what, x, ...) R_FOR_EACH_15(what, __VA_ARGS__) what(16, x)
#define R_FOR_EACH_17(what, x, ...) R_FOR_EACH_16(what, __VA_ARGS__) what(17, x)
#define R_FOR_EACH_18(what, x, ...) R_FOR_EACH_17(what, __VA_ARGS__) what(18, x)
#define R_FOR_EACH_19(what, x, ...) R_FOR_EACH_18(what, __VA_ARGS__) what(19, x)
#define R_FOR_EACH_20(what, x, ...) R_FOR_EACH_19(what, __VA_ARGS__) what(20, x)
#define R_FOR_EACH_21(what, x, ...) R_FOR_EACH_20(what, __VA_ARGS__) what(21, x)
#define R_FOR_EACH_22(what, x, ...) R_FOR_EACH_21(what, __VA_ARGS__) what(22, x)
#define R_FOR_EACH_23(what, x, ...) R_FOR_EACH_22(what, __VA_ARGS__) what(23, x)
#define R_FOR_EACH_24(what, x, ...) R_FOR_EACH_23(what, __VA_ARGS__) what(24, x)
#define R_FOR_EACH_25(what, x, ...) R_FOR_EACH_24(what, __VA_ARGS__) what(25, x)
#define R_FOR_EACH_26(what, x, ...) R_FOR_EACH_25(what, __VA_ARGS__) what(26, x)
#define R_FOR_EACH_27(what, x, ...) R_FOR_EACH_26(what, __VA_ARGS__) what(27, x)
#define R_FOR_EACH_28(what, x, ...) R_FOR_EACH_27(what, __VA_ARGS__) what(28, x)
#define R_FOR_EACH_29(what, x, ...) R_FOR_EACH_28(what, __VA_ARGS__) what(29, x)
#define R_FOR_EACH_30(what, x, ...) R_FOR_EACH_29(what, __VA_ARGS__) what(30, x)
#define R_FOR_EACH_31(what, x, ...) R_FOR_EACH_30(what, __VA_ARGS__) what(31, x)
#define R_FOR_EACH_32(what, x, ...) R_FOR_EACH_31(what, __VA_ARGS__) what(32, x)
#define R_FOR_EACH_33(what, x, ...) R_FOR_EACH_32(what, __VA_ARGS__) what(33, x)
#define R_FOR_EACH_34(what, x, ...) R_FOR_EACH_33(what, __VA_ARGS__) what(34, x)
#define R_FOR_EACH_35(what, x, ...) R_FOR_EACH_34(what, __VA_ARGS__) what(35, x)
#define R_FOR_EACH_36(what, x, ...) R_FOR_EACH_35(what, __VA_ARGS__) what(36, x)
#define R_FOR_EACH_37(what, x, ...) R_FOR_EACH_36(what, __VA_ARGS__) what(37, x)
#define R_FOR_EACH_38(what, x, ...) R_FOR_EACH_37(what, __VA_ARGS__) what(38, x)
#define R_FOR_EACH_39(what, x, ...) R_FOR_EACH_38(what, __VA_ARGS__) what(39, x)


#define FOR_EACH_(N, what, x, ...) CONCATENATE(FOR_EACH_, N)(what, x, __VA_ARGS__)
#define FOR_EACH(what, x, ...) FOR_EACH_(FOR_EACH_NARG(x, ##__VA_ARGS__), what, x, ##__VA_ARGS__)

#define R_FOR_EACH_(N, what, x, ...) CONCATENATE(R_FOR_EACH_, N)(what, x, __VA_ARGS__)
#define R_FOR_EACH(what, x, ...) R_FOR_EACH_(FOR_EACH_NARG(x, ##__VA_ARGS__), what, x, ##__VA_ARGS__)

#define REST_ARGS(first,...) __VA_ARGS__
#define FIRST_ARG(first, ...) first


#endif /* VARIADIC_MACRO_H_ */
