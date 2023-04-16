DeciCalc
========

Why on Earth would I make a spreadsheet program? Haven't these been done to death? Yes, and for a short while still no, but also yes.

If you go to http://gnumeric.org/faq.html, you find that Gnumeric has, at the time of this writing, two frequently asked questions, and the first is "Why doesn't 0.3-(3*0.1) evaluate to zero?"

The very first spreadsheet programs for computers used decimal arithmetic: VisiCalc and SuperCalc. Using decimal eliminates this issue, however it is much slower and only business users will really notice the difference. I originally wrote libdecmath to be a full-featured decimal implementation for integration into Gnumeric (that's why it's in C). However, after I got into the Gnumeric code, I realized _why_ the main developers don't want to support decimal until they can flip a compiler switch. I also witnessed all of the hacks and bodges that Gnumeric goes through to hide the fact that it is doing binary math. It goes above and beyond hiding that floating-point numbers aren't real numbers, in my opinion. And, I didn't want to be working through all of that baggage.

As Sagan said, "If you want to bake an apple pie from scratch, you must first invent the universe." I want a spreadsheet program that does math in decimal, so it doesn't need to hide binary errors, but also doesn't try to hide that floating-point numbers aren't real numbers from me. And, the only real way I see to get that is by doing it myself. Maybe, one day, GCC will support C's optional decimal floating point types on x86 and Gnumeric will be compiled in decimal mode. Then, however, it will still have decades of hiding the minute differences between decimal and binary math in the code base.

So, I made a spreadsheet program to do simple things with money. Mostly, I want to track my portfolio with it. And, at this time, it is ready for me to begin eating my own dogfood.


How To Build
------------

First, get the libdecmath repo and put it at the same level as you have the DeciCalc checkout, with the name `libdecmath`. Then, run `make` to make a `DeciCalc.exe` binary. I know that this isn't a convention on Linux, but I am not good enough at Makefiles to change it.

The standard library is in `OddsAndEnds/StandardLibrary.cpp` if you so feel the need to change it. Yes, I ate my own dogfood and implemented it in Backwards. So, this backwards has had everything that uses transcendental functions removed: powers, pi, and trig functions. SlowFloat uses the binary functions for these, as 53 bits of precision is more than nine digits. However, it is arguable if 53 bits will always give sixteen digits of precision. As it is important to me that the program never lies about the precision of a result, so I removed them. This also means that I don't have to research algorithms to compute them (though, I really should do that for square root anyway). In general, if you want to do those sorts of operations, binary would serve you better anyway, and Gnumeric is a superior tool. This program has a very specific niche.


Capabilities
------------

The program is rather limited. It doesn't need features, because it has no competition: if you want to do anything more complicated than track a small number of currency values, you probably want a competent spreadsheet program that uses faster binary anyway. The only strength it has is that it supports 999,999,999 rows by 18,278 columns. Note that it doesn't handle sparse columns very well, and having a column that is empty except for the last row eats up 8 GB of RAM on my 64 bit system (it would probably crash a 32 bit system).

The actual language for cell input is documented ... in code. And the language for writing functions is documented in the Backwards repository, with the caveats in the Backwards directory of this repo.


Command Line
------------

* The only accepted argument is `-l`, which specifies a Backwards library file to load.
* The first argument after all specified libraries is a file to load. If no file is loaded, then an empty spreadsheet is given.
* The second argument is the file name to use to save files. If no second argument is specified, then the file is saved with the name of the file read in. If NO file name is specified, then the name "untitled.html" is used.
* Any other arguments are ignored.


Commands
--------
* Arrow keys : navigate.
* Page Up / Page Down : move to the next screen of rows.
* Home : goto cell A1
* `g` : type in a cell name, then enter, and the current cell cursor will be moved to that cell. Note that you cannot see the cell name that you are typing.
* `<` : start entering a label in this cell. Finish by pressing enter.
* `=` : start entering a formula in this cell. Finish by pressing enter.
* `q` or F7 : exit. You must next press either 'y' to save and exit, or 'n' to not save and exit to actually exit.
* `!` : recalculate the sheet
* `W` : save the sheet
* `dd` : delete the current cell
* `yy` : copy the current cell
* `pp` : paste the current cell
* `e` : edit the current cell's contents
* Shift left/right (also F9/F12 because ... Windows) : widen or narrow the current column. Columns can be between 1 and 40 cells wide. This is not a saved setting.
* `#` : Switch between column-major and row-major recalculation.
* `$` : Switch between top-to-bottom and bottom-to-top recalculation.
* `%` : Switch between left-to-right and right-to-left recalculation.
* `,` : Toggle between using ',' and '.' as the decimal separator. This is not a saved setting.

The sheet automatically recalculates after you finish entering a label or formula, and when you paste a cell. If a cell references a cell that hasn't been computed yet, then that cell will be computed, unless we are already in the process of computing that cell (circular reference). This ought to remove most of the reasons for wanting to change the order of sheet computation (but, if you feel the need, it is very customizable).


Formula Language
----------------

Examples:  
`A$1+@SUM(C2:D3)+4/7`

Cell references are like `A1` or `$B$2`. Anchoring row or column with '$' only matters when you copy/paste cells. Functions start with '@' like in DOS spreadsheet applications; their arguments are separated by semicolons (';'). Ranges use the colon (':'). You can do comparisons with '=', '>=', '<=', '>', '>', or '<>': the result is 1.0 for true and 0.0 for false. Use '&' to concatenate the string representations of two cells. The only half-attempt at internationalization that is supported is that 12,5 and 12.5 are treated the same. When you type a comma on numeric input, all of the numeric outputs will change to displaying a comma.


Standard Library
----------------

The following functions are all that is implemented. You can see the implementation in `Forwards/Tests/StdLib.txt`. If you load a library that redefines a function, it will successfully redefine that function. This can be used to improve the standard library (even though it is compiled into the program).

* MIN (%) - for functions marked (%), input is a variable number of arguments that can also be cell ranges. Empty cells and cells with labels are ignored. NaN is treated as an error value, not a missing value.
* MAX (%)
* SUM (%)
* COUNT (%)
* AVERAGE (%) - literally SUM / COUNT
* NAN - returns the special Not-a-Number value
* ABS - absolute value
* INT - truncate to integer
* ROUND - round to integer (ties away from zero)
