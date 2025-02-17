# Swipe

**Swipe** is a purpose-built interpreted scripting language for BetterEdit. Its syntax is a mash between JavaScript and Rust, and it has been built to be simple and easy to understand.

## Declaring variables

Swipe is dynamically typed, so no type annotations are needed. Swipe also doesn't have a variable declaration keyword, and instead variables automatically come into existence the first time their name is referenced:
```js
a = 5
b = "string value"
c = [1, 2, 3]
```

Variables get dropped at the end of the scope they were declared in:
```js
{
    a = "hi"
}
assert(a == null) // a has been dropped and its value is now null
```

## Semicolons

Semicolons are not required in Swipe, however they are allowed and can be used to disambiguate certain situations.

Swipe's parser by default matches greedily and ignores all whitespace; in other words, if an expression could match longer, it will. This means that something like: 
```rust
a = 5

+ 2


   + 3
```

will get matched as a single statement `a = 5 + 2 + 3`. In most cases, this is fine, as the most greedily matched tree is the one intended by the programmer, but in some situations semicolons may be used to cut the matching short:
```rust
a = 5;
+ 2
// gets parsed as two statements: `a = 5` and `+2`
```

If so desired, semicolons can be made obligatory by using `@strict`:
```rust
@strict true

a = 5
b = 2 // error: semicolon expected
```

Even in strict mode, the last semicolon of a block may still be omitted, similar to Rust:
```js
function myFun(a, b) {
    a += 5;
    a + b
}
```

Although unlike Rust, this does not have any semantic meaning; even if the last statement is concluded with a semicolon, the function evaluates to that statement as its return value.

## If statements

If statements follow the standard pattern of `if` keyword followed by a condition and a block of code, and then optionally an `else` keyword followed by another if statement or block of code.

If statements don't require parenthesis, however they may be used if you so wish to.
```rust
if x > 5 {
    // something new
}
else {
    // something blue
}
```

The truthy value of an if statement must be surrounded by braces `{}`. This is not required for an `else` however:
```rust
if x > 5 { "More than five!" } else "Less than five"
```

## Functions

Functions can be declared using the `function` keyword:
```js
function awesome(a, b) {
    return a + b;
}

awesome(2, 7);
```

Functions can be declared anonymously using the `function` syntax or the arrow syntax:
```js
myFun = function(a, b) {
    a + b
}
yourFun = (a, b) => a + b
herFun = x => x * 2
```

Function arguments can either be passed as positional arguments, or named arguments:
```js

```

Functions can be passed as arguments to other functions:
```js
function invoke(fun) {
    fun("hi!")
}

invoke(print)
invoke(x => error(x))
```

## Dictionaries

Dictionaries can be defined using the familiar `{}` syntax:
```js
dict = {
    key: 5,
    other: "value"
}
```

Unlike JavaScript, the keys of a dictionary must be identifiers, and not string literals. This means that an object with a key such as `"contains spaces"` is not valid in Swipe.

Note that in other languages, dictionaries are also often called objects, but since "object" in Swipe is already reserved for referring to a game object in the editor, they are instead called dictionaries.

## Objects

Objects, in other words level objects in the editor, can be accessed and manipulated through various built-in functions and methods:
```js
// Get selected objects as an array
objs = getSelectedObjects()
```

A new object can be created using the `create` function:
```js
obj = create(1, x = 60, y = 60)
```

The first argument of `create` is the object ID. You can find a list of all object IDs by running the following script, which adds all of them in a line into the current level:
```rust
x = 60
for i in 2000 {
    create(914, x = x, y = 300, text = i)
    try create(i, x = x, y = 240)
    x += 60
}
```

## Statements & expressions

All statements in Swipe are expressions, meaning that you can use constructs like If statements to assign values:
```rust
x = if condition {
    "value"
}
else {
    "other"
}
```

By default, a block of code evaluates to the last value in the block, or `null` if the block is empty:
```rust
x = {
    "value"
    "other"
}
assert(x == "other")

x = {}
assert(x == null)
```

## Returning

You can return from a function using the familiar `return` keyword:
```js
function addTwo(a, b) {
    return a + b
}
```

By default, a list expression always evaluates to the last expression in the list, so you can also omit the `return` keyword if the last expression is what you want to return:
```js
function addTwo(a, b) {
    a + b
}
```

Note that unlike Rust, where inserting a semicolon at the end of that statement would make the function return nothing, in Swipe semicolons serve no semantic purpose and even the following returns the result of `a + b` as its value:
```js
function addTwo(a, b) {
    a + b;
}
```

You can also use the `break` keyword to return from a block:
```rust
function some() {
    a = {
        b = 5
        b += 2
        break b
    }
    return a
}
```

## Comments

Line comments in Swipe are declared with `//` and block comments with `/*`:
```rust
// this is a comment
/* 
 this is also a comment
*/
```

## Error handling

You can catch errors using a `try` expression. Unlike other languages, where this is followed by a `catch` statement, in Swipe it is in the form of just `try ...` or `try ... else {}`.

```js
value = try null.member() else "error"
```

## Metadata

A script can have metadata at the start of the file, prefixed with `@`:
```rust
@version 1.0
@input value 1.0 "Value to use"
```

Valid metadata tags:

 * `@version <number>` - The version of Swipe the script was made for
 * `@input <ident> <value> <string>` - A value that the user is prompted to provide before running the script. 2nd argument is the default value for the value, and 3rd is a string literal that describes what the value is for
 * `@strict <bool>` - Enable strict mode, which makes the parser require semicolons at the end of statements
 * `@name <string>` - The name of the script

The metadata tags may be in any order, with the exception that `@version` must always be first.
