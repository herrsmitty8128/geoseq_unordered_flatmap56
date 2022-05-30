
import math


decimal_places = 7
max_probes = 126


def calc_probes(ratio) -> float:
    i = 1
    for _ in range(max_probes):
        i = math.ceil(i * ratio)
    return i


if __name__ == '__main__':
    
    ratios = []
    
    for bits in range(0,65):
        max_value = math.pow(2,bits)
        ratio = 1.0 if bits != 0 else 0.0
        divisor = 1
        for _ in range(decimal_places):
            divisor /= 10
            while calc_probes(ratio + divisor) < max_value:
                ratio += divisor
        if ratio == 1.0: ratio = 1.01
        ratios.append(round(ratio, decimal_places))
    
    print(ratios)
            
            