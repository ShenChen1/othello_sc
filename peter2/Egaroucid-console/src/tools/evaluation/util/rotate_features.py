s = 'COORD_C1, COORD_D1, COORD_E1, COORD_F1, COORD_B2, COORD_C2, COORD_A3, COORD_B3, COORD_A4, COORD_B4'

s = s.split(', ')

res = [[] for _ in range(8)]

def xy_to_coord(x, y):
    return 'COORD_' + chr(ord('A') + x) + str(y + 1)

for elem in s:
    x = ord(elem[-2]) - ord('A')
    y = int(elem[-1]) - 1
    
    res[0].append(xy_to_coord(x, y))
    res[1].append(xy_to_coord(7 - y, 7 - x)) # black
    res[2].append(xy_to_coord(y, x)) # white
    res[3].append(xy_to_coord(7 - x, 7 - y)) # 180
    res[4].append(xy_to_coord(7 - x, y)) # v mirror
    res[5].append(xy_to_coord(x, 7 - y)) # h mirror
    res[6].append(xy_to_coord(7 - y, x)) # v black
    res[7].append(xy_to_coord(y, 7 - x)) # v white

for arr in res:
    arr_s = '{' + str(len(arr)) + ', {' + ', '.join(arr) + '}},'
    print(arr_s)