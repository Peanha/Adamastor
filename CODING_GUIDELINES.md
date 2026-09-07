# Coding Style Guide

## 1. Control flow

### 1.1 Early return instead of nesting
Rationale: keeps indentation shallow and separates reject conditions
from the actual logic. Known as guard clauses; avoids the arrow
anti-pattern.

Bad:

    if (a) {
        if (b) {
            do_something();
        }
    }

Good:

    if (!a)
        return;

    if (!b)
        return;

    do_something();

## 2. Variables

### 2.1 Declarations
Declare at the point of first use, always initialised. Prefer const,
then non-const. Never reorder declarations for cosmetic reasons: with
RAII, declaration order is construction order and reverse destruction
order.

### 2.2 Scope
Declare in the innermost block that needs the variable.

### 2.3 Naming
No aggressive abbreviations.

    Bad:  usr_cnt_tmp
    Good: user_count
