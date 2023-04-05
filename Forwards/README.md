Forwards
========

The name is a pun on Backwards. You could see it as: Forwards is forwards-facing (user-facing), and Backwards is the back-end language.

Forwards is a stripped-down spreadsheet language that is a blend of VisiCalc (or Lotus 1-2-3) and Excel.

It supports addition, subtraction, multiplication, and division, but not exponents. They follow the normal order of operations. It also has the six relational operations (retaining `<>` for not equal), but doesn't have a logical type, instead using 1.0 for true and 0.0 for false. It uses `&` for string concatenation. You specify cell ranges as `A1:B2`, and it uses that notation for cells. All functions are in ALL CAPS and have to have a `@` before the function name.

The StdLib.txt file in the unit tests IS the only standard library I intend to implement. All other functions will be user-supplied through the `-l` switch.
