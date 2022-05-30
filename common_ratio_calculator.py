
#          Copyright Christopher Smith 2022.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)


# PLEASE NOTE: The purpose of this file is to calculate the common
# ratios used by geoseq_unordered_flatmap56 to determine the hash
# table probe sequence that should be used for each size of table.


import math

# The number of decimal places in our common ratios.
# A 'float' type in C has 6 decimal places of accuracy.
decimal_places = 6

# The number of values in the geometric sequence.
# Same as the maximum number of hash table probes.
geometric_sequence_length = 126

# Calculates the last number in a geometric
# sequence using 'ratio' and starting from 1.
def last_in_geometric_sequence(ratio) -> float:
    last_num = 1
    for _ in range(geometric_sequence_length):
        last_num = math.ceil(last_num * ratio)
    return last_num

# Convert the ratio to a string and append spaces
# until its length == decimal_places + 2
def ratio_to_string(ratio):
    ratio_str = str(ratio)
    while len(ratio_str) < decimal_places + 2:
        ratio_str += ' '
    return ratio_str


if __name__ == '__main__':
    
    # an array to hold the results
    ratios = []
    
    # Iterate over the range of 2^7 to 2^64 table sizes
    # the reason that we start with 7 bits is becuase the 
    # minimum capacity of the hash table is 128 buckets.
    for bits in range(7,65):
        hash_table_size = math.pow(2,bits)
        ratio = 1.0
        fract = 0.1
        for _ in range(decimal_places):
            while last_in_geometric_sequence(ratio + fract) < hash_table_size:
                ratio += fract
            fract /= 10
        ratios.append(round(ratio, decimal_places))
    
    # print the array of common ratios to stdout in a format
    # that we can use to copy-and-paste it into our c code.
    print(f'#define NUM_COMMON_RATIOS   {len(ratios)}')
    print('const float common_ratios[NUM_COMMON_RATIOS] = {\n\t',end='')
    for i,ratio in enumerate(ratios,1):
        ratio_string = ratio_to_string(ratio)
        if(i % 8 == 0):
            print(ratio_string + ',\n\t', end='')
        elif i == len(ratios):
            print(ratio_string + '\n', end='')
        else:
            print(ratio_string + ', ', end='')
    print('};')
    