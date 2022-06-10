#!/usr/bin/python3

sz = 10

print(1)
print('pP('+', '.join(['vx{0}, f(vx{1}, vx{1})'.format(i, i+1) for i in range(sz)]) + ', ' + ', '.join(['f(vw{1}, vw{1}), vw{0}'.format(i, i+1) for i in range(sz)]) + ', vx0)')
print('pP('+', '.join(['f(vy{1}, vy{1}), vy{0}'.format(i, i+1) for i in range(sz)]) + ', ' + ', '.join(['vz{0}, f(vz{1}, vz{1})'.format(i, i+1) for i in range(sz)]) + ', vy0)')
