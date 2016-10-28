sketch:

    1 --walk-- 2 --walk-- 3 --S1,S2-- 4 --S2,S3-- 5 --walk-- 6 --walk-- 8
                                                  |                     |
                                                S4|S7                 S5|S8
                                                  |                     |
                                                 11                     9

services:
  - S1: 3,4     [10:10,11:00]
  - S2: 3,4,5   [10:10,11:00,11:10,12:00]
  - S3:   4,5   [11:10,12:00]
  - S4: 5,11    [12:10,13:00]
  - S5: 8,9     [13:10,14:00]
  - S6: 3,4,5   [18:10,19:00,19:10,20:00]
  - S7: 5,11    [20:00,21:00]
  - S8: 8,9     [21:10,22:00]

rule services:
  - S1+S2
  - S2+S3
  
foot edges:
  1->2 [10]
  2->3 [10]
  5->6 [10]
  6->8 [10]

test cases:
  A: first enter
  B: last leave
  C: leave+enter = interchange
     - walk(s) / same station
     - delay / cancel

A:     3,4,5     | cancel 3
   1,2,3,4,5     | cancel 3
B:     3,4,5     | cancel 5
       3,4,5,6,8 | cancel 5
C: 2,3,4,5,11    | cancel 5 (arrival S6)
   2,3,4,5,11    | cancel 5 (departure S4)
   2,3,4,5,6,8,9 | cancel 5 (arrival S6)
   2,3,4,5,6,8,9 | cancel 8 (departure S5) 