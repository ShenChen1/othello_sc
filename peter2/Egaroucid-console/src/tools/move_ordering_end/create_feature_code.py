s = '''    // 7 edge + 2x
    {10, {COORD_B2, COORD_A1, COORD_B1, COORD_C1, COORD_D1, COORD_E1, COORD_F1, COORD_G1, COORD_H1, COORD_G2}}, // 26
    {10, {COORD_B2, COORD_A1, COORD_A2, COORD_A3, COORD_A4, COORD_A5, COORD_A6, COORD_A7, COORD_A8, COORD_B7}}, // 27
    {10, {COORD_B7, COORD_A8, COORD_B8, COORD_C8, COORD_D8, COORD_E8, COORD_F8, COORD_G8, COORD_H8, COORD_G7}}, // 28
    {10, {COORD_G2, COORD_H1, COORD_H2, COORD_H3, COORD_H4, COORD_H5, COORD_H6, COORD_H7, COORD_H8, COORD_G7}}, // 29

    // 8 triangle
    {10, {COORD_A1, COORD_B1, COORD_C1, COORD_D1, COORD_A2, COORD_B2, COORD_C2, COORD_A3, COORD_B3, COORD_A4}}, // 30
    {10, {COORD_H1, COORD_G1, COORD_F1, COORD_E1, COORD_H2, COORD_G2, COORD_F2, COORD_H3, COORD_G3, COORD_H4}}, // 31
    {10, {COORD_A8, COORD_B8, COORD_C8, COORD_D8, COORD_A7, COORD_B7, COORD_C7, COORD_A6, COORD_B6, COORD_A5}}, // 32
    {10, {COORD_H8, COORD_G8, COORD_F8, COORD_E8, COORD_H7, COORD_G7, COORD_F7, COORD_H6, COORD_G6, COORD_H5}}, // 33

    // 9 corner + block
    {10, {COORD_A1, COORD_C1, COORD_D1, COORD_E1, COORD_F1, COORD_H1, COORD_C2, COORD_D2, COORD_E2, COORD_F2}}, // 34
    {10, {COORD_A1, COORD_A3, COORD_A4, COORD_A5, COORD_A6, COORD_A8, COORD_B3, COORD_B4, COORD_B5, COORD_B6}}, // 35
    {10, {COORD_A8, COORD_C8, COORD_D8, COORD_E8, COORD_F8, COORD_H8, COORD_C7, COORD_D7, COORD_E7, COORD_F7}}, // 36
    {10, {COORD_H1, COORD_H3, COORD_H4, COORD_H5, COORD_H6, COORD_H8, COORD_G3, COORD_G4, COORD_G5, COORD_G6}}, // 37

    // 11 corner9
    {9, {COORD_A1, COORD_B1, COORD_C1, COORD_A2, COORD_B2, COORD_C2, COORD_A3, COORD_B3, COORD_C3, COORD_NO}}, // 42
    {9, {COORD_H1, COORD_G1, COORD_F1, COORD_H2, COORD_G2, COORD_F2, COORD_H3, COORD_G3, COORD_F3, COORD_NO}}, // 43
    {9, {COORD_A8, COORD_B8, COORD_C8, COORD_A7, COORD_B7, COORD_C7, COORD_A6, COORD_B6, COORD_C6, COORD_NO}}, // 44
    {9, {COORD_H8, COORD_G8, COORD_F8, COORD_H7, COORD_G7, COORD_F7, COORD_H6, COORD_G6, COORD_F6, COORD_NO}}  // 45'''

s = s.replace('{10, {', '').replace('{9, {', '').replace('{8, {', '').replace('{8,  {', '').replace('{7,  {', '').replace('{7, {', '').replace('{6, {', '').replace('{5, {', '').replace('\n\n', '\n').replace('}', '').replace('    ', '')
for num in reversed(range(100)):
    s = s.replace(', // ' + str(num), '')
    s = s.replace('  // ' + str(num), '')
    ss = '// ' + str(num) + ' '
    idx = s.find(ss)
    if idx >= 0:
        end = idx
        for i in range(idx, len(s)):
            if s[i] == '\n':
                end = i
                break
        s = s.replace(s[idx:end + 1], '')

s = s.replace(', COORD_NO', '')

print(s)

ss = [line.split(', ') for line in s.splitlines()]

def cell_to_coord(cell):
    cell = 63 - cell
    x = cell % 8
    y = cell // 8
    return 'COORD_' + chr(ord('A') + x) + str(y + 1)

def digit_space(n, r):
    n = str(n)
    l = len(n)
    for i in range(r - l):
        n = ' ' + n
    return n

#for cell in range(64):
#    print('#define COORD_' + cell_to_coord(cell) + ' ' + str(cell))

res = ''
for cell in range(64):
    coord = cell_to_coord(cell)
    tmp_arr = []
    for i in range(len(ss)):
        if coord in ss[i]:
            tmp = '{' + digit_space(i, 2) + ', P3' + str(len(ss[i]) - 1 - ss[i].index(coord)) + '}'
            tmp_arr.append(tmp)
    len_main = len(tmp_arr)
    for i in range(6 - len_main):
        tmp = '{ 0, PNO}'
        tmp_arr.append(tmp)
    res_tmp = '{' + digit_space(len_main, 2) + ', {' + ', '.join(tmp_arr) + '}}, // ' + coord
    res += res_tmp + '\n'
print(res)
