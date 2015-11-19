Types:
| X | int32 |
|:--|:------|
| X | uint32 |
| O | int64 |
| O | uint64 |
| X | ascii |
| O | unicode |
| O | decimal |
| O | byte vector |

Operators:
| O | constant |
|:--|:---------|
| O | default  |
| X | increment |
| O | delta    |
| O | copy     |
| O | tail     |

DELTA EXAMPLE
| MSG -> MSG |
|:-----------|
| -1A -> ASG |
| 2A -> AA   |


| 3 -> 3 |
|:-------|
| -2 -> 1 |
| 10 -> 11 |
| NULL -> NULL |
| 2 -> 13  (DOUBLE CHECK THIS) |

TODO: Figure out how NULL is determined
<br />http://jettekfix.com/ - tutorial



&lt;uint32 presence="optional"&gt;




&lt;constant /&gt;




&lt;/uint32&gt;



Say we have an optional uint32 copy operator
<br />If it's not present, use the previous value.

Find a table that tells if things are nullable or not.

For the next release, 0.3 get done:
  * All types
  * Operators: tail, delta (for numbers), copy, constant, default
  * XML parsing

For 0.4 get done:
  * Delta string
  * Decimal field operations - have separate operators for mantissa
  * Group, Sequence

For 0.45 get done:
  * Dictionary