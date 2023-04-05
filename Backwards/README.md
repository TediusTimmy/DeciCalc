Backwards ... Or Is It?
=======================

This is, essentially, Backwards. See the Backwards repository for how the language works.

Differences:  
1. Uses libdecmath for a 16 digit decimal floating point type, rather than nine digit.
2. Has a pretty-printer for numbers, so that numbers are more natural looking in the human range.
3. Removed the power operator. (It's was just a call to `pow`, which is backed by `exp` and `ln`.)
4. Removed 21 functions of the standard library, and added six. Functions Removed: PI, Date, Time, Sin and family (Asin, Sinh, etc), Exp, Ln, Sqrt, Cbrt, DegToRad, RadToDeg, Hypot, and Log. With the exception of the time functions, all of these represent functions backed by series algorithms that should just be done in binary. Functions Added: NaN, IsNil, IsCellRef, IsCellRange, EvalCell, and ExpandRange.
5. Added three types: Nil, CellRef, and CellRange. They are all opaque types that allow manipulating the spreadsheet constructs: the empty cell, a reference to another cell (overloaded to be any spreadsheet expression), and a specifier for a range of cells (that can only be expanded into references to those cells).

While I realize that powers are used in financial calculations, and Exp for continuously compounded interest, generalized power operations probably mean that you should just do your computation in binary.


How To Use
----------

Library files can be specified with `-l <filename>`. Function names that are ALL CAPS and lack any underscores will be exported to the spreadsheet as a usable function.
